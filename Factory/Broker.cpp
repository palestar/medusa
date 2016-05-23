/*
	Broker.h

	This object is used to store / load widgets 
	(c)2005 Palestar, Richard Lyle
*/

#define FACTORY_DLL
#include "Broker.h"
#include "Standard/Time.h"
#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Debug/SafeThread.h"
#include "Widget.h"

#include <time.h>

// how many items to keep cached...
const int MAX_WIDGET_CACHE	= 1024 * 2;

//----------------------------------------------------------------------------

bool						Broker::sm_bEnableWidgetCache = false;

CriticalSection				Broker::sm_Lock;
Broker::BlockingLoadSet		Broker::sm_BlockingLoadSet;
Broker::BrokerHash			Broker::sm_BrokerHash;
Array< WidgetKey >			Broker::sm_AutoLoadList;		// list of widgets to autoload
Hash< WidgetKey, Widget * >	Broker::sm_AutoLoaded;			// hash of loaded widgets

bool						Broker::sm_bLoadingThreadActive = false;
Event						Broker::sm_LoadThreadEvent;		// this event is signaled when new load requests are made...
Broker::LoadList			Broker::sm_LoadList;			// list of widgets to load
Broker::LoadRequestHash		Broker::sm_LoadRequestHash;		// hash of widgets to the list of observers waiting for them to load..
Broker::RequestHash			Broker::sm_RequestHash;			// hash of all observers by their ID
Broker::LoadingThread *		Broker::sm_pLoadingThread = NULL;

//---------------------------------------------------------------------------------------------------

Broker::LoadingThread::LoadingThread() : SafeThread( Thread::STANDARD )
{}

//---------------------------------------------------------------------------------------------------

Broker::Request::Request() : m_bAttached( false )
{}

Broker::Request::~Request()
{
	detach();
}

void Broker::Request::attach()
{
	if (! m_bAttached )
	{
		AutoLock lock( &sm_Lock );
		sm_RequestHash.insert( requestID(), this );
		m_bAttached = true;
	}
}

void Broker::Request::detach()
{
	if ( m_bAttached )
	{
		AutoLock lock( &sm_Lock );
		sm_RequestHash.remove( requestID() );
		m_bAttached = false;
	}
}

//----------------------------------------------------------------------------

bool Broker::isLoadingThreadActive()
{
	return sm_bLoadingThreadActive;
}

bool Broker::startLoadingThread()
{
	AutoLock lock( &sm_Lock );
	if ( sm_bLoadingThreadActive )
		return false;		// thread is already active!
	
	sm_bLoadingThreadActive = true;
	sm_pLoadingThread = new LoadingThread();
	sm_pLoadingThread->resume();

	return true;
}

bool Broker::stopLoadingThread()
{
	AutoLock lock( &sm_Lock );
	if (! sm_bLoadingThreadActive )
		return false;		// thread is not active!
	
	sm_bLoadingThreadActive = false;
	sm_LoadThreadEvent.signal();
	delete sm_pLoadingThread;
	sm_pLoadingThread = NULL;

	flushCache();

	return true;
}

void Broker::flushCache()
{
	AutoLock lock( &sm_Lock );
	widgetCache().clear();
}

//! This starts an asynchronous load of the widget in the background, it will invoked the onLoaded() virtual 
//! function in the LoadRequest object once the widget has been loaded...
bool Broker::requestLoad( const WidgetKey & a_nKey, const ClassKey & a_nType, Request * a_pRequest, bool a_bBlocking )
{
	// first, check if widget is already available, if so then just notify the request object and early out.. 
	Widget::Ref pWidget = Widget::findWidgetByType( a_nKey, a_nType );
	if ( pWidget.valid() )
	{
		a_pRequest->onLoaded( pWidget );
		a_pRequest->signal();
		return true;
	}

	dword nStartTime = clock();

	AutoLock lock( &sm_Lock );
	// automatically start the loading thread if needed...
	if ( !isLoadingThreadActive() )
		if (! startLoadingThread() )
			return false;
	if (! sm_pLoadingThread || !a_pRequest )
		return false;

	// any other thread pushes a load request into the request hash, then optionally blocks
	// until their load request is completed..
	LoadRequestHash::Iterator iRequests = sm_LoadRequestHash.find( a_nKey );
	if (! iRequests.valid() )
	{
		sm_LoadRequestHash[ a_nKey ].insert( a_pRequest->requestID() );
		sm_LoadList.insert( a_nKey );
	}
	else
	{
		// request for this widget has already been queued, add our request ID onto the linked list...
		if (! (*iRequests).find( a_pRequest->requestID() ).valid() )
			(*iRequests).insert( a_pRequest->requestID() );
	}

	// register this request with the hash..
	a_pRequest->attach();

	// we need to detect if this is a request from the loading thread, if so, then 
	// the loading is done in a blocking fashion...
	if ( sm_pLoadingThread->inThread() )
	{
		Widget::Ref pWidget = Widget::findWidgetByType( a_nKey, a_nType );
		if ( !pWidget.valid() )
		{
			Broker * pBroker = findBroker( a_nKey );
			if ( pBroker != NULL )
			{
				// watch out for circular references while loading asset, if this condition is true
				// then it means we already have a pending request to load this asset 
				// so we should just return from this function.
				if ( sm_BlockingLoadSet.find( a_nKey ) != sm_BlockingLoadSet.end() )
				{
					LOG_WARNING( "Broker", "Detected circular load for %llu", a_nKey.m_Id );
					return false;
				}
				
				sm_BlockingLoadSet.insert( a_nKey );
				lock.release();

				//LOG( CharString().format("requestLoad() - Loading Widget %s", a_nKey.string() ) );
				pWidget = pBroker->load( a_nKey );

				lock.set( &sm_Lock );
				sm_BlockingLoadSet.erase( a_nKey );
			}
		}

		loadNotify( a_nKey, pWidget );
		return true;
	}

	lock.release();

	sm_LoadThreadEvent.signal();		// wake up the loading thread!
	if ( a_bBlocking )
	{
		a_pRequest->wait();				// wait for request to fail or succeed!
		LOG_DEBUG_LOW( "Broker", "Load of %s blocked for %u ms", 
			a_nKey.string().cstr(), clock() - nStartTime );
	}

	return true;
}

void Broker::loadNotify( const WidgetKey & a_nKey, Widget * a_pWidget )
{
	AutoLock lock( &sm_Lock );

	if ( a_pWidget != NULL && sm_bEnableWidgetCache )
	{
		bool bBumped = false;

		WidgetCacheList & cache = widgetCache();
		for( WidgetCacheList::iterator iWidget = cache.begin(); 
			iWidget != cache.end(); ++iWidget )
		{
			if ( *iWidget == a_pWidget )
			{
				cache.erase( iWidget );
				cache.push_front( a_pWidget );
				bBumped = true;
				break;
			}
		}

		if (! bBumped )
			cache.push_front( a_pWidget );
	
		// clear items from the cache..
		// TODO: We should be using the size of each widget to determine how big our cache 
		// should be instead of the number of widgets.
		while( cache.size() > MAX_WIDGET_CACHE )
		{
			cache.pop_back();
		}
	}

	// notify all requests of the loaded widget...
	LoadRequestHash::Iterator iRequests = sm_LoadRequestHash.find( a_nKey );
	if ( iRequests.valid() )
	{
		RequestList & RequestList = *iRequests;

		RequestList::Iterator iRequest = RequestList.head();
		while( iRequest.valid() )
		{
			dword nRequestID = *iRequest;
			iRequest.next();

			// If the request got deleted, it won't be in our hash..
			RequestHash::Iterator iFind = sm_RequestHash.find( nRequestID );
			if ( iFind.valid() )
			{
				Request * pRequest = *iFind;
				pRequest->onLoaded( a_pWidget );
				pRequest->signal();
				pRequest->detach();		// remove from sm_RequestHash now..		
			}
		}

		sm_LoadRequestHash.remove( a_nKey );
	}
}

Broker * Broker::findBroker( const WidgetKey & key )
{
	AutoLock lock( &sm_Lock );
	BrokerHashIt find = sm_BrokerHash.find( key );
	if ( find.valid() )
		return( (*find).broker );

	//TRACE( String("Broker not found for widget (%s)", key.string()) );
	return NULL;
}

//----------------------------------------------------------------------------

void Broker::registerWidget( Broker * pBroker, const WidgetKey & key, dword version, bool autoLoad, bool local )
{
	AutoLock lock( &sm_Lock );
	if ( autoLoad )
		sm_AutoLoadList.push( key );

	BrokerHashIt find = sm_BrokerHash.find( key );
	if ( find.valid() )
	{
		// compare versions, only use the newest version
		if ( version > (*find).version || ( local && version == (*find).version ) )
		{
			if ( sm_AutoLoaded.find( key ).valid() )
			{
				delete sm_AutoLoaded[ key ];
				sm_AutoLoaded.remove( key );
			}

			(*find).broker = pBroker;
			(*find).version = version;
		}
	}
	else
	{
		// add widget to the hash table
		WidgetBroker widget;
		widget.broker = pBroker;
		widget.version = version;

		sm_BrokerHash.insert( key, widget );
	}
}

void Broker::unregisterWidget( Broker * pBroker, const WidgetKey & key, dword version )
{
	AutoLock lock( &sm_Lock );

	BrokerHashIt find = sm_BrokerHash.find( key );
	if ( find.valid() )
	{
		if ( (*find).broker == pBroker && 
			(*find).version == version )
		{
			if ( sm_AutoLoaded.find( key ).valid() )
			{
				delete sm_AutoLoaded[ key ];
				sm_AutoLoaded.remove( key );
			}

			sm_BrokerHash.remove( key );
		}
	}
}

void Broker::autoLoadWidgets()
{
	AutoLock lock( &sm_Lock );
	for(int i=0;i<sm_AutoLoadList.size();i++)
	{
		WidgetKey key = sm_AutoLoadList[ i ];

		Broker * pBroker = findBroker( key );
		ASSERT( pBroker );

		sm_AutoLoaded[ key ] = pBroker->load( key );
	}
	sm_AutoLoadList.release();
}

//---------------------------------------------------------------------------------------------------

int Broker::loadingThread()
{
	while( sm_bLoadingThreadActive )
	{
		// wait for a load event...
		sm_LoadThreadEvent.wait( 100 );		// changed to a 100 ms wait time, just in case ...
		
		sm_Lock.lock();
		while( sm_LoadList.head().valid() )
		{
			LoadList::Iterator iLoad = sm_LoadList.head();
            WidgetKey nLoadKey = *iLoad;
			sm_LoadList.remove( iLoad );

			Widget::Ref pWidget = Widget::findWidget( nLoadKey );
			if (! pWidget.valid() )
			{
				Broker * pBroker = findBroker( nLoadKey );
				if ( pBroker != NULL )
				{
					sm_BlockingLoadSet.insert( nLoadKey );
					sm_Lock.unlock();

					//LOG( CharString().format("loadingThread() - Loading Widget %s", nLoadKey.string() ) );

					pWidget = pBroker->load( nLoadKey );

					sm_Lock.lock();
					sm_BlockingLoadSet.erase( nLoadKey );
				}
			}
		
			// notify all requests of the loaded widget...
			loadNotify( nLoadKey, pWidget );
		}
		sm_Lock.unlock();
	}

	return 0;
}

Broker::WidgetCacheList & Broker::widgetCache()
{
	static WidgetCacheList cache;
	return cache;
}


//----------------------------------------------------------------------------
// EOF



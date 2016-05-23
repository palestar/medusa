/*
	Broker.h

	This object is used to store / load widgets 
	(c)2005 Palestar, Richard Lyle
*/

#ifndef BROKER_H
#define BROKER_H

#include "Standard/Hash.h"
#include "Standard/CriticalSection.h"
#include "Standard/Event.h"
#include "Debug/SafeThread.h"
#include "WidgetKey.h"
#include "ClassKey.h"

#include <set>
#include <list>

#include "MedusaDll.h"

//----------------------------------------------------------------------------

class Widget;

class DLL Broker
{
public:
	// Types
	class DLL Request 
	{
	public:
		// Construction
		Request();
		virtual ~Request();
		// Interface
		virtual bool	onLoaded( Widget * a_pWidget ) { return false; };
		// Accessors
		bool			isAttached() const;						// is this request already attached
		dword			requestID() const;
		// Mutators
		void			attach();								// register this request with the hash
		void			detach();								// detach this request from the hash
		void			signal();								// signal's the load event
		bool			wait( dword nTimeout = 0xffffffff );	// blocks on the load event until signaled..  returns true on timeout
	private:
		// Data
		bool			m_bAttached;
		Event			m_LoadEvent;
	};
	friend class Request;

	// Data
	static bool			sm_bEnableWidgetCache;

	// Mutators
	virtual dword		version( const WidgetKey & key ) = 0;	
	virtual dword		size( const WidgetKey & key ) = 0;
	virtual Widget *	load( const WidgetKey & key ) = 0;
	virtual bool		store( Widget * pWidget, dword version, bool autoLoad ) = 0;

	// Static
	static bool			isLoadingThreadActive();
	static bool			startLoadingThread();
	static bool			stopLoadingThread();
	static void			flushCache();

	//! This starts an asynchronous load of the widget in the background, it will invoked the onLoaded() virtual 
	//! function in the LoadRequest object once the widget has been loaded...
	static bool			requestLoad( const WidgetKey & a_nKey, const ClassKey & a_nType, Request * a_pRequest, bool a_bBlocking );
	//! This is invoked to notify all waiting requests that the given load has either completed or failed.
	static void			loadNotify( const WidgetKey & a_nKey, Widget * a_pWidget );
	//! This function will return the broker object that has the given widget
	static Broker *		findBroker( const WidgetKey & key );

protected:
	// Static
	static void			registerWidget( Broker * pBroker, 
							const WidgetKey & key, 
							dword version, bool autoLoad, bool local );
	static void			unregisterWidget( Broker * pBroker,
							const WidgetKey & key, 
							dword version );
	
	static void			autoLoadWidgets();

private:
	// Types
	struct WidgetBroker
	{
		Broker *	broker;
		dword		version;
	};
	class LoadingThread : public SafeThread
	{
	public:
		// Construction
		LoadingThread();
		// Thread interface
		int run()
		{
			return loadingThread();
		}
	};
	friend class LoadingThread;

	typedef Hash< WidgetKey, WidgetBroker >		BrokerHash;
	typedef BrokerHash::Iterator				BrokerHashIt;
	typedef Hash< dword, Request * >			RequestHash;
	typedef List< dword >						RequestList;
	typedef Hash< WidgetKey, RequestList >		LoadRequestHash;
	typedef List< WidgetKey >					LoadList;
	typedef std::set< WidgetKey >				BlockingLoadSet;
	typedef std::list< Reference< Widget > >	WidgetCacheList;

	static CriticalSection
						sm_Lock;				// lock for broker static data
	static BlockingLoadSet
						sm_BlockingLoadSet;		// this set is used to prevent circular asset dependecy lock ups
	static BrokerHash	sm_BrokerHash;

	static Array< WidgetKey >
						sm_AutoLoadList;			// list of widgets to autoload
	static Hash< WidgetKey, Widget * >
						sm_AutoLoaded;			// hash of loaded widgets

	// Background Loading...
	static bool			sm_bLoadingThreadActive;
	static Event		sm_LoadThreadEvent;		// this event is signaled when new load requests are made...
	static LoadList		sm_LoadList;			// list of widgets to load
	static LoadRequestHash
						sm_LoadRequestHash;		// hash of widgets to the list of observers waiting for them to load..
	static RequestHash	sm_RequestHash;			// hash of all requests by their ID

	static LoadingThread *		
						sm_pLoadingThread;

	// Static
	static int			loadingThread();
	static WidgetCacheList &
						widgetCache();
};

//---------------------------------------------------------------------------------------------------

#pragma warning(disable:4311 )		// pointer truncation from 'const Broker::Request *const ' to 'dword'

inline bool Broker::Request::isAttached() const
{
	return m_bAttached;
}

inline dword Broker::Request::requestID() const
{
	return reinterpret_cast<dword>( this );
}

inline void Broker::Request::signal()
{
	m_LoadEvent.signal();
}

inline bool Broker::Request::wait( dword nTimeout /*= 0xffffffff*/ )
{
	return m_LoadEvent.wait( nTimeout );
}

//---------------------------------------------------------------------------------------------------


#endif

//----------------------------------------------------------------------------
// EOF



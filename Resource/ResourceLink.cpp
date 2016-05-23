/**
	@file ResourceLink.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 1/27/2012 7:53:03 PM
*/

#include "ResourceLink.h"

//---------------------------------------------------------------------------------------------------

ResourceObserver::ResourceObserver()
{}

ResourceObserver::ResourceObserver( ResourceLinkBase * a_pWatch )
{
	attach( a_pWatch );
}

ResourceObserver::~ResourceObserver()
{
	detachAll();
}

void ResourceObserver::attach( ResourceLinkBase * a_pWatch )
{
	a_pWatch->m_Observers.insert( this );
	m_Watching.insert( a_pWatch );
}

void ResourceObserver::detach( ResourceLinkBase * a_pWatch )
{
	WatchSet::iterator iWatch = m_Watching.find( a_pWatch );
	if ( iWatch != m_Watching.end() )
	{
		a_pWatch->m_Observers.erase( this );
		m_Watching.erase( iWatch );
	}
}

void ResourceObserver::detachAll()
{
	for( WatchSet::iterator iWatch = m_Watching.begin(); 
		iWatch != m_Watching.end(); )
	{
		WatchSet::iterator iDetach = iWatch++;
		(*iDetach)->m_Observers.erase( this );
		m_Watching.erase( iDetach );
	}
}

//---------------------------------------------------------------------------------------------------

ResourceLinkBase::ResourceLinkBase()
{}

ResourceLinkBase::~ResourceLinkBase()
{
	detach();

	for( ObserverSet::iterator iObserver = m_Observers.begin();
		iObserver != m_Observers.end(); )
	{
		ObserverSet::iterator iDetach = iObserver++;
		(*iDetach)->detach( this );
	}
}

bool ResourceLinkBase::onLoaded( Widget * a_pWidget )
{
	for( ObserverSet::iterator iObserver = m_Observers.begin();
		iObserver != m_Observers.end(); ++iObserver )
	{
		(*iObserver)->onLoaded( a_pWidget, this );
	}
	return true;
}

//---------------------------------------------------------------------------------------------------
//EOF

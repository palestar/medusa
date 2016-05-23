/*
    Referenced.h
    The class 'Referenced' is used as a mixin class to add support for smart-pointers.

	see Reference.h

    (c)2005 Palestar, Richard Lyle
*/

#ifndef REFERENCED_H
#define REFERENCED_H

#include <assert.h>
#include <stddef.h>
#include <map>

#include "Atomic.h"
#include "Types.h"
#include "CharString.h"
#include "Debug/ExceptionHandler.h"		// GetStack()

#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

//! Define to non-zero to enable reference tracking code, it adds a bit of overhead even when disabled, adds even more when turned on.
#define ENABLE_REF_TRACKING				0

//---------------------------------------------------------------------------------------------------

class DLL Referenced	// mix-in class to add reference counting
{
public:
	// Exceptions
	DECLARE_EXCEPTION( AlreadyDestroyed );

	// Types
	struct WeakReferenced : public AtomicCount
	{
		WeakReferenced( Referenced * a_pReferenced ) : AtomicCount( 1 ), m_pReferenced( a_pReferenced )
		{}

		Referenced *	m_pReferenced;
	};

#if ENABLE_REF_TRACKING
	// Set to true to enable tracking on all referenced objects.
	static bool			sm_bEnableTracking;
#endif

    // Construction
	Referenced();
	Referenced( const Referenced & a_Copy );
	virtual ~Referenced();											// do not remove, otherwise Reference<> pointers to this object will not destroy it correctly

    // Accessors
    int					referenceCount() const;						// what is the current reference count
	bool				isTrackingEnabled() const;	

    // Mutators
	WeakReferenced *	grabWeakReferenced();						// grab the WeakReferenced object, used by WeakReference<>, increments the weak reference count
    bool				grabReference( dword nTrackingId = 0 );		// grab a reference to this object, returns false if object is already being destroyed
    void				releaseReference( dword nTrackingId = 0 );			
	void				dumpTracking( CharString & a_sDump );		// dump tracking data into the provided string

protected:
	//! Data
    AtomicCount			m_StrongCount;
	WeakReferenced *	m_pWeakRef;							// our WeakReferenced object

#if ENABLE_REF_TRACKING
	//! Types
	struct Stack {
		dword nStack[ 8 ];
	};
	typedef std::map< dword, Stack >	TrackingMap;

	TrackingMap *		m_pTrackingMap;								// list of all current references
	static CriticalSection		
						sm_TrackingLock;
#endif
};

//---------------------------------------------------------------------------------------------------

inline Referenced::Referenced() : 
	m_pWeakRef( NULL )
#if ENABLE_REF_TRACKING
	,m_pTrackingMap( sm_bEnableTracking ? new TrackingMap() : NULL )
#endif
{}

inline Referenced::Referenced( const Referenced & a_Copy ) :
	m_pWeakRef( NULL )
#if ENABLE_REF_TRACKING
	,m_pTrackingMap( sm_bEnableTracking ? new TrackingMap() : NULL )
#endif
{}

inline Referenced::~Referenced()
{
	if ( m_pWeakRef != NULL )
	{
		m_pWeakRef->m_pReferenced = NULL;
		if ( m_pWeakRef->decrement() )
			delete m_pWeakRef;
	}
#if ENABLE_REF_TRACKING
	if ( m_pTrackingMap != NULL )
		delete m_pTrackingMap;
#endif
}

//---------------------------------------------------------------------------------------------------

inline int Referenced::referenceCount() const
{ 
    return m_StrongCount.m_nCount; 
}

inline bool Referenced::isTrackingEnabled() const
{
#if ENABLE_REF_TRACKING
	return m_pTrackingMap != NULL;
#else
	return false;
#endif
}

//---------------------------------------------------------------------------------------------------

#pragma warning( disable : 4311 )

inline Referenced::WeakReferenced * Referenced::grabWeakReferenced() 
{
	while( m_pWeakRef == NULL || !m_pWeakRef->increment() )
	{
		WeakReferenced * pNewWeak = new WeakReferenced( const_cast<Referenced *>( this ) );
		if ( !Atomic::compareSwap( reinterpret_cast<volatile int *>( &m_pWeakRef ), 
			0, reinterpret_cast<int>( pNewWeak ) ) )
		{
			// another thread got there before us, so delete our copy..
			delete pNewWeak;		
		}
	}

	return m_pWeakRef;
}

inline bool Referenced::grabReference( dword nTrackingId /*= 0*/ )
{
	if (! m_StrongCount.increment() )
		return false;		// failed to grab, because the object is being destroyed already..

#if ENABLE_REF_TRACKING
	if ( m_pTrackingMap != NULL && nTrackingId != 0 )
	{
		AutoLock lock( &sm_TrackingLock );
		Stack & stack = (*m_pTrackingMap)[ nTrackingId ];
		GetStack( stack.nStack, sizeof(stack.nStack)/sizeof(stack.nStack[0]) );
	}
#endif

	return true;
}

inline void Referenced::releaseReference( dword nTrackingId /*= 0*/ )
{
#if ENABLE_REF_TRACKING
	if ( m_pTrackingMap != NULL && nTrackingId != 0 )
	{
		AutoLock lock( &sm_TrackingLock );
		(*m_pTrackingMap).erase( nTrackingId );
	}
#endif
	
	if ( m_StrongCount.decrement() )
		delete this;
}

//---------------------------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------------------
// EOF

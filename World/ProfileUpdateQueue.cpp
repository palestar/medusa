/*
	ProfileUpdateQueue.cpp
	(c)2003 Palestar Inc, Richard Lyle
*/


#include "Debug/Assert.h"
#include "Standard/Time.h"
#include "ProfileUpdateQueue.h"
#include "WorldContext.h"

//----------------------------------------------------------------------------

// Construction
ProfileUpdateQueue::ProfileUpdateQueue() : m_DelayTime( 30 )
{}

ProfileUpdateQueue::~ProfileUpdateQueue()
{
	release();
}

void ProfileUpdateQueue::setDelayTime( dword seconds )
{
	m_DelayTime = seconds;
}

bool ProfileUpdateQueue::pushUpdate( dword nUserId, dword nFieldId, float fAmount )
{
	m_Lock.lock();

	// firstly, check if an ProfileUpdate structure is already in the queue, if found update that object
	ProfileUpdateHashIt user = m_UpdateHash.find( nUserId );
	if ( user.valid() )
	{
		FieldUpdateTreeIt field = (*user).find( nFieldId );
		if ( field.valid() )
		{
			ProfileUpdate * pUpdate = *field;
			ASSERT( pUpdate );
			
			// add amount to existing amount, return false
			pUpdate->fAmount += fAmount;
			m_Lock.unlock();

			return false;		
		}
	}

	ProfileUpdate * pUpdate = new ProfileUpdate;
	pUpdate->nTime = Time::seconds();
	pUpdate->nUserId = nUserId;
	pUpdate->nFieldId = nFieldId;
	pUpdate->fAmount = fAmount;

	// push into the queue
	m_Updates.push( pUpdate );
	// hash the object, so any further calls to pushUpdate() can quickly find the object we've already allocated
	m_UpdateHash[ nUserId ][ nFieldId ] = pUpdate;

	m_Lock.unlock();
	return true;
}

bool ProfileUpdateQueue::popUpdate( ProfileUpdate & update, bool bForce /*= false*/ )
{
	m_Lock.lock();
	if ( m_Updates.valid() )
	{
		ProfileUpdate * pUpdate = *m_Updates;
		if ( bForce || (Time::seconds() - pUpdate->nTime) > m_DelayTime )
		{
			// enough time has elapsed, pop the record so it can be sent to the client/server
			update = *pUpdate;
			
			// remove from the hash
			m_UpdateHash[ pUpdate->nUserId ].remove( pUpdate->nFieldId );
			if ( m_UpdateHash[ pUpdate->nUserId ].size() == 0 )
				m_UpdateHash.remove( pUpdate->nUserId );		// last field removed for user, remove the user entry now

			// remove from the queue
			m_Updates.pop();
			// release the memory
			delete pUpdate;
			// unlock this queue
			m_Lock.unlock();

			return true;
		}
	}

	m_Lock.unlock();
	return false;
}

void ProfileUpdateQueue::release()
{
	m_Lock.lock();
	// clean up allocated memory
	while( m_Updates.valid() )
	{
		delete *m_Updates;
		m_Updates.next();
	}
	m_UpdateHash.release();
	m_Lock.unlock();
}

//----------------------------------------------------------------------------
//EOF

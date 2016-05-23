/*
	ProfileUpdateQueue.h

	This object is used by WorldContext to manage updates to player profiles. It accumulates changes
	to player profiles over time as to lesson the amount of updates sent over the network.

	setDelayTime() sets the number of seconds to hold a update in the queue before it will get removed by popUpdate().

	(c)2003 Palestar Inc, Richard Lyle
*/

#ifndef PROFILEUPDATEQUEUE_H
#define PROFILEUPDATEQUEUE_H

#include "Standard/Types.h"
#include "Standard/Hash.h"
#include "Standard/Queue.h"
#include "Standard/Mutex.h"
#include "Profile.h"

//----------------------------------------------------------------------------

class ProfileUpdateQueue 
{
public:
	// Types
	struct ProfileUpdate
	{
		dword					nTime;				// time when the event occured
		dword					nUserId;			// user to modify profile
		dword					nFieldId;			// stat to modify
		float					fAmount;			// amount to add
	};

	// Construction
	ProfileUpdateQueue();
	~ProfileUpdateQueue();

	// Accessors
	dword					delayTime() const;
	int						updateCount() const;
	// Mutators
	void					setDelayTime( dword seconds );
	bool					pushUpdate( dword nUserId, dword nFieldId, float fAmount );
	bool					popUpdate( ProfileUpdate & update, bool bForce = false );
	void					release();

private:
	// Types
	typedef Tree< dword, ProfileUpdate *>			FieldUpdateTree;
	typedef FieldUpdateTree::Iterator				FieldUpdateTreeIt;
	typedef Hash< dword, FieldUpdateTree >			ProfileUpdateHash;
	typedef ProfileUpdateHash::Iterator				ProfileUpdateHashIt;

	// Data
	Mutex					m_Lock;
	dword					m_DelayTime;
	Queue< ProfileUpdate * >
							m_Updates;
	ProfileUpdateHash		m_UpdateHash;
};

//----------------------------------------------------------------------------

inline dword ProfileUpdateQueue::delayTime() const
{
	return m_DelayTime;
}

inline int ProfileUpdateQueue::updateCount() const
{
	return m_Updates.size();
}

//----------------------------------------------------------------------------


#endif

//----------------------------------------------------------------------------
//EOF

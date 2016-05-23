/**
	@file NBQueue.h
	@brief non-blocking (almost) queue that allows for thread-safe queue access with no locking.

	This queue works by using a Atomic operation to swap out a pointer to the queue when a thread
	wants to modify the queue. While a thread owns this pointer, no other threads can access this 
	queue. 

	This class provided a very efficient method to pass data to threads without having to use a 
	lock.

	(c)2012 Palestar Inc
	@author Richard Lyle @date 11/11/2012 9:17:45 PM
*/

#ifndef NBQUEUE_H
#define NBQUEUE_H

#include "Atomic.h"
#include "Thread.h"

#include <list>

//---------------------------------------------------------------------------------------------------

template<typename T>
class NBQueue
{
public:
	//! Types
	typedef std::list< T >		QueueList;

	//! Construction
	NBQueue() : m_pList( new QueueList() ), m_bDestroyed( false )
	{}
	~NBQueue()
	{
		m_bDestroyed = true;
		delete lockList();
	}

	//! thread-safe method to enqueue a value into this queue
	bool enqueue( T a_Push )
	{
		QueueList * pList = lockList();
		if (! pList )
			return false;
		
		pList->push_back( a_Push );
		m_pList = pList;

		return true;
	}

	//! thread-safe method to dequeue a value from this queue, returns false when no value is available.
	bool dequeue( T & a_Pop )
	{
		bool bPopped = false;
		
		QueueList * pList = lockList();
		if ( pList != 0 )
		{
			if ( pList->size() > 0 )
			{
				bPopped = true;
				a_Pop = pList->front();
				pList->pop_front();
			}

			m_pList = pList;
		}

		return bPopped;
	}

private:
	QueueList * lockList()		// this locks the queue list, this will yield the CPU until it can get the list..
	{
		// first get the pointer to our list, if we get 0 then another thread has is locked, so keep trying to
		// get the correct pointer..
		QueueList * pList = (QueueList *)m_pList;
		while( pList == 0 )
		{
			Thread::sleep( 0 );
			pList = (QueueList *)m_pList;
			if ( m_bDestroyed )
				return 0;
		}

		// now that we have the correct pointer, try to compare & swap it out to 0. Once that succeeds no other threads
		// will be able to lock our pointer.
		while(! Atomic::compareSwap( reinterpret_cast<volatile int *>( &m_pList ), reinterpret_cast<int>( pList ), 0 ) )
		{
			Thread::sleep( 0 );	
			if ( m_bDestroyed )
				return 0;
		}

		return pList;
	}

	//! Data
	volatile QueueList *	
					m_pList;
	volatile bool	m_bDestroyed;	
};


#endif

//---------------------------------------------------------------------------------------------------
//EOF

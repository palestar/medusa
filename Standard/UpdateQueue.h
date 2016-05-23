/**
	@file UpdateQueue.h
	@brief This queue is used to update a list of objects. These updates are time based so the CPU usage is always known.

	(c)2006 Palestar Inc
	@author Richard Lyle @date 12/30/2008 8:08:49 PM
*/

#ifndef UPDATEQUEUE_H
#define UPDATEQUEUE_H

#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class DLL UpdateQueue
{
public:
	//! This is the base class for any object that you want to insert into this queue.
	class DLL Node
	{
	public:
		// Construction
		Node( UpdateQueue * a_pUpdateQueue );
		virtual ~Node();

		//! Accessors
		UpdateQueue *		updateQueue() const;
		Node *				nextNode() const;
		Node *				previousNode() const;

		//! Update interface, return false to remove this node from the queue
		virtual bool		update() = 0;

	private:
		// Data
		UpdateQueue *		m_pQueue;
		Node *				m_pNext;
		Node *				m_pPrev;
	};

	// Construction
	UpdateQueue( qword a_nMaxCPU );
	
	//! Accessors
	Node *			head() const;
	Node *			tail() const;

	//! This updates as many object as possible in the queue, returns false if no objects are in the queue.
	bool			update();

protected:
	// Data
	Node *			m_pHead;
	Node *			m_pHead;
	qword			m_nMaxTime;
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF

/*
	Queue.h

	This template class is used to build a queue of objects (FIFO)
	(c)2005 Palestar, Richard Lyle
*/

#ifndef QUEUE_H
#define QUEUE_H

#include "List.h"

//-------------------------------------------------------------------------------

template< class T >
class Queue
{
public:
	// Construction
	Queue();
	virtual	~Queue();

	// Accessors
	bool				valid() const;				// is the head pointing at a valid object
	bool				empty() const;				// is the queue empty
	int					size() const;				// number of objects in the queue

	const List<T> &		list() const;				// get the const List object
	typename List<T>::Iterator	
						head() const;		
	typename List<T>::Iterator	
						tail() const;		

	T &					operator*() const;			// make sure it's valid() before

	// Mutators
	void				begin();					// move to the first item in the queue
	void				end();						// move to the last item in the queue
	
	List<T> &			list();
	void				setHead( typename List<T>::Iterator it );
	void				setTail( typename List<T>::Iterator it );

	void				insert( const T & object );	// insert object before the head
	T &					insert();
	void				insertAfter( const T & object );// insert after the head
	T &					insertAfter();
	void				push( const T & object );	// push object onto the tail
	T &					push();

	void				pop();						// pop current object off queue
	int					popBefore();				// remove all objects before the current object from the queue
	int					popAfter();					// remove all objects after the current object from the queue

	void				next();						// skip to next object in queue
	void				prev();						// skip to the previous object in the queue

	void				release();

	// Depreciated
	void				reset();					// depreciated, use begin() instead

private:
	// Types
	typedef typename List<T>::Iterator		Iterator;

	// Data
	List<T>				m_List;
	Iterator			m_iHead;
	Iterator			m_iTail;
};

//-------------------------------------------------------------------------------

template<class T>
inline Queue<T>::Queue()
{}

template<class T>
inline Queue<T>::~Queue()
{}

//-------------------------------------------------------------------------------

template<class T>
inline bool Queue<T>::valid() const
{
	return m_iHead.valid();
}

template<class T>
inline bool Queue<T>::empty() const
{
	return !m_List.head().valid();
}

template<class T>
inline int Queue<T>::size() const
{
	return m_List.size();
}


template<class T>
inline const List<T> & Queue<T>::list() const
{
	return m_List;
}

template<class T>
inline typename List<T>::Iterator Queue<T>::head() const
{
	return m_iHead;
}

template<class T>
inline typename List<T>::Iterator Queue<T>::tail() const
{
	return m_iTail;
}

template<class T>
inline T & Queue<T>::operator*() const
{
	return *m_iHead;
}

//-------------------------------------------------------------------------------

template<class T>
inline void Queue<T>::begin()
{
	m_iHead = m_List.head();
}

template<class T>
inline void Queue<T>::end()
{
	m_iHead = m_iTail;
}

template<class T>
inline List<T> & Queue<T>::list()
{
	return m_List;
}

template<class T>
inline void Queue<T>::setHead( typename List<T>::Iterator it )
{
	m_iHead = it;
}

template<class T>
inline void Queue<T>::setTail( typename List<T>::Iterator it )
{
	m_iTail = it;
}

template<class T>
inline void Queue<T>::insert( const T & object )
{
	m_iHead = m_List.insertBefore( m_iHead, object );
	if (! m_iTail.valid() )
		m_iTail = m_iHead;
}

template<class T>
inline T & Queue<T>::insert()
{
	m_iHead = m_List.insertBefore( m_iHead, T() );
	if (! m_iTail.valid() )
		m_iTail = m_iHead;
	
	return *m_iHead;
}

template<class T>
inline void Queue<T>::insertAfter( const T & object )
{
	m_iHead = m_List.insertAfter( m_iHead, object );
	if (! m_iTail.valid() )
		m_iTail = m_iHead;
}

template<class T>
inline T & Queue<T>::insertAfter()
{
	m_iHead = m_List.insertAfter( m_iHead, T() );
	if (! m_iTail.valid() )
		m_iTail = m_iHead;

	return *m_iHead;
}

template<class T>
inline void	Queue<T>::push( const T & object )
{
	m_iTail = m_List.insertAfter( m_iTail, object );
	if (! m_iHead.valid() )
		m_iHead = m_iTail;
}

template<class T>
inline T & Queue<T>::push()
{
	m_iTail = m_List.insertAfter( m_iTail, T() );
	if (! m_iHead.valid() )
		m_iHead = m_iTail;

	return *m_iTail;		// return the created object
}

template<class T>
inline void Queue<T>::pop()
{
	if ( m_iHead.valid() )
	{
		typename List<T>::Iterator remove = m_iHead;
		
		// advance to the next item
		m_iHead.next();
		// is this the last item in the list
		if ( remove == m_iTail )
			m_iTail.prev();
		// remove the item
		m_List.remove( remove );
	}
}

template<class T>
inline int Queue<T>::popBefore()
{
	// remove all linked list items before the current head
	if ( m_iHead.valid() )
		return m_List.removeBefore( m_iHead );

	return 0;
}

template<class T>
inline int Queue<T>::popAfter()
{
	if ( m_iHead.valid() )
	{
		// tail is now the current item
		m_iTail = m_iHead;
		// remove all objects after the current object
		return m_List.removeAfter( m_iHead );
	}

	return 0;
}

template<class T>
inline void Queue<T>::next()
{
	m_iHead.next();
}

template<class T>
inline void Queue<T>::prev()
{
	m_iHead.prev();
}

template<class T>
inline void Queue<T>::release()
{
	m_List.release();
	m_iHead = Iterator();
	m_iTail = Iterator();
}

//----------------------------------------------------------------------------

template<class T>
inline void Queue<T>::reset()
{
	m_iHead = m_List.head();
}

//-------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------
// EOF

/*
	List.h

	Linked List template class
	(c)2005 Palestar, Richard Lyle
*/

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

//----------------------------------------------------------------------------

template<class T> 
class List
{
public:
	// Types
	class Iterator;

	// Construction
	List();
	virtual ~List();

	// accessors
	Iterator			head() const;
	int					size() const;
	Iterator			find( const T & object ) const;

	// mutators
	Iterator			insert();
	Iterator			insert( const T &object );	
	Iterator			insertBefore( const Iterator &before, const T &object );
	Iterator			insertAfter( const Iterator &after, const T &object );

	void				remove( Iterator &remove );						// remove the specified item from the list	
	int					removeBefore( Iterator & iBefore );				// remove all items before the provided item
	int					removeAfter( Iterator & iAfter );				// remove all items after the specified item
	bool				remove( const T & object );

	void				release();			// release entire list

private:
	// Types
	struct Node
	{
		Node( const T &object ) : m_Object(object), m_pNext(NULL), m_pPrev(NULL) {}
		Node() : m_pNext(NULL), m_pPrev(NULL) {}

		T						m_Object;	// Do not move, this must be the first data member
		Node *					m_pNext;
		Node *					m_pPrev;
	};

	// Data
	Node *				m_pHead;
	int					m_nNodeCount;

public:
	// Types
	class Iterator	// This imbedded class is used to iterate through the linked list
	{
	public:
		// Friends
		friend class	List<T>;

		// Construction
		Iterator() : m_pNode( NULL ) 
		{}
		Iterator( Node * current ) : m_pNode( current ) 
		{}
		Iterator( const Iterator & copy ) : m_pNode( copy.m_pNode )
		{}
		Iterator( T * pointer) : m_pNode( (Node *)pointer ) 
		{}

		// Accessors
		T & operator*() const	
		{ 
			return m_pNode->m_Object; 
		}
		bool operator==( const Iterator & compare ) const
		{
			return m_pNode == compare.m_pNode;
		}
		bool valid() const		
		{ 
			return m_pNode != NULL; 
		}

		// Mutators
		bool find( const T & object )
		{
			while( valid() )
			{
				if ( m_pNode->m_Object == object )
					return true;
				next();
			}
			return false;
		}

		void next()				
		{ 
			m_pNode = m_pNode->m_pNext; 
		}
		void operator++( int )	
		{ 
			next(); 
		}	
		const Iterator & operator++()
		{
			next();
			return *this;
		}
		void prev()				
		{ 
			m_pNode = m_pNode->m_pPrev; 
		}
		void operator--( int )	
		{ 
			prev();
		}
		const Iterator & operator--()
		{
			prev();
			return *this;
		}

	private:
		// Data
		Node *		m_pNode;
	};
};

//-------------------------------------------------------------------------------

template<class T>
inline List<T>::List() : m_pHead(NULL), m_nNodeCount( 0 )
{
}

template<class T>
inline List<T>::~List()				
{  
	release();
}

//-------------------------------------------------------------------------------

template<class T>
inline typename List<T>::Iterator List<T>::head() const		
{ 
	return( Iterator( m_pHead ) ); 
}

template<class T>
inline int List<T>::size() const
{
	return m_nNodeCount;
}

template<class T>
inline typename List<T>::Iterator List<T>::find( const T & object ) const
{
	Iterator it = head();
	while( it.valid() )
	{
		if ( (*it) == object )
			return it;
		++it;
	}
	
	return Iterator();
}

//----------------------------------------------------------------------------

template<class T>
inline typename List<T>::Iterator List<T>::insert()
{
	Node *newNode = new Node;

	// attach node to head
	newNode->m_pNext = m_pHead;
	if (m_pHead != NULL)
		m_pHead->m_pPrev = newNode;
	m_pHead = newNode;
	// increment the number of nodes
	m_nNodeCount++;

	return( Iterator( newNode ) );
}

template<class T>
inline typename List<T>::Iterator List<T>::insert( const T &object )
{
	Node *newNode = new Node( object );

	// attach node to head
	newNode->m_pNext = m_pHead;
	if (m_pHead != NULL)
		m_pHead->m_pPrev = newNode;
	m_pHead = newNode;
	// increment the number of nodes
	m_nNodeCount++;

	return( Iterator( newNode ) );
}

template<class T>
inline typename List<T>::Iterator List<T>::insertBefore( const Iterator &before, const T &object )
{
	if ( ! before.valid() )
		return( insert( object ) );

	Node *newNode = new Node( object );

	newNode->m_pNext = before.m_pNode;
	newNode->m_pPrev = before.m_pNode->m_pPrev;

	if ( newNode->m_pNext )
		newNode->m_pNext->m_pPrev = newNode;
	if ( newNode->m_pPrev )
		newNode->m_pPrev->m_pNext = newNode;
	else
		m_pHead = newNode;
	// increment the number of nodes
	m_nNodeCount++;

	return( Iterator( newNode ) );
}

template<class T>
inline typename List<T>::Iterator List<T>::insertAfter( const Iterator &after, const T &object )
{
	if ( ! after.valid() )
		return( insert( object ) );

	Node *newNode = new Node( object );

	newNode->m_pNext = after.m_pNode->m_pNext;
	newNode->m_pPrev = after.m_pNode;

	if ( newNode->m_pNext )
		newNode->m_pNext->m_pPrev = newNode;
	if ( newNode->m_pPrev )
		newNode->m_pPrev->m_pNext = newNode;
	// increment the number of nodes
	m_nNodeCount++;

	return( Iterator( newNode ) );
}

template<class T>
inline void List<T>::remove( Iterator & remove )
{
	if ( remove.m_pNode == NULL )
		return;								// not a valid iterator

	Node * removeNode = remove.m_pNode;
	remove.m_pNode = removeNode->m_pNext;	// advance interator to next list item

	if (removeNode->m_pPrev != NULL)
		removeNode->m_pPrev->m_pNext = removeNode->m_pNext;
	else
		m_pHead = removeNode->m_pNext;

	if ( removeNode->m_pNext )
		removeNode->m_pNext->m_pPrev = removeNode->m_pPrev;

	// release the node
	delete removeNode;
	// decrease the node count
	m_nNodeCount--;
}

template<class T>
inline int List<T>::removeBefore( Iterator & iBefore )
{
	int nRemoved = 0;

	if ( iBefore.valid() )
	{
		Node * pBefore = iBefore.m_pNode;
		
		Node * pRemove = pBefore->m_pPrev;
		while( pRemove != NULL )
		{
			// get the previous item before delete
			Node * pNext = pRemove->m_pPrev;
			// delete the node
			delete pRemove;
			// decrease the node count
			m_nNodeCount--;
			nRemoved++;
			// next item
			pRemove = pNext;
		}

		// fix the links
		pBefore->m_pPrev = NULL;
		// set this item as the head now
		m_pHead = pBefore;
	}

	return nRemoved;
}

template<class T>
inline int List<T>::removeAfter( Iterator & iAfter )
{
	int nRemoved = 0;

	if ( iAfter.valid() )
	{
		Node * pAfter = iAfter.m_pNode;

		Node * pRemove = pAfter->m_pNext;
		while( pRemove != NULL )
		{
			// get the next list item before the delete
			Node * pNext = pRemove->m_pNext;
			// delete the node node
			delete pRemove;
			// decrement the node count
			m_nNodeCount--;
			nRemoved++;
			// next item
			pRemove = pNext;
		}

		// fix links now
		pAfter->m_pNext = NULL;
	}

	return nRemoved;
}

template<class T>
inline bool List<T>::remove( const T & object )
{
	Iterator iRemove = find( object );
	if ( iRemove.valid() )
	{
		remove( iRemove );
		return true;
	}
	return false;
}

template<class T>
inline void List<T>::release()
{
	Node * pNode = m_pHead;
	while( pNode != NULL )
	{
		Node * pNext = pNode->m_pNext;
		delete pNode;

		pNode = pNext;
	}

	m_pHead = NULL;
	m_nNodeCount = 0;
}

//-------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF

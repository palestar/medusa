/*
	Tree.h

	Binary Tree template class, see Standard/Tree_Test for how to use this class
	(c)2005 Palestar, Richard Lyle
*/

#ifndef TREE_H
#define TREE_H

#include "Standard/List.h"

#include <assert.h>
#include <stdlib.h>

#pragma warning( disable: 4786 )

//----------------------------------------------------------------------------

template<class K,class O> 
class Tree
{
public:
	// Exceptions
	class LeafNotFound {};

	// Types
	struct Leaf
	{
		Leaf( const K &key ) : m_Key(key), 
			m_pParent(NULL), m_pLower(NULL), m_pHigher(NULL) {}
		Leaf( const K &key, const O & object ) : m_Key( key ), m_Object( object ), 
			m_pParent(NULL), m_pLower(NULL), m_pHigher(NULL) {}

		K			m_Key;
		O			m_Object;

		Leaf *		m_pParent;
		Leaf *		m_pLower;
		Leaf *		m_pHigher;
	};

	class Iterator	// This imbedded class is used to iterate through the tree
	{
	public:
		// Construction
		Iterator() : m_pLeaf( NULL ) 
		{}
		Iterator( Leaf * pLeaf ) : m_pLeaf( pLeaf ) 
		{}

		// Accessors
		O &			operator*() const	
		{ 
			return m_pLeaf->m_Object; 
		}
		K &			key() const
		{
			return m_pLeaf->m_Key;
		}
		Leaf *		leaf() const
		{
			return m_pLeaf;
		}
		bool		operator==( const Iterator & compare ) const
		{
			return m_pLeaf == compare.m_pLeaf;		// same leaf?
		}
		bool		valid() const		
		{ 
			return m_pLeaf != NULL; 
		}
	
		// Mutators
		bool		next()				
		{ 
			if ( m_pLeaf == NULL )
				return false;
			if ( m_pLeaf->m_pLower != NULL )
				return (m_pLeaf = m_pLeaf->m_pLower) != NULL;
			if ( m_pLeaf->m_pHigher != NULL )
				return (m_pLeaf = m_pLeaf->m_pHigher) != NULL;

			// move back up the tree until we get to a leaf that we have not branched right (higher)
			Leaf * pParent = m_pLeaf->m_pParent;
			while( pParent != NULL )
			{
				Leaf * pHigher = pParent->m_pHigher;
				if ( pHigher != NULL && pHigher != m_pLeaf )
					return (m_pLeaf = pHigher) != NULL;

				// set the parent as the current leaf
				m_pLeaf = pParent;
				pParent = m_pLeaf->m_pParent;
			}

			// last item, set leaf to null
			m_pLeaf = NULL;
			return false;
		}
		void operator++( int )	
		{ 
			next(); 
		}	
		void operator++()
		{
			next();
		}

	private:
		// Data
		Leaf *		m_pLeaf;
	};

	// Construction
	Tree();
	Tree( const Tree<K,O> & copy );
	virtual			~Tree();

	// Accessors
	Iterator		find( const K &key ) const;
	Iterator		head() const; 
	int				size() const;

	Leaf *			root() const;

	// Mutators
	Iterator		insert( const K &key, const O &object);			// add leaf, replaces if key already in tree
	bool			remove( const K &key );							// remove leaf, returns true if object was removed
	void			release();										// release entire tree

	Tree<K,O> &		operator=(const Tree<K,O> & copy );				// copies another tree into this tree

	O &				operator[](const K &key);						// access/add leaf
	const O &		operator[](const K &key) const ;				// access leaf
		
private:
	// Data
	Leaf *			m_pRoot;										// root leaf
	int				m_nLeafCount;									// number of leaves in this tree
	// Accessors
	Leaf *			findLeaf( const K &key ) const;
	// Mutators
	void			attachLeaf( Leaf * pLeaf );
	void			detachLeaf( Leaf * pLeaf );
	void			destroyTree( Leaf * pLeaf );
	void			copyTree( Leaf * pLeaf );
};

//-------------------------------------------------------------------------------

template<class K,class O>
inline Tree<K,O>::Tree() : m_pRoot(NULL), m_nLeafCount( 0 )
{}

template<class K,class O>
inline Tree<K,O>::Tree( const Tree<K,O> & copy ) : m_pRoot( NULL ), m_nLeafCount( 0 )
{
	// call operator=
	*this = copy;
}

template<class K,class O>
inline Tree<K,O>::~Tree()
{
	release();
}

template<class K,class O>
inline typename Tree<K,O>::Iterator Tree<K,O>::find( const K &key ) const
{
	Leaf *pLeaf = findLeaf( key );
	if ( pLeaf != NULL)
		return Iterator( pLeaf );

	return( Iterator() );	// return invalid widget
}

template<class K,class O>
inline typename Tree<K,O>::Iterator Tree<K,O>::head() const
{
	return Iterator( m_pRoot );
}

template<class K,class O>
inline int Tree<K,O>::size() const
{
	return m_nLeafCount;
}

template<class K,class O>
inline typename Tree<K,O>::Leaf * Tree<K,O>::root() const
{
	return m_pRoot;
}

//----------------------------------------------------------------------------

template<class K,class O>
inline typename Tree<K,O>::Iterator Tree<K,O>::insert( const K &key, const O & object )
{
	Leaf * pLeaf = findLeaf( key );
	if ( pLeaf == NULL)
	{
		// create a new leaf
		pLeaf = new Leaf( key, object );
		attachLeaf( pLeaf );

		m_nLeafCount++;
	}
	else
		pLeaf->m_Object = object;		// key already exists, reassign object

	return Iterator( pLeaf );
}

template<class K,class O>
inline bool Tree<K,O>::remove( const K &key )
{
	Leaf * pLeaf = findLeaf( key );
	if ( pLeaf != NULL)
	{
		detachLeaf( pLeaf );
		delete pLeaf;

		// increment the number of leaves attached
		m_nLeafCount--;
		return true;
	}

	return false;
}

template<class K,class O>
inline void Tree<K,O>::release()
{
	if (m_pRoot != NULL)
	{
		destroyTree( m_pRoot );
		m_pRoot = NULL;
		m_nLeafCount = 0;
	}
}

template<class K,class O>
inline Tree<K,O> & Tree<K,O>::operator=(const Tree<K,O> & copy )
{
	// destroy previous tree
	if (m_pRoot != NULL)
	{
		destroyTree( m_pRoot );
		m_pRoot = NULL;
		m_nLeafCount = 0;
	}

	// copy in new tree
	copyTree( copy.m_pRoot );
	// set the leaf count
	m_nLeafCount = copy.m_nLeafCount;

	return *this;
}

template<class K,class O>
inline O & Tree<K,O>::operator[](const K &key)
{
	Leaf * pLeaf = findLeaf( key );
	if ( pLeaf == NULL)
	{
		// create a new leaf
		pLeaf = new Leaf( key, O() );
		attachLeaf( pLeaf );

		m_nLeafCount++;
	}

	return pLeaf->m_Object;
}

template<class K,class O>
inline const O & Tree<K,O>::operator[](const K &key) const
{
	Leaf * pLeaf = findLeaf( key );
	if ( pLeaf == NULL)
		throw LeafNotFound();

	return pLeaf->m_Object;
}

//-------------------------------------------------------------------------------

template<class K,class O>
inline typename Tree<K,O>::Leaf * Tree<K,O>::findLeaf( const K &key ) const
{
	Leaf * pTraverse = m_pRoot;
	while(pTraverse != NULL)
	{
		if ( pTraverse->m_Key == key)
			break;

		if ( pTraverse->m_Key < key )
			pTraverse = pTraverse->m_pHigher;
		else 
			pTraverse = pTraverse->m_pLower;
	}

	return( pTraverse );
}

// This function assumes a unfound leaf is being attached

template<class K,class O>
inline void Tree<K,O>::attachLeaf( Leaf * pLeaf )
{
	if ( m_pRoot != NULL )
	{
		// attach the leaf to the tree
		Leaf * pTraverse = m_pRoot;
		while( true )
		{
			if ( pLeaf->m_Key > pTraverse->m_Key )
			{
				if (pTraverse->m_pHigher == 0)
				{
					// attach the leaf as a child
					pLeaf->m_pParent = pTraverse;
					pTraverse->m_pHigher = pLeaf;
					return;
				}
				else
					pTraverse = pTraverse->m_pHigher;
			}
			else 
			{
				if (pTraverse->m_pLower == 0)
				{
					// attach leaf as a child
					pLeaf->m_pParent = pTraverse;
					pTraverse->m_pLower = pLeaf;
					return;
				}
				else
					pTraverse = pTraverse->m_pLower;
			}
		}
	}

	m_pRoot = pLeaf;
}

template<class K,class O>
inline void Tree<K,O>::detachLeaf( Leaf * pLeaf )
{
	if (pLeaf != m_pRoot)
	{
		// detach the leaf from it's parent
		if ( pLeaf->m_pParent->m_pLower == pLeaf )
			pLeaf->m_pParent->m_pLower = 0;
		else
			pLeaf->m_pParent->m_pHigher = 0;
	}
	else
		m_pRoot = 0;

	// attach the children of leaf to the tree
	if (pLeaf->m_pLower != 0)
	{
		pLeaf->m_pLower->m_pParent = 0;
		attachLeaf( pLeaf->m_pLower );
	}
	if (pLeaf->m_pHigher != 0)
	{
		pLeaf->m_pHigher->m_pParent = 0;
		attachLeaf( pLeaf->m_pHigher );
	}
}

template<class K,class O>
inline void Tree<K,O>::destroyTree( Leaf * pLeaf )
{
	if ( pLeaf == NULL )
		return;

	destroyTree( pLeaf->m_pLower);
	destroyTree( pLeaf->m_pHigher);

	delete pLeaf;
}

template<class K,class O>
inline void Tree<K,O>::copyTree( Leaf * pLeaf )
{
	if ( pLeaf == NULL )
		return;

	// create a new leaf
	attachLeaf( new Leaf( pLeaf->m_Key, pLeaf->m_Object ) );

	copyTree( pLeaf->m_pLower );
	copyTree( pLeaf->m_pHigher );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF


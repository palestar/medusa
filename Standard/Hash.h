/*
	Hash.h

	Hash Table template class
	(c)2005 Palestar, Richard Lyle
*/

#ifndef HASH_H
#define HASH_H

#include "Standard/Tree.h"
#include <assert.h>

//----------------------------------------------------------------------------

// NOTE: The key is required to be a number AND tableSize shoud always be a power of 2 (such as 4,8,16,32, ... )

template<class K,class O, unsigned int tableSize = 32> 
class Hash
{
public:
	// Types
	class Iterator	// This imbedded class is used to iterate through the tree
	{
	public:
		// Construction
		Iterator() : m_pHash( NULL ), m_nTree( -1 )
		{}
		Iterator( const Hash * pHash, unsigned int nTree, typename Tree<K,O>::Iterator iLeaf ) 
			: m_pHash( pHash ), m_nTree( nTree ), m_iLeaf( iLeaf ) 
		{}

		// Accessors
		O &			operator*() const	
		{ 
			return *m_iLeaf; 
		}
		K &			key() const
		{
			return m_iLeaf.key();
		}
		bool		operator==( const Iterator & compare ) const
		{
			return m_iLeaf == compare.m_iLeaf;		// same leaf?
		}
		bool		valid() const		
		{ 
			return m_iLeaf.valid();
		}
	
		// Mutators
		void		next()				
		{ 
			// move to next item in the current tree
			if (! m_iLeaf.next() )
			{
				// if that was the last item, move the the head of the next tree if possible
				while( !m_iLeaf.valid() )
				{
					++m_nTree;
					if ( m_nTree >= tableSize )
						break;
					m_iLeaf = m_pHash->constTree( m_nTree ).head();
				}
			}
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
		const Hash<K,O,tableSize> *	
								m_pHash;

		unsigned int			m_nTree;
		typename Tree<K,O>::Iterator
							m_iLeaf;
	};

	// Accessors
	Iterator		find( const K &key ) const;			// find leaf, returns 0 if leaf not found
	Iterator		head() const;						// returns the first item in the hash
	int				size() const;						// how many entries in the hash table

	unsigned int	treeCount() const;
	const Tree<K,O> &		
					constTree( int n ) const;			// const version of tree()

	// Mutators
	Iterator		insert( const K &key, const O &object);	// add leaf, replaces if key already in tree
	bool			remove( const K &key );				// remove leaf, returns true if object was removed
	void			release();							// release entire tree

	O &				operator[](const K &key);			// access/add leaf
	const O &		operator[](const K &key) const;

	Tree<K,O> &		tree( int n );						// needed for streaming

private:
	// Data
	Tree<K,O>		m_Hash[ tableSize ];

	// Accessors
	Tree<K,O> &		hashKey( const K & key );
	const Tree<K,O> & 
					constHashKey( const K &key ) const;	
	unsigned int	keyTree( const K & key ) const;
};

//-------------------------------------------------------------------------------

template<class K,class O,unsigned int tableSize>
inline typename Hash<K,O,tableSize>::Iterator Hash<K,O,tableSize>::find( const K &key ) const
{
	unsigned int nTree = keyTree( key );
	return Iterator( this, nTree, m_Hash[ nTree ].find( key ) );
}

template<class K,class O,unsigned int tableSize>
inline typename Hash<K,O,tableSize>::Iterator Hash<K,O,tableSize>::head() const
{
	for(unsigned int i=0;i<tableSize;i++)
		if ( m_Hash[ i ].head().valid() )
			return Iterator( this, i, m_Hash[ i ].head() );

	return Iterator();
}

template<class K,class O,unsigned int tableSize>
inline int Hash<K,O,tableSize>::size() const
{
	int size = 0;
	for(unsigned int i=0;i<tableSize;i++)
		size += m_Hash[ i ].size();

	return size;
}

template<class K,class T,unsigned int S>
inline unsigned int	Hash<K,T,S>::treeCount() const
{
	return S;
}

template<class K,class T,unsigned int S>
inline const Tree<K,T> & Hash<K,T,S>::constTree( int n ) const
{
	return m_Hash[ n ];
}

//-------------------------------------------------------------------------------

template<class K,class O,unsigned int tableSize>
inline typename Hash<K,O,tableSize>::Iterator Hash<K,O,tableSize>::insert( const K &key, const O &object)
{
	unsigned int nTree = keyTree( key );
	return Iterator( this, nTree, m_Hash[ nTree ].insert( key, object ) );
}

template<class K,class O,unsigned int tableSize>
inline bool Hash<K,O,tableSize>::remove( const K &key )
{
	return( hashKey( key ).remove( key ));
}

template<class K,class O,unsigned int tableSize>
inline void Hash<K,O,tableSize>::release()
{
	for(unsigned int i=0;i<tableSize;i++)
		m_Hash[i].release();
}

template<class K,class O,unsigned int tableSize>
inline O & Hash<K,O,tableSize>::operator[](const K &key)
{
	return( hashKey( key )[key] );
}

template<class K,class O,unsigned int tableSize>
inline const O & Hash<K,O,tableSize>::operator[](const K &key) const
{
	return( constHashKey( key )[key] );
}

template<class K,class T,unsigned int S>
inline Tree<K,T> & Hash<K,T,S>::tree( int n )
{
	return( m_Hash[ n ] );
}

//----------------------------------------------------------------------------

// explicitly cast to unsigned so we don't crash on negative values for the key

template<class K,class O,unsigned int tableSize>
inline unsigned int Hash<K,O,tableSize>::keyTree( const K & key ) const
{
	return (unsigned int)( key & (tableSize-1) );
}

template<class K,class O,unsigned int tableSize>
inline const Tree<K,O> & Hash<K,O,tableSize>::constHashKey( const K &key ) const	
{ 
	return m_Hash[ key & (tableSize-1) ];			
}

template<class K,class O,unsigned int tableSize>
inline Tree<K,O> & Hash<K,O,tableSize>::hashKey( const K &key )
{ 
	return m_Hash[ key & (tableSize-1)]; 
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF


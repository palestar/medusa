/*
	HashList.h

	This hash object uses a linked list instead of a binary tree for storing the hashed items.
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef HASHLIST_H
#define HASHLIST_H



//----------------------------------------------------------------------------

template<class KEY,class OBJECT,unsigned int SIZE >
class HashList
{
public:
	// Construction
	HashList();
	~HashList();

	// Accessors
	OBJECT *		find( const KEY & nKey ) const;			// find item

	// Mutators
	OBJECT &		operator[]( const KEY & nKey );			// access/add leaf

	bool			remove( KEY & nKey );
	void			release();								// release entire hash
	
private:
	// Types
	struct Item
	{
		Item();
		~Item();

		KEY			nKey;
		OBJECT		object;
		Item *		pNext;
	};

	// Data
	Item *			m_Hash[ SIZE ];
};

//----------------------------------------------------------------------------

template<class KEY,class OBJECT,unsigned int SIZE >
inline HashList<KEY,OBJECT,SIZE>::HashList()
{
	// clear the list
	for(int i=0;i<SIZE;i++)
		m_Hash[i] = NULL;
}

template<class KEY,class OBJECT,unsigned int SIZE >
inline HashList<KEY,OBJECT,SIZE>::~HashList()
{
	release();
}

template<class KEY,class OBJECT,unsigned int SIZE >
inline OBJECT * HashList<KEY,OBJECT,SIZE>::find( const KEY & nKey ) const
{
	Item * pFind = m_Hash[ nKey & (SIZE-1) ];
	while( pFind != NULL )
	{
		if ( pFind->nKey == nKey )
			return pFind->object;
		pFind = pFind->pNext;
	}

	return NULL;
}

template<class KEY,class OBJECT,unsigned int SIZE >
inline OBJECT & HashList<KEY,OBJECT,SIZE>::operator[](const KEY & nKey)
{
	int nIndex = (int)(nKey & (SIZE-1));

	Item * pFind = m_Hash[ nIndex ];
	while( pFind != NULL )
	{
		if ( pFind->nKey == nKey )
			return pFind->object;
		pFind = pFind->pNext;
	}

	// create new item
	pFind = new Item;
	pFind->nKey = nKey;
	pFind->pNext = m_Hash[ nIndex ];
	m_Hash[ nIndex ] = pFind;

	return pFind->object;
}

template<class KEY,class OBJECT,unsigned int SIZE >
inline bool HashList<KEY,OBJECT,SIZE>::remove( KEY & nKey )
{
	int nIndex = (int)(nKey & (SIZE-1));

	Item * pFind = m_Hash[ nIndex ];
	Item * pPrev = NULL;
	while( pFind != NULL )
	{
		if ( pFind->nKey == nKey )
		{
			if ( pPrev != NULL )
				pPrev->pNext = pFind->pNext;
			else
				m_Hash[ nIndex ] = pFind->pNext;

			pFind->pNext = NULL;
			delete pFind;

			return true;
		}

		pPrev = pFind;
		pFind = pFind->pNext;
	}

	return false;
}


template<class KEY,class OBJECT,unsigned int SIZE >
inline void HashList<KEY,OBJECT,SIZE>::release()
{
	for(int i=0;i<SIZE;i++)
	{
		if ( m_Hash[i] != NULL )
			delete m_Hash[i];
		m_Hash[i] = NULL;
	}
}

//----------------------------------------------------------------------------

template<class KEY,class OBJECT,unsigned int SIZE >
inline HashList<KEY,OBJECT,SIZE>::Item::Item() : pNext( NULL )
{}

template<class KEY,class OBJECT,unsigned int SIZE >
inline HashList<KEY,OBJECT,SIZE>::Item::~Item()
{
	if ( pNext != NULL )
		delete pNext;
}

//----------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
//EOF

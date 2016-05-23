/*
	Containers.h
	Property container objects

	(c)2006 Palestar, Richard Lyle
*/

#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <vector>
#include <list>

#include "Reflection/Property.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class StaticContainer : public Property::Container
{
public:
	StaticContainer( int nCount, int nSize ) : m_nCount( nCount ), m_nSize( nSize )
	{}

	// Property::Container interface
	virtual bool isDynamic() const
	{
		return false;
	}
	virtual int count( void * pThis ) const
	{
		return m_nCount;
	}
	virtual void * pointer( void * pThis, int n ) const
	{
		if ( n >= m_nCount || n < 0 )
			return NULL;
		return (byte *)pThis + (n * m_nSize);
	}

private:
	// Data
	int			m_nCount;
	int         m_nSize;
};


//---------------------------------------------------------------------------------------------------

// Array<T> container
template<typename T>
class ArrayContainer : public Property::Container
{
public:
	// Property::Container interface
	virtual bool isDynamic() const
	{
		return true;
	}
	virtual int count( void * pThis ) const
	{
		Array<T> * pArray = (Array<T> *)pThis;
		return pArray->size();
	}
	virtual void * pointer( void * pThis, int n ) const
	{
		if ( n < 0 )
			return NULL;

		Array<T> * pArray = (Array<T> *)pThis;
		if ( (int)pArray->size() < (n + 1) )
			pArray->realloc( n + 1 );		// grow the vector if needed

		return &((*pArray)[ n ]);
	}
	virtual void allocate( void * pThis, int nSize )
	{
		Array<T> * pArray = (Array<T> *)pThis;
		pArray->realloc( nSize );
	}

	virtual int add( void * pThis )
	{
		Array<T> * pArray = (Array<T> *)pThis;
		pArray->push();
		return static_cast<int>( pArray->size() - 1 );
	}
	virtual bool remove( void * pThis, int n )
	{
		Array<T> * pArray = (Array<T> *)pThis;
		if ( n < 0 || n >= pArray->size() )
			return false;
		pArray->remove( n );
		return true;
	}
	virtual void clear( void * pThis )
	{
		Array<T> * pArray = (Array<T> *)pThis;
		pArray->release();
	}
};

//---------------------------------------------------------------------------------------------------

// std::vector container
template<typename T>
class VectorContainer : public Property::Container
{
public:
	// Property::Container interface
	virtual bool isDynamic() const
	{
		return true;
	}
	virtual int count( void * pThis ) const
	{
		std::vector<T> * pVector = (std::vector<T> *)pThis;
		return pVector->size();
	}
	virtual void * pointer( void * pThis, int n ) const
	{
		if ( n < 0 )
			return NULL;

		std::vector<T> * pVector = (std::vector<T> *)pThis;
		if ( (int)pVector->size() < (n + 1) )
			pVector->resize( n + 1 );		// grow the vector if needed

		return &((*pVector)[ n ]);
	}
	virtual void allocate( void * pThis, int nSize )
	{
		std::vector<T> * pVector = (std::vector<T> *)pThis;
		pVector->resize( nSize );
	}

	virtual int add( void * pThis )
	{
		std::vector<T> * pVector = (std::vector<T> *)pThis;
		pVector->push_back( T() );
		return static_cast<int>( pVector->size() - 1 );
	}
	virtual bool remove( void * pThis, int n )
	{
		std::vector<T> * pVector = (std::vector<T> *)pThis;
		if ( n < 0 || n >= (int)pVector->size() )
			return false;
		pVector->erase( pVector->begin() + n );
		return true;
	}
	virtual void clear( void * pThis )
	{
		std::vector<T> * pVector = (std::vector<T> *)pThis;
		pVector->clear();
	}
};

//---------------------------------------------------------------------------------------------------

// std::list container
template<typename T>
class ListContainer : public Property::Container
{
public:
	// Property::Container interface
	virtual bool isDynamic() const
	{
		return true;
	}
	virtual int count( void * pThis ) const
	{
		std::list<T> * pList = (std::list<T> *)pThis;
		return pList->size();
	}
	virtual void * pointer( void * pThis, int n ) const
	{
		if ( n < 0 )
			return NULL;

		std::list<T> * pList = (std::list<T> *)pThis;
		while( (int)pList->size() < (n + 1) )
			pList->push_back( T() );
		
		if ( n == (pList->size() - 1) )
			return &pList->back();

		typename std::list<T>::iterator i = pList->begin();
		for( ; n > 0; --n, ++i );
		return &(*i);
	}
	virtual void allocate( void * pThis, int nSize )
	{}

	virtual int add( void * pThis )
	{
		std::list<T> * pList = (std::list<T> *)pThis;
		pList->push_back( T() );
		return static_cast<int>( pList->size() - 1 );
	}
	virtual bool remove( void * pThis, int n )
	{
		std::list<T> * pList = (std::list<T> *)pThis;
		if ( n < 0 || n >= (int)pList->size() )
			return false;

		typename std::list<T>::iterator i = pList->begin();
		for( ; n > 0; ++i, --n );
		pList->erase( i );

		return true;
	}
	virtual void clear( void * pThis )
	{
		std::list<T> * pList = (std::list<T> *)pThis;
		pList->clear();
	}
};

//---------------------------------------------------------------------------------------------------


#endif

//---------------------------------------------------------------------------------------------------
//EOF

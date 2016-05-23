/*
	Reference.h
	This Reference class is used as a smart pointer to an object derived from Referenced...

	(c)2006 Palestar, Richard Lyle
*/

#ifndef REFERENCE_H
#define REFERENCE_H

#include "Referenced.h"

#pragma warning(disable:4311)		// pointer truncation from 'const Reference<T> *const ' to 'dword'

//---------------------------------------------------------------------------------------------------

template< typename T > 
class Reference		// pointer to a class with reference counting
{
public:
    // Construction
	Reference();
	Reference( T * pPointer );
	Reference( const Reference<T> &assign );
	~Reference();

    // Accessors
	T *				pointer() const;
	bool			valid() const;

	T *             operator->() const;
	T &				operator*() const;

	operator		T *() const;

    // Mutators
	void			setPointer( T * pPointer );
    Reference&		operator=( T * pPointer );
    Reference&		operator=( const Reference& ptr );

private:
    // Data
    T *				m_pPointer;
};

//-------------------------------------------------------------------------------

template<typename T>
inline Reference<T>::Reference() : m_pPointer( NULL ) 
{}

template<typename T>
inline Reference<T>::Reference( T * pPointer ) : m_pPointer( NULL )
{
	(*this) = pPointer;
}

template<typename T>
inline Reference<T>::Reference( const Reference<T> &assign ) : m_pPointer( NULL )
{
	(*this) = assign.m_pPointer;
}

template<typename T>
inline Reference<T>::~Reference()
{
	(*this) = NULL;
}

template<typename T>
inline T * Reference<T>::pointer() const
{ 
	return( m_pPointer ); 
}

template<typename T>
inline bool Reference<T>::valid() const									
{ 
	return( pointer() != NULL ); 
}

template<typename T>
inline T * Reference<T>::operator->() const								
{ 
	T * p = pointer();
	assert( p != NULL );
	return( p ); 
}

template<typename T>
inline T & Reference<T>::operator*() const								
{ 
	T * p = pointer();
	assert( p != NULL );
	return( *p ); 
}

template<typename T>
inline Reference<T>::operator T *() const										
{ 
	return( pointer() ); 
}

//---------------------------------------------------------------------------------------------------

template<typename T>
inline void	Reference<T>::setPointer( T * pPointer )
{
	if ( pPointer != m_pPointer )
	{
		if ( pPointer == NULL || pPointer->grabReference( reinterpret_cast<dword>( this ) ) )
		{
			if( m_pPointer != NULL )
				m_pPointer->releaseReference( reinterpret_cast<dword>( this ) );
			m_pPointer = pPointer;
		}
	}
}

template<typename T>
inline Reference<T> & Reference<T>::operator=( T * pPointer )
{
	setPointer( pPointer );
    return(*this);
}

template<typename T>
inline Reference<T> & Reference<T>::operator=( const Reference& ptr )
{
	setPointer( ptr.m_pPointer  );
    return(*this);
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF

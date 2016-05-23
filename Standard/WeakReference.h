/*
	WeakReference.h

	This class is used as a smart pointer to an object derived from Referenced.

	This is a WeakReference<>, which means it does not increment / decrement the 
	reference count of the object. Thus if the object is deleted, this pointer
	is set to NULL automatically.

	(c)2006 Palestar, Richard Lyle
*/

#ifndef WEAKREFERENCE_H
#define WEAKREFERENCE_H

#include "Referenced.h"
#include "Atomic.h"

//---------------------------------------------------------------------------------------------------

template< typename T > 
class WeakReference		// pointer to a class with reference counting
{
public:
    // Construction
	WeakReference();
	WeakReference( T * pPointer );
	WeakReference( const WeakReference<T> &assign );
	~WeakReference();

    // Accessors
	T *				pointer() const;
	bool			valid() const;

	T *             operator->() const;
	T &				operator*() const;

	operator		T *() const;

	bool			operator==( const WeakReference<T> &compare ) const;
	bool			operator==( const T * compare ) const;

    // Mutators
	void			setPointer( T * pPointer );
    WeakReference&	operator=( T * pPointer );
    WeakReference&	operator=( const WeakReference& ptr );

protected:
	// Types
	typedef Referenced::WeakReferenced		WeakRef;
    // Data
	WeakRef *		m_pWeakReferenced;
};

//-------------------------------------------------------------------------------

template<typename T>
inline WeakReference<T>::WeakReference() : m_pWeakReferenced( NULL ) 
{}

template<typename T>
inline WeakReference<T>::WeakReference( T * pPointer ) : m_pWeakReferenced( NULL )
{
	(*this) = pPointer;
}

template<typename T>
inline WeakReference<T>::WeakReference( const WeakReference<T> &assign ) : m_pWeakReferenced( NULL )
{
	(*this) = assign.pointer();
}

template<typename T>
inline WeakReference<T>::~WeakReference()
{
	(*this) = NULL;
}

template<typename T>
inline T * WeakReference<T>::pointer() const
{ 
	return m_pWeakReferenced != NULL ? (T *)m_pWeakReferenced->m_pReferenced : NULL;
}

template<typename T>
inline bool WeakReference<T>::valid() const									
{ 
	return pointer() != NULL; 
}

template<typename T>
inline T * WeakReference<T>::operator->() const								
{ 
	T * p = pointer();
	assert( p != NULL );
	return p; 
}

template<typename T>
inline T & WeakReference<T>::operator*() const								
{ 
	T * p = pointer();
	assert( p != NULL );
	return *p; 
}

template<typename T>
inline WeakReference<T>::operator T *() const										
{ 
	return pointer(); 
}

template<typename T>
inline bool WeakReference<T>::operator==( const WeakReference<T> & compare ) const	
{ 
	return pointer() == compare.pointer(); 
}

template<typename T>
inline bool WeakReference<T>::operator==( const T * compare ) const			
{ 
	return pointer() == compare; 
}

template<typename T>
inline void	WeakReference<T>::setPointer( T * pPointer )
{
	WeakRef * pWeakReferenced = pPointer != NULL ? ((Referenced *)pPointer)->grabWeakReferenced() : NULL;
	if ( pWeakReferenced != m_pWeakReferenced )
	{
		if( m_pWeakReferenced != NULL && m_pWeakReferenced->decrement() )
			delete m_pWeakReferenced;

		m_pWeakReferenced = pWeakReferenced;
	}
	else if ( pWeakReferenced != NULL )
		pWeakReferenced->decrement();		// grabWeakReference() incremented, but since it's not different we need to decrement
}

template<typename T>
inline WeakReference<T> & WeakReference<T>::operator=( T * pPointer )
{
	setPointer( pPointer );
    return(*this);
}

template<typename T>
inline WeakReference<T> & WeakReference<T>::operator=( const WeakReference& ptr )
{
	setPointer( ptr.pointer()  );
    return(*this);
}


#endif

//---------------------------------------------------------------------------------------------------
//EOF

/*
	WidgetReference.h

	This smart pointer template class is used to point to a widget derived object. 

	* When this object is serialized only the key is written
	* This Smart pointer should only be used on classes derived from Widget ( see Factory/FactoryTypes.h )
	* This object will call Widget::findWidget() to try to locate the Widget instance when used.
	* If a widget instance is assigned the same key as another widget, this pointer automatically updates
	to point at the newer instance.
	* This will NOT reference the object keeping it memory same as the WeakReference<> class.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef WIDGETREFERENCE_H
#define WIDGETREFERENCE_H

#include "Standard/Atomic.h"
#include "Widget.h"
#include "WidgetKey.h"

//---------------------------------------------------------------------------------------------------

template<class T>
class WidgetReference
{
public:
    // Construction
	WidgetReference();
	WidgetReference( T * pPointer );
	WidgetReference( const WidgetReference<T> & assign );
	WidgetReference( const WidgetKey & assign );
	~WidgetReference();

    // Accessors
	T *					pointer() const;
	const WidgetKey &	key() const;
	bool				valid() const;

	T *					operator->() const;
	T &					operator*() const;

	operator			Reference<T>() const;		// returns a thread-safe reference to the object 
	operator			T *() const;

    // Mutators
	void				setPointer( T * pPointer );
	void				setKey( const WidgetKey & key );
    WidgetReference&	operator=( T * pPointer );
    WidgetReference&	operator=( const WidgetReference& ptr );
    WidgetReference&	operator=( const WidgetKey & nKey );

private:
    // Data
	Widget::WidgetHandle::Ref
						m_pWidgetHandle;
	u32					m_nPadding;		// make this 16 bytes so replication doesn't keep trying to replicate this class.
	WidgetKey			m_Key;		
};

//---------------------------------------------------------------------------------------------------

template<typename T>
inline WidgetReference<T>::WidgetReference() 
	: m_Key( NULL_WIDGET ), m_nPadding( 0 )
{}

template<typename T>
inline WidgetReference<T>::WidgetReference( T * pPointer ) 
	: m_Key( NULL_WIDGET ), m_nPadding( 0 )
{
	setPointer( pPointer );
}

template<typename T>
inline WidgetReference<T>::WidgetReference( const WidgetReference<T> & assign ) 
	: m_pWidgetHandle( assign.m_pWidgetHandle ), m_Key( assign.m_Key ), m_nPadding( 0 )
{}

template<typename T>
inline WidgetReference<T>::WidgetReference( const WidgetKey & assign ) 
	: m_Key( assign ), m_nPadding( 0 )
{}

template<typename T>
inline WidgetReference<T>::~WidgetReference()
{
	setPointer( NULL );
}

//---------------------------------------------------------------------------------------------------

template<typename T>
inline T * WidgetReference<T>::pointer() const
{ 
	// if the pointer is NULL try to find the widget object...
	if ( m_pWidgetHandle == NULL && m_Key != NULL_WIDGET )
	{
		Widget::Ref pWidget = Widget::findWidgetByType( m_Key, T::classKey() );
		if ( pWidget.valid() )
			const_cast<WidgetReference<T> *>( this )->setPointer( (T *)pWidget.pointer() );
	}

	return m_pWidgetHandle != NULL ? static_cast<T *>( m_pWidgetHandle->m_pWidget.pointer() ) : NULL;
}

template<class T>
inline const WidgetKey & WidgetReference<T>::key() const
{
	return m_Key;
}

template<typename T>
inline bool WidgetReference<T>::valid() const									
{ 
	return pointer() != NULL; 
}

template<typename T>
inline T * WidgetReference<T>::operator->() const								
{ 
	T * p = pointer();
	assert( p != NULL );
	return( p ); 
}

template<typename T>
inline T & WidgetReference<T>::operator*() const								
{ 
	T * p = pointer();
	assert( p != NULL );
	return( *p ); 
}

template<typename T>
inline WidgetReference<T>::operator T *() const										
{ 
	return( pointer() ); 
}

template<typename T>
inline WidgetReference<T>::operator Reference<T>() const
{
	// if the pointer is NULL try to find the widget object...
	if ( m_pWidgetHandle == NULL && m_Key != NULL_WIDGET )
	{
		Widget::Ref pWidget = Widget::findWidgetByType( m_Key, T::classKey() );
		if ( pWidget.valid() )
			const_cast<WidgetReference<T> *>( this )->setPointer( (T *)pWidget.pointer() );
	}

	AutoLock lock( &Widget::widgetLock()  );
	return m_pWidgetHandle != NULL ? Reference<T>( static_cast<T *>( m_pWidgetHandle->m_pWidget.pointer() ) ) : Reference<T>();
}

template<typename T>
inline void	WidgetReference<T>::setPointer( T * pPointer )
{
	Widget::WidgetHandle::Ref pHandle = pPointer != NULL ? pPointer->grabWidgetHandle() : NULL;
	if ( pHandle != m_pWidgetHandle )
	{
		m_pWidgetHandle = pHandle;
		m_Key = pPointer != NULL ? pPointer->key() : NULL_WIDGET;
	}
	else if ( pPointer == NULL && m_Key != NULL_WIDGET )
		m_Key = NULL_WIDGET;		// handle the case when we are setting a WidgetReference to NULL that hasn't been referenced yet..
}

template<class T>
inline void WidgetReference<T>::setKey( const WidgetKey & nKey )
{
	if ( m_Key != nKey )
	{
		setPointer( NULL );
		m_Key = nKey;
	}
}

template<typename T>
inline WidgetReference<T> & WidgetReference<T>::operator=( T * pPointer )
{
	setPointer( pPointer );
    return(*this);
}

template<typename T>
inline WidgetReference<T> & WidgetReference<T>::operator=( const WidgetReference & ptr )
{
	setPointer( NULL );

	m_Key = ptr.m_Key;
	m_pWidgetHandle = ptr.m_pWidgetHandle;

    return(*this);
}

template<typename T>
inline WidgetReference<T> & WidgetReference<T>::operator=( const WidgetKey & nKey )
{
	setKey( nKey );
    return(*this);
}

//---------------------------------------------------------------------------------------------------

template<class T>
inline const OutStream & operator<<( const OutStream &output, const WidgetReference<T> & ref )
{
	output << ref.key();
	return output;
}

template<class T>
inline const InStream & operator>>( const InStream &input, WidgetReference<T> & ref )
{
	WidgetKey key( (qword)0 );
	input >> key;

	ref.setKey( key );
	return input;
}

#endif

//---------------------------------------------------------------------------------------------------
//EOF

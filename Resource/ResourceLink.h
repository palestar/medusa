/*
	ResourceLink.h

	This smart pointer template class is used to point to a Resource derived object. 

	* When this object is serialized only the key is written
	* This Smart pointer should only be used on classes derived from Resource ( see Resource/Resource.h )
	* This object will call Widget::findWidget() to try to locate the Resource instance when used. If not
	found then it will call Broker::findBroker() to locate the broker for the object.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef RESOURCE_LINK_H
#define RESOURCE_LINK_H

#include "Debug/Trace.h"
#include "Factory/Broker.h"
#include "Factory/Widget.h"
#include "Standard/StringHash64.h"

#include <set>

#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class ResourceLinkBase;

// this class allows a user to attach to a ResourceLink and receive a notification when the resource has been loaded..
class DLL ResourceObserver		
{
public:
	//! Construction
	ResourceObserver();
	ResourceObserver( ResourceLinkBase * a_pWatch );
	virtual ~ResourceObserver();

	//! Interface
	virtual void	onLoaded( Widget * a_pWidget, ResourceLinkBase * a_pLink ) = 0;
	//! Mutators
	void			attach( ResourceLinkBase * a_pWatch );
	void			detach( ResourceLinkBase * a_pWatch );
	void			detachAll();

protected:
	//! Types
	typedef std::set< ResourceLinkBase * >		WatchSet;
	//! Data
	WatchSet		m_Watching;
};

//---------------------------------------------------------------------------------------------------

class DLL ResourceLinkBase : public Broker::Request
{
public:
	//! Construction
	ResourceLinkBase();
	virtual ~ResourceLinkBase();

	// Broker::Request interface
	virtual bool		onLoaded( Widget * a_pWidget );

protected:
	//! Types
	typedef std::set< ResourceObserver * >		ObserverSet;
	//! Data
	ObserverSet			m_Observers;

	friend class ResourceObserver;
};

//---------------------------------------------------------------------------------------------------

template<typename T>
class ResourceLink : public ResourceLinkBase
{
public:
    // Construction
	ResourceLink();
	ResourceLink( const WidgetKey & assign );
	ResourceLink( const ResourceLink<T> & pLink );
	ResourceLink( T * pPointer );
	~ResourceLink();

	// Broker::Request interface
	bool				onLoaded( Widget * pWidget );

	// Helpers for converting from a child class type..
	template<typename K>
	ResourceLink( const ResourceLink<K> & pLink );
	template<typename K>
	ResourceLink( K * pPointer );

    // Accessors
	T *					pointer( bool bAllowLoad = true, bool bBlocking = true ) const;
	bool				valid( bool bAllowLoad = true, bool bBlocking = true ) const;
	bool				isLoaded() const;
	const WidgetKey &	key() const;

	T *					operator->() const;
	T &					operator*() const;

	operator			T *() const;

	bool				operator==( const ResourceLink<T> &compare ) const;
	bool				operator==( const T * compare ) const;

    // Mutators
	void				setPointer( T * pPointer );
	void				setKey( const WidgetKey & key );

	void				load( const char * a_pResourceName, bool a_bBlocking = false );
	void				load( const WidgetKey & a_nKey, bool a_bBlocking = false );
	void				load( bool bBlocking = true );
	void				unload();
	void				reset();			// reset this object, releasing any object pointed at...

    ResourceLink &		operator=( T * pPointer );
    ResourceLink &		operator=( const ResourceLink<T> & pLink );
	ResourceLink &		operator=( const WidgetKey & nKey );

protected:
    // Data
    T *					m_pPointer;
	WidgetKey			m_Key;
};

//---------------------------------------------------------------------------------------------------

template<typename T>
inline ResourceLink<T>::ResourceLink() : m_pPointer( NULL ), m_Key( NULL_WIDGET )
{}

template<typename T>
inline ResourceLink<T>::ResourceLink( const WidgetKey & nKey ) : m_pPointer( NULL ), m_Key( NULL_WIDGET )
{
	operator=( nKey );
}

template<typename T>
inline ResourceLink<T>::ResourceLink( const ResourceLink<T> & pLink ) : m_pPointer( NULL ), m_Key( NULL_WIDGET )
{
	operator=( pLink );
}

template<typename T>
inline ResourceLink<T>::ResourceLink( T * pPointer )  : m_pPointer( NULL ), m_Key( NULL_WIDGET )
{
	operator=( pPointer );
}

template<typename T>
inline ResourceLink<T>::~ResourceLink()
{
	if( m_pPointer )
		m_pPointer->releaseReference( (dword)this );
}

template<typename T>
template<typename K>
inline ResourceLink<T>::ResourceLink( const ResourceLink<K> & pLink ) : m_pPointer( NULL ), m_Key( NULL_WIDGET )
{
	if ( pLink.pointer( false ) )
		setPointer( WidgetCast<T>( pLink.pointer( false ) ) );
	else
		setKey( pLink.key() );
}

template<typename T>
template<typename K>
inline ResourceLink<T>::ResourceLink( K * pPointer ) : m_pPointer( NULL ), m_Key( NULL_WIDGET )
{
	setPointer( WidgetCast<T>( pPointer ) );
}

//---------------------------------------------------------------------------------------------------

template<typename T>
bool ResourceLink<T>::onLoaded( Widget * pWidget )
{
	setPointer( WidgetCast<T>( pWidget ) );
	return ResourceLinkBase::onLoaded( pWidget );		// notify any observers..
}

//---------------------------------------------------------------------------------------------------

template<typename T>
inline T * ResourceLink<T>::pointer( bool bAllowLoad /*= true*/, bool bBlocking /*= true*/ ) const
{ 
	if ( m_pPointer == NULL && bAllowLoad )
	{
		if( isAttached() && !bBlocking )
			return NULL;		// we are already attached into the load request hash, return NULL until onLoaded() is invoked.
		const_cast< ResourceLink<T> *>( this )->load( bBlocking );
	}

	return m_pPointer;
}

template<typename T>
inline bool ResourceLink<T>::valid( bool bAllowLoad /*= true*/, bool bBlocking /*= true*/ ) const									
{ 
	return pointer( bAllowLoad, bBlocking ) != NULL; 
}

template<typename T>
inline bool ResourceLink<T>::isLoaded() const
{
	return m_pPointer != NULL;
}

template<typename T>
inline const WidgetKey & ResourceLink<T>::key() const
{
	return m_Key;
}

template<typename T>
inline T * ResourceLink<T>::operator->() const								
{ 
	T * p = pointer();
	assert( p != NULL );
	return( p ); 
}

template<typename T>
inline T & ResourceLink<T>::operator*() const								
{ 
	T * p = pointer();
	assert( p != NULL );
	return( *p ); 
}

template<typename T>
inline ResourceLink<T>::operator T *() const										
{ 
	return( pointer() ); 
}

template<typename T>
inline bool ResourceLink<T>::operator==( const ResourceLink<T> &compare ) const	
{ 
	return( pointer() == compare.pointer() ); 
}

template<typename T>
inline bool ResourceLink<T>::operator==( const T * compare ) const			
{ 
	return( pointer() == compare ); 
}

//---------------------------------------------------------------------------------------------------

template<typename T>
inline void	ResourceLink<T>::setPointer( T * pPointer )
{
	if ( pPointer != m_pPointer )
	{
		if ( pPointer == NULL || pPointer->grabReference( reinterpret_cast<dword>( this ) ) )
		{
			if( m_pPointer  != NULL )
				m_pPointer->releaseReference( reinterpret_cast<dword>( this ) );

			m_pPointer = pPointer;
			m_Key = m_pPointer != NULL ? m_pPointer->key() : NULL_WIDGET;
		}
	}
}

template<typename T>
inline void ResourceLink<T>::setKey( const WidgetKey & key )
{
	setPointer( NULL );
	m_Key = key;
}

template<typename T>
inline void ResourceLink<T>::load( const char * a_pResourceName, bool a_bBlocking /*= false*/ )
{
	setKey( StringHash64( a_pResourceName ) );
	load( a_bBlocking );
}

template<typename T>
inline void	 ResourceLink<T>::load( const WidgetKey & a_nKey, bool a_bBlocking /*= false*/ )
{
	setKey( a_nKey );
	load( a_bBlocking );
}

template<typename T>
inline void ResourceLink<T>::load( bool bBlocking /*= true*/ )
{
	if ( !m_pPointer && m_Key != NULL_WIDGET )
		Broker::requestLoad( m_Key, T::classKey(), this, bBlocking );
}

template<typename T>
inline void ResourceLink<T>::unload()
{
	setPointer( NULL );
}

template<typename T>
inline void ResourceLink<T>::reset()
{
	setKey( NULL_WIDGET );
}

template<typename T>
inline ResourceLink<T> & ResourceLink<T>::operator=( T * pPointer )
{
	setPointer( pPointer );
	return *this;
}

template<typename T>
inline ResourceLink<T> & ResourceLink<T>::operator=( const ResourceLink<T> & pLink )
{
	if ( pLink.pointer( false ) )
		setPointer( WidgetCast<T>( pLink.pointer( false ) ) );
	else
		setKey( pLink.key() );
	return *this;
}

template<typename T>
inline ResourceLink<T> & ResourceLink<T>::operator=( const WidgetKey & nKey )
{
	setKey( nKey );
	return *this;
}

//---------------------------------------------------------------------------------------------------

template<typename T>
inline const OutStream & operator<<( const OutStream &output, const ResourceLink<T> & ref )
{
	output << ref.key();
	return output;
}

template<typename T>
inline const InStream & operator>>( const InStream &input, ResourceLink<T> & ref )
{
	WidgetKey key( (qword)0 );
	input >> key;

	ref.setKey( key );
	return input;
}

//----------------------------------------------------------------------------

template<typename T>
inline bool loadResource( const char * pKey, ResourceLink<T> & resource )
{
	if ( strncmp( pKey, "0x", 2 ) == 0 )
		resource = WidgetKey( pKey );
	else
		resource = WidgetKey( StringHash64( pKey ) );

	return resource.valid();
}

template<typename T>
inline bool loadResource( const WidgetKey &key, ResourceLink<T> &resource )
{
	resource = key;
	return resource.valid();
}

#endif

//----------------------------------------------------------------------------
// EOF

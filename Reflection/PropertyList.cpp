/*
	PropertyList.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Reflection/PropertyList.h"
#include "Reflection/TypeCopy.h"
#include "Standard/StringHash.h"

//----------------------------------------------------------------------------

PropertyList::PropertyList( Type * pType, Type * pPointerType, PropertyList * pBaseClass ) 
	: m_bInitialized( false ), m_pName(""), m_pType( pType ), m_pPointerType( pPointerType ), m_pBaseClass( pBaseClass )
{
	if ( m_pType != NULL )
	{
		m_pType->setPropertyList( this );
		m_pName = m_pType->name();
	}
	if ( m_pPointerType != NULL )
		m_pPointerType->setPropertyList( this );
	if ( m_pBaseClass != NULL )
		m_pBaseClass->addChild( this );
}

PropertyList::~PropertyList()
{
	if ( m_pBaseClass != NULL )
		m_pBaseClass->removeChild( this );
	if ( m_pType != NULL )
		m_pType->setPropertyList( NULL );
	if ( m_pPointerType != NULL )
		m_pPointerType->setPropertyList( NULL );
}

//----------------------------------------------------------------------------

void PropertyList::initialize()
{
	if (! m_bInitialized )
	{
		if ( m_pBaseClass != NULL )
		{
			// copy over the properties from the base class...

			// NOTE: Previously, I just copied the pointers, but then that prevented certain classes from setting new flags for 
			// certain properties, this way we have a polymorphism of sorts..

			for(int i=0;i<(int)m_pBaseClass->m_Properties.size();i++)
                m_Properties.push( new Property( *m_pBaseClass->m_Properties[i] ) );
		}

		m_bInitialized = true;
	}
}

void PropertyList::release()
{
	if ( m_bInitialized )
	{
		m_Properties.release();
		m_bInitialized = false;
	}
}

void PropertyList::setClassName( const char * a_pName )
{
	m_pName = a_pName;
}

int PropertyList::addProperty( Property * pAdd )
{
	for(int i=0;i<m_Properties.size();i++)
		if ( strcmp( pAdd->name(), m_Properties[i]->name() ) == 0 )
		{
			// replace an existing property with the same name..
			m_Properties[i] = pAdd;
			return i;
		}
	pAdd->setPropertyId( (dword)m_Properties.size() );
	m_Properties.push( pAdd );
	return m_Properties.size() - 1;
}

void PropertyList::removeProperty( int n )
{
	if ( m_Properties.isValid( n ) )
		m_Properties.remove( n );
}

bool PropertyList::addOption( const CharString & sProp, const CharString & sName, const CharString & sValue )
{
	int n = find( sProp );
	if ( n < 0 )
		return false;

	m_Properties[n]->addOption( sName, sValue );
	return true;
}

bool PropertyList::setFlags( const char * pName, dword nFlags )
{
	int n = find( pName );
	if ( n < 0 )
		return false;

	m_Properties[n]->setFlags( nFlags );
	return true;
}

bool PropertyList::addFlags( const char * pName, dword nFlags )
{
	int n = find( pName );
	if ( n < 0 )
		return false;

	m_Properties[n]->addFlags( nFlags );
	return true;
}

bool PropertyList::clearFlags( const char * pName, dword nFlags )
{
	int n = find( pName );
	if ( n < 0 )
		return false;

	m_Properties[n]->clearFlags( nFlags );
	return true;
}

bool PropertyList::setNote( const char * pName, const char * pNote )
{
	int n = find( pName );
	if ( n < 0 )
		return false;

	m_Properties[n]->setNote( pNote );
	return true;
}

bool PropertyList::find( const Expression & e, void * pThis, PropertyList::Find & found )
{
	if ( pThis == NULL )
		return false;
	for(int i=0;i<(int)m_Properties.size();i++)
		if ( m_Properties[i]->find( e, pThis, found ) )
			return true;
	return false;
}

bool PropertyList::enumerate( void * pThis, Array< Find > & found )
{
	if ( pThis == NULL )
		return true;			// return success, otherwise it will stop enumerating...
	for(int i=0;i<(int)m_Properties.size();i++)
		if (! m_Properties[i]->enumerate( pThis, found ) )
			return false;
	return true;
}

//---------------------------------------------------------------------------------------------------

void PropertyList::addChild( PropertyList * pChild )
{
   m_Children.push( pChild );
}

void PropertyList::removeChild( PropertyList * pChild )
{
   m_Children.removeSearchSwap( pChild );
}

//---------------------------------------------------------------------------------------------------
//EOF

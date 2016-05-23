/*
	Collection.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RESOURCE_DLL
#include "Resource/Collection.h"
#include "Factory/FactoryTypes.h"

//-------------------------------------------------------------------------------

IMPLEMENT_RESOURCE_FACTORY( Collection, Resource );

BEGIN_PROPERTY_LIST( Collection, Resource );
	ADD_PROPERTY( m_Type );
	ADD_PROPERTY( m_Resources );
END_PROPERTY_LIST();

Collection::Collection() : m_Type( CLASS_KEY( Resource ) )
{}

//-------------------------------------------------------------------------------

void Collection::setType( const ClassKey & type )
{
	m_Type = type;
}

bool Collection::push( Resource::Link pResource )
{
	m_Resources.push( pResource );
	return true;
}

bool Collection::push( Collection * pCollection )
{
	for(int i=0;i<pCollection->resourceCount();i++)
		push( pCollection->resource( i ) );

	// collections are merged!
	return true;
}

void Collection::replace( int n, Resource::Link pResource )
{
	m_Resources[ n ] = pResource;
}

void Collection::remove( int resource )
{
	m_Resources.remove( resource );
}

void Collection::load() 
{
	for(int i=0;i<m_Resources.size();i++)
		m_Resources[i].load();
}

void Collection::unload()
{
	for(int i=0;i<m_Resources.size();i++)
		m_Resources[i].unload();
}

void Collection::release()
{
	m_Resources.release();
}

//-------------------------------------------------------------------------------
// EOF

/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define COLLECTIONPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "Resource.h"

#include "Resource\Collection.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( CollectionPort, Port );
REGISTER_FACTORY_KEY(  CollectionPort, 4221671980793000893 );

BEGIN_PROPERTY_LIST( CollectionPort, Port );
	ADD_CLASSKEY_PROPERTY( m_Type, Resource );
	ADD_PORT_PROPERTY( m_Collection, Port );
END_PROPERTY_LIST();

CollectionPort::CollectionPort() : Port()
{
	m_Type = CLASS_KEY( Resource );
}

//-------------------------------------------------------------------------------

const int VERSION_022301 = 22301;

bool CollectionPort::read( const InStream & input )
{
	if (! Port::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_022301:
			input >> m_Type;
			input >> m_Collection;
			break;
		default:
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------------

bool CollectionPort::autoLoad()
{
	return false;
}

void CollectionPort::dependencies( DependentArray & dep )
{
	for(int i=0;i<m_Collection.size();i++)
		dep.push( m_Collection[ i ] );
}

Resource::Ref CollectionPort::createResource()
{
	Collection * pCollection = new Collection;
	pCollection->setType( m_Type );

	for(int i=0;i<m_Collection.size();i++)
	{
		CharString sResource( m_Collection[ i ] );
		if ( sResource.length() == 0 )
			continue;		// ignore empty strings

		if (! pCollection->push( Port::portResource( sResource ) ) )
		{
			report( CharString().format("Failed to add '%s' to collection; failed to load resource!", sResource) );
			continue;
		}
	}

	return pCollection;
}

CFrameWnd * CollectionPort::createView()
{
	return Port::createView();
}

bool CollectionPort::importFile( const char * fileName )
{
	return true;	// return true of successful import, false on failure
}

void CollectionPort::shellOpen() const
{
	MessageBox( NULL, _T("shellOpen() is not valid for this port"), NULL, MB_OK);
}

//-------------------------------------------------------------------------------
// EOF


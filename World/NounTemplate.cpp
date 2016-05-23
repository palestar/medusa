/*
	NounTemplate.cpp

	This class provides the hook for the game to enumerate the available ship types
	(c)1999 PaleStar Development, Richard Lyle
*/


#include "Debug/Assert.h"
#include "NounTemplate.h"
#include "Noun.h"
#include "WorldContext.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NounTemplate, Resource );
REGISTER_FACTORY_KEY( NounTemplate, 4040652215298323130LL );

BEGIN_PROPERTY_LIST( NounTemplate, Resource )
	ADD_PROPERTY( m_nFlags );
	ADD_PROPERTY( m_nRank );
	ADD_PROPERTY( m_pScene );
END_PROPERTY_LIST();

NounTemplate::NounTemplate()
{
	m_nFlags = 0;
	m_nRank = 0;
}

//----------------------------------------------------------------------------

bool NounTemplate::read( const InStream & input )
{
	if (! Resource::read( input ) )
		return false;
	// force the Resource to be loaded
	m_pScene.load();
	return true;
}

//----------------------------------------------------------------------------

Noun * NounTemplate::noun() 
{
	if ( m_pScene.valid() && m_pScene->node() && m_pScene->node()->childCount() > 0 )
		return WidgetCast<Noun>( m_pScene->node()->child( 0 ) );

	return NULL;
}

Noun * NounTemplate::spawn()
{
	Noun * pTemplate = noun();
	if ( pTemplate != NULL )
	{
		// cloning automatically generates a new key
		Noun * pCopy = WidgetCast<Noun>( pTemplate->copy() );
		ASSERT( pCopy );

		return( pCopy );
	}

	return( NULL );
}

//----------------------------------------------------------------------------
// EOF

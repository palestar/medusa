/*
	NounZoneTarget.cpp
	(c)2004 Palestar Inc, Richard Lyle
*/

#include "NounZoneTarget.h"
#include "WorldContext.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NounZoneTarget, NounTarget );
REGISTER_FACTORY_KEY( NounZoneTarget, 4618981335918160308LL );

BEGIN_PROPERTY_LIST( NounZoneTarget, NounTarget )
END_PROPERTY_LIST();

NounZoneTarget::NounZoneTarget()
{}

NounZoneTarget::NounZoneTarget( NodeZone * pZone, const Vector3 & vPosition ) : NounTarget( vPosition )
{
	setZone( pZone );
	if ( pZone != NULL )
		setContext( pZone->context() );
}

NounZoneTarget::NounZoneTarget( const Vector3 & vWorldPosition, WorldContext * pContext ) : NounTarget( vWorldPosition )
{
	setPosition( vWorldPosition, pContext );
}

//----------------------------------------------------------------------------

bool NounZoneTarget::write( const OutStream & output )
{
	if (! NounTarget::write( output ) )
		return false;
	output << NodeZone::wRef( m_pZone );
	return true;
}

bool NounZoneTarget::read( const InStream & input )
{
	if (! NounTarget::read( input ) )
		return false;

	NodeZone::wRef pZone;
	input >> pZone;

	// set the context if the zone is valid
	if ( pZone.valid() )
	{
		setContext( pZone->context() );
		setZone( pZone );
	}
	return true;
}

//----------------------------------------------------------------------------

void NounZoneTarget::calculateWorld() const
{
	m_WorldPosition = position();
	if ( m_pZone != NULL )
		m_WorldPosition += m_pZone->position();
}

//----------------------------------------------------------------------------

void NounZoneTarget::setPosition( const Vector3 & v, WorldContext * pContext )
{
	setContext( pContext );

	NodeZone * pZone = pContext->findZone( v );
	if ( pZone != NULL )
	{
		m_pZone = pZone;
		m_Position = v - pZone->position();

		//TRACE( CharString().format("NounZoneTarget::setPosition, v = %s, zone pos = %s, pos = %s",
		//	(CharString)v, (CharString)pZone->position(), (CharString)m_Position ) );
	}
	else
	{
		// outside of any zone, NULL zone
		m_pZone = NULL;
		m_Position = v;
	}

	updateName();
	calculateWorld();
}

//----------------------------------------------------------------------------
//EOF

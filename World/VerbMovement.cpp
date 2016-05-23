/**
	@file VerbMovement.cpp

	(c)2006 Palestar Inc
	@author Richard Lyle @date 5/15/2007 10:30:14 PM
*/

#include "VerbMovement.h"
#include "WorldContext.h"
#include "Noun.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_FACTORY( VerbMovement, Verb );

BEGIN_PROPERTY_LIST( VerbMovement, Verb )
	ADD_TRANSMIT_PROPERTY( m_pZone );
	ADD_TRANSMIT_PROPERTY( m_vPosition );
	ADD_TRANSMIT_PROPERTY( m_vOrientation );
	ADD_TRANSMIT_PROPERTY( m_bGlobal );
END_PROPERTY_LIST();

VerbMovement::VerbMovement()
{}

VerbMovement::VerbMovement( Noun * pTarget, bool bGlobal ) 
	: Verb( pTarget->tick() ), 
	m_pZone( pTarget->zone() ), 
	m_vPosition( pTarget->position() ), 
	m_vOrientation( pTarget->orientation() ), 
	m_bGlobal( bGlobal )
{}

bool VerbMovement::useUDP() const
{
	return true;
}

Verb::Priority VerbMovement::priority() const
{
	return CRITICAL;
}

Verb::Scope VerbMovement::scope() const
{
	return m_bGlobal ? GLOBAL : LOCAL;
}

bool VerbMovement::client() const
{
	return true;
}

bool VerbMovement::queue() const
{
	return true;
}

bool VerbMovement::checkNoun() const
{
	return true;
}

void VerbMovement::onExecute()
{
	Noun * pTarget = target();
	if ( pTarget )
		pTarget->onMovement( tick(), m_pZone, m_vPosition, m_vOrientation, m_bGlobal );
}

//---------------------------------------------------------------------------------------------------
//EOF

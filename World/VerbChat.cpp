/*
	VerbChat.cpp
	(c)1999 Palestar, Richard Lyle
*/

#include "Debug/Assert.h"
#include "VerbChat.h"
#include "WorldContext.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( VerbChat, Verb );

BEGIN_PROPERTY_LIST( VerbChat, Verb )
	ADD_TRANSMIT_PROPERTY( m_Scope );
	ADD_TRANSMIT_PROPERTY( m_sChat );
END_PROPERTY_LIST();

VerbChat::VerbChat()
{}

VerbChat::VerbChat( Noun * pTarget, Scope scope, const char * chat ) 
	: m_Scope( scope ), m_sChat( chat )
{
	attachVerb( pTarget );
}

//----------------------------------------------------------------------------

Verb::Priority VerbChat::priority() const
{
	return LOW;
}

Verb::Scope VerbChat::scope() const
{
	return m_Scope;
}

bool VerbChat::client() const
{
	return true;
}

void VerbChat::onExecute()
{
	// send message to GameContext::User
	context()->user()->onChat( target(), m_sChat );
}

//----------------------------------------------------------------------------
// EOF

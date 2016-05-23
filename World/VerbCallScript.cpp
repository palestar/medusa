/*
	VerbCallScript.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/


#include "Debug/Assert.h"
#include "VerbCallScript.h"
#include "WorldContext.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( VerbCallScript, Verb );

BEGIN_PROPERTY_LIST( VerbCallScript, Verb )
	ADD_TRANSMIT_PROPERTY( m_Scope );
	ADD_TRANSMIT_PROPERTY( m_nScriptId );
	ADD_TRANSMIT_PROPERTY( m_Call );
	ADD_TRANSMIT_PROPERTY( m_CreateNewContext );
END_PROPERTY_LIST();

VerbCallScript::VerbCallScript()
{}

VerbCallScript::VerbCallScript( Noun * pTarget, Scope scope, UniqueNumber nScriptId, const char * pCall, bool bNewContext ) 
	: m_Scope( scope ), m_nScriptId( nScriptId ), m_Call( pCall ), m_CreateNewContext( bNewContext )
{
	attachVerb( pTarget );
}

//----------------------------------------------------------------------------

Verb::Priority VerbCallScript::priority() const
{
	return CRITICAL;
}

Verb::Scope VerbCallScript::scope() const
{
	return m_Scope;
}

bool VerbCallScript::client() const
{
	return true;
}

void VerbCallScript::onExecute()
{
	// call script function in GameContext
	context()->scriptCall( m_nScriptId, m_Call, m_CreateNewContext );
}

//----------------------------------------------------------------------------
//EOF

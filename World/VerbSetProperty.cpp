/**
	@file VerbSetProperty.cpp

	TODO: This class is a bit large for setting data in the classes, we could really compress the size of this 
	verb by using dictionaries for the property name rather than sending the text of the property over the network.

	Also, need to provide support for setting multiple properties in the same object if we ever want to use
	this class as a general solution for replication of objects over the network.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 11/25/2009 11:51:31 PM
*/

#include "Debug/Assert.h"
#include "World/Noun.h"
#include "VerbSetProperty.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_FACTORY( VerbSetProperty, Verb );

BEGIN_PROPERTY_LIST( VerbSetProperty, Verb )
	ADD_TRANSMIT_PROPERTY( m_nProperty );			
	ADD_TRANSMIT_PROPERTY( m_Value );
END_PROPERTY_LIST();

VerbSetProperty::VerbSetProperty() : m_nProperty( 0 ), m_bGlobal( false ), m_bUDP( false )
{}

VerbSetProperty::VerbSetProperty( Noun * pTarget, const char * a_pProperty, const Value & a_Value, 
								 bool a_bGlobal /*= true*/, bool a_bUDP /*= false*/ ) :
	m_nProperty( 0 ), m_Value( a_Value ), m_bGlobal( a_bGlobal ), m_bUDP( a_bUDP )
{
	int nProperty = pTarget->propertyList()->find( a_pProperty );
	ASSERT( nProperty >= 0 && nProperty <= 255 );

	m_nProperty = nProperty;
	attachVerb( pTarget );
}

VerbSetProperty::VerbSetProperty( Noun * pTarget, int a_nProperty, const Value & a_Value, 
								 bool a_bGlobal /*= true*/, bool a_bUDP /*= false*/ ) :
	m_nProperty( a_nProperty ), m_Value( a_Value ), m_bGlobal( a_bGlobal ), m_bUDP( a_bUDP )
{
	attachVerb( pTarget );
}

//----------------------------------------------------------------------------

bool VerbSetProperty::useUDP() const
{
	return m_bUDP;
}

Verb::Priority VerbSetProperty::priority() const
{
	return MEDIUM;
}

VerbSetProperty::Scope VerbSetProperty::scope() const
{
	return m_bGlobal ? GLOBAL : LOCAL;
}

bool VerbSetProperty::client() const
{
	return true;
}

bool VerbSetProperty::canAttach( Noun * pNoun )
{
	return pNoun != NULL;
}

void VerbSetProperty::onExecute()
{
	Noun * pTarget = target();
	if ( validate( pTarget ) )
	{
		Property * pProperty = pTarget->propertyList()->property( m_nProperty );
		if ( pProperty != NULL )
		{
			//TRACE( CharString().format( "VerbSetProperty() - '%s' of '%s'", pProperty->name(), pTarget->name() ) );
			if (! pTarget->set( pProperty, 0, m_Value ) )
				LOG_DEBUG_MED( "VerbSetProperty", "Failed to set property %s", pProperty->name() );
		}
	}
}

//---------------------------------------------------------------------------------------------------
//EOF

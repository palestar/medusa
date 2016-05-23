/*
	Verb.cpp

	This base class is used to represent a action towards a noun
	(c)1999 Palestar Development, Richard Lyle
*/


#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Verb.h"
#include "Noun.h"
#include "NounTarget.h"
#include "WorldContext.h"

//-------------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_NOKEY_FACTORY( Verb, Widget );

BEGIN_ABSTRACT_PROPERTY_LIST( Verb, Widget );
END_PROPERTY_LIST();

Verb::Verb( dword nTick /*= 0*/ ) : m_nTick( nTick ), m_pTarget( NULL ), m_nClientId( 0 ), m_nUserId( 0 )
{}

//----------------------------------------------------------------------------

bool Verb::read( const InStream & input )
{
	if (! Widget::read( input ) )
		return false;
	if ( queue() )
		input >> m_nTick;
	return true;
}

bool Verb::write( const OutStream & output )
{
	if (! Widget::write( output ) )
		return false;
	if ( queue() )
		output << m_nTick;
	return true;
}

//----------------------------------------------------------------------------

bool Verb::useUDP() const
{
	return false;
}

bool Verb::checkNoun() const
{
	return false;
}

Verb::Priority Verb::priority() const
{
	return CRITICAL;
}

Verb::Scope Verb::scope() const
{
	return LOCAL;
}

bool Verb::client() const
{
	return false;
}

bool Verb::queue() const
{
	return false;
}

//----------------------------------------------------------------------------

bool Verb::canAttach( Noun * pNoun )
{
	if (! pNoun )
		return false;
	return true;
}

//----------------------------------------------------------------------------

Verb::Context * Verb::context() const
{
	return m_pTarget != NULL ? m_pTarget->context() : NULL;
}

bool Verb::validate( Noun * pNoun ) const
{
	if ( pNoun == NULL )
		return false;		// pointer invalid
	if ( WidgetCast<NounTarget>( pNoun ) != NULL )
		return true;		// target objects are always valid!
	if ( pNoun->zone() == NULL )
		return false;		// noun no longer attached to a zone
	if ( pNoun->detach() )
		return false;		// noun is flagged for detachment..

	return true;
}

bool Verb::attachVerb( Noun * pNoun)
{
	if ( pNoun == NULL )
		return false;

	return pNoun->attachVerb( this );
}

int Verb::lag() const
{
	if ( m_pTarget != NULL )
		return m_pTarget->tick() - m_nTick;

	return 0;
}

//----------------------------------------------------------------------------
// EOF

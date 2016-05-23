/*
	Trait.cpp
	(c)2004 Palestar Inc, Richard Lyle
*/


#include "Trait.h"
#include "Noun.h"
#include "WorldContext.h"

//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_NOKEY_FACTORY( Trait, Widget ); 
REGISTER_FACTORY_KEY( Trait, 4780777378146859413LL );

BEGIN_ABSTRACT_PROPERTY_LIST( Trait, Widget );
END_PROPERTY_LIST();

Trait::Trait() : m_pParent( NULL ), m_bDetach( false )
{}

Trait::~Trait()
{}

//----------------------------------------------------------------------------

void Trait::initialize()
{}

void Trait::release()
{}

void Trait::simulate( dword nTick )
{}

void Trait::onTransferZones( NodeZone * pNewzone, NodeZone * pOldZone )
{}

void Trait::setParent( Noun * pNoun )
{
	m_pParent = pNoun;
}

void Trait::setDetach( bool a_bDetach /*= true*/ )
{
	m_bDetach = a_bDetach;
}

//----------------------------------------------------------------------------
//EOF

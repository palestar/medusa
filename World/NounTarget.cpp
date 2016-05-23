/*
	NounTarget.cpp

	Target object
	(c)1999 PaleStar Development, Richard Lyle
*/

#include "Constants.h"
#include "NounTarget.h"
#include "WorldContext.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NounTarget, Noun );
REGISTER_FACTORY_KEY( NounTarget, 4042557618313215082LL );

BEGIN_PROPERTY_LIST( NounTarget, Noun )
END_PROPERTY_LIST();

NounTarget::NounTarget()
{}

NounTarget::NounTarget( const Vector3 & position )
{
	setPosition( position );
	updateName();
}

//----------------------------------------------------------------------------

bool NounTarget::read( const InStream & input )
{
	if (! Noun::read( input ) )
		return false;
	updateName();
	return true;
}

//----------------------------------------------------------------------------

void NounTarget::updateName()
{
	Vector3 P( position() );
	m_Name = CharString().format("%.1f.%.1f", P.x / UNIVERSE_SECTOR, P.z / UNIVERSE_SECTOR );
}

//----------------------------------------------------------------------------
// EOF


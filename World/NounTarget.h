/*
	NounTarget.h

	Target object
	(c)1999 PaleStar Development, Richard Lyle
*/

#ifndef NOUN_TARGET_H
#define NOUN_TARGET_H

#include "Noun.h"
#include "WorldDll.h"

//----------------------------------------------------------------------------

class DLL NounTarget : public Noun
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< NounTarget >		Ref;

	// Construction
	NounTarget();
	NounTarget( const Vector3 & position );

	// Widget interface
	bool				read( const InStream & );
	// Mutators
	void				updateName();
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF


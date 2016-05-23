/*
	$$root$$.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "$$root$$.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( $$root$$, WindowView::View );

BEGIN_PROPERTY_LIST( $$root$$, WindowView::View );
END_PROPERTY_LIST();

$$root$$::$$root$$()
{
	// Construct your view class
}

//----------------------------------------------------------------------------

void $$root$$::onActivate()
{
	//{{BEGIN_DATA_INIT
	//END_DATA_INIT}}

	// called when this view is created
}

void $$root$$::onDeactivate()
{
	// called before this view is destroyed
}

void $$root$$::onUpdate( float t )
{
	// implement view updating
}

bool $$root$$::onMessage( const Message & msg )
{
	//{{BEGIN_MSG_MAP
	//END_MSG_MAP}}

	return false;
}

void $$root$$::onDocumentUpdate()
{
	// document data has changed, update this view if needed
}

//----------------------------------------------------------------------------



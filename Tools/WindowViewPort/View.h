/*
	$$root$$.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef $$ROOT$$_H
#define $$ROOT$$_H

#include "System/Messages.h"
#include "Gui3d/WindowView.h"

//----------------------------------------------------------------------------

class $$root$$ : public WindowView::View
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< $$root$$ >	Ref;

	// Construction
	$$root$$();

	// View interface
	void				onActivate();
	void				onDeactivate();
	void				onUpdate( float t );
	bool				onMessage( const Message & msg );
	void				onDocumentUpdate();

	// {{BEGIN_DATA
	// END_DATA}}

	// {{BEGIN_MSG
	// END_MSG}}
};

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF

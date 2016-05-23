/*
	Text.h

	These classes are used to store / access the text for an application
	(c)2005 Palestar, Richard Lyle
*/

#ifndef TEXT_H
#define TEXT_H

#include "Resource.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Text : public Resource		
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef ResourceLink<Text>			Link;
	typedef Reference<Text>				Ref;

	// Construction
	Text();
	Text( const char * pText );

	// Accessors
	operator		const char *() const;
	const char *	text() const;
	// Mutators
	void			setText( const char * pText );

private:
	// Data
	CharString		m_Text;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

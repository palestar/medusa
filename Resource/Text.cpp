/*
	Text.cpp

	These classes are used to store / access the text for an application
	(c)2005 Palestar, Richard Lyle
*/

#define RESOURCE_DLL
#include "Resource/Text.h"

//----------------------------------------------------------------------------

IMPLEMENT_RESOURCE_FACTORY( Text, Resource );

BEGIN_PROPERTY_LIST( Text, Resource )
	ADD_PROPERTY( m_Text );
END_PROPERTY_LIST();

Text::Text()
{}

Text::Text( const char * pText )
{
	m_Text = pText;
}

//----------------------------------------------------------------------------

Text::operator const char *() const
{
	return text();
}

const char * Text::text() const
{
	return m_Text;
}

//----------------------------------------------------------------------------

void Text::setText( const char * pText )
{
	m_Text = pText;
}

//----------------------------------------------------------------------------
// EOF

/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define TEXTPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"
#include "ChildFrame.h"
#include "Resource.h"

#include "File/FileDisk.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( TextPort, Port );
REGISTER_FACTORY_KEY(  TextPort, 4280354006228992407 );

BEGIN_PROPERTY_LIST( TextPort, Port );
	ADD_PROPERTY( m_Text );
END_PROPERTY_LIST();

TextPort::TextPort() : Port()
{}

//-------------------------------------------------------------------------------

const int VERSION_081501 = 81501;
const int VERSION_052902 = 52902;

bool TextPort::read( const InStream & input )
{
	if (! Port::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_052902:
			input >> m_Text;
			break;
		case VERSION_081501:
			{
				Tree< int, CharString >	tree;
				input >> tree;

				m_Text = tree[ 0 ];
			}
			break;
		default:
			throw BadWidget();
		}
	}
	return true;
}

//----------------------------------------------------------------------------

bool TextPort::autoLoad()
{
	return false;
}

void TextPort::dependencies( DependentArray & dep )
{
	// push the dependencies of this port into this array, for example:
	// dep.push( "L:\\Projects\\Seeker\\Image1.prt" );
}

Resource::Ref TextPort::createResource()
{
	Text * pText = new Text();
	pText->setText( m_Text );

	return pText;
}

CFrameWnd * TextPort::createView()
{
	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

bool TextPort::importFile( const char * fileName )
{
	return true;	// return true of successful import, false on failure
}

void TextPort::shellOpen() const
{
	MessageBox( NULL, "shellOpen() is not valid for this port", NULL, MB_OK);
}

//-------------------------------------------------------------------------------
// EOF


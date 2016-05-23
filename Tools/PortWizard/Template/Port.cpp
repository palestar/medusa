/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define $$ROOT$$_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"
#include "ChildFrame.h"

#include "Resource.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( $$root$$, Port );

BEGIN_PROPERTY_LIST( $$root$$, Port );
END_PROPERTY_LIST();

$$root$$::$$root$$() : Port()
{}

//-------------------------------------------------------------------------------

const int VERSION_031600 = 31600;

bool $$root$$::read( InStream & input )
{
	Port::read( input );

	int version;
	input >> version;

	switch( version )
	{
	case VERSION_031600:
		// TODO: serialize in your port class
		break;
	default:
		throw BadWidget();
	}
}

bool $$root$$::write( OutStream &output )
{
	Port::write( output );

	output << VERSION_031600;
	// TODO: serialize out your port class
}

//----------------------------------------------------------------------------

bool $$root$$::autoLoad()
{
	return false;
}

void $$root$$::dependencies( DependentArray & dep )
{
	// push the dependencies of this port into this array, for example:
	// dep.push( "L:\\Projects\\Seeker\\Image1.prt" );
}

Resource * $$root$$::createResource()
{
	// return a created resource
	return NULL;
}

CFrameWnd * $$root$$::createView()
{
	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

bool $$root$$::importFile( const char * fileName )
{
	return true;	// return true of successful import, false on failure
}

void $$root$$::shellOpen() const
{
	MessageBox( NULL, "shellOpen() is not valid for this port", NULL, MB_OK);
}

//-------------------------------------------------------------------------------
// EOF


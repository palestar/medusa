/*
	DisplayDevice.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define DISPLAY_DLL
#include "Display/DisplayDevice.h"
#include "Standard/Library.h"
#include "Factory/FactoryTypes.h"

//----------------------------------------------------------------------------

// Display options
bool					DisplayDevice::sm_bUseFixedFunction = false;					// if true, then the fixed function pipeline will always be used
bool					DisplayDevice::sm_bWaitVB = true;								// wait for verticle blank
dword					DisplayDevice::sm_nTrianglesRendered = 0;						// total number of triangles rendered via lists
dword					DisplayDevice::sm_nLinesRendered = 0;
DisplayDevice *			DisplayDevice::sm_pCacheDevice = NULL;							// device for used to precache graphics assets...

CharString				DisplayDevice::sm_sShadersPath;									// base directory of shader files

#if defined(_DEBUG)
bool					DisplayDevice::sm_bEnableShaderDebug = true;			
#else
bool					DisplayDevice::sm_bEnableShaderDebug = false;				
#endif

//-------------------------------------------------------------------------------

void LoadDisplayLibs()
{
#ifdef _DEBUG
	static Library		LIB_DISPLAYD3D( "DisplayD3DD.dll" );			// Direct3D
	static Library		LIB_DISPLAYGL( "DisplayGLD.dll" );				// OpenGL
#else
	static Library		LIB_DISPLAYD3D( "DisplayD3D.dll" );			// Direct3D
	static Library		LIB_DISPLAYGL( "DisplayGL.dll" );				// OpenGL
#endif
}

//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_FACTORY( DisplayDevice, Widget );

DisplayDevice::DisplayDevice()
{}


//----------------------------------------------------------------------------

DisplayDevice * DisplayDevice::create( const char * pClass )
{
	// load the DLL's
	LoadDisplayLibs();
	// return the created object, or NULL on failure
	return WidgetCast<DisplayDevice>( Factory::createNamedWidget( pClass ) );
}

DisplayDevice * DisplayDevice::create()
{
	// load the DLL's
	LoadDisplayLibs();
	// if found, create the first available class type derived from this object
	if ( Factory::typeCount( classKey() ) > 0 )
		return WidgetCast<DisplayDevice>( Factory::createWidget( Factory::type( classKey(), 0 ) ) );

	return NULL;
}

const char * DisplayDevice::describeFSAA( FSAA eFSAA )
{
	static const char * TEXT[] = 
	{
		"FSAA_NONE",
		"FSAA_NONMASKABLE",
		"FSAA_2_SAMPLES",
		"FSAA_3_SAMPLES",
		"FSAA_4_SAMPLES",
		"FSAA_5_SAMPLES",
		"FSAA_6_SAMPLES",
		"FSAA_7_SAMPLES",
		"FSAA_8_SAMPLES",
		"FSAA_9_SAMPLES",
		"FSAA_10_SAMPLES",
		"FSAA_11_SAMPLES",
		"FSAA_12_SAMPLES",
		"FSAA_13_SAMPLES",
		"FSAA_14_SAMPLES",
		"FSAA_15_SAMPLES",
		"FSAA_16_SAMPLES",
	};

	return TEXT[ eFSAA ];
}

DisplayDevice::FSAA DisplayDevice::findFSAA( const char * pText )
{
	for(int i=0;i<FSAA_COUNT;++i)
	{
		if ( strcmp( describeFSAA( (FSAA)i ), pText ) == 0 )
			return (FSAA)i;
	}

	return FSAA_NONE;
}

//----------------------------------------------------------------------------
// EOF

/*
	PortImport.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define IMAGEPORT_DLL
#include "stdafx.h"
#include "Port.h"

#include "File/Path.h"
#include "Standard/Types.h"
#include "File/FileDisk.h"
#include "Draw/Palette.h"

#include "FreeImage/FreeImage.h"

//#include "bmp.h"
#include <vfw.h>		// video for windows

//-------------------------------------------------------------------------------

Image * ImagePort::import( const char * fileName )
{
	Path	path( fileName );
	CharString	extension( path.extension() );

	CharString sFile;
	sFile.copy( fileName );

	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(fileName);
    if( fif == FIF_UNKNOWN )
		return NULL;

	// use FreeImage library to import the graphics
	FIBITMAP * pDib = FreeImage_Load( fif, fileName );
	if ( pDib == NULL )
		return NULL;

	bool bSetAlpha = false;
	if ( FreeImage_GetBPP( pDib ) != 32 )
	{
		FIBITMAP * pConverted = FreeImage_ConvertTo32Bits( pDib );
		FreeImage_Unload( pDib );

		pDib = pConverted;
		bSetAlpha = true;
	}

	// flip the image, it is current for drawing as a DIB
	FreeImage_FlipVertical( pDib );

	SizeInt size( FreeImage_GetWidth( pDib ), FreeImage_GetHeight( pDib ) );
	Image * pImage = new Image( ColorFormat::RGB8888, size );

	Color * pColors = (Color *)FreeImage_GetBits( pDib );
	dword nPixels = size.width * size.height;
	dword nBytes = nPixels * sizeof(Color);

	// some images don't come with a alpha channel, set it to 255 in those cases..
	if ( bSetAlpha )
	{
		for(dword p=0;p<nPixels;++p)
			pColors[p].m_A = 255;
	}

	pImage->addFrame( Buffer( pColors, nBytes, true ), ColorFormat::RGB8888 );

	// unload the jpg
	FreeImage_Unload( pDib );

	// return the image
	return pImage;
}

//-------------------------------------------------------------------------------
// EOF


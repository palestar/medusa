/*
	PortHelpers.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define IMAGEPORT_DLL
#include "stdafx.h"
#include "Port.h"



//-------------------------------------------------------------------------------

BITMAPINFO * ImagePort::createDIB( const Image & image )
{
	BITMAPINFO		*pDIB = (BITMAPINFO *)new byte[ sizeof(BITMAPINFOHEADER) ];

	memset( &pDIB->bmiHeader, 0, sizeof( pDIB->bmiHeader ) );
	pDIB->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pDIB->bmiHeader.biWidth = image.size().width;
	pDIB->bmiHeader.biHeight = -image.size().height;
	pDIB->bmiHeader.biXPelsPerMeter = 1;
	pDIB->bmiHeader.biYPelsPerMeter = 1;
	pDIB->bmiHeader.biPlanes = 1;
	pDIB->bmiHeader.biBitCount = 32;
	pDIB->bmiHeader.biCompression = BI_RGB;
	pDIB->bmiHeader.biSizeImage = image.frameSize();

	return( pDIB );
}

dword ImagePort::getFileTime( const char * fileName )
{
    CFileFind fileFinder;
    if ( fileFinder.FindFile(fileName) )
    {
        fileFinder.FindNextFile();

        CTime time;
        fileFinder.GetLastWriteTime(time);

		return (dword)time.GetTime();
	}

	return(0);
}

//-------------------------------------------------------------------------------
// EOF

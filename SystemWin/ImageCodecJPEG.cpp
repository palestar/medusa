/*
	ImageCodecJPEG.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define SYSTEMWIN_DLL
#include "Debug/Assert.h"
#include "SystemWin/ImageCodecJPEG.h"

#include "SystemWin/ijl/ijl.h"

//----------------------------------------------------------------------------

bool EncodeToJPEGBuffer( byte* lpRgbBuffer, dword dwWidth, dword dwHeight, byte** lppJpgBuffer, dword* lpdwJpgBufferSize, int quality);
bool DecodeFromJPEGBuffer( byte* lpJpgBuffer, dword dwJpgBufferSize, byte** lppRgbBuffer, 
						  dword* lpdwWidth, dword* lpdwHeight, dword* lpdwNumberOfChannels);

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( ImageCodecJPEG, ImageCodec );

int	ImageCodecJPEG::encode( const Buffer & input, Buffer & output, const SizeInt & size, EncodeLevel nLevel )
{
	int nQuality = 100;
	if ( nLevel == CL_FAST )
		nQuality = 80;
	else if ( nLevel == CL_NONE )
		nQuality = 50;

	// compress the frame data
	byte *	jpgBuffer = NULL;
	dword	jpgSize;

	if (! EncodeToJPEGBuffer( (byte *)input.buffer(), size.width, size.height, &jpgBuffer, &jpgSize, nQuality ) )
		return -1;

	output.set( jpgBuffer, jpgSize );
	return jpgSize;
}

int ImageCodecJPEG::decode( const Buffer & input, Buffer & output, const SizeInt & size )
{
	dword rgbWidth, rgbHeight, rgbChannels;
	byte * rgbBuffer = NULL;

	// decompress the jpeg data into the buffer
	if (! DecodeFromJPEGBuffer( (byte *)input.buffer(), input.bufferSize(), &rgbBuffer, &rgbWidth, &rgbHeight, &rgbChannels ) )
		return -1;		// an error has occured
	if ( size.width != rgbWidth || size.height != rgbHeight )
	{
		delete [] rgbBuffer;
		return -1;		// image is not the correct size
	}

	int nBytes = rgbWidth * rgbHeight * rgbChannels;
	output.set( rgbBuffer, nBytes );

	return nBytes;
}

//----------------------------------------------------------------------------

static bool EncodeToJPEGBuffer( byte* lpRgbBuffer, dword dwWidth, dword dwHeight, byte** lppJpgBuffer, dword* lpdwJpgBufferSize, int quality )
{
	bool	bres = true;
	IJLERR	jerr;
	dword	dwRgbBufferSize;
	byte *	lpTemp = NULL;

	// Allocate the IJL JPEG_CORE_PROPERTIES structure.
	JPEG_CORE_PROPERTIES jcprops;
	__try {
		// Initialize the Intel(R) JPEG Library.
		jerr = ijlInit(&jcprops);
		if(IJL_OK != jerr)
		{
			bres = false;
			__leave;
		}

		dwRgbBufferSize = dwWidth * dwHeight * 4;
		lpTemp = new byte [dwRgbBufferSize];
		if(NULL == lpTemp)
		{
			bres = false;
			__leave;
		}

		// Set up information to write from the pixel buffer.
		jcprops.DIBWidth = dwWidth;
		jcprops.DIBHeight = dwHeight; // Implies a bottom-up DIB.
		jcprops.DIBBytes = lpRgbBuffer;
		jcprops.DIBPadBytes = 0;
		jcprops.DIBChannels = 4;
		jcprops.DIBColor = IJL_RGBA_FPX;
		jcprops.JPGWidth = dwWidth;
		jcprops.JPGHeight = dwHeight;
		jcprops.JPGFile = NULL;
		jcprops.JPGBytes = lpTemp;
		jcprops.JPGSizeBytes = dwRgbBufferSize;
		jcprops.JPGChannels = 4;
		jcprops.JPGColor = IJL_YCBCRA_FPX;
		jcprops.JPGSubsampling = IJL_4114; // 4:1:1 subsampling.
		jcprops.jquality = quality;

		// Write the actual JPEG image from the pixel buffer.
		jerr = ijlWrite(&jcprops,IJL_JBUFF_WRITEWHOLEIMAGE);
		if(IJL_OK != jerr)
		{
			bres = false;
			__leave;
		}

	} // __try
	__finally
	{
		if( bres == false )
		{
			if(NULL != lpTemp)
			{
				delete[] lpTemp;
				lpTemp = NULL;
			}
		}

		*lppJpgBuffer = lpTemp;
		*lpdwJpgBufferSize = jcprops.JPGSizeBytes;
		// Clean up the Intel(R) JPEG Library.
		ijlFree(&jcprops);
	}
	
	return bres;
} // EncodeToJPEGBuffer()

static bool DecodeFromJPEGBuffer( byte* lpJpgBuffer, dword dwJpgBufferSize, byte** lppRgbBuffer, 
						  dword* lpdwWidth, dword* lpdwHeight, dword* lpdwNumberOfChannels)
{
	bool	bres = true;
	IJLERR	jerr;
	dword	dwWholeImageSize;
	byte *	lpTemp = NULL;

	// Allocate the IJL JPEG_CORE_PROPERTIES structure.
	JPEG_CORE_PROPERTIES jcprops;
	__try
	{
		// Initialize the Intel(R) JPEG Library.
		jerr = ijlInit(&jcprops);
		if(IJL_OK != jerr)
		{
			bres = false;
			__leave;
		}
		// Get information on the JPEG image
		// (i.e., width, height, and channels).

		jcprops.JPGFile = NULL;
		jcprops.JPGBytes = lpJpgBuffer;
		jcprops.JPGSizeBytes = dwJpgBufferSize;
		jerr = ijlRead(&jcprops, IJL_JBUFF_READPARAMS);
		if(IJL_OK != jerr)
		{
			bres = false;
			__leave;
		}

		ASSERT( jcprops.JPGChannels == 4 );
		//jcprops.JPGColor = IJL_YCBCRA_FPX;
		jcprops.DIBColor = IJL_RGBA_FPX;
		jcprops.DIBChannels = 4;

		// Compute size of desired pixel buffer.
		dwWholeImageSize = jcprops.JPGWidth * jcprops.JPGHeight * jcprops.DIBChannels;
		// Allocate memory to hold the decompressed image data.
		lpTemp = new byte [dwWholeImageSize];
		if(NULL == lpTemp)
		{
			bres = false;
			__leave;
		}
		// Set up the info on the desired DIB properties.
		jcprops.DIBWidth = jcprops.JPGWidth;
		jcprops.DIBHeight = jcprops.JPGHeight;
		jcprops.DIBPadBytes = 0;
		jcprops.DIBBytes = lpTemp;
		// Now get the actual JPEG image data into the pixel buffer.
		jerr = ijlRead(&jcprops, IJL_JBUFF_READWHOLEIMAGE);
		if(IJL_OK != jerr)
		{
			bres = false;
			__leave;
		}
	} // __try
	__finally
	{
		if(false == bres)
		{
			if(NULL != lpTemp)
			{
				delete [] lpTemp;
				lpTemp = NULL;
			}
		}

		// Clean up the Intel(R) JPEG Library.
		ijlFree(&jcprops);
		*lpdwWidth = jcprops.DIBWidth;
		*lpdwHeight = jcprops.DIBHeight;
		*lpdwNumberOfChannels = jcprops.DIBChannels;
		*lppRgbBuffer = lpTemp;
	} // __finally
	
	return bres;

} // DecodeFromJPEGBuffer()

//----------------------------------------------------------------------------
//EOF

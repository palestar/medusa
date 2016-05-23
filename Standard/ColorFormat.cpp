/*
	ColorFormat.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Standard/ColorFormat.h"
#include "Standard/Bits.h"



//----------------------------------------------------------------------------

ColorFormat::Format ColorFormat::determineFormat( dword bitCount, 
	dword redMask, dword greenMask, 
	dword blueMask, dword alphaMask )
{
	switch( bitCount )
	{
	case 8:
		if ( redMask == 0xE0 && greenMask == 0x1C && blueMask == 0x3 && alphaMask == 0x0 )
			return( RGB332 );
		return INVALID;
	case 16:
		if ( redMask == 0x7C00 && greenMask == 0x3E0 && blueMask == 0x1F && alphaMask == 0x8000 )
			return RGB5551;
		if ( redMask == 0x7C00 && greenMask == 0x3E0 && blueMask == 0x1F && alphaMask == 0x0 )
			return RGB555;
		if ( redMask == 0xF800 && greenMask == 0x7E0 && blueMask == 0x1F && alphaMask == 0x0 )
			return RGB565;
		if ( redMask == 0x1F && greenMask == 0x7E0 && blueMask == 0xF800 && alphaMask == 0x0 )
			return RGB565E;
		if ( redMask == 0xF00 && greenMask == 0xF0 && blueMask == 0xF && alphaMask == 0xF000 )
			return RGB4444;
		return INVALID;
	case 24:
		if ( redMask == 0xFF0000 && greenMask == 0xFF00 && blueMask == 0xFF && alphaMask == 0x0 )
			return RGB888;
		if ( redMask == 0xFF && greenMask == 0xFF00 && blueMask == 0xFF0000 && alphaMask == 0x0 )
			return RGB888E;
		return INVALID;
	case 32:
		if ( redMask == 0xFF0000 && greenMask == 0xFF00 && blueMask == 0xFF && alphaMask == 0xFF000000 )
			return RGB8888;
		if ( redMask == 0xFF0000 && greenMask == 0xFF00 && blueMask == 0xFF && alphaMask == 0x0 )
			return RGB888L;
		if ( redMask == 0xFF && greenMask == 0xFF00 && blueMask == 0xFF0000 && alphaMask == 0x0 )
			return RGB888LE;
		return INVALID;
	}

	return INVALID;
}

ColorFormat::Ref ColorFormat::allocateFormat( Format format )
{
	switch( format )
	{
	case RGB888:
		return( new ColorFormatRGB< RGB888, dword, 16,8, 8,8, 0,8, 24,0 > );
	case RGB888E:
		return( new ColorFormatRGB< RGB888E, dword, 0,8, 8,8, 16,8, 24,0 > );
	case RGB888L:
		return( new ColorFormatRGB< RGB888L, dword, 16,8, 8,8, 0,8, 24,0, 8 > );
	case RGB888LE:
		return( new ColorFormatRGB< RGB888LE, dword, 0,8, 8,8, 16,8, 24,0, 8 > );
	case RGB565:
		return( new ColorFormatRGB< RGB565, word, 11,5, 5,6, 0,5, 16,0 > );
	case RGB565E:
		return( new ColorFormatRGB< RGB565E, word, 0,5, 5,6, 11,5, 16,0 > );
	case RGB555:
		return( new ColorFormatRGB< RGB555, word, 10,5, 5,5, 0,5, 15,0 > );
	case RGB8888:
		return( new ColorFormatRGB< RGB8888, dword, 16,8, 8,8, 0,8, 24,8 > );
	case RGB5551:
		return( new ColorFormatRGB< RGB5551, word, 10,5, 5,5, 0,5, 15,1 > );
	case RGB4444:
		return( new ColorFormatRGB< RGB4444, word, 8,4, 4,4, 0,4, 12,4 > );
	case RGB332:
		return( new ColorFormatRGB< RGB332, byte, 5,3, 2,3, 0,2, 8,0 > );
	}

	return NULL;
}

ColorFormat::Ref ColorFormat::copyFormat( const ColorFormat * pFormat )
{
	return allocateFormat( pFormat->format() );
}

bool ColorFormat::convert( const Buffer & input, ColorFormat * pInputFormat,
							Buffer & output, ColorFormat * pOutputFormat )
{
	dword nPixels = input.bufferSize() / pInputFormat->byteSize();
	if (! output.allocate( nPixels * pOutputFormat->byteSize() ) )
		return false;		// failed to allocate memory!

	byte * pInput = (byte *)input.buffer();
	byte * pOutput = (byte *)output.buffer();

	for(dword p=0;p<nPixels;++p)
	{
		pOutputFormat->setColor( pOutput, pInputFormat->getColor( pInput ) );
		pInput += pInputFormat->byteSize();
		pOutput += pOutputFormat->byteSize();
	}

	return true;
}

const char * ColorFormat::formatText( Format eFormat )
{
	static const char * FORMAT_TEXT[] = 
	{
		"RGB888",				// rgb formats
		"RGB888E",
		"RGB888L",			
		"RGB888LE",
		"RGB565",
		"RGB565E",
		"RGB555",
		"RGB8888",
		"RGB5551",
		"RGB4444",
		"RGB332",
		"JPEG",
		"DXT1",
		"DXT3",
		"DXT5"
	};
	if ( eFormat >= sizeof(FORMAT_TEXT)/sizeof(FORMAT_TEXT[0]) )
		return "INVALID";

	return FORMAT_TEXT[ eFormat ];
}

ColorFormat::Format ColorFormat::findFormat( const char * pFormat )
{
	for( Format eFormat = FIRST; eFormat <= LAST; eFormat = (Format)(eFormat + 1) )
		if ( strcmp( pFormat, formatText( eFormat ) ) == 0 )
			return eFormat;
	return INVALID;
}

//----------------------------------------------------------------------------
// EOF

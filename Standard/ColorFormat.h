/*
	ColorFormat.h
	This object is used to read/write into a buffer in a specific color format

	(c)2005 Palestar, Richard Lyle
*/

#ifndef COLOR_FORMAT_H
#define COLOR_FORMAT_H

#include <stddef.h>					// NULL
#include <memory.h>
#include <limits.h>

#include "Reference.h"
#include "Color.h"
#include "Buffer.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL ColorFormat : public Referenced
{
public:
	// Types
	typedef Reference< ColorFormat >			Ref;

	enum Format				// defined format types
	{
		RGB888 = 0,				// rgb formats
		RGB888E,
		RGB888L,			
		RGB888LE,
		RGB565,
		RGB565E,
		RGB555,
		RGB8888,
		RGB5551,
		RGB4444,
		RGB332,

		// compressed formats - allocateFormat() not supported for these formats, ImageCodec can be used 
		// to convert from these formats into RGB8888, then into other pixel formats.
		JPEG,				
		DXT1,
		DXT3,
		DXT5,

		INVALID,
		FIRST = RGB888,
		LAST = DXT5
	};

	// Accessors
	virtual Format			format() const = 0;
	virtual int				byteSize() const = 0;
	virtual int				bitSize() const = 0;

	virtual dword			redMask() const = 0;
	virtual dword			greenMask() const = 0;
	virtual dword			blueMask() const = 0;
	virtual dword			alphaMask() const = 0;

	// Mutators
	virtual dword			pixel( Color color ) = 0;								// convert color to pixel
	virtual Color			color( dword pixel ) = 0;								// convert pixel to color

	virtual Color			getColor( const void * pBuffer ) = 0;					// get color from buffer pointer
	virtual void 			setColor( void * pBuffer, Color color ) = 0;			// put color into buffer
	virtual dword			getPixel( const void * pBuffer ) = 0;
	virtual void 			setPixel( void * pBuffer, dword pixel ) = 0;

	// Static
	static Format			determineFormat( dword bitCount, 
								dword redMask, dword greenMask, 
								dword blueMask, dword alphaMask );
	static ColorFormat::Ref	allocateFormat( Format format );	
	static ColorFormat::Ref	copyFormat( const ColorFormat * pFormat );	
	static bool				convert( const Buffer & input, ColorFormat * pInputFormat,
								Buffer & output, ColorFormat * pOutputFormat );

	static const char *		formatText( Format eFormat );
	static Format			findFormat( const char * pFormat );
};

inline bool operator==( const ColorFormat & lh, const ColorFormat & rh )
{
	return( lh.format() == rh.format() );
}

inline bool operator!=( const ColorFormat & lh, const ColorFormat & rh )
{
	return( ! operator==( lh, rh ) );
}

//----------------------------------------------------------------------------

#pragma warning(disable:4244)	// conversion from 'unsigned long' to 'unsigned char', possible loss of data

template< unsigned pixelFormat, class pixelType, 
unsigned redShift, unsigned redBits, 
unsigned greenShift, unsigned greenBits,
unsigned blueShift, unsigned blueBits,
unsigned alphaShift, unsigned alphaBits, 
unsigned alignment = 0 > 

class ColorFormatRGB : public ColorFormat
{
public:
	// Constants
	enum {
		BITSIZE				= (redBits + greenBits + blueBits + alphaBits + alignment),
		BYTESIZE			= ((BITSIZE + 7) / 8),

		REDMASK				= ((1 << redBits) - 1) << redShift,
		REDSCALE			= (8 - redBits),
		GREENMASK			= ((1 << greenBits) - 1) << greenShift,
		GREENSCALE			= (8 - greenBits),
		BLUEMASK			= ((1 << blueBits) - 1) << blueShift,
		BLUESCALE			= (8 - blueBits),
		ALPHAMASK			= ((1 << alphaBits) - 1) << alphaShift,
		ALPHASCALE			= (8 - alphaBits),
	};

	// Accessors
	Format format() const
	{
		return (Format)pixelFormat;
	}
	int	byteSize() const
	{
		return BYTESIZE;
	}
	int bitSize() const
	{
		return BITSIZE;
	}

	dword redMask() const
	{
		return REDMASK;
	}
	dword greenMask() const
	{
		return GREENMASK;
	}
	dword blueMask() const
	{
		return BLUEMASK;
	}
	dword alphaMask() const
	{
		return ALPHAMASK;
	}

	dword pixel( Color color )
	{
		return( (((dword)color.m_R >> REDSCALE) << redShift) |
			(((dword)color.m_G >> GREENSCALE) << greenShift) |
			(((dword)color.m_B >> BLUESCALE) << blueShift) |
			(((dword)color.m_A >> ALPHASCALE) << alphaShift) );
	}
	Color color( dword pixel )
	{
		return( Color( ((pixel & REDMASK) >> redShift) << REDSCALE, 
			((pixel & GREENMASK) >> greenShift) << GREENSCALE, 
			((pixel & BLUEMASK) >> blueShift) << BLUESCALE, 
			((pixel & ALPHAMASK) >> alphaShift) << ALPHASCALE ) );
	}

	Color getColor( const void * pBuffer )
	{
		return( color( getPixel( pBuffer ) ) );
	}
	void  setColor( void * pBuffer, Color color )
	{
		setPixel( pBuffer, pixel( color ) ); 
	}
	dword getPixel( const void * pBuffer )
	{
		return( *((const pixelType *)pBuffer) );
	}
	void setPixel( void * pBuffer, dword pixel )
	{
		*((pixelType *)pBuffer) = pixel;
	}
};

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF

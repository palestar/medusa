/*
	Palette.h

	8-bit palette object, NOTE: this could be made a template, then we could support any palette size
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PALETTE_H
#define PALETTE_H

#include "Standard/Color.h"
#include "Standard/Array.h"
#include "Standard/ColorFormat.h"
#include "File/Stream.h"
#include "MedusaDll.h"



//----------------------------------------------------------------------------

#pragma warning(disable:4251)	// needs to have dll-interface to be used by clients of class 'Hydra::Palette'

class Image;	// forward declare

class DLL Palette 
{
public:
	// Construction
	Palette();

	// Accessors
	Color			operator[](int index) const;
	const Color *	pointer() const;
	int				colors() const;
	int				findColor( Color color ) const;						// find closest color

	// Mutators
	Color &			operator[](int index);								// throws OutOfBounds()

	void			copy( const Palette &copy );
	void			createUsingMedianCut( const Array< Image * > & images );
	void			createUsingMedianCut( void * pPixels, int pixelCount,
						ColorFormat * pFormat, byte *pDst = NULL );		// optionally remap the RGB data into indexed data
private:
	// Constants
	enum {
		COLORS			= 256,

		REMAP_BITS		= 5,
		REMAP_SCALE		= 8 - REMAP_BITS,
		REMAP_COUNT		= 1 << REMAP_BITS,
		REMAP_SIZE		= REMAP_COUNT * REMAP_COUNT * REMAP_COUNT,
	};
	// 	Data
	Color			m_Palette[ COLORS ];
};

//----------------------------------------------------------------------------

inline int	Palette::colors() const
{
	return( COLORS );
}

inline const Color * Palette::pointer() const
{
	return( m_Palette );
}

inline Color Palette::operator[](int index) const
{
	if (index >= 0 && index < COLORS)
		return( m_Palette[ index ] );
	return BLACK;
}

inline Color & Palette::operator[](int index)
{
	if (index >= 0 && index < COLORS)
		return( m_Palette[ index ] );
	static Color NONE(0,0,0);
	return NONE;
}

//----------------------------------------------------------------------------

inline const OutStream & operator<<( const OutStream & output, const Palette & palette )
{
	for(int i=0;i<palette.colors();i++)
		output << palette[i];
	return output;
}


inline const InStream & operator>>( const InStream & input, Palette & palette )
{
	for(int i=0;i<palette.colors();i++)
		input >> palette[i];
	return input;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

/*
	Font.h

	Font Resource
	(c)2005 Palestar, Richard Lyle
*/

#ifndef FONT_H
#define FONT_H

#include "Display/PrimitiveSurface.h"
#include "Display/PrimitiveMaterial.h"
#include "Resource/Resource.h"
#include "Standard/Size.h"
#include "Standard/Array.h"
#include "Standard/String.h"
#include "Math/Vector3.h"

#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Font : public Resource
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef ResourceLink< Font >	Link;
	typedef Reference< Font >		Ref;

	enum Constants
	{
		TAB_SIZE				= 20,			// how many pixels is a tab (/t)
		MAX_FONT_SURFACE_WIDTH	= 1024,
		MAX_FONT_SURFACE_HEIGHT = 1024,
	};

	// Construction
	Font();
	Font( const Font & copy );
	Font( const SizeInt &size, int asciiOffset, int charCount );
	virtual	~Font();

	// Widget Interface
	bool				read( const InStream & input );
	bool				write( const OutStream &output );	

	// Accessors
	const SizeInt &		size() const;								// max font dimensions
	SizeInt 			size( int character ) const;				// width of a specific character
	SizeInt				size( const wchar * pString ) const;		// get the size of the string
	SizeInt				size( const char * pString ) const;			// get the size of the string

	int					asciiOffset() const;						// ascii offset
	int					characterWidth( int character ) const;		// get the width of the character
	int					characterSize() const;						// size in bytes of each character
	int					characterPitch() const;						// number of bytes for each line of the character
	int					characterCount() const;						// number of characters in font
	byte *				character( int character ) const;			// get bitmap data for specific character
	
	// Mutators
	void				setAsciiOffset( int offset );				// set the ascii offset
	void				setWidth( int character, int width );

	byte *				addCharacter( int width );					// add character to end
	byte *				insertCharacter( int character, int width );	// insert character
	void				removeCharacter( int character );			

	void				release();									// release font

	void				push( DisplayDevice * pDisplay, 
							Vector3 & pos, const wchar * pText, Color color );

	// static
	static void			push( DisplayDevice * pDisplay, Font * pFont,
							Vector3 & pos, const wchar * pText, Color color );
	static void			push( DisplayDevice * pDisplay, Font * pFont,
							PointInt & pos, const wchar * pText, Color color );
	static void			push( DisplayDevice * pDisplay, Font * pFont,
							PointInt & pos, const char * pText, Color color );
							
	static bool			sm_bEnableAlpha;								// enable alpha fonts instead of additive

private:
	// Data
	SizeInt				m_Size;										// Maximum character size
	int					m_Offset;									// Ascii offset
	
	Array< byte * > 
						m_Characters;								// character bitmaps
	Array< int >
						m_CharacterWidth;							// width of each character

	// non-serialized
	int					m_SurfaceShift;								// character to surface
	PrimitiveMaterial::Ref
						m_FontMaterial;
	Array< PrimitiveSurface::Ref >
						m_FontSurface;								// created font surface
	Array< RectFloat >
						m_FontUV;									// character UV's

	// Mutators
	void				createFontMaterial( DisplayDevice * pDisplay );
	bool				createFontSurface( DisplayDevice * pDisplay );
	ColorFormat::Ref	findFormat( DisplayDevice * pDisplay );
};

//----------------------------------------------------------------------------

inline int Font::characterWidth( int ch ) const
{
	if ( ch == '\t' )
		return( TAB_SIZE );
	if ( ch >= m_Offset && ch < (m_Offset + m_CharacterWidth.size()) )
		return( m_CharacterWidth[ ch - m_Offset ] );

	return 0;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

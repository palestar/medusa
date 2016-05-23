/*
	Font.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define DRAW_DLL
#include "Draw/Font.h"
#include "Draw/Draw.h"
#include "Standard/Limits.h"
#include "Standard/Bits.h"
#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Display/PrimitiveMaterial.h"
#include "Display/PrimitiveTriangleList.h"

#include <math.h>

//----------------------------------------------------------------------------

bool			Font::sm_bEnableAlpha = true;					// enable alpha fonts instead of additive

//-------------------------------------------------------------------------------

IMPLEMENT_RESOURCE_FACTORY( Font, Resource );

Font::Font() : m_Offset( 0 )
{}

Font::Font( const Font & copy )
{
	//TRACE("Font: Copy Constructor Called");
	m_Size = copy.m_Size;
	m_Offset = copy.m_Offset;

	for(int i=0;i<copy.characterCount();i++)
		memcpy( addCharacter( copy.size(i).width ), copy.character(i), characterSize() );
}

Font::Font( const SizeInt &size, int asciiOffset, int charCount ) 
{
	m_Size = size;
	m_Offset = asciiOffset;

	for(int i=0;i<charCount;i++)
		addCharacter( 0 );
}

Font::~Font()
{
	release();
}

//-------------------------------------------------------------------------------

const int VERSION_031600 = 31600;

bool Font::read( const InStream & input )
{
	if (! Resource::read( input ) )
		return false;

	int version;
	input >> version;

	switch( version )
	{
	case VERSION_031600:
		{
			input >> m_Size;
			input >> m_Offset;

			int characterCount;
			input >> characterCount;
			for(int i=0;i<characterCount;i++)
			{
				int characterWidth;
				input >> characterWidth;

				input.read( addCharacter( characterWidth ), characterSize() );
			}
		}
		break;
	default:
		return false;
	}

	if ( DisplayDevice::sm_pCacheDevice != NULL )
	{
		createFontMaterial( DisplayDevice::sm_pCacheDevice );
		createFontSurface( DisplayDevice::sm_pCacheDevice );
	}

	return true;
}

bool Font::write( const OutStream &output )
{
	if (! Resource::write( output ) )
		return false;

	output << VERSION_031600;
	output << m_Size;
	output << m_Offset;

	output << characterCount();
	for(int i=0;i<characterCount();i++)
	{
		output << m_CharacterWidth[i];
		output.write( m_Characters[i], characterSize() );
	}

	return true;
}

//-------------------------------------------------------------------------------

const SizeInt & Font::size() const
{
	return( m_Size );
}

SizeInt Font::size( int ch ) const
{
	return( SizeInt( characterWidth( ch ), m_Size.height ) );
}

SizeInt Font::size( const wchar * pString ) const
{
	int maxWidth = 0;
	int width = 0;
	int height = m_Size.height;
	while( *pString != 0 )
	{
		if ( *pString == '\n' )
		{
			maxWidth = Max( maxWidth, width );
			height += m_Size.height;
			width = 0;

			pString++;
			continue;
		}

		width += characterWidth( *pString );
		pString++;
	}

	maxWidth = Max( maxWidth, width );

	return( SizeInt( maxWidth, height ) );
}

SizeInt Font::size( const char * pString ) const
{
	return size( WideString( pString ) );
}

int Font::asciiOffset() const
{
	return( m_Offset );
}

int Font::characterSize() const
{
	return( characterPitch() * m_Size.height );
}

int Font::characterPitch() const
{
	return( (m_Size.width + 7) / 8 );
}

int Font::characterCount() const
{
	return( m_Characters.size() );
}

byte * Font::character( int ch ) const
{
	return( m_Characters[ch] );
}

//-------------------------------------------------------------------------------

void Font::setAsciiOffset( int offset )
{
	m_Offset = offset;
}

void Font::setWidth( int character, int width )
{
	m_CharacterWidth[ character ] = width;
}

byte * Font::addCharacter( int width )
{
	byte *newCharacter = new byte[ characterSize() ];
	m_Characters.push( newCharacter );
	m_CharacterWidth.push( width );

	return( newCharacter );
}

byte * Font::insertCharacter(  int character, int width )
{
	byte *newCharacter = new byte[ characterSize() ];
	m_Characters.insert( character, newCharacter );
	m_CharacterWidth.insert( character, width );

	return( newCharacter );
}

void Font::removeCharacter( int character )
{
	delete m_Characters[ character ];
	m_Characters.remove( character );
	m_CharacterWidth.remove( character );
}

void Font::release()
{
	for(int i=0;i<m_Characters.size();i++)
		delete m_Characters[ i ];
	m_Characters.release();
	m_CharacterWidth.release();
}

//----------------------------------------------------------------------------

void Font::push( DisplayDevice * pDisplay, Vector3 & pos, const wchar * pText, Color color )
{
	// check for zero length string, if so just return
	if ( *pText == 0 )
		return;

	if (! m_FontMaterial.valid() )
		createFontMaterial( pDisplay );
	if ( m_FontSurface.size() < 1 )
		createFontSurface( pDisplay );

	// bias the position
	pos.x = ceil( pos.x ) - 0.5f;
	pos.y = ceil( pos.y ) - 0.5f;

	// push material
	PrimitiveMaterial::push( pDisplay, m_FontMaterial );

	const Vector3 Y( 0, m_Size.height, 0 );
	float left = pos.x;

	int nLastSurface = -1;
	int characters = m_Characters.size();
	while( *pText != 0 )
	{
		int nSurface = -1;

		// scan forward until the end of the string or a surface change occurs
		const wchar * pEnd = pText;
		while( *pEnd != 0 )
		{
			int ch = (*pEnd) - m_Offset;
			if ( ch >= 0 && ch < characters )
			{
				int chsurface = ch >> m_SurfaceShift;
				if ( nSurface < 0 )
					nSurface = chsurface;
				else if ( chsurface != nSurface )
					break;
			}

			pEnd++;
		}

		if ( nSurface < 0 )
			break;		// invalid surface

		// set the current surface for the font
		if ( nLastSurface != nSurface )
		{
			DevicePrimitive::push( pDisplay, m_FontSurface[ nSurface ] );
			nLastSurface = nSurface;
		}

		int count = pEnd - pText;
		PrimitiveTriangleListDTL::Ref triangleList = 
			PrimitiveTriangleListDTL::create( pDisplay, count * 2 );		// two triangles per character, six verticies per character

		VertexTL * pVertex = (VertexTL *)triangleList->lock();
		while( pText < pEnd )
		{
			int ch = *pText++;
			if ( ch == '\n') 
			{
				pos.x = left;
				pos += Y;
			}

			ch -= m_Offset;
			if ( ch >= 0 && ch < characters )
			{
				int			characterWidth = m_CharacterWidth[ ch ];
				RectFloat &	characterUV = m_FontUV[ ch ];

				const Vector3 X( characterWidth, 0, 0 );

				pVertex->position = pos;
				pVertex->diffuse = color;
				pVertex->w = 1.0f;
				pVertex->u = characterUV.m_Left;
				pVertex->v = characterUV.m_Top;
				pVertex++;

				pVertex->position = pos + X;
				pVertex->diffuse = color;
				pVertex->w = 1.0f;
				pVertex->u = characterUV.m_Right;
				pVertex->v = characterUV.m_Top;
				pVertex++;

				pVertex->position = pos + Y;
				pVertex->diffuse = color;
				pVertex->w = 1.0f;;
				pVertex->u = characterUV.m_Left;
				pVertex->v = characterUV.m_Bottom;
				pVertex++;

				pVertex->position = pos + X + Y;
				pVertex->diffuse = color;
				pVertex->w = 1.0f;
				pVertex->u = characterUV.m_Right;
				pVertex->v = characterUV.m_Bottom;
				pVertex++;

				pVertex->position = pos + Y;
				pVertex->diffuse = color;
				pVertex->w = 1.0f;
				pVertex->u = characterUV.m_Left;
				pVertex->v = characterUV.m_Bottom;
				pVertex++;

				pVertex->position = pos + X;
				pVertex->diffuse = color;
				pVertex->w = 1.0f;
				pVertex->u = characterUV.m_Right;
				pVertex->v = characterUV.m_Top;
				pVertex++;

				// advance the position
				pos += X;
			}
			else
			{
				// invalid character in the string, push a blank character because it's too late to reallocate the triangle list
				for(int j=0;j<6;j++)
				{
					pVertex->position = pos;
					pVertex->diffuse = color;
					pVertex->w = 1.0f;
					pVertex->u = pVertex->v = 0.0f;

					pVertex++;
				}
			}

		}	// end while


		triangleList->unlock();
		// push the triangle list
		pDisplay->push( triangleList );

	}

	// unbias the position
	pos.x += 0.5f;
	pos.y += 0.5f;
}

//----------------------------------------------------------------------------

void Font::push( DisplayDevice * pDisplay, Font * pFont, Vector3 & pos, const wchar * pText, Color color )
{
	if ( pFont != NULL )
		pFont->push( pDisplay, pos, pText, color );
}

void Font::push( DisplayDevice * pDisplay, Font * pFont, PointInt & pos, const wchar * pText, Color color )
{
	if ( pFont != NULL )
	{
		Vector3 vpos( pos.m_X, pos.m_Y, 0 );
		pFont->push( pDisplay, vpos, pText, color );

		// store position back to point
		pos.m_X = (int)vpos.x;
		pos.m_Y = (int)vpos.y;
	}
}

void Font::push( DisplayDevice * pDisplay, Font * pFont, PointInt & pos, const char * pText, Color color )
{
	push( pDisplay, pFont, pos, WideString( pText ), color );
}

//----------------------------------------------------------------------------

void Font::createFontMaterial( DisplayDevice * pDisplay )
{
	m_FontMaterial = PrimitiveMaterial::create( pDisplay, WHITE, WHITE, WHITE, WHITE, 0.0f, 
		sm_bEnableAlpha ? PrimitiveMaterial::ALPHA : PrimitiveMaterial::ADDITIVE );
	m_FontMaterial->setPass( DisplayDevice::SECONDARY );
	m_FontMaterial->setFilterMode( PrimitiveMaterial::FILTER_OFF );
	m_FontMaterial->setLightEnable( false );
}

bool Font::createFontSurface( DisplayDevice * pDisplay )
{
	m_FontSurface.release();
	m_FontUV.release();

	// determine the size and number of surfaces needed for this font
	int surfaceCount = 1;
	int charactersPerSurface = 1;

	SizeInt maxSize( pDisplay->textureMaxSize() );
	if ( maxSize.width > MAX_FONT_SURFACE_WIDTH )
		maxSize.width = MAX_FONT_SURFACE_WIDTH;
	if ( maxSize.height > MAX_FONT_SURFACE_HEIGHT )
		maxSize.height = MAX_FONT_SURFACE_HEIGHT;
	SizeInt minSize( pDisplay->textureMinSize() );
	SizeInt size( Max( minSize.width, 1), Max( minSize.height, 1 ) );					// begin with the smallest texture size possible

	ASSERT( size.width <= maxSize.width && size.height <= maxSize.height );
	ASSERT( size.width == size.height );
	ASSERT( (size.width & (size.width - 1)) == 0 );

	while( true)
	{
		// determine how many characters will fit on the current surface size
		int columns = size.width / (m_Size.width + 2);
		int rows = size.height / (m_Size.height + 2 );
		
		charactersPerSurface = 1 << GetLastBit( columns * rows );	// calc the number of characters on 1 surface, make it a power of 2
		if ( (charactersPerSurface * surfaceCount) < m_Characters.size() )
		{
			// either increase the surface size or count
			if ( (size.width << 1) <= maxSize.width && (size.height << 1) <= maxSize.height )
			{
				// increase the surface size, keep it a power of 2 and square
				size.width <<= 1;
				size.height <<= 1;
			}
			else
			{
				// surface size is maximum, increase the surface count
				surfaceCount++;
			}
		}
		else
			break;		// done
	}

	m_SurfaceShift = GetLastBit( charactersPerSurface );
	ASSERT( (1 << m_SurfaceShift) == charactersPerSurface );		// assert characters per surface power of 2

	//TRACE( String().format("Font::createFontSurface(), surfaces = %d, width = %d, height = %d", surfaceCount, size.width, size.height) );

	// find the best format
	ColorFormat::Ref pFormat = findFormat( pDisplay );
	if (! pFormat.valid() )
		return false;

	int ch = 0;
	int characters = m_Characters.size();
	int col = size.width / (m_Size.width + 2);
	int row = size.height / (m_Size.height + 2 );

	// allocate the UV information for the characters
	m_FontUV.allocate( characters );
	// create and initialize the surfaces
	m_FontSurface.allocate( surfaceCount );
	for(int k=0;k<surfaceCount;k++)
	{
		// create the surface
		m_FontSurface[k] = PrimitiveSurface::create( pDisplay, size.width, size.height, pFormat->format(), false );

		// create the draw object for the surface
		Draw::Ref drawer = Draw::create( m_FontSurface[k] );
		
		// calculate the texel size per pixel of this surface
		double du = 1.0 / size.width;
		double dv = 1.0 / size.height;

		Color fontColor( 255, 255, 255, 255 );

		// draw the font into the surface
		int chs = 0;
		for(int i=0;i<row && ch < characters && chs < charactersPerSurface;i++)
			for(int j=0;j<col && ch < characters && chs < charactersPerSurface;j++)
			{
				// calculate the draw position
				PointInt position( (j * (m_Size.width + 2)) + 1, (i * (m_Size.height + 2)) + 1 );
				// store the UVs for this character
				m_FontUV[ ch ] = RectFloat( 
					(du * position.x), 
					(dv * position.y), 
					(du * (position.x + m_CharacterWidth[ ch ])),
					(dv * (position.y + m_Size.height)) );

				// draw the character into the texture
				drawer->draw( position, this, ch + m_Offset, fontColor );
				
				// next character
				ch++;
				chs++;
			}

		// outline the font text with black with an alpha
		if ( pFormat->alphaMask() != 0 )
			drawer->outline( fontColor, Color(0,0,0,255) );
	}

	return true;
}

ColorFormat::Ref Font::findFormat( DisplayDevice * pDisplay )
{
	// determine a color format for this surface, no pixel alpha
	ColorFormat::Format bestFormat = ColorFormat::INVALID;
	ColorFormat::Format bestAlphaFormat = ColorFormat::INVALID;

	for(int i=0;i<pDisplay->surfaceFormatCount();i++)
	{
		ColorFormat::Format f = pDisplay->surfaceFormat(i);
		switch( f )
		{
		case ColorFormat::RGB888:
		case ColorFormat::RGB888E:
		case ColorFormat::RGB888L:
		case ColorFormat::RGB888LE:
			if ( bestFormat == ColorFormat::INVALID )
				bestFormat = f;
			break;
		case ColorFormat::RGB565:
		case ColorFormat::RGB565E:
		case ColorFormat::RGB555:
			bestFormat = f;
			break;
		case ColorFormat::RGB8888:
			bestAlphaFormat = f;
			break;
		case ColorFormat::RGB4444:
			if ( bestAlphaFormat == ColorFormat::INVALID || bestAlphaFormat == ColorFormat::RGB5551 )
				bestAlphaFormat = f;
			break;
		case ColorFormat::RGB5551:
			if ( bestAlphaFormat == ColorFormat::INVALID )
				bestAlphaFormat = f;
			break;
		default:
			break;			// ignore these formats
		}
	}
	if ( bestFormat == ColorFormat::INVALID )
		return NULL;

	if ( sm_bEnableAlpha && bestAlphaFormat != ColorFormat::INVALID )
		return ColorFormat::allocateFormat( bestAlphaFormat );
	
	// either alpha fonts are disabled or failed to find a surface format with alpha channel
	sm_bEnableAlpha = false;
	return( ColorFormat::allocateFormat( bestFormat ) );
}

//-------------------------------------------------------------------------------
// EOF

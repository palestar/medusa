/*
	Draw.cpp

	Generic Draw object
	(c)2005 Palestar, Richard Lyle
*/

#define DRAW_DLL

#include "Debug/Assert.h"
#include "Draw/Draw.h"
#include "Standard/Library.h"
#include "DrawBitIterator.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( Draw, Widget );

Draw::Draw()
{}

//-------------------------------------------------------------------------------

void Draw::setSurface( PrimitiveSurface::Ref surface )
{
	m_Surface = surface;
}

bool Draw::draw( const PointInt &point, Image * pImage, int f /*= 0*/)
{
	if ( f < 0 || f >= pImage->frameCount() )
		return false;
	if ( pImage->format() != ColorFormat::RGB8888 && !pImage->setFormat( ColorFormat::RGB8888 ) )
		return false;

	PointInt dstPoint( point - pImage->hotSpot() );
	RectInt	dstRect( dstPoint, pImage->size() );
	RectInt dstRectClipped( m_Surface->rectangle() & dstRect );
	RectInt srcRect( dstRectClipped - dstPoint );
	
	if (! srcRect.valid() )
	{
		TRACE( "Draw::draw() - srcRect is not valid!" );
		return false;
	}

	ColorFormat::Ref dstFormat = ColorFormat::allocateFormat( m_Surface->colorFormat() );
	if (! dstFormat )
	{
		TRACE( "Draw::draw() - dstFormat is NULL!" );
		return false;
	}
	int pixelSize = dstFormat->byteSize();

	Color * srcSurface = (Color *)pImage->frame( f );
	if ( !srcSurface )
	{
		TRACE( "Draw::draw() - srcSurface is NULL!" );
		return false;
	}
	srcSurface += srcRect.left + (srcRect.top * pImage->size().width );

	byte * dstSurface = m_Surface->lock();
	if (! dstSurface )
	{
		TRACE( "Draw::draw() - Failed to lock surface!" );
		return false;
	}
	dstSurface += (dstRectClipped.left * pixelSize) + (dstRectClipped.top * m_Surface->pitch());

	if ( pImage->colorKeyEnable() )
	{
		Color ignore( pImage->colorKey() );

		for(int line = srcRect.top; line <= srcRect.bottom;line++)
		{
			byte * dst = dstSurface;
			dstSurface += m_Surface->pitch();
			Color * src = srcSurface;
			srcSurface += pImage->size().width;

			for(int pixel = srcRect.left;pixel <= srcRect.right;pixel++)
			{
				Color color = *src++;
				if ( color != ignore )
					dstFormat->setColor( dst, color );
				dst += pixelSize;
			}
		}
	}
	else	// straight copy, no colorkey
	{
		for(int line = srcRect.top; line <= srcRect.bottom;line++)
		{
			byte * dst = dstSurface;
			dstSurface += m_Surface->pitch();
			Color * src = srcSurface;
			srcSurface += pImage->size().width;

			for(int pixel = srcRect.left;pixel <= srcRect.right;pixel++)
			{
				dstFormat->setColor( dst, *src++ );
				dst += pixelSize;
			}
		}
	}

	m_Surface->unlock();

	return true;
}

bool Draw::drawAlpha( const PointInt &point, Image * pImage, int f /*= 0*/)
{
	if ( f < 0 || f >= pImage->frameCount() )
	{
		TRACE( "Draw::drawAlpha() - Invalid frame!" );
		return false;
	}
	if ( pImage->format() != ColorFormat::RGB8888 && !pImage->setFormat( ColorFormat::RGB8888 ) )
		return false;

	ColorFormat::Ref dstFormat = ColorFormat::allocateFormat( m_Surface->colorFormat() );
	if ( dstFormat->alphaMask() == 0x0 )
		return false;		// no alpha channel in surface

	PointInt dstPoint( point - pImage->hotSpot() );
	RectInt	dstRect( dstPoint, pImage->size() );
	RectInt dstRectClipped( m_Surface->rectangle() & dstRect );
	RectInt srcRect( dstRectClipped - dstPoint );

	if (! srcRect.valid() )
	{
		TRACE( "Draw::drawAlpha() - srcRect is not valid!" );
		return false;
	}
	int pixelSize = dstFormat->byteSize();

	Color * srcSurface = (Color *)pImage->frame( f );
	if ( !srcSurface )
	{
		TRACE( "Draw::draw() - srcSurface is NULL!" );
		return false;
	}
	srcSurface += srcRect.left + (srcRect.top * pImage->size().width );

	byte * dstSurface = m_Surface->lock();
	if (! dstSurface )
	{
		TRACE( "Draw::draw() - Failed to lock surface!" );
		return false;
	}
	dstSurface += (dstRectClipped.left * pixelSize) + (dstRectClipped.top * m_Surface->pitch());

	for(int line = srcRect.top; line <= srcRect.bottom;line++)
	{
		byte * dst = dstSurface;
		dstSurface += m_Surface->pitch();
		Color * src = srcSurface;
		srcSurface += pImage->size().width;

		for(int pixel = srcRect.left;pixel <= srcRect.right;pixel++)
		{
			// get the color from the surface
			Color color( dstFormat->getColor( dst ) );

			// modify it's alpha channel
			color.m_A = (*src).magnitudeNoAlpha();
			src++;

			// set the color in the surface
			dstFormat->setColor( dst, color );
			dst += pixelSize;
		}
	}

	m_Surface->unlock();

	return true;
}

bool Draw::draw( PointInt &point, Font * pFont, int ch, const Color &color )
{
	if ( ch < pFont->asciiOffset() || ch >= (pFont->asciiOffset() + pFont->characterCount()) )
	{
		TRACE( "Draw::draw() - Font character is invalid!" );
		return false;
	}

	int chIndex = ch - pFont->asciiOffset();

	RectInt	dstRect( point, pFont->size( ch ) );
	RectInt dstRectClipped( m_Surface->rectangle() & dstRect );
	RectInt srcRect( dstRectClipped - point );
	
	if (! srcRect.valid() )
	{
		TRACE( "Draw::draw() - srcRect is not valid!" );
		return false;
	}

	ColorFormat::Ref dstFormat = ColorFormat::allocateFormat( m_Surface->colorFormat() );

	dword	pixelColor = dstFormat->pixel( color );
	int		pixelSize = dstFormat->byteSize();

	byte * dstSurface = m_Surface->lock();
	if (! dstSurface )
	{
		TRACE( "Draw::draw() - Failed to lock surface for font draw!" );
		return false;
	}
	dstSurface += (dstRectClipped.left * pixelSize) + (dstRectClipped.top * m_Surface->pitch());

	DrawBitIterator srcSurface( pFont->character( chIndex ) );
	srcSurface += srcRect.left + (srcRect.top * (pFont->characterPitch() * 8));

	dword dstPitch = m_Surface->pitch();
	dword srcPitch = pFont->characterPitch() * 8;

	for(int line = srcRect.top; line <= srcRect.bottom;line++)
	{
		byte * dst = dstSurface;
		dstSurface += dstPitch;
		DrawBitIterator src = srcSurface;
		srcSurface += srcPitch;

		for(int pixel = srcRect.left;pixel <= srcRect.right;pixel++)
		{
			if ( src.getBit() )
				dstFormat->setPixel( dst, pixelColor );
			dst += pixelSize;
		}
	}

	m_Surface->unlock();

	// advance the draw position
	point.set( point.x + dstRect.width(), point.y );

	return true;
}

bool Draw::draw( PointInt &point, Font * pFont, const String &string, const Color &color )
{
	int nLen = string.length();
	for(int i=0;i<nLen;i++)
		draw( point, pFont, string[i], color );

	return true;
}

//----------------------------------------------------------------------------

inline int lineSgn( int a )
{
	if ( a > 0 )
		return 1;
	else if ( a < 0 )
		return -1;

	return 0;
}

bool Draw::line( const PointInt &from, const PointInt &to, const Color &color )
{
	// TODO: add some trival rejection code

	int u = to.m_X - from.m_X;
	int v = to.m_Y - from.m_Y;
	int d1x = lineSgn( u );
	int d1y = lineSgn( v );
	int d2x = lineSgn( u );
	int d2y = 0;
	int m = abs( u );
	int n = abs( v );

	if ( m <= n )
	{
		d2x = 0;
		d2y = lineSgn( v );
		m = abs( v );
		n = abs( u );
	}

	RectInt			dstRect( m_Surface->rectangle() );
	ColorFormat::Ref	dstFormat = ColorFormat::allocateFormat( m_Surface->colorFormat() );
	int				dstPixelSize = dstFormat->byteSize();
	dword			dstPixel = dstFormat->pixel( color );
	byte *			dstSurface = m_Surface->lock();
	if (! dstSurface )
	{
		TRACE( "Draw::line() - Failed to lock surface" );
		return false;
	}

	int				dstPitch = m_Surface->pitch(); // / dstFormat->byteSize();

	int s = m / 2;

	PointInt plot( from );

	for(int i=0;i<m;i++)
	{
		if ( dstRect.inRect( plot ) )
			dstFormat->setPixel( dstSurface + (plot.m_X * dstPixelSize) + (plot.m_Y * dstPitch), dstPixel );

		s += n;
		if ( s >= m )
		{
			s -= m;
			plot.m_X += d1x;
			plot.m_Y += d1y;
		}
		else
		{
			plot.m_X += d2x;
			plot.m_Y += d2y;
		}
	}

	m_Surface->unlock();

	return true;
}

//----------------------------------------------------------------------------

inline void circlePlot( byte * dst, ColorFormat * dstFormat, int dstPitch, RectInt & dstRect, 
					   int x, int y, dword pixel )
{
	if ( dstRect.inRect( x, y ) )
		dstFormat->setPixel( dst + (x * dstFormat->byteSize()) + (y * dstPitch), pixel );
}

inline void circlePoints( byte * dst, ColorFormat * dstFormat, int dstPitch, RectInt & dstRect, 
						 const PointInt &center, PointInt &offset, dword pixel )
{
	circlePlot( dst, dstFormat, dstPitch, dstRect, center.m_X + offset.m_X, center.m_Y + offset.m_Y, pixel );
	circlePlot( dst, dstFormat, dstPitch, dstRect, center.m_X + offset.m_Y, center.m_Y + offset.m_X, pixel );
	circlePlot( dst, dstFormat, dstPitch, dstRect, center.m_X + offset.m_Y, center.m_Y - offset.m_X, pixel );
	circlePlot( dst, dstFormat, dstPitch, dstRect, center.m_X + offset.m_X, center.m_Y - offset.m_Y, pixel );
	circlePlot( dst, dstFormat, dstPitch, dstRect, center.m_X - offset.m_X, center.m_Y - offset.m_Y, pixel );
	circlePlot( dst, dstFormat, dstPitch, dstRect, center.m_X - offset.m_Y, center.m_Y - offset.m_X, pixel );
	circlePlot( dst, dstFormat, dstPitch, dstRect, center.m_X - offset.m_Y, center.m_Y + offset.m_X, pixel );
	circlePlot( dst, dstFormat, dstPitch, dstRect, center.m_X - offset.m_X, center.m_Y + offset.m_Y, pixel );
}

bool Draw::circle( const PointInt &center, int radius, const Color &color )
{
	RectInt			dstRect( m_Surface->rectangle() );
	ColorFormat::Ref	dstFormat = ColorFormat::allocateFormat( m_Surface->colorFormat() );
	dword			dstPixel = dstFormat->pixel( color );
	byte *			dstSurface = m_Surface->lock();
	if (! dstSurface )
	{
		TRACE( "Draw::circle() - Failed to lock surface" );
		return false;
	}
	int				dstPitch = m_Surface->pitch() / dstFormat->byteSize();

	PointInt offset( 0, radius );

	int d = 1 - radius;
	int deltaE = 3;
	int deltaSE = -2 * radius + 5;

	circlePoints( dstSurface, dstFormat, dstPitch, dstRect, center, offset, dstPixel );

	while ( offset.m_Y > offset.m_X)
	{
		if (d < 0)
		{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
		}
		else
		{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;
			offset.m_Y--;
		}
		offset.m_X++;

		circlePoints( dstSurface, dstFormat, dstPitch, dstRect, center, offset, dstPixel );
	}

	m_Surface->unlock();

	return true;
}

bool Draw::setAlpha( Color key )
{
	ColorFormat::Ref dstFormat = ColorFormat::allocateFormat( m_Surface->colorFormat() );
	if ( dstFormat->alphaMask() == 0x0 )
		return false;		// no alpha channel in surface

	RectInt srcRect( m_Surface->rectangle() );
	int pixelSize = dstFormat->byteSize();

	byte * dstSurface = m_Surface->lock();
	if (! dstSurface )
	{
		TRACE( "Draw::setAlpha() - Failed to lock surface" );
		return false;
	}
	for(int line = srcRect.top; line <= srcRect.bottom;line++)
	{
		byte * dst = dstSurface;
		dstSurface += m_Surface->pitch();

		for(int pixel = srcRect.left;pixel <= srcRect.right;pixel++)
		{
			// get the color from the surface
			Color color( dstFormat->getColor( dst ) );
			// modify it's alpha channel
			if ( color.RGB() == key.RGB() )
				color.m_A = 0;
			else
				color.m_A = 255;

			// set the color in the surface
			dstFormat->setColor( dst, color );
			dst += pixelSize;
		}
	}

	m_Surface->unlock();

	return true;
}

// set the alpha for all pixels in the surface around the outline color, used to outline fonts so they appear
// with a black border around them
bool Draw::outline( Color outline, Color with )
{
	ColorFormat::Ref dstFormat = ColorFormat::allocateFormat( m_Surface->colorFormat() );
	if ( dstFormat->alphaMask() == 0x0 )
		return false;		// no alpha channel in surface

	dword outlinePixel = dstFormat->pixel( outline );
	dword withPixel = dstFormat->pixel( with );

	RectInt srcRect( m_Surface->rectangle() );
	int pixelSize = dstFormat->byteSize();

	dword dstPitch = m_Surface->pitch();

	byte * dstSurface = m_Surface->lock();
	if (! dstSurface )
	{
		TRACE( "Draw::outline() - Failed to lock surface" );
		return false;
	}

	for(int line = srcRect.top; line <= srcRect.bottom;line++)
	{
		byte * dst = dstSurface;
		dstSurface += dstPitch;

		for(int pixel = srcRect.left;pixel <= srcRect.right;pixel++)
		{
			// get the color from the surface
			dword dstPixel = dstFormat->getPixel( dst );
			// if the color is the outline color, then set the 4 surrounding pixels to the with color
			if ( dstPixel == outlinePixel )
			{
				if ( line > srcRect.top && pixel > srcRect.left && dstFormat->getPixel( dst - dstPitch - pixelSize ) != outlinePixel )	
					dstFormat->setPixel( dst - dstPitch - pixelSize, withPixel );	// top left
				if ( line > srcRect.top && dstFormat->getPixel( dst - dstPitch ) != outlinePixel )	
					dstFormat->setPixel( dst - dstPitch, withPixel );				// top
				if ( line > srcRect.top && pixel < srcRect.right && dstFormat->getPixel( dst - dstPitch + pixelSize ) != outlinePixel )	
					dstFormat->setPixel( dst - dstPitch + pixelSize, withPixel );	// top right
				if ( pixel < srcRect.right && dstFormat->getPixel( dst + pixelSize ) != outlinePixel )	
					dstFormat->setPixel( dst + pixelSize, withPixel );				// right
				if ( line < srcRect.bottom && pixel < srcRect.right && dstFormat->getPixel( dst + dstPitch + pixelSize ) != outlinePixel )	
					dstFormat->setPixel( dst + dstPitch + pixelSize, withPixel );	// bottom right
				if ( line < srcRect.bottom && dstFormat->getPixel( dst + dstPitch ) != outlinePixel )
					dstFormat->setPixel( dst + dstPitch, withPixel );				// bottom
				if ( line < srcRect.bottom && pixel > srcRect.left && dstFormat->getPixel( dst + dstPitch - pixelSize ) != outlinePixel )	
					dstFormat->setPixel( dst + dstPitch - pixelSize, withPixel );	// bottom left
				if ( pixel > srcRect.left && dstFormat->getPixel( dst - pixelSize ) != outlinePixel )
					dstFormat->setPixel( dst - pixelSize, withPixel );				// left
			}

			dst += pixelSize;
		}
	}

	m_Surface->unlock();

	return true;
}

//-------------------------------------------------------------------------------

// based on the colorformat create the appropriate draw object
Draw::Ref Draw::create( PrimitiveSurface * pSurface )
{
	ASSERT( pSurface );

	Draw::Ref drawObject = WidgetCast<Draw>( Factory::createNamedWidget( "Draw" ) );
	drawObject->setSurface( pSurface );

	return( drawObject );
}

//-------------------------------------------------------------------------------
// EOF









/*
	WindowText.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define GUI3D_DLL
#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "Display/PrimitiveLineStrip.h"
#include "Gui3d/WindowText.h"

#include <stdio.h>

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowText, NodeWindow );
REGISTER_FACTORY_KEY(  WindowText, 4094092924522604111 );

BEGIN_PROPERTY_LIST( WindowText, NodeWindow );
	ADD_PROPERTY( m_Text );
END_PROPERTY_LIST();

WindowText::WindowText()
{}

WindowText::WindowText( NodeWindow * pParent, const RectInt & window  )
{
	setName( pParent->name() );
	setWindowStyle( pParent->windowStyle() );
	setContext( pParent->context() );
	setWindow( window );

	// attach ourselves to the parent as a child window
	pParent->attachNode( this );
}

//----------------------------------------------------------------------------

void WindowText::onActivate()
{}

void WindowText::onDeactivate()
{
	// flush material and font caches
	m_CacheWindow.setInvalid();
	m_LineCache.release();
	m_ImageCache.release();
	m_ConnectCache.release();
}

//----------------------------------------------------------------------------

const RectFloat IMAGE_UV(0,0,1,1);

void WindowText::onRender( RenderContext & context, const RectInt & window )
{
	PROFILE_FUNCTION();
	if ( m_Text.length() > 0 )
	{
		updateCache( window );

		//PROFILE_START( "WindowText::onRender" );

		DisplayDevice * pDisplay = context.display();
		ASSERT( pDisplay );

		// get the clipping window from the context
		RectInt clipWindow( context.window() );

		// draw text
		m_LineCache.reset();
		while( m_LineCache.valid() )
		{
			TextLine draw = *m_LineCache;
			m_LineCache.next();

			if ( (draw.position.y + draw.font->size().height) < clipWindow.top )
				continue;
			if ( draw.position.y > clipWindow.bottom )
				continue;

			Color drawColor( draw.color.r, draw.color.g, draw.color.b, (u8)(m_Alpha * draw.color.a) );
			Font::push( pDisplay, draw.font, draw.position, draw.line, drawColor );

			//TRACE( CharString("WindowText::onRender, line = %s, drawColor = %d, %d, %d, %d", draw.line, drawColor.r, drawColor.g, drawColor.b, drawColor.a ) );
		}

		// draw images
		m_ImageCache.reset();
		while( m_ImageCache.valid() )
		{
			TextImage draw = *m_ImageCache;
			m_ImageCache.next();

			if ( draw.rect.bottom < clipWindow.top )
				continue;
			if ( draw.rect.top > clipWindow.bottom )
				continue;

			// push the image material
			Material::push( context, draw.material );
			// draw a window with the pushed material
			Color drawColor( draw.color.r, draw.color.g, draw.color.b, (u8)(m_Alpha * draw.color.a) );
			PrimitiveWindow::push( pDisplay, draw.rect, IMAGE_UV, drawColor );

		}

		// draw connectors
		m_ConnectCache.reset();
		while( m_ConnectCache.valid() )
		{
			Connect draw = *m_ConnectCache;
			m_ConnectCache.next();

			if ( draw.src.y < clipWindow.top )
				continue;
			if ( draw.src.y > clipWindow.bottom )
				continue;

			Color drawColor( draw.color.r, draw.color.g, draw.color.b, (u8)(m_Alpha * draw.color.a) );

			PrimitiveMaterial::push( pDisplay, PrimitiveMaterial::ALPHA );
			PrimitiveLineStripDTL::pushConnector( pDisplay, draw.src, draw.dst, drawColor );
		}

		//PROFILE_END();
	}
}

//----------------------------------------------------------------------------

const char *	TAG_WHITE = ";";

enum Command
{
	CMD_INVALID = -1,

	CMD_LARGE,				// large font
	CMD_SMALL,				// small font
	CMD_FONT,				// change the font
	CMD_DEFAULT_FONT,		// restore the default font
	CMD_COLOR,				// set the current color
	CMD_HIGH_COLOR,			// high color
	CMD_DEFAULT_COLOR,		// default color
	CMD_IMAGE,				// display an image
	CMD_X,					// set the x position
	CMD_Y,					// set the y position
	CMD_CONNECT,			// display a connector from the current position to a window
};

struct CommandLookup
{
	const char *	pToken;
	Command			command;
};

static CommandLookup COMMAND_TABLE[] = 
{
	{ "large",		CMD_LARGE },
	{ "small",		CMD_SMALL },
	{ "font",		CMD_FONT },
	{ "/font",		CMD_DEFAULT_FONT },
	{ "/large",		CMD_DEFAULT_FONT },
	{ "/small",		CMD_DEFAULT_FONT },
	{ "color",		CMD_COLOR },
	{ "/color",		CMD_DEFAULT_COLOR },
	{ "bold",		CMD_HIGH_COLOR },
	{ "/bold",		CMD_DEFAULT_COLOR },
	{ "image",		CMD_IMAGE },
	{ "x",			CMD_X },
	{ "y",			CMD_Y },
	{ "connect",	CMD_CONNECT },

	{ NULL,			CMD_INVALID }		// leave at end
};


void WindowText::updateCache( const RectInt & window )
{
	PROFILE_FUNCTION();
	if ( m_CacheWindow == window )
		return;		// cache is up to date
	if ( m_CacheWindow.size() == window.size() )
	{
		// window size has not changed, update the positions and continue
		PointInt delta( window.upperLeft() - m_CacheWindow.upperLeft() );

		m_LineCache.reset();
		while( m_LineCache.valid() )
		{
			(*m_LineCache).position += delta;
			m_LineCache.next();
		}

		m_ImageCache.reset();
		while( m_ImageCache.valid() )
		{
			(*m_ImageCache).rect += delta;
			m_ImageCache.next();
		}
		
		m_ConnectCache.reset();
		while( m_ConnectCache.valid() )
		{
			(*m_ConnectCache).src += delta;
			m_ConnectCache.next();
		}

		// window psoition change has been applied, cache information is up to date now
		m_CacheWindow = window;
		return;
	}

	// the size of the window has changed or the text has been changed, rebuild the entire cache
	m_LineCache.release();
	m_ImageCache.release();
	m_ConnectCache.release();
	m_CacheWindow = window;

	RectInt		textWindow( window );

	static char line[ 1024 ];
	int			lineChar = 0;

	PointInt	linePosition( textWindow.m_Left, textWindow.m_Top );
	if ( (style() & USER2) != 0 )
		linePosition += PointInt( windowStyle()->borderSize(), windowStyle()->borderSize() );

	Font::Link	font = windowStyle()->font();
	SizeInt		fontSize( font->size() );

	Color		color = windowStyle()->color();

	int			lineWidth = 0;
	int			lineHeight = fontSize.height;

	SizeInt		windowSize( textWindow.size() );

	bool		bFlushLine = false;
	bool		bNextLine = false;
	bool		bDone = false;

	bool		bParseTag = false;
	CharString	sTag;

	// copy the string to display into a mutable new buffer
	CharString text( m_Text );

	// parse the text in the buffer
	char * pText = text.buffer();
	while( !bDone )
	{
		int character = *pText++;
		while( true )
		{
			if ( character == 0 )
				break;
			if ( character == '<' && *pText != '<' )
				break;
			else if ( character == '<' )
				++pText;			// if character was an escaped <<. then skip the 2nd << as well when drawing..
			if ( character == '\n' )
				break;

			// add character to output line
			line[ lineChar++ ] = character;
			lineWidth += font->characterWidth( character );

			// check the total line width
			if ( (linePosition.m_X + lineWidth) > textWindow.m_Right )
				break;

			// get next character
			character = *pText++;
		}

		switch( character )
		{
		case 0:		// null terminator
			bFlushLine = true;
			bDone = true;
			break;
		case '\n':	// newline
			bNextLine = true;
			break;
		case '<':	// begin tag
			{
				char * pTagBegin = pText;
				char * pTagEnd = strchr( pTagBegin, '>' );

				if ( pTagEnd != NULL )
				{
					*pTagEnd = 0;

					sTag = pTagBegin;
					bParseTag = true;
					bFlushLine = true;

					pText = pTagEnd + 1;
				}
				else
				{
					bFlushLine = true;
					bDone = true;
				}
			}
			break;
		}

		bool bOverflow = (linePosition.m_X + lineWidth) > textWindow.m_Right;
		if ( bFlushLine || bNextLine || bOverflow )
		{
			line[ lineChar ] = 0;

			CharString wrapped;		// characters to be moved to the next line

			// handle word wrapping
			if ( bOverflow )
			{
				char * pLastWhite = strrchr( line, ' ' );
				if ( pLastWhite != NULL )
				{
					wrapped = pLastWhite + 1;
					// terminate the line at the last whitespace
					*pLastWhite = 0;
					// recalculate the lineWidth
					lineWidth = font->size( WideString( line ) ).width;
				}
				bNextLine = true;
			}

			// push the text into the cache
			TextLine draw;
			draw.position = linePosition;
			draw.font = font;
			draw.line = line;
			draw.color = color;
			m_LineCache.push( draw );

			// advance the line position
			linePosition.x += lineWidth;

			if ( bNextLine )
			{
				linePosition.m_X = textWindow.m_Left;
				if ( (style() & USER2) != 0 )
					linePosition.m_X += windowStyle()->borderSize();
				linePosition.m_Y += lineHeight;

				// update the line height, it may have been modified by a font change or an embedded image
				lineHeight = fontSize.height;
			}

			// copy wrapped text over to the next line
			strcpy<char>( line, wrapped );

			lineChar = strlen( line );
			lineWidth = font->size( WideString( line ) ).width;
			bFlushLine = bNextLine = false;
		}

		if ( bParseTag )
		{
			CharString cmd, arg;
			while ( sTag.tokenize( cmd, TAG_WHITE ) )
			{
				cmd.trim();
				
				Command command = CMD_INVALID;

				int k = 0;
				while( COMMAND_TABLE[k].pToken != NULL )
				{
					if ( _strnicmp( COMMAND_TABLE[k].pToken, cmd, cmd.length()) == 0 )
					{
						command = COMMAND_TABLE[k].command;
						break;
					}
					k++;
				}

				switch( command )
				{
				case CMD_LARGE:
					font = windowStyle()->largeFont();
					fontSize = font->size();
					lineHeight = Max( lineHeight, fontSize.height );
					break;
				case CMD_SMALL:
					font = windowStyle()->smallFont();
					fontSize = font->size();
					lineHeight = Max( lineHeight, fontSize.height );
					break;
				case CMD_FONT:
					if ( sTag.tokenize( arg, TAG_WHITE ) )	
					{
						arg.trim();

						Font::Link newFont = WidgetCast<Font>( resource( CharString( arg ) ) );
						if ( newFont.valid() )
						{
							font = newFont;
							fontSize = font->size();

							lineHeight = Max( lineHeight, fontSize.height );
						}
					}
					break;
				case CMD_DEFAULT_FONT:
					font = windowStyle()->font();
					fontSize = font->size();
					lineHeight = Max( lineHeight, fontSize.height );
					break;
				case CMD_COLOR:
					if ( sTag.tokenize( arg, TAG_WHITE ) )	
					{
						arg.trim();

						color = Color( strtoul( arg, NULL, 16 ) );
						
						if ( arg.length() < 8 )
							color.a = 255;

						//TRACE( CharString("CMD_COLOR, arg = %s, color = 0x%x", arg, color.RGBA() ) );
					}
					break;
				case CMD_HIGH_COLOR:
					color = windowStyle()->highColor();
					break;
				case CMD_DEFAULT_COLOR:
					color = windowStyle()->color();
					break;
				case CMD_IMAGE:
					if ( sTag.tokenize( arg, TAG_WHITE ) )
					{
						arg.trim();

						Resource * pResource = resource( CharString( arg ) );
						if ( pResource != NULL )
						{
							Material * pMaterial = WidgetCast<Material>( pResource );
							if ( pMaterial == NULL )
							{
								Image * pImage = WidgetCast<Image>( pResource );
								if ( pImage != NULL )
								{
									pMaterial = new Material( pImage, PrimitiveMaterial::ADDITIVE );
									// add the created material as a resource, so we don't keep making it each time..
									// this prevents the game from pausing while the profile window is up.
									addResource( arg, pMaterial );
								}
							}

							if ( pMaterial != NULL )
							{
								SizeInt imageSize( fontSize );
								int nDiffuse = pMaterial->findTexture( PrimitiveSurface::DIFFUSE );
								if ( nDiffuse >= 0 )
									imageSize = pMaterial->texture(nDiffuse).m_pImage->size();

								if ( sTag.tokenize( arg, TAG_WHITE ) )	// width
								{
									arg.trim();
									imageSize.m_Width = strtoul( arg, NULL, 10 );
								}
								if ( sTag.tokenize( arg, TAG_WHITE ) )	// height
								{
									arg.trim();
									imageSize.m_Height = strtoul( arg, NULL, 10 );
								}
								if ( sTag.tokenize( arg, TAG_WHITE ) )	// color
								{
									arg.trim();
									color = Color( strtoul( arg, NULL, 16 ) );
								}

								if ( (linePosition.m_X + lineWidth + imageSize.width ) > textWindow.m_Right )
								{
									linePosition.m_X = textWindow.m_Left;
									linePosition.m_Y += lineHeight;
								}

								TextImage draw;
								draw.material = pMaterial;
								draw.rect = RectInt( linePosition, imageSize );
								draw.color = color;
								m_ImageCache.push( draw );

								linePosition.m_X += imageSize.m_Width;
								lineHeight = Max( lineHeight, imageSize.m_Height );
							}
						}
					}
					break;
				case CMD_X:
					if ( sTag.tokenize( arg, TAG_WHITE ) )
					{
						arg.trim();
						linePosition.m_X = window.m_Left + strtoul( arg, NULL, 10 );;
					}
					break;
				case CMD_Y:
					if ( sTag.tokenize( arg, TAG_WHITE ) )
					{
						arg.trim();
						linePosition.m_Y = window.m_Top + strtoul( arg, NULL, 10 );;
						lineHeight = fontSize.height;
					}
					break;
				case CMD_CONNECT:
					if ( sTag.tokenize( arg, TAG_WHITE ) )
					{
						arg.trim();

						// get the root parent
						BaseNode * pParent = this;
						while( pParent != NULL && pParent->parent() != NULL )
							pParent = pParent->parent();

						NodeWindow * pWindow = WidgetCast<NodeWindow>( pParent->findNode( arg ) );
						if ( pWindow != NULL )
						{
							Connect draw;
							draw.src = linePosition;
							draw.dst = 	pWindow->windowToScreen( pWindow->localWindow().center() );
							draw.color = color;

							m_ConnectCache.push( draw );

							linePosition.x += 8;
						}
					}
					break;
				}
			}
		}
	}
	
	linePosition.m_Y += lineHeight;
	if ( (style() & USER2) != 0 )
		linePosition.m_Y += windowStyle()->borderSize();
	else
		linePosition.m_Y += windowStyle()->borderSize() * 2;

	// check the window bottom, make it large if need to hold the current line
	if ( linePosition.m_Y != textWindow.m_Bottom && (style() & USER1) == 0 )
	{
		// expand/contract the window bottom
		textWindow.m_Bottom = linePosition.m_Y;
		windowSize = textWindow.size();
		
		// resize this window
		m_Window = RectInt( m_Window.upperLeft(), windowSize );
	}
}

//----------------------------------------------------------------------------
// EOF

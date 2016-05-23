/*
	WindowStyle.h

	Window style class, NodeWindow objects link to this object to get the resources for their style
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOW_STYLE_H
#define WINDOW_STYLE_H

#include "Audio/Sound.h"
#include "Draw/Font.h"
#include "Render3D/Material.h"
#include "Resource/Resource.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowStyle : public Resource
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef ResourceLink< WindowStyle >		Link;
	typedef Reference< WindowStyle >		Ref;

	// Construction
	WindowStyle();
	WindowStyle( const WindowStyle & copy );

	// Widget interface
	bool				read( const InStream & input );

	// Accessors
	Font *				font() const;
	Font *				largeFont() const;
	Font *				smallFont() const;

	Color				color() const;
	Color				highColor() const;
	Color				borderColor() const;
	Color				backColor() const;
	Color				scrollBarColor() const;
	Color				scrollTabColor() const;

	Material *			background() const;
	int					borderSize() const;
	int					scrollSize() const;
	Sound *				hoverSound() const;
	Sound *				selectSound() const;

	// Mutators
	void				setFont( Font::Link pFont );
	void				setLargeFont( Font::Link pFont );
	void				setSmallFont( Font::Link pFont );

	void				setColor( Color color );
	void				setHighColor( Color color );
	void				setBorderColor( Color color );
	void				setBackColor( Color color );
	void				setScrollColor( Color bar, Color tab );

	void				setBackground( Material::Link pMaterial );
	void				setBorderSize( int size );
	void				setScrollSize( int size );
	void				setHoverSound( Sound::Link pSound );
	void				setSelectSound( Sound::Link pSound );

private:
	// Data
	Font::Link			m_Font;
	Font::Link			m_LargeFont;
	Font::Link			m_SmallFont;

	Color				m_Color;
	Color				m_HighColor;
	Color				m_BorderColor;
	Color				m_BackColor;
	Color				m_ScrollBarColor;
	Color				m_ScrollTabColor;

	Material::Link		m_Background;
	int					m_BorderSize;
	int					m_ScrollSize;
	Sound::Link			m_HoverSound;
	Sound::Link			m_SelectSound;
};

//----------------------------------------------------------------------------

inline Font * WindowStyle::font() const
{
	return( m_Font );
}

inline Font * WindowStyle::largeFont() const
{
	return m_LargeFont;
}

inline Font * WindowStyle::smallFont() const
{
	return m_SmallFont;
}

inline Color WindowStyle::color() const
{
	return( m_Color );
}

inline Color WindowStyle::highColor() const
{
	return( m_HighColor );
}

inline Color WindowStyle::borderColor() const
{
	return( m_BorderColor );
}

inline Color WindowStyle::backColor() const
{
	return( m_BackColor );
}

inline Color WindowStyle::scrollBarColor() const
{
	return( m_ScrollBarColor );
}

inline Color WindowStyle::scrollTabColor() const
{
	return( m_ScrollTabColor );
}

inline Material * WindowStyle::background() const
{
	return( m_Background );
}

inline int WindowStyle::borderSize() const
{
	return( m_BorderSize );
}

inline int WindowStyle::scrollSize() const
{
	return( m_ScrollSize );
}

inline Sound * WindowStyle::hoverSound() const
{
	return m_HoverSound.pointer( true, false );
}

inline Sound * WindowStyle::selectSound() const
{
	return m_SelectSound.pointer( true, false );
}

//----------------------------------------------------------------------------

inline void WindowStyle::setFont( Font::Link pFont )
{
	m_Font = pFont;
}

inline void WindowStyle::setLargeFont( Font::Link pFont )
{
	m_LargeFont = pFont;
}

inline void WindowStyle::setSmallFont( Font::Link pFont )
{
	m_SmallFont = pFont;
}

inline void WindowStyle::setColor( Color color )
{
	m_Color = color;
}

inline void WindowStyle::setHighColor( Color color )
{
	m_HighColor = color;
}

inline void WindowStyle::setBorderColor( Color color )
{
	m_BorderColor = color;
}

inline void WindowStyle::setBackColor( Color color )
{
	m_BackColor = color;
}

inline void WindowStyle::setScrollColor( Color bar, Color tab )
{
	m_ScrollBarColor = bar;
	m_ScrollTabColor = tab;
}

inline void WindowStyle::setBackground( Material::Link pMaterial )
{
	m_Background = pMaterial;
}

inline void WindowStyle::setBorderSize( int size )
{
	m_BorderSize = size;
}

inline void WindowStyle::setScrollSize( int size )
{
	m_ScrollSize = size;
}

inline void WindowStyle::setHoverSound( Sound::Link pSound )
{
	m_HoverSound = pSound;
}

inline void WindowStyle::setSelectSound( Sound::Link pSound )
{
	m_SelectSound = pSound;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

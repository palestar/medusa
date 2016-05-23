/*
	Color.h

	Color data type
	(c)2005 Palestar, Richard Lyle
*/

#ifndef COLOR_H
#define COLOR_H

#include "Types.h"
#include "Limits.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

// windows tends to use RGB as a macro, undef this so it doesn't mess up our class declaration -rol
#undef RGB

struct _Color
{
	union {
		u8		m_B, b, blue;
	};
	union {
		u8		m_G, g, green;
	};
	union {
		u8		m_R, r, red;
	};
	union {
		u8		m_A, a, alpha;
	};
};

class Color : public _Color
{
public:
    // Creators
	Color();
	Color(const Color &color);
	Color(u8 r, u8 g, u8 b, u8 a = 0 );
	Color( u32 RGBA );
    
    // Accessors
	operator			u32() const;
    u32					RGBA() const;		
    u32					BGRA() const;		
    u32					RGB() const;			

	bool				operator==(const Color &compare ) const;	// compare colors
	bool				operator!=(const Color &compare ) const;

	int					distance( const Color &from ) const;		// get color distance
	int					magnitude() const;							// get color magnitude
	int					magnitudeNoAlpha() const;					// get color magnitude, exclude alpha

    // Mutators
    Color &				operator+=(const Color & Rhs);				// addition
	Color &				operator-=(const Color & Rhs);				// subtract 
	Color &				operator%=(const Color & Rhs);				// average color
	Color &				operator*=(float scalar);

	Color &				iterpolate( const Color & dst, float t );	// interpole this color towards dst
	Color &				greyscale( float fScale = 1.0f );			// convert into a grey scale
};

//----------------------------------------------------------------------------

inline Color::Color()
{
	m_R = m_G = m_B = m_A = 0;
}

inline Color::Color(const Color &color)
{
	*this = color;
}

inline Color::Color(u8 r, u8 g, u8 b, u8 a /*= 0*/ )
{
	m_R = r;
	m_G = g;
	m_B = b;
	m_A = a;
}

inline Color::Color(u32 RGBA )
{
	m_A = RGBA >> 24;
	m_B = (RGBA >> 16) & 0xff;
	m_G = (RGBA >> 8) & 0xff;
	m_R = RGBA & 0xff;
}

//----------------------------------------------------------------------------

inline Color::operator u32() const
{
	return RGBA();
}

inline u32 Color::RGBA() const
{ 
	return (m_A << 24) | (m_B << 16) | (m_G << 8) | m_R; 
}

inline u32 Color::BGRA() const
{ 
	return (m_A << 24) | (m_R << 16) | (m_G << 8) | m_B; 
}

inline u32 Color::RGB() const
{ 
	return (m_B << 16) | (m_G << 8) | m_R; 
}

inline bool Color::operator==(const Color &compare ) const
{
	return( m_R == compare.m_R && m_G == compare.m_G && m_B == compare.m_B && m_A == compare.m_A );
}

inline bool Color::operator!=(const Color &compare ) const
{
	return( ! operator==( compare ) );
}

#define SQUARE(value)		((value) * (value))

inline int Color::distance( const Color &from ) const
{
	return( SQUARE(m_R - from.m_R) + SQUARE(m_G - from.m_G) + SQUARE(m_B - from.m_B) + SQUARE(m_A - from.m_A) );
}

inline int Color::magnitude() const
{
	return( (m_R + m_G + m_B + m_A) / 4 );
}

inline int Color::magnitudeNoAlpha() const
{
	return( (m_R + m_G + m_B) / 3 );
}

//----------------------------------------------------------------------------

inline Color &Color::operator+=(const Color & Rhs)
{
	m_R = ClampMax( m_R + Rhs.m_R, 255);
	m_G = ClampMax( m_G + Rhs.m_G, 255);
	m_B = ClampMax( m_B + Rhs.m_B, 255);
	m_A = ClampMax( m_A + Rhs.m_A, 255);
	return *this;
}

inline Color &Color::operator-=(const Color & Rhs)
{
	m_R = ClampMin( m_R - Rhs.m_R, 0 );
	m_G = ClampMin( m_G - Rhs.m_G, 0 );
	m_B = ClampMin( m_B - Rhs.m_B, 0 );
	m_A = ClampMin( m_A - Rhs.m_A, 0 );
	return *this;
}

inline Color &Color::operator%=(const Color & Rhs)
{
	m_R = (m_R + Rhs.m_R) >> 1;
	m_G = (m_G + Rhs.m_G) >> 1;
	m_B = (m_B + Rhs.m_B) >> 1;
	m_A = (m_A + Rhs.m_A) >> 1;
	return *this;
}

inline Color & Color::operator*=(float scalar)
{
	m_R = (u8)Clamp( m_R * scalar, 0.f, 255.f);
	m_G = (u8)Clamp( m_G * scalar, 0.f, 255.f);
	m_B = (u8)Clamp( m_B * scalar, 0.f, 255.f);
	m_A = (u8)Clamp( m_A * scalar, 0.f, 255.f);
	return *this;
}

inline Color & Color::iterpolate( const Color & dst, float t )
{
	m_R = (u8)(m_R + (t * (dst.m_R - m_R)));
	m_G = (u8)(m_G + (t * (dst.m_G - m_G)));
	m_B = (u8)(m_B + (t * (dst.m_B - m_B)));
	m_A = (u8)(m_A + (t * (dst.m_A - m_A)));
	return *this;
}

inline Color & Color::greyscale( float fScale /*= 1.0f*/ )
{
	m_R = m_G = m_B = (u8)Clamp( magnitudeNoAlpha() * fScale, 0.0f, 255.0f );
	return *this;
}

//----------------------------------------------------------------------------

inline Color operator+(const Color &Lhs, const Color &Rhs)
{
    Color lhs(Lhs);
    return lhs += Rhs;
}

inline Color operator-(const Color &Lhs, const Color &Rhs)
{
    Color lhs(Lhs);
    return lhs -= Rhs;
}

inline Color operator%(const Color &Lhs, const Color &Rhs)
{
    Color lhs(Lhs);
    return lhs %= Rhs;
}

inline Color operator*(const Color &Lhs, float scalar )
{
	Color lhs(Lhs);
	return lhs *= scalar;
}

//----------------------------------------------------------------------------

// Color Constants
const Color WHITE			(	255,	255,	255,	255);
const Color BLACK			(	0,		0,		0,		255);
const Color BLUE			(	0,		0,		255,	255);
const Color RED				(	255,	0,		0,		255);
const Color GREEN			(	0,		255,	0,		255);
const Color DARK_BLUE		(	0,		0,		128,	255);
const Color DARK_RED		(	128,	0,		0,		255);
const Color DARK_GREEN		(	0,		128,	0,		255);
const Color YELLOW			(	255,	255,	0,		255);
const Color DARK_YELLOW		(	128,	128,	0,		255);
const Color GREY			(	128,	128,	128,	255);
const Color LIGHT_GREY		(	192,	192,	192,	255);
const Color MAGENTA			(	255,	0,		255,	255);
const Color CYAN			(	0,		255,	255,	255);
const Color ORANGE			(	255,	128,	0,		255);
const Color PINK			(	255,	0,		128,	255);
const Color CORAL			(	255,	127,	80,		255);
const Color GOLD			(	255,	215,	0,		255);
const Color TAN				(	210,	180,	140,	255);
const Color KHAKI			(	240,	230,	140,	255);
const Color LAVENDER		(	230,	230,	250,	255);
const Color HOTPINK			(	255,	105,	180,	255);
const Color PLUM			(	221,	160,	221,	255);
const Color AQUAMARINE		(	127,	255,	121,	255);
const Color SKYBLUE			(	135,	206,	235,	255);
const Color BRIGHT_PURPLE	(	128,	0,		128,	255);
const Color TEAL			(	0,		128,	128,	255);

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

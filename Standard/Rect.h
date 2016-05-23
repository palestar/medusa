/*
	Rect.h

	Rect template class
	(c)2005 Palestar, Richard Lyle
*/

#ifndef RECT_H
#define RECT_H

#include "Point.h"
#include "Size.h"

//----------------------------------------------------------------------------

template<class T> 
class Rect 
{
public:
	// construction
	Rect();
	Rect(const Point<T> &point, const Size<T> &size);
	Rect(T X, T Y, Size<T> size);
	Rect(T left, T top, T right, T bottom);

	// accessors
	bool		valid() const;							// is this rectangle valid
	T			width() const;							// right - left + 1
	T			height() const;							// bottom - top + 1
	Size<T>		size() const;							// get the size of the rectangle

	bool		inRect( int x, int y ) const;
	bool		inRect( const Point<T> & point ) const;
	bool		inRect( const Rect<T> & rect ) const;	

	Point<T>	center() const;
	Point<T>	upperLeft() const;						// corners of this rectangle
	Point<T>	upperRight() const;
	Point<T>	lowerLeft() const;
	Point<T>	lowerRight() const;

	bool		operator==(const Rect<T>& Rhs);
	bool		operator!=(const Rect<T>& Rhs);
	Rect<T>		scale(T scaleWidth,T scaleHeight) const;	
	Rect<T>		operator+(const Point<T> &Rhs) const;
	Rect<T>		operator-(const Point<T> &Rhs) const;

	// mutators
	Rect<T>&	setInvalid();
    void		setLeft(const T Left);
    void		setTop(const T Top);
    void		setRight(const T Right);
    void		setBottom(const T Bottom);
    
	void		setWidth(const T Width);				// expands to the right
	void		setHeight(const T Height);				// expands to the bottom
	
	void		set(T left,T top,T right,T bottom);
	void		set(T X, T Y, const Size<T> & size);
	void		set(const Point<T> &point,const Size<T> &size);

	Rect<T>&	inset( T inset );						// >0 inset or <0 outset the rectangle

	Rect<T>&	operator=(const Rect<T> &Rhs);
	Rect<T>&	operator+=(const Point<T> & Rhs);
	Rect<T>&	operator-=(const Point<T>& Rhs);
	Rect<T>&	operator&=(const Rect<T>& Rhs);			// Intersection
	Rect<T>&	operator|=(const Rect<T>& Rhs);			// Union
	Rect<T>&	operator*=(const T Multiplier);
	Rect<T>&	operator/=(const T Divisor);
	Rect<T>		operator&(const Rect<T>& Rhs);
	Rect<T>		operator|(const Rect<T>& Rhs);

	// Data
	union {
		T	m_Left, left;
	};
	union {
		T	m_Top, top;
	};
	union {
		T	m_Right, right;
	};
	union {
		T	m_Bottom, bottom;
	};

};

//-------------------------------------------------------------------------------

template<class T>
inline Rect<T>::Rect()
{
	set(0,0,0,0);
}

template<class T>
inline Rect<T>::Rect(const Point<T> &point, const Size<T> &size)
{
	set(point,size);
}

template<class T>
inline Rect<T>::Rect(T X, T Y, Size<T> size)
{
	set(X,Y,size);
}

template<class T>
inline Rect<T>::Rect(T left, T top, T right, T bottom)
{
	set(left,top,right,bottom);
}

//-------------------------------------------------------------------------------

template<class T>
inline bool Rect<T>::valid() const	
{ 
	return( (m_Left <= m_Right) && (m_Top <= m_Bottom) ); 
}

template<class T>
inline T Rect<T>::width() const		
{ 
	return( (m_Right - m_Left) + 1); 
}

template<class T>
inline T Rect<T>::height() const		
{ 
	return( (m_Bottom - m_Top) + 1); 
}

template<class T>
inline Size<T> Rect<T>::size() const		
{ 
	return( Size<T>( width(), height() ) ); 
}

template<class T>
inline bool Rect<T>::inRect( int x, int y ) const
{
	return( x >= m_Left && x <= m_Right && y >= m_Top && y <= m_Bottom );
}

template<class T>
inline bool Rect<T>::inRect( const Point<T> & point ) const
{
	return( point.x >= m_Left 
		&& point.x <= m_Right 
		&& point.y >= m_Top 
		&& point.y <= m_Bottom );
}

template<class T>
inline bool Rect<T>::inRect( const Rect<T> & rect ) const
{
	return( m_Left <= rect.m_Left 
		&& m_Right >= rect.m_Right 
		&& m_Top <= rect.m_Top 
		&& m_Bottom >= rect.m_Bottom );
}

template<class T>
inline Point<T>	Rect<T>::center() const
{
	return(Point<T>( (m_Left + m_Right) / 2, (m_Top + m_Bottom) / 2 ) );
}

template<class T>
inline Point<T>	Rect<T>::upperLeft() const
{
	return(Point<T>( m_Left, m_Top) );
}

template<class T>
inline Point<T> Rect<T>::upperRight() const
{
	return(Point<T>( m_Right, m_Top) );
}

template<class T>
inline Point<T> Rect<T>::lowerLeft() const
{
	return(Point<T>( m_Left, m_Bottom ) );
}

template<class T>
inline Point<T> Rect<T>::lowerRight() const
{
	return(Point<T>( m_Right, m_Bottom) );
}

template<class T>
inline Rect<T> Rect<T>::scale(T scaleWidth,T scaleHeight) const	
{
	Rect<T> result( *this );
	result.m_Left *= scaleWidth;
	result.m_Right *= scaleWidth;
	result.m_Top *= scaleHeight;
	result.m_Bottom *= scaleHeight;

	return(result);
}

template<class T>
inline bool Rect<T>::operator==(const Rect<T>& Rhs)
{
	return(m_Left == Rhs.m_Left && m_Top == Rhs.m_Top && 
			m_Right == Rhs.m_Right && m_Bottom == Rhs.m_Bottom);
}		

template<class T>
inline bool Rect<T>::operator!=(const Rect<T>& Rhs)
{
	return(!(*this == Rhs));
}		

//-------------------------------------------------------------------------------

template<class T>
inline Rect<T>& Rect<T>::setInvalid()				
{ 
	set( 1,1,-1,-1 ); 
	return( *this );
}

template<class T>
inline void Rect<T>::setLeft(const T Left)		
{ 
	m_Left = Left; 
}

template<class T>
inline void Rect<T>::setTop(const T Top)			
{ 
	m_Top = Top; 
}

template<class T>
inline void Rect<T>::setRight(const T Right)		
{ 
	m_Right = Right; 
}

template<class T>
inline void Rect<T>::setBottom(const T Bottom)	
{ 
	m_Bottom = Bottom; 
}

template<class T>
inline void Rect<T>::setWidth(const T Width)		
{ 
	m_Right = m_Left + (Width - 1); 
}

template<class T>
inline void Rect<T>::setHeight(const T Height)	
{ 
	m_Bottom = m_Top + (Height - 1); 
}

template<class T>
inline void Rect<T>::set(T left,T top,T right,T bottom)
{
	m_Left = left;
	m_Top = top;
	m_Right = right;
	m_Bottom = bottom;
}

template<class T>
inline void Rect<T>::set(T X, T Y, const Size<T> & size)
{
	m_Left = X;
	m_Top = Y;
	m_Right = X + size.width - 1;
	m_Bottom = Y + size.height - 1;
}

template<class T>
inline void Rect<T>::set(const Point<T> &point,const Size<T> &size)
{
	m_Left = point.x;
	m_Top = point.y;
	m_Right = m_Left + size.width - 1;
	m_Bottom = m_Top + size.height - 1;
}

template<class T>
inline Rect<T>& Rect<T>::inset( T inset )
{
	m_Left += inset;
	m_Top += inset;
	m_Right -= inset;
	m_Bottom -= inset;

	return *this;
}

template<class T>
inline Rect<T>& Rect<T>::operator=(const Rect<T> &Rhs)
{
	m_Left = Rhs.m_Left;
	m_Top = Rhs.m_Top;
	m_Right = Rhs.m_Right;
	m_Bottom = Rhs.m_Bottom;
	
	return(*this);
}

template<class T>
inline Rect<T>& Rect<T>::operator+=(const Point<T> & Rhs)
{
	m_Left += Rhs.x;
	m_Top += Rhs.y;
	m_Right += Rhs.x;
	m_Bottom += Rhs.y;

	return(*this);
}

template<class T>
inline Rect<T>& Rect<T>::operator-=(const Point<T>& Rhs)
{
	m_Left -= Rhs.x;
	m_Top -= Rhs.y;
	m_Right -= Rhs.x;
	m_Bottom -= Rhs.y;

	return(*this);
}

template<class T>
inline Rect<T> Rect<T>::operator+(const Point<T> &Rhs) const
{
	Rect result = *this;
	result += Rhs;

	return( result );
}

template<class T>
inline Rect<T> Rect<T>::operator-(const Point<T> &Rhs) const
{
	Rect result = *this;
	result -= Rhs;

	return( result );
}

template<class T>
inline Rect<T>& Rect<T>::operator&=(const Rect<T>& Rhs)	// intersect with Rhs
{
	if(m_Left < Rhs.m_Left)
		m_Left = Rhs.m_Left;
	if(m_Top < Rhs.m_Top)
		m_Top = Rhs.m_Top;
	if(m_Right > Rhs.m_Right)
		m_Right = Rhs.m_Right;
	if(m_Bottom > Rhs.m_Bottom)
		m_Bottom = Rhs.m_Bottom;

	return(*this);
}

template<class T>
inline Rect<T>& Rect<T>::operator|=(const Rect<T>& Rhs)	// union with Rhs
{
	if(m_Left > Rhs.m_Left)
		m_Left = Rhs.m_Left;
	if(m_Top > Rhs.m_Top)
		m_Top = Rhs.m_Top;
	if(m_Right < Rhs.m_Right)
		m_Right = Rhs.m_Right;
	if(m_Bottom < Rhs.m_Bottom)
		m_Bottom = Rhs.m_Bottom;

	return(*this);
}

template<class T>
inline Rect<T>& Rect<T>::operator*=(const T Multiplier)
{
	m_Left *= Multiplier;
	m_Top *= Multiplier;
	m_Right *= Multiplier;
	m_Bottom *= Multiplier;
	return(*this);
}

template<class T>
inline Rect<T>& Rect<T>::operator/=(const T Divisor)
{
	m_Left /= Divisor;
	m_Top /= Divisor;
	m_Right /= Divisor;
	m_Bottom /= Divisor;
	return(*this);
}

template<class T>
inline Rect<T> Rect<T>::operator&(const Rect<T>& Rhs)
{
	Rect<T>	result(*this);
	result &= Rhs;

	return(result);
}

template<class T>
inline Rect<T> Rect<T>::operator|(const Rect<T>& Rhs)
{
	Rect<T>	result(*this);
	result |= Rhs;

	return(result);
}

//-------------------------------------------------------------------------------

typedef Rect<int>		RectInt;
typedef Rect<float>		RectFloat;



#endif


//----------------------------------------------------------------------------
// EOF


/*
	Point.h

	Point template class
	(c)2005 Palestar, Richard Lyle
*/

#ifndef POINT_H
#define POINT_H

#include <math.h>

//----------------------------------------------------------------------------

template<class T> 
class Point
{
public:
	// Types
	enum				// returned by classify
	{
		LEFT,
		RIGHT,
		BEYOND,
		BEHIND,
		BETWEEN,
		ORIGIN,
		DESTINATION
	};

	// Construction
	Point();
	Point(T x, T y);

	// Accessors
	Point<T>		scale( T scaleX, T scaleY) const;		// create a scaled point from this point
	bool			operator==(const Point Rhs) const;		// equal operator
	bool			operator!=(const Point Rhs) const;		// not-equal operator
	int				operator<(const Point & p ) const;
	int				operator>(const Point & p ) const;

	int				classify( const Point & p0, 
						const Point & p1 ) const;			// classify this point to line p0-p1
	T				length() const;							// get the mangitude of this point

	// Mutators
	void			set(T X, T Y);
	Point&			operator=(const Point& Src);
	Point&			operator+=(const Point& Src);
	Point&			operator-=(const Point& Src);
	Point&			operator*=(const Point& Scalar);
	Point&			operator/=(const Point& Divisor);
	Point&			operator*=(T Scalar);
	Point&			operator/=(T Divisor);

	// Data
	union {
		T		m_X, x;
	};
	union {
		T		m_Y, y;
	};
};

//----------------------------------------------------------------------------

template<class T>
inline Point<T>::Point()									
{}

template<class T>
inline Point<T>::Point(T x, T y)
{
	m_X = x;
	m_Y = y;
}

//-------------------------------------------------------------------------------

template<class T>
inline Point<T> Point<T>::scale( T scaleX, T scaleY) const
{
	Point<T>	result( *this );
	result.m_X *= scaleX;
	result.m_Y *= scaleY;

	return(result);
}

template<class T>
inline bool Point<T>::operator==(const Point Rhs) const
{
	return(m_X == Rhs.m_X && m_Y == Rhs.m_Y);
}

template<class T>
inline bool Point<T>::operator!=(const Point Rhs) const
{
	return(!(*this == Rhs));
}

template<class T>
inline int Point<T>::operator<(const Point & p ) const
{
	return( (x < p.x) || ((x == p.x) && (y < p.y)) );
}

template<class T>
inline int Point<T>::operator>(const Point & p ) const
{
	return( (x > p.x) || ((x == p.x) && (y > p.y)) );
}

template<class T>
inline int Point<T>::classify( const Point & p0, const Point & p1 ) const
{
	Point<T> p2 = *this;
	Point<T> a = p1 - p0;
	Point<T> b = p2 - p0;

	float sa = a.x * b.y - b.x * a.y;
	if ( sa > 0.0f )
		return LEFT;
	if ( sa < 0.0f )
		return RIGHT;
	if ( (a.x * b.x < 0.0f) || (a.y * b.y < 0.0f) )
		return BEHIND;
	if ( a.length() < b.length() )
		return BEYOND;
	if ( p0 == p2 )
		return ORIGIN;
	if ( p1 == p2 )
		return DESTINATION;

	return BETWEEN;
}

template<class T>
inline T Point<T>::length() const
{
	return (T)sqrtf( (float)(x*x + y*y) );
}

//-------------------------------------------------------------------------------

template<class T>
inline void Point<T>::set(T X, T Y)
{
	m_X = X;
	m_Y = Y;
}

template<class T>
inline Point<T>& Point<T>::operator=(const Point& Src)
{
	m_X = Src.m_X;
	m_Y = Src.m_Y;
	return(*this);
}

template<class T>
inline Point<T>& Point<T>::operator+=(const Point& Src)
{
	m_X += Src.m_X;
	m_Y += Src.m_Y;
	return(*this);
}

template<class T>
inline Point<T>& Point<T>::operator-=(const Point& Src)
{
	m_X -= Src.m_X;
	m_Y -= Src.m_Y;
	return(*this);
}

template<class T>
inline Point<T>& Point<T>::operator*=(const Point& Scalar)
{
	m_X *= Scalar.m_X;
	m_Y *= Scalar.m_Y;
	return(*this);
}

template<class T>
inline Point<T>& Point<T>::operator/=(const Point& Divisor)
{
	m_X /= Divisor.m_X;
	m_Y /= Divisor.m_Y;
	return(*this);
}

template<class T>
inline Point<T>& Point<T>::operator*=(T Scalar)
{
	m_X *= Scalar;
	m_Y *= Scalar;
	return(*this);
}

template<class T>
inline Point<T>& Point<T>::operator/=(T Divisor)
{
	m_X /= Divisor;
	m_Y /= Divisor;
	return(*this);
}

//-------------------------------------------------------------------------------

template<class T>
inline Point<T> operator+( const Point<T> &a,const Point<T> &b)
{
	Point<T> result( a );
	result += b;

	return( result );
}

template<class T>
inline Point<T> operator-( const Point<T> &a,const Point<T> &b)
{
	Point<T> result( a );
	result -= b;

	return( result );
}

//-------------------------------------------------------------------------------------------

typedef Point<int>		PointInt;
typedef Point<float>	PointFloat;

//----------------------------------------------------------------------------



#endif


//----------------------------------------------------------------------------
// EOF


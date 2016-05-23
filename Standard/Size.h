/*
	Size.h

	Size template class
	(c)2005 Palestar, Richard Lyle
*/

#ifndef SIZE_H
#define SIZE_H



//----------------------------------------------------------------------------

template<class T> 
class Size
{
public:
	// Construction
	Size(); 
	Size(T width,T height);

	// Accessors
	bool	operator==( const Size<T> &compare ) const;		// equal
	bool	operator!=( const Size<T> &compare ) const;		// not-equal
	bool	isP2() const;									// is size power of 2 

	// Mutators
	void	set(T Width, T Height);

	template<typename K>
	Size &	operator=( const Size<K> & assign )
	{
		m_Width = (T)assign.width;
		m_Height = (T)assign.height;
		return *this;
	}

	union {
		T		m_Width, width;
	};
	union {
		T		m_Height, height;
	};
};

//----------------------------------------------------------------------------

template<class T>
inline Size<T>::Size() 
{}

template<class T>
inline Size<T>::Size(T width,T height)
{
	m_Width = width;
	m_Height = height;
}

//----------------------------------------------------------------------------

template<class T>
inline bool Size<T>::operator==( const Size<T> &compare ) const		
{ 
	return( m_Width == compare.m_Width && m_Height == compare.m_Height );
}

template<class T>
inline bool Size<T>::operator!=( const Size<T> &compare ) const
{
	return( ! operator==( compare ) );
}

template<class T>
inline bool Size<T>::isP2() const
{
	return( (m_Height & (m_Height - 1)) == 0 && (m_Width & (m_Width - 1)) == 0 );
}

//----------------------------------------------------------------------------

template<class T>
inline void	Size<T>::set(T Width, T Height)	
{ 
	m_Width = Width; 
	m_Height = Height; 
}

//----------------------------------------------------------------------------

template<class T>
inline Size<T> operator+( const Size<T> &Lh, const Size<T> &Rh )
{
	return( Size<T>( Lh.m_Width + Rh.m_Width, Lh.m_Height + Rh.m_Height ) );
}

template<class T>
inline Size<T> operator-( const Size<T> &Lh, const Size<T> &Rh )
{
	return( Size<T>( Lh.m_Width - Rh.m_Width, Lh.m_Height - Rh.m_Height ) );
}

template<typename T,typename K>
inline Size<T> operator/( const Size<T> & lh, K scalar )
{
	return Size<T>( lh.width / scalar, lh.height / scalar );
}

template<typename T,typename K>
inline Size<T> operator*( const Size<T> & lh, K scalar )
{
	return Size<T>( lh.width * scalar, lh.height * scalar );
}

//-------------------------------------------------------------------------------

typedef Size<int>		SizeInt;
typedef Size<float>		SizeFloat;



#endif

//----------------------------------------------------------------------------
// EOF

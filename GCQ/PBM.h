/*
	PBM.h

	Portable Bitmap class, this class is used as a generic container for a bitmap
	(c)2000 Palestar Inc, Richard Lyle
*/

#ifndef PBM_H
#define PBM_H

#include "Standard/Color.h"



//----------------------------------------------------------------------------

class PBM
{
public:

	// Construction
				PBM();
				PBM( int width, int height, Color * pPixels );
	virtual		~PBM();

	// Accessors
	int			width() const;					// image width
	int			height() const;					// image height
	int			pixelCount() const;				// total number of pixels
	Color *		pixels() const;					// returns the buffer to the pixels

	// Mutators
	void		initialize( int width, int height, Color * pPixels );
	void		release();

	PBM &		operator=( const PBM & copy );

private:
	int			m_Width;
	int			m_Height;
	Color *		m_pPixels;
};

//----------------------------------------------------------------------------

inline PBM::PBM() : m_pPixels( NULL )
{}

inline PBM::PBM( int width, int height, Color * pPixels ) : m_pPixels( NULL )
{
	initialize( width, height, pPixels );
}

inline PBM::~PBM()
{
	release();
}

//----------------------------------------------------------------------------

inline int PBM::width() const
{
	return m_Width;
}

inline int PBM::height() const
{
	return m_Height;
}

inline int PBM::pixelCount() const
{
	return m_Width * m_Height;
}

inline Color * PBM::pixels() const
{
	return m_pPixels;
}

//----------------------------------------------------------------------------

inline void PBM::initialize( int width, int height, Color * pPixels )
{
	m_Width = width;
	m_Height = height;

	m_pPixels = new Color[ pixelCount() ];
	memcpy( m_pPixels, pPixels, sizeof(Color) * pixelCount() );
}

inline void PBM::release()
{
	delete m_pPixels;
	m_pPixels = NULL;
}

//----------------------------------------------------------------------------

inline PBM & PBM::operator=( const PBM & copy )
{
	initialize( copy.m_Width, copy.m_Height, copy.m_pPixels );
	return *this;
}

inline const InStream & operator>>( const InStream & input, PBM & pbm )
{
	int width;
	input >> width;
	int height;
	input >> height;

	int pixelCount = width * height;
	Color * pPixels = new Color[ pixelCount ];
	input.read( pPixels, sizeof(Color) * pixelCount );

	pbm.initialize( width, height, pPixels );

	delete pPixels;

	return input;
}

inline const OutStream & operator<<( const OutStream & output, const PBM & pbm )
{
	output << pbm.width();
	output << pbm.height();
	output.write( pbm.pixels(), sizeof(Color) * pbm.pixelCount() );
	return output;
}

//----------------------------------------------------------------------------


#endif

//----------------------------------------------------------------------------
//EOF

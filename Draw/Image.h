/*
	Image.h

	Image Resource
	(c)2005 Palestar, Richard Lyle
*/

#ifndef IMAGE_H
#define IMAGE_H

#include "Resource/Resource.h"
#include "Standard/Color.h"
#include "Standard/ColorFormat.h"
#include "Standard/Size.h"
#include "Standard/Rect.h"
#include "Standard/Point.h"
#include "Standard/Array.h"
#include "Standard/Buffer.h"
#include "Draw/ImageCodec.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Image	: public Resource
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef ResourceLink<Image>		Link;
	typedef Reference<Image>		Ref;
	typedef ColorFormat::Format		Format;

	// Construction
	Image();
	Image( const Image & copy );
	Image( Format eFormat, const SizeInt & size );
	virtual	~Image();

	// Widget Interface
	bool				read( const InStream & input );
	bool				write( const OutStream & output );	

	// Accessors
	Format				format() const;								// image format
	const SizeInt &		size() const;								// image dimensions
	const PointInt &	hotSpot() const;							// get the hot spot
	
	int					frameCount() const;							// frame count
	dword				frameSize( int frameIndex = 0) const;		// size of frame in bytes
	void *				frame( int frameIndex = 0) const;			// get pixel data for frame

	bool				isP2() const;								// is image power of 2
	bool				colorKeyEnable() const;						// is the colorkey enabled for this image
	Color				colorKey() const;							// what is the colorkey

	Image *				mipMap() const;								// next mipmap level of this image

	// Mutators
	bool				setFormat( Format eFormat );				// set the color format of this image
	void				setSize( const SizeInt & size );			// change image size, do not keep pixels in buffers
	void				setHotSpot( const PointInt & point );		// set the hot spot
	
	void				enableColorKey( bool enable );				// enable the color key
	void				setColorKey( Color color );					// set the color key

	bool				addFrame( const Buffer & pixels, 
							Format eFormat = ColorFormat::RGB8888 );
	bool 				insertFrame( int before, const Buffer & pixels, 
							Format eFormat = ColorFormat::RGB8888 );
	bool				removeFrame( int frameIndex );	

	bool				copy( const Image & image );				// replace this image
	bool				add( const Image & image );					// append image on to this
	void				release();									// release all frames

	// NOTE: Most of these functions have to convert to RGBA format to perform these operations, this 
	// may result in the loss of some pixel data in those cases.
	bool				resize( const SizeInt & size );				// resize image, point sample - 
	bool				resample( const SizeInt & size );			// resize image, average sample
	bool				resampleP2();								// resize image to power of 2
	bool				crop( const RectInt & crop );				// crop image to rectangle
	bool				setAlpha( Image::Ref pAlpha );				// set the alpha of this image from the pixel data of another image

	bool				exportTGA( const char * pTGAFile, 
							int nFrame = 0 );						// exports image frame to TGA

	bool				createMipMaps();							// builds mipmap levels for this image

private:
	// Data
	Format				m_eFormat;									// format of this image
	SizeInt				m_Size;										// Image size
	PointInt			m_HotSpot;									// Image hot spot
	bool				m_ColorKeyEnable;
	Color				m_ColorKey;
	Array< Buffer > 	m_Frames;									// pixel data

	Image::Ref			m_pMipMap;									// first mipmap in chain of mipmaps
	
	static ImageCodec::Ref
						allocateCodec( Format eFormat );			// allocate a encoder/decoder for the given format

	static bool			convert( const Buffer & input, Format eInputFormat, 
							Buffer & output, Format eOutputFormat, const SizeInt & size );
	static bool			convertPD( const Buffer & input, Format eInputFormat, 
							Buffer & output, Format eOutputFormat );
};

IMPLEMENT_RAW_STREAMING( Image::Format );

//----------------------------------------------------------------------------

inline Image::Format Image::format() const
{
	return m_eFormat;
}

inline Image * Image::mipMap() const
{
	return m_pMipMap;
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF

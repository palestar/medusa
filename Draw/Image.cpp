/*
	Image.cpp

	Image Resource
	(c)2005 Palestar, Richard Lyle
*/

#define DRAW_DLL

#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Draw/Image.h"
#include "Draw/ImageCodec.h"
#include "Draw/Palette.h"
#include "File/FileDisk.h"
#include "Standard/Bits.h"
#include "Standard/LZW.h"
#include "Standard/Library.h"

//----------------------------------------------------------------------------

// Make sure the ImageCodecJPEG gets loaded
#if defined(_DEBUG)
Library		SYSTEMWIN_DLL( "SystemWinD.dll" );
#else
Library		SYSTEMWIN_DLL( "SystemWin.dll" );
#endif

const dword RGB_CODE = 0xffffffff;

//---------------------------------------------------------------------------------------------------

IMPLEMENT_RESOURCE_FACTORY( Image, Resource );
REGISTER_FACTORY_KEY( Image, 3913014870769538136LL );

BEGIN_PROPERTY_LIST( Image, Resource );
	ADD_ENUM_PROPERTY( m_eFormat );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB888 );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB888E );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB888L );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB888LE );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB565 );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB565E );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB555 );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB8888 );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB5551 );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB4444 );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::RGB332 );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::JPEG );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::DXT1 );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::DXT3 );
		ADD_ENUM_OPTION( m_eFormat, ColorFormat::DXT5 );
	ADD_PROPERTY( m_Size );
	ADD_PROPERTY( m_HotSpot );
	ADD_PROPERTY( m_ColorKeyEnable );
	ADD_PROPERTY( m_ColorKey );
	ADD_PROPERTY( m_pMipMap );
END_PROPERTY_LIST();

//-------------------------------------------------------------------------------

#pragma warning( disable: 4018 )	// warning C4018: '==' : signed/unsigned mismatch

Image::Image()
{
	m_eFormat = ColorFormat::RGB8888;
	m_Size = SizeInt(0,0);
	m_HotSpot = PointInt(0,0);
	m_ColorKeyEnable = false;
	m_ColorKey = Color(255,0,255);
}

Image::Image( const Image & image ) 
{
	copy( image );
}

Image::Image( Format eFormat, const SizeInt & size  ) 
{
	m_eFormat = eFormat;
	m_Size = size;
	m_ColorKeyEnable = false;
	m_ColorKey = Color(255,0,255);
}

Image::~Image()
{
	release();
}

//----------------------------------------------------------------------------

const int VERSION_20060416 = 20060416;
const int VERSION_20100903 = 20100903;

bool Image::read( const InStream & input )
{
	if (! Resource::read( input ) )
		return false;

	int version;
	input >> version;

	switch( version )
	{
	case VERSION_20060416:
		{
			static ImageCodec * pJPEGCodec = WidgetCast<ImageCodec>( Factory::createNamedWidget( "ImageCodecJPEG" ) );

			m_eFormat = ColorFormat::RGB8888;

			int nFrames;
			input >> nFrames;

			for(int i=0;i<nFrames;i++)
			{
				dword nBytes;
				input >> nBytes;

				if ( nBytes != RGB_CODE )
				{
					// read in the compressed pixel information
					byte * pBuffer = new byte[ nBytes ];
					if (! pBuffer )
						throw std::bad_alloc();

					input.read( pBuffer, nBytes );
						
					Buffer input( pBuffer, nBytes, false );
					if ( pJPEGCodec != NULL )
					{
						Buffer output;
						int nDecompressed = pJPEGCodec->decode( input, output, m_Size );
						if ( nDecompressed > 0 )
							m_Frames.push( output );
					}
				}
				else
				{
					// image too small for jpeg, load raw image
					nBytes = m_Size.width * m_Size.height * sizeof(Color);

					byte * pBuffer = new byte[ nBytes ];
					if (! pBuffer )
						throw std::bad_alloc();

					input.read( pBuffer, nBytes );

					m_Frames.push( Buffer( pBuffer, nBytes, false ) );
				}
			}
		}
		break;
	case VERSION_20100903:
		input >> m_Frames;
		break;
	default:
		return false;
	}

	return true;
}

bool Image::write( const OutStream &output )
{
	if (! Resource::write( output ) )
		return false;

	output << VERSION_20100903;
	output << m_Frames;

	return true;
}

//----------------------------------------------------------------------------

const SizeInt & Image::size() const
{
	return( m_Size );
}

const PointInt & Image::hotSpot() const
{
	return( m_HotSpot );
}

int Image::frameCount() const
{
	return( m_Frames.size() );
}

dword Image::frameSize( int frameIndex /*= 0*/ ) const
{
	return m_Frames[ frameIndex ].bufferSize();
}

void * Image::frame( int frameIndex /*= 0*/) const
{
	return m_Frames[ frameIndex ].buffer();
}

bool Image::isP2() const
{
	return( m_Size.isP2() );
}

bool Image::colorKeyEnable() const
{
	return( m_ColorKeyEnable );
}

Color Image::colorKey() const
{
	return( m_ColorKey );
}

//----------------------------------------------------------------------------

bool Image::setFormat( Format eFormat )
{
	if ( m_eFormat != eFormat )
	{
		if ( m_Frames.size() == 0 )
		{
			// no frames to convert, early out..
			m_eFormat = eFormat;
			return true;
		}

		Array< Buffer > newFrames;
		for(int i=0;i<m_Frames.size();++i)
		{
			Buffer & frame = m_Frames[i];
			Buffer & newFrame = newFrames.push();

			if (! convert( frame, m_eFormat, newFrame, eFormat, m_Size ) )
			{
				TRACE( CharString().format( "Failed to convert frame %d", i ) );
				return false;
			}
		}

		m_Frames = newFrames;
		m_eFormat = eFormat;

		// convert our mipmaps as well if we have them..
		if ( m_pMipMap )
			m_pMipMap->setFormat( eFormat );
	}

	return true;
}

void Image::setSize( const SizeInt & size )
{
	m_Size = size;
}

void Image::setHotSpot( const PointInt & point )
{
	m_HotSpot = point;
}

void Image::enableColorKey( bool enable )
{
	m_ColorKeyEnable = enable;
}

void Image::setColorKey( Color color )
{
	m_ColorKey = color;
}

bool Image::addFrame(const Buffer & pixels, Format eFormat /*= ColorFormat::RGB8888*/ )
{
	if ( m_eFormat == eFormat )
	{
		// formats are the same, so just push the pixel data..
		m_Frames.push( pixels );
		return true;
	}
	
	Buffer converted;
	if (! convert( pixels, eFormat, converted, m_eFormat, m_Size ) )
		return false;

	m_Frames.push( converted );
	return true;
}

bool Image::insertFrame( int before, const Buffer & pixels, Format eFormat /*= ColorFormat::RGB8888*/ )
{
	if ( m_eFormat == eFormat )
	{
		m_Frames.insert( before, pixels );
		return true;
	}

	Buffer converted;
	if (! convert( pixels, eFormat, converted, m_eFormat, m_Size ) )
		return false;

	m_Frames.insert( before, converted );
	return true;
}

bool Image::removeFrame( int frameIndex )
{
	if ( frameIndex < 0 || frameIndex >= m_Frames.size() )
		return false;

	m_Frames.remove( frameIndex );
	return true;
}

//---------------------------------------------------------------------------------------------------

bool Image::add( const Image & image )
{
	if (image.size() != size() )
		return false;

	for(int i=0;i<image.m_Frames.size();i++)
	{
		if ( image.m_eFormat != m_eFormat )
		{
			Buffer converted;
			if (! convert( image.m_Frames[i], image.m_eFormat, converted, m_eFormat, m_Size ) )
				return false;
			m_Frames.push( converted );
		}
		else
		{
			// same format, no conversion required..
			m_Frames.push( image.m_Frames[i] );
		}
	}

	return true;
}

bool Image::copy( const Image & image )
{
	release();

	m_eFormat = image.m_eFormat;
	m_Size = image.m_Size;
	m_HotSpot = image.m_HotSpot;
	m_ColorKeyEnable = image.m_ColorKeyEnable;
	m_ColorKey = image.m_ColorKey;
	m_Frames = image.m_Frames;

	if ( image.m_pMipMap.valid() )
		m_pMipMap = new Image( *image.m_pMipMap );
	else
		m_pMipMap = NULL;

	return true;
}

void Image::release()
{
	m_Frames.release();
	m_pMipMap = NULL;
}

//---------------------------------------------------------------------------------------------------

bool Image::resize( const SizeInt & size )
{
	if (m_Size != size)
	{
		Format eSavedFormat = m_eFormat;
		if ( m_eFormat != ColorFormat::RGB8888 )
			setFormat( ColorFormat::RGB8888 );

		// calulate stepping values
		int	ddx = (m_Size.width << 16) / size.width;
		int	ddy = (m_Size.height << 16) / size.height;
		
		for(int f=0;f<m_Frames.size();f++)
		{
			// allocate a new pdat
			Color * source = (Color *)m_Frames[ f ].buffer();
			ASSERT( source );
			Color * destination = new Color[ size.width * size.height ];
			ASSERT( destination );

			int dy = 0;
			for(int y=0;y<size.height;y++)
			{
				int dx = 0;
				for(int x=0;x<size.width;x++)
				{
					destination[ x + (y * size.width) ] = source[ (dx >> 16) + ((dy >> 16) * m_Size.width) ];
					dx += ddx;
				}
				dy += ddy;
			}

			m_Frames[f].set( destination, size.width * size.height * sizeof(Color) );
		}
		m_Size = size;

		// restore the color format..
		if ( eSavedFormat != m_eFormat )
			setFormat( eSavedFormat );
	}

	return true;
}

bool Image::resample( const SizeInt & size )
{
	// TODO: finish this routine, for now just call the point sample version
	return resize( size );
}

bool Image::resampleP2()
{
	if (! isP2() )
		return resample( SizeInt( 1 << GetLastBit( m_Size.width ), 1 << GetLastBit( m_Size.height ) ) );
	return true;
}

bool Image::crop( const RectInt & crop )
{
	SizeInt newSize( crop.width(), crop.height() );
	if ( newSize != m_Size )
	{
		Format eSavedFormat = m_eFormat;
		if ( m_eFormat != ColorFormat::RGB8888 )
			setFormat( ColorFormat::RGB8888 );

		for(int f=0;f<m_Frames.size();f++)
		{
			// copy from the source to the destination
			Color * source = (Color *)m_Frames[f].buffer();
			Color * newFrame = new Color[ newSize.width * newSize.height ];
			Color * destination = newFrame;

			int line = crop.top;
			source += line * m_Size.width;

			int skip = crop.left;
			int copy = newSize.width;
			int dSkip = 0;
			if ( (copy + skip) > m_Size.width)
			{
				dSkip = copy - m_Size.width;
				copy = m_Size.width - skip;
			}

			int skipEnd = m_Size.width - ( copy + skip );
			while( line <= crop.bottom && line < m_Size.height )
			{
				source += skip;
				memcpy(destination,source, copy * sizeof(Color) );
				source += skipEnd + copy;
				destination += dSkip + copy;

				line++;
			}

			m_Frames[f].set( newFrame, newSize.width * newSize.height * sizeof(Color) );
		}
		m_Size = newSize;

		// restore the color format..
		if ( eSavedFormat != m_eFormat )
			setFormat( eSavedFormat );
	}

	return true;
}

bool Image::setAlpha( Image::Ref pAlpha )
{
	// alpha must be the same size and in RGB8888
	if ( pAlpha->size() != size() || pAlpha->format() != ColorFormat::RGB8888 )
	{
		// make a copy so we don't modify the original..
		pAlpha = new Image( *pAlpha );
		if ( pAlpha->format() != ColorFormat::RGB8888 )
			pAlpha->setFormat( ColorFormat::RGB8888 );
		if ( pAlpha->size() != size() )
			pAlpha->resample( size() );
	}

	// our image must be in RGB8888 to set the alpha..
	Format eSavedFormat = m_eFormat;
	if ( m_eFormat != ColorFormat::RGB8888 )
		setFormat( ColorFormat::RGB8888 );
	
	int nAlphaFrames = pAlpha->frameCount();
	dword nPixels = m_Size.width * m_Size.height;
	for(int f=0;f<m_Frames.size();f++)
	{
		Color * pDest = (Color *)m_Frames[f].buffer();
		Color * pSrc = (Color *)pAlpha->m_Frames[f % nAlphaFrames].buffer();
		
		for(dword p=0;p<nPixels;++p)
			pDest[p].m_A = pSrc[p].magnitudeNoAlpha();
	}

	// restore the color format..
	if ( eSavedFormat != m_eFormat )
		setFormat( eSavedFormat );

	return true;
}

//----------------------------------------------------------------------------

#pragma pack(push, 1 )

struct RGB 
{
	byte	b,g,r;
};

struct TGA_HEADER
{
	byte	identsize;          // size of ID field that follows 18 byte header (0 usually)
	byte	palette;			// type of color map 0=none, 1=has palette
	byte	imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

	word	color_start;		// first color map entry in palette
	word	num_palette_colors;	// number of colors in palette
	byte	bits_per_color;		// number of bits per palette entry 15,16,24,32

	word	xstart;             // image x origin
	word	ystart;             // image y origin
	word	width;              // image width in pixels
	word	height;             // image height in pixels
	byte	bits_per_pixel;		// image bits per pixel 8,16,24,32
	byte	descriptor;			// image descriptor bits (vh flip bits)

};

#pragma pack( pop )

#define TGA_DESC_FLIPX			(0x10)
#define TGA_DESC_FLIPY			(0x20)

enum	//  TGA_TYPE
{
	TGA_NONE = 0,
	TGA_INDEXED_UNCOMPRESSED = 1,
	TGA_RGB_UNCOMPRESSED = 2,
	TGA_GREY_UNCOMPRESSED = 3,
	TGA_INDEXED_COMPRESSED = 9,
	TGA_RGB_COMPRESSED = 10,
	TGA_GREY_COMPRESSED = 11,
};


bool Image::exportTGA( const char * pTGAFile, int nFrame /*= 0*/ )
{
	if (! m_Frames.isValid( nFrame ) )
		return false;		// invalid image frame

	FileDisk fd;
	if (! fd.open( pTGAFile, FileDisk::WRITE ) )
		return false;		// failed to open file

	Format eSavedFormat = m_eFormat;
	if ( m_eFormat != ColorFormat::RGB8888 )
		setFormat( ColorFormat::RGB8888 );

	TGA_HEADER header;
	memset( &header, 0, sizeof(header));

	header.imagetype = TGA_RGB_UNCOMPRESSED;
	header.bits_per_pixel = 32;
	header.width = size().width;
	header.height = size().height;
	header.descriptor = 0;
 
	bool bSuccess = true;
	try {
		fd.write( &header, sizeof(header) );

		Color * pFrame = (Color *)frame( nFrame );
		for(int y=size().height-1;y>=0;y--)
			fd.write( pFrame + (y * size().width), sizeof(Color) * size().width );
	}
	catch( FileDisk::FileError )
	{
		bSuccess = false;
	}

	fd.close();

	// restore the color format..
	if ( eSavedFormat != m_eFormat )
		setFormat( eSavedFormat );

	return bSuccess;
}

bool Image::createMipMaps()
{
	// clear our mipmap now, so we don't bother up converting our current mipmap just to replace it.
	m_pMipMap = NULL;		

	SizeInt mipSize( m_Size.width >> 1, m_Size.height >> 1 );
	if ( mipSize.width < 1 || mipSize.height < 1 )
		return false;

	// up convert this image now, just in case it's in a lower format.. 
	Format eSavedFormat = m_eFormat;
	if ( m_eFormat != ColorFormat::RGB8888 )
		setFormat( ColorFormat::RGB8888 );

	m_pMipMap = new Image( *this );
	if (! m_pMipMap->resample( mipSize ) )
		return false;

	// follow the chain..
	m_pMipMap->createMipMaps();

	// restore the color format..
	if ( eSavedFormat != m_eFormat )
		setFormat( eSavedFormat );

	return true;
}

//---------------------------------------------------------------------------------------------------

ImageCodec::Ref Image::allocateCodec( Format eFormat )
{
	Widget::Ref pUncasted;
	switch( eFormat )
	{
	case ColorFormat::JPEG:
		pUncasted = Factory::createNamedWidget( "ImageCodecJPEG" );
		break;
	case ColorFormat::DXT1:
		pUncasted = Factory::createNamedWidget( "ImageCodecDXT1" );
		break;
	case ColorFormat::DXT3:
		pUncasted = Factory::createNamedWidget( "ImageCodecDXT3" );
		break;
	case ColorFormat::DXT5:
		pUncasted = Factory::createNamedWidget( "ImageCodecDXT5" );
		break;
	default:
		return NULL;
	}

	if (! pUncasted.valid() )
		TRACE( "Failed to create image codec!" );

	return WidgetCast<ImageCodec>( pUncasted );
}

//---------------------------------------------------------------------------------------------------

bool Image::convert( const Buffer & input, Format eInputFormat, 
						Buffer & output, Format eOutputFormat, const SizeInt & size )
{
	if ( eInputFormat == eOutputFormat )
	{
		output = input;
		return true;
	}

	Buffer uncompressed;

	// firstly, decompress the source data if needed...
	ImageCodec::Ref pDecoder = allocateCodec( eInputFormat );
	if ( pDecoder.valid() )
	{
		if ( pDecoder->decode( input, uncompressed, size ) < 0 )
			return false;
		eInputFormat = ColorFormat::RGB8888;
	}
	else
	{
		// source data is already in a uncompressed format..
		uncompressed = input;
	}

	// recompress if needed...
	ImageCodec::Ref pEncoder = allocateCodec( eOutputFormat );
	if ( pEncoder.valid() )
	{
		// encoders require RGB8888, convert the pixels into that format if needed...
		if ( eInputFormat != ColorFormat::RGB8888 )
		{
			Buffer converted;
			if (! convertPD( uncompressed, eInputFormat, converted, ColorFormat::RGB8888 ) )
				return false;

			uncompressed = converted;
			eInputFormat = ColorFormat::RGB8888;
		}

		if ( pEncoder->encode( uncompressed, output, size, ImageCodec::CL_BEST ) < 0 )
			return false;

		return true;
	}

	if ( eInputFormat != eOutputFormat )
		return convertPD( uncompressed, eInputFormat, output, eOutputFormat );

	// no conversion needed, just copy the pixel data to the output..
	output = uncompressed;
	return true;
}

bool Image::convertPD( const Buffer & input, Format eInputFormat, 
						Buffer & output, Format eOutputFormat )
{
	if ( eInputFormat == eOutputFormat )
		return false;		// no conversion needed..

	ColorFormat::Ref pInputFormat = ColorFormat::allocateFormat( eInputFormat );
	if (! pInputFormat )
	{
		TRACE( "Failed to allocate source pixel format!" );
		return false;
	}
	ColorFormat::Ref pOutputFormat = ColorFormat::allocateFormat( eOutputFormat );
	if (! pOutputFormat )
	{
		TRACE( "Failed to allocate dest pixel format!" );
		return false;
	}

	bool bSuccess = true;
	if (! ColorFormat::convert( input, pInputFormat, output, pOutputFormat ) )
	{
		TRACE( "Failed to convert pixel formats!" );
		bSuccess = false;
	}

	return bSuccess;
}

//----------------------------------------------------------------------------
// EOF

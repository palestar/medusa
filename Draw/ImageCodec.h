/*
	ImageCodec.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef IMAGECODEC_H
#define IMAGECODEC_H

#include "Standard/Size.h"
#include "Standard/Buffer.h"
#include "Factory/FactoryTypes.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

const ClassKey		CLASS_KEY_JPEG_IMAGE_CODEC( "ImageCodecJPEG" );
const ClassKey		CLASS_KEY_DXT1_IMAGE_CODEC( "ImageCodecDXT1" );
const ClassKey		CLASS_KEY_DXT2_IMAGE_CODEC( "ImageCodecDXT2" );
const ClassKey		CLASS_KEY_DXT3_IMAGE_CODEC( "ImageCodecDXT3" );
const ClassKey		CLASS_KEY_DXT4_IMAGE_CODEC( "ImageCodecDXT4" );
const ClassKey		CLASS_KEY_DXT5_IMAGE_CODEC( "ImageCodecDXT5" );

//----------------------------------------------------------------------------

class DLL ImageCodec : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference< ImageCodec >			Ref;

	enum EncodeLevel {
		CL_BEST,			// smallest size, most time
		CL_NORMAL,			// default compression
		CL_FAST,			// fastest compression
		CL_NONE,			// none/faster compression

	};

	// Construction
	ImageCodec()
	{}

	//! This should encode RGBA data into the compressed image format.
	virtual int		encode( const Buffer & input, Buffer & output, const SizeInt & size, EncodeLevel nLevel ) = 0;
	//! This should decode the compressed image into the RGBA format.
	virtual int		decode( const Buffer & input, Buffer & output, const SizeInt & size ) = 0;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF

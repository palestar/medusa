/**
	@file ImageCodecDXT.h
	@brief DXT compression codecs.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/3/2010 1:16:48 PM
*/

#ifndef IMAGECODECDXT_H
#define IMAGECODECDXT_H

#include "Draw/ImageCodec.h"

//---------------------------------------------------------------------------------------------------

class ImageCodecDXT1 : public ImageCodec
{
public:
	DECLARE_WIDGET_CLASS();

	// ImageCodec interface
	int				encode( const Buffer & input, Buffer & output, const SizeInt & size, EncodeLevel nLevel );
	int				decode( const Buffer & input, Buffer & output, const SizeInt & size );
};

class ImageCodecDXT3 : public ImageCodec
{
public:
	DECLARE_WIDGET_CLASS();

	// ImageCodec interface
	int				encode( const Buffer & input, Buffer & output, const SizeInt & size, EncodeLevel nLevel );
	int				decode( const Buffer & input, Buffer & output, const SizeInt & size );
};

class ImageCodecDXT5 : public ImageCodec
{
public:
	DECLARE_WIDGET_CLASS();

	// ImageCodec interface
	int				encode( const Buffer & input, Buffer & output, const SizeInt & size, EncodeLevel nLevel );
	int				decode( const Buffer & input, Buffer & output, const SizeInt & size );
};


#endif

//---------------------------------------------------------------------------------------------------
//EOF

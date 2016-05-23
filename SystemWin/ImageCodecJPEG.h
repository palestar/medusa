/*
	ImageCodecJPEG.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef IMAGECODECJPEG_H
#define IMAGECODECJPEG_H

#include "Draw/ImageCodec.h"
#include "SystemWin/SystemWin.h"

//----------------------------------------------------------------------------

class ImageCodecJPEG : public ImageCodec
{
public:
	DECLARE_WIDGET_CLASS();

	// ImageCodec interface
	int				encode( const Buffer & input, Buffer & output, const SizeInt & size, EncodeLevel nLevel );
	int				decode( const Buffer & input, Buffer & output, const SizeInt & size );
};

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF

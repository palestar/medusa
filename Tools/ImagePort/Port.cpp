/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define IMAGEPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"
#include "ChildFrame.h"
#include "Resource.h"

#include "File/FileDisk.h"
#include "File/Path.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( ImagePort, Port );
REGISTER_FACTORY_KEY(  ImagePort, 3914102874601741960 );

BEGIN_PROPERTY_LIST( ImagePort, Port );
	ADD_NOEDIT_PROPERTY( m_Image );
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
	ADD_PROPERTY( m_HotSpot );
	ADD_PROPERTY( m_DoCrop );
	ADD_PROPERTY( m_Crop );
	ADD_PROPERTY( m_DoResize );
	ADD_PROPERTY( m_Resize );
	ADD_PROPERTY( m_DoResample );
	ADD_PROPERTY( m_KeepFrames );
	ADD_PROPERTY( m_Start );
	ADD_PROPERTY( m_End );
	ADD_PROPERTY( m_ColorKeyEnable );
	ADD_COLOR_PROPERTY( m_ColorKey );
	ADD_PROPERTY( m_bCreateMipMaps );
END_PROPERTY_LIST();

ImagePort::ImagePort() : Port()
{
	m_eFormat = ColorFormat::DXT1;
	m_DoCrop = false;
	m_DoResize = false;
	m_DoResample = false;
	m_KeepFrames = false;
	m_Start = m_End = 0;
	m_Resize.set(0,0);
	m_HotSpot.set(0,0);

	m_ColorKeyEnable = false;
	m_ColorKey = Color(255,0,255);
	m_bCreateMipMaps = true;
}

//-------------------------------------------------------------------------------

bool ImagePort::read( const InStream & input )
{
	if (! Port::read( input ) )
		return false;

	return true;
}

//----------------------------------------------------------------------------

Resource::Ref ImagePort::createResource()
{
	if (! m_Image.valid() )
		return NULL;

	// create a copy of our image
	Image * pImage = new Image( *m_Image );

	pImage->setHotSpot( m_HotSpot );
	pImage->enableColorKey( m_ColorKeyEnable );
	pImage->setColorKey( m_ColorKey );

	if ( m_DoCrop )
		pImage->crop( m_Crop );
	if ( m_DoResize )
	{
		if ( m_DoResample )
			pImage->resample( m_Resize );
		else
			pImage->resize( m_Resize );
	}
	if ( m_KeepFrames )
	{
		for(int i=pImage->frameCount() - 1;i >= 0;i--)
			if ( i < m_Start || i > m_End )
				pImage->removeFrame( i );
	}

	// DXT formats require the image to be divisible by 4 ...
	if ( m_eFormat == ColorFormat::DXT1 || m_eFormat == ColorFormat::DXT3 || m_eFormat == ColorFormat::DXT5 )
	{
		SizeInt validSize( pImage->size() );
		validSize.height &= ~0x3;			// remove the lower 2 bits
		validSize.width &= ~0x3;			

		if ( validSize != pImage->size() )
			pImage->resample( validSize	);
	}

	// create our mipmap levels before we compress, it saves decompressing and recompressing to resample..
	if ( m_bCreateMipMaps )
		pImage->createMipMaps();

	// lastly set the format, which will compress the image if needed..
	if (! pImage->setFormat( m_eFormat ) )
	{
		report( "Failed to set pixel format!" );
		return NULL;
	}

	return pImage;
}

CFrameWnd * ImagePort::createView()
{
	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

bool ImagePort::importFile( const char * fileName )
{
	SortedArray< CharString > import;
	import.insert( fileName );

	m_Image = importImages( import );
	
	Resource::Ref resource = createResource();
	if ( resource.valid() )
		return true;

	return false;
}

void ImagePort::shellOpen() const
{}

//----------------------------------------------------------------------------

Image * ImagePort::importImages( const SortedArray< CharString > & importFiles )
{
	Image * pImage = NULL;

	// import the images
	Array< Image * > images;
	for(int i=0;i<importFiles.size();i++)
	{
		Image * pImage = import( importFiles[i] );
		if (! pImage )
		{
			report( CharString().format("Failed to import image '%s'!", importFiles[i]) );

			// clean up imported images and return NULL
			for(int j=0;j<images.size();j++)
				delete images[j];
			return NULL;
		}
		images.push( pImage );
	}

	if ( images.size() == 0 )
		return NULL;		

	if ( images.size() > 1 )
	{
		pImage = images[ 0 ];
		for(int j=1;j<images.size();j++)
		{
			Image * pAddImage = images[ j ];

			if ( pAddImage->size() != pImage->size() )
				pAddImage->resample( pImage->size() );

			// append the image
			pImage->add( *pAddImage );

			// delete the imported image
			delete pAddImage;
		}
	}
	else
		pImage = images[ 0 ];

	return pImage;
}

//----------------------------------------------------------------------------

Image * ImagePort::editImage()
{
	if ( m_Image.valid() )
	{
		// export all images into our current directory for this port
		CharString sExportDirectory( Path( GetPathName() ).directory() );

		// write out all frames to temp files
		for(int f=0;f<m_Image->frameCount();f++)
		{
			// generate a temp file name
			CharString sExportFile;
			sExportFile.format( "%s%s%u.tga", sExportDirectory, GetTitle(), f );

			if ( m_Image->exportTGA( sExportFile, f ) )
				ShellExecute(0, "open", sExportFile, NULL, NULL, SW_SHOWNORMAL);
		}

		if ( MessageBox( AfxGetMainWnd()->GetSafeHwnd(), "Edit frames then click YES to reload image?", "Reload Frames", MB_YESNO) == IDYES )
		{
			int nFrames = m_Image->frameCount();

			m_Image->release();

			// import edited files back into image
			for(int f=0;f<nFrames;f++)
			{
				CharString sImportFile;
				sImportFile.format( "%s%s%u.tga", sExportDirectory, GetTitle(), f );

				Image * pImage = import( sImportFile );
				if ( pImage != NULL )
				{
					if ( pImage->size() != m_Image->size() )
					{
						if ( f > 0 )
							pImage->resample( m_Image->size() );
						else
							m_Image->resample( pImage->size() );
					}
					
					m_Image->add( *pImage );

					delete pImage;
				}
			}

			SetModifiedFlag();
			UpdateAllViews( NULL );
		}

		//// clean up temp files
		//for(int f=0;f<m_Image->frameCount();f++)
		//{
		//	// generate a temp file name
		//	CharString tempFile;
		//	tempFile.format( "%s%s%u.tga", tempPath, GetTitle(), f );

		//	DeleteFile( tempFile );
		//}
	}

	return m_Image;
}

//-------------------------------------------------------------------------------
// EOF


/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef IMAGEPORT_H
#define IMAGEPORT_H

#include "Standard/SortedArray.h"
#include "Draw/Image.h"

#include "Tools/ResourcerDoc/Port.h"
#include "Tools/ImagePort/PortDll.h"

//-------------------------------------------------------------------------------

class DLL ImagePort : public Port
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<ImagePort>		Ref;
	typedef Image						Image;

	// Construction
	ImagePort();
	// Widget Interface
	bool						read( const InStream & input );
	// Port Interface
	virtual	Resource::Ref		createResource();							// create the resource 
	virtual CFrameWnd *			createView();								// create a view of the port

	virtual bool				importFile( const char * fileName );		// import from file
	virtual void				shellOpen() const;							// open the source object through a shell program

	// Helpers
	Image *						importImages( const SortedArray< CharString > & images );
	Image *						editImage();
	static Image *				import( const char * fileName );			// PortImport.cpp

	static BITMAPINFO *			createDIB( const Image & image );	
	static dword				getFileTime( const char * fileName );

	// Data
	Reference< Image >			m_Image;									// imported image in RGB8888 format
	PointInt					m_HotSpot;

	ColorFormat::Format			m_eFormat;									// output format...
	bool						m_DoCrop;
	RectInt						m_Crop;
	bool						m_DoResize;
	SizeInt						m_Resize;
	bool						m_DoResample;
	bool						m_KeepFrames;								// keep the specified frames
	int							m_Start,m_End;	

	bool						m_ColorKeyEnable;
	Color						m_ColorKey;

	bool						m_bCreateMipMaps;
};


#endif

//-------------------------------------------------------------------------------
// EOF

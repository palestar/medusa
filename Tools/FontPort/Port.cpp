/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define FONTPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"
#include "ChildFrame.h"
#include "Resource.h"

#include "Draw/Font.h"

//-------------------------------------------------------------------------------

extern AFX_EXTENSION_MODULE					g_PortDLL;

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( FontPort, Port );
REGISTER_FACTORY_KEY(  FontPort, 3921601665451203698 );
IMPLEMENT_RAW_STREAMING( LOGFONT );

BEGIN_PROPERTY_LIST( FontPort, Port );
	ADD_PROPERTY( m_LogicalFont );
END_PROPERTY_LIST();

FontPort::FontPort() : Port()
{
	memset( &m_LogicalFont, 0, sizeof(m_LogicalFont));
	strcpy_s( m_LogicalFont.lfFaceName, sizeof(m_LogicalFont.lfFaceName), "Verdana" );
	m_LogicalFont.lfHeight = 20;
}

//-------------------------------------------------------------------------------

const int VERSION_20040626 = 20040626;
const int VERSION_20050619 = 20050619;

bool FontPort::read( const InStream & input )
{
	if (! Port::read( input ) )
	{
		int nVersion;
		input >> nVersion;

		switch( nVersion )
		{
		default:
			// pre-version font
			input.file()->rewind( sizeof(int) );
			input.read( &m_LogicalFont, sizeof(m_LogicalFont));
			break;
		case VERSION_20050619:
			input.read( &m_LogicalFont, sizeof(m_LogicalFont));
			break;
		case VERSION_20040626:
			{
				LOGFONTW logf;
				input.read( &logf, sizeof(LOGFONTW) );

				m_LogicalFont.lfHeight = logf.lfHeight;
				m_LogicalFont.lfWidth = logf.lfWidth;
				m_LogicalFont.lfEscapement = logf.lfEscapement;
				m_LogicalFont.lfOrientation = logf.lfOrientation;
				m_LogicalFont.lfWeight = logf.lfWeight;
				m_LogicalFont.lfItalic = logf.lfItalic;
				m_LogicalFont.lfUnderline = logf.lfUnderline;
				m_LogicalFont.lfStrikeOut = logf.lfStrikeOut;
				m_LogicalFont.lfCharSet = logf.lfCharSet;
				m_LogicalFont.lfOutPrecision = logf.lfOutPrecision;
				m_LogicalFont.lfClipPrecision = logf.lfClipPrecision;
				m_LogicalFont.lfQuality = logf.lfQuality;
				m_LogicalFont.lfPitchAndFamily = logf.lfPitchAndFamily;
				strncpy_s( m_LogicalFont.lfFaceName, sizeof(m_LogicalFont.lfFaceName), CharString( logf.lfFaceName ), LF_FACESIZE );
			}
			break;
		}
	}

	return true;
}

//----------------------------------------------------------------------------

#define ALIGN( value, by )		((((value) + ((by) - 1)) / (by)) * (by))

void FontPort::dependencies( DependentArray & dep )
{}

Resource::Ref FontPort::createResource()
{
	// create the memory DC
	CDC dc;
	dc.CreateCompatibleDC(NULL);

	// create the font
	HFONT hFont = CreateFontIndirect( &m_LogicalFont );
	CFont * previousFont = dc.SelectObject( CFont::FromHandle( hFont ) );

	// get the text metrics
	TEXTMETRICW metric;
	GetTextMetricsW( dc, &metric);

	int characterCount = (metric.tmLastChar - metric.tmFirstChar) + 1;
	int alignedWidth = ALIGN( metric.tmMaxCharWidth, 2 );

	// get the character widths
	int * pCharWidth = new int[ characterCount ];
	if (! GetCharWidth32W( dc, metric.tmFirstChar, metric.tmLastChar, pCharWidth ) )
	{
		report( "Failed to get font character widths!" );
		return NULL;
	}

	// create the font resource
	Font * pFont = new Font( SizeInt( alignedWidth, metric.tmHeight), 
								metric.tmFirstChar, 0 );
	//pFont->setKey( key() );

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap( &dc, alignedWidth, metric.tmHeight );

	// generate the bitmap data
	for(int i=0;i<characterCount;i++)
	{
		CBitmap * previousBitmap = dc.SelectObject(&bitmap);

		RECT fillRect = { 0,0, alignedWidth, metric.tmHeight };
		dc.FillSolidRect(&fillRect , 0x0 );

		// TODO: Does TextOutW work?
		dc.SetBkMode( TRANSPARENT );
		dc.SetTextColor( 0xffffff );

		TextOutW( dc, 0, 0, String().format(STR("%c"), metric.tmFirstChar + i ), 1 );
		//dc.TextOut( 0, 0, (const wchar *)String().format(STR("%c"), metric.tmFirstChar + i ) );

		dc.SelectObject( previousBitmap );

		BITMAP bitmapInfo;
		bitmap.GetObject( sizeof(bitmapInfo), &bitmapInfo );
		ASSERT( bitmapInfo.bmBitsPixel == 1 );
		ASSERT( bitmapInfo.bmHeight == metric.tmHeight );
		ASSERT( bitmapInfo.bmWidth == alignedWidth );

		int alignedBufferSize = bitmapInfo.bmWidthBytes * bitmapInfo.bmHeight;
		byte * pAlignedBuffer = new byte[ alignedBufferSize ];
		memset( pAlignedBuffer, 0, alignedBufferSize );

		bitmap.GetBitmapBits( alignedBufferSize, pAlignedBuffer );

		// allocate the character buffer
		byte * dst = pFont->addCharacter( pCharWidth[i] );
		byte * src = pAlignedBuffer;

		// copy the bitmap information line by line
		for(int j=0;j<bitmapInfo.bmHeight;j++)
		{
			memcpy( dst, src, pFont->characterPitch() );
			dst += pFont->characterPitch();
			src += bitmapInfo.bmWidthBytes;
		}

		delete [] pAlignedBuffer;
	}

	delete [] pCharWidth;

	dc.SelectObject( previousFont );

	return( pFont );
}

CFrameWnd * FontPort::createView()
{
	static CMultiDocTemplate	docTemplate( IDR_PORT,
							NULL,
							RUNTIME_CLASS(CChildFrame),
							RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

bool FontPort::importFile( const char * fileName )
{
	return false;	// return true of successful import, false on failure
}

void FontPort::shellOpen() const
{
	MessageBox( NULL, _T("shellOpen() is not valid for this port"), NULL, MB_OK);
}

//-------------------------------------------------------------------------------
// EOF


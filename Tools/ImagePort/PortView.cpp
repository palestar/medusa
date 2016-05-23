// PortView.cpp : implementation file
//

#define IMAGEPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"
#include "ImageView.h"
#include "ChildFrame.h"
#include ".\portview.h"



#pragma warning(disable:4800)	// forcing value to bool 'true' or 'false' (performance warning)

extern AFX_EXTENSION_MODULE					g_PortDLL;

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_HotSpotX = 0;
	m_HotSpotY = 0;
	m_DoCrop = FALSE;
	m_Left = 0;
	m_Top = 0;
	m_Right = 0;
	m_Bottom = 0;
	m_DoResize = FALSE;
	m_DoResample = FALSE;
	m_KeepFrames = FALSE;
	m_Start = 0;
	m_End = 0;
	m_ResizeWidth = 0;
	m_ResizeHeight = 0;
	m_Width = _T("");
	m_Height = _T("");
	m_Frames = _T("");
	m_ColorKeyEnable = FALSE;
	m_bCreateMipMaps = TRUE;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Control(pDX, IDC_SPIN12, m_SpinHeight);
	DDX_Control(pDX, IDC_SPIN11, m_SpinWidth);
	DDX_Control(pDX, IDC_SPIN8, m_SpinEnd);
	DDX_Control(pDX, IDC_SPIN7, m_SpinStart);
	DDX_Control(pDX, IDC_SPIN6, m_SpinBottom);
	DDX_Control(pDX, IDC_SPIN5, m_SpinRight);
	DDX_Control(pDX, IDC_SPIN4, m_SpinTop);
	DDX_Control(pDX, IDC_SPIN3, m_SpinLeft);
	DDX_Control(pDX, IDC_SPIN2, m_SpinHY);
	DDX_Control(pDX, IDC_SPIN1, m_SpinHX);
	DDX_Control(pDX, IDC_THUMB, m_Thumb);
	DDX_Text(pDX, IDC_EDIT2, m_HotSpotX);
	DDX_Text(pDX, IDC_EDIT3, m_HotSpotY);
	DDX_Check(pDX, IDC_CHECK1, m_DoCrop);
	DDX_Text(pDX, IDC_EDIT4, m_Left);
	DDX_Text(pDX, IDC_EDIT5, m_Top);
	DDX_Text(pDX, IDC_EDIT6, m_Right);
	DDX_Text(pDX, IDC_EDIT7, m_Bottom);
	DDX_Check(pDX, IDC_CHECK2, m_DoResize);
	DDX_Check(pDX, IDC_CHECK4, m_DoResample);
	DDX_Check(pDX, IDC_CHECK5, m_KeepFrames);
	DDX_Text(pDX, IDC_EDIT10, m_Start);
	DDX_Text(pDX, IDC_EDIT11, m_End);
	DDX_Text(pDX, IDC_EDIT8, m_ResizeWidth);
	DDX_Text(pDX, IDC_EDIT9, m_ResizeHeight);
	DDX_Text(pDX, IDC_WIDTH, m_Width);
	DDX_Text(pDX, IDC_HEIGHT, m_Height);
	DDX_Text(pDX, IDC_FRAMES, m_Frames);
	DDX_Check(pDX, IDC_COLORKEY_ENABLE, m_ColorKeyEnable);
	DDX_Control(pDX, IDC_COMBO1, m_PixelFormat );
	DDX_Check(pDX, IDC_CHECK3, m_bCreateMipMaps );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON3, OnView)
	ON_UPDATE_COMMAND_UI(ID_PORT_VIEW, OnUpdateView)
	ON_BN_CLICKED(IDC_BUTTON1, OnImport)
	ON_COMMAND(ID_PORT_VIEW, OnView)
	ON_BN_CLICKED(IDC_BUTTON4, OnEdit)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON6, OnImportAlpha)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPortView diagnostics

#ifdef _DEBUG
void CPortView::AssertValid() const
{
	CFormView::AssertValid();
}

void CPortView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPortView message handlers

void CPortView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	m_ColorKey.SubclassDlgItem(IDC_COLORKEY,this);

	m_SpinHeight.SetRange( 0, 2048 );
	m_SpinWidth.SetRange( 0, 2048 );
	m_SpinHY.SetRange( -2048, 2048 );
	m_SpinHX.SetRange( -2048, 2048 );

	for( ColorFormat::Format eFormat = ColorFormat::FIRST; eFormat <= ColorFormat::LAST; 
		eFormat = (ColorFormat::Format)(eFormat + 1) )
	{
		m_PixelFormat.AddString( ColorFormat::formatText( eFormat ) );
	}

	// copy data from port to view..
	OnUpdate( NULL, FALSE, NULL );
}

void CPortView::OnDraw(CDC* pDC) 
{
	ImagePort * pDoc = dynamic_cast<ImagePort *>(GetDocument());
	ASSERT( pDoc );

	if ( pDoc->m_Image.valid() && pDoc->m_Image->frameCount() > 0)
	{
		RECT thumbRect;
		m_Thumb.GetClientRect(&thumbRect);

		// Update the thumbnail image
		CClientDC screenDC( &m_Thumb );
		pDC = &screenDC;

		BITMAPINFO * pDIB = ImagePort::createDIB( *pDoc->m_Image );

		// copy into aligned buffer, otherwise StretchDIBits will fail
		void * pImage = ::malloc( pDoc->m_Image->frameSize() );
		memcpy( pImage, pDoc->m_Image->frame( 0 ), pDoc->m_Image->frameSize() );

		SetStretchBltMode( pDC->m_hDC, COLORONCOLOR );
		// blit the pixels
		SizeInt szImage = pDoc->m_Image->size();
		StretchDIBits( pDC->m_hDC, 0, 0, thumbRect.right - thumbRect.left, thumbRect.bottom - thumbRect.top,
			0,0,szImage.width,szImage.height, pImage, pDIB, DIB_RGB_COLORS, SRCCOPY);

		::free( pImage );
		delete pDIB;
	}
}

void CPortView::OnView() 
{
	ImagePort * pDoc = dynamic_cast<ImagePort *>(GetDocument());
	ASSERT( pDoc );

	if ( pDoc->m_Image.valid() && pDoc->m_Image->frameCount() > 0)
	{
		static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
			RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CImageView) );

		CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( pDoc, NULL ) );
		ASSERT( pFrame );

		docTemplate.InitialUpdateFrame(pFrame, pDoc);
	}
}

void CPortView::OnUpdateView(CCmdUI* pCmdUI) 
{
	ImagePort * pDoc = dynamic_cast<ImagePort *>(GetDocument());
	ASSERT( pDoc );

	pCmdUI->Enable(pDoc->m_Image->frameCount() > 0);
}

#pragma warning( disable: 4805 )		// warning C4805: '!=' : unsafe mix of type 'bool' and type 'int' in operation

#define UPDATE_DOC( VAR, VAL )					\
	if ( pDoc->VAR != VAL ) {					\
		pDoc->VAR = VAL;						\
		pDoc->SetModifiedFlag();	}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	ImagePort * pDoc = dynamic_cast<ImagePort *>(GetDocument());
	ASSERT( pDoc );

	if ( lHint )
	{
		if ( UpdateData() )
		{
			UPDATE_DOC( m_Crop, RectInt( m_Left, m_Top, m_Right, m_Bottom ) );
			UPDATE_DOC( m_HotSpot, PointInt( m_HotSpotX, m_HotSpotY ) );
			UPDATE_DOC( m_Resize, SizeInt( m_ResizeWidth, m_ResizeHeight ) );
			UPDATE_DOC( m_DoCrop, m_DoCrop );
			UPDATE_DOC( m_DoResize, m_DoResize );
			UPDATE_DOC( m_KeepFrames, m_KeepFrames );
			UPDATE_DOC( m_Start, m_Start );
			UPDATE_DOC( m_End, m_End );
			UPDATE_DOC( m_ColorKeyEnable, m_ColorKeyEnable );
			UPDATE_DOC( m_ColorKey, Color( m_ColorKey.currentcolor ) );
			UPDATE_DOC( m_bCreateMipMaps, m_bCreateMipMaps );

			CString sFormat;
			m_PixelFormat.GetLBText( m_PixelFormat.GetCurSel(), sFormat );
			ColorFormat::Format eFormat = ColorFormat::findFormat( sFormat );

			UPDATE_DOC( m_eFormat, eFormat );
		}
	}
	else
	{
		m_HotSpotX			= pDoc->m_HotSpot.x;
		m_HotSpotY			= pDoc->m_HotSpot.y;
		m_DoCrop			= pDoc->m_DoCrop;
		m_Left				= pDoc->m_Crop.left;
		m_Top				= pDoc->m_Crop.top;
		m_Right				= pDoc->m_Crop.right;
		m_Bottom			= pDoc->m_Crop.bottom;
		m_DoResize			= pDoc->m_DoResize;
		m_DoResample		= pDoc->m_DoResample;
		m_KeepFrames		= pDoc->m_KeepFrames;
		m_Start				= pDoc->m_Start;
		m_End				= pDoc->m_End;
		m_ResizeWidth		= pDoc->m_Resize.width;
		m_ResizeHeight		= pDoc->m_Resize.height;
		
		m_ColorKeyEnable	= pDoc->m_ColorKeyEnable;
		m_ColorKey.currentcolor = (COLORREF) pDoc->m_ColorKey.RGB();
		
		m_bCreateMipMaps	= pDoc->m_bCreateMipMaps;

		if ( pDoc->m_Image.valid() )
		{
			SizeInt imageSize( pDoc->m_Image->size() );

			m_Width.Format(_T("%d"), imageSize.width );
			m_Height.Format(_T("%d"), imageSize.height );
			m_Frames.Format(_T("%d"), pDoc->m_Image->frameCount() );

			m_SpinStart.SetRange( 0, pDoc->m_Image->frameCount() );
			m_SpinEnd.SetRange( 0, pDoc->m_Image->frameCount() );
			m_SpinBottom.SetRange(0, imageSize.height );
			m_SpinTop.SetRange(0, imageSize.height );
			m_SpinRight.SetRange(0, imageSize.width );
			m_SpinLeft.SetRange(0, imageSize.width );
		}

		UpdateData(FALSE);

		int nSelected = m_PixelFormat.SelectString( -1, ColorFormat::formatText( pDoc->m_eFormat ) );
		if (nSelected >= 0 )
			m_PixelFormat.SetCurSel( nSelected );
	}
}


void CPortView::OnImport() 
{
	ImagePort * pDoc = dynamic_cast<ImagePort *>(GetDocument());
	ASSERT( pDoc );

	OnUpdate(this,true,NULL);

    CFileDialog dialog(TRUE, NULL, NULL, 
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, 
		_T("Image Files|*.tga;*.pcx;*.bmp;*.jpg;*.tif;*.tiff;*.ico;*.pcd;*.png;*.flc;*.avi|")
		_T("FLC|*.flc|")
		_T("TGA|*.tga|")
		_T("PCX|*.pcx|")
		_T("BMP|*.bmp|")
		_T("JPG|*.jpg;*.jpeg|")
		_T("TIF|*.tif;*.tiff|")
		_T("ICO|*.ico|")
		_T("PCD|*.pcd|")
		_T("PNG|*.png|")
		_T("AVI|*.avi|")
		_T("All Files|*.*||"));

	TCHAR fileBuffer[ 32 * 1024 + 1];
	fileBuffer[0] = 0;

	dialog.m_ofn.lpstrFile = fileBuffer;
	dialog.m_ofn.nMaxFile = 32 * 1024;

	if (dialog.DoModal() == IDOK)
	{
		SortedArray< CharString > import;

		POSITION pos = dialog.GetStartPosition();
		while( pos != NULL )
		{
			CString portFile = dialog.GetNextPathName( pos );
			import.insert( String( portFile ) );
		}

		pDoc->m_Image = pDoc->importImages( import );
		pDoc->createResource();
		pDoc->SetModifiedFlag();
		pDoc->UpdateAllViews(NULL);
    }
	
}

void CPortView::OnImportAlpha()
{
	ImagePort * pDoc = dynamic_cast<ImagePort *>(GetDocument());
	ASSERT( pDoc );

	OnUpdate(this,true,NULL);

    CFileDialog dialog(TRUE, NULL, NULL, 
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, 
		_T("Image Files|*.tga;*.pcx;*.bmp;*.jpg;*.tif;*.tiff;*.ico;*.pcd;*.png;*.flc;*.avi|")
		_T("FLC|*.flc|")
		_T("TGA|*.tga|")
		_T("PCX|*.pcx|")
		_T("BMP|*.bmp|")
		_T("JPG|*.jpg;*.jpeg|")
		_T("TIF|*.tif;*.tiff|")
		_T("ICO|*.ico|")
		_T("PCD|*.pcd|")
		_T("PNG|*.png|")
		_T("AVI|*.avi|")
		_T("All Files|*.*||"));

	TCHAR fileBuffer[ 32 * 1024 + 1];
	fileBuffer[0] = 0;

	dialog.m_ofn.lpstrFile = fileBuffer;
	dialog.m_ofn.nMaxFile = 32 * 1024;

	if (dialog.DoModal() == IDOK)
	{
		SortedArray< CharString > import;

		POSITION pos = dialog.GetStartPosition();
		while( pos != NULL )
		{
			CString portFile = dialog.GetNextPathName( pos );
			import.insert( String( portFile ) );
		}

		Image::Ref pAlpha = pDoc->importImages( import );
		pDoc->m_Image->setAlpha( pAlpha );

		pDoc->createResource();
		pDoc->SetModifiedFlag();
		pDoc->UpdateAllViews(NULL);
		Invalidate();
    }
}

void CPortView::OnEdit() 
{
	ImagePort * pDoc = dynamic_cast<ImagePort *>(GetDocument());
	ASSERT( pDoc );

	pDoc->editImage();
	Invalidate();
}


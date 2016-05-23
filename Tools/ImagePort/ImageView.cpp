// ImageView.cpp : implementation file
//

#define IMAGEPORT_DLL
#include "stdafx.h"
#include "ImageView.h"
#include "Port.h"
#include "Resource.h"



/////////////////////////////////////////////////////////////////////////////
// CImageView

IMPLEMENT_DYNCREATE(CImageView, CScrollView)

CImageView::CImageView() : m_pDIB( NULL ), m_pPalette( NULL )
{
	m_Zoom = 1;
	m_Frame = 0;
	m_Scrolling = m_Selecting = m_SetHotSpot = m_SetCapture = m_Playing = false;
}

CImageView::~CImageView()
{
}


BEGIN_MESSAGE_MAP(CImageView, CScrollView)
	//{{AFX_MSG_MAP(CImageView)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEWER_SETHOTSPOT, OnViewerSethotspot)
	ON_COMMAND(ID_VIEWER_SETCAPTURE, OnViewerSetcapture)
	ON_COMMAND(ID_VIEWER_ZOOMIN, OnViewerZoomin)
	ON_COMMAND(ID_VIEWER_ZOOMOUT, OnViewerZoomout)
	ON_COMMAND(ID_VIEWER_PLAYFRAMES, OnViewerPlayframes)
	ON_COMMAND(ID_VIEWER_NEXTFRAME, OnViewerNextframe)
	ON_COMMAND(ID_VIEWER_PREVFRAME, OnViewerPrevframe)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_ZOOMIN, OnUpdateViewerZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_ZOOMOUT, OnUpdateViewerZoomout)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_NEXTFRAME, OnUpdateViewerNextframe)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_PLAYFRAMES, OnUpdateViewerPlayframes)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_PREVFRAME, OnUpdateViewerPrevframe)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_SETCAPTURE, OnUpdateViewerSetcapture)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_SETHOTSPOT, OnUpdateViewerSethotspot)
	ON_COMMAND(ID_VIEWER_EDITFRAME, OnViewerEditframe)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//----------------------------------------------------------------------------

void CImageView::updateStatusBar(void)
{
	CString	status;
	status.Format(_T("FRAME: %d, X: %d, Y: %d"),m_Frame,m_Cursor.x,m_Cursor.y);

	m_StatusBar.SetWindowText(status);
}

void CImageView::drawCursor(CDC* pDC)
{
	int dx = m_Image->size().width * m_Zoom;
	int dy = m_Image->size().height * m_Zoom;

	// draw the cursor
	CPen pen(PS_SOLID,m_Zoom,0x7f7f7f);

	CPen * prevPen = pDC->SelectObject( &pen );
	int prevROP = pDC->SetROP2( R2_XORPEN );

	// draw the cursor
	int x = (m_Cursor.x * m_Zoom) + (m_Zoom / 2);
	int y = (m_Cursor.y * m_Zoom) + (m_Zoom / 2);

	pDC->MoveTo(x,1);
	pDC->LineTo(x,dy);
	pDC->MoveTo(1,y);
	pDC->LineTo(dx,y);

	// restore the write mode
	pDC->SetROP2( prevROP );
	pDC->SelectObject( prevPen );
}

void CImageView::drawCapture(CDC* pDC)
{
	CPen pen(PS_SOLID,m_Zoom,0xffffff);

	CPen *prevPen = pDC->SelectObject( &pen );
	int prevROP = pDC->SetROP2( R2_XORPEN );

	int offset = m_Zoom / 2;
	CRect scaledRect( (m_Capture.left * m_Zoom) + offset,
		(m_Capture.top * m_Zoom) + offset,
		(m_Capture.right * m_Zoom) + offset,
		(m_Capture.bottom * m_Zoom) + offset );

	pDC->MoveTo(scaledRect.left,scaledRect.top);
	pDC->LineTo(scaledRect.right,scaledRect.top);
	pDC->LineTo(scaledRect.right,scaledRect.bottom);
	pDC->LineTo(scaledRect.left,scaledRect.bottom);
	pDC->LineTo(scaledRect.left,scaledRect.top);

	pDC->SetROP2(prevROP);
	pDC->SelectObject(prevPen);
}

enum {
	HOTSPOT_SIZE = 1,
};

void CImageView::drawHotSpot(CDC* pDC)
{
	CPen pen(PS_SOLID,m_Zoom,0xffffff);

	CPen *prevPen = pDC->SelectObject( &pen );
	int prevROP = pDC->SetROP2( R2_XORPEN );

	int x = (m_HotSpot.x * m_Zoom) + (m_Zoom / 2);
	int y = (m_HotSpot.y * m_Zoom) + (m_Zoom / 2);

	int offset = m_Zoom / 2;
	CRect scaledRect( x - (HOTSPOT_SIZE * m_Zoom) - offset,
		y - (HOTSPOT_SIZE * m_Zoom) - offset,
		x + (HOTSPOT_SIZE * m_Zoom) + offset,
		y + (HOTSPOT_SIZE * m_Zoom) + offset );

	pDC->Ellipse( &scaledRect );

	pDC->SetROP2(prevROP);
	pDC->SelectObject(prevPen);
}

//----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CImageView drawing

void CImageView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	// Set the scroll sizes
	SetScrollSizes(MM_TEXT, CSize( m_Image->size().width,  m_Image->size().height ) );
	// resize the parent frame to what CalcWindowRect() 
	ResizeParentToFit();
}

void CImageView::OnDraw(CDC* pDC)
{
	ASSERT( pDC );

	ImagePort * pDoc = dynamic_cast<ImagePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( m_Image->frameCount() > 0)
	{
		m_pDIB = ImagePort::createDIB( *m_Image );

		SizeInt szImage = m_Image->size();
		// copy into 16-byte aligned buffer, otherwise StretchDIBits will fail
		void * pImage = ::malloc( m_Image->frameSize() );
		memcpy( pImage, m_Image->frame( m_Frame ), m_Image->frameSize() );

		SetStretchBltMode( pDC->m_hDC, COLORONCOLOR );
		// blit the pixels
		int r = StretchDIBits(pDC->m_hDC,
			0, 0, szImage.width * m_Zoom, szImage.height * m_Zoom,
			0, 0, szImage.width, szImage.height, pImage, m_pDIB, DIB_RGB_COLORS, SRCCOPY);

		if ( r != szImage.height )
			TRACE("CImageView::OnDraw() - StretchDIBits returned the wrong number of lines!" );

		::free( pImage );
		delete m_pDIB;
		m_pDIB = NULL;
	}

	drawCursor( pDC );
	drawCapture( pDC );
	drawHotSpot( pDC );
}

/////////////////////////////////////////////////////////////////////////////
// CImageView diagnostics

#ifdef _DEBUG
void CImageView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CImageView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CImageView message handlers

int CImageView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create the tool bar
	m_ToolBar.Create( GetParentFrame(), CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|WS_VISIBLE);
	m_ToolBar.LoadToolBar( IDR_PORTBAR );

	// create the status bar
	UINT pane = 0;
	m_StatusBar.Create( GetParentFrame(), CBRS_BOTTOM|WS_VISIBLE);
	m_StatusBar.SetIndicators(&pane,1);

	return 0;
}

void CImageView::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType) 
{
	ImagePort * pDoc = dynamic_cast<ImagePort *>( GetDocument() );
	ASSERT( pDoc );

	CRect clientRect( lpClientRect );
	CPoint clientPoint( clientRect.TopLeft() );

	CSize imageSize( 128, 128 );
	if ( m_Image.valid() )
		imageSize = CSize( m_Image->size().width, m_Image->size().height );

	// get the minimum size of the toolbar
	CSize toolBarSize( m_ToolBar.CalcFixedLayout(FALSE,TRUE) );
	CRect minRect( clientPoint + CPoint(0,toolBarSize.cy), imageSize );
	CRect toolBarRect( clientPoint, toolBarSize );

	clientRect &= minRect;
	clientRect |= toolBarRect;

	CScrollView::CalcWindowRect(&clientRect, nAdjustType);
}

void CImageView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	ImagePort * pDoc = dynamic_cast<ImagePort *>( GetDocument() );
	ASSERT( pDoc );

	// Load the Image
	m_Image = WidgetCast<Image>( pDoc->createResource() );
	if (! m_Image.valid() )
	{
		MessageBox(_T("Failed to import image"));

		// create a dummy image
		m_Image = new Image( ColorFormat::RGB8888, SizeInt(4,4));
	}

	if ( m_Image->format() != ColorFormat::RGB8888 )
		m_Image->setFormat( ColorFormat::RGB8888 );

	// check the width, if not dword aligned resize the image
	if ( (m_Image->size().width % 4) != 0 )
		m_Image->resize( SizeInt( m_Image->size().width & 0xfffffffc, m_Image->size().height) );

	m_Capture.SetRect( pDoc->m_Crop.left, pDoc->m_Crop.top, pDoc->m_Crop.right, pDoc->m_Crop.bottom );
	m_HotSpot.x = pDoc->m_HotSpot.x;
	m_HotSpot.y = pDoc->m_HotSpot.y;
}

void CImageView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CClientDC	dc( this );
	dc.SetViewportOrg( -GetScrollPosition() );

	CPoint scrollPosition = GetScrollPosition();
	if (m_Scrolling)
	{
		CPoint delta = m_Cursor;

		CRect scrollRect;
		GetClientRect(&scrollRect);

		delta.x -= (scrollPosition.x + point.x) / m_Zoom;
		delta.y -= (scrollPosition.y + point.y) / m_Zoom;
		scrollPosition += delta;

		if ( scrollRect.Width() > ( m_Image->size().width * m_Zoom ) )
			scrollPosition.x = 0;
		if ( scrollRect.Height() > ( m_Image->size().height * m_Zoom ) )
			scrollPosition.y = 0;

		ScrollToPosition(scrollPosition);
	}
	else if ( m_Selecting )
	{
		drawCapture(&dc);	// erase previous cursor and capture rect
		drawCursor(&dc);	

		m_Cursor.x = (scrollPosition.x + point.x) / m_Zoom;
		m_Cursor.y = (scrollPosition.y + point.y) / m_Zoom;
		drawCursor(&dc);	// redraw cursor position

		m_Capture.right = m_Cursor.x;
		m_Capture.bottom = m_Cursor.y;
		drawCapture(&dc);
	}
	else
	{
		drawCursor(&dc);

		m_Cursor.x = (scrollPosition.x + point.x) / m_Zoom;
		m_Cursor.y = (scrollPosition.y + point.y) / m_Zoom;
		drawCursor(&dc);
	}

	// update the status bar
	updateStatusBar();

	CScrollView::OnMouseMove(nFlags, point);
}


void CImageView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_SetCapture)
	{
		CClientDC	dc( this );
		dc.SetViewportOrg( -GetScrollPosition() );

		drawCapture(&dc);

		m_Capture.left = m_Capture.right = m_Cursor.x;
		m_Capture.top = m_Capture.bottom = m_Cursor.y;
		m_Selecting = true;
	}
	else if ( !m_SetHotSpot )
		m_Scrolling = true;

	CScrollView::OnLButtonDown(nFlags, point);
}

void CImageView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_Selecting)
	{
		m_Capture.NormalizeRect();

		if ((m_Capture.left == m_Capture.right) &&
			(m_Capture.top == m_Capture.bottom))
		{
			// select entire image
			m_Capture.left = m_Capture.top = 0;
			m_Capture.right = m_Image->size().width - 1;
			m_Capture.bottom = m_Image->size().height - 1;
		}

		ImagePort * pDoc = dynamic_cast<ImagePort *>( GetDocument() );
		ASSERT( pDoc );

		pDoc->m_Crop.set( m_Capture.left, m_Capture.top, m_Capture.right, m_Capture.bottom );
		pDoc->UpdateAllViews( this );
		pDoc->SetModifiedFlag();

		m_Selecting = false;
	}
	else if (m_SetHotSpot)
	{
		CClientDC	dc( this );
		dc.SetViewportOrg( -GetScrollPosition() );

		drawHotSpot(&dc);

		MessageBeep(MB_ICONASTERISK);
		m_HotSpot = m_Cursor;

		ImagePort * pDoc = dynamic_cast<ImagePort *>( GetDocument() );
		ASSERT( pDoc );

		pDoc->m_HotSpot.set( m_HotSpot.x, m_HotSpot.y );
		pDoc->UpdateAllViews( this );
		pDoc->SetModifiedFlag();

		drawHotSpot(&dc);
	}

	m_Scrolling = false;

	CScrollView::OnLButtonUp(nFlags, point);
}

void CImageView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CScrollView::OnRButtonDown(nFlags, point);
}

void CImageView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CScrollView::OnRButtonUp(nFlags, point);
}

BOOL CImageView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

void CImageView::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 1 )
	{
		// Playing animation
		OnViewerNextframe();
	}

	CScrollView::OnTimer(nIDEvent);
}


void CImageView::OnViewerSethotspot() 
{
	if (!m_SetHotSpot && m_SetCapture)
		m_SetCapture = FALSE;
	m_SetHotSpot = !m_SetHotSpot;
}

void CImageView::OnUpdateViewerSethotspot(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_SetHotSpot );
}

void CImageView::OnViewerSetcapture() 
{
	if (!m_SetCapture && m_SetHotSpot )
		m_SetHotSpot = false;
	m_SetCapture = !m_SetCapture;
}

void CImageView::OnUpdateViewerSetcapture(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_SetCapture );
}

void CImageView::OnViewerZoomin() 
{
	if (m_Zoom < 32)
	{
		m_Zoom++;
		
		SetScrollSizes(MM_TEXT, CSize( m_Image->size().width * m_Zoom,  m_Image->size().height * m_Zoom ) );
		Invalidate();
	}
}

void CImageView::OnUpdateViewerZoomin(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Zoom < 32 );	
}

void CImageView::OnViewerZoomout() 
{
	if (m_Zoom > 1)
	{
		m_Zoom--;
		
		SetScrollSizes(MM_TEXT, CSize( m_Image->size().width * m_Zoom,  m_Image->size().height * m_Zoom ) );
		Invalidate();
	}
}

void CImageView::OnUpdateViewerZoomout(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Zoom > 1 );	
}

void CImageView::OnViewerPlayframes() 
{
	m_Playing = !m_Playing;

	if (m_Playing)
		SetTimer( 1, 1000 / 15 ,NULL);	// Play @ 15 FPS
	else
		KillTimer( 1 );
}

void CImageView::OnViewerNextframe() 
{
	if (m_Image->frameCount() > 1)
	{
		m_Frame++;
		if (m_Frame >= m_Image->frameCount())
			m_Frame = 0;

		//Invalidate();
		updateStatusBar();

		CClientDC dc( this );
		OnDraw( &dc );
	}
}

void CImageView::OnViewerPrevframe() 
{
	if (m_Image->frameCount() > 1)
	{
		m_Frame--;
		if (m_Frame < 0)
			m_Frame = m_Image->frameCount() - 1;

		//Invalidate();
		updateStatusBar();

		CClientDC dc( this );
		OnDraw( &dc );	
	}
}

void CImageView::OnUpdateViewerNextframe(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Image->frameCount() > 1 );
}

void CImageView::OnUpdateViewerPlayframes(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Image->frameCount() > 1 );
	pCmdUI->SetCheck( m_Playing );
}

void CImageView::OnUpdateViewerPrevframe(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Image->frameCount() > 1 );
}

//----------------------------------------------------------------------------


void CImageView::OnViewerEditframe() 
{
	ImagePort * pDoc = dynamic_cast<ImagePort *>( GetDocument() );
	ASSERT( pDoc );

	// edit the current frame
	pDoc->editImage();
}

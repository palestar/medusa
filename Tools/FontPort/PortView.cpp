// PortView.cpp : implementation file
//

#define FONTPORT_DLL
#include "stdafx.h"
#include "PortView.h"

#include "Port.h"

#include "Draw/Font.h"
#include "Draw/Draw.h"



/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD), m_nOffset( 0 )

{
	//{{AFX_DATA_INIT(CPortView)
	m_Characters = _T("");
	m_Height = _T("");
	m_Width = _T("");
	m_Offset = _T("");
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Control(pDX, IDC_PREVIEW, m_Preview);
	DDX_Text(pDX, IDC_CHARACTERS, m_Characters);
	DDX_Text(pDX, IDC_HEIGHT, m_Height);
	DDX_Text(pDX, IDC_WIDTH, m_Width);
	DDX_Text(pDX, IDC_OFFSET, m_Offset);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON1, OnPickFont)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON3, OnPrevious)
	ON_BN_CLICKED(IDC_BUTTON2, OnNext)
	//}}AFX_MSG_MAP
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

void CPortView::OnPickFont() 
{
	CFontDialog	dialog( &m_LogicalFont );
	if (dialog.DoModal() == IDOK)
	{
		dialog.GetCurrentFont( &m_LogicalFont );

		FontPort * pDoc = dynamic_cast<FontPort *>(GetDocument());
		ASSERT( pDoc );

		// update the document
		pDoc->m_LogicalFont	= m_LogicalFont;
		pDoc->SetModifiedFlag();

		m_nOffset = 0;
		updateFontInformation();
	}
}

/*
void CPortView::OnCancel() 
{
	FontPort * pDoc = dynamic_cast<FontPort *>(GetDocument());
	ASSERT( pDoc );

	pDoc->unlock( false );					// abort save
	GetParent()->DestroyWindow();		// close view
}

void CPortView::OnOkay() 
{
	FontPort * pDoc = dynamic_cast<FontPort *>(GetDocument());
	ASSERT( pDoc );

	pDoc->m_LogicalFont	= m_LogicalFont;

	pDoc->setModified( true );
	pDoc->unlock();				// save port

	GetParent()->DestroyWindow();	// close view
}
*/

void CPortView::OnDestroy() 
{
	CFormView::OnDestroy();
}

void CPortView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	FontPort * pDoc = dynamic_cast<FontPort *>(GetDocument());
	ASSERT( pDoc );

	// initialize preview
	m_Device = DisplayDevice::create( "DisplayDeviceD3D" );
	m_Device->initialize( m_Preview.m_hWnd, NULL, true, true );

	m_LogicalFont = pDoc->m_LogicalFont;
	updateFontInformation();
}

void CPortView::OnDraw(CDC* pDC) 
{
	if (! m_Device )
		return;

	m_Device->lock();
	ASSERT( m_Font.valid() );

	CRect rect;
	m_Preview.GetClientRect( rect );

	// clear the primary surface
	m_Device->clear( Color(0,0,0) );
	m_Device->beginScene();

	SizeInt render = m_Device->renderWindow().size();
	SizeInt fontSize = m_Font->size();

	wchar c = m_nOffset;
	wchar cFirst = m_Font->asciiOffset();
	wchar cLast = cFirst + m_Font->characterCount();

	int cy = 0;
	while( cy < render.height )
	{
		String out;
		while( m_Font->size( out ).width < (render.width - fontSize.width) )
		{
			if ( c >= cFirst && c <= cLast )
				out += c;
			c++;
		}

		// render the line
		Font::push( m_Device, m_Font, PointInt( 0, cy), out, Color(255,255,255,255) );
		cy += fontSize.height;
	}

	// show the drawn image
	m_Device->endScene();
	m_Device->present();
	m_Device->unlock();
}

void CPortView::updateFontInformation()
{
	FontPort * pDoc = dynamic_cast<FontPort *>(GetDocument());
	ASSERT( pDoc );

	// create the font
	m_Font = WidgetCast<Font>( pDoc->createResource() );
	ASSERT( m_Font.valid() );

	OnUpdate( NULL, 0, NULL );
}

void CPortView::OnPrevious() 
{
	m_nOffset -= 512;
	OnUpdate( NULL, 0, NULL );
}

void CPortView::OnNext() 
{
	m_nOffset += 512;
	OnUpdate( NULL, 0, NULL );
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if ( m_Font.valid() )
	{
		m_Characters.Format(_T("%d"), m_Font->characterCount() );
		m_Offset.Format(_T("%d"), wchar(m_nOffset) );
		m_Width.Format(_T("%d"), m_Font->size().width );
		m_Height.Format(_T("%d"), m_Font->size().height );
		UpdateData( false );

		OnDraw(NULL);
	}
}

// PortView.cpp : implementation file
//

#define NODEGLAREPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"



/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_Intensity = 0.0f;
	m_IntensityV = 0.0f;
	m_IntensityA = 0.0f;
	m_Focus = 0.0f;
	m_Range = 0.0f;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_EDIT1, m_Intensity);
	DDX_Text(pDX, IDC_EDIT2, m_IntensityV);
	DDX_Text(pDX, IDC_EDIT3, m_IntensityA);
	DDX_Text(pDX, IDC_EDIT4, m_Focus);
	DDX_Text(pDX, IDC_EDIT5, m_Range);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnUpdateNode)
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

void CPortView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	m_Color.SubclassDlgItem(IDC_COLOR,this); 	
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeGlarePort * pDoc = dynamic_cast<NodeGlarePort *>( GetDocument() );
	if ( pDoc == NULL )
		return;

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Color = Color( m_Color.currentcolor );
		pDoc->m_Intensity = m_Intensity;
		pDoc->m_IntensityV = m_IntensityV;
		pDoc->m_IntensityA = m_IntensityA;
		pDoc->m_Focus = m_Focus;
		pDoc->m_Range = m_Range;

		pDoc->setUpdate();
		pDoc->updateParent();
		pDoc->UpdateAllViews( NULL );
	}
	else
	{
		m_Color.currentcolor = (COLORREF) pDoc->m_Color.RGB();
		m_Intensity = pDoc->m_Intensity;
		m_IntensityV = pDoc->m_IntensityV;
		m_IntensityA = pDoc->m_IntensityA;
		m_Focus = pDoc->m_Focus;
		m_Range = pDoc->m_Range;

		UpdateData( false );
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate( NULL, true, NULL );
}

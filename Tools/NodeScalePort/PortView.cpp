// PortView.cpp : implementation file
//

#define NODESCALEPORT_DLL
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
	m_XV = 0.0f;
	m_XA = 0.0f;
	m_YV = 0.0f;
	m_YA = 0.0f;
	m_ZV = 0.0f;
	m_ZA = 0.0f;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_EDIT1, m_XV);
	DDX_Text(pDX, IDC_EDIT2, m_XA);
	DDX_Text(pDX, IDC_EDIT3, m_YV);
	DDX_Text(pDX, IDC_EDIT4, m_YA);
	DDX_Text(pDX, IDC_EDIT5, m_ZV);
	DDX_Text(pDX, IDC_EDIT6, m_ZA);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnUpdateNode)
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
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeScalePort * pDoc = dynamic_cast<NodeScalePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_XV = m_XV;
		pDoc->m_XA = m_XA;
		pDoc->m_YV = m_YV;
		pDoc->m_YA = m_YA;
		pDoc->m_ZV = m_ZV;
		pDoc->m_ZA = m_ZA;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_XV = pDoc->m_XV;
		m_XA = pDoc->m_XA;
		m_YV = pDoc->m_YV;
		m_YA = pDoc->m_YA;
		m_ZV = pDoc->m_ZV;
		m_ZA = pDoc->m_ZA;

		UpdateData( false );
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate( NULL, true, NULL );
}

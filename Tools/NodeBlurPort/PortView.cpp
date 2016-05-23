// PortView.cpp : implementation file
//

#define NODEBLURPORT_DLL
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
	m_Blending = -1;
	m_Time = 0.0f;
	m_Steps = 0;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_CBIndex(pDX, IDC_COMBO1, m_Blending);
	DDX_Text(pDX, IDC_EDIT1, m_Time);
	DDV_MinMaxFloat(pDX, m_Time, 0.f, 1000.f);
	DDX_Text(pDX, IDC_EDIT2, m_Steps);
	DDV_MinMaxInt(pDX, m_Steps, 1, 10);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
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
	NodeBlurPort * pDoc = dynamic_cast<NodeBlurPort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Time = m_Time;
		pDoc->m_Steps = m_Steps;
		pDoc->m_Blending = (NodeBlur::Blending)m_Blending;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_Time = pDoc->m_Time;
		m_Steps = pDoc->m_Steps;
		m_Blending = pDoc->m_Blending;

		UpdateData( false );
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate( NULL, true, NULL );
}

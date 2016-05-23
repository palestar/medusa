// PortView.cpp : implementation file
//

#define NODEPATHPORT_DLL
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
	m_VX = 0.0f;
	m_VY = 0.0f;
	m_VZ = 0.0f;
	m_AX = 0.0f;
	m_AY = 0.0f;
	m_AZ = 0.0f;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_EDIT1, m_VX);
	DDX_Text(pDX, IDC_EDIT2, m_VY);
	DDX_Text(pDX, IDC_EDIT3, m_VZ);
	DDX_Text(pDX, IDC_EDIT4, m_AX);
	DDX_Text(pDX, IDC_EDIT5, m_AY);
	DDX_Text(pDX, IDC_EDIT6, m_AZ);
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
	NodePathPort * pDoc = dynamic_cast<NodePathPort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Velocity.x = m_VX;
		pDoc->m_Velocity.y = m_VY;
		pDoc->m_Velocity.z = m_VZ;
		pDoc->m_Acceleration.x = m_AX;
		pDoc->m_Acceleration.y = m_AY;
		pDoc->m_Acceleration.z = m_AZ;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_VX = pDoc->m_Velocity.x;
		m_VY = pDoc->m_Velocity.y;
		m_VZ = pDoc->m_Velocity.z;
		m_AX = pDoc->m_Acceleration.x;
		m_AY = pDoc->m_Acceleration.y;
		m_AZ = pDoc->m_Acceleration.z;

		UpdateData( false );
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate( NULL, true, NULL );
}

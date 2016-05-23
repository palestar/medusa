// PortView.cpp : implementation file
//

#define NODEBOOLEANTIMEPORT_DLL
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
	m_Begin = 0.0f;
	m_End = 0.0f;
	m_Invert = FALSE;
	m_WarpTime = FALSE;
	m_LoopTime = 0.0f;
	m_Loop = FALSE;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_EDIT1, m_Begin);
	DDX_Text(pDX, IDC_EDIT2, m_End);
	DDX_Check(pDX, IDC_CHECK1, m_Invert);
	DDX_Check(pDX, IDC_CHECK2, m_WarpTime);
	DDX_Text(pDX, IDC_EDIT3, m_LoopTime);
	DDX_Check(pDX, IDC_CHECK3, m_Loop);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK2, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK3, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnUpdateNode)
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

#pragma warning(disable:4800)

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeBooleanTimePort * pDoc = dynamic_cast<NodeBooleanTimePort *>( GetDocument() );
	if ( pDoc == NULL )
		return;

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Begin = m_Begin;
		pDoc->m_End = m_End;
		pDoc->m_Invert = m_Invert;
		pDoc->m_WarpTime = m_WarpTime;
		pDoc->m_Loop = m_Loop;
		pDoc->m_LoopTime = m_LoopTime;

		pDoc->setUpdate();
		pDoc->updateParent();
		pDoc->UpdateAllViews( NULL );
	}
	else
	{
		m_Begin = pDoc->m_Begin;
		m_End = pDoc->m_End;
		m_Invert = pDoc->m_Invert;
		m_WarpTime = pDoc->m_WarpTime;
		m_Loop = pDoc->m_Loop;
		m_LoopTime = pDoc->m_LoopTime;

		UpdateData( false );
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate( NULL, true, NULL );
}

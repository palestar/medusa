// PortView.cpp : implementation file
//

#include "stdafx.h"
#include "Port.h"
#include "PortView.h"



/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_Bits = _T("");
	m_Type = -1;
	m_MinDistance = 0.0f;
	m_MaxDistance = 0.0f;
	m_MinDetailLevel = -1;
	m_MaxDetailLevel = -1;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_EDIT1, m_Bits);
	DDX_CBIndex(pDX, IDC_COMBO1, m_Type);
	DDX_Text(pDX, IDC_EDIT2, m_MinDistance);
	DDX_Text(pDX, IDC_EDIT3, m_MaxDistance);
	DDX_CBIndex(pDX, IDC_COMBO2, m_MinDetailLevel);
	DDX_CBIndex(pDX, IDC_COMBO3, m_MaxDetailLevel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnUpdateNode)
	ON_CBN_SELCHANGE(IDC_COMBO3, OnUpdateNode)
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
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	CFormView::OnInitialUpdate();
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeBooleanPort * pDoc = dynamic_cast<NodeBooleanPort *>( GetDocument() );
	if ( pDoc == NULL )
		return;

	if ( lHint )
	{
		UpdateData( true );

		sscanf( m_Bits, "%x", &pDoc->m_Bits );
		pDoc->m_Type = (NodeBoolean::Type)m_Type;
		pDoc->m_MinDistance = m_MinDistance;
		pDoc->m_MaxDistance = m_MaxDistance;
		pDoc->m_MinDetailLevel = (NodeBooleanPort::DetailLevel)m_MinDetailLevel;
		pDoc->m_MaxDetailLevel = (NodeBooleanPort::DetailLevel)m_MaxDetailLevel;

		pDoc->setUpdate();
		pDoc->updateParent();
		pDoc->UpdateAllViews( NULL );
	}
	else
	{
		m_Type = pDoc->m_Type;
		m_Bits.Format("0x%8.8x", pDoc->m_Bits );
		m_MinDistance = pDoc->m_MinDistance;
		m_MaxDistance = pDoc->m_MaxDistance;
		m_MinDetailLevel = pDoc->m_MinDetailLevel;
		m_MaxDetailLevel = pDoc->m_MaxDetailLevel;

		UpdateData( false );
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate(this,true,NULL);
}


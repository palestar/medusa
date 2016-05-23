// PortView.cpp : implementation file
//

#define NODESPINPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "Math/Helpers.h"



/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_Pitch = 0.0f;
	m_Roll = 0.0f;
	m_Yaw = 0.0f;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_PITCH, m_Pitch);
	DDX_Text(pDX, IDC_ROLL, m_Roll);
	DDX_Text(pDX, IDC_YAW, m_Yaw);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_EN_KILLFOCUS(IDC_PITCH, OnUpdateSpin)
	ON_EN_KILLFOCUS(IDC_ROLL, OnUpdateSpin)
	ON_EN_KILLFOCUS(IDC_YAW, OnUpdateSpin)
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

void CPortView::OnUpdateSpin() 
{
	OnUpdate(this,true,NULL);
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeSpinPort * pDoc = dynamic_cast<NodeSpinPort *>( GetDocument() );
	if ( pDoc == NULL )
		return;

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Pitch = DegToRad( m_Pitch );
		pDoc->m_Roll = DegToRad( m_Roll );
		pDoc->m_Yaw = DegToRad( m_Yaw );

		pDoc->UpdateAllViews( this );
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_Pitch = RadToDeg( pDoc->m_Pitch );
		m_Roll = RadToDeg( pDoc->m_Roll );
		m_Yaw = RadToDeg( pDoc->m_Yaw );

		UpdateData( false );
	}
}

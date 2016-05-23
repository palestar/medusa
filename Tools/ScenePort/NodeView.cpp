// NodeView.cpp : implementation file
//

#pragma warning( disable: 4786 )	// identifier was truncated to '255' characters in the browser information

#define SCENE_PORT_DLL
#include "stdafx.h"
#include "NodeView.h" 
#include "NodePort.h"

#include "Math/Helpers.h"



/////////////////////////////////////////////////////////////////////////////
// CNodeView

IMPLEMENT_DYNCREATE(CNodeView, CFormView)

CNodeView::CNodeView()
	: CFormView(CNodeView::IDD)
{
	//{{AFX_DATA_INIT(CNodeView)
	m_X = 0.0f;
	m_Y = 0.0f;
	m_Z = 0.0f;
	m_IX = 0.0f;
	m_IY = 0.0f;
	m_IZ = 0.0f;
	m_JX = 0.0f;
	m_JY = 0.0f;
	m_JZ = 0.0f;
	m_KX = 0.0f;
	m_KY = 0.0f;
	m_KZ = 0.0f;
	m_Scale = 0.0f;
	//}}AFX_DATA_INIT

	m_ControlUpdate = false;
}

CNodeView::~CNodeView()
{
}

void CNodeView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNodeView)
	DDX_Text(pDX, IDC_X, m_X);
	DDX_Text(pDX, IDC_Y, m_Y);
	DDX_Text(pDX, IDC_Z, m_Z);
	DDX_Text(pDX, IDC_IX, m_IX);
	DDX_Text(pDX, IDC_IY, m_IY);
	DDX_Text(pDX, IDC_IZ, m_IZ);
	DDX_Text(pDX, IDC_JX, m_JX);
	DDX_Text(pDX, IDC_JY, m_JY);
	DDX_Text(pDX, IDC_JZ, m_JZ);
	DDX_Text(pDX, IDC_KX, m_KX);
	DDX_Text(pDX, IDC_KY, m_KY);
	DDX_Text(pDX, IDC_KZ, m_KZ);
	DDX_Text(pDX, IDC_EDIT1, m_Scale);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNodeView, CFormView)
	//{{AFX_MSG_MAP(CNodeView)
	ON_BN_CLICKED(IDC_BUTTON2, OnIdentity)
	ON_EN_KILLFOCUS(IDC_IX, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_IY, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_IZ, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_JX, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_JY, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_JZ, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_KX, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_KY, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_KZ, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_X, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_Y, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_Z, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNodeView diagnostics

#ifdef _DEBUG
void CNodeView::AssertValid() const
{
	CFormView::AssertValid();
}

void CNodeView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNodeView message handlers

BOOL CNodeView::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CFormView::PreCreateWindow(cs);
}

void CNodeView::OnInitialUpdate() 
{
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	CFormView::OnInitialUpdate();
}

void CNodeView::OnUpdateNode() 
{
	OnUpdate(this,true,NULL);
}

void CNodeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodePort * pPort = dynamic_cast<NodePort *>( GetDocument() );
	if ( pPort == NULL ) 
		return;

	if ( lHint )
	{
		// get the data from the dialog
		UpdateData( true );

		// update the node position
		pPort->m_Position.x = m_X;
		pPort->m_Position.y = m_Y;
		pPort->m_Position.z = m_Z;
		
		pPort->m_Frame.i.x = m_IX;
		pPort->m_Frame.i.y = m_IY;
		pPort->m_Frame.i.z = m_IZ;

		pPort->m_Frame.j.x = m_JX;
		pPort->m_Frame.j.y = m_JY;
		pPort->m_Frame.j.z = m_JZ;

		pPort->m_Frame.k.x = m_KX;
		pPort->m_Frame.k.y = m_KY;
		pPort->m_Frame.k.z = m_KZ;

		pPort->m_Scale = m_Scale;

		pPort->UpdateAllViews( this );

		pPort->setUpdate();
		pPort->updateParent();
	}
	else
	{
		m_X = pPort->m_Position.x;
		m_Y = pPort->m_Position.y;
		m_Z = pPort->m_Position.z;

		m_IX = pPort->m_Frame.i.x;
		m_IY = pPort->m_Frame.i.y;
		m_IZ = pPort->m_Frame.i.z;

		m_JX = pPort->m_Frame.j.x;
		m_JY = pPort->m_Frame.j.y;
		m_JZ = pPort->m_Frame.j.z;

		m_KX = pPort->m_Frame.k.x;
		m_KY = pPort->m_Frame.k.y;
		m_KZ = pPort->m_Frame.k.z;

		m_Scale = pPort->m_Scale;

		UpdateData( false );
	}
}



void CNodeView::OnIdentity() 
{
	NodePort * pDoc = dynamic_cast<NodePort *>( GetDocument() );
	ASSERT( pDoc );

	pDoc->m_Frame.identity();
	pDoc->m_Scale = 1.0f;
	pDoc->UpdateAllViews( NULL );
}

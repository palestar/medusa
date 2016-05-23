// PortView.cpp : implementation file
//

#define NODEFLAREPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "Tools/ResourcerDoc/SelectResource.h"
#include "Tools/MaterialPort/Port.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_MaterialPort = _T("");
	m_Scale = 0.0f;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_EDIT1, m_MaterialPort);
	DDX_Text(pDX, IDC_EDIT2, m_Scale);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON1, OnOpenMaterial)
	ON_BN_CLICKED(IDC_BUTTON2, OnBrowseMaterial)
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
	NodeFlarePort * pDoc = dynamic_cast<NodeFlarePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Scale = m_Scale;

		pDoc->UpdateAllViews( NULL );
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_MaterialPort = (const char *)pDoc->m_MaterialPort;
		m_Scale = pDoc->m_Scale;

		UpdateData( false );
	}
}

void CPortView::OnOpenMaterial() 
{
	AfxGetApp()->OpenDocumentFile( m_MaterialPort );
}

void CPortView::OnBrowseMaterial() 
{
	NodeFlarePort * pDoc = dynamic_cast<NodeFlarePort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectResource dialog( CLASS_KEY( MaterialPort ) );
	if ( dialog.DoModal() == IDOK)
	{
		pDoc->m_MaterialPort = (const char *)dialog.m_Selected;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();	
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate( NULL, true, NULL );
}

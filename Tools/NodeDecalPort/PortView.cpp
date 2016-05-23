// PortView.cpp : implementation file
//

#define NODEDECALPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "Tools/MaterialPort/Port.h"
#include "Tools/ResourcerDoc/SelectResource.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_Material = _T("");
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_EDIT1, m_Material);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON1, OnOpenMaterial)
	ON_BN_CLICKED(IDC_BUTTON2, OnBrowseMaterial)
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

void CPortView::OnOpenMaterial() 
{
	AfxGetApp()->OpenDocumentFile( m_Material );
}

void CPortView::OnBrowseMaterial() 
{
	NodeDecalPort * pDoc = dynamic_cast<NodeDecalPort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectResource dialog( CLASS_KEY( MaterialPort ) );
	if ( dialog.DoModal() == IDOK)
	{
		pDoc->m_Material = (const char *)dialog.m_Selected;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();	
	}
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeDecalPort * pDoc = dynamic_cast<NodeDecalPort *>( GetDocument() );
	if ( pDoc != NULL )
	{
		if ( lHint )
		{
			UpdateData( true );

			pDoc->UpdateAllViews(NULL);
			pDoc->setUpdate();
			pDoc->updateParent();	
		}
		else
		{
			m_Material = (const char *)pDoc->m_Material;
			UpdateData( false );
		}
	}
}

void CPortView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);
}

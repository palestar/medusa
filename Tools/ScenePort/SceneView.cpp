// SceneView.cpp : implementation file
//

#include "stdafx.h"
#include "SceneView.h" 
#include "Port.h"

#include "Standard/Bits.h"

/////////////////////////////////////////////////////////////////////////////
// CSceneView

IMPLEMENT_DYNCREATE(CSceneView, CFormView)

CSceneView::CSceneView()
	: CFormView(CSceneView::IDD)
{
	//{{AFX_DATA_INIT(CSceneView)
	m_Fps = 0.0f;
	//}}AFX_DATA_INIT
}

CSceneView::~CSceneView()
{
}

void CSceneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSceneView)
	DDX_Text(pDX, IDC_EDIT3, m_Fps);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSceneView, CFormView)
	//{{AFX_MSG_MAP(CSceneView)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnUpdateScene)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSceneView diagnostics

#ifdef _DEBUG
void CSceneView::AssertValid() const
{
	CFormView::AssertValid();
}

void CSceneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSceneView message handlers

void CSceneView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);
}

void CSceneView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Fps = m_Fps;
		pDoc->UpdateAllViews( NULL );
	}
	else
	{
		m_Fps = pDoc->m_Fps;
		UpdateData( false );
	}
}

void CSceneView::OnUpdateScene() 
{
	// get the data from the dialog
	OnUpdate(this,true,NULL);
}

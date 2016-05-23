// PortView.cpp : implementation file
//

#define NODESCENEPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "Render3d/NodeScene.h"
#include "Render3d/Scene.h"

#include "Tools/ScenePort/Port.h"
#include "Tools/ResourcerDoc/Port.h"
#include "Tools/ResourcerDoc/ResourcerDoc.h"
#include "Tools/ResourcerDoc/SelectResource.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_SceneName = _T("");
	m_NormalizeFrame = FALSE;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_SCENE_NAME, m_SceneName);
	DDX_Check(pDX, IDC_CHECK1, m_NormalizeFrame);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_SCENE_BROWSE, OnSceneBrowse)
	ON_BN_CLICKED(IDC_SCENE_PROPERTIES, OnSceneProperties)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateNode)
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

//-------------------------------------------------------------------------------

void CPortView::OnSceneBrowse() 
{
	NodeScenePort * pPort = dynamic_cast<NodeScenePort *>( GetDocument() );
	ASSERT( pPort );

	UpdateData( true );

	CSelectResource dialog( CLASS_KEY(ScenePort) );
	if ( dialog.DoModal() == IDOK)
	{
		m_SceneName = dialog.m_Selected;
		UpdateData( false );

		pPort->m_ScenePort = (const char *)m_SceneName;
		pPort->setUpdate();
		pPort->updateParent();	
	}
}

void CPortView::OnSceneProperties() 
{
	AfxGetApp()->OpenDocumentFile( m_SceneName );
}

void CPortView::OnInitialUpdate() 
{
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	CFormView::OnInitialUpdate();
}

#pragma warning( disable: 4800 )

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeScenePort * pDoc = dynamic_cast<NodeScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_NormalizeFrame = m_NormalizeFrame;

		pDoc->UpdateAllViews( this );
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_SceneName = (const char *)pDoc->m_ScenePort;
		m_NormalizeFrame = pDoc->m_NormalizeFrame;

		UpdateData( false );
	}
}

/*
void CPortView::OnSelectBits() 
{
	NodeScenePort * pDoc = dynamic_cast<NodeScenePort *>( GetDocument() );
	ASSERT( pDoc );

	ScenePort::Ref scene = dynamic_cast<ScenePort *>( Port::portLoad( pDoc->m_ScenePort ) );
	if ( scene.valid() )
	{
		CSelectFlags dialog( scene, pDoc->m_Bits );
		if ( dialog.DoModal() == IDOK )
		{
			pDoc->m_Bits = dialog.m_Bits;
			pDoc->UpdateAllViews( NULL );
			pDoc->setUpdate();
			pDoc->updateParent();
		}
	}
	else
		MessageBox("Failed to load scene");
}


void CPortView::OnUpdateNode() 
{
	// edit control loss focus, update the document
	OnUpdate( this, true, NULL );
}
*/

void CPortView::OnUpdateNode() 
{
	OnUpdate( NULL, true, NULL );
}

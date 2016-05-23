// ChildFrame.cpp : implementation file
//

#pragma warning( disable: 4786 )	// identifier was truncated to '255' characters in the browser information

#include "stdafx.h"
#include "ChildFrame.h" 

#include "SceneTree.h"
#include "SceneRender.h"
#include "SegmentView.h"
#include "SceneBits.h"
#include "SceneView.h"
#include "PropertyView.h"
#include "BaseNodePort.h"
#include "Port.h"

#include "Render3d/NodeBoolean.h"
#include "Render3d/NodeTransform.h"

#include "resource.h"

extern AFX_EXTENSION_MODULE g_PortDLL;

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

CChildFrame::CChildFrame()
{
}

CChildFrame::~CChildFrame()
{
}


BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_COMMAND(ID_SCENE_EDITSEGMENTS, OnSceneEditsegments)
	ON_COMMAND(ID_SCENE_PROPERTIES, OnSceneProperties)
	ON_WM_MDIACTIVATE()
	ON_COMMAND(ID_SCENE_BITS, OnSceneBits)
	ON_COMMAND(ID_RENDER_SDETAILLEVELLOW, OnRenderSdetaillevellow)
	ON_UPDATE_COMMAND_UI(ID_RENDER_SDETAILLEVELLOW, OnUpdateRenderSdetaillevellow)
	ON_COMMAND(ID_RENDER_SDETAILEVELMEDIUM, OnRenderSdetailevelmedium)
	ON_UPDATE_COMMAND_UI(ID_RENDER_SDETAILEVELMEDIUM, OnUpdateRenderSdetailevelmedium)
	ON_COMMAND(ID_RENDER_SDETAILLEVELHIGH, OnRenderSdetaillevelhigh)
	ON_UPDATE_COMMAND_UI(ID_RENDER_SDETAILLEVELHIGH, OnUpdateRenderSdetaillevelhigh)
	ON_COMMAND(ID_RENDER_SSHOWALIGNEDHULL, OnRenderSshowalignedhull)
	ON_UPDATE_COMMAND_UI(ID_RENDER_SSHOWALIGNEDHULL, OnUpdateRenderSshowalignedhull)
	ON_COMMAND(ID_RENDER_SSHOWHULL, OnRenderSshowhull)
	ON_UPDATE_COMMAND_UI(ID_RENDER_SSHOWHULL, OnUpdateRenderSshowhull)
	ON_COMMAND(ID_RENDER_SSHOWNODEAXIS, OnRenderSshownodeaxis)
	ON_UPDATE_COMMAND_UI(ID_RENDER_SSHOWNODEAXIS, OnUpdateRenderSshownodeaxis)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

BOOL CChildFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CMDIFrameWnd* pParentWnd, CCreateContext* pContext) 
{
	// we want to load a specific menu from this DLL
	HINSTANCE hInstOld = AfxGetResourceHandle();
	AfxSetResourceHandle(g_PortDLL.hModule);

	if (!m_Menu.LoadMenu(IDR_PORT))
	{
		// restore the old resource chain and return error
		AfxSetResourceHandle(hInstOld);
		return FALSE;
	}
	AfxSetResourceHandle(hInstOld); // restore the old resource chain

	// when the list output frame window is active, use this menu
	m_hMenuShared = m_Menu.m_hMenu;
	
	return CMDIChildWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, pContext);
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	if ( pContext->m_pNewViewClass != NULL )
		return( CMDIChildWnd::OnCreateClient(lpcs, pContext) );

	// create the main ScenePort window
	//
	if (!m_Splitter.CreateStatic(this,1,2))
		return( FALSE );

	int nLeftSize = lpcs->cx / 4;
	int nRightSize = lpcs->cx - nLeftSize;
	m_Splitter.CreateView(0,0,RUNTIME_CLASS(CView),CSize(nLeftSize,lpcs->cy),pContext);
	m_Splitter.CreateView(0,1,RUNTIME_CLASS(CSceneRender),CSize(nRightSize,lpcs->cy),pContext);

	if (!m_Splitter2.CreateStatic( &m_Splitter, 2, 1, WS_CHILD|WS_VISIBLE, m_Splitter.IdFromRowCol(0,0) ) )
		return FALSE;

	int nTopSize = lpcs->cy / 3;
	int nBottomSize = lpcs->cy - nTopSize;
	m_Splitter2.CreateView(0,0,RUNTIME_CLASS(CSceneTree),CSize(nLeftSize, nTopSize), pContext );
	m_Splitter2.CreateView(1,0,RUNTIME_CLASS(CPropertyView),CSize(nLeftSize, nBottomSize), pContext );

	EnableDocking( CBRS_ALIGN_ANY );

	// create the tool barw
	m_ToolBar.Create( this, CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|WS_VISIBLE);
	m_ToolBar.LoadToolBar( IDR_PORTBAR );
	m_ToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_ToolBar,AFX_IDW_DOCKBAR_TOP);

	// create the status bar
	UINT pane = 0;
	m_StatusBar.Create( this, CBRS_BOTTOM|WS_VISIBLE);
	m_StatusBar.SetIndicators(&pane,1);

	/*
	// Add tool-bar with all the available nodes types, TODO: add tool-tips also
	//

	m_NodeBar.Create( this );

	#define BAR_CX			(16)
	#define BAR_CY			(16)

	CImageList * pImageList = new CImageList;
	pImageList->Create( BAR_CX,BAR_CY,ILC_COLOR32 | ILC_MASK,0,0); 

	for(int k=0;k<BaseNodePort::portCount();k++)
		pImageList->Add( BaseNodePort::portIcon( BaseNodePort::port( k )->classKey() ) );

	m_NodeBar.GetToolBarCtrl().SetImageList( pImageList );
	m_NodeBar.SetSizes( CSize( BAR_CX + 7,BAR_CY + 6 ), CSize( BAR_CX, BAR_CY ) );
	m_NodeBar.SetBarStyle(m_NodeBar.GetBarStyle() | CBRS_SIZE_DYNAMIC);

	// add the resource types to the toolbar for creation
	for(int j=0;j< BaseNodePort::portCount() ;j++)
	{
		TBBUTTON nbutton;
		nbutton.iBitmap = j;

		nbutton.idCommand = ID_NODE_PORT_BEGIN + j;
		nbutton.fsState = TBSTATE_ENABLED;
		nbutton.fsStyle = TBSTYLE_BUTTON;
		nbutton.dwData = NULL;
		nbutton.iString = NULL; 
		
		m_NodeBar.GetToolBarCtrl().AddButtons(1, &nbutton);
	}

	m_NodeBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_NodeBar,AFX_IDW_DOCKBAR_LEFT);
	*/

	return(TRUE);
}

void CChildFrame::OnSceneEditsegments() 
{
	// get the document
	CDocument * pDoc = GetActiveView()->GetDocument();
	ASSERT( pDoc );

	// CMDIChildWnd
	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CSegmentView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( pDoc, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, pDoc);
}


void CChildFrame::OnSceneProperties() 
{
	// get the document
	CDocument * pDoc = GetActiveView()->GetDocument();
	ASSERT( pDoc );

	// CMDIChildWnd
	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CSceneView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( pDoc, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, pDoc);
}

void CChildFrame::OnSceneBits() 
{
	// get the document
	CDocument * pDoc = GetActiveView()->GetDocument();
	ASSERT( pDoc );

	// CMDIChildWnd
	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CSceneBits) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( pDoc, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, pDoc);
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	
	if (bActivate)
	{
		CMenu* pTopMenu = AfxGetMainWnd()->GetMenu();
		for(unsigned int i = 0; i < pTopMenu->GetMenuItemCount(); i++)
		{
			CMenu *pSubMenu = pTopMenu->GetSubMenu(i);
			if (pSubMenu)
				for(unsigned int j=0; j < pSubMenu->GetMenuItemCount(); j++)
				{
					if (pSubMenu->GetMenuItemID(j) == ID_NODE_CREATE)
					{
						CMenu ResourceMenu;
						ResourceMenu.CreatePopupMenu();

						for(int k=0;k < BaseNodePort::portCount() ; k++)
							ResourceMenu.AppendMenu(MF_STRING,ID_NODE_PORT_BEGIN + k, BaseNodePort::portName( k ) );

						pSubMenu->DeleteMenu(j,MF_BYPOSITION);
						pSubMenu->InsertMenu(j,MF_POPUP|MF_BYPOSITION,(unsigned int)ResourceMenu.Detach(),_T("Create"));

						return;
					}
				}
		}
	}
}

void CChildFrame::OnRenderSdetaillevellow() 
{
	NodeBoolean::s_DetailLevel = NodeBoolean::LOW;
	RedrawWindow();
}

void CChildFrame::OnUpdateRenderSdetaillevellow(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( NodeBoolean::s_DetailLevel == NodeBoolean::LOW );
}

void CChildFrame::OnRenderSdetailevelmedium() 
{
	NodeBoolean::s_DetailLevel = NodeBoolean::MEDIUM;
	RedrawWindow();
}

void CChildFrame::OnUpdateRenderSdetailevelmedium(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( NodeBoolean::s_DetailLevel == NodeBoolean::MEDIUM );
}

void CChildFrame::OnRenderSdetaillevelhigh() 
{
	NodeBoolean::s_DetailLevel = NodeBoolean::HIGH;
	RedrawWindow();
}

void CChildFrame::OnUpdateRenderSdetaillevelhigh(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( NodeBoolean::s_DetailLevel == NodeBoolean::HIGH );
}

void CChildFrame::OnRenderSshowalignedhull() 
{
	NodeTransform::s_ShowAlignedHull = !NodeTransform::s_ShowAlignedHull;
}

void CChildFrame::OnUpdateRenderSshowalignedhull(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( NodeTransform::s_ShowAlignedHull );
}

void CChildFrame::OnRenderSshowhull() 
{
	NodeTransform::s_ShowHull = !NodeTransform::s_ShowHull;
	RedrawWindow();
}

void CChildFrame::OnUpdateRenderSshowhull(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( NodeTransform::s_ShowHull );
}

void CChildFrame::OnRenderSshownodeaxis() 
{
	NodeTransform::s_bDrawNodeAxis = !NodeTransform::s_bDrawNodeAxis;
	RedrawWindow();
}

void CChildFrame::OnUpdateRenderSshownodeaxis(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( NodeTransform::s_bDrawNodeAxis );
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CMDIChildWnd::PreCreateWindow(cs);
}

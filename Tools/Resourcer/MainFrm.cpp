// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Resource.h"
#include "MainFrm.h"
#include "ClassHeirarchy.h"

#include "Tools/ResourcerDoc/Port.h"
#include "Tools/ResourcerDoc/ChoosePorts.h"

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_PORTLIBRARIES, OnViewPortlibraries)
	ON_COMMAND(ID_VIEW_CLASSHEIRARCHY, OnViewClassheirarchy)
//	ON_COMMAND(ID_VIEW_UPDATESETTINGS, OnViewUpdatesettings)
	ON_WM_DESTROY()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		//TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		//TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	//----------------------------------------------------------------------------

	// create the resources toolbar
	//m_ResourceBar.CreateEx(this, TBSTYLE_WRAPABLE | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT, 
	//	WS_CHILD | WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_SIZE_DYNAMIC );
	m_ResourceBar.Create( this );

	#define BAR_CX			(16)
	#define BAR_CY			(16)

	m_IconList.Create( BAR_CX,BAR_CY,ILC_COLOR32 | ILC_MASK,0,0); 
	for(int k=0;k< Port::portCount();k++)
		m_IconList.Add( Port::portIcon( k ) );
	m_ResourceBar.GetToolBarCtrl().SetImageList( &m_IconList );
	m_ResourceBar.SetSizes( CSize( BAR_CX + 7,BAR_CY + 6 ), CSize( BAR_CX, BAR_CY ) );

	m_ResourceBar.SetBarStyle(m_ResourceBar.GetBarStyle() | CBRS_SIZE_DYNAMIC);

	// add the resource types to the toolbar for creation
	for(int j=0;j< Port::portCount() ;j++)
	{
		TBBUTTON nbutton;
		nbutton.iBitmap = j;

		nbutton.idCommand = ID_RESOURCE_PORT_BEGIN + j; 
		nbutton.fsState = TBSTATE_ENABLED;
		nbutton.fsStyle = TBSTYLE_BUTTON;
		nbutton.dwData = NULL;
		nbutton.iString = NULL; // m_ResourceBar.GetToolBarCtrl().AddStrings( Port::portName(j) + "\0" );  
		
		m_ResourceBar.GetToolBarCtrl().AddButtons(1, &nbutton);
	}

	m_ResourceBar.EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBar(&m_ResourceBar,AFX_IDW_DOCKBAR_TOP);
	DockControlBar(&m_ResourceBar,AFX_IDW_DOCKBAR_RIGHT);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnViewPortlibraries() 
{
	CChoosePorts().DoModal();	
}

void CMainFrame::OnViewClassheirarchy() 
{
	CClassHeirarchy().DoModal();
}

//void CMainFrame::OnViewUpdatesettings() 
//{
//	CUpdateSettings().DoModal();	
//}

void CMainFrame::OnDestroy() 
{
	AfxGetApp()->CloseAllDocuments( false );
	CMDIFrameWnd::OnDestroy();
}


static bool IsEdit( CWnd* pWnd, bool needSelection )
{
	if ( pWnd == NULL )
		return false;

	HWND hWnd = pWnd->GetSafeHwnd();
	if (hWnd == NULL)
		return false;

	TCHAR szClassName[6];
	if (::GetClassName(hWnd, szClassName, 6) && _tcsicmp(szClassName, _T("Edit")) == 0 )
	{
		if ( needSelection )
		{
			CEdit * pEdit = (CEdit *)pWnd;

			int nBeg, nEnd;
			pEdit->GetSel( nBeg, nEnd );
			if ( nBeg == nEnd )
				return false;		// no text selected
		}
	}

	return true;
}

void CMainFrame::OnEditCopy() 
{
	CEdit * pEdit = (CEdit *)GetFocus();
	ASSERT( IsEdit( pEdit, true ) );

	pEdit->Copy();
}

void CMainFrame::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( IsEdit( GetFocus(), true ) );
}

void CMainFrame::OnEditCut() 
{
	CEdit * pEdit = (CEdit *)GetFocus();
	ASSERT( IsEdit( pEdit, true ) );

	pEdit->Cut();
}

void CMainFrame::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( IsEdit( GetFocus(), true ) );
}

void CMainFrame::OnEditPaste() 
{
	CEdit * pEdit = (CEdit *)GetFocus();
	ASSERT( IsEdit( pEdit, false ) );

	pEdit->Paste();
}

void CMainFrame::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( IsEdit( GetFocus(), false ) );
}

void CMainFrame::OnEditUndo() 
{
	CEdit * pEdit = (CEdit *)GetFocus();
	ASSERT( IsEdit( pEdit, false ) );

	pEdit->Undo();
}

void CMainFrame::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( IsEdit( GetFocus(), false ) );
}

void CMainFrame::OnEditSelectAll() 
{
	CEdit * pEdit = (CEdit *)GetFocus();
	ASSERT( IsEdit( pEdit, false ) );
	
	pEdit->SetSel( 0, -1 );
}

void CMainFrame::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( IsEdit( GetFocus(), false ) );
}

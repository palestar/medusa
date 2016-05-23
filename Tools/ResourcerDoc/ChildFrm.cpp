// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"

#include "ChildFrm.h"
#include "ResourcerTree.h"
#include "ResourcerList.h"
#include "MessageList.h"
#include "Port.h"

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_MDIACTIVATE()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_RESOURCE_PORT_BEGIN, ID_RESOURCE_PORT_END, OnPortCreate )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	m_SplitterInitialized = false;
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	if (!m_Splitter2.CreateStatic(this,2,1))
		return FALSE;
	
	int bottomSize = lpcs->cy / 4;
	int topSize = lpcs->cy - bottomSize;

	m_Splitter2.CreateView(0,0,RUNTIME_CLASS(CView),CSize(lpcs->cx, topSize),pContext );
	m_Splitter2.CreateView(1,0,RUNTIME_CLASS(CMessageList),CSize(lpcs->cx, bottomSize),pContext );

	if (!m_Splitter.CreateStatic(&m_Splitter2, 1,2,WS_CHILD | WS_VISIBLE, m_Splitter2.IdFromRowCol(0,0) ))
		return FALSE;
	
	int leftSize = lpcs->cx / 4;
	int rightSize = lpcs->cx - leftSize;
	m_Splitter.CreateView(0,0,RUNTIME_CLASS(ResourcerTree),CSize(leftSize,topSize),pContext);
	m_Splitter.CreateView(0,1,RUNTIME_CLASS(ResourcerList),CSize(rightSize,topSize),pContext);
	m_SplitterInitialized = true;

	return(TRUE);
	//return CMDIChildWnd::OnCreateClient(lpcs, pContext);
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
					if (pSubMenu->GetMenuItemID(j) == ID_PORTS_CREATE)
					{
						CMenu ResourceMenu;
						ResourceMenu.CreatePopupMenu();

						for(int k=0;k < Port::portCount() ; k++)
							ResourceMenu.AppendMenu(MF_STRING,ID_RESOURCE_PORT_BEGIN + k,
								Port::portName( k ) );

						pSubMenu->DeleteMenu(j,MF_BYPOSITION);
						pSubMenu->InsertMenu(j,MF_POPUP|MF_BYPOSITION,(unsigned int)ResourceMenu.Detach(),_T("Create"));

						return;
					}
				}
		}
	}
}

void CChildFrame::OnPortCreate( UINT nID ) 
{
	int portIndex = nID - ID_RESOURCE_PORT_BEGIN;
	
	Factory * pFactory = Port::portFactory( portIndex );
	ASSERT( pFactory );

	Port * pDoc = dynamic_cast<Port *>( pFactory->createWidget() );
	ASSERT( pDoc );

	pDoc->SetModifiedFlag();
	pDoc->SetTitle( CString("New ") + pFactory->className() );
	pDoc->createView();
}


BOOL CChildFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CMDIFrameWnd* pParentWnd, CCreateContext* pContext) 
{
	return CMDIChildWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, pContext);
}



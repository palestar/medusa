// ChildFrame.cpp : implementation file
//

#define IMAGEPORT_DLL
#include "stdafx.h"
#include "ChildFrame.h"

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
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

BOOL CChildFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, 
						 const RECT& rect, CMDIFrameWnd* pParentWnd, CCreateContext* pContext) 
{
	if (!m_Menu.LoadMenu(IDR_PORT))
		return FALSE;
	m_hMenuShared = m_Menu.m_hMenu;
	
	return CMDIChildWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, pContext);
}

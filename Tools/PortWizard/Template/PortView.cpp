// PortView.cpp : implementation file
//

#define $$ROOT$$_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CView)

CPortView::CPortView()
{
}

CPortView::~CPortView()
{
}


BEGIN_MESSAGE_MAP(CPortView, CView)
	//{{AFX_MSG_MAP(CPortView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPortView drawing

void CPortView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CPortView diagnostics

#ifdef _DEBUG
void CPortView::AssertValid() const
{
	CView::AssertValid();
}

void CPortView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPortView message handlers

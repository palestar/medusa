// PortView.cpp : implementation file
//

#define $$ROOT$$_DLL
#include "stdafx.h"
#include "PortView.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
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

// PortView.cpp : implementation file
//

#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CEditView)

CPortView::CPortView()
{
}

CPortView::~CPortView()
{
}


BEGIN_MESSAGE_MAP(CPortView, CEditView)
	//{{AFX_MSG_MAP(CPortView)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillfocus)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
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
	CEditView::AssertValid();
}

void CPortView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPortView message handlers

void CPortView::OnInitialUpdate() 
{
	CEditView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	TextPort * pDoc = (TextPort *)GetDocument();
	ASSERT( pDoc );

	if ( lHint )
	{
		CString text;
		GetWindowText( text );

		pDoc->m_Text = text;
		pDoc->UpdateAllViews( this );
	}
	else
		SetWindowText( pDoc->m_Text );
}

void CPortView::OnChange() 
{
	TextPort * pDoc = (TextPort *)GetDocument();
	ASSERT( pDoc );

	CString text;
	GetWindowText( text );

	pDoc->m_Text = text;
}

void CPortView::OnKillfocus() 
{
	TextPort * pDoc = (TextPort *)GetDocument();
	ASSERT( pDoc );

	CString text;
	GetWindowText( text );

	pDoc->m_Text = text;
	pDoc->UpdateAllViews( this );
}

void CPortView::OnEditCopy() 
{
	GetEditCtrl().Copy();
}

void CPortView::OnEditCut() 
{
	GetEditCtrl().Cut();
}

void CPortView::OnEditPaste() 
{
	GetEditCtrl().Paste();
}

void CPortView::OnEditUndo() 
{
	GetEditCtrl().Undo();
}

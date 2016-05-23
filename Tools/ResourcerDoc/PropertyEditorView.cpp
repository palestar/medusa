// PropertyEditorView.cpp : implementation of the CPropertyEditorView class
//

#include "stdafx.h"
#include "Resource.h"
#include "PropertyEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPropertyEditorView

IMPLEMENT_DYNCREATE(CPropertyEditorView, CView)

BEGIN_MESSAGE_MAP(CPropertyEditorView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
   ON_COMMAND(ID_EDIT_ADD, OnEditAdd)
   ON_UPDATE_COMMAND_UI(ID_EDIT_ADD, OnUpdateEditAdd)
   ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
   ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
   ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
   ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
END_MESSAGE_MAP()

// CPropertyEditorView construction/destruction

CPropertyEditorView::CPropertyEditorView()
{
	// TODO: add construction code here

}

CPropertyEditorView::~CPropertyEditorView()
{
}

BOOL CPropertyEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

void CPropertyEditorView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

}

void CPropertyEditorView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
   m_cList.DeleteAll();

	Port * pDoc = GetDocument();
	if ( pDoc != NULL )
		m_cList.AddClass( pDoc ); 
}

// CPropertyEditorView diagnostics

#ifdef _DEBUG
void CPropertyEditorView::AssertValid() const
{
	CView::AssertValid();
}

void CPropertyEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

Port * CPropertyEditorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(Port)));
	return (Port*)m_pDocument;
}
#endif //_DEBUG


// CPropertyEditorView message handlers

int CPropertyEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_cList.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect(0,0,0,0), this, 711 );
	return 0;
}

void CPropertyEditorView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if ( ::IsWindow( m_cList.m_hWnd ) )
		m_cList.MoveWindow( 0, 0, cx, cy, TRUE );
}

void CPropertyEditorView::OnDraw(CDC* /*pDC*/)
{
	// TODO: Add your specialized code here and/or call the base class
}

void CPropertyEditorView::OnEditAdd()
{
   m_cList.AddToContainer( m_cList.GetCurSel() );
}

void CPropertyEditorView::OnUpdateEditAdd(CCmdUI *pCmdUI)
{
   pCmdUI->Enable( m_cList.CanAddToContainer( m_cList.GetCurSel() ) );
}

void CPropertyEditorView::OnEditClear()
{
   m_cList.ClearContained( m_cList.GetCurSel() );
}

void CPropertyEditorView::OnUpdateEditClear(CCmdUI *pCmdUI)
{
   pCmdUI->Enable( m_cList.CanClearContained( m_cList.GetCurSel() ) );
}

void CPropertyEditorView::OnEditDelete()
{
   m_cList.DeleteFromContainer( m_cList.GetCurSel() );
}

void CPropertyEditorView::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
   pCmdUI->Enable( m_cList.CanDeleteFromContainer( m_cList.GetCurSel() ) );
}


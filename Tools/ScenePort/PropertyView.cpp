// PropertyView.cpp : implementation file
//

#include "stdafx.h"
#include "Port.h"
#include "PropertyView.h"
#include "Resource.h"

// CPropertyView

IMPLEMENT_DYNCREATE(CPropertyView, CView)

BEGIN_MESSAGE_MAP(CPropertyView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_ADD, OnEditAdd)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD, OnUpdateEditAdd)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
END_MESSAGE_MAP()

CPropertyView::CPropertyView()
{
}

CPropertyView::~CPropertyView()
{
}

// CPropertyView diagnostics

#ifdef _DEBUG
void CPropertyView::AssertValid() const
{
	CView::AssertValid();
}

void CPropertyView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


void CPropertyView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	m_cList.DeleteAll();

	ScenePort * pDoc = (ScenePort *)GetDocument();
	if ( pDoc != NULL )
		m_cList.AddClass( pDoc->m_pSelectedNode ); 
}

int CPropertyView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_cList.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect(0,0,0,0), this, 711 );
	return 0;
}

void CPropertyView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if ( ::IsWindow( m_cList.m_hWnd ) )
		m_cList.MoveWindow( 0, 0, cx, cy, TRUE );
}

void CPropertyView::OnDraw(CDC* /*pDC*/)
{
	// TODO: Add your specialized code here and/or call the base class
}

void CPropertyView::OnEditAdd()
{
   m_cList.AddToContainer( m_cList.GetCurSel() );
}

void CPropertyView::OnUpdateEditAdd(CCmdUI *pCmdUI)
{
   pCmdUI->Enable( m_cList.CanAddToContainer( m_cList.GetCurSel() ) );
}

void CPropertyView::OnEditClear()
{
   m_cList.ClearContained( m_cList.GetCurSel() );
}

void CPropertyView::OnUpdateEditClear(CCmdUI *pCmdUI)
{
   pCmdUI->Enable( m_cList.CanClearContained( m_cList.GetCurSel() ) );
}

void CPropertyView::OnEditDelete()
{
   m_cList.DeleteFromContainer( m_cList.GetCurSel() );
}

void CPropertyView::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
   pCmdUI->Enable( m_cList.CanDeleteFromContainer( m_cList.GetCurSel() ) );
}


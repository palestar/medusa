// MessageList.cpp : implementation file
//

#include "stdafx.h"
#include "ResourcerDoc.h"
#include "MessageList.h"

#include "Tools/Resourcer/MainFrm.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageList

IMPLEMENT_DYNCREATE(CMessageList, CListView)

CMessageList::CMessageList()
{
	m_LastMessageId = 0;
}

CMessageList::~CMessageList()
{
}


BEGIN_MESSAGE_MAP(CMessageList, CListView)
	//{{AFX_MSG_MAP(CMessageList)
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageList drawing

void CMessageList::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CMessageList diagnostics

#ifdef _DEBUG
void CMessageList::AssertValid() const
{
	CListView::AssertValid();
}

void CMessageList::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMessageList message handlers

void CMessageList::OnInitialUpdate() 
{
	GetListCtrl().ModifyStyle( LVS_ICON|LVS_SMALLICON|LVS_LIST, 
		LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_REPORT|LVS_SINGLESEL);
	GetListCtrl().SetExtendedStyle( LVS_EX_FULLROWSELECT );

	CListView::OnInitialUpdate();

	CRect rect;
	GetListCtrl().GetClientRect( &rect );
	GetListCtrl().InsertColumn(0,TEXT("Output"),LVCFMT_LEFT, rect.Width() );

	// begin checking for new messages
	SetTimer( 0x1, 50, NULL );	
}

void CMessageList::OnSize(UINT nType, int cx, int cy) 
{
	CListView::OnSize(nType, cx, cy);
	
	if (::IsWindow(GetListCtrl().m_hWnd))
		GetListCtrl().SetColumnWidth(0 , cx );
}

void CMessageList::OnTimer(UINT nIDEvent) 
{
	CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
	ASSERT( pMain );

	if ( nIDEvent == 0x1 )
	{
		CResourcerDoc * pDoc = CResourcerDoc::document();
		if ( pDoc != NULL )
		{
			pDoc->lockMessageQueue( true );
			if ( pDoc->messageCount() < m_LastMessageId )
			{
				GetListCtrl().DeleteAllItems();
				m_LastMessageId = 0;
			}

			// add new messages
			for(;m_LastMessageId<pDoc->messageCount();m_LastMessageId++)
			{
				int item = GetListCtrl().InsertItem( GetListCtrl().GetItemCount(), pDoc->message( m_LastMessageId ) );
				GetListCtrl().EnsureVisible( item, false );
			}

			// set the status text
			pMain->m_wndStatusBar.SetWindowText( pDoc->status() );
			pDoc->lockMessageQueue( false );
		}
	}

	//CListView::OnTimer(nIDEvent);
}


void CMessageList::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

	int selected = GetListCtrl().GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	CString item( GetListCtrl().GetItemText( selected, 0 ) );
	int seperator = item.Find( ';' );
	if ( seperator < 0 )
		return;

	CString file = item.Left( seperator );
	file.TrimRight( TEXT(" ") );

	AfxGetApp()->OpenDocumentFile( file );
}

// ResourcerTree.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"

#include "ChildFrm.h"
#include "ResourcerTree.h"
#include "ResourcerList.h"
#include "ResourcerDoc.h"
#include "CreateDirectory.h"
#include ".\resourcertree.h"

/////////////////////////////////////////////////////////////////////////////
// ResourcerTree

IMPLEMENT_DYNCREATE(ResourcerTree, CTreeView)

ResourcerTree::ResourcerTree()
{
}

ResourcerTree::~ResourcerTree()
{
}


BEGIN_MESSAGE_MAP(ResourcerTree, CTreeView)
	//{{AFX_MSG_MAP(ResourcerTree)
	ON_COMMAND(ID_TREE_CREATE, OnTreeCreate)
	ON_COMMAND(ID_TREE_DELETE, OnTreeDelete)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_COMMAND(ID_EDIT_RENAME, OnEditRename)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_COMMAND(ID_DOCUMENT_BUILDSELECTED, OnDocumentBuildselected)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRightClick)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_PORTS_TOUCH, OnPortsTouch)
	ON_COMMAND(ID_PORTS_UPGRADE, OnPortsUpgrade)
	ON_UPDATE_COMMAND_UI(ID_PORTS_UPGRADE, OnUpdatePortsUpgrade)
	ON_UPDATE_COMMAND_UI(ID_PORTS_TOUCH, OnUpdatePortsTouch)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnTvnItemexpanding)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ResourcerTree drawing

void ResourcerTree::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// ResourcerTree diagnostics

#ifdef _DEBUG
void ResourcerTree::AssertValid() const
{
	CTreeView::AssertValid();
}

void ResourcerTree::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// ResourcerTree message handlers

void ResourcerTree::populateTree( const CString &path, HTREEITEM hParent)
{
	// remove all children items from the parent
	HTREEITEM hItem = GetTreeCtrl().GetChildItem( hParent );
	while( hItem != NULL)
	{
		GetTreeCtrl().DeleteItem( hItem );
		hItem = GetTreeCtrl().GetChildItem( hParent );
	}

	// search the current directory
	CString searchPath( path + "*.*" );

	// add the directories to the parent
	WIN32_FIND_DATA	ff;
	HANDLE ffh = FindFirstFile( searchPath , &ff);
	if (ffh == INVALID_HANDLE_VALUE)
		return;

	BOOL found = true;
	while(found)
	{
		if ( (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && ff.cFileName[0] != '.')
		{
			CString childName( ff.cFileName );
			childName += "\\";

			HTREEITEM hChild = GetTreeCtrl().InsertItem( childName, hParent );

			// recurse into that directory
			populateTree( path + childName, hChild );
		}

		found = FindNextFile(ffh,&ff);
	}
	FindClose(ffh);

	GetTreeCtrl().SortChildren( hParent );
}

bool ResourcerTree::removeDirectory( const char * pPath )
{
	WIN32_FIND_DATA	ff;

	// delete all the files and directories in this directory
	HANDLE ffh = FindFirstFile( CString(pPath) + "\\*.*" , &ff);
	if (ffh == INVALID_HANDLE_VALUE)
		return true;

	BOOL found = true;
	while(found)
	{
		if ( ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if ( ff.cFileName[0] != '.')
				if ( ! removeDirectory( CString(pPath) + "\\" + ff.cFileName ) )
					return false;
		}
		else
		{
			if ( ! DeleteFile( CString(pPath) + "\\" + ff.cFileName) )
			{
				MessageBox( CString(pPath) + ff.cFileName, TEXT("Failed to delete file") );
				return false;
			}
		}

		found = FindNextFile(ffh,&ff);
	}
	FindClose(ffh);
	
	// remove the directory now
	if ( ! ::RemoveDirectory( pPath ) )
	{
		MessageBox( pPath, TEXT("Failed to remove directory") );
		return false;
	}

	return true;
}

CString ResourcerTree::path( HTREEITEM hTop )
{
	// build the path from the selected item
	CString path = "";
	while( hTop != NULL )
	{
		path = GetTreeCtrl().GetItemText( hTop ) + path;
		hTop = GetTreeCtrl().GetParentItem( hTop );
	}

	return( path );
}

//-------------------------------------------------------------------------------

void ResourcerTree::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument()); 
	ASSERT( pDoc );

	// TODO: ReadDirectoryChangesW
	CBitmap treeMap;
	treeMap.LoadBitmap( IDB_TREE );

	// set the image list
	m_ImageList.Create( 16,15,ILC_COLOR32 | ILC_MASK,0,0); 
	m_ImageList.Add( &treeMap, 0xffffff );

	//m_ImageList.Create( IDB_TREE, 16, 0, 0xffffff );
	GetTreeCtrl().SetImageList( &m_ImageList,TVSIL_NORMAL);

	// create the root item
	m_Root = GetTreeCtrl().InsertItem( pDoc->rootDirectory() );
	// populate the tree
	populateTree( pDoc->rootDirectory(), m_Root );
	// expand the root item
	GetTreeCtrl().Expand( m_Root, TVE_EXPAND );
}

void ResourcerTree::OnDestroy() 
{
	CTreeView::OnDestroy();
}

void ResourcerTree::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
}

void ResourcerTree::OnTreeCreate() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument()); 
	ASSERT( pDoc );

	// create a folder in the current directory
	if ( CreateDirectory( pDoc->currentDirectory() ).DoModal() == IDOK )
		populateTree( pDoc->currentDirectory(), GetTreeCtrl().GetSelectedItem() );
}

void ResourcerTree::OnTreeDelete() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument()); 
	ASSERT( pDoc );

	if (GetTreeCtrl().GetSelectedItem() == m_Root )
		return;
	if (MessageBox( pDoc->currentDirectory(), TEXT("Confirm Delete"), MB_OKCANCEL) != IDOK)
		return;

	CharString sDirectory( pDoc->currentDirectory() );
	if ( sDirectory.endsWith( "\\" ) )
		sDirectory.left( sDirectory.length() - 1 );

	if ( removeDirectory( sDirectory ) )
	{
		pDoc->pushMessage( CharString().format("%s deleted...", sDirectory) );
		GetTreeCtrl().DeleteItem( GetTreeCtrl().GetSelectedItem() );
	}
	else
		pDoc->pushMessage( CharString().format("Failed to delete %s...", sDirectory) );
}

BOOL ResourcerTree::PreCreateWindow(CREATESTRUCT& cs) 
{
	m_dwDefaultStyle |= TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_EDITLABELS;
	return CTreeView::PreCreateWindow(cs);
}

void ResourcerTree::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument()); 
	ASSERT( pDoc );

	pDoc->setCurrentDirectory( path( ((NM_TREEVIEW*)pNMHDR)->itemNew.hItem ) );
	pDoc->UpdateAllViews(this);

	*pResult = 0;
}

//-------------------------------------------------------------------------------

void ResourcerTree::OnEditRename() 
{
	GetTreeCtrl().EditLabel( GetTreeCtrl().GetSelectedItem() );
}

void ResourcerTree::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument()); 
	ASSERT( pDoc );

	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if ( pTVDispInfo->item.pszText != NULL && pTVDispInfo->item.hItem != m_Root )
	{
		CString newName( pTVDispInfo->item.pszText );
		newName += "\\";

		CString newPath = path( GetTreeCtrl().GetParentItem( pTVDispInfo->item.hItem ) );
		newPath += newName;

		// rename the directory
		if ( ! ::MoveFile( pDoc->currentDirectory(), newPath ))
		{
			MessageBox( pDoc->currentDirectory(), TEXT("Failed to rename directory") );
			return;
		}

		GetTreeCtrl().SetItemText( pTVDispInfo->item.hItem, newName );
		pDoc->setCurrentDirectory( newPath );
	}

	*pResult = 0;
}

//-------------------------------------------------------------------------------

void ResourcerTree::OnViewRefresh() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument()); 
	ASSERT( pDoc );

	populateTree( pDoc->rootDirectory(), m_Root );

	pDoc->UpdateAllViews( this );
}

void ResourcerTree::OnDocumentBuildselected() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument()); 
	ASSERT( pDoc );

	pDoc->beginBuild();
	pDoc->updateBroker( pDoc->currentDirectory(),true );
	pDoc->endBuild();
}

void ResourcerTree::OnEditCopy() 
{
	CChildFrame * pFrame = dynamic_cast<CChildFrame *>( GetParentFrame() );
	ASSERT( pFrame );
	ResourcerList * pView = dynamic_cast<ResourcerList *>( pFrame->m_Splitter.GetPane(0,1) );
	ASSERT( pView );

	pView->OnEditCopy();
}

void ResourcerTree::OnEditCut() 
{
	CChildFrame * pFrame = dynamic_cast<CChildFrame *>( GetParentFrame() );
	ASSERT( pFrame );
	ResourcerList * pView = dynamic_cast<ResourcerList *>( pFrame->m_Splitter.GetPane(0,1) );
	ASSERT( pView );

	pView->OnEditCut();
}

void ResourcerTree::OnEditPaste() 
{
	CChildFrame * pFrame = dynamic_cast<CChildFrame *>( GetParentFrame() );
	ASSERT( pFrame );
	ResourcerList * pView = dynamic_cast<ResourcerList *>( pFrame->m_Splitter.GetPane(0,1) );
	ASSERT( pView );

	pView->OnEditPaste();
}

void ResourcerTree::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu		ctmenu;		// context menu

	ctmenu.CreatePopupMenu();
	ctmenu.AppendMenu(MF_STRING,ID_DOCUMENT_BUILDSELECTED,"Build");
	ctmenu.AppendMenu(MF_STRING,ID_PORTS_TOUCH,"Touch");
	ctmenu.AppendMenu(MF_STRING,ID_PORTS_UPGRADE,"Upgrade");
	ctmenu.AppendMenu(MF_SEPARATOR);
	ctmenu.AppendMenu(MF_STRING,ID_TREE_CREATE,"Create");
	ctmenu.AppendMenu(MF_STRING,ID_TREE_DELETE,"Delete");
	ctmenu.AppendMenu(MF_STRING,ID_EDIT_RENAME,"Rename");

	// track the context menu
	ctmenu.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,GetParent());
	ctmenu.DestroyMenu();
}

void ResourcerTree::OnRightClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint pt;
	GetCursorPos( &pt );

	OnContextMenu(this, pt );
	*pResult = 0;
}

void ResourcerTree::OnPortsTouch()
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	pDoc->touchPorts( (CharString)pDoc->currentDirectory() + "*.PRT", true );
}

void ResourcerTree::OnPortsUpgrade()
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	pDoc->upgradePorts( (CharString)pDoc->currentDirectory() + "*.PRT", true );
}

void ResourcerTree::OnUpdatePortsUpgrade(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( GetTreeCtrl().GetSelectedItem() != NULL );
}

void ResourcerTree::OnUpdatePortsTouch(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( GetTreeCtrl().GetSelectedItem() != NULL );
}

void ResourcerTree::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

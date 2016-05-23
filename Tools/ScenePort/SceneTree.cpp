// SceneTree.cpp : implementation file
//

#pragma warning( disable: 4786 )	// identifier was truncated to '255' characters in the browser information

#include "stdafx.h"
 
#include "ChildFrame.h"
#include "SceneTree.h"
#include "Port.h"
#include "Resource.h"

#include "File/FileMemory.h"
#include "Tools/MaterialPort/Port.h"
#include "Tools/ImagePort/Port.h"
#include "Tools/ResourcerDoc/ResourcerDoc.h"

extern AFX_EXTENSION_MODULE						g_PortDLL;

/////////////////////////////////////////////////////////////////////////////
// CSceneTree

IMPLEMENT_DYNCREATE(CSceneTree, CTreeView)

CSceneTree::CSceneTree() : m_pSelectedNode( NULL )
{
}

CSceneTree::~CSceneTree()
{
}

BEGIN_MESSAGE_MAP(CSceneTree, CTreeView)
	//{{AFX_MSG_MAP(CSceneTree)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_COMMAND(ID_NODE_DELETE, OnNodeDelete)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_COMMAND(ID_NODE_OPEN, OnNodeOpen)
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_NODE_PORT_BEGIN, ID_NODE_PORT_END, OnPortCreate )
	//ON_UPDATE_COMMAND_UI_RANGE( ID_NODE_PORT_BEGIN, ID_NODE_PORT_END, OnUpdatePortCreate )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSceneTree drawing

void CSceneTree::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CSceneTree diagnostics

#ifdef _DEBUG
void CSceneTree::AssertValid() const
{
	CTreeView::AssertValid();
}

void CSceneTree::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------------------

void CSceneTree::populateTree( HTREEITEM hParent, BaseNodePort::Ref node )
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( pDoc->m_pSelectedNode.pointer() == node.pointer() )
		GetTreeCtrl().SelectItem( hParent );

	// recurse into children
	HTREEITEM hItem = GetTreeCtrl().GetChildItem( hParent );
	for(int i=0;i<node->childCount();i++)
	{
		// get the child object
		BaseNodePort * pChild = node->child(i);

		// get the child image index
		Factory * pFactory = pChild->factory();
		int	portIndex = BaseNodePort::portIndex( pFactory );

		// generate item name
		CharString itemName;
		itemName.format("%s (%s)", pChild->name(), pFactory->className() );

		TVITEM treeItem;
		treeItem.mask = TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT;
		treeItem.pszText = itemName.buffer();
		treeItem.iImage = treeItem.iSelectedImage = portIndex;
		treeItem.lParam = (DWORD)pChild;

		if ( hItem != NULL )
		{
			// update existing item
			treeItem.hItem = hItem;
			treeItem.mask |= TVIF_HANDLE;
			GetTreeCtrl().SetItem( &treeItem );
		}
		else
		{
			// create new item
			TVINSERTSTRUCT newItem;
			newItem.hParent = hParent;
			newItem.hInsertAfter = TVI_LAST;
			newItem.item = treeItem;

			// insert the child object
			hItem = GetTreeCtrl().InsertItem( &newItem );
		}
	
		// populate the child
		populateTree( hItem, pChild );

		// get the next tree item
		hItem = GetTreeCtrl().GetNextSiblingItem( hItem );
	}

	// remove excess items
	while( hItem != NULL )
	{
		HTREEITEM hNext = GetTreeCtrl().GetNextSiblingItem( hItem );
		GetTreeCtrl().DeleteItem( hItem );
		
		hItem = hNext;
	}
}

//-------------------------------------------------------------------------------

void CSceneTree::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
		HTREEITEM hSelected = GetTreeCtrl().GetSelectedItem();
		pDoc->m_pSelectedNode = (BaseNodePort *)GetTreeCtrl().GetItemData( hSelected );
		pDoc->UpdateAllViews( this );
	}
	else
		populateTree( m_Root, (BaseNodePort *)GetTreeCtrl().GetItemData( m_Root ) );
}

void CSceneTree::OnInitialUpdate() 
{
	ScenePort * pPort = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pPort );

	// Create the image list for the tree control
	m_ImageList.Create( 16,16,ILC_COLOR32 | ILC_MASK,0,0); 
	for(int i=0;i< BaseNodePort::portCount();i++)
		m_ImageList.Add( BaseNodePort::portIcon( BaseNodePort::port( i )->classKey() ));

	GetTreeCtrl().SetImageList(&m_ImageList,TVSIL_NORMAL);

	// get the root node image
	BaseNodePort * rootNode = pPort->m_Root;
	int rootImage = BaseNodePort::portIndex( rootNode->factory() );

	// add the root item
	m_Root = GetTreeCtrl().InsertItem( _T("Scene"), rootImage, rootImage );
	GetTreeCtrl().SetItemData( m_Root, (DWORD)rootNode );
	pPort->m_pSelectedNode = rootNode;

	// populate the tree
	CTreeView::OnInitialUpdate();
	//populateTree( m_Root, rootNode );

	GetTreeCtrl().Expand( m_Root, TVE_EXPAND );
}

BOOL CSceneTree::PreCreateWindow(CREATESTRUCT& cs) 
{
	m_dwDefaultStyle |= TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_SHOWSELALWAYS;
	return CTreeView::PreCreateWindow(cs);
}

void CSceneTree::OnPortCreate( UINT nID ) 
{
	ScenePort * pPort = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pPort );

	pPort->SetModifiedFlag();

	// get the factory
	int portIndex = nID - ID_NODE_PORT_BEGIN;
	Factory * pFactory = BaseNodePort::port( portIndex );
	
	// create the object and store the it to disk
	BaseNodePort * newPort = dynamic_cast<BaseNodePort *>(pFactory->createWidget());
	ASSERT( newPort );

	// attach to selected node
	HTREEITEM selectedItem = GetTreeCtrl().GetSelectedItem();
	BaseNodePort * selectedPort = (BaseNodePort *)GetTreeCtrl().GetItemData( selectedItem );
	ASSERT( selectedPort );

	selectedPort->addChild( newPort );
	selectedPort->setParent( pPort );

	// update the tree
	populateTree( selectedItem, selectedPort );
	// expand the selected item
	GetTreeCtrl().Expand( selectedItem, TVE_EXPAND );

	pPort->UpdateAllViews( this );
}

void CSceneTree::OnUpdatePortCreate(CCmdUI* pCmdUI,  UINT nID ) 
{
	pCmdUI->Enable( GetTreeCtrl().GetSelectedItem() != NULL );
}

static bool s_ConfirmDelete = true;

void CSceneTree::OnNodeDelete() 
{
	HTREEITEM selectedItem = GetTreeCtrl().GetSelectedItem();
	HTREEITEM parentItem = GetTreeCtrl().GetParentItem( selectedItem );

	if (parentItem != NULL)
	{
		BaseNodePort * selectedPort = (BaseNodePort *)GetTreeCtrl().GetItemData( selectedItem );
		ASSERT( selectedPort );
		BaseNodePort * parentPort = (BaseNodePort *)GetTreeCtrl().GetItemData( parentItem );
		ASSERT( parentPort );

		GetTreeCtrl().SelectItem( parentItem );

		if ( s_ConfirmDelete && MessageBox( _T("Delete this node and it's children?"), selectedPort->name(), MB_YESNO | MB_DEFBUTTON2 ) != IDYES )
			return;

		// remove the selected port from the parent
		for(int i=0;i<parentPort->childCount();i++)
			if ( parentPort->child( i ) == selectedPort )
			{
				parentPort->removeChild( i );
				break;
			}

		GetTreeCtrl().DeleteItem( selectedItem );

		// update all other views
		ScenePort * pPort = dynamic_cast<ScenePort *>( GetDocument() );
		ASSERT( pPort );

		pPort->SetModifiedFlag();
		pPort->UpdateAllViews( this );
	}
}

void CSceneTree::OnNodeOpen() 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( pDoc->m_pSelectedNode )
		pDoc->m_pSelectedNode->createView();
}

void CSceneTree::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnNodeOpen();
	*pResult = 0;
}

void CSceneTree::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	HTREEITEM hItem = ((NM_TREEVIEW*)pNMHDR)->itemNew.hItem;
	pDoc->m_pSelectedNode = (BaseNodePort *)GetTreeCtrl().GetItemData( hItem );
	pDoc->UpdateAllViews( this );

	OnUpdate(this,false,NULL);

	*pResult = 0;
}


const unsigned int	SCENE_CLIP_ID = 'SCNT';

void CSceneTree::OnEditCopy() 
{
	ScenePort * pPort = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pPort );
	BaseNodePort * selectedPort = (BaseNodePort *)GetTreeCtrl().GetItemData( GetTreeCtrl().GetSelectedItem() );
	ASSERT( selectedPort );

	// serialize all selected ports into a FileMemory object, then store the data on the clipboard
	FileMemory::Ref pFile = new FileMemory();
	
	OutStream output( pFile, FF_COPY );
	output << SCENE_CLIP_ID;
	output << selectedPort;

	// paste to clipboard
	HGLOBAL pasteGlobal = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, pFile->size() );
	memcpy(GlobalLock(pasteGlobal), pFile->bytes(), pFile->size() );
	GlobalUnlock(pasteGlobal);

	if (OpenClipboard())
	{
		SetClipboardData( CF_PRIVATEFIRST, pasteGlobal );
		CloseClipboard();
	}
	else
		MessageBox(_T("Failed to open clipboard"));
}

void CSceneTree::OnEditPaste() 
{
	ScenePort * pPort = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pPort );

	if (!OpenClipboard())
		return;

	HGLOBAL hPaste = GetClipboardData( CF_PRIVATEFIRST );
	if ( hPaste == NULL )
	{
		CloseClipboard();
		return;		// no data, close clipboard and exit...
	}

	FileMemory::Ref pFile = new FileMemory( GlobalLock( hPaste ), GlobalSize( hPaste ) );
	GlobalUnlock( hPaste );
	CloseClipboard();

	InStream input( pFile, FF_COPY );

	unsigned int validateId;
	input >> validateId;

	if ( validateId != SCENE_CLIP_ID )
		return;

	BaseNodePort * pNode = NULL;
	input >> pNode;

	HTREEITEM selectedItem = GetTreeCtrl().GetSelectedItem();
	BaseNodePort * selectedPort = (BaseNodePort *)GetTreeCtrl().GetItemData( selectedItem );
	ASSERT( selectedPort );

	selectedPort->addChild( pNode );
	selectedPort->setParent( pPort );

	// update the tree control
	populateTree( selectedItem, selectedPort );
	GetTreeCtrl().Expand( selectedItem, TVE_EXPAND );

	// update the other views
	pPort->SetModifiedFlag();
	pPort->UpdateAllViews( this );

}

void CSceneTree::OnEditCut() 
{
	s_ConfirmDelete = false;

	OnEditCopy();
	OnNodeDelete();

	s_ConfirmDelete = true;
}

void CSceneTree::OnViewRefresh() 
{
	populateTree( m_Root, (BaseNodePort *)GetTreeCtrl().GetItemData( m_Root ) );
}

void CSceneTree::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// translate to screen coordinates
	ClientToScreen( &point );

	CMenu createMenu;
	createMenu.CreatePopupMenu();
	for(int k=0;k < BaseNodePort::portCount() ; k++)
		createMenu.AppendMenu(MF_STRING,ID_NODE_PORT_BEGIN + k, BaseNodePort::portName( k ) );

	CMenu contextMenu;
	contextMenu.CreatePopupMenu();
	contextMenu.AppendMenu(MF_STRING,ID_EDIT_COPY,_T("Copy"));
	contextMenu.AppendMenu(MF_STRING,ID_EDIT_CUT,_T("Cut"));
	contextMenu.AppendMenu(MF_STRING,ID_EDIT_PASTE,_T("Paste"));
	contextMenu.AppendMenu(MF_SEPARATOR);
	contextMenu.AppendMenu(MF_STRING,ID_NODE_OPEN,_T("Open"));
	contextMenu.AppendMenu(MF_STRING,ID_NODE_DELETE,_T("Delete"));
	contextMenu.InsertMenu(-1,MF_POPUP|MF_BYPOSITION,(unsigned int)createMenu.Detach(),_T("Create"));

	// track the context menu
	contextMenu.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,GetParent());
	contextMenu.DestroyMenu();
	
	//CTreeView::OnRButtonDown(nFlags, point);
}

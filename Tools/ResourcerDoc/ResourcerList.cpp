// ResourcerList.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"

#include "ResourcerList.h"
#include "ResourcerDoc.h"
#include "Port.h"
#include "PortDepend.h"

#include "File/Path.h"
#include "File/FileDisk.h"
#include "File/FileMemory.h"
#include "Factory/Widget.h"
#include "Factory/Broker.h"

#include "resource.h"

#include <io.h>
#include ".\resourcerlist.h"



/////////////////////////////////////////////////////////////////////////////
// ResourcerList

IMPLEMENT_DYNCREATE(ResourcerList, CListView)

ResourcerList::ResourcerList()
{
}

ResourcerList::~ResourcerList()
{
}


BEGIN_MESSAGE_MAP(ResourcerList, CListView)
	//{{AFX_MSG_MAP(ResourcerList)
	ON_COMMAND(ID_PORTS_DELETE, OnPortsDelete)
	ON_COMMAND(ID_EDIT_RENAME, OnEditRename)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_COMMAND(ID_PORTS_PROPERTIES, OnPortsProperties)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_PORTS_SHELLOPEN, OnPortsShellopen)
	ON_UPDATE_COMMAND_UI(ID_PORTS_DELETE, OnUpdatePortsDelete)
	ON_UPDATE_COMMAND_UI(ID_PORTS_PROPERTIES, OnUpdatePortsProperties)
	ON_UPDATE_COMMAND_UI(ID_PORTS_SHELLOPEN, OnUpdatePortsShellopen)
	ON_WM_DROPFILES()
	ON_COMMAND(ID_DOCUMENT_BUILDSELECTED, OnDocumentBuildselected)
	ON_UPDATE_COMMAND_UI(ID_DOCUMENT_BUILDSELECTED, OnUpdateDocumentBuildselected)
	ON_COMMAND(ID_PORTS_DEPENDENCIES, OnPortsDependencies)
	ON_UPDATE_COMMAND_UI(ID_PORTS_DEPENDENCIES, OnUpdatePortsDependencies)
	ON_COMMAND(ID_PORTS_TOUCH, OnPortsTouch)
	ON_COMMAND(ID_PORTS_UPGRADE, OnPortsUpgrade)
	ON_UPDATE_COMMAND_UI(ID_PORTS_UPGRADE, OnUpdatePortsUpgrade)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ResourcerList drawing

void ResourcerList::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// ResourcerList diagnostics

#ifdef _DEBUG
void ResourcerList::AssertValid() const
{
	CListView::AssertValid();
}

void ResourcerList::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// ResourcerList message handlers

void ResourcerList::OnPortsProperties() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	for(int i=0;i<GetListCtrl().GetItemCount(); i++ )
		if (GetListCtrl().GetItemState(i,LVIS_SELECTED))
		{
			CharString sFileName = CharString( pDoc->currentDirectory() ) + GetListCtrl().GetItemText(i,0);

			// open up ports using this program, other file types send to windows
			if ( stricmp<char>( Path( sFileName ).extension(), ".PRT" ) == 0 )
				AfxGetApp()->OpenDocumentFile( sFileName );
			else
				ShellExecute( NULL, "open", sFileName, NULL, pDoc->currentDirectory(), SW_SHOWNORMAL );

		}

	OnUpdate(this,0,NULL);
}

void ResourcerList::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (GetAsyncKeyState(VK_SHIFT) < 0)
		OnPortsShellopen();
	else
		OnPortsProperties();
	
	*pResult = 0;
}

void ResourcerList::OnPortsDelete() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	if (MessageBox("Are you sure you want to delete all selected ports?","Delete Port", MB_YESNO) == IDYES)
	{
		for(int i=0;i<GetListCtrl().GetItemCount(); i++ )
			if (GetListCtrl().GetItemState(i,LVIS_SELECTED))
			{
				CharString sFile = pDoc->currentDirectory();
				sFile += GetListCtrl().GetItemText(i,0);
				sFile.lower();

				if ( FileDisk::deleteFile( sFile ) )
					pDoc->pushMessage( CharString().format("%s deleted...", sFile) );
				else
					pDoc->pushMessage( CharString().format("Failed to delete %s...", sFile) );
			}

		OnUpdate(NULL,0,NULL);
	}
}


//-------------------------------------------------------------------------------

BOOL ResourcerList::PreCreateWindow(CREATESTRUCT& cs) 
{
	//m_dwDefaultStyle |= LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_EDITLABELS|LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}

void ResourcerList::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();

	m_ListIcons.Create( 16,16,ILC_COLOR32 | ILC_MASK,0,0); 
	for(int i=0;i<Port::portCount();i++)
		m_ListIcons.Add( Port::portIcon( i ));
	
	int index = m_ListIcons.Add( AfxGetApp()->LoadIcon(IDI_LOCKED) );
	m_ListIcons.SetOverlayImage(index, 1);
	index = m_ListIcons.Add( AfxGetApp()->LoadIcon(IDI_UPDATE) );
	m_ListIcons.SetOverlayImage(index, 2 );
	index = m_ListIcons.Add( AfxGetApp()->LoadIcon(IDI_LOCKEDUPDATE) );
	m_ListIcons.SetOverlayImage(index, 3 );

	GetListCtrl().SetImageList( &m_ListIcons, LVSIL_SMALL );
	GetListCtrl().ModifyStyle( LVS_ICON|LVS_SMALLICON|LVS_LIST, 
		LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_EDITLABELS|LVS_REPORT|LVS_SORTASCENDING);
	//GetListCtrl().ModifyStyleEx( 0, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES  );

	// setup the report columns
	RECT rect;
	GetListCtrl().GetWindowRect(&rect);

	int	cwidth = (rect.right - rect.left) / 5;
	GetListCtrl().InsertColumn(0,"Name",LVCFMT_LEFT,cwidth);
	GetListCtrl().InsertColumn(1,"Type",LVCFMT_LEFT,cwidth);
	GetListCtrl().InsertColumn(2,"Date",LVCFMT_LEFT,cwidth);
	GetListCtrl().InsertColumn(3,"Size",LVCFMT_LEFT,cwidth);
	GetListCtrl().InsertColumn(4,"Widget Size",LVCFMT_LEFT,cwidth);

	DragAcceptFiles();
}

void ResourcerList::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	// release all items from the list control
	GetListCtrl().DeleteAllItems();
	
	// search the current directory
	CString searchPath( CString(pDoc->currentDirectory()) + "*.*" );

	// add the directories to the parent
	WIN32_FIND_DATA	ff;
	HANDLE ffh = FindFirstFile( searchPath , &ff);
	if (ffh == INVALID_HANDLE_VALUE)
		return;

	BOOL found = true;
	while(found)
	{
		if ( (ff.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN)) == 0 )
		{
			CharString	sFileName = CharString( pDoc->currentDirectory() ) + ff.cFileName;

			int		nImage = -1;
			dword	nState = 0;
			CharString	sFileType = Path( ff.cFileName ).extension();
			dword	nFileDate = FileDisk::fileDate( sFileName );
			dword	nFileSize = FileDisk::fileSize( sFileName );
			dword	nWidgetSize = 0;
			
			Port::PortInfo info;
			if ( Port::portInfo( sFileName, info ) )
			{
				nImage = info.type;
				sFileType = Port::portName( info.type );
				nFileDate = info.version;
				nState = info.locked ? INDEXTOOVERLAYMASK(1) : 0;
				nWidgetSize = pDoc->m_Broker.size( info.widgetKey );

				// get broker information
				if ( nWidgetSize == 0 || pDoc->m_Broker.version( info.widgetKey ) < info.version )
				{
					if ( nState )
						nState = INDEXTOOVERLAYMASK(3);
					else
						nState = INDEXTOOVERLAYMASK(2);
				}
			}

			// add list item
			int nItem = GetListCtrl().InsertItem( 0, ff.cFileName , nImage );
			// set the port state, locked or unlocked
			GetListCtrl().SetItemState( nItem, nState, LVIS_OVERLAYMASK );
			// set the type 
			GetListCtrl().SetItemText( nItem, 1, sFileType );
			// set the version
			GetListCtrl().SetItemText( nItem, 2, CTime( nFileDate ).Format("%m/%d/%y %I:%M:%S %p") );
			// set the size
			CString sSize;
			sSize.Format("%.1f k", float(nFileSize) / 1024.0f );
			GetListCtrl().SetItemText( nItem, 3, sSize );

			sSize.Format("%.1f k", float(nWidgetSize) / 1024.0f );
			GetListCtrl().SetItemText( nItem, 4, sSize );
		}

		found = FindNextFile(ffh,&ff);
	}
	FindClose(ffh);
}

void ResourcerList::OnEditRename() 
{
	GetListCtrl().EditLabel( GetListCtrl().GetNextItem( -1, LVNI_FOCUSED) );
}

void ResourcerList::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	if ( pDispInfo->item.pszText != NULL)
	{
		CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
		ASSERT( pDoc );

		CString newName( pDispInfo->item.pszText );
		CString oldName( GetListCtrl().GetItemText( pDispInfo->item.iItem, 0 ) );

		if (! ::MoveFile( pDoc->currentDirectory() + oldName, pDoc->currentDirectory() + newName ) )
		{
			MessageBox("Failed to rename port", oldName );
			return;
		}

		GetListCtrl().SetItemText( pDispInfo->item.iItem, 0, newName );
	}

	*pResult = 0;
}

void ResourcerList::OnViewRefresh() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	OnUpdate(NULL,0,NULL);
}

//-------------------------------------------------------------------------------

void ResourcerList::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu		ctmenu;		// context menu

	ctmenu.CreatePopupMenu();
	ctmenu.AppendMenu(MF_STRING,ID_EDIT_COPY,"Copy");
	ctmenu.AppendMenu(MF_STRING,ID_EDIT_CUT,"Cut");
	ctmenu.AppendMenu(MF_STRING,ID_EDIT_PASTE,"Paste");
	ctmenu.AppendMenu(MF_SEPARATOR);
	ctmenu.AppendMenu(MF_STRING,ID_PORTS_PROPERTIES,"Open");
	ctmenu.AppendMenu(MF_STRING,ID_PORTS_UPGRADE,"Upgrade");
	ctmenu.AppendMenu(MF_STRING,ID_PORTS_SHELLOPEN,"Shell Open");
	ctmenu.AppendMenu(MF_STRING,ID_PORTS_DELETE,"Delete");
	ctmenu.AppendMenu(MF_SEPARATOR);
	ctmenu.AppendMenu(MF_STRING,ID_PORTS_DEPENDENCIES,"Dependencies");
	ctmenu.AppendMenu(MF_SEPARATOR);
	ctmenu.AppendMenu(MF_STRING,ID_DOCUMENT_BUILDSELECTED,"Build");
	ctmenu.AppendMenu(MF_STRING,ID_PORTS_TOUCH,"Touch");

	// track the context menu
	ctmenu.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,GetParent());
	ctmenu.DestroyMenu();
}

const unsigned int	RESOURCER_CLIP_ID = 'RSRC';

void ResourcerList::OnEditCopy() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	//OnUpdate(this,0,NULL);

	// serialize all selected ports into a FileMemory object, then store the data on the clipboard
	FileMemory::Ref pFile = new FileMemory();

	OutStream output( pFile );
	output << RESOURCER_CLIP_ID;
	output << GetListCtrl().GetSelectedCount();

	for(int i=0;i<GetListCtrl().GetItemCount(); i++ )
		if (GetListCtrl().GetItemState(i,LVIS_SELECTED))
		{
			CString portName( GetListCtrl().GetItemText(i,0) );
			CString fileName( pDoc->currentDirectory() + portName );

			Port::Ref port = Port::portLoad( fileName, false );

			if ( port.valid() )
			{
				// create a copy of this port, so all new keys get generated for any internal widgets
				Port::Ref portCopy = (Port *)port->copy();
				if (! portCopy.valid() )
					return;

				// save the port copy
				output << CharString( portName );
				output << portCopy.pointer();
			}
		}

	HGLOBAL pasteGlobal = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, pFile->size() );
	memcpy(GlobalLock(pasteGlobal), pFile->bytes(), pFile->size() );
	GlobalUnlock(pasteGlobal);

	if (OpenClipboard())
	{
		SetClipboardData( CF_PRIVATEFIRST, pasteGlobal );
		CloseClipboard();
	}
}

void ResourcerList::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetListCtrl().GetSelectedCount() > 0 ? TRUE : FALSE );
}

void ResourcerList::OnEditCut() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	// copy the ports before deleting them
	OnEditCopy();	

	for(int i=0;i<GetListCtrl().GetItemCount(); i++ )
		if (GetListCtrl().GetItemState(i,LVIS_SELECTED))
		{
			CString sFile( pDoc->currentDirectory() + GetListCtrl().GetItemText(i,0) );

			// remove actual file
			if ( FileDisk::deleteFile( sFile ) )
				pDoc->pushMessage( CharString().format("%s cut...", sFile) );
			else
				pDoc->pushMessage( CharString().format("Failed to cut %s...", sFile) );
		}

	OnUpdate(NULL,0,NULL);
}

void ResourcerList::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetListCtrl().GetSelectedCount() > 0 ? TRUE : FALSE );
}

void ResourcerList::OnEditPaste() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	if (!OpenClipboard())
		return;

	HGLOBAL hPaste = GetClipboardData( CF_PRIVATEFIRST );
	if ( hPaste == NULL )
	{
		// no data, close the clipboard and exit..
		CloseClipboard();
		return;
	}

	try {
		FileMemory::Ref pFile = new FileMemory( GlobalLock( hPaste ), GlobalSize( hPaste ), true );
		GlobalUnlock( hPaste );
		CloseClipboard();

		InStream input( pFile );
		unsigned int validateId;
		input >> validateId;

		if ( validateId != RESOURCER_CLIP_ID )
			return;			// invalid clipboard data, return...

		int count;
		input >> count;

		for(int i=0;i<count;i++)
		{
			// get the port name
			CharString portName;
			input >> portName;
			// get the object
			Port::Ref port;
			input >> port;
			
			// generate a unique file name
			CharString sFileName( (CharString)pDoc->currentDirectory() + portName );

			int fileIndex = 1;
			while( GetFileAttributes( sFileName ) != 0xffffffff )
			{
				sFileName = (CharString)pDoc->currentDirectory() + Path( portName ).name() + 
					CharString().format("%d", fileIndex++) + Path( portName ).extension(); 
			}

			try {
				OutStream( new FileDisk( sFileName, 
					FileDisk::READ_WRITE) ) << port;
			}
			catch( File::FileError )
			{
				MessageBox(_T("Failed to write port to disk!"), sFileName );
				break;
			}
		}

		OnUpdate( NULL, 0, NULL );
	}
	catch( File::FileError )
	{
		MessageBox( _T("Error reading clipboard data!") );
	}

}

void ResourcerList::OnPortsShellopen() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	for(int i=0;i<GetListCtrl().GetItemCount(); i++ )
		if (GetListCtrl().GetItemState(i,LVIS_SELECTED))
		{
			CharString sFileName = CharString( pDoc->currentDirectory() ) + GetListCtrl().GetItemText(i,0);
			if ( stricmp<char>( Path( sFileName ).extension(), ".PRT" ) == 0 )
			{
				Port::Ref pPort = Port::portLoad( sFileName, false );
				if ( pPort.valid() )
					pPort->shellOpen();
			}
			else
			{
				ShellExecute( NULL, "open", sFileName, NULL, pDoc->currentDirectory(), SW_SHOWNORMAL );
			}
		}
}

void ResourcerList::OnPortsTouch() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	for(int i=0;i<GetListCtrl().GetItemCount(); i++ )
		if (GetListCtrl().GetItemState(i,LVIS_SELECTED))
			pDoc->touchPorts( pDoc->currentDirectory() + GetListCtrl().GetItemText(i,0), false );
}

void ResourcerList::OnPortsUpgrade() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	for(int i=0;i<GetListCtrl().GetItemCount(); i++ )
	{
		if (GetListCtrl().GetItemState(i,LVIS_SELECTED))
			pDoc->upgradePorts( pDoc->currentDirectory() + GetListCtrl().GetItemText(i,0), false );
	}
}

void ResourcerList::OnUpdatePortsUpgrade(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetListCtrl().GetSelectedCount() > 0 ? TRUE : FALSE );
}

void ResourcerList::OnUpdatePortsDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetListCtrl().GetSelectedCount() > 0 ? TRUE : FALSE );
}

void ResourcerList::OnUpdatePortsProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetListCtrl().GetSelectedCount() > 0 ? TRUE : FALSE );
}

void ResourcerList::OnUpdatePortsShellopen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetListCtrl().GetSelectedCount() > 0 ? TRUE : FALSE );
}

void ResourcerList::OnPortsDependencies() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	Array< CharString > dep;
	for(int i=0;i<GetListCtrl().GetItemCount(); i++ )
		if (GetListCtrl().GetItemState(i,LVIS_SELECTED))
		{
			CString port( CString(pDoc->currentDirectory()) + GetListCtrl().GetItemText(i,0) );
			dep.push( CharString( port ) );
		}

	// show dependencies
	CPortDepend( dep ).DoModal();
}

void ResourcerList::OnUpdatePortsDependencies(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetListCtrl().GetSelectedCount() > 0 ? TRUE : FALSE );
}

void ResourcerList::OnDropFiles(HDROP hDropInfo) 
{
	int count = DragQueryFile(hDropInfo,-1,NULL,0);

	CWaitCursor	wait;		// show wait cursor
	for(int f = 0; f < count; f++)
	{
		char charPath[MAX_PATH];
		DragQueryFile(hDropInfo,f,charPath,MAX_PATH);

		ImportFile( charPath );
	}

	OnUpdate(NULL,0,NULL);
}

void ResourcerList::OnFileImport()
{
    CFileDialog dialog(TRUE, NULL, NULL, 
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, 
		_T("All Files|*.*||"));

	TCHAR fileBuffer[ 16 * 1024 + 1];
	fileBuffer[0] = 0;

	dialog.m_ofn.lpstrFile = fileBuffer;
	dialog.m_ofn.nMaxFile = 16 * 1024;

	if (dialog.DoModal() == IDOK)
	{
		POSITION pos = dialog.GetStartPosition();
		while( pos != NULL )
		{
			ImportFile( dialog.GetNextPathName( pos ) );
		}
    }

	OnUpdate(NULL,0,NULL);
}

bool ResourcerList::ImportFile( const char * a_pFile )
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	Path path( a_pFile );

	int portIndex = Port::findImportExt( path.extension() );
	if ( portIndex >= 0 )
	{
		Factory * pFactory = Port::portFactory( portIndex );
		ASSERT( pFactory );

		// create the object and store the it to disk
		Port::Ref pPort = dynamic_cast<Port *>( pFactory->createWidget() );
		// get the port name
		CharString sPortName( (CharString)pDoc->currentDirectory() + path.name() + ".prt" );
		pPort->SetPathName( sPortName, false );

		// update the port
		if ( ! pPort->importFile( a_pFile ) )
		{
			MessageBox(_T("Failed to import file!"), a_pFile );
			return false;
		}

		// save the port to disk
		try {
			OutStream( new FileDisk( sPortName, FileDisk::WRITE ) ) << pPort;
		}
		catch(...)
		{
			MessageBox(_T("Failed to save port!"), sPortName );
			return false;
		}
	}

	return true;
}

void ResourcerList::OnDocumentBuildselected() 
{
	CResourcerDoc *pDoc = dynamic_cast<CResourcerDoc *>(GetDocument());
	ASSERT( pDoc );

	pDoc->beginBuild();

	CWaitCursor	wait;	

	for(int i=0;i<GetListCtrl().GetItemCount(); i++ )
		if (GetListCtrl().GetItemState(i,LVIS_SELECTED))
			pDoc->updateBroker( pDoc->currentDirectory() + GetListCtrl().GetItemText(i,0) );
	
	pDoc->endBuild();
}

void ResourcerList::OnUpdateDocumentBuildselected(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetListCtrl().GetSelectedCount() > 0 ? TRUE : FALSE );
}

//----------------------------------------------------------------------------
// EOF




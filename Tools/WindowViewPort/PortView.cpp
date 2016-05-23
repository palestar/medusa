// PortView.cpp : implementation file
//

#define WINDOWVIEWPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "CreateClass.h"
#include "CreateFunction.h"
#include "CreateVariable.h"

#include "File/FileDisk.h"



extern AFX_EXTENSION_MODULE WindowViewPortDLL;

//----------------------------------------------------------------------------

struct FunctionPrototype
{
	const TCHAR *		className;			// name of the class that generates this message
	const TCHAR *		message;			// id of message
	const TCHAR *		declaration;		// function declaration
	const TCHAR *		implementation;		// function implementation
	const TCHAR *		messagemap;			// message map implementation
};

const int					FUNCTION_PROTOTYPE_COUNT = 27;
const FunctionPrototype		FUNCTION_PROTOTYPES[ FUNCTION_PROTOTYPE_COUNT ] =
{
	{
		_T("NodeInterfaceClient"),
		_T("HM_KEYDOWN"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_KEYUP"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_CHAR"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_DEADCHAR"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_SYSKEYDOWN"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_SYSKEYUP"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_SYSCHAR"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_SYSDEADCHAR"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_MOUSEMOVE"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_LBUTTONDOWN"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_LBUTTONUP"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_RBUTTONDOWN"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_RBUTTONUP"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_MBUTTONDOWN"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_MBUTTONUP"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{
		_T("NodeInterfaceClient"),
		_T("HM_MOUSEWHEEL"),
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP_HM( $$message$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowButton"), 
		_T("WB_OVERBUTTON"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowButton"), 
		_T("WB_BUTTONDOWN"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowButton"), 
		_T("WB_BUTTONUP"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowEdit"), 
		_T("WE_BEGINEDIT"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowEdit"), 
		_T("WE_EDITDONE"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowList"), 
		_T("WL_SELECT_CHANGE"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowList"), 
		_T("WL_SELECT"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowSlider"), 
		_T("WS_BEGIN_SLIDE"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowSlider"), 
		_T("WS_END_SLIDE"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowMenu"), 
		_T("WM_SELECT_CHANGE"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
	{ 
		_T("WindowMenu"), 
		_T("WM_SELECT"), 
		_T("\tbool\t\t\t\t$$function$$(const Message & msg);\r\n"),
		_T("bool $$class$$::$$function$$(const Message & msg)\r\n{\r\n\treturn true;\r\n}\r\n\r\n"),
		_T("\tMESSAGE_MAP( $$message$$, $$node$$, $$function$$);\r\n")
	},
};

//----------------------------------------------------------------------------

const TCHAR * BEGIN_MSG = _T("{{BEGIN_MSG");
const TCHAR * END_MSG = _T("END_MSG}}");
const TCHAR * BEGIN_DATA = _T("{{BEGIN_DATA");
const TCHAR * END_DATA = _T("END_DATA}}");
const TCHAR * BEGIN_MSG_MAP = _T("{{BEGIN_MSG_MAP");
const TCHAR * END_MSG_MAP = _T("END_MSG_MAP}}");
const TCHAR * BEGIN_DATA_INIT = _T("{{BEGIN_DATA_INIT");
const TCHAR * END_DATA_INIT = _T("END_DATA_INIT}}");

//----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_H = _T("");
	m_CPP = _T("");
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Control(pDX, IDC_LIST3, m_Messages);
	DDX_Control(pDX, IDC_BUTTON5, m_AddVariableCtrl);
	DDX_Control(pDX, IDC_BUTTON1, m_AddFunctionCtrl);
	DDX_Control(pDX, IDC_LIST2, m_Variables);
	DDX_Control(pDX, IDC_LIST1, m_Functions);
	DDX_Control(pDX, IDC_TREE1, m_Nodes);
	DDX_Text(pDX, IDC_H, m_H);
	DDX_Text(pDX, IDC_CPP, m_CPP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON1, OnAddFunction)
	ON_BN_CLICKED(IDC_BUTTON5, OnAddVariable)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelectedNode)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST3, OnSelectMessage)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, OnOpenNode)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST3, OnAddFunctionList)
	ON_BN_CLICKED(IDC_BUTTON2, OnNewClass)
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

void CPortView::OnInitialUpdate() 
{
	WindowViewPort * pDoc = dynamic_cast<WindowViewPort *>( GetDocument() );
	ASSERT( pDoc );

	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	CString sourceDirectory = AfxGetApp()->GetProfileString( _T("WindowViewPort"), _T("SourceDirectory"), _T("") );
	m_CPP = sourceDirectory + pDoc->name() + _T(".cpp");
	m_H = sourceDirectory + pDoc->name() + _T(".h");

	if ( !FileDisk::fileExists( CharString( m_CPP ) ) || !FileDisk::fileExists( CharString( m_H ) ) )
		OnNewClass();

	CFormView::OnInitialUpdate();
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	WindowViewPort * pDoc = dynamic_cast<WindowViewPort *>( GetDocument() );
	if ( pDoc == NULL )
		return;

	if ( lHint )
	{
		UpdateData( true );
	}
	else
	{
		UpdateData( false );

		updateNodeTree();
		updateFunctionList();
		updateVariableList();
	}
}

void CPortView::OnAddFunction() 
{
	WindowViewPort * pDoc = dynamic_cast<WindowViewPort *>( GetDocument() );
	ASSERT( pDoc );

	HTREEITEM nodeItem = m_Nodes.GetSelectedItem();
	ASSERT( nodeItem );
	NodeWindowPort * pNode = (NodeWindowPort *)m_Nodes.GetItemData( nodeItem );
	ASSERT( pNode );
	int messageItem = m_Messages.GetNextItem( -1, LVNI_SELECTED );
	ASSERT( messageItem >= 0 );

	// display a dialog, asking the user for a function name
	const FunctionPrototype & function = FUNCTION_PROTOTYPES[ m_Messages.GetItemData( messageItem ) ];

	CCreateFunction dialog;
	dialog.m_Template = function.declaration;
	dialog.m_Template.Replace('\t', ' ' );
	dialog.m_FunctionName = _T("on");
	dialog.m_FunctionName += pNode->name();

	if ( dialog.DoModal() == IDOK )
	{
		if ( dialog.m_FunctionName == function.message )
		{
			MessageBox( _T("Illegal function name!"), dialog.m_FunctionName );
			return;
		}

		// update the header
		CString sHeaderText( loadH() );
		if ( sHeaderText.GetLength() < 1 )
		{
			MessageBox( _T("Failed to load H, cannot add function!"), m_H );
			return;
		}

		CString declare( function.declaration );
		declare.Replace( _T("$$function$$"), dialog.m_FunctionName );

		if (! insertLine( sHeaderText, BEGIN_MSG, END_MSG, declare ) )
		{
			MessageBox( _T("Failed to add function to header!") );
			return;
		}

		// update the source
		CString sSourceText( loadCPP() );
		if ( sSourceText.IsEmpty() )
		{
			MessageBox( _T("Failed to load CPP, cannot add function!"), m_CPP );
			return;
		}

		CString className( pDoc->name() );
		CString messageMap( function.messagemap );
		messageMap.Replace( _T("$$function$$"), dialog.m_FunctionName );
		messageMap.Replace( _T("$$message$$"), function.message );
		CString nodeHash;
		nodeHash.Format(_T("%u"), StringHash( pNode->name() ) );
		messageMap.Replace( _T("$$node$$"), nodeHash );

		if ( ! insertLine( sSourceText,BEGIN_MSG_MAP,END_MSG_MAP, messageMap ) )
		{
			MessageBox( _T("Failed to add function to source!") );
			return;
		}

		CString implement( function.implementation );
		implement.Replace( _T("$$class$$"), className );
		implement.Replace( _T("$$function$$"), dialog.m_FunctionName );

		// just add the implementation onto the end of the file
		sSourceText += implement;

		// save the source
		if ( ! saveCPP( CharString( sSourceText ) ) || !saveH( CharString( sHeaderText ) ) )
		{
			MessageBox(_T("Failed to save the source, cannot add function!"), m_CPP );
			return;
		}

		// COMPLETE!
		updateFunctionList();
	}
}

void CPortView::OnAddVariable() 
{
	const TCHAR * VAR_DECLARE = _T("\t$$class$$ *\t\t\t\tm_p$$variable$$;\r\n");
	const TCHAR * VAR_MAP = _T("\tDATA_MAP( m_p$$variable$$, $$class$$, \"$$node$$\" );\r\n");

	HTREEITEM nodeItem = m_Nodes.GetSelectedItem();
	ASSERT( nodeItem );
	NodeWindowPort * pNode = (NodeWindowPort *)m_Nodes.GetItemData( nodeItem );
	ASSERT( pNode );

	Factory * pFactory = Factory::findFactory( pNode->m_Class );
	if ( pFactory == NULL )
	{
		MessageBox( _T("Failed to find factory for node class!") );
		return;
	}

	CString sClassName = pFactory->className();
	
	CCreateVariable dialog;
	dialog.m_PrototypeTemplate = VAR_DECLARE;
	dialog.m_PrototypeTemplate.Replace('\t', ' ' );
	dialog.m_PrototypeTemplate.Replace(_T("$$class$$"), sClassName );
	dialog.m_VariableName = pNode->name();

	if ( dialog.DoModal() == IDOK )
	{
		// update the header
		CString sHeaderText( loadH() );
		if ( sHeaderText.GetLength() < 1 )
		{
			MessageBox( _T("Failed to load H, cannot add variable!"), m_H );
			return;
		}

		CString sDeclare = VAR_DECLARE;
		sDeclare.Replace( _T("$$class$$"), sClassName );
		sDeclare.Replace( _T("$$variable$$"), dialog.m_VariableName );

		if (! insertLine( sHeaderText, BEGIN_DATA, END_DATA, sDeclare ) )
		{
			MessageBox( _T("Failed to add variable to header!") );
			return;
		}

		// update the source
		CString sSourceText( loadCPP() );
		if ( sSourceText.IsEmpty() )
		{
			MessageBox( _T("Failed to load CPP, cannot add function!"), m_CPP );
			return;
		}

		CString sMap = VAR_MAP;
		sMap.Replace( _T("$$class$$"), sClassName );
		sMap.Replace( _T("$$variable$$"), dialog.m_VariableName );
		sMap.Replace( _T("$$node$$"),  CString( pNode->name() ) );

		if ( ! insertLine( sSourceText,BEGIN_DATA_INIT,END_DATA_INIT, sMap ) )
		{
			MessageBox( _T("Failed to add variable to source!") );
			return;
		}

		// save the source
		if ( ! saveCPP( CharString(sSourceText) ) || !saveH( CharString(sHeaderText) ) )
		{
			MessageBox(_T("Failed to save the source, cannot add variable!"), m_CPP );
			return;
		}

		// COMPLETE!
		updateVariableList();
	}
}

//----------------------------------------------------------------------------

void CPortView::createViewClass()
{
	WindowViewPort * pDoc = dynamic_cast<WindowViewPort *>( GetDocument() );
	ASSERT( pDoc );

	// create the header file if needed!
	if ( !FileDisk::fileExists( CharString( m_H ) ) || 
		( FileDisk::fileExists( CharString( m_H ) ) && MessageBox( _T("A file already exists, do you wish to overwrite?"), m_H, MB_YESNO ) == IDYES) )
	{
		// load the template resource for the header
		HRSRC hHeader = FindResource( WindowViewPortDLL.hModule, MAKEINTRESOURCE(IDR_VIEW_H), _T("TEMPLATE") );
		ASSERT( hHeader );

		// get the size of the header template
		dword nHeaderSize = SizeofResource( WindowViewPortDLL.hModule, hHeader );
		// get the header text
		char * pHeaderText = new char[ nHeaderSize + 1 ];
		memcpy( pHeaderText, LockResource( LoadResource(WindowViewPortDLL.hModule, hHeader ) ), nHeaderSize );
		pHeaderText[ nHeaderSize ] = 0;

		CString sHeaderText = pHeaderText;
		delete [] pHeaderText;
		
		// customize the header information
		CString className( pDoc->name() );
		CString classNameUpper( className );
		classNameUpper.MakeUpper();
		// replace tokens in the header
		sHeaderText.Replace( _T("$$root$$"), className );
		sHeaderText.Replace( _T("$$ROOT$$"), classNameUpper );

		// save header to disk
		if ( ! saveH( CharString(sHeaderText) ) )
		{
			MessageBox(_T("Failed to create header for view class!"), m_H );
			return;
		}
	}

	if (! FileDisk::fileExists( CharString( m_CPP ) ) || 
		( FileDisk::fileExists( CharString( m_H ) ) && MessageBox( _T("A file already exists, do you wish to overwrite?"), m_CPP, MB_YESNO ) == IDYES))
	{
		// generate a new class key
		pDoc->m_ViewClass = ClassKey();
		pDoc->SetModifiedFlag();

		// load the template resource for the source
		HRSRC hSource = FindResource( WindowViewPortDLL.hModule, MAKEINTRESOURCE(IDR_VIEW_CPP), _T("TEMPLATE") );
		ASSERT( hSource );

		// get the size of the header template
		dword nSourceSize = SizeofResource( WindowViewPortDLL.hModule, hSource );

		// get the cpp text
		char * pSourceText = new char[ nSourceSize + 1 ];
		memcpy( pSourceText, LockResource( LoadResource(WindowViewPortDLL.hModule, hSource ) ), nSourceSize );
		pSourceText[ nSourceSize ] = 0;

		CString sSourceText = pSourceText;
		delete [] pSourceText;

		// customize the header information
		CString className( pDoc->name() );
		CString classNameUpper( className );
		classNameUpper.MakeUpper();
		// replace tokens
		sSourceText.Replace( _T("$$root$$"), className );
		sSourceText.Replace( _T("$$ROOT$$"), classNameUpper );
		sSourceText.Replace( _T("$$key$$"), CString( pDoc->m_ViewClass.string() ) );

		// save source to disk
		if (! saveCPP( CharString(sSourceText) ) )
		{
			MessageBox(_T("Failed to create source for view class!"), m_CPP );
			return;
		}
	}
}

void CPortView::updateNodeTree()
{
	WindowViewPort * pDoc = dynamic_cast<WindowViewPort *>( GetDocument() );
	ASSERT( pDoc );

	m_Nodes.DeleteAllItems();

	for(int i=0;i<pDoc->childCount();i++)
		updateNodeTree( TVI_ROOT, pDoc->child(i) );
}

void CPortView::updateNodeTree(HTREEITEM parent, BaseNodePort * pNode)
{
	if ( dynamic_cast< NodeWindowPort * >( pNode ) )
	{
		parent = m_Nodes.InsertItem( CString( pNode->name() ), parent );
		m_Nodes.SetItemData( parent, (dword)pNode );

	}
	for(int i=0;i<pNode->childCount();i++)
		updateNodeTree( parent, pNode->child(i) );
}

void CPortView::OnSelectedNode(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	// rebuild the message list for the selected node
	updateMessageList();
	// enable / disable the add variable button
	m_AddVariableCtrl.EnableWindow( m_Nodes.GetSelectedItem() != NULL );
}

void CPortView::updateMessageList()
{
	m_Messages.DeleteAllItems();

	WindowViewPort * pDoc = dynamic_cast<WindowViewPort *>( GetDocument() );
	ASSERT( pDoc );

	HTREEITEM treeItem = m_Nodes.GetSelectedItem();
	if ( treeItem != NULL )
	{
		// initialize the combobox of the dialog
		NodeWindowPort * pNode = (NodeWindowPort * )m_Nodes.GetItemData( treeItem );
		Factory * pFactory = Factory::findFactory( pNode->m_Class );
		while( pFactory != NULL)
		{
			for(int i=0;i<FUNCTION_PROTOTYPE_COUNT;i++)
				if ( _tcscmp( FUNCTION_PROTOTYPES[ i ].className, CString( pFactory->className() ) ) == 0 )
				{
					int listItem = m_Messages.InsertItem( 0, FUNCTION_PROTOTYPES[i].message );
					m_Messages.SetItemData( listItem, i );
				}

			// recurse up through the factories
			pFactory = Factory::findFactory( pFactory->typeKey() );
		}
	}

	m_AddFunctionCtrl.EnableWindow( false );
}

void CPortView::updateFunctionList()
{
	m_Functions.DeleteAllItems();

	// parse the header file, displaying the function declarations
	CString sHeaderText( loadH() );
	if ( sHeaderText.GetLength() < 1 )
		return;

	int sectionBegin = sHeaderText.Find( BEGIN_MSG, 0) + _tcslen( BEGIN_MSG ); 
	int sectionEnd = sHeaderText.Find( END_MSG, sectionBegin );
	if ( sectionBegin < 0 || sectionEnd < 0 || sectionEnd < sectionBegin )
	{
		MessageBox(_T("Failed to find function section in header!"), m_H );
		return;
	}

	CString msgSection( sHeaderText.Mid( sectionBegin, sectionEnd - sectionBegin ) );
	msgSection.TrimLeft( _T(" \r\n\t") );

	int endLine = msgSection.Find( _T("\r\n"), 0 );
	while( endLine >= 0 )
	{
		CString function( msgSection.Mid( 0, endLine ) );
		function.Replace( '\t', ' ' );

		msgSection.Delete( 0, endLine );
		msgSection.TrimLeft( _T(" \r\n\t") );

		if ( function.GetLength() > 0 )
			m_Functions.InsertItem( 0, function );

		endLine = msgSection.Find( _T("\r\n"), 0 );
	}
	m_AddVariableCtrl.EnableWindow( false );
}

void CPortView::updateVariableList()
{
	m_Variables.DeleteAllItems();

	// parse the header file, displaying the function declarations
	CString sHeaderText( loadH() );
	if ( sHeaderText.GetLength() < 1 )
		return;

	int sectionBegin = sHeaderText.Find( BEGIN_DATA, 0) + _tcslen( BEGIN_DATA ); 
	int sectionEnd = sHeaderText.Find( END_DATA, sectionBegin );
	if ( sectionBegin < 0 || sectionEnd < 0 || sectionEnd < sectionBegin )
	{
		MessageBox(_T("Failed to find function section in header!"), m_H );
		return;
	}

	CString dataSection( sHeaderText.Mid( sectionBegin, sectionEnd - sectionBegin ) );
	dataSection.TrimLeft( _T(" \r\n\t") );

	int endLine = dataSection.Find( _T("\r\n"), 0 );
	while( endLine >= 0 )
	{
		CString variable( dataSection.Mid( 0, endLine ) );
		variable.Replace( '\t', ' ' );

		dataSection.Delete( 0, endLine );
		dataSection.TrimLeft( _T(" \r\n\t") );

		if ( variable.GetLength() > 0 )
			m_Variables.InsertItem( 0, variable );

		endLine = dataSection.Find( _T("\r\n"), 0 );
	}
}

void CPortView::OnSelectMessage(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

	if ( m_Messages.GetNextItem( -1, LVNI_SELECTED ) >= 0 && m_Nodes.GetSelectedItem() )
		m_AddFunctionCtrl.EnableWindow( true );
	else
		m_AddFunctionCtrl.EnableWindow( false );
}

CString CPortView::loadH()
{
	// parse the header file, displaying the function declarations
	CString sHeaderText;

	// load the header into memory for parsing
	char * pHeader = FileDisk::loadTextFile( CharString( m_H ) );
	if ( pHeader != NULL )
	{
		sHeaderText = pHeader;
		delete [] pHeader;
	}

	return( sHeaderText );
}

bool CPortView::saveH( const char * pHeaderText )
{
	return FileDisk::saveTextFile( CharString( m_H ), pHeaderText ); 
}

CString CPortView::loadCPP()
{
	// parse the header file, displaying the function declarations
	CString sSourceText;

	// load the header into memory for parsing
	char * source = FileDisk::loadTextFile( CharString( m_CPP ) );
	if ( source != NULL )
	{
		sSourceText = source;
		delete source;
	}

	return( sSourceText );
}

bool CPortView::saveCPP( const char * pSourceText )
{
	// save source to disk
	return FileDisk::saveTextFile( CharString( m_CPP ), pSourceText );
}

//----------------------------------------------------------------------------

bool CPortView::insertLine( CString & text, const TCHAR * sectionBegin, const TCHAR * sectionEnd, const TCHAR * line )
{
	int begin = text.Find( sectionBegin, 0) + _tcslen( sectionBegin ); 
	int end = text.Find( sectionEnd, begin );
	if ( begin < 0 || end < 0 || end < begin )
		return false;

	int insert = text.Find( _T("\r\n"), begin ) + 2;
	if ( insert < end )
		text.Insert( insert, line );
	else
		return false;

	return true;
}

//----------------------------------------------------------------------------

void CPortView::OnOpenNode(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

	HTREEITEM treeItem = m_Nodes.GetSelectedItem();
	if ( treeItem != NULL )
		((NodeWindowPort * )m_Nodes.GetItemData( treeItem ))->createView();
}


void CPortView::OnAddFunctionList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	OnAddFunction();
}

void CPortView::OnNewClass() 
{
	WindowViewPort * pDoc = dynamic_cast<WindowViewPort *>( GetDocument() );
	ASSERT( pDoc );

	CString sourceDirectory = AfxGetApp()->GetProfileString( _T("WindowViewPort"), _T("SourceDirectory"), _T("") );

	CCreateClass dialog;
	dialog.m_ClassName = pDoc->name();
	dialog.m_Directory = sourceDirectory;

	if ( dialog.DoModal() == IDOK )
	{
		// get the class source file names
		m_CPP = dialog.m_CPP;
		m_H = dialog.m_H;
		// save the directory
		AfxGetApp()->WriteProfileString( _T("WindowViewPort"), _T("SourceDirectory"), dialog.m_Directory );
		// update the nodes name
		pDoc->setName( CharString( dialog.m_ClassName ) );
		pDoc->updateAllViews();

		createViewClass();
	}
}

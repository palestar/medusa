// PortView.cpp : implementation file
//

#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "ChildFrame.h"

#include "System/Platform.h"
#include "Gui3d/InterfaceContext.h"
#include "Audio/Sound.h"
#include "Render3d/NodeSound.h"

#include "Tools/ResourcerDoc/SelectClass.h"
#include "Tools/ResourcerDoc/SelectResource.h"
#include "Tools/ScenePort/Port.h"
#include "Tools/WindowStylePort/Port.h"
#include "Tools/MaterialPort/Port.h"

#include "File/Path.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_DocumentClass = _T("");
	m_Cursor = _T("");
	m_Style = _T("");
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Control(pDX, IDC_LIST1, m_Scenes);
	DDX_Control(pDX, IDC_COMBO1, m_StartingScene);
	DDX_Text(pDX, IDC_EDIT1, m_DocumentClass);
	DDX_Text(pDX, IDC_EDIT3, m_Cursor);
	DDX_Text(pDX, IDC_EDIT2, m_Style);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON1, OnAddScene)
	ON_BN_CLICKED(IDC_BUTTON3, OnRemoveScene)
	ON_BN_CLICKED(IDC_BUTTON4, OnOpenScene)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST1, OnRenameScene)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnChangeStart)
	ON_BN_CLICKED(IDC_BUTTON2, OnExecute)
	ON_BN_CLICKED(IDC_BUTTON5, OnBrowseDocumentClass)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnListOpenScene)
	ON_BN_CLICKED(IDC_BUTTON8, OnOpenCursor)
	ON_BN_CLICKED(IDC_BUTTON9, OnBrowseCursor)
	ON_BN_CLICKED(IDC_BUTTON6, OnOpenStyle)
	ON_BN_CLICKED(IDC_BUTTON7, OnBrowseStyle)
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
	CFormView::OnInitialUpdate();

	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	// initialize the list box
	RECT rect;
	m_Scenes.GetWindowRect(&rect);

	int	cwidth = (rect.right - rect.left) / 2;
	m_Scenes.InsertColumn(0,_T("Id"),LVCFMT_LEFT,cwidth);
	m_Scenes.InsertColumn(1,_T("Scene"),LVCFMT_LEFT,cwidth);

	OnUpdate(NULL,false,NULL);
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	InterfaceContextPort * pDoc = dynamic_cast<InterfaceContextPort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
	}
	else
	{
		m_DocumentClass = Factory::className( pDoc->m_DocumentClass );
		m_Style = pDoc->m_Style;
		m_Cursor = pDoc->m_Cursor;

		UpdateData( false );

		m_StartingScene.ResetContent();
		m_Scenes.DeleteAllItems();
		for(int i=0;i<pDoc->m_Scenes.size();i++)
		{
			int listItem = m_Scenes.InsertItem( 0, pDoc->m_SceneId[i], 0 );
			m_Scenes.SetItemText( listItem, 1, pDoc->m_Scenes[i] );
			m_Scenes.SetItemData( listItem, i );

			int comboItem = m_StartingScene.AddString( pDoc->m_SceneId[i] );
			m_StartingScene.SetItemData( comboItem, i );

			if ( i == pDoc->m_StartingScene )
				m_StartingScene.SetCurSel( comboItem );
		}

	}
}

void CPortView::OnAddScene() 
{
	InterfaceContextPort * pDoc = dynamic_cast<InterfaceContextPort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectResource dialog( CLASS_KEY(ScenePort) );
	if ( dialog.DoModal() == IDOK)
	{
		pDoc->m_SceneId.push( Path( dialog.m_Selected ).name() );
		pDoc->m_Scenes.push( CharString(dialog.m_Selected) );

		pDoc->UpdateAllViews(NULL);
	}
}

void CPortView::OnRemoveScene() 
{
	InterfaceContextPort * pDoc = dynamic_cast<InterfaceContextPort *>( GetDocument() );
	ASSERT( pDoc );

	int selectedItem = m_Scenes.GetNextItem( -1, LVNI_SELECTED );
	if ( selectedItem >= 0 )
	{
		int sceneIndex = m_Scenes.GetItemData( selectedItem );

		pDoc->m_SceneId.remove( sceneIndex );
		pDoc->m_Scenes.remove( sceneIndex );

		pDoc->UpdateAllViews(NULL);
	}
}

void CPortView::OnOpenScene() 
{
	InterfaceContextPort * pDoc = dynamic_cast<InterfaceContextPort *>( GetDocument() );
	ASSERT( pDoc );

	int selectedItem = m_Scenes.GetNextItem( -1, LVNI_SELECTED );
	if ( selectedItem >= 0 )
	{
		int sceneIndex = m_Scenes.GetItemData( selectedItem );
		AfxGetApp()->OpenDocumentFile( pDoc->m_Scenes[sceneIndex] );
	}
}

void CPortView::OnRenameScene(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	if ( pDispInfo->item.pszText != NULL)
	{
		InterfaceContextPort * pDoc = dynamic_cast<InterfaceContextPort *>( GetDocument() );
		ASSERT( pDoc );

		int sceneIndex = m_Scenes.GetItemData( pDispInfo->item.iItem );
		pDoc->m_SceneId[ sceneIndex ] = pDispInfo->item.pszText;
		pDoc->UpdateAllViews(NULL);
	}

	*pResult = 0;
}

void CPortView::OnChangeStart() 
{
	InterfaceContextPort * pDoc = dynamic_cast<InterfaceContextPort *>( GetDocument() );
	ASSERT( pDoc );

	int selectedItem = m_StartingScene.GetCurSel();
	if ( selectedItem >= 0 )
	{
		int sceneIndex = m_StartingScene.GetItemData( selectedItem );
		pDoc->m_StartingScene = sceneIndex;
		pDoc->UpdateAllViews(NULL);
	}
}

void CPortView::OnExecute() 
{
	InterfaceContextPort * pDoc = dynamic_cast<InterfaceContextPort *>( GetDocument() );
	ASSERT( pDoc );

	// build the interface context object
	InterfaceContext::Ref pContext = WidgetCast<InterfaceContext>( pDoc->createResource() );
	if (! pContext )
	{
		MessageBox( _T("Failed to create interface context!"), _T("Failed") );
		return;
	}

	// create the platform object
	Platform::Ref pPlatform = Platform::create();
	if (! pPlatform )
	{
		MessageBox( _T("Failed to create platform object!"), _T("Failed") );
		return;
	}

	// initialize the platform object
	Platform::Config config;
	config.instance = AfxGetApp()->m_hInstance;
	config.className = "InterfaceContext";
	config.applicationName = "InterfaceContext";
	config.displayMode = "0:1024x768x32";
	config.bWindowed = true;
	config.audioKhz = 44100;
	config.audioChan = 2;
	config.audioVoices = 64;
	config.audioBits = 16;

	if (! pPlatform->initialize( config ) )
	{
		MessageBox( _T("Failed to initialize platform!"), _T("Failed"));
		return;
	}

	// THE MAIN GAME LOOP 
	while( pContext->render() );

	// release instance data
	pContext->renderContext()->flushInstanceData();
	// release all sounds
	Sound::releaseAudioBuffers();
}

void CPortView::OnBrowseDocumentClass() 
{
	InterfaceContextPort * pDoc = dynamic_cast<InterfaceContextPort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectClass dialog;
	if ( dialog.DoModal() == IDOK )
	{
		pDoc->m_DocumentClass = dialog.m_Selected;
		pDoc->UpdateAllViews( NULL );
	}
}

void CPortView::OnListOpenScene(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnOpenScene();
	*pResult = 0;
}

void CPortView::OnOpenCursor() 
{
	AfxGetApp()->OpenDocumentFile( m_Cursor );
}

void CPortView::OnBrowseCursor() 
{
	InterfaceContextPort * pDoc = dynamic_cast<InterfaceContextPort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectResource dialog( CLASS_KEY(MaterialPort) );
	if ( dialog.DoModal() == IDOK)
	{
		pDoc->m_Cursor = dialog.m_Selected;
		pDoc->UpdateAllViews(NULL);
	}
}


void CPortView::OnOpenStyle() 
{
	AfxGetApp()->OpenDocumentFile( m_Style );
}

void CPortView::OnBrowseStyle() 
{
	InterfaceContextPort * pDoc = dynamic_cast<InterfaceContextPort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectResource dialog( CLASS_KEY(WindowStylePort) );
	if ( dialog.DoModal() == IDOK)
	{
		pDoc->m_Style = dialog.m_Selected;
		pDoc->UpdateAllViews(NULL);
	}
}

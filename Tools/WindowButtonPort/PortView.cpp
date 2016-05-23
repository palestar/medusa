// PortView.cpp : implementation file
//

#define WINDOWBUTTONPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "Tools/MaterialPort/Port.h"
#include "Tools/ResourcerDoc/SelectResource.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_Tip = _T("");
	m_Toggle = FALSE;
	m_Button = _T("");
	m_Label = _T("");
	m_HotKey = 0;
	m_ButtonDown = FALSE;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Control(pDX, IDC_HOTKEY1, m_HotKeyControl);
	DDX_Text(pDX, IDC_EDIT6, m_Tip);
	DDX_Check(pDX, IDC_CHECK1, m_Toggle);
	DDX_Text(pDX, IDC_EDIT1, m_Button);
	DDX_Text(pDX, IDC_EDIT2, m_Label);
	DDX_Text(pDX, IDC_EDIT5, m_HotKey);
	DDX_Check(pDX, IDC_CHECK2, m_ButtonDown);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnUpdateNode)
	ON_BN_CLICKED(IDC_BUTTON1, OnOpenButton)
	ON_BN_CLICKED(IDC_BUTTON2, OnBrowseButton)
	ON_BN_CLICKED(IDC_BUTTON3, OnAssignHotkey)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK2, OnUpdateNode)
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

	// no rules
	m_HotKeyControl.SetRules( 0, 0 );
}

#pragma warning(disable:4800) // D:\Projects\Hydra\Tools\WindowButtonPort\PortView.cpp(89) : warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	WindowButtonPort * pDoc = dynamic_cast<WindowButtonPort *>( GetDocument() );
	if ( !pDoc )
		return;
	//ASSERT( pDoc );

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Label = (const char *)m_Label;
		pDoc->m_Toggle = m_Toggle;
		pDoc->m_ButtonDown = m_ButtonDown;
		pDoc->m_Tip = (const char *)m_Tip;
		pDoc->m_HotKey = m_HotKey;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_Label = pDoc->m_Label;
		m_Button = pDoc->m_Button;
		m_ButtonDown = pDoc->m_ButtonDown;
		m_Toggle = pDoc->m_Toggle;
		m_Tip = pDoc->m_Tip;
		m_HotKey = pDoc->m_HotKey;

		m_HotKeyControl.SetHotKey( m_HotKey & 0xffff, m_HotKey >> 16 );

		UpdateData( false );
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate(NULL,true,NULL);
}

void CPortView::OnOpenButton() 
{
	AfxGetApp()->OpenDocumentFile( m_Button );
}

void CPortView::OnBrowseButton() 
{
	WindowButtonPort * pDoc = dynamic_cast<WindowButtonPort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectResource dialog( CLASS_KEY( MaterialPort ) );
	if ( dialog.DoModal() == IDOK)
	{
		pDoc->m_Button = (const char *)dialog.m_Selected;
		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
}

void CPortView::OnAssignHotkey() 
{
	UpdateData( true );

	WORD vk, dk;
	m_HotKeyControl.GetHotKey( vk, dk );
	m_HotKey = vk | (dk << 16);
	
	UpdateData( false );	// put new data back to dialog

	OnUpdateNode();
}


// PortView.cpp : implementation file
//

#define NODESOUNDPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "Tools/SoundPort/Port.h"
#include "Tools/ResourcerDoc/SelectResource.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_SoundPort = _T("");
	m_Falloff = 0.0f;
	m_Loop = FALSE;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_EDIT1, m_SoundPort);
	DDX_Text(pDX, IDC_EDIT3, m_Falloff);
	DDV_MinMaxFloat(pDX, m_Falloff, 0.f, 1.f);
	DDX_Check(pDX, IDC_CHECK1, m_Loop);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON1, OnOpenSound)
	ON_BN_CLICKED(IDC_BUTTON2, OnBrowseSound)
	ON_WM_DESTROY()
	ON_EN_KILLFOCUS(IDC_EDIT3, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateNode)
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
}

#pragma warning(disable:4800) // warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeSoundPort * pDoc = dynamic_cast<NodeSoundPort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Falloff = m_Falloff;
		pDoc->m_Loop = m_Loop;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_SoundPort = (const char *)pDoc->m_SoundPort;
		m_Falloff = pDoc->m_Falloff;
		m_Loop = pDoc->m_Loop;

		UpdateData( false );
	}
}

void CPortView::OnDestroy() 
{
	OnUpdateNode();
	CFormView::OnDestroy();
}

void CPortView::OnOpenSound() 
{
	AfxGetApp()->OpenDocumentFile( m_SoundPort );
}

void CPortView::OnBrowseSound() 
{
	NodeSoundPort * pDoc = dynamic_cast<NodeSoundPort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectResource dialog( CLASS_KEY( SoundPort ) );
	if ( dialog.DoModal() == IDOK)
	{
		pDoc->m_SoundPort = (const char *)dialog.m_Selected;
		OnUpdateNode();
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate(NULL,true,NULL);
}

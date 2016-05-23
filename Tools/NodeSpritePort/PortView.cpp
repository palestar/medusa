// PortView.cpp : implementation file
//

#define NODESPRITEPORT_DLL
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
	m_Scale = 0.0f;
	m_MaterialPort = _T("");
	m_AxisLock = -1;
	m_ScaleLock = FALSE;
	m_EnableAlpha = FALSE;
	m_Alpha = 0.0f;
	m_AlphaV = 0.0f;
	m_AlphaA = 0.0f;
	m_ClampBack = FALSE;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_EDIT1, m_Scale);
	DDX_Text(pDX, IDC_EDIT3, m_MaterialPort);
	DDX_CBIndex(pDX, IDC_COMBO1, m_AxisLock);
	DDX_Check(pDX, IDC_CHECK1, m_ScaleLock);
	DDX_Check(pDX, IDC_CHECK2, m_EnableAlpha);
	DDX_Text(pDX, IDC_EDIT2, m_Alpha);
	DDX_Text(pDX, IDC_EDIT4, m_AlphaV);
	DDX_Text(pDX, IDC_EDIT5, m_AlphaA);
	DDX_Check(pDX, IDC_CHECK3, m_ClampBack);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON2, OnOpenMaterial)
	ON_BN_CLICKED(IDC_BUTTON3, OnBrowseMaterial)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK3, OnUpdateNode)
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

#pragma warning( disable:4800 )	// D:\Projects\Hydra\Tools\NodeSpritePort\PortView.cpp(114) : warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeSpritePort * pDoc = dynamic_cast<NodeSpritePort *>( GetDocument() );
	if ( !pDoc )
		return;

	//ASSERT( pDoc );

	static int axisLockIndexed[8] = 
	{
		NodeSprite::LOCKNONE,
		NodeSprite::LOCKX,
		NodeSprite::LOCKY,
		NodeSprite::LOCKZ,
		NodeSprite::LOCKXY,
		NodeSprite::LOCKXZ,
		NodeSprite::LOCKYZ,
		NodeSprite::LOCKXYZ,
	};

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Scale = m_Scale;
		pDoc->m_ScaleLock = m_ScaleLock;
		pDoc->m_ClampBack = m_ClampBack;
		pDoc->m_EnableAlpha = m_EnableAlpha;
		pDoc->m_Alpha = m_Alpha;
		pDoc->m_AlphaV = m_AlphaV;
		pDoc->m_AlphaA = m_AlphaA;
		pDoc->m_AxisLock = (NodeSprite::AxisLock)axisLockIndexed[ m_AxisLock ];

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();	
	}
	else
	{
		m_Scale = pDoc->m_Scale;
		m_ScaleLock = pDoc->m_ScaleLock;
		m_ClampBack = pDoc->m_ClampBack;
		m_EnableAlpha = pDoc->m_EnableAlpha;
		m_Alpha = pDoc->m_Alpha;
		m_AlphaV = pDoc->m_AlphaV;
		m_AlphaA = pDoc->m_AlphaA;
		m_MaterialPort = (const char *)pDoc->m_MaterialPort;
		
		for(int i=0;i<8;i++)
			if ( axisLockIndexed[i] == pDoc->m_AxisLock )
			{
				m_AxisLock = i;
				break;
			}

		UpdateData( false );
	}
}

void CPortView::OnDestroy() 
{
	// make sure any changed data is copied to the object
	OnUpdate(NULL,true,NULL);

	CFormView::OnDestroy();
}

void CPortView::OnOpenMaterial() 
{
	AfxGetApp()->OpenDocumentFile( m_MaterialPort );
}

void CPortView::OnBrowseMaterial() 
{
	NodeSpritePort * pDoc = dynamic_cast<NodeSpritePort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectResource dialog( CLASS_KEY( MaterialPort ) );
	if ( dialog.DoModal() == IDOK)
	{
		pDoc->m_MaterialPort = (const char *)dialog.m_Selected;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();	
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate(NULL,true,NULL);
}

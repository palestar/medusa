// PortView.cpp : implementation file
//

#define NODEPARTICLEFIELD_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "Tools/ResourcerDoc/SelectResource.h"
#include "Tools/MaterialPort/Port.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_MaterialPort = _T("");
	m_ParticleCount = 0;
	m_Scale = 0.0f;
	m_Front = 0.0f;
	m_Back = 0.0f;
	m_VX = 0.0f;
	m_VY = 0.0f;
	m_VZ = 0.0f;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_EDIT1, m_MaterialPort);
	DDX_Text(pDX, IDC_EDIT2, m_ParticleCount);
	DDX_Text(pDX, IDC_EDIT4, m_Scale);
	DDX_Text(pDX, IDC_EDIT5, m_Front);
	DDX_Text(pDX, IDC_EDIT6, m_Back);
	DDX_Text(pDX, IDC_EDIT7, m_VX);
	DDX_Text(pDX, IDC_EDIT8, m_VY);
	DDX_Text(pDX, IDC_EDIT9, m_VZ);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON1, OnBrowseMaterial)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT7, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT8, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT9, OnUpdateNode)
	ON_BN_CLICKED(IDC_BUTTON2, OnOpenMaterial)
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

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeParticleFieldPort * pDoc = dynamic_cast<NodeParticleFieldPort *>( GetDocument() );
	if ( ! pDoc )
		return;
	
	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_ParticleCount = m_ParticleCount;
		pDoc->m_Scale = m_Scale;
		pDoc->m_Front = m_Front;
		pDoc->m_Back = m_Back;
		pDoc->m_Velocity.x = m_VX;
		pDoc->m_Velocity.y = m_VY;
		pDoc->m_Velocity.z = m_VZ;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_MaterialPort = pDoc->m_MaterialPort;
		m_ParticleCount = pDoc->m_ParticleCount;
		m_Scale = pDoc->m_Scale;
		m_Front = pDoc->m_Front;
		m_Back = pDoc->m_Back;
		m_VX = pDoc->m_Velocity.x;
		m_VY = pDoc->m_Velocity.y;
		m_VZ = pDoc->m_Velocity.z;

		UpdateData( false );
	}
}

void CPortView::OnOpenMaterial() 
{
	AfxGetApp()->OpenDocumentFile( m_MaterialPort );
}

void CPortView::OnBrowseMaterial() 
{
	NodeParticleFieldPort * pDoc = dynamic_cast<NodeParticleFieldPort *>( GetDocument() );
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
	OnUpdate( NULL, true, NULL );
}


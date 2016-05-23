// PortView.cpp : implementation file
//

#define NODEPARTICLESYSTEM2PORT_DLL
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
	m_Template = -1;
	m_ParticleCount = 0;
	m_MinLife = 0.0f;
	m_MaxLife = 0.0f;
	m_MinOriginX = 0.0f;
	m_MinOriginY = 0.0f;
	m_MinOriginZ = 0.0f;
	m_MaxOriginX = 0.0f;
	m_MaxOriginY = 0.0f;
	m_MaxOriginZ = 0.0f;
	m_MinVelocityX = 0.0f;
	m_MinVelocityY = 0.0f;
	m_MinVelocityZ = 0.0f;
	m_MaxVelocityX = 0.0f;
	m_MaxVelocityY = 0.0f;
	m_MaxVelocityZ = 0.0f;
	m_MinAccelerationX = 0.0f;
	m_MinAccelerationY = 0.0f;
	m_MinAccelerationZ = 0.0f;
	m_MaxAccelerationX = 0.0f;
	m_MaxAccelerationY = 0.0f;
	m_MaxAccelerationZ = 0.0f;
	m_Life = 0.0f;
	m_Visible = 0.0f;
	m_VisibleV = 0.0f;
	m_VisibleA = 0.0f;
	m_Scale = 0.0f;
	m_ScaleV = 0.0f;
	m_ScaleA = 0.0f;
	m_Alpha = 0.0f;
	m_AlphaV = 0.0f;
	m_AlphaA = 0.0f;
	m_MaterialPort = _T("");
	m_ScaleVar = 0.0f;
	m_OriginType = -1;
	m_ReverseTime = FALSE;
	m_Seed = 0;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_CBIndex(pDX, IDC_COMBO1, m_Template);
	DDX_Text(pDX, IDC_EDIT1, m_ParticleCount);
	DDV_MinMaxInt(pDX, m_ParticleCount, 1, 256);
	DDX_Text(pDX, IDC_EDIT31, m_MinLife);
	DDX_Text(pDX, IDC_EDIT32, m_MaxLife);
	DDX_Text(pDX, IDC_EDIT2, m_MinOriginX);
	DDX_Text(pDX, IDC_EDIT3, m_MinOriginY);
	DDX_Text(pDX, IDC_EDIT4, m_MinOriginZ);
	DDX_Text(pDX, IDC_EDIT5, m_MaxOriginX);
	DDX_Text(pDX, IDC_EDIT6, m_MaxOriginY);
	DDX_Text(pDX, IDC_EDIT7, m_MaxOriginZ);
	DDX_Text(pDX, IDC_EDIT8, m_MinVelocityX);
	DDX_Text(pDX, IDC_EDIT9, m_MinVelocityY);
	DDX_Text(pDX, IDC_EDIT10, m_MinVelocityZ);
	DDX_Text(pDX, IDC_EDIT11, m_MaxVelocityX);
	DDX_Text(pDX, IDC_EDIT12, m_MaxVelocityY);
	DDX_Text(pDX, IDC_EDIT13, m_MaxVelocityZ);
	DDX_Text(pDX, IDC_EDIT14, m_MinAccelerationX);
	DDX_Text(pDX, IDC_EDIT15, m_MinAccelerationY);
	DDX_Text(pDX, IDC_EDIT16, m_MinAccelerationZ);
	DDX_Text(pDX, IDC_EDIT17, m_MaxAccelerationX);
	DDX_Text(pDX, IDC_EDIT18, m_MaxAccelerationY);
	DDX_Text(pDX, IDC_EDIT19, m_MaxAccelerationZ);
	DDX_Text(pDX, IDC_EDIT21, m_Life);
	DDX_Text(pDX, IDC_EDIT22, m_Visible);
	DDV_MinMaxFloat(pDX, m_Visible, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT23, m_VisibleV);
	DDX_Text(pDX, IDC_EDIT24, m_VisibleA);
	DDX_Text(pDX, IDC_EDIT25, m_Scale);
	DDX_Text(pDX, IDC_EDIT26, m_ScaleV);
	DDX_Text(pDX, IDC_EDIT27, m_ScaleA);
	DDX_Text(pDX, IDC_EDIT28, m_Alpha);
	DDV_MinMaxFloat(pDX, m_Alpha, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT29, m_AlphaV);
	DDX_Text(pDX, IDC_EDIT30, m_AlphaA);
	DDX_Text(pDX, IDC_EDIT20, m_MaterialPort);
	DDX_Text(pDX, IDC_EDIT33, m_ScaleVar);
	DDX_CBIndex(pDX, IDC_COMBO3, m_OriginType);
	DDX_Check(pDX, IDC_CHECK1, m_ReverseTime);
	DDX_Text(pDX, IDC_EDIT34, m_Seed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON3, OnApplyTemplate)
	ON_BN_CLICKED(IDC_BUTTON1, OnOpenMaterial)
	ON_BN_CLICKED(IDC_BUTTON2, OnBrowseMaterial)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	ON_BN_CLICKED(IDC_BUTTON4, OnGenerate)
	ON_EN_KILLFOCUS(IDC_EDIT10, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT11, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT12, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT13, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT14, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT15, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT16, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT17, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT18, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT19, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT21, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT22, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT23, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT24, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT25, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT26, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT27, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT28, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT29, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT30, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT31, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT32, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT7, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT8, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT9, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT33, OnUpdateNode)
	ON_CBN_SELCHANGE(IDC_COMBO3, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT34, OnUpdateNode)
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

#pragma warning( disable: 4800 )

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeParticleSystem2Port * pDoc = dynamic_cast<NodeParticleSystem2Port *>( GetDocument() );
	if ( ! pDoc )
		return;
	
	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Seed = m_Seed;
		pDoc->m_ParticleCount = m_ParticleCount;
		pDoc->m_OriginType = m_OriginType;
		pDoc->m_MinOrigin.x = m_MinOriginX;
		pDoc->m_MinOrigin.y = m_MinOriginY;
		pDoc->m_MinOrigin.z = m_MinOriginZ;
		pDoc->m_MaxOrigin.x = m_MaxOriginX;
		pDoc->m_MaxOrigin.y = m_MaxOriginY;
		pDoc->m_MaxOrigin.z = m_MaxOriginZ;
		pDoc->m_MinVelocity.x = m_MinVelocityX;
		pDoc->m_MinVelocity.y = m_MinVelocityY;
		pDoc->m_MinVelocity.z = m_MinVelocityZ;
		pDoc->m_MaxVelocity.x = m_MaxVelocityX;
		pDoc->m_MaxVelocity.y = m_MaxVelocityY;
		pDoc->m_MaxVelocity.z = m_MaxVelocityZ;
		pDoc->m_MinAcceleration.x = m_MinAccelerationX;
		pDoc->m_MinAcceleration.y = m_MinAccelerationY;
		pDoc->m_MinAcceleration.z = m_MinAccelerationZ;
		pDoc->m_MaxAcceleration.x = m_MaxAccelerationX;
		pDoc->m_MaxAcceleration.y = m_MaxAccelerationY;
		pDoc->m_MaxAcceleration.z = m_MaxAccelerationZ;
		pDoc->m_MinLife = m_MinLife;
		pDoc->m_MaxLife = m_MaxLife;
		pDoc->m_ReverseTime = m_ReverseTime;
		pDoc->m_Life = m_Life;
		pDoc->m_MaterialPort = (const char *)m_MaterialPort;
		pDoc->m_Visible = m_Visible;
		pDoc->m_VisibleV = m_VisibleV;
		pDoc->m_VisibleA = m_VisibleA;
		pDoc->m_Scale = m_Scale;
		pDoc->m_ScaleV = m_ScaleV;
		pDoc->m_ScaleA = m_ScaleA;
		pDoc->m_ScaleVar = m_ScaleVar;
		pDoc->m_Alpha = m_Alpha;
		pDoc->m_AlphaV = m_AlphaV;
		pDoc->m_AlphaA = m_AlphaA;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_Seed = pDoc->m_Seed;
		m_ParticleCount = pDoc->m_ParticleCount;
		m_OriginType = pDoc->m_OriginType;
		m_MinOriginX = pDoc->m_MinOrigin.x;
		m_MinOriginY = pDoc->m_MinOrigin.y;
		m_MinOriginZ = pDoc->m_MinOrigin.z;
		m_MaxOriginX = pDoc->m_MaxOrigin.x;
		m_MaxOriginY = pDoc->m_MaxOrigin.y;
		m_MaxOriginZ = pDoc->m_MaxOrigin.z;
		m_MinVelocityX = pDoc->m_MinVelocity.x;
		m_MinVelocityY = pDoc->m_MinVelocity.y;
		m_MinVelocityZ = pDoc->m_MinVelocity.z;
		m_MaxVelocityX = pDoc->m_MaxVelocity.x;
		m_MaxVelocityY = pDoc->m_MaxVelocity.y;
		m_MaxVelocityZ = pDoc->m_MaxVelocity.z;
		m_MinAccelerationX = pDoc->m_MinAcceleration.x;
		m_MinAccelerationY = pDoc->m_MinAcceleration.y;
		m_MinAccelerationZ = pDoc->m_MinAcceleration.z;
		m_MaxAccelerationX = pDoc->m_MaxAcceleration.x;
		m_MaxAccelerationY = pDoc->m_MaxAcceleration.y;
		m_MaxAccelerationZ = pDoc->m_MaxAcceleration.z;
		m_MinLife = pDoc->m_MinLife;
		m_MaxLife = pDoc->m_MaxLife;
		m_ReverseTime = pDoc->m_ReverseTime;
		m_Life = pDoc->m_Life;
		m_MaterialPort = pDoc->m_MaterialPort;
		m_Visible = pDoc->m_Visible;
		m_VisibleV = pDoc->m_VisibleV;
		m_VisibleA = pDoc->m_VisibleA;
		m_Scale = pDoc->m_Scale;
		m_ScaleV = pDoc->m_ScaleV;
		m_ScaleA = pDoc->m_ScaleA;
		m_ScaleVar = pDoc->m_ScaleVar;
		m_Alpha = pDoc->m_Alpha;
		m_AlphaV = pDoc->m_AlphaV;
		m_AlphaA = pDoc->m_AlphaA;

		UpdateData( false );
	}
}

enum Templates
{
	FIRE,
	SMOKE,
	SMALL_EXPLOSION,
	MEDIUM_EXPLOSION,
	LARGE_EXPLOSION,
};

void CPortView::OnApplyTemplate() 
{
	UpdateData( true );
	switch( m_Template )
	{
	case FIRE:
		m_ParticleCount = 25;
		m_OriginType = NodeParticleSystem2Port::BOX;
		m_MinOriginX = -1;
		m_MinOriginY = 0;
		m_MinOriginZ = -1;
		m_MaxOriginX = 1;
		m_MaxOriginY = 0;
		m_MaxOriginZ = 1;
		m_MinVelocityX = 0;
		m_MinVelocityY = 0;
		m_MinVelocityZ = 0;
		m_MaxVelocityX = 0.5f;
		m_MaxVelocityY = 1;
		m_MaxVelocityZ = 0.5f;
		m_MinAccelerationX = 0;
		m_MinAccelerationY = 0;
		m_MinAccelerationZ = 0;
		m_MaxAccelerationX = 0;
		m_MaxAccelerationY = 1;
		m_MaxAccelerationZ = 0;
		m_MinLife = 1;
		m_MaxLife = 2.5f;
		m_Life = 32768.0f;
		m_Visible = 1;
		m_VisibleV = 0;
		m_VisibleA = 0;
		m_Scale = 1;
		m_ScaleV = -0.1f;
		m_ScaleA = 0;
		m_ScaleVar = 0;
		m_Alpha = 1;
		m_AlphaV = -0.25f;
		m_AlphaA = 0;
		break;
	case SMOKE:
		m_ParticleCount = 25;
		m_OriginType = NodeParticleSystem2Port::BOX;
		m_MinOriginX = -1;
		m_MinOriginY = 0;
		m_MinOriginZ = -1;
		m_MaxOriginX = 1;
		m_MaxOriginY = 0;
		m_MaxOriginZ = 1;
		m_MinVelocityX = 0;
		m_MinVelocityY = 0;
		m_MinVelocityZ = 0;
		m_MaxVelocityX = 0.5f;
		m_MaxVelocityY = 1;
		m_MaxVelocityZ = 0.5f;
		m_MinAccelerationX = 0;
		m_MinAccelerationY = 0.75f;
		m_MinAccelerationZ = 0;
		m_MaxAccelerationX = 0;
		m_MaxAccelerationY = 1;
		m_MaxAccelerationZ = 0;
		m_MinLife = 1;
		m_MaxLife = 8;
		m_Life = 32768.0f;
		m_Visible = 1;
		m_VisibleV = 0;
		m_VisibleA = 0;
		m_Scale = 1;
		m_ScaleV = 1;
		m_ScaleA = 0;
		m_ScaleVar = 0;
		m_Alpha = 1;
		m_AlphaV = -0.1f;
		m_AlphaA = 0;
		break;
	case SMALL_EXPLOSION:
	case MEDIUM_EXPLOSION:
	case LARGE_EXPLOSION:
		m_ParticleCount = m_Template == SMALL_EXPLOSION ? 50 : m_Template == MEDIUM_EXPLOSION ? 100 : 150;
		m_OriginType = NodeParticleSystem2Port::BOX;
		m_MinOriginX = 0;
		m_MinOriginY = 0;
		m_MinOriginZ = 0;
		m_MaxOriginX = 0;
		m_MaxOriginY = 0;
		m_MaxOriginZ = 0;
		m_MinVelocityX = -30;
		m_MinVelocityY = -30;
		m_MinVelocityZ = -30;
		m_MaxVelocityX = 30;
		m_MaxVelocityY = 30;
		m_MaxVelocityZ = 30;
		m_MinAccelerationX = 0;
		m_MinAccelerationY = 0;
		m_MinAccelerationZ = 0;
		m_MaxAccelerationX = 0;
		m_MaxAccelerationY = 0;
		m_MaxAccelerationZ = 0;
		m_MinLife = 2;
		m_MaxLife = 2;
		m_Life = 2;
		m_Visible = 1;
		m_VisibleV = 0;
		m_VisibleA = 0;
		m_Scale = 1;
		m_ScaleV = 0;
		m_ScaleA = 0;
		m_ScaleVar = 0;
		m_Alpha = 1;
		m_AlphaV = -0.33f;
		m_AlphaA = 0;
		break;
	default:
		MessageBox("You must select a valid template", "Invalid particle template" );
		return;
	}

	// set the dialog values
	UpdateData( false );

	// apply the template
	OnUpdateNode();
}

void CPortView::OnOpenMaterial() 
{
	AfxGetApp()->OpenDocumentFile( m_MaterialPort );
}

void CPortView::OnBrowseMaterial() 
{
	NodeParticleSystem2Port * pDoc = dynamic_cast<NodeParticleSystem2Port *>( GetDocument() );
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

void CPortView::OnGenerate() 
{
	m_Seed = rand();
	OnUpdateNode();
}

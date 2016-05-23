// PortView.cpp : implementation file
//

#define NODEPARTICLESYSTEMPORT_DLL
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
	m_MaterialPort = _T("");
	m_ParticleRadius = 0.0f;
	m_percentRadiusRandomness = 0.0f;
	m_MeanInitialSpeed = 0.0f;
	m_InitialPositionMaxRadius = 0.0f;
	m_percentInitialSpeedRandomness = 0.0f;
	m_ParticleEmissionRate = 0.0f;
	m_percentEmissionRateRandomness = 0.0f;
	m_ParticleLifetime = 0.0f;
	m_ParticleMass = 0.0f;
	m_BouyantForceMagnitude = 0.0f;
	m_TerminalSpeed = 0.0f;
	m_percentInitialDirectionRandomness_x = 0.0f;
	m_percentInitialDirectionRandomness_y = 0.0f;
	m_percentInitialDirectionRandomness_z = 0.0f;
	m_WindAcceleration_x = 0.0f;
	m_WindAcceleration_y = 0.0f;
	m_WindAcceleration_z = 0.0f;
	m_SystemDuration = 0.0f;
	m_FadeInDuration = 0.0f;
	m_FadeOutDuration = 0.0f;
	m_ParticleLifeRandomness = 0.0f;
	m_percentInitialPositionRandomness_x = 0.0f;
	m_percentInitialPositionRandomness_y = 0.0f;
	m_percentInitialPositionRandomness_z = 0.0f;
	m_InitVel_x = 0.0f;
	m_InitVel_y = 0.0f;
	m_InitVel_z = 0.0f;
	m_ColorStart_r = 0.0f;
	m_ColorStart_g = 0.0f;
	m_ColorStart_b = 0.0f;
	m_ColorEnd_r = 0.0f;
	m_ColorEnd_g = 0.0f;
	m_ColorEnd_b = 0.0f;
	m_maxNumParticles = 0;
	m_BoxDiag_x = 0.0f;
	m_BoxDiag_y = 0.0f;
	m_BoxDiag_z = 0.0f;
	m_bContinuous = FALSE;
	m_bAlphaFadeIn = FALSE;
	m_bAlphaFadeOut = FALSE;
	m_bScaleIn = FALSE;
	m_bScaleOut = FALSE;
	m_bTransformToWorld = FALSE;
	m_bPlayInReverse = FALSE;
	m_bCreateOnSurface = FALSE;
	m_bCreateInBox = FALSE;
	m_bLiveInBox = FALSE;
	m_bRadialVelocity = FALSE;
	m_bAnimateColor = FALSE;
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
	DDX_Text(pDX, IDC_EDIT29, m_ParticleRadius);
	DDV_MinMaxFloat(pDX, m_ParticleRadius, 0.f, 20000.f);
	DDX_Text(pDX, IDC_EDIT30, m_percentRadiusRandomness);
	DDV_MinMaxFloat(pDX, m_percentRadiusRandomness, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT33, m_MeanInitialSpeed);
	DDX_Text(pDX, IDC_EDIT28, m_InitialPositionMaxRadius);
	DDX_Text(pDX, IDC_EDIT6, m_percentInitialSpeedRandomness);
	DDV_MinMaxFloat(pDX, m_percentInitialSpeedRandomness, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT7, m_ParticleEmissionRate);
	DDX_Text(pDX, IDC_EDIT8, m_percentEmissionRateRandomness);
	DDV_MinMaxFloat(pDX, m_percentEmissionRateRandomness, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT13, m_ParticleLifetime);
	DDV_MinMaxFloat(pDX, m_ParticleLifetime, 0.1f, 20000.f);
	DDX_Text(pDX, IDC_EDIT21, m_ParticleMass);
	DDX_Text(pDX, IDC_EDIT22, m_BouyantForceMagnitude);
	DDX_Text(pDX, IDC_EDIT23, m_TerminalSpeed);
	DDX_Text(pDX, IDC_EDIT17, m_percentInitialDirectionRandomness_x);
	DDV_MinMaxFloat(pDX, m_percentInitialDirectionRandomness_x, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT18, m_percentInitialDirectionRandomness_y);
	DDV_MinMaxFloat(pDX, m_percentInitialDirectionRandomness_y, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT19, m_percentInitialDirectionRandomness_z);
	DDV_MinMaxFloat(pDX, m_percentInitialDirectionRandomness_z, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT14, m_WindAcceleration_x);
	DDX_Text(pDX, IDC_EDIT15, m_WindAcceleration_y);
	DDX_Text(pDX, IDC_EDIT16, m_WindAcceleration_z);
	DDX_Text(pDX, IDC_EDIT2, m_SystemDuration);
	DDX_Text(pDX, IDC_EDIT20, m_FadeInDuration);
	DDX_Text(pDX, IDC_EDIT31, m_FadeOutDuration);
	DDX_Text(pDX, IDC_EDIT9, m_ParticleLifeRandomness);
	DDV_MinMaxFloat(pDX, m_ParticleLifeRandomness, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT25, m_percentInitialPositionRandomness_x);
	DDV_MinMaxFloat(pDX, m_percentInitialPositionRandomness_x, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT26, m_percentInitialPositionRandomness_y);
	DDV_MinMaxFloat(pDX, m_percentInitialPositionRandomness_y, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT27, m_percentInitialPositionRandomness_z);
	DDV_MinMaxFloat(pDX, m_percentInitialPositionRandomness_z, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT34, m_InitVel_x);
	DDX_Text(pDX, IDC_EDIT35, m_InitVel_y);
	DDX_Text(pDX, IDC_EDIT36, m_InitVel_z);
	DDX_Text(pDX, IDC_EDIT37, m_ColorStart_r);
	DDV_MinMaxFloat(pDX, m_ColorStart_r, 0.f, 255.f);
	DDX_Text(pDX, IDC_EDIT38, m_ColorStart_g);
	DDV_MinMaxFloat(pDX, m_ColorStart_g, 0.f, 255.f);
	DDX_Text(pDX, IDC_EDIT39, m_ColorStart_b);
	DDV_MinMaxFloat(pDX, m_ColorStart_b, 0.f, 255.f);
	DDX_Text(pDX, IDC_EDIT40, m_ColorEnd_r);
	DDV_MinMaxFloat(pDX, m_ColorEnd_r, 0.f, 255.f);
	DDX_Text(pDX, IDC_EDIT41, m_ColorEnd_g);
	DDV_MinMaxFloat(pDX, m_ColorEnd_g, 0.f, 255.f);
	DDX_Text(pDX, IDC_EDIT42, m_ColorEnd_b);
	DDV_MinMaxFloat(pDX, m_ColorEnd_b, 0.f, 255.f);
	DDX_Text(pDX, IDC_EDIT24, m_maxNumParticles);
	DDV_MinMaxDWord(pDX, m_maxNumParticles, 0, 512);
	DDX_Text(pDX, IDC_EDIT48, m_BoxDiag_x);
	DDX_Text(pDX, IDC_EDIT49, m_BoxDiag_y);
	DDX_Text(pDX, IDC_EDIT50, m_BoxDiag_z);
	DDX_Check(pDX, IDC_CHECK1, m_bContinuous);
	DDX_Check(pDX, IDC_CHECK2, m_bAlphaFadeIn);
	DDX_Check(pDX, IDC_CHECK3, m_bAlphaFadeOut);
	DDX_Check(pDX, IDC_CHECK4, m_bScaleIn);
	DDX_Check(pDX, IDC_CHECK5, m_bScaleOut);
	DDX_Check(pDX, IDC_CHECK6, m_bTransformToWorld);
	DDX_Check(pDX, IDC_CHECK7, m_bPlayInReverse);
	DDX_Check(pDX, IDC_CHECK8, m_bCreateOnSurface);
	DDX_Check(pDX, IDC_CHECK9, m_bCreateInBox);
	DDX_Check(pDX, IDC_CHECK10, m_bLiveInBox);
	DDX_Check(pDX, IDC_CHECK11, m_bRadialVelocity);
	DDX_Check(pDX, IDC_CHECK12, m_bAnimateColor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON1, OnOpenMaterial)
	ON_BN_CLICKED(IDC_BUTTON2, OnBrowseMaterial)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT29, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT30, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT33, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT28, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT7, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT8, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT10, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT11, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT12, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT9, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT13, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT20, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT21, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT22, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT23, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT24, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT25, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT26, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT27, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT17, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT18, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT19, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT14, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT15, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT16, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT31, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT32, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT34, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT35, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT36, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT37, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT38, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT39, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT40, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT41, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT42, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT43, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT48, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT49, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT50, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK2, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK3, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK4, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK5, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK6, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK7, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK8, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK9, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK10, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK11, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK12, OnUpdateNode)
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

#pragma warning( disable:4800 ) // D:\Projects\Hydra\Tools\NodeParticleSystemPort\PortView.cpp(237) : warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeParticleSystemPort * pDoc = dynamic_cast<NodeParticleSystemPort *>( GetDocument() );
	ASSERT( pDoc );
	
	if ( lHint )
	{
		UpdateData( true );
//		pDoc->m_Type = (NodeParticleSystemPort::Type)m_Type;
        pDoc->m_SystemDuration = m_SystemDuration;

        pDoc->m_bContinuous = m_bContinuous;

		// using this data for now
        pDoc->m_bTimeReversible = m_bPlayInReverse;

        pDoc->m_ParticleRadius = m_ParticleRadius;
        pDoc->m_percentRadiusRandomness = m_percentRadiusRandomness;

	    pDoc->m_maxNumParticles = m_maxNumParticles;

        pDoc->m_MeanInitialSpeed = m_MeanInitialSpeed;
        pDoc->m_InitialPositionMaxRadius = m_InitialPositionMaxRadius;
        pDoc->m_percentInitialPositionRandomness.x = m_percentInitialPositionRandomness_x;
        pDoc->m_percentInitialPositionRandomness.y = m_percentInitialPositionRandomness_y;
        pDoc->m_percentInitialPositionRandomness.z = m_percentInitialPositionRandomness_z;
        pDoc->m_percentInitialSpeedRandomness = m_percentInitialSpeedRandomness;
        pDoc->m_percentInitialDirectionRandomness.x = m_percentInitialDirectionRandomness_x;
        pDoc->m_percentInitialDirectionRandomness.y = m_percentInitialDirectionRandomness_y;
        pDoc->m_percentInitialDirectionRandomness.z = m_percentInitialDirectionRandomness_z;

        pDoc->m_ParticleEmissionRate = m_ParticleEmissionRate;
        pDoc->m_percentEmissionRateRandomness = m_percentEmissionRateRandomness;

        pDoc->m_ParticleLifeRandomness = m_ParticleLifeRandomness;
        pDoc->m_ParticleLifetime = m_ParticleLifetime;
	    pDoc->m_FadeInDuration = m_FadeInDuration;
	    pDoc->m_FadeOutDuration = m_FadeOutDuration;

        pDoc->m_ParticleMass = m_ParticleMass;
        pDoc->m_BouyantForceMagnitude = m_BouyantForceMagnitude;
        pDoc->m_WindAcceleration.x = m_WindAcceleration_x;
        pDoc->m_WindAcceleration.y = m_WindAcceleration_y;
        pDoc->m_WindAcceleration.z = m_WindAcceleration_z;
        pDoc->m_TerminalSpeed = m_TerminalSpeed;

        pDoc->m_bAlphaFadeIn = m_bAlphaFadeIn;
        pDoc->m_bAlphaFadeOut = m_bAlphaFadeOut;
        pDoc->m_bScaleIn = m_bScaleIn;
        pDoc->m_bScaleOut = m_bScaleOut;

        pDoc->m_bTransformToWorld = m_bTransformToWorld;
        pDoc->m_bRadialVelocity = m_bRadialVelocity;

        pDoc->m_InitVelocity.x = m_InitVel_x;
        pDoc->m_InitVelocity.y = m_InitVel_y;
        pDoc->m_InitVelocity.z = m_InitVel_z;

        pDoc->m_bAnimateColor = m_bAnimateColor;

        pDoc->m_ColorGradStart.m_R = m_ColorStart_r;
        pDoc->m_ColorGradStart.m_G = m_ColorStart_g;
        pDoc->m_ColorGradStart.m_B = m_ColorStart_b;

        pDoc->m_ColorGradEnd.m_R = m_ColorEnd_r;
        pDoc->m_ColorGradEnd.m_G = m_ColorEnd_g;
        pDoc->m_ColorGradEnd.m_B = m_ColorEnd_b;

		pDoc->m_bCreateOnSurface = m_bCreateOnSurface;
		pDoc->m_bCreateInBox = m_bCreateInBox;
		pDoc->m_bLiveInBox = m_bLiveInBox;

        pDoc->m_BoxDiagonal.x = m_BoxDiag_x;
        pDoc->m_BoxDiagonal.y = m_BoxDiag_y;
        pDoc->m_BoxDiagonal.z = m_BoxDiag_z;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
//		m_Type = pDoc->m_Type;
		m_MaterialPort = (const char *)pDoc->m_MaterialPort;

        m_SystemDuration = pDoc->m_SystemDuration;

        m_bContinuous = pDoc->m_bContinuous;
        m_bPlayInReverse = pDoc->m_bTimeReversible;

        m_ParticleRadius = pDoc->m_ParticleRadius;
        m_percentRadiusRandomness = pDoc->m_percentRadiusRandomness;

	    m_maxNumParticles = pDoc->m_maxNumParticles;

        m_MeanInitialSpeed = pDoc->m_MeanInitialSpeed;
        m_InitialPositionMaxRadius = pDoc->m_InitialPositionMaxRadius;
        m_percentInitialPositionRandomness_x = pDoc->m_percentInitialPositionRandomness.x;
        m_percentInitialPositionRandomness_y = pDoc->m_percentInitialPositionRandomness.y;
        m_percentInitialPositionRandomness_z = pDoc->m_percentInitialPositionRandomness.z;
        m_percentInitialSpeedRandomness = pDoc->m_percentInitialSpeedRandomness;
        m_percentInitialDirectionRandomness_x = pDoc->m_percentInitialDirectionRandomness.x;
        m_percentInitialDirectionRandomness_y = pDoc->m_percentInitialDirectionRandomness.y;
        m_percentInitialDirectionRandomness_z = pDoc->m_percentInitialDirectionRandomness.z;

        m_ParticleEmissionRate = pDoc->m_ParticleEmissionRate;
        m_percentEmissionRateRandomness = pDoc->m_percentEmissionRateRandomness;

        m_ParticleLifeRandomness = pDoc->m_ParticleLifeRandomness;
        m_ParticleLifetime = pDoc->m_ParticleLifetime;
	    m_FadeInDuration = pDoc->m_FadeInDuration;
	    m_FadeOutDuration = pDoc->m_FadeOutDuration;

        m_ParticleMass = pDoc->m_ParticleMass;
        m_BouyantForceMagnitude = pDoc->m_BouyantForceMagnitude;
        m_WindAcceleration_x = pDoc->m_WindAcceleration.x;
        m_WindAcceleration_y = pDoc->m_WindAcceleration.y;
        m_WindAcceleration_z = pDoc->m_WindAcceleration.z;
        m_TerminalSpeed = pDoc->m_TerminalSpeed;

        m_bAlphaFadeIn = pDoc->m_bAlphaFadeIn;
        m_bAlphaFadeOut = pDoc->m_bAlphaFadeOut;
        m_bScaleIn = pDoc->m_bScaleIn;
        m_bScaleOut = pDoc->m_bScaleOut;

        m_bTransformToWorld = pDoc->m_bTransformToWorld;
        m_bRadialVelocity = pDoc->m_bRadialVelocity;

        m_InitVel_x = pDoc->m_InitVelocity.x;
        m_InitVel_y = pDoc->m_InitVelocity.y;
        m_InitVel_z = pDoc->m_InitVelocity.z;

        m_bAnimateColor = pDoc->m_bAnimateColor;

        m_ColorStart_r = pDoc->m_ColorGradStart.m_R;
        m_ColorStart_g = pDoc->m_ColorGradStart.m_G;
        m_ColorStart_b = pDoc->m_ColorGradStart.m_B;

        m_ColorEnd_r = pDoc->m_ColorGradEnd.m_R;
        m_ColorEnd_g = pDoc->m_ColorGradEnd.m_G;
        m_ColorEnd_b = pDoc->m_ColorGradEnd.m_B;

		m_bCreateOnSurface = pDoc->m_bCreateOnSurface;
		m_bCreateInBox = pDoc->m_bCreateInBox;
		m_bLiveInBox = pDoc->m_bLiveInBox;

		m_BoxDiag_x = pDoc->m_BoxDiagonal.x;
		m_BoxDiag_y = pDoc->m_BoxDiagonal.y;
		m_BoxDiag_z = pDoc->m_BoxDiagonal.z;

		UpdateData( false );
	}
}

void CPortView::OnOpenMaterial() 
{
	AfxGetApp()->OpenDocumentFile( m_MaterialPort );
}

void CPortView::OnBrowseMaterial() 
{
	NodeParticleSystemPort * pDoc = dynamic_cast<NodeParticleSystemPort *>( GetDocument() );
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

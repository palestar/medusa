#if !defined(AFX_PORTVIEW_H__D8D093AA_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_PORTVIEW_H__D8D093AA_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPortView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "Resource.h"

class CPortView : public CFormView
{
protected:
	CPortView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPortView)

// Form Data
public:
	//{{AFX_DATA(CPortView)
	enum { IDD = IDD_NODEVIEW };
	CString	m_MaterialPort;
	float	m_ParticleRadius;
	float	m_percentRadiusRandomness;
	float	m_MeanInitialSpeed;
	float	m_InitialPositionMaxRadius;
	float	m_percentInitialSpeedRandomness;
	float	m_ParticleEmissionRate;
	float	m_percentEmissionRateRandomness;
	float	m_ParticleLifetime;
	float	m_ParticleMass;
	float	m_BouyantForceMagnitude;
	float	m_TerminalSpeed;
	float	m_percentInitialDirectionRandomness_x;
	float	m_percentInitialDirectionRandomness_y;
	float	m_percentInitialDirectionRandomness_z;
	float	m_WindAcceleration_x;
	float	m_WindAcceleration_y;
	float	m_WindAcceleration_z;
	float	m_SystemDuration;
	float	m_FadeInDuration;
	float	m_FadeOutDuration;
	float	m_ParticleLifeRandomness;
	float	m_percentInitialPositionRandomness_x;
	float	m_percentInitialPositionRandomness_y;
	float	m_percentInitialPositionRandomness_z;
	float	m_InitVel_x;
	float	m_InitVel_y;
	float	m_InitVel_z;
	float	m_ColorStart_r;
	float	m_ColorStart_g;
	float	m_ColorStart_b;
	float	m_ColorEnd_r;
	float	m_ColorEnd_g;
	float	m_ColorEnd_b;
	DWORD	m_maxNumParticles;
	float	m_BoxDiag_x;
	float	m_BoxDiag_y;
	float	m_BoxDiag_z;
	BOOL	m_bContinuous;
	BOOL	m_bAlphaFadeIn;
	BOOL	m_bAlphaFadeOut;
	BOOL	m_bScaleIn;
	BOOL	m_bScaleOut;
	BOOL	m_bTransformToWorld;
	BOOL	m_bPlayInReverse;
	BOOL	m_bCreateOnSurface;
	BOOL	m_bCreateInBox;
	BOOL	m_bLiveInBox;
	BOOL	m_bRadialVelocity;
	BOOL	m_bAnimateColor;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPortView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPortView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CPortView)
	afx_msg void OnOpenMaterial();
	afx_msg void OnBrowseMaterial();
	afx_msg void OnUpdateNode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__D8D093AA_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_)

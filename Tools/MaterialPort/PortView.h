#if !defined(AFX_PORTVIEW_H__47DD3021_8E7D_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_PORTVIEW_H__47DD3021_8E7D_11D2_B451_00C04F6FF8BD__INCLUDED_

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

#include "ColorBtn.h"
#include "resource.h"
#include "Port.h"

#include "Render3d/Material.h"
#include "Render3d/NodeMesh.h"
#include "Render3d/Scene.h"

class CPortView : public CFormView
{
private:
	// Data
	bool						m_bInitTextureList;
	CColorBtn 					m_Diffuse;
	CColorBtn 					m_Ambient;
	CColorBtn 					m_Emissive;
	CColorBtn 					m_Specular;

protected:
	CPortView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPortView)

	class CPreview : public CStatic
	{
	public:
		//! Construction
		CPreview();
		//! CStatic interface
		virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
		//! Data
		RenderContext				m_Context;
		Scene::Ref					m_Scene;
		NodeMesh::Ref				m_Cube;
		float						m_Time;
		Vector3						m_vLightDirection;

		bool						m_bAmbientLight;
		bool						m_bDiffuseLight;
		CPoint						m_ptLastMousePos;
		bool						m_bRotatingObject;
		bool						m_bRotatingLight;

		//! Mutators
		bool						Initialize();
		//! Message Handlers
		afx_msg void				OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void				OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void				OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void				OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void				OnMouseMove(UINT nFlags, CPoint point);

		DECLARE_MESSAGE_MAP()
	};

// Form Data
public:
	//{{AFX_DATA(CPortView)
	enum { IDD = IDD_FORMVIEW };
	CPreview	m_Preview;
	float	m_Flammable;
	float	m_Friction;
	float	m_Hardness;
	float	m_SpecularPower;
	float	m_Strength;
	int		m_Blending;
	float	m_Fps;
	int		m_Frames;
	float	m_Width;
	float	m_Height;
	BOOL	m_AmbientLight;
	BOOL	m_DiffuseLight;
	int		m_Alpha;
	BOOL	m_bSpin;
	BOOL	m_DoubleSided;
	BOOL	m_LightEnable;
	int		m_Stack;
	CListCtrl	m_cTextureList;
	CString m_sShader;
	BOOL	m_bFixedFunction;
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
	virtual void OnDraw(CDC* pDC);
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
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCancel();
	afx_msg void OnUpdateMaterial();
	afx_msg void OnLight();
	afx_msg void OnAmbient();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CColorBtn;
public:
	afx_msg void OnAddTexture();
	afx_msg void OnRemoveTexture();
	afx_msg void OnEditTexture();
	afx_msg void OnDoubleClickTexture(NMHDR *pNMHDR, LRESULT *pResult);

	void UpdateTextureListControl();
	afx_msg void OnFixedFunction();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTVIEW_H__47DD3021_8E7D_11D2_B451_00C04F6FF8BD__INCLUDED_)

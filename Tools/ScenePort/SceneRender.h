#if !defined(AFX_SCENERENDER_H__3D222E82_95E7_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_SCENERENDER_H__3D222E82_95E7_11D2_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000 
// SceneRender.h : header file
//

#include "Display/DisplayDevice.h"
#include "Render3d/Scene.h"
#include "Audio/AudioDevice.h"
#include "Math/Vector3.h"
#include "Math/Matrix33.h"

/////////////////////////////////////////////////////////////////////////////
// CSceneRender view

class CSceneRender : public CView
{
private:
	// Types
	enum RenderMode
	{
		NODE_SELECT,
		NODE_MOVEXY,
		NODE_MOVEZ,
		NODE_SCALE,
		NODE_YAW,
		NODE_PITCH,
		NODE_ROLL,

		ZOOM,
		DOLLY,
		ROTATE,
	};
	enum ProjectionType
	{
		PERSPECTIVE,
		ORTHOGONAL,
	};
	
	// Data
	Scene::Ref				m_Scene;

	bool					m_LeftDown;
	bool					m_bUpdateNode;
	CPoint					m_LeftPoint;

	RenderMode				m_Mode;
	ProjectionType			m_Projection;

	RenderContext			m_Context;
	float					m_Time;						// render time

	Vector3					m_TargetPosition;			// what is the camera looking at
	float					m_TargetDistance;			// distance of camera from target
	Matrix33				m_TargetFrame;


	DisplayDevice::FillMode
							m_FillMode;

	bool					m_Playing;
	bool					m_Light;					// illuminate the scene
	bool					m_bSoundsEnabled;			// true if sounds should be enabled

protected:
	CSceneRender();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSceneRender)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSceneRender)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSceneRender();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSceneRender)
	afx_msg void OnCameraPerspective();
	afx_msg void OnUpdateCameraPerspective(CCmdUI* pCmdUI);
	afx_msg void OnCameraOrthogonal();
	afx_msg void OnUpdateCameraOrthogonal(CCmdUI* pCmdUI);
	afx_msg void OnCameraXy();
	afx_msg void OnCameraYz();
	afx_msg void OnCameraZx();
	afx_msg void OnCameraIsometric();
	afx_msg void OnCameraSolid();
	afx_msg void OnUpdateCameraSolid(CCmdUI* pCmdUI);
	afx_msg void OnCameraWireframe();
	afx_msg void OnUpdateCameraWireframe(CCmdUI* pCmdUI);
	afx_msg void OnCameraPoint();
	afx_msg void OnUpdateCameraPoint(CCmdUI* pCmdUI);
	afx_msg void OnScenePlay();
	afx_msg void OnUpdateScenePlay(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCameraZoom();
	afx_msg void OnUpdateCameraZoom(CCmdUI* pCmdUI);
	afx_msg void OnCameraDolly();
	afx_msg void OnUpdateCameraDolly(CCmdUI* pCmdUI);
	afx_msg void OnCameraZoomextents();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCameraPan();
	afx_msg void OnUpdateCameraPan(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnViewRefresh();
	afx_msg void OnNodeSelect();
	afx_msg void OnNodeMovexy();
	afx_msg void OnUpdateNodeMovexy(CCmdUI* pCmdUI);
	afx_msg void OnNodeMovez();
	afx_msg void OnUpdateNodeMovez(CCmdUI* pCmdUI);
	afx_msg void OnNodeYaw();
	afx_msg void OnUpdateNodeYaw(CCmdUI* pCmdUI);
	afx_msg void OnNodePitch();
	afx_msg void OnUpdateNodePitch(CCmdUI* pCmdUI);
	afx_msg void OnNodeRoll();
	afx_msg void OnUpdateNodeRoll(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNodeSelect(CCmdUI* pCmdUI);
	afx_msg void OnNodeResetframe();
	afx_msg void OnUpdateNodeResetframe(CCmdUI* pCmdUI);
	afx_msg void OnNodeScale();
	afx_msg void OnUpdateNodeScale(CCmdUI* pCmdUI);
	afx_msg void OnNodeResetposition();
	afx_msg void OnUpdateNodeResetposition(CCmdUI* pCmdUI);
	afx_msg void OnSceneNext();
	afx_msg void OnSceneRewind();
	afx_msg void OnSceneLight();
	afx_msg void OnUpdateSceneLight(CCmdUI* pCmdUI);
	afx_msg void OnScenePrevious();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnRenderSm();
	afx_msg void OnUpdateRenderSm(CCmdUI *pCmdUI);
	afx_msg void OnUpdateRenderSoundenabled(CCmdUI *pCmdUI);
	afx_msg void OnRenderSoundenabled();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCENERENDER_H__3D222E82_95E7_11D2_B451_00C04F6FF8BD__INCLUDED_)

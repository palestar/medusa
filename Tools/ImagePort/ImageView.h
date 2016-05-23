#if !defined(AFX_IMAGEVIEW_H__66B03E0A_7F16_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_IMAGEVIEW_H__66B03E0A_7F16_11D2_B451_00C04F6FF8BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageView.h : header file
//

#include "Draw/Image.h"

/////////////////////////////////////////////////////////////////////////////
// CImageView view

class CImageView : public CScrollView
{
private:
	// Mutators
	void					updateStatusBar();
	void					drawCursor(CDC* pDC);
	void					drawCapture(CDC* pDC);
	void					drawHotSpot(CDC* pDC);

	// Data
	ResourceLink<Image>
							m_Image;			// the viewed image
	BITMAPINFO *			m_pDIB;				// DIB and palette for pDoc->m_Thumb
	CPalette *				m_pPalette;	

	int						m_Zoom;				// magnification
	int						m_Frame;			// current frame
	CPoint					m_Cursor;			// current mouse position
	CPoint					m_HotSpot;			// current hot spot
	CRect					m_Capture;			// current capture rect

	CToolBar				m_ToolBar;	
	CStatusBar				m_StatusBar;	

	bool					m_Scrolling;		// scrolling the view
	bool					m_Selecting;		// selecting a capture box
	bool					m_SetHotSpot;		// selecting hot spot
	bool					m_SetCapture;		// auto capture
	bool					m_Playing;			// animation is playing

protected:
	CImageView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CImageView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CImageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CImageView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnViewerSethotspot();
	afx_msg void OnViewerSetcapture();
	afx_msg void OnViewerZoomin();
	afx_msg void OnViewerZoomout();
	afx_msg void OnViewerPlayframes();
	afx_msg void OnViewerNextframe();
	afx_msg void OnViewerPrevframe();
	afx_msg void OnUpdateViewerZoomin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewerZoomout(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewerNextframe(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewerPlayframes(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewerPrevframe(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewerSetcapture(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewerSethotspot(CCmdUI* pCmdUI);
	afx_msg void OnViewerEditframe();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEVIEW_H__66B03E0A_7F16_11D2_B451_00C04F6FF8BD__INCLUDED_)

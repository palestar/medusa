#if !defined(AFX_SELECTRESOURCE_H__B1D8E0C1_8F4D_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_SELECTRESOURCE_H__B1D8E0C1_8F4D_11D2_A4ED_00C04F6FF8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectResource.h : header file
//

#include "Standard/String.h"
#include "Tools/ResourcerDoc/Port.h"
#include "Tools/ResourcerDoc/ResourcerDoc.h"
#include "Tools/ResourcerDoc/ResourcerDll.h"

// NOTE: pulled from Resource.h, so that we don't have to include that file
#define IDD_SELECT_RESOURCE             1006

/////////////////////////////////////////////////////////////////////////////
// CSelectResource dialog

class DLL CSelectResource : public CDialog
{
// Construction
public:
	CSelectResource( const ClassKey &typeKey, CWnd* pParent = NULL);   // standard constructor

	CString				m_Selected;

private:
	void				BuildTree( const CharString & sPath, const CharString & sFolder, HTREEITEM hParent );

	// Data
	CImageList			m_ImageList;	
	ClassKey			m_TypeKey;					// type of object we're selecting
	CString				m_CurrentDirectory;
	
// Dialog Data
	//{{AFX_DATA(CSelectResource)
	enum { IDD = IDD_SELECT_RESOURCE };
	CTreeCtrl			m_FileTree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectResource)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectResource)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkFileTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNull();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTRESOURCE_H__B1D8E0C1_8F4D_11D2_A4ED_00C04F6FF8CE__INCLUDED_)


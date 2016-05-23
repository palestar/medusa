#pragma once
#include "afxwin.h"
#include "resource.h"

// CTexture dialog

class CTexture : public CDialog
{
	DECLARE_DYNAMIC(CTexture)

public:
	CTexture(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTexture();

// Dialog Data
	enum { IDD = IDD_TEXTURE };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cTypeList;
	int m_nIndex;
	int m_nUV;
	CString m_sImagePort;
	afx_msg void OnOpenImage();
	afx_msg void OnBrowseImage();
	CString m_sParameters;
	int m_nType;
	BOOL m_bMipMap;
};

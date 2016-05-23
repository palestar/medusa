#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "Resource.h"
#include "ResourcerDll.h"

// CSelectBits dialog

class DLL CSelectBits : public CDialog
{
	DECLARE_DYNAMIC(CSelectBits)

public:
	CSelectBits(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectBits();

// Dialog Data
	enum { IDD = IDD_SELECT_BITS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   virtual BOOL OnInitDialog();
protected:
   virtual void OnOK();
public:
   CListCtrl   m_cBitList;

   dword       m_nBits;
   Property *  m_pProperty;
//   afx_msg void OnHdnItemclickList2(NMHDR *pNMHDR, LRESULT *pResult);
//   afx_msg void OnNMClickList2(NMHDR *pNMHDR, LRESULT *pResult);
};

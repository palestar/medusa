#pragma once
#include "afxcmn.h"

#include "Factory/ClassKey.h"
#include "ResourcerDll.h"

//---------------------------------------------------------------------------------------------------

#define IDD_SELECTCLASS                 1018

//---------------------------------------------------------------------------------------------------

// CSelectClass dialog
class DLL CSelectClass : public CDialog
{
	DECLARE_DYNAMIC(CSelectClass)

public:
	CSelectClass(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectClass();

	ClassKey		m_Base;
	ClassKey		m_Selected;

// Dialog Data
	enum { IDD = IDD_SELECTCLASS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_cClassTree;
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
};

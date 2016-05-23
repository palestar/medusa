// EditServer.cpp : implementation file
//

#include "stdafx.h"
#include "ProfilerClient.h"
#include "EditServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditServer dialog


CEditServer::CEditServer(CWnd* pParent /*=NULL*/)
	: CDialog(CEditServer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditServer)
	m_sAddress = _T("");
	m_nPort = 0;
	m_bDisabled = FALSE;
	//}}AFX_DATA_INIT
}


void CEditServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditServer)
	DDX_Text(pDX, IDC_EDIT1, m_sAddress);
	DDX_Text(pDX, IDC_EDIT2, m_nPort);
	DDV_MinMaxInt(pDX, m_nPort, 1, 65535);
	DDX_Check(pDX, IDC_CHECK1, m_bDisabled);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditServer, CDialog)
	//{{AFX_MSG_MAP(CEditServer)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditServer message handlers

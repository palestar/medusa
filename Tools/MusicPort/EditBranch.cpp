// EditBranch.cpp : implementation file
//

#include "stdafx.h"
#include "stdafx.h"
#include "EditBranch.h"

/////////////////////////////////////////////////////////////////////////////
// CEditBranch dialog


CEditBranch::CEditBranch( MusicPort * pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CEditBranch::IDD, pParent), m_pDoc( pDoc )
{
	//{{AFX_DATA_INIT(CEditBranch)
	m_ConditionSelect = -1;
	//}}AFX_DATA_INIT

	m_Trigger = 0;
	m_Segment = 0;
	m_Condition = true;
}


void CEditBranch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditBranch)
	DDX_Control(pDX, IDC_COMBO3, m_SegmentSelect);
	DDX_Control(pDX, IDC_COMBO1, m_TriggerSelect);
	DDX_CBIndex(pDX, IDC_COMBO2, m_ConditionSelect);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditBranch, CDialog)
	//{{AFX_MSG_MAP(CEditBranch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditBranch message handlers

BOOL CEditBranch::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// setup the triggers combo box
	//
	for(int i=0;i<m_pDoc->m_Triggers.size();i++)
	{
		int comboItem = m_TriggerSelect.AddString( m_pDoc->m_Triggers[i] );
		m_TriggerSelect.SetItemData( comboItem, i );

		if ( m_Trigger == i )
			m_TriggerSelect.SetCurSel( comboItem );
	}
	
	m_ConditionSelect = m_Condition ? 0 : 1;

	// setup the segments combo box
	//
	for(int i=0;i<m_pDoc->m_SegmentId.size();i++)
	{
		int comboItem = m_SegmentSelect.AddString( m_pDoc->m_SegmentId[i] );
		m_SegmentSelect.SetItemData( comboItem, i );

		if ( m_Segment == i )
			m_SegmentSelect.SetCurSel( comboItem );
	}
	
	UpdateData( false );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditBranch::OnOK() 
{
	UpdateData( true );
	
	m_Trigger = m_TriggerSelect.GetItemData( m_TriggerSelect.GetCurSel() );
	m_Segment = m_SegmentSelect.GetItemData( m_SegmentSelect.GetCurSel() );
	m_Condition = m_ConditionSelect ? false : true;

	CDialog::OnOK();
}

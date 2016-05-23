// SceneBits.cpp : implementation file
//

#include "stdafx.h"
#include "SceneBits.h"
#include "Port.h"

/////////////////////////////////////////////////////////////////////////////
// CSceneBits

IMPLEMENT_DYNCREATE(CSceneBits, CFormView)

CSceneBits::CSceneBits()
	: CFormView(CSceneBits::IDD)
{
	//{{AFX_DATA_INIT(CSceneBits)
	m_10 = FALSE;
	m_1 = FALSE;
	m_11 = FALSE;
	m_12 = FALSE;
	m_13 = FALSE;
	m_14 = FALSE;
	m_15 = FALSE;
	m_16 = FALSE;
	m_17 = FALSE;
	m_18 = FALSE;
	m_19 = FALSE;
	m_2 = FALSE;
	m_20 = FALSE;
	m_21 = FALSE;
	m_22 = FALSE;
	m_23 = FALSE;
	m_24 = FALSE;
	m_25 = FALSE;
	m_26 = FALSE;
	m_27 = FALSE;
	m_28 = FALSE;
	m_29 = FALSE;
	m_3 = FALSE;
	m_30 = FALSE;
	m_31 = FALSE;
	m_32 = FALSE;
	m_4 = FALSE;
	m_5 = FALSE;
	m_6 = FALSE;
	m_7 = FALSE;
	m_8 = FALSE;
	m_9 = FALSE;
	//}}AFX_DATA_INIT
}

CSceneBits::~CSceneBits()
{
}

void CSceneBits::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSceneBits)
	DDX_Check(pDX, IDC_CHECK10, m_10);
	DDX_Check(pDX, IDC_CHECK1, m_1);
	DDX_Check(pDX, IDC_CHECK11, m_11);
	DDX_Check(pDX, IDC_CHECK12, m_12);
	DDX_Check(pDX, IDC_CHECK13, m_13);
	DDX_Check(pDX, IDC_CHECK14, m_14);
	DDX_Check(pDX, IDC_CHECK15, m_15);
	DDX_Check(pDX, IDC_CHECK16, m_16);
	DDX_Check(pDX, IDC_CHECK17, m_17);
	DDX_Check(pDX, IDC_CHECK18, m_18);
	DDX_Check(pDX, IDC_CHECK19, m_19);
	DDX_Check(pDX, IDC_CHECK2, m_2);
	DDX_Check(pDX, IDC_CHECK20, m_20);
	DDX_Check(pDX, IDC_CHECK21, m_21);
	DDX_Check(pDX, IDC_CHECK22, m_22);
	DDX_Check(pDX, IDC_CHECK23, m_23);
	DDX_Check(pDX, IDC_CHECK24, m_24);
	DDX_Check(pDX, IDC_CHECK25, m_25);
	DDX_Check(pDX, IDC_CHECK26, m_26);
	DDX_Check(pDX, IDC_CHECK27, m_27);
	DDX_Check(pDX, IDC_CHECK28, m_28);
	DDX_Check(pDX, IDC_CHECK29, m_29);
	DDX_Check(pDX, IDC_CHECK3, m_3);
	DDX_Check(pDX, IDC_CHECK30, m_30);
	DDX_Check(pDX, IDC_CHECK31, m_31);
	DDX_Check(pDX, IDC_CHECK32, m_32);
	DDX_Check(pDX, IDC_CHECK4, m_4);
	DDX_Check(pDX, IDC_CHECK5, m_5);
	DDX_Check(pDX, IDC_CHECK6, m_6);
	DDX_Check(pDX, IDC_CHECK7, m_7);
	DDX_Check(pDX, IDC_CHECK8, m_8);
	DDX_Check(pDX, IDC_CHECK9, m_9);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSceneBits, CFormView)
	//{{AFX_MSG_MAP(CSceneBits)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateScene)
	ON_BN_CLICKED(IDC_BUTTON1, OnClearAll)
	ON_BN_CLICKED(IDC_CHECK10, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK11, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK12, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK13, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK14, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK15, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK16, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK17, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK18, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK19, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK2, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK20, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK21, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK22, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK23, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK24, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK25, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK26, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK27, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK28, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK29, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK3, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK30, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK31, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK32, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK4, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK5, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK6, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK7, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK8, OnUpdateScene)
	ON_BN_CLICKED(IDC_CHECK9, OnUpdateScene)
	ON_BN_CLICKED(IDC_BUTTON2, OnSetAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSceneBits diagnostics

#ifdef _DEBUG
void CSceneBits::AssertValid() const
{
	CFormView::AssertValid();
}

void CSceneBits::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSceneBits message handlers

void CSceneBits::OnInitialUpdate() 
{
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);
	
	CFormView::OnInitialUpdate();
}

void CSceneBits::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
		UpdateData( true );

		// move data from view into document
		pDoc->m_Bits = (m_1 ? 0x00000001 : 0x0 ) |
			(m_2 ? 0x00000002 : 0x0 ) |
			(m_3 ? 0x00000004 : 0x0 ) |
			(m_4 ? 0x00000008 : 0x0 ) |
			(m_5 ? 0x00000010 : 0x0 ) |
			(m_6 ? 0x00000020 : 0x0 ) |
			(m_7 ? 0x00000040 : 0x0 ) |
			(m_8 ? 0x00000080 : 0x0 ) |
			(m_9 ? 0x00000100 : 0x0 ) |
			(m_10 ? 0x00000200 : 0x0 ) |
			(m_11 ? 0x00000400 : 0x0 ) |
			(m_12 ? 0x00000800 : 0x0 ) |
			(m_13 ? 0x00001000 : 0x0 ) |
			(m_14 ? 0x00002000 : 0x0 ) |
			(m_15 ? 0x00004000 : 0x0 ) |
			(m_16 ? 0x00008000 : 0x0 ) |
			(m_17 ? 0x00010000 : 0x0 ) |
			(m_18 ? 0x00020000 : 0x0 ) |
			(m_19 ? 0x00040000 : 0x0 ) |
			(m_20 ? 0x00080000 : 0x0 ) |
			(m_21 ? 0x00100000 : 0x0 ) |
			(m_22 ? 0x00200000 : 0x0 ) |
			(m_23 ? 0x00400000 : 0x0 ) |
			(m_24 ? 0x00800000 : 0x0 ) |
			(m_25 ? 0x01000000 : 0x0 ) |
			(m_26 ? 0x02000000 : 0x0 ) |
			(m_27 ? 0x04000000 : 0x0 ) |
			(m_28 ? 0x08000000 : 0x0 ) |
			(m_29 ? 0x10000000 : 0x0 ) |
			(m_30 ? 0x20000000 : 0x0 ) |
			(m_31 ? 0x40000000 : 0x0 ) |
			(m_32 ? 0x80000000 : 0x0 );

		pDoc->UpdateAllViews( this );
	}
	else
	{
		dword bits = pDoc->m_Bits;
		m_1 = bits & 0x00000001 ? true : false;
		m_2 = bits & 0x00000002 ? true : false;
		m_3 = bits & 0x00000004 ? true : false;
		m_4 = bits & 0x00000008 ? true : false;
		m_5 = bits & 0x00000010 ? true : false;
		m_6 = bits & 0x00000020 ? true : false;
		m_7 = bits & 0x00000040 ? true : false;
		m_8 = bits & 0x00000080 ? true : false;
		m_9 = bits & 0x00000100 ? true : false;
		m_10 = bits & 0x00000200 ? true : false;
		m_11 = bits & 0x00000400 ? true : false;
		m_12 = bits & 0x00000800 ? true : false;
		m_13 = bits & 0x00001000 ? true : false;
		m_14 = bits & 0x00002000 ? true : false;
		m_15 = bits & 0x00004000 ? true : false;
		m_16 = bits & 0x00008000 ? true : false;
		m_17 = bits & 0x00010000 ? true : false;
		m_18 = bits & 0x00020000 ? true : false;
		m_19 = bits & 0x00040000 ? true : false;
		m_20 = bits & 0x00080000 ? true : false;
		m_21 = bits & 0x00100000 ? true : false;
		m_22 = bits & 0x00200000 ? true : false;
		m_23 = bits & 0x00400000 ? true : false;
		m_24 = bits & 0x00800000 ? true : false;
		m_25 = bits & 0x01000000 ? true : false;
		m_26 = bits & 0x02000000 ? true : false;
		m_27 = bits & 0x04000000 ? true : false;
		m_28 = bits & 0x08000000 ? true : false;
		m_29 = bits & 0x10000000 ? true : false;
		m_30 = bits & 0x20000000 ? true : false;
		m_31 = bits & 0x40000000 ? true : false;
		m_32 = bits & 0x80000000 ? true : false;

		UpdateData( false );
	}
}

void CSceneBits::OnUpdateScene() 
{
	OnUpdate( NULL, true, NULL );
}

void CSceneBits::OnClearAll() 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	pDoc->m_Bits = 0x0;
	pDoc->UpdateAllViews( NULL );
}

void CSceneBits::OnSetAll() 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	pDoc->m_Bits = 0xffffffff;
	pDoc->UpdateAllViews( NULL );
}



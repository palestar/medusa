// SegmentView.cpp : implementation file
//

#include "stdafx.h"
#include "SegmentView.h" 
#include "Port.h"

#include "Render3d/NodeSpin.h"

/////////////////////////////////////////////////////////////////////////////
// CSegmentView

IMPLEMENT_DYNCREATE(CSegmentView, CFormView)

CSegmentView::CSegmentView()
	: CFormView(CSegmentView::IDD)
{
	//{{AFX_DATA_INIT(CSegmentView)
	m_CurrentSegment = -1;
	m_SegmentName = _T("");
	m_BeginTime = 0.0f;
	m_EndTime = 0.0f;
	m_InFrames = FALSE;
	//}}AFX_DATA_INIT
}

CSegmentView::~CSegmentView()
{
}

void CSegmentView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSegmentView)
	DDX_Control(pDX, IDC_COMBO1, m_Segments);
	DDX_CBIndex(pDX, IDC_COMBO1, m_CurrentSegment);
	DDX_Text(pDX, IDC_EDIT5, m_SegmentName);
	DDX_Text(pDX, IDC_EDIT1, m_BeginTime);
	DDX_Text(pDX, IDC_EDIT2, m_EndTime);
	DDX_Check(pDX, IDC_CHECK1, m_InFrames);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSegmentView, CFormView)
	//{{AFX_MSG_MAP(CSegmentView)
	ON_BN_CLICKED(IDC_BUTTON2, OnAddSegment)
	ON_BN_CLICKED(IDC_BUTTON3, OnRemoveSegment)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnChangedSegment)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK1, OnInFrames)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSegmentView diagnostics

#ifdef _DEBUG
void CSegmentView::AssertValid() const
{
	CFormView::AssertValid();
}

void CSegmentView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSegmentView message handlers

#define SLIDER_BEGIN		(0)
#define SLIDER_END			(4096)
#define SLIDER_DELTA		(SLIDER_END - SLIDER_BEGIN)

void CSegmentView::OnInitialUpdate() 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	CFormView::OnInitialUpdate();
}

void CSegmentView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( lHint )
	{
		UpdateData( true );

		// move data from view into document
		int segment = pDoc->m_CurrentSegment;
		if ( segment >= 0 )
		{
			float beginTime = m_BeginTime;
			float endTime = m_EndTime;

			// translate from frames
			if ( m_InFrames )
			{
				beginTime = beginTime / pDoc->m_Fps;
				endTime = endTime / pDoc->m_Fps;
			}

			pDoc->m_Segments[ segment ].id = m_SegmentName;
			pDoc->m_Segments[ segment ].beginTime = beginTime;
			pDoc->m_Segments[ segment ].endTime = endTime;
		}

		pDoc->UpdateAllViews( this );
	}
	else
	{
		int segment = pDoc->m_CurrentSegment;

		// init the combo box with the scene segments
		m_Segments.ResetContent();
		for(int i=0;i<pDoc->m_Segments.size();i++)
		{
			int comboItem = m_Segments.AddString( pDoc->m_Segments[i].id );
			m_Segments.SetItemData( comboItem, i );
		}

		for(int i=0;i<m_Segments.GetCount();i++)
			if ( m_Segments.GetItemData(i) == (unsigned int)segment )
				m_CurrentSegment = i;

		if ( segment >= 0 )
		{
			m_SegmentName = pDoc->m_Segments[ segment ].id;
			m_BeginTime = pDoc->m_Segments[ segment ].beginTime;
			m_EndTime = pDoc->m_Segments[ segment ].endTime;

			// translate to frame numbers
			if ( m_InFrames )
			{
				m_BeginTime = int(pDoc->m_Fps * m_BeginTime);
				m_EndTime = int(pDoc->m_Fps * m_EndTime );
			}

		}
	
		UpdateData( false );
	}
}

void CSegmentView::OnAddSegment() 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	// add new segment
	Segment & newSegment = pDoc->m_Segments.push();
	newSegment.id = "New Segment";
	newSegment.beginTime = 0;
	newSegment.endTime = 1;

	pDoc->m_CurrentSegment = pDoc->m_Segments.size() - 1;
	pDoc->UpdateAllViews( NULL );
}

void CSegmentView::OnRemoveSegment() 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( pDoc->m_CurrentSegment >= 0 )
	{
		pDoc->m_Segments.remove( pDoc->m_CurrentSegment );
		if ( pDoc->m_CurrentSegment >= pDoc->m_Segments.size() )
			pDoc->m_CurrentSegment--;
		pDoc->UpdateAllViews( this );

		OnUpdate(this,false,NULL);
	}
}

void CSegmentView::OnChangedSegment() 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	// get the latest data from the dialog
	UpdateData( true );

	if ( m_CurrentSegment >= 0 )
	{
		// put the data to the dialog
		pDoc->m_CurrentSegment = m_Segments.GetItemData( m_CurrentSegment );
		pDoc->UpdateAllViews( NULL );
	}
}

void CSegmentView::OnUpdateNode() 
{
	OnUpdate( NULL, true, NULL );
}

void CSegmentView::OnInFrames() 
{
	m_InFrames = !m_InFrames;
	OnUpdate( NULL, false, NULL );
}

// PortView.cpp : implementation file
//

#define MUSICPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "CreateSegments.h"
#include "EditSegment.h"
#include "EditBranch.h"

#include "Audio/Sound.h"



/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD), m_Initialized(false)
{
	m_ActiveTriggerIndex = -1;

	//{{AFX_DATA_INIT(CPortView)
	m_Wave = _T("");
	m_ActiveSegment = _T("");
	m_ActiveTrigger = _T("");
	m_Rate = _T("");
	m_Channels = _T("");
	m_BranchCount = _T("");
	m_Bits = _T("");
	m_Samples = _T("");
	m_SegmentCount = _T("");
	m_TriggerCount = _T("");
	m_BufferSize = 0;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Control(pDX, IDC_LIST3, m_Triggers);
	DDX_Control(pDX, IDC_LIST2, m_Branches);
	DDX_Control(pDX, IDC_LIST1, m_Segments);
	DDX_Text(pDX, IDC_EDIT1, m_Wave);
	DDX_Text(pDX, IDC_ACTIVE_SEGMENT, m_ActiveSegment);
	DDX_Text(pDX, IDC_ACTIVE_TRIGGER, m_ActiveTrigger);
	DDX_Text(pDX, IDC_SAMPLERATE, m_Rate);
	DDX_Text(pDX, IDC_CHANNELS, m_Channels);
	DDX_Text(pDX, IDC_BRANCHES, m_BranchCount);
	DDX_Text(pDX, IDC_BITS, m_Bits);
	DDX_Text(pDX, IDC_SAMPLES, m_Samples);
	DDX_Text(pDX, IDC_SEGMENTS, m_SegmentCount);
	DDX_Text(pDX, IDC_TRIGGERS, m_TriggerCount);
	DDX_Text(pDX, IDC_EDIT2, m_BufferSize);
	DDV_MinMaxInt(pDX, m_BufferSize, 1024, 512000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON1, OnOpenWave)
	ON_BN_CLICKED(IDC_BUTTON2, OnBrowseWave)
	ON_COMMAND(ID_MUSIC_PLAY, OnMusicPlay)
	ON_UPDATE_COMMAND_UI(ID_MUSIC_PLAY, OnUpdateMusicPlay)
	ON_COMMAND(ID_MUSIC_STOP, OnMusicStop)
	ON_UPDATE_COMMAND_UI(ID_MUSIC_STOP, OnUpdateMusicStop)
	ON_COMMAND(ID_SEGMENTS_AUTOCREATE, OnSegmentsAutocreate)
	ON_COMMAND(ID_SEGMENTS_CREATE, OnSegmentsCreate)
	ON_COMMAND(ID_SEGMENTS_DELETE, OnSegmentsDelete)
	ON_UPDATE_COMMAND_UI(ID_SEGMENTS_DELETE, OnUpdateSegmentsDelete)
	ON_COMMAND(ID_SEGMENTS_OPEN, OnSegmentsOpen)
	ON_UPDATE_COMMAND_UI(ID_SEGMENTS_OPEN, OnUpdateSegmentsOpen)
	ON_COMMAND(ID_BRANCHES_CREATE, OnBranchesCreate)
	ON_UPDATE_COMMAND_UI(ID_BRANCHES_CREATE, OnUpdateBranchesCreate)
	ON_COMMAND(ID_BRANCHES_DELETE, OnBranchesDelete)
	ON_UPDATE_COMMAND_UI(ID_BRANCHES_DELETE, OnUpdateBranchesDelete)
	ON_COMMAND(ID_BRANCHES_OPEN, OnBranchesOpen)
	ON_UPDATE_COMMAND_UI(ID_BRANCHES_OPEN, OnUpdateBranchesOpen)
	ON_COMMAND(ID_TRIGGERS_CREATE, OnTriggersCreate)
	ON_COMMAND(ID_TRIGGERS_DELETE, OnTriggersDelete)
	ON_UPDATE_COMMAND_UI(ID_TRIGGERS_DELETE, OnUpdateTriggersDelete)
	ON_COMMAND(ID_TRIGGERS_OPEN, OnTriggersOpen)
	ON_UPDATE_COMMAND_UI(ID_TRIGGERS_OPEN, OnUpdateTriggersOpen)
	ON_COMMAND(ID_TRIGGERS_ACTIVATE, OnTriggersActivate)
	ON_UPDATE_COMMAND_UI(ID_TRIGGERS_ACTIVATE, OnUpdateTriggersActivate)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST1, OnRenameSegment)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnChangeSegment)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST3, OnRenameTrigger)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnSegmentsOpenList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST2, OnBranchesOpenList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST3, OnTriggersOpenList)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateBufferSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPortView diagnostics

#ifdef _DEBUG
void CPortView::AssertValid() const
{
	CFormView::AssertValid();
}

void CPortView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPortView message handlers

BOOL CPortView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	m_ToolBar.Create( pParentWnd, CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|WS_VISIBLE);
	m_ToolBar.LoadToolBar( IDR_MUSICBAR );
	
	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void CPortView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	m_SegmentImage.Create( 16,16,ILC_COLOR32 | ILC_MASK,0,0); 
	m_SegmentImage.Add( AfxGetApp()->LoadIcon(IDI_SEGMENT) );
	m_Segments.SetImageList( &m_SegmentImage, LVSIL_SMALL );

	m_BranchImage.Create( 16,16,ILC_COLOR32 | ILC_MASK,0,0); 
	m_BranchImage.Add( AfxGetApp()->LoadIcon(IDI_BRANCH) );
	m_Branches.SetImageList( &m_BranchImage, LVSIL_SMALL );

	m_TriggerImage.Create( 16,16,ILC_COLOR32 | ILC_MASK,0,0); 
	m_TriggerImage.Add( AfxGetApp()->LoadIcon(IDI_TRIGGER) );
	m_Triggers.SetImageList( &m_TriggerImage, LVSIL_SMALL );

	// initialize the list controls
	//
	RECT rect;
	m_Segments.GetWindowRect(&rect);
	
	int columnWidth = (rect.right - rect.left) / 3;
	m_Segments.InsertColumn(0,_T("Name"), LVCFMT_LEFT, columnWidth );
	m_Segments.InsertColumn(1,_T("Begin"), LVCFMT_LEFT, columnWidth );
	m_Segments.InsertColumn(2,_T("End"), LVCFMT_LEFT, columnWidth);
	
	updateSegmentList();

	m_Branches.GetWindowRect(&rect);
	columnWidth = (rect.right - rect.left) / 3;
	m_Branches.InsertColumn(0,_T("Trigger"), LVCFMT_LEFT, columnWidth );
	m_Branches.InsertColumn(1,_T("Condition"), LVCFMT_LEFT, columnWidth );
	m_Branches.InsertColumn(2,_T("Segment"), LVCFMT_LEFT, columnWidth );

	updateBranchList();

	m_Triggers.GetWindowRect(&rect);
	m_Triggers.InsertColumn(0,_T("Name"), LVCFMT_LEFT, rect.right - rect.left );

	updateTriggerList();

	// initialize the dialog
	//
	updateDialog();

	// initialize the audio device
	//
	m_Device = AudioDevice::create( "AudioDeviceDS" );
	m_Device->initialize( m_hWnd );

	m_Initialized = true;
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	if ( lHint )
	{
		if ( UpdateData( true ) )
		{
			pDoc->m_BufferSize = m_BufferSize;
		}
	}
	else
	{
		m_BufferSize = pDoc->m_BufferSize;
		UpdateData( false );

		updateSegmentList();
		updateBranchList();
		updateTriggerList();
		updateDialog();
	}
}

void CPortView::OnDestroy() 
{
	OnMusicStop();

	CFormView::OnDestroy();
}

void CPortView::OnOpenWave() 
{
    if ( m_Wave.GetLength() > 0 )
        ShellExecute(0, _T("open"), m_Wave, NULL, NULL, SW_SHOWNORMAL);
}

void CPortView::OnBrowseWave() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

    CFileDialog dialog(TRUE, NULL, m_Wave, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, 
		_T("Wave Files (*.wav)|*.wav||"));
	if (dialog.DoModal() == IDOK)
	{
		pDoc->m_WaveFile = dialog.GetPathName();
		pDoc->UpdateAllViews( NULL );

		OnSegmentsAutocreate();
    }
}

void CPortView::OnMusicPlay() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	m_Music = WidgetCast<Music>( pDoc->createResource() );
	if ( m_Music.valid() )
	{
		int focusedItem = m_Segments.GetNextItem(-1,LVNI_FOCUSED);
		
		m_Music->setTrigger( m_ActiveTrigger ); 
		m_Music->setSegment( focusedItem >= 0 ? m_Segments.GetItemData( focusedItem ) : 0 );
		m_Music->play( m_Device, 1.0f );
		
		SetTimer( 0x1, 500, NULL );
	}
}

void CPortView::OnTimer(UINT nIDEvent) 
{
	if ( m_Music.valid() )
	{
		MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
		ASSERT( pDoc );

		int activeSegment = m_Music->activeSegment();

		m_ActiveSegment = pDoc->m_SegmentId[ activeSegment ];
		UpdateData( false );

		// set the active segment as selected in the list control
		//
		LVFINDINFO findInfo;
		findInfo.flags = LVFI_PARAM;
		findInfo.lParam = activeSegment;

		int listItem = m_Segments.FindItem( &findInfo );
		m_Segments.SetItemState( listItem,LVNI_FOCUSED,LVNI_FOCUSED);
		m_Segments.EnsureVisible( listItem, false );
	}

	CFormView::OnTimer(nIDEvent);
}

void CPortView::OnUpdateMusicPlay(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ! m_Music.valid() );
}

void CPortView::OnMusicStop() 
{
	if ( m_Music.valid() )
	{
		KillTimer( 0x1 );

		m_Music->stop();
		m_Music.reset();
	}
}

void CPortView::OnUpdateMusicStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Music.valid() );
}

void CPortView::OnSegmentsAutocreate() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	CCreateSegments dialog;
	if ( dialog.DoModal() == IDOK )
	{
		if ( dialog.m_UseCue )
		{
			// get the region information from the wave file
			//
			Array< Sound::Region > regions;
			if (! Sound::loadRegions( pDoc->m_WaveFile, regions ) )
			{
				MessageBox(_T("Failed to get cue information!"), pDoc->m_WaveFile );
				return;
			}

			int segments = regions.size();

			pDoc->m_SegmentId.allocate( segments );
			pDoc->m_Segments.allocate( segments );

			if ( dialog.m_KeepBranches )
				pDoc->m_Branches.grow( segments - pDoc->m_Branches.size() );
			else
				pDoc->m_Branches.allocate( segments );

			for(int i=0;i<segments;i++)
			{
				pDoc->m_SegmentId[i] = regions[i].name;
				pDoc->m_Segments[i].begin = regions[i].start;
				pDoc->m_Segments[i].end = regions[i].end;
			}

			pDoc->UpdateAllViews( NULL );
		}
		else 
		{
			if ( dialog.m_Samples < 1 )
			{
				MessageBox(_T("Samples per segment must be > 0"));
				return;
			}

			int totalSamples = _tcstol( m_Samples, NULL, 10 );
			int segments = (totalSamples / dialog.m_Samples) + 1;

			pDoc->m_SegmentId.allocate( segments );
			pDoc->m_Segments.allocate( segments );

			if ( dialog.m_KeepBranches )
				pDoc->m_Branches.grow( segments - pDoc->m_Branches.size() );
			else
				pDoc->m_Branches.allocate( segments );

			int sample = 0;
			for(int i=0;i<segments;i++)
			{
				pDoc->m_SegmentId[i] = CharString().format("%4.4d",i );
				pDoc->m_Segments[i].begin = sample;
				
				sample += dialog.m_Samples;
				if ( sample > totalSamples )
					sample = totalSamples;

				pDoc->m_Segments[i].end = sample;
			}

			pDoc->UpdateAllViews( NULL );
		}
	}
}

void CPortView::OnSegmentsCreate() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	unsigned int totalSamples = _tcstol( m_Samples, NULL, 10 );

	CEditSegment dialog;
	dialog.m_Begin = 0;
	dialog.m_End = totalSamples;

	if ( dialog.DoModal() == IDOK )
	{
		if ( dialog.m_End > totalSamples )
			dialog.m_End = totalSamples;
		if ( dialog.m_Begin > dialog.m_End )
			dialog.m_Begin = dialog.m_End;

		MusicPort::Segment newSegment;
		newSegment.begin = dialog.m_Begin;
		newSegment.end = dialog.m_End;

		int segmentIndex = pDoc->m_Segments.size();
		pDoc->m_SegmentId.push("New Segment");
		pDoc->m_Segments.push( newSegment );
		pDoc->m_Branches.push();

		pDoc->UpdateAllViews(NULL);

		m_Segments.SetItemState( segmentIndex,LVNI_FOCUSED|LVNI_SELECTED,LVNI_FOCUSED|LVNI_SELECTED);
		m_Segments.EnsureVisible( segmentIndex, false );
	}
}

void CPortView::OnSegmentsDelete() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	unsigned int totalSamples = _tcstol( m_Samples, NULL, 10 );

	for(int i=m_Segments.GetItemCount()-1;i>=0;i--)
		if ( m_Segments.GetItemState( i, LVIS_SELECTED ) )
		{
			int removeSegment = m_Segments.GetItemData( i );

			// correct all branches to segments above the one being deleted
			//
			for(int j=0;j<pDoc->m_Branches.size();j++)
				for(int k=pDoc->m_Branches[j].size() - 1;k>=0;k--)
					if ( pDoc->m_Branches[j][k].segment > removeSegment )
						pDoc->m_Branches[j][k].segment--;
					else if ( pDoc->m_Branches[j][k].segment == removeSegment )
						 pDoc->m_Branches[j].remove(k);
			
			pDoc->m_SegmentId.remove( removeSegment );
			pDoc->m_Segments.remove( removeSegment );
			pDoc->m_Branches.remove( removeSegment );
		}

	updateSegmentList();
	updateBranchList();
	updateDialog();
}

void CPortView::OnUpdateSegmentsDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Initialized ? m_Segments.GetSelectedCount() > 0 : false );
}

void CPortView::OnSegmentsOpen() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	unsigned int totalSamples = _tcstol( m_Samples, NULL, 10 );

	for(int i=0;i<m_Segments.GetItemCount();i++)
		if ( m_Segments.GetItemState( i, LVIS_SELECTED ) )
		{
			CEditSegment dialog;
	
			int segment = m_Segments.GetItemData( i );
			dialog.m_Begin = pDoc->m_Segments[segment].begin;
			dialog.m_End = pDoc->m_Segments[segment].end;

			if ( dialog.DoModal() == IDOK )
			{
				if ( dialog.m_End > totalSamples )
					dialog.m_End = totalSamples;
				if ( dialog.m_Begin > dialog.m_End )
					dialog.m_Begin = dialog.m_End;

				pDoc->m_Segments[segment].begin = dialog.m_Begin;
				pDoc->m_Segments[segment].end = dialog.m_End;

				CString number;
				number.Format(_T("%d"), pDoc->m_Segments[segment].begin );
				m_Segments.SetItemText( i, 1, number );
				number.Format(_T("%d"), pDoc->m_Segments[segment].end );
				m_Segments.SetItemText( i, 2, number );
			}
		}
}

void CPortView::OnSegmentsOpenList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	OnSegmentsOpen();
}

void CPortView::OnChangeSegment(NMHDR* pNMHDR, LRESULT* pResult) 
{
	updateBranchList();
	*pResult = 0;
}

void CPortView::OnUpdateSegmentsOpen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Initialized ? m_Segments.GetSelectedCount() > 0 : false );
}

void CPortView::OnRenameSegment(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	if ( pDispInfo->item.pszText != NULL)
	{
		MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
		ASSERT( pDoc );

		int segment = m_Segments.GetItemData( pDispInfo->item.iItem );
		pDoc->m_SegmentId[ segment ] = pDispInfo->item.pszText;

		m_Segments.SetItemText( pDispInfo->item.iItem, 0, pDispInfo->item.pszText );
	}

	*pResult = 0;
}

void CPortView::OnBranchesCreate() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	CEditBranch dialog( pDoc );
	if ( dialog.DoModal() == IDOK )
	{
		MusicPort::Branch newBranch;
		newBranch.trigger = dialog.m_Trigger;
		newBranch.condition = dialog.m_Condition;
		newBranch.segment = dialog.m_Segment;

		// add the new branch to all selected segments
		//
		for(int i=0;i<m_Segments.GetItemCount();i++)
			if ( m_Segments.GetItemState( i, LVIS_SELECTED ) )
			{
				int segment = m_Segments.GetItemData( i );
				pDoc->m_Branches[ segment ].push( newBranch );
			}

		updateBranchList();
		updateDialog();
	}
}

void CPortView::OnUpdateBranchesCreate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Initialized ? m_Segments.GetSelectedCount() > 0 && m_Triggers.GetItemCount() > 0 : false );
}

void CPortView::OnBranchesDelete() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	int segment = m_Segments.GetItemData( m_Segments.GetNextItem(-1,LVNI_SELECTED) );

	Array< MusicPort::Branch > removeBranches;

	for(int i=m_Branches.GetItemCount()-1;i>=0;i--)
		if ( m_Branches.GetItemState( i, LVIS_SELECTED ) )
		{
			int branchIndex = m_Branches.GetItemData( i );
			removeBranches.push( pDoc->m_Branches[segment][branchIndex] );
		}

	for(int i=0;i<removeBranches.size();i++)
	{
		MusicPort::Branch & removeBranch = removeBranches[i];

		for(int k=0;k<m_Segments.GetItemCount();k++)
			if ( m_Segments.GetItemState( k, LVIS_SELECTED ) )
			{
				int segment = m_Segments.GetItemData( k );
				
				for(int j=pDoc->m_Branches[segment].size() - 1;j>=0;j--)
					if ( pDoc->m_Branches[segment][j].trigger == removeBranch.trigger &&
						pDoc->m_Branches[segment][j].condition == removeBranch.condition &&
						pDoc->m_Branches[segment][j].segment == removeBranch.segment )
					{
						pDoc->m_Branches[segment].remove(j);
						break;
					}
			}
	}

	updateBranchList();
}

void CPortView::OnUpdateBranchesDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Initialized ? m_Branches.GetSelectedCount() > 0 : false );
}

void CPortView::OnBranchesOpen() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	int segment = m_Segments.GetItemData( m_Segments.GetNextItem(-1,LVNI_SELECTED) );
	for(int i=m_Branches.GetItemCount()-1;i>=0;i--)
		if ( m_Branches.GetItemState( i, LVIS_SELECTED ) )
		{
			int branchIndex = m_Branches.GetItemData( i );
			MusicPort::Branch editBranch = pDoc->m_Branches[segment][branchIndex];

			CEditBranch dialog( pDoc );
			dialog.m_Trigger = editBranch.trigger;
			dialog.m_Condition = editBranch.condition;
			dialog.m_Segment = editBranch.segment;

			if ( dialog.DoModal() == IDOK )
			{
				// update all the branches
				//
				for(int k=0;k<m_Segments.GetItemCount();k++)
					if ( m_Segments.GetItemState( k, LVIS_SELECTED ) )
					{
						int segment = m_Segments.GetItemData( k );
						
						for(int j=0;j<pDoc->m_Branches[segment].size();j++)
							if ( pDoc->m_Branches[segment][j].trigger == editBranch.trigger &&
								pDoc->m_Branches[segment][j].condition == editBranch.condition &&
								pDoc->m_Branches[segment][j].segment == editBranch.segment )
							{
								pDoc->m_Branches[segment][j].trigger = dialog.m_Trigger;
								pDoc->m_Branches[segment][j].condition = dialog.m_Condition;
								pDoc->m_Branches[segment][j].segment = dialog.m_Segment;
								break;
							}
					}
			}
		}

	updateBranchList();
}

void CPortView::OnBranchesOpenList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	OnSegmentsOpen();
}

void CPortView::OnUpdateBranchesOpen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Initialized ? m_Branches.GetSelectedCount() > 0 : false );
}

void CPortView::OnTriggersCreate() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	pDoc->m_Triggers.push("New Trigger");
	updateTriggerList();
	updateDialog();
}

void CPortView::OnTriggersDelete() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	for(int i=m_Triggers.GetItemCount()-1;i>=0;i--)
		if ( m_Triggers.GetItemState( i, LVIS_SELECTED ) )
		{
			unsigned int removeTrigger = m_Triggers.GetItemData( i );

			// remove all branches that use this trigger
			//
			for(int j=0;j<pDoc->m_Branches.size();j++)
				for(int k=pDoc->m_Branches[j].size() - 1;k>=0;k--)
					if ( pDoc->m_Branches[j][k].trigger > removeTrigger )
						pDoc->m_Branches[j][k].trigger--;
					else if ( pDoc->m_Branches[j][k].trigger == removeTrigger )
						pDoc->m_Branches[j].remove( k );

			pDoc->m_Triggers.remove( removeTrigger );
		}

	updateTriggerList();
	updateBranchList();
	updateDialog();
}

void CPortView::OnUpdateTriggersDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Initialized ? m_Triggers.GetSelectedCount() > 0 : false );
}

void CPortView::OnTriggersOpen() 
{
	// TODO: Add your command handler code here
	
}

void CPortView::OnTriggersOpenList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	OnTriggersOpen();
}

void CPortView::OnUpdateTriggersOpen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_Initialized ? m_Triggers.GetSelectedCount() > 0 : false );
}

void CPortView::OnTriggersActivate() 
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	int selectItem = m_Triggers.GetNextItem(-1,LVNI_SELECTED);
	if ( selectItem >= 0 )
	{
		m_ActiveTriggerIndex = m_Triggers.GetItemData( selectItem );
		m_ActiveTrigger = (const char *)pDoc->m_Triggers[m_ActiveTriggerIndex];
	}
	else
	{
		m_ActiveTriggerIndex = -1;
		m_ActiveTrigger = "";
	}

	UpdateData( false );

	if ( m_Music.valid() )
		m_Music->setTrigger( m_ActiveTrigger ); 
}

void CPortView::OnUpdateTriggersActivate(CCmdUI* pCmdUI) 
{}

void CPortView::OnRenameTrigger(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	if ( pDispInfo->item.pszText != NULL)
	{
		MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
		ASSERT( pDoc );

		int trigger = m_Triggers.GetItemData( pDispInfo->item.iItem );
		pDoc->m_Triggers[ trigger ] = pDispInfo->item.pszText;

		m_Triggers.SetItemText( pDispInfo->item.iItem, 0, pDispInfo->item.pszText );
	}

	*pResult = 0;
}

//-------------------------------------------------------------------------------

void CPortView::updateSegmentList()
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	//m_Segments.DeleteAllItems();
	for(int i=0;i<pDoc->m_Segments.size();i++)
	{
		int listItem = m_Segments.InsertItem(i,pDoc->m_SegmentId[i] );

		CString number;
		number.Format("%d", pDoc->m_Segments[i].begin );
		m_Segments.SetItemText( listItem, 1, number );
		number.Format("%d", pDoc->m_Segments[i].end );
		m_Segments.SetItemText( listItem, 2, number );
		m_Segments.SetItemData( listItem, i );
	}

	// remove excess list items
	//
	while( m_Segments.GetItemCount() > pDoc->m_Segments.size() )
		m_Segments.DeleteItem( m_Segments.GetItemCount() - 1 );
}

void CPortView::updateBranchList()
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	// list of displayed branches
	bool listInitlized = false;
	Array< MusicPort::Branch > listBranches;
	Array< int > branchIndex;

	// build a list of branches for all selected segments, only show branches
	// that are in all selected segments

	for(int i=0;i<m_Segments.GetItemCount();i++)
		if ( m_Segments.GetItemState( i, LVIS_SELECTED ) )
		{
			int segment = m_Segments.GetItemData( i );

			Array< MusicPort::Branch > & branches = pDoc->m_Branches[ segment ];
			if (! listInitlized )
			{
				listInitlized = true;
				for(int j=0;j<branches.size();j++)
				{
					listBranches.push( branches[j] );
					branchIndex.push( j );
				}
			}
			else
			{
				for(int j=listBranches.size() - 1;j>=0;j--)
				{
					bool branchFound = false;

					for(int k=0;k<branches.size();k++)
					{
						if ( listBranches[j].trigger == branches[k].trigger &&
							listBranches[j].condition == branches[k].condition &&
							listBranches[j].segment == branches[k].segment )
						{
							branchFound = true;
							break;
						}
					}

					if ( ! branchFound )
					{
						listBranches.remove(j);
						branchIndex.remove(j);
					}
				}
			}
		}

	m_Branches.DeleteAllItems();
	for(int i=0;i<listBranches.size();i++)
	{
		int listItem = m_Branches.InsertItem(0,pDoc->m_Triggers[ listBranches[i].trigger ] );
		m_Branches.SetItemText( listItem, 1, listBranches[i].condition ? _T("==") : _T("!=") );
		m_Branches.SetItemText( listItem, 2, pDoc->m_SegmentId[ listBranches[i].segment ] );
		m_Branches.SetItemData( listItem, branchIndex[i] );
	}
}

void CPortView::updateTriggerList()
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	m_Triggers.DeleteAllItems();
	for(int i=0;i<pDoc->m_Triggers.size();i++)
	{
		int listItem = m_Triggers.InsertItem(0,pDoc->m_Triggers[i] );
		m_Triggers.SetItemData( listItem, i );
	}
}

void CPortView::updateDialog()
{
	MusicPort * pDoc = dynamic_cast<MusicPort *>(GetDocument());
	ASSERT( pDoc );

	m_Wave = (const char *)pDoc->m_WaveFile;
	m_SegmentCount.Format(_T("%d"), pDoc->m_Segments.size() );
	m_TriggerCount.Format(_T("%d"), pDoc->m_Triggers.size() );

	// count the number of branches
	//
	int branchCount = 0;
	for(int i=0;i<pDoc->m_Branches.size();i++)
		branchCount += pDoc->m_Branches[i].size();

	m_BranchCount.Format(_T("%d"), branchCount );

	// get the wave file information
	int rate, channels, bits, size;
	File * pFile = Sound::mapWave( m_Wave, &rate, &channels, &bits, &size, NULL );
	if ( pFile != NULL )
	{
		m_Rate.Format(_T("%d Hz"), rate );
		m_Channels.Format(_T("%d"), channels );
		m_Bits.Format(_T("%d"), bits );
		m_Samples.Format(_T("%d"), size / ((bits >> 3) * channels ) );

		delete pFile;
	}

	UpdateData( false );
}

void CPortView::OnUpdateBufferSize() 
{
	OnUpdate( NULL, true, NULL );	
}

//-------------------------------------------------------------------------------
// EOF



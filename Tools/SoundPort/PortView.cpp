// PortView.cpp : implementation file
//

#define SOUNDPORT_DLL
#include "stdafx.h"
#include "Port.h"
#include "PortView.h"

#include "mmsystem.h"

#include "File/FileDisk.h"



/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_sRate = _T("");
	m_sChannels = _T("");
	m_sBytes = _T("");
	m_sTime = _T("");
	m_sBits = _T("");
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
	// remove the temporary file
	DeleteFile( m_sTempFile );
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Control(pDX, IDC_BUTTON1, m_cEditButton);
	DDX_Control(pDX, IDC_BUTTON2, m_cPlayButton);
	DDX_Text(pDX, IDC_RATE, m_sRate);
	DDX_Text(pDX, IDC_CHANNELS, m_sChannels);
	DDX_Text(pDX, IDC_BYTES, m_sBytes);
	DDX_Text(pDX, IDC_TIME, m_sTime);
	DDX_Text(pDX, IDC_BITS, m_sBits);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON2, OnPlay)
	ON_BN_CLICKED(IDC_BROWSE, OnImport)
	ON_BN_CLICKED(IDC_BUTTON1, OnExport)
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

void CPortView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	UpdateTempFile();
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	SoundPort * pDoc = dynamic_cast<SoundPort *>( GetDocument() );
	ASSERT( pDoc );

	if (! lHint )
	{
		if ( pDoc->m_pSound.valid() )
		{
			m_sRate.Format( _T("%d hz"), pDoc->m_pSound->rate() );
			m_sChannels.Format( _T("%d"), pDoc->m_pSound->channels() );
			m_sBits.Format( _T("%d"), pDoc->m_pSound->bits() );
			m_sBytes.Format( _T("%d"), pDoc->m_pSound->waveSize() );
			m_sTime.Format( _T("%.2f s"), pDoc->m_pSound->time() );
		}
		else
		{
			m_sRate = _T("");
			m_sChannels = _T("");
			m_sBytes = _T("");
			m_sTime = _T("");
			m_sBits = _T("");
		}

		UpdateData(false);
	}
}


void CPortView::OnPlay() 
{
	PlaySound( m_sTempFile, NULL, SND_FILENAME );
}

void CPortView::OnImport() 
{
	SoundPort * pDoc = dynamic_cast<SoundPort *>( GetDocument() );
	ASSERT( pDoc );

    CFileDialog dialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, 
		_T("Wave Files (*.wav)|*.wav||"));
	if (dialog.DoModal() == IDOK)
	{
		if ( pDoc->importFile( dialog.GetPathName() ) )
		{
			pDoc->SetModifiedFlag();
			pDoc->UpdateAllViews(NULL);

			UpdateTempFile();
		}
		else
			MessageBox( dialog.GetPathName(), _T("Failed to import") );
    }
}

void CPortView::OnExport() 
{
	CFileDialog dialog(FALSE, _T(".wav"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Wave Files (*.wav)|*.wav||") );
	if ( dialog.DoModal() == IDOK )
		if ( ! CopyFile( m_sTempFile, dialog.GetPathName(), FALSE ) )
			MessageBox( _T("Failed to export file!"), dialog.GetPathName() );
}


//----------------------------------------------------------------------------

void CPortView::UpdateTempFile()
{
	SoundPort * pDoc = dynamic_cast<SoundPort *>( GetDocument() );
	ASSERT( pDoc );

	if ( m_sTempFile.GetLength() > 0 )
	{
		DeleteFile( m_sTempFile );
		m_sTempFile = _T("");
	}

	// export a WAV file to a temp file
	if ( pDoc->m_pSound.valid() )
	{
		// get the temp directory
		char sTempPath[ MAX_PATH ];
		GetTempPath( MAX_PATH, sTempPath );

		// generate a temp file name
		CharString sTempFile;
		sTempFile.format( "%s%s.wav", sTempPath, pDoc->GetTitle() );

		if ( pDoc->m_pSound->exportWave( sTempFile ) )
			m_sTempFile = sTempFile;
		else
			MessageBox( _T("Failed to export sound file!"), sTempFile );

	}

	m_cEditButton.EnableWindow( m_sTempFile.GetLength() > 0 );
	m_cPlayButton.EnableWindow( m_sTempFile.GetLength() > 0 );
}

//----------------------------------------------------------------------------
// EOF

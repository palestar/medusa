// PortView.cpp : implementation file
//

#define NODECOMPLEXMESHPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"
#include "DirDialog.h"

#include "File/FileDisk.h"
#include "File/Path.h"

#include "Tools/MaterialPort/Port.h"
#include "Tools/ResourcerDoc/SelectResource.h"

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_Triangles = _T("");
	m_Verts = _T("");
	m_FPS = 0;
	m_Reverse = FALSE;
	m_Optimize = FALSE;
	m_EnableLOD = FALSE;
	m_MinLOD = 0.0f;
	m_MaxLOD = 0.0f;
	m_fMeshScale = 0.0f;
	m_CalcNormals = FALSE;
	m_Frames = _T("");
	m_bExportNodeFrames = FALSE;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Control(pDX, IDC_LIST1, m_Materials);
	DDX_Text(pDX, IDC_TRIANGLES, m_Triangles);
	DDX_Text(pDX, IDC_VERTS, m_Verts);
	DDX_Text(pDX, IDC_EDIT5, m_FPS);
	DDX_Check(pDX, IDC_CHECK1, m_Reverse);
	DDX_Check(pDX, IDC_CHECK2, m_Optimize);
	DDX_Check(pDX, IDC_CHECK3, m_EnableLOD);
	DDX_Text(pDX, IDC_EDIT1, m_MinLOD);
	DDX_Text(pDX, IDC_EDIT2, m_MaxLOD);
	DDX_Text(pDX, IDC_EDIT3, m_fMeshScale);
	DDX_Check(pDX, IDC_CHECK4, m_CalcNormals);
	DDX_Text(pDX, IDC_FRAMES, m_Frames);
	DDX_Check(pDX, IDC_CHECK5, m_bExportNodeFrames);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnUpdateNode)
	ON_BN_CLICKED(IDC_BUTTON4, OnBuildLOD)
	ON_BN_CLICKED(IDC_BUTTON3, OnEditMaterial)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDCMaterial)
	ON_BN_CLICKED(IDC_BUTTON2, OnOpenMaterial)
	ON_BN_CLICKED(IDC_BUTTON5, OnRebase)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK2, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK3, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK4, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK5, OnUpdateNode)
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

	// initialize the list control
	m_Materials.InsertColumn(0,_T("ID:TRI"),LVCFMT_LEFT,50);
	m_Materials.InsertColumn(1,_T("Material"),LVCFMT_LEFT,250);

	OnUpdate( NULL, 0, NULL );
}

#pragma warning(disable:4800)

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeComplexMeshPort * pDoc = dynamic_cast<NodeComplexMeshPort *>( GetDocument() );
	if ( pDoc == NULL )
		return;
	//ASSERT( pDoc );

	if ( lHint )
	{
		if ( UpdateData( true ) )
		{
			if ( m_EnableLOD && m_MaxLOD < m_MinLOD )
			{
				MessageBox(_T("Max LOD must be larger than the min LOD!"));
				return;
			}

			pDoc->m_Fps = m_FPS;
			pDoc->m_fMeshScale = m_fMeshScale;
			pDoc->m_Reverse = m_Reverse;
			pDoc->m_Optimize = m_Optimize;
			pDoc->m_CalcNormals = m_CalcNormals;
			pDoc->m_EnableLOD = m_EnableLOD;
			pDoc->m_MinLOD = m_MinLOD;
			pDoc->m_MaxLOD = m_MaxLOD;
			pDoc->m_bExportNodeFrames = m_bExportNodeFrames;

			pDoc->UpdateAllViews(NULL);
			pDoc->setUpdate();
			pDoc->updateParent();
		}
	}
	else
	{
		m_Verts.Format(_T("%d"), pDoc->m_Verts.size() );

		int triangles = 0;
		for(int k=0;k<pDoc->m_Triangles.size();k++)						// frames
			for(int i=0;i<pDoc->m_Triangles[k].size();i++)				// materials 
				triangles += pDoc->m_Triangles[k][i].size();

		m_Materials.DeleteAllItems();
		for(int k=0;k<pDoc->m_Materials.size();k++)
		{
			// calculate number of triangles for this material
			int triangles = 0;
			for(int i=0;i<pDoc->m_Triangles.size();i++)
				triangles += pDoc->m_Triangles[i][k].size();

			// add material to list
			int item = m_Materials.InsertItem( k, CharString().format("%u:%u", k, triangles ) );
			m_Materials.SetItemText( item, 1, Port::relative( pDoc->m_Materials[k] ) );
			m_Materials.SetItemData( item, k );
		}

		m_Triangles.Format( _T("%d"), triangles );
		m_Frames.Format( _T("%d"), pDoc->m_Triangles.size() );

		m_FPS = pDoc->m_Fps;
		m_fMeshScale = pDoc->m_fMeshScale;
		m_Reverse = pDoc->m_Reverse;
		m_Optimize = pDoc->m_Optimize;
		m_CalcNormals = pDoc->m_CalcNormals;
		m_EnableLOD = pDoc->m_EnableLOD;
		m_MinLOD = pDoc->m_MinLOD;
		m_MaxLOD = pDoc->m_MaxLOD;
		m_bExportNodeFrames = pDoc->m_bExportNodeFrames;

		UpdateData( false );
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate( NULL, true, NULL );
}

//----------------------------------------------------------------------------

const int MAX_LEVELS = 64;
const int MIN_LEVELS = 16;

/*
struct DVertex					
{
	int				index;						// index into m_Vertex
	Vector3			position;					// position of this vertex, copied here for convenience
	bool			boundary;					// vertex is on a material boundary so don't collapse
												// if true this vertex will not be collapsed into another
	Array< DVertex * >		
					edges;						// verticies this vertex is connected too
	Array< DTriangle * >	
					triangles;					// triangles that use this vertex

	DVertex *		collapse;					// the best collapse for this vertex
	float			collapseCost;
};

struct DTriangle							
{
	int				index;						// triangle index
	int				material;					// triangle material
	DVertex *		verts[ 3 ];					// triangle verticies
	UV				uv[ 3 ];					// uv's
	Vector3			normal;						// normal for this triangle
};
*/

void CPortView::OnBuildLOD() 
{
	NodeComplexMeshPort * pDoc = dynamic_cast<NodeComplexMeshPort *>( GetDocument() );
	ASSERT( pDoc );

	if ( pDoc->m_Triangles.size() > 1 )
	{
		if ( MessageBox( _T("Building levels of detail will destroy the animation information for this object; are you sure?"), 
			_T("Confirm"), MB_YESNO ) != IDYES )
			return;
	}

	
}

void CPortView::OnEditMaterial() 
{
	NodeComplexMeshPort * pDoc = dynamic_cast<NodeComplexMeshPort *>( GetDocument() );
	ASSERT( pDoc );

	int selected = m_Materials.GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	int mid = m_Materials.GetItemData( selected );

	CSelectResource dialog( CLASS_KEY(MaterialPort) );
	if ( dialog.DoModal() == IDOK)
	{
		pDoc->m_Materials[ mid ] = dialog.m_Selected;
		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
}

void CPortView::OnDCMaterial(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnOpenMaterial();
	*pResult = 0;
}

void CPortView::OnOpenMaterial() 
{
	NodeComplexMeshPort * pDoc = dynamic_cast<NodeComplexMeshPort *>( GetDocument() );
	ASSERT( pDoc );

	int selected = m_Materials.GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	int mid = m_Materials.GetItemData( selected );
	
	AfxGetApp()->OpenDocumentFile( pDoc->m_Materials[mid] );
}

void CPortView::OnRebase() 
{
	NodeComplexMeshPort * pDoc = dynamic_cast<NodeComplexMeshPort *>( GetDocument() );
	ASSERT( pDoc );

	CDirDialog dialog;
	dialog.m_strWindowTitle = _T("Source Directory");
	dialog.m_strTitle = _T("Select directory where materials can be found?");
	dialog.m_strInitDir = CResourcerDoc::document()->rootDirectory();

	if ( dialog.DoBrowse() )
	{
		for(int i=0;i<pDoc->m_Materials.size();i++)
		{
			Path path( pDoc->m_Materials[i] );
			CharString file( path.name() + path.extension() );
			
			CharString newPath( dialog.m_strPath );
			newPath += "\\";
			newPath += file;

			if (! FileDisk::fileExists( newPath ) )
				MessageBox( _T("Failed to find material in directory!"), newPath );
			else
				pDoc->m_Materials[i] = Port::relative( newPath );
		}

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
}

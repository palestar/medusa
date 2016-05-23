// PortView.cpp : implementation file
//

#define NODEMESHPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "Tools/MaterialPort/Port.h"

#include "Tools/ResourcerDoc/Port.h"
#include "Tools/ResourcerDoc/ResourcerDoc.h"
#include "Tools/ResourcerDoc/SelectResource.h"



#pragma warning(disable:4800)

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_TriangleCount = _T("");
	m_VertexCount = _T("");
	m_SnapVerticies = TRUE;
	m_FrameCount = _T("");
	m_MaxX = 1.0f;
	m_MaxY = 1.0f;
	m_MaxZ = 1.0f;
	m_MinX = -1.0f;
	m_MinY = -1.0f;
	m_MinZ = -1.0f;
	m_Radius = 1.0f;
	m_Detail = 3;
	m_MaterialName = _T("");
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Text(pDX, IDC_TRIANGLE_COUNT, m_TriangleCount);
	DDX_Text(pDX, IDC_VERTEX_COUNT, m_VertexCount);
	DDX_Check(pDX, IDC_CHECK2, m_SnapVerticies);
	DDX_Text(pDX, IDC_FRAME_COUNT, m_FrameCount);
	DDX_Text(pDX, IDC_MAXX, m_MaxX);
	DDX_Text(pDX, IDC_MAXY, m_MaxY);
	DDX_Text(pDX, IDC_MAXZ, m_MaxZ);
	DDX_Text(pDX, IDC_MINX, m_MinX);
	DDX_Text(pDX, IDC_MINY, m_MinY);
	DDX_Text(pDX, IDC_MINZ, m_MinZ);
	DDX_Text(pDX, IDC_RADIUS, m_Radius);
	DDX_Text(pDX, IDC_DETAIL, m_Detail);
	DDV_MinMaxInt(pDX, m_Detail, 1, 32);
	DDX_Text(pDX, IDC_EDIT1, m_MaterialName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_BN_CLICKED(IDC_BUTTON1, OnMakeCube)
	ON_BN_CLICKED(IDC_BUTTON2, OnMakeSphere)
	ON_BN_CLICKED(IDC_BUTTON3, OnMaterialBrowse)
	ON_BN_CLICKED(IDC_BUTTON4, OnMaterialProperties)
	ON_BN_CLICKED(IDC_CHECK2, OnUpdateNode)
	ON_BN_CLICKED(IDC_BUTTON5, OnPositionRotation)
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
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	CFormView::OnInitialUpdate();
}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeMeshPort * pPort = dynamic_cast<NodeMeshPort *>( GetDocument() );
	ASSERT( pPort );

	if ( lHint )
	{
		UpdateData( true );

		pPort->m_SnapVerts = m_SnapVerticies;
		pPort->UpdateAllViews(this );
	}
	else
	{
		m_TriangleCount.Format("%d", pPort->m_Triangles.size() );
		m_VertexCount.Format("%d",pPort->m_Verticies.size() );
		m_FrameCount.Format("%d",pPort->m_Frames.size() );

		m_SnapVerticies = pPort->m_SnapVerts;
		m_MaterialName = pPort->m_MaterialPort;

		UpdateData( false );
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate(this,true,NULL);
}

void CPortView::OnMakeCube() 
{
	NodeMeshPort * pPort = dynamic_cast<NodeMeshPort *>( GetDocument() );
	ASSERT( pPort );

	if ( pPort->m_Triangles.size() > 0 
		&& MessageBox( _T("This will destroy existing geometry in mesh; are you sure?"), NULL, MB_OKCANCEL) != IDOK )
			return;

	// get the data from the dialog
	UpdateData( true );

	// build the mesh
	NodeMesh	cube;
	cube.buildCube( Vector3( m_MinX, m_MinY, m_MinZ ), Vector3( m_MaxX, m_MaxY, m_MaxZ ) );

	// release the previous geoemetry, leave animation information alone
	pPort->m_Triangles.release();
	pPort->m_Verticies.release();

	for(int i=0;i<cube.vertexCount();i++)
		pPort->m_Verticies.push( cube.vertex(i) );
	for(int j=0;j<cube.triangleCount();j++)
		pPort->m_Triangles.push( cube.triangle(j) );

	// update this view
	OnUpdate(this,false,NULL);
	// update the view of this object
	pPort->setUpdate();
	pPort->updateParent();
}

void CPortView::OnMakeSphere() 
{
	NodeMeshPort * pPort = dynamic_cast<NodeMeshPort *>( GetDocument() );
	ASSERT( pPort );

	if ( pPort->m_Triangles.size() > 0 
		&& MessageBox(_T("This will destroy existing geometry in mesh; are you sure?"), NULL, MB_OKCANCEL) != IDOK )
			return;

	// get the data from the dialog
	UpdateData( true );

	// build the mesh
	NodeMesh sphere;
	sphere.buildSphere( Vector3( 0.0f ), m_Radius, m_Detail );
		
	// release the previous geoemetry, leave animation information alone
	pPort->m_Triangles.release();
	pPort->m_Verticies.release();

	for(int i=0;i<sphere.vertexCount();i++)
		pPort->m_Verticies.push( sphere.vertex(i) );
	for(int j=0;j<sphere.triangleCount();j++)
		pPort->m_Triangles.push( sphere.triangle(j) );

	// update this view
	OnUpdate(this,false,NULL);
	// update the view of this object
	pPort->setUpdate();
	pPort->updateParent();
}

void CPortView::OnMaterialBrowse() 
{
	NodeMeshPort * pPort = dynamic_cast<NodeMeshPort *>( GetDocument() );
	ASSERT( pPort );

	UpdateData( true );

	CSelectResource dialog( CLASS_KEY( MaterialPort ) );
	if ( dialog.DoModal() == IDOK)
	{
		m_MaterialName = dialog.m_Selected;
		pPort->m_MaterialPort = dialog.m_Selected;

		UpdateData( false );

		pPort->setUpdate();
		pPort->updateParent();	
	}
}

void CPortView::OnMaterialProperties() 
{
	AfxGetApp()->OpenDocumentFile( m_MaterialName );
}


void CPortView::OnPositionRotation() 
{
	NodeMeshPort * pPort = dynamic_cast<NodeMeshPort *>( GetDocument() );
	ASSERT( pPort );

	pPort->createNodeView();
}

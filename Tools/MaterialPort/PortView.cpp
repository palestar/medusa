// PortView.cpp : implementation file
//

#include "stdafx.h"
#include "PortView.h"
#include "Port.h"
#include "Texture.h"

#include "Tools/ResourcerDoc/SelectResource.h"
#include "Tools/ImagePort/Port.h"

#include "Debug/ExceptionHandler.h"
#include "Render3d/NodeSpin.h"
#include "Draw/Image.h"
#include "Draw/Draw.h"
#include ".\portview.h"

#pragma warning(disable : 4800 ) // forcing value to bool 'true' or 'false' (performance warning)

//----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
	, m_sShader(_T(""))
{
	m_bInitTextureList = true;

	//{{AFX_DATA_INIT(CPortView)
	m_Flammable = 0.0f;
	m_Friction = 0.0f;
	m_Hardness = 0.0f;
	m_SpecularPower = 0.0f;
	m_Strength = 0.0f;
	m_Blending = 0;
	m_Fps = 0.0f;
	m_Frames = 0;
	m_Width = 0.0f;
	m_Height = 0.0f;
	m_AmbientLight = TRUE;
	m_DiffuseLight = TRUE;
	m_Alpha = 0;
	m_bSpin = FALSE;
	m_DoubleSided = FALSE;
	m_LightEnable = FALSE;
	m_Stack = -1;
	m_bFixedFunction = FALSE;
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Control(pDX, IDC_PREVIEW, m_Preview);
	DDX_Text(pDX, IDC_FLAMMABLE, m_Flammable);
	DDX_Text(pDX, IDC_FRICTION, m_Friction);
	DDX_Text(pDX, IDC_HARDNESS, m_Hardness);
	DDX_Text(pDX, IDC_SPECULAR_POWER, m_SpecularPower);
	DDX_Text(pDX, IDC_STRENGTH, m_Strength);
	DDX_CBIndex(pDX, IDC_BLENDING, m_Blending);
	DDX_Text(pDX, IDC_EDIT1, m_Fps);
	DDV_MinMaxFloat(pDX, m_Fps, 1.f, 60.f);
	DDX_Text(pDX, IDC_EDIT2, m_Frames);
	DDV_MinMaxInt(pDX, m_Frames, 1, 100);
	DDX_Text(pDX, IDC_EDIT3, m_Width);
	DDV_MinMaxFloat(pDX, m_Width, 0.f, 1024.f);
	DDX_Text(pDX, IDC_EDIT4, m_Height);
	DDV_MinMaxFloat(pDX, m_Height, 0.f, 1024.f);
	DDX_Check(pDX, IDC_CHECK3, m_AmbientLight);
	DDX_Check(pDX, IDC_CHECK2, m_DiffuseLight);
	DDX_Text(pDX, IDC_EDIT5, m_Alpha);
	DDV_MinMaxInt(pDX, m_Alpha, 0, 255);
	DDX_Check(pDX, IDC_CHECK1, m_bFixedFunction );
	DDX_Check(pDX, IDC_CHECK4, m_bSpin);
	DDX_Check(pDX, IDC_CHECK5, m_DoubleSided);
	DDX_Check(pDX, IDC_CHECK6, m_LightEnable);
	DDX_CBIndex(pDX, IDC_COMBO1, m_Stack);
	DDX_Control(pDX, IDC_LIST1, m_cTextureList );
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDIT6, m_sShader);
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_WM_TIMER()
	ON_EN_KILLFOCUS(IDC_FLAMMABLE, OnUpdateMaterial)
	ON_BN_CLICKED(IDC_CHECK2, OnLight)
	ON_BN_CLICKED(IDC_CHECK3, OnAmbient)
	ON_EN_KILLFOCUS(IDC_HARDNESS, OnUpdateMaterial)
	ON_EN_KILLFOCUS(IDC_STRENGTH, OnUpdateMaterial)
	ON_EN_KILLFOCUS(IDC_FRICTION, OnUpdateMaterial)
	ON_EN_KILLFOCUS(IDC_SPECULAR_POWER, OnUpdateMaterial)
	ON_BN_CLICKED(IDC_MIPMAP, OnUpdateMaterial)
	ON_BN_CLICKED(IDC_COLORKEY_ENABLE, OnUpdateMaterial)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateMaterial)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateMaterial)
	ON_CBN_SELCHANGE(IDC_BLENDING, OnUpdateMaterial)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnUpdateMaterial)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnUpdateMaterial)
	ON_EN_KILLFOCUS(IDC_PIXEL_SHADER, OnUpdateMaterial)
	ON_BN_CLICKED(IDC_CHECK4, OnUpdateMaterial)
	ON_BN_CLICKED(IDC_CHECK5, OnUpdateMaterial)
	ON_BN_CLICKED(IDC_CHECK6, OnUpdateMaterial)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnUpdateMaterial)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnAddTexture)
	ON_BN_CLICKED(IDC_BUTTON2, OnRemoveTexture)
	ON_BN_CLICKED(IDC_BUTTON3, OnEditTexture)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDoubleClickTexture)
	ON_BN_CLICKED(IDC_CHECK1, OnFixedFunction)
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

	m_Diffuse.SubclassDlgItem(IDC_DIFFUSE,this); 	
	m_Ambient.SubclassDlgItem(IDC_AMBIENT,this); 	
	m_Emissive.SubclassDlgItem(IDC_EMISSIVE,this); 	
	m_Specular.SubclassDlgItem(IDC_SPECULAR,this); 	

	//----------------------------------------------------------------------------

	CFormView::OnInitialUpdate();

	m_Preview.Initialize();
	
	OnUpdate( NULL, false, NULL );

	SetTimer( 0x1, 20, NULL );
}

#pragma warning( disable: 4805 )		// warning C4805: '!=' : unsafe mix of type 'bool' and type 'int' in operation

#define UPDATE_DOC( VAR, VAL )					\
	if ( pDoc->VAR != VAL ) {					\
		pDoc->VAR = VAL;						\
		pDoc->SetModifiedFlag();	}

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	MaterialPort * pDoc = dynamic_cast<MaterialPort *>(GetDocument());
	ASSERT( pDoc );

	if ( lHint )
	{
		if ( !UpdateData() )	// Get Data from dialog
			return;

		UPDATE_DOC( m_Flammable, m_Flammable );
		UPDATE_DOC( m_Hardness, m_Hardness );
		UPDATE_DOC( m_Strength, m_Strength );
		UPDATE_DOC( m_Friction, m_Friction );
		UPDATE_DOC( m_nPass, m_Stack );
		UPDATE_DOC( m_Width, m_Width );
		UPDATE_DOC( m_Height, m_Height );

		UPDATE_DOC( m_Diffuse, Color( m_Diffuse.currentcolor ) );
		UPDATE_DOC( m_Diffuse.m_A, m_Alpha );
		UPDATE_DOC( m_Ambient, Color( m_Ambient.currentcolor ) );
		UPDATE_DOC( m_Emissive, Color( m_Emissive.currentcolor ) );
		UPDATE_DOC( m_Specular, Color( m_Specular.currentcolor ) );

		UPDATE_DOC( m_SpecularPower, m_SpecularPower );
		UPDATE_DOC( m_LightEnable, m_LightEnable );
		UPDATE_DOC( m_Blending, (Material::Blending)m_Blending );
		UPDATE_DOC( m_DoubleSided, m_DoubleSided );
		UPDATE_DOC( m_Fps, m_Fps );
		UPDATE_DOC( m_Frames, m_Frames );
		UPDATE_DOC( m_sShader, CharString(m_sShader) );

		pDoc->UpdateAllViews( NULL );
	}
	else
	{
		m_Flammable = pDoc->m_Flammable;			// physical properties
		m_Hardness = pDoc->m_Hardness;
		m_Strength = pDoc->m_Strength;
		m_Friction = pDoc->m_Friction;
		m_Stack = pDoc->m_nPass;
		m_Width = pDoc->m_Width;
		m_Height = pDoc->m_Height;

		m_Diffuse.currentcolor = (COLORREF) pDoc->m_Diffuse.RGB();
		m_Alpha = pDoc->m_Diffuse.m_A;
		m_Ambient.currentcolor = (COLORREF) pDoc->m_Ambient.RGB();
		m_Emissive.currentcolor = (COLORREF) pDoc->m_Emissive.RGB();
		m_Specular.currentcolor = (COLORREF) pDoc->m_Specular.RGB();

		m_SpecularPower = pDoc->m_SpecularPower;
		m_LightEnable = pDoc->m_LightEnable;
		m_Blending = pDoc->m_Blending;
		m_DoubleSided = pDoc->m_DoubleSided;
		m_Fps = pDoc->m_Fps;
		m_Frames = pDoc->m_Frames;
		m_sShader = pDoc->m_sShader;

		UpdateData( false );

		UpdateTextureListControl();

		// flush the surface cache, so we can see image changes..
		Material::flushSurfaceCache();
		// rebuild our material of our cube..
		if ( m_Preview.m_Cube )
			m_Preview.m_Cube->setMaterial( WidgetCast<Material>( pDoc->createResource() ) );
		// invalidate our preview so it will get re-drawn
		m_Preview.Invalidate();
	}
}

void CPortView::OnTimer(UINT nIDEvent) 
{
	CFormView::OnTimer(nIDEvent);

	if( m_bSpin )
	{
		NodeTransform * pNode = WidgetCast<NodeTransform>( m_Preview.m_Scene->node() );
		if ( pNode != NULL )
		{
			Matrix33 mFrame( pNode->frame() );
			pNode->setFrame( mFrame.rotate( PI / 64.0f, PI / 64.0f, 0.0f ) );
		}

		m_Preview.Invalidate();
	}
}

//void CPortView::OnDiffuseBrowse() 
//{
//	MaterialPort * pDoc = dynamic_cast<MaterialPort *>(GetDocument());
//	ASSERT( pDoc );
//
//	CSelectResource dialog( CLASS_KEY(ImagePort) );
//	if ( dialog.DoModal() == IDOK)
//	{
//		pDoc->m_DiffusePort = dialog.m_Selected;
//		pDoc->SetModifiedFlag();
//		pDoc->UpdateAllViews( NULL );
//	}
//}
//
//void CPortView::OnDiffuseProperties() 
//{
//	AfxGetApp()->OpenDocumentFile( m_DiffuseName );
//}
//
//void CPortView::OnAlphaBrowse() 
//{
//	MaterialPort * pDoc = dynamic_cast<MaterialPort *>(GetDocument());
//	ASSERT( pDoc );
//
//	// classKey for ImagePort is 3914102874601741960
//	CSelectResource dialog( CLASS_KEY(ImagePort) );
//	if ( dialog.DoModal() == IDOK)
//	{
//		pDoc->m_AlphaPort = dialog.m_Selected;
//		pDoc->SetModifiedFlag();
//		pDoc->UpdateAllViews( NULL );
//	}
//}
//
//void CPortView::OnAlphaProperties() 
//{
//	AfxGetApp()->OpenDocumentFile( m_AlphaName );
//}
//
//void CPortView::OnOpenLightMap() 
//{
//	AfxGetApp()->OpenDocumentFile( m_LightMap );
//}
//
//void CPortView::OnBrowseLightMap() 
//{
//	MaterialPort * pDoc = dynamic_cast<MaterialPort *>(GetDocument());
//	ASSERT( pDoc );
//
//	// classKey for ImagePort is 3914102874601741960
//	CSelectResource dialog( CLASS_KEY(ImagePort) );
//	if ( dialog.DoModal() == IDOK)
//	{
//		pDoc->m_LightMap = dialog.m_Selected;
//		pDoc->SetModifiedFlag();
//		pDoc->UpdateAllViews( NULL );
//	}
//}

//----------------------------------------------------------------------------

void CPortView::OnUpdateMaterial() 
{
	OnUpdate(NULL,true,NULL);
}

void CPortView::OnLight() 
{
	UpdateData();
}

void CPortView::OnAmbient() 
{
	UpdateData();
}

void CPortView::OnDraw(CDC* pDC) 
{
	m_Preview.m_bAmbientLight = m_AmbientLight;
	m_Preview.m_bDiffuseLight = m_DiffuseLight;
}

void CPortView::OnAddTexture()
{
	CTexture dialog;
	if ( dialog.DoModal() == IDOK )
	{
		MaterialPort::Texture texture;
		texture.m_eType = (PrimitiveSurface::Type)dialog.m_nType;
		texture.m_sImagePort = dialog.m_sImagePort;
		texture.m_bMipMap = dialog.m_bMipMap;
		texture.m_nIndex = dialog.m_nIndex;
		texture.m_nUV = dialog.m_nUV;

		const char * pParse = dialog.m_sParameters;
		for(int k=0;k<PrimitiveSurface::MAX_TEXTURE_PARAMS;++k)
		{
			texture.m_fParams[k] = atof( pParse );

			pParse = strchr( pParse, ',' );
			if ( pParse == NULL )
				break;
			++pParse;
		}
		
		MaterialPort * pDoc = dynamic_cast<MaterialPort *>(GetDocument());
		ASSERT( pDoc );
		
		pDoc->m_Textures.push( texture );
		pDoc->SetModifiedFlag();
		pDoc->UpdateAllViews( NULL );
	}
}

void CPortView::OnRemoveTexture()
{
	MaterialPort * pDoc = dynamic_cast<MaterialPort *>(GetDocument());
	ASSERT( pDoc );

	POSITION p = m_cTextureList.GetFirstSelectedItemPosition();
	while( p != NULL )
	{
		int nItem = m_cTextureList.GetNextSelectedItem( p );
		int nTexture = m_cTextureList.GetItemData( nItem);
		
		if ( nTexture < 0 || nTexture >= pDoc->m_Textures.size() )
			continue;
		pDoc->m_Textures[nTexture].m_bRemove = true;
	}
	
	for(int i=0;i<pDoc->m_Textures.size(); )
	{
		if ( pDoc->m_Textures[i].m_bRemove )
			pDoc->m_Textures.remove( i );
		else
			++i;
	}

	pDoc->SetModifiedFlag();
	pDoc->UpdateAllViews( NULL );
}

void CPortView::OnEditTexture()
{
	MaterialPort * pDoc = dynamic_cast<MaterialPort *>(GetDocument());
	ASSERT( pDoc );

	POSITION p = m_cTextureList.GetFirstSelectedItemPosition();
	while( p != NULL )
	{
		int nItem = m_cTextureList.GetNextSelectedItem( p );
		int nTexture = m_cTextureList.GetItemData( nItem );
		if ( nTexture < 0 || nTexture >= pDoc->m_Textures.size() )
			continue;

		MaterialPort::Texture & texture = pDoc->m_Textures[ nTexture ];

		CTexture dialog;
		dialog.m_nType = texture.m_eType;
		dialog.m_sImagePort = texture.m_sImagePort;
		dialog.m_bMipMap = texture.m_bMipMap;
		dialog.m_nIndex = texture.m_nIndex;
		dialog.m_nUV = texture.m_nUV;

		for(int k=0;k<PrimitiveSurface::MAX_TEXTURE_PARAMS;++k)
		{
			if ( dialog.m_sParameters.GetLength() > 0 )
				dialog.m_sParameters += ",";
			dialog.m_sParameters += CharString().format( "%.2f", texture.m_fParams[k] );
		}

		if ( dialog.DoModal() == IDOK )
		{
			texture.m_eType = (PrimitiveSurface::Type)dialog.m_nType;
			texture.m_sImagePort = dialog.m_sImagePort;
			texture.m_bMipMap = dialog.m_bMipMap;
			texture.m_nIndex = dialog.m_nIndex;
			texture.m_nUV = dialog.m_nUV;

			const char * pParse = dialog.m_sParameters;
			for(int k=0;k<PrimitiveSurface::MAX_TEXTURE_PARAMS;++k)
			{
				texture.m_fParams[k] = atof( pParse );

				pParse = strchr( pParse, ',' );
				if ( pParse == NULL )
					break;
				++pParse;
			}
		}
	}
	
	pDoc->SetModifiedFlag();
	pDoc->UpdateAllViews( NULL );
}

void CPortView::OnDoubleClickTexture(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	OnEditTexture();
}

void CPortView::UpdateTextureListControl()
{
	MaterialPort * pDoc = dynamic_cast<MaterialPort *>(GetDocument());
	ASSERT( pDoc );

	m_cTextureList.DeleteAllItems();
	if ( m_bInitTextureList )
	{
		CRect r;
		m_cTextureList.GetClientRect( &r );

		int cWidth = r.Width() / 4;
		m_cTextureList.InsertColumn( 0, "Type", LVCFMT_LEFT, cWidth, 0 );	
		m_cTextureList.InsertColumn( 1, "Image", LVCFMT_LEFT, cWidth * 2, 1 );	
		m_cTextureList.InsertColumn( 2, "Index", LVCFMT_LEFT, cWidth, 2 );	
		m_cTextureList.InsertColumn( 3, "UV", LVCFMT_LEFT, cWidth, 3 );	
		m_cTextureList.InsertColumn( 4, "Parameters", LVCFMT_LEFT, cWidth, 4 );	
		m_cTextureList.SetExtendedStyle( LVS_EX_FULLROWSELECT );
		m_bInitTextureList = false;
	}

	for(int i=0;i<pDoc->m_Textures.size();++i)
	{
		MaterialPort::Texture & texture = pDoc->m_Textures[i];
		
		int nItem = m_cTextureList.InsertItem( 0, PrimitiveSurface::typeText( texture.m_eType ) );
		m_cTextureList.SetItemText( nItem, 1, texture.m_sImagePort );
		m_cTextureList.SetItemText( nItem, 2, CharString().format("%d", texture.m_nIndex ) );
		m_cTextureList.SetItemText( nItem, 3, CharString().format("%d", texture.m_nUV ) );
		m_cTextureList.SetItemData( nItem, i );

		CharString sParameters;
		for(int k=0;k<PrimitiveSurface::MAX_TEXTURE_PARAMS;++k)
		{
			if ( sParameters.length() > 0 )
				sParameters += ",";
			sParameters += CharString().format( "%.2f", texture.m_fParams[k] );
		}
		m_cTextureList.SetItemText( nItem, 4, sParameters );
	}
}

//---------------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPortView::CPreview, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

CPortView::CPreview::CPreview() : m_Time( 0.0f ), 
	m_bAmbientLight( true ), 
	m_bDiffuseLight( true ),
	m_bRotatingObject( false ),
	m_bRotatingLight( false )
{
	m_vLightDirection = Vector3(1,-1,1).normalize();
}

void CPortView::CPreview::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	DisplayDevice::Ref pDisplay = m_Context.display();
	if (! pDisplay )
		return;
	
	pDisplay->lock();
	pDisplay->clearLights();
	m_Context.setProjection( RectInt().setInvalid(), PI / 4, 1.0f, 32768.0f );

	if ( m_bAmbientLight )
		pDisplay->setAmbient( Color(128,128,128 ) );
	else
		pDisplay->setAmbient( Color(0,0,0) );

	// push a single directional light
	if ( m_bDiffuseLight )
		m_Context.display()->addDirectionalLight( 10000, Color(255,255,255), m_vLightDirection );

	// render the cube
	m_Context.setTime( m_Time );

	m_Context.beginRender( Color(0,0,64), m_bAmbientLight ? Color(32,32,32) : Color(0,0,0) );
	m_Context.render( m_Scene );
	m_Context.endRender();

	m_Time += 1.0f / m_Context.fps();

	pDisplay->unlock();
}

bool CPortView::CPreview::Initialize()
{
	m_Cube = new NodeMesh;
	m_Cube->buildCube( Vector3(-1,-1,-1), Vector3(1,1,1) );
	//m_Cube->buildSphere( Vector3( 0.0f ), 1.0f, 3 ); 

	// make sure the shaders path is set correctly..
	DisplayDevice::sm_sShadersPath = FileDisk::home() + "\\";
	// turn on shader debugging as well..
	DisplayDevice::sm_bEnableShaderDebug = true;

	// create the display device
	DisplayDevice::Ref pDisplay = DisplayDevice::create( "DisplayDeviceD3D" );
	if (! pDisplay->initialize( m_hWnd, NULL, true, true ) )
	{
		MessageBox( _T("Failed to initialize display device!") );
		m_Context.setDisplay( NULL );
	}
	else
	{
		m_Context.setDisplay( pDisplay );
		pDisplay->setFillMode( DisplayDevice::FILL_SOLID );
	}

	m_Cube->setPosition( Vector3( 0.0f, 0.0f, 4.0f ) );

	// setup the scene
	m_Scene = new Scene;
	m_Scene->setNode( m_Cube );

	return true;
}

void CPortView::CPreview::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bRotatingObject = true;
	m_ptLastMousePos = point;
	SetCapture();

	CStatic::OnLButtonDown(nFlags, point);
}

void CPortView::CPreview::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bRotatingObject = false;
	if (! m_bRotatingObject && !m_bRotatingLight )
		ReleaseCapture();

	CStatic::OnLButtonUp(nFlags, point);
}

void CPortView::CPreview::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_bRotatingLight = true;
	m_ptLastMousePos = point;
	SetCapture();

	CStatic::OnRButtonDown(nFlags, point);
}

void CPortView::CPreview::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bRotatingLight = false;
	if (! m_bRotatingObject && !m_bRotatingLight )
		ReleaseCapture();

	CStatic::OnRButtonUp(nFlags, point);
}

const float PITCH_RATE = PI / 128.0f;
const float YAW_RATE = PI / 128.0f;

void CPortView::CPreview::OnMouseMove(UINT nFlags, CPoint point)
{
	CStatic::OnMouseMove(nFlags, point);

	if ( m_Scene && (m_bRotatingLight || m_bRotatingObject) )
	{
		int nDX = m_ptLastMousePos.x - point.x;
		int nDY = m_ptLastMousePos.y - point.y;
		m_ptLastMousePos = point;

		if ( m_bRotatingObject )
		{
			NodeTransform * pNode = WidgetCast<NodeTransform>( m_Scene->node() );
			if ( pNode != NULL )
			{
				Matrix33 mFrame( pNode->frame() );
				pNode->setFrame( mFrame.rotate( nDY * PITCH_RATE, nDX * YAW_RATE, 0.0f ) );
			}
		}

		if ( m_bRotatingLight )
		{
			Matrix33 mLightFrame( m_vLightDirection );
			mLightFrame.rotate( nDY * PITCH_RATE, nDX * YAW_RATE, 0.0f );
			m_vLightDirection = mLightFrame.k;
		}

		// invalidate this window, forcing it to be redrawn..
		Invalidate();
	}
}

void CPortView::OnFixedFunction()
{
	m_bFixedFunction = !m_bFixedFunction;
	DisplayDevice::sm_bUseFixedFunction = m_bFixedFunction != 0;
}

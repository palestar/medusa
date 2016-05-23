// SceneRender.cpp : implementation file
//

#pragma warning( disable: 4786 )	// identifier was truncated to '255' characters in the browser information

#include "stdafx.h"
#include "SceneRender.h"
#include "Port.h" 
#include "ChildFrame.h"

#include "Audio/Sound.h"
#include "Debug/ExceptionHandler.h"
#include "File/FileDisk.h"
#include "Render3d/NodeSound.h"
#include "Resource.h"

#include "Math/Helpers.h"
//#include "Gui3d/NodeInterfaceClient.h"
//#include "Render3d/NodeBSP.h"



/////////////////////////////////////////////////////////////////////////////
// CSceneRender

IMPLEMENT_DYNCREATE(CSceneRender, CView)

CSceneRender::CSceneRender()
{
	m_Mode = DOLLY;
	m_Projection = PERSPECTIVE;
	m_FillMode = DisplayDevice::FILL_SOLID;
	m_Playing = false;
	m_Light = false;
	m_bSoundsEnabled = true;
	m_LeftDown = false;

	m_Time = 0.0f;
	//m_Bits = 0xffffffff;
}

CSceneRender::~CSceneRender()
{
}


BEGIN_MESSAGE_MAP(CSceneRender, CView)
	//{{AFX_MSG_MAP(CSceneRender)
	ON_COMMAND(ID_CAMERA_PERSPECTIVE, OnCameraPerspective)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_PERSPECTIVE, OnUpdateCameraPerspective)
	ON_COMMAND(ID_CAMERA_ORTHOGONAL, OnCameraOrthogonal)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_ORTHOGONAL, OnUpdateCameraOrthogonal)
	ON_COMMAND(ID_CAMERA_XY, OnCameraXy)
	ON_COMMAND(ID_CAMERA_YZ, OnCameraYz)
	ON_COMMAND(ID_CAMERA_ZX, OnCameraZx)
	ON_COMMAND(ID_CAMERA_ISOMETRIC, OnCameraIsometric)
	ON_COMMAND(ID_CAMERA_SOLID, OnCameraSolid)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_SOLID, OnUpdateCameraSolid)
	ON_COMMAND(ID_CAMERA_WIREFRAME, OnCameraWireframe)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_WIREFRAME, OnUpdateCameraWireframe)
	ON_COMMAND(ID_CAMERA_POINT, OnCameraPoint)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_POINT, OnUpdateCameraPoint)
	ON_COMMAND(ID_SCENE_PLAY, OnScenePlay)
	ON_UPDATE_COMMAND_UI(ID_SCENE_PLAY, OnUpdateScenePlay)
	ON_WM_TIMER()
	ON_COMMAND(ID_CAMERA_ZOOM, OnCameraZoom)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_ZOOM, OnUpdateCameraZoom)
	ON_COMMAND(ID_CAMERA_DOLLY, OnCameraDolly)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_DOLLY, OnUpdateCameraDolly)
	ON_COMMAND(ID_CAMERA_ZOOMEXTENTS, OnCameraZoomextents)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_CAMERA_PAN, OnCameraPan)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_PAN, OnUpdateCameraPan)
	ON_WM_DESTROY()
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_COMMAND(ID_NODE_SELECT, OnNodeSelect)
	ON_COMMAND(ID_NODE_MOVEXY, OnNodeMovexy)
	ON_UPDATE_COMMAND_UI(ID_NODE_MOVEXY, OnUpdateNodeMovexy)
	ON_COMMAND(ID_NODE_MOVEZ, OnNodeMovez)
	ON_UPDATE_COMMAND_UI(ID_NODE_MOVEZ, OnUpdateNodeMovez)
	ON_COMMAND(ID_NODE_YAW, OnNodeYaw)
	ON_UPDATE_COMMAND_UI(ID_NODE_YAW, OnUpdateNodeYaw)
	ON_COMMAND(ID_NODE_PITCH, OnNodePitch)
	ON_UPDATE_COMMAND_UI(ID_NODE_PITCH, OnUpdateNodePitch)
	ON_COMMAND(ID_NODE_ROLL, OnNodeRoll)
	ON_UPDATE_COMMAND_UI(ID_NODE_ROLL, OnUpdateNodeRoll)
	ON_UPDATE_COMMAND_UI(ID_NODE_SELECT, OnUpdateNodeSelect)
	ON_COMMAND(ID_NODE_RESETFRAME, OnNodeResetframe)
	ON_UPDATE_COMMAND_UI(ID_NODE_RESETFRAME, OnUpdateNodeResetframe)
	ON_COMMAND(ID_NODE_SCALE, OnNodeScale)
	ON_UPDATE_COMMAND_UI(ID_NODE_SCALE, OnUpdateNodeScale)
	ON_COMMAND(ID_NODE_RESETPOSITION, OnNodeResetposition)
	ON_UPDATE_COMMAND_UI(ID_NODE_RESETPOSITION, OnUpdateNodeResetposition)
	ON_COMMAND(ID_SCENE_NEXT, OnSceneNext)
	ON_COMMAND(ID_SCENE_REWIND, OnSceneRewind)
	ON_COMMAND(ID_SCENE_LIGHT, OnSceneLight)
	ON_UPDATE_COMMAND_UI(ID_SCENE_LIGHT, OnUpdateSceneLight)
	ON_COMMAND(ID_SCENE_PREVIOUS, OnScenePrevious)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_RENDER_SM, &CSceneRender::OnRenderSm)
	ON_UPDATE_COMMAND_UI(ID_RENDER_SM, &CSceneRender::OnUpdateRenderSm)
	ON_UPDATE_COMMAND_UI(ID_RENDER_SOUNDENABLED, &CSceneRender::OnUpdateRenderSoundenabled)
	ON_COMMAND(ID_RENDER_SOUNDENABLED, &CSceneRender::OnRenderSoundenabled)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSceneRender drawing


void CSceneRender::OnDraw(CDC* pDC)
{
	DisplayDevice::Ref pDisplay = m_Context.display();
	if (! pDisplay )
		return;

	pDisplay->lock();

	try {
		ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
		ASSERT( pDoc );

		// update all modifed nodes before we render
		pDoc->m_Root->updateNodeCache();

		// update the camera position and frame
		m_Context.setPosition( m_TargetPosition - (m_TargetFrame.k * m_TargetDistance) );
		m_Context.setFrame( m_TargetFrame );

		// set the fill mode
		m_Context.display()->setFillMode( m_FillMode );

		// display the selected node
		ASSERT( pDoc->m_pSelectedNode );
		BaseNode::s_pSelectedNode = dynamic_cast< BaseNode * >( pDoc->m_pSelectedNode->getCachedNode() );

		if ( pDoc->m_CurrentSegment >= 0 )
		{
			Segment & segment = pDoc->m_Segments[ pDoc->m_CurrentSegment ];
			if ( m_Time > segment.endTime )
				m_Time = segment.beginTime;
			else if ( m_Time < segment.beginTime )
				m_Time = segment.beginTime;
		}

		RectInt window( m_Context.display()->clientWindow() );
		window -= window.upperLeft();

		m_Context.setTime( m_Time );
		m_Context.setBits( pDoc->m_Bits );
		m_Context.setProjection( window, PI / 4, 1.0f, 32768.0f );

		// render the scene
		m_Context.beginRender( Color(0,0,0), Color(0,0,0) );
		m_Context.beginScene();

		// create a directional light
		if ( m_Light )
			m_Context.display()->addDirectionalLight( 1000, Color(255,255,255), m_TargetFrame.k );

		m_Context.render( m_Scene );
		m_Context.endScene();

		m_Context.endRender();

		// Update the status bar
		CString nodeStatus;
		if ( dynamic_cast< NodeTransform *>( BaseNode::s_pSelectedNode ) )
		{
			NodeTransform * pNode = (NodeTransform *)BaseNode::s_pSelectedNode;
			nodeStatus.Format("%s( %.2f : %.2f : %.2f )",pNode->name(),
				pNode->position().x,
				pNode->position().y,
				pNode->position().z );
		}
		
		CString status;
		status.Format("Camera( %.2f : %.2f : %.2f ) %s Time = %.2f, Frame = %d", 
			m_Context.position().x,
			m_Context.position().y, 
			m_Context.position().z, 
			nodeStatus, m_Time, int(m_Time * pDoc->m_Fps) );
		
		CChildFrame * pParent = (CChildFrame *)GetParentFrame();
		if ( pParent != NULL )
			pParent->m_StatusBar.SetPaneText( 0, status );

		// animate the scene
		if ( m_Playing )
			m_Time += 1.0f / m_Context.fps();
	}
	catch( DisplayDevice::PrimitiveFailure )
	{
		TRACE( "Caught DisplayDevice::PrimitiveFailure()" );
	}

	pDisplay->unlock();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneRender diagnostics

#ifdef _DEBUG
void CSceneRender::AssertValid() const
{
	CView::AssertValid();
}

void CSceneRender::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSceneRender message handlers

void CSceneRender::OnInitialUpdate() 
{
	ScenePort * pPort = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pPort );

	m_bSoundsEnabled = AfxGetApp()->GetProfileInt( "CSceneRender", "m_bSoundsEnabled", 1 ) != 0;
	m_Light = AfxGetApp()->GetProfileInt( "CSceneRender", "m_Light", 0 ) != 0;

	DisplayDevice::sm_sShadersPath = FileDisk::home() + "\\";
	NodeTransform::s_ShowSelected = true;
	//NodeTransform::s_bDrawNodeAxis = true;

	CView::OnInitialUpdate();

	m_Context.setDisplay( DisplayDevice::create( "DisplayDeviceD3D" ) );
	if ( !m_Context.display() || !m_Context.display()->initialize( m_hWnd, NULL, true, true ) )
	{
		MessageBox( _T("Failed to initialize display device!") );
		m_Context.setDisplay( NULL );
	}

	m_Context.setAudio( AudioDevice::create( "AudioDeviceDS" ) );
	if ( !m_Context.audio() || !m_Context.audio()->initialize( m_hWnd ) )
	{
		MessageBox( _T("Failed to initialize audio device!") );
		m_Context.setAudio( NULL );
	}
	else
	{
		// turn volume up/down based on sounds being enabled or not
		m_Context.audio()->setVolume( m_bSoundsEnabled ? 1.0f : 0.0f );
	}
}

void CSceneRender::OnDestroy() 
{
	Sound::releaseAudioBuffers();
	m_Context.flushInstanceData();

	CView::OnDestroy();
}

void CSceneRender::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	ScenePort * pPort = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pPort );

	if ( lHint )
	{
		pPort->m_TargetPosition = m_TargetPosition;
		pPort->m_TargetDistance = m_TargetDistance;
		pPort->m_TargetFrame = m_TargetFrame;
	}
	else
	{
		m_TargetPosition = pPort->m_TargetPosition;
		m_TargetDistance = pPort->m_TargetDistance;
		m_TargetFrame = pPort->m_TargetFrame;

		// build the scene hierarchy
		m_Scene = WidgetCast<Scene>( pPort->createResource() );

		// redraw the scene
		OnDraw(NULL);
	}
}

void CSceneRender::OnViewRefresh() 
{
	ScenePort * pPort = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pPort );

	pPort->m_Root->updateNodeCache( true );
	OnUpdate(NULL,0,NULL);
}

void CSceneRender::OnCameraPerspective() 
{
	m_Projection = PERSPECTIVE;
}

void CSceneRender::OnUpdateCameraPerspective(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Projection == PERSPECTIVE );
}

void CSceneRender::OnCameraOrthogonal() 
{
	m_Projection = ORTHOGONAL;
}

void CSceneRender::OnUpdateCameraOrthogonal(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Projection == ORTHOGONAL );
}

void CSceneRender::OnCameraXy() 
{
	m_TargetFrame.build( Vector3(0,0,1) );
	OnUpdate(this,true,NULL);
	OnDraw(NULL);
}

void CSceneRender::OnCameraYz() 
{
	m_TargetFrame.build( Vector3(1,0,0) );
	OnUpdate(this,true,NULL);
	OnDraw(NULL);
}

void CSceneRender::OnCameraZx() 
{
	m_TargetFrame.build( Vector3(0,-1,0) );
	OnUpdate(this,true,NULL);
	OnDraw(NULL);
}

void CSceneRender::OnCameraIsometric() 
{
	m_TargetFrame.build( Vector3(1,-1,1).normalize() );
	OnUpdate(this,true,NULL);
	OnDraw(NULL);
}

void CSceneRender::OnCameraSolid() 
{
	m_FillMode = DisplayDevice::FILL_SOLID;
	OnUpdate(NULL,0,NULL);
}

void CSceneRender::OnUpdateCameraSolid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_FillMode == DisplayDevice::FILL_SOLID );
}

void CSceneRender::OnCameraWireframe() 
{
	m_FillMode = DisplayDevice::FILL_WIREFRAME;
	OnUpdate(NULL,0,NULL);
}

void CSceneRender::OnUpdateCameraWireframe(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_FillMode == DisplayDevice::FILL_WIREFRAME );
}

void CSceneRender::OnCameraPoint() 
{
	m_FillMode = DisplayDevice::FILL_POINT;
	OnUpdate(NULL,0,NULL);
}

void CSceneRender::OnUpdateCameraPoint(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_FillMode == DisplayDevice::FILL_POINT );
}

void CSceneRender::OnScenePlay() 
{
	m_Playing = !m_Playing;
	if ( m_Playing )
		SetTimer( 0x1, 10, NULL );
	else
		KillTimer( 0x1 );
}

void CSceneRender::OnUpdateScenePlay(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Playing );
}

void CSceneRender::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 0x1 )
		OnDraw(NULL);

	CView::OnTimer(nIDEvent);
}

void CSceneRender::OnCameraZoom() 
{
	m_Mode = ZOOM;
}

void CSceneRender::OnUpdateCameraZoom(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Mode == ZOOM );
}

void CSceneRender::OnCameraDolly() 
{
	m_Mode = DOLLY;
}

void CSceneRender::OnUpdateCameraDolly(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Mode == DOLLY );
}

void CSceneRender::OnCameraZoomextents() 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	m_TargetPosition = Vector3(0,0,0);
	m_TargetDistance = 50.0f;

	if ( pDoc->m_pSelectedNode )
	{
		// find the world position of the selected node
		BaseNode * pBaseNode = pDoc->m_pSelectedNode->getCachedNode();
		if ( pBaseNode )
		{
			// get the hull of the selected node
			BoxHull hull( pBaseNode->hull() );

			Vector3 position( 0.0f );
			while( pBaseNode )
			{
				if ( dynamic_cast< NodeTransform * >( pBaseNode ) )
				{
					NodeTransform * pNode = (NodeTransform *)pBaseNode;
					position = (~pNode->frame() * position) + pNode->position();
				}
				pBaseNode = pBaseNode->parent();
			}

			m_TargetPosition = position;
			m_TargetDistance = hull.radius() * 2.0f;
		}
	}

	// redraw the scene
	OnUpdate(this,true,NULL);
	OnDraw(NULL);
}

void CSceneRender::OnCameraPan() 
{
	m_Mode = ROTATE;
}

void CSceneRender::OnUpdateCameraPan(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Mode == ROTATE );
}

void CSceneRender::OnNodeSelect() 
{
	m_Mode = NODE_SELECT;
}

void CSceneRender::OnUpdateNodeSelect(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Mode == NODE_SELECT );
}

void CSceneRender::OnNodeMovexy() 
{
	m_Mode = NODE_MOVEXY;
}

void CSceneRender::OnUpdateNodeMovexy(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Mode == NODE_MOVEXY );

	BaseNodePort * pNode = ((ScenePort *)GetDocument())->m_pSelectedNode;
	pCmdUI->Enable( pNode ? pNode->canMove() : false );
}

void CSceneRender::OnNodeMovez() 
{
	m_Mode = NODE_MOVEZ;
}

void CSceneRender::OnUpdateNodeMovez(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Mode == NODE_MOVEZ );

	BaseNodePort * pNode = ((ScenePort *)GetDocument())->m_pSelectedNode;
	pCmdUI->Enable( pNode ? pNode->canMove() : false );
}


void CSceneRender::OnNodeScale() 
{
	m_Mode = NODE_SCALE;
}

void CSceneRender::OnUpdateNodeScale(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Mode == NODE_SCALE );

	BaseNodePort * pNode = ((ScenePort *)GetDocument())->m_pSelectedNode;
	pCmdUI->Enable( pNode ? pNode->canScale() : false );
}

void CSceneRender::OnNodeYaw() 
{
	m_Mode = NODE_YAW;
}

void CSceneRender::OnUpdateNodeYaw(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Mode == NODE_YAW );
	BaseNodePort * pNode = ((ScenePort *)GetDocument())->m_pSelectedNode;
	pCmdUI->Enable( pNode ? pNode->canYaw() : false );
}

void CSceneRender::OnNodePitch() 
{
	m_Mode = NODE_PITCH;
}

void CSceneRender::OnUpdateNodePitch(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Mode == NODE_PITCH );
	BaseNodePort * pNode = ((ScenePort *)GetDocument())->m_pSelectedNode;
	pCmdUI->Enable( pNode ? pNode->canPitch() : false );
}

void CSceneRender::OnNodeRoll() 
{
	m_Mode = NODE_ROLL;
}

void CSceneRender::OnUpdateNodeRoll(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Mode == NODE_ROLL );
	BaseNodePort * pNode = ((ScenePort *)GetDocument())->m_pSelectedNode;
	pCmdUI->Enable( pNode ? pNode->canRoll() : false );
}

void CSceneRender::OnNodeResetframe() 
{
	BaseNodePort * pNode = ((ScenePort *)GetDocument())->m_pSelectedNode;
	pNode->resetFrame();

	OnDraw( NULL );
}

void CSceneRender::OnUpdateNodeResetframe(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ((ScenePort *)GetDocument())->m_pSelectedNode != NULL );
}

void CSceneRender::OnNodeResetposition() 
{
	BaseNodePort * pNode = ((ScenePort *)GetDocument())->m_pSelectedNode;
	pNode->resetPosition( m_TargetPosition );

	OnDraw( NULL );
}

void CSceneRender::OnUpdateNodeResetposition(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ((ScenePort *)GetDocument())->m_pSelectedNode != NULL );
}

//----------------------------------------------------------------------------

#define DOLLY_RATE	(300.0f)
#define YAW_RATE	(PI / 256.0f)
#define PITCH_RATE	(PI / 512.0f)

void CSceneRender::OnMouseMove(UINT nFlags, CPoint point) 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( m_LeftDown )
	{
		// get the mouse motion
		CPoint delta = point - m_LeftPoint;
		m_LeftPoint = point;

		if ( m_Mode > NODE_SELECT && m_Mode <= NODE_ROLL )
		{
			BaseNodePort * pNodePort = pDoc->m_pSelectedNode;
			if ( pNodePort == NULL )
				return;

			switch( m_Mode )
			{
				case NODE_YAW:
					pNodePort->yaw( -DegToRad( delta.x ) );
					m_bUpdateNode = true;
					break;
				case NODE_PITCH:
					pNodePort->pitch( -DegToRad( delta.y ) );
					m_bUpdateNode = true;
					break;
				case NODE_ROLL:
					pNodePort->roll( -DegToRad( delta.x ) );
					m_bUpdateNode = true;
					break;
				case NODE_MOVEZ:
					pNodePort->moveXY( delta.x, delta.y );
					pNodePort->move( -(m_TargetFrame.k * delta.y) );
					m_bUpdateNode = true;
					break;
				case NODE_MOVEXY:
					{
						pNodePort->moveXY( delta.x, delta.y );

						float scrollRate = m_TargetDistance / DOLLY_RATE;
						pNodePort->move( (m_TargetFrame.i * (scrollRate * delta.x)) - 
							(m_TargetFrame.j * (scrollRate * delta.y)) );
						m_bUpdateNode = true;
					}
					break;
				case NODE_SCALE:
					pNodePort->scale( delta.x, delta.y );
					m_bUpdateNode = true;
					break;
			}
			OnDraw( NULL );

			return;
		}


		switch( m_Mode )
		{
			case ZOOM:
				{
					float move = Max( m_TargetDistance / 64.0f, 0.001f );
					m_TargetDistance += (move * delta.y);
					OnDraw(NULL);
				}
				break;
			case DOLLY:
				{
					Vector3 xAxis( -m_TargetFrame.i );
					Vector3 yAxis( m_TargetFrame.j );
					
					float scrollRate = m_TargetDistance / DOLLY_RATE;
					m_TargetPosition += (xAxis * (scrollRate * delta.x)) + 
						(yAxis * (scrollRate * delta.y));

					OnDraw(NULL);
				}
				break;
			case ROTATE:
				{
					m_TargetFrame.rotate( DegToRad( delta.y ), DegToRad( delta.x ), 0 );
					OnDraw(NULL);
				}
				break;
		}
	}

	CView::OnMouseMove(nFlags, point);
}

void CSceneRender::OnLButtonDown(UINT nFlags, CPoint point) 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( m_Mode > NODE_SELECT && m_Mode <= NODE_ROLL && pDoc->m_pSelectedNode == NULL )
	{
		MessageBeep( MB_ICONASTERISK );
		return;
	}

	switch( m_Mode )
	{
		case NODE_SELECT:
			/*
			{
				Vector3 ray( point.x, point.y, 1024.0f );
				m_Context.unProject( ray );
				ray.normalize();

				BaseNode * intersectNode = m_Scene->intersect( Vector3(0.0f), ray, Vector3() );
				if ( intersectNode != NULL )
				{
					ASSERT( pDoc->m_Root.valid() );

					// find the object selected in the area
					pDoc->m_pSelectedNode = pDoc->m_Root->findNode( intersectNode->key() );
					pDoc->UpdateAllViews( this );

					OnDraw(NULL);
				}
			}
			*/
			break;
		default:
			m_LeftDown = true;
			m_LeftPoint = point;
			m_bUpdateNode = false;
			SetCapture();
			break;
	}

	CView::OnLButtonDown(nFlags, point);
}

void CSceneRender::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	if ( m_bUpdateNode && pDoc->m_pSelectedNode != NULL )
		pDoc->m_pSelectedNode->updateAllViews();

	m_bUpdateNode = false;
	m_LeftDown = false;
	ReleaseCapture();

	OnUpdate(this,true,NULL);

	CView::OnLButtonUp(nFlags, point);
}

//----------------------------------------------------------------------------

void CSceneRender::OnScenePrevious() 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	m_Time -= 1.0f / pDoc->m_Fps;
	OnDraw(NULL);
}

void CSceneRender::OnSceneNext() 
{
	ScenePort * pDoc = dynamic_cast<ScenePort *>( GetDocument() );
	ASSERT( pDoc );

	m_Time += 1.0f / pDoc->m_Fps;
	OnDraw(NULL);
}

void CSceneRender::OnSceneRewind() 
{
	m_Time = 0.0f;
	OnDraw(NULL);
}

void CSceneRender::OnSceneLight() 
{
	m_Light = !m_Light;
	AfxGetApp()->WriteProfileInt( "CSceneRender", "m_Light", m_Light );

	OnDraw(NULL);
}

void CSceneRender::OnUpdateSceneLight(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_Light );
}

void CSceneRender::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	OnDraw(NULL);
}

void CSceneRender::OnRenderSm()
{
	DisplayDevice::sm_bUseFixedFunction = !DisplayDevice::sm_bUseFixedFunction;

}

void CSceneRender::OnUpdateRenderSm(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( DisplayDevice::sm_bUseFixedFunction );
}

void CSceneRender::OnUpdateRenderSoundenabled(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( m_bSoundsEnabled );
}

void CSceneRender::OnRenderSoundenabled()
{
	m_bSoundsEnabled = !m_bSoundsEnabled;
	AfxGetApp()->WriteProfileInt( "CSceneRender", "m_bSoundsEnabled", m_bSoundsEnabled );

	if ( m_Context.audio() != NULL )
		m_Context.audio()->setVolume( m_bSoundsEnabled ? 1.0f : 0.0f );
}

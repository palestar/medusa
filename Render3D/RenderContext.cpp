/*
	RenderContext.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL

#pragma warning( disable:4786 ) // identifier was truncated to '255' characters in the browser information

#include "Debug/Assert.h"
#include "Standard/Bits.h"
#include "Standard/Time.h"
#include "Display/Types.h"
#include "Display/PrimitiveLineStrip.h"
#include "Display/PrimitiveSetTransform.h"
#include "Render3D/Material.h" 
#include "Render3D/RenderContext.h"
#include "Render3D/Scene.h"
#include "Render3D/NodeSound.h"

#pragma warning( disable : 4244 ) // d:\projects\hydra\render3d\viewfulstrum.cpp(28) : warning C4244: 'initializing' : conversion from 'double' to 'float', possible loss of data

IMPLEMENT_NAMED_ABSTRACT_FACTORY( FACTORY_RenderContextInstanceData, RenderContext::InstanceData, Widget, true );

//-------------------------------------------------------------------------------

float	RenderContext::sm_fDefaultDetail = 0.5f;				// default detail level, should be initialized by client options..
int		RenderContext::sm_nShadowMapSize = 2048;				// default shadow map size
float	RenderContext::sm_fShadowMapRadius = 5000.0f;			// default shadow map radius

//---------------------------------------------------------------------------------------------------

RenderContext::RenderContext() 
{}

RenderContext::RenderContext( DisplayDevice * pDevice, AudioDevice * pAudioDevice,
								float time, dword bits, const Matrix33 &frame, const Vector3 &position,
								const RectInt & window, float fov, float front, float back )
{
	m_Display = pDevice;
	m_Audio = pAudioDevice;
	m_State.m_Time = time;
	m_State.m_Bits = bits;
	m_State.m_nInstanceKey = 0;
	m_State.m_Offset.set( 0, 0 );
	m_State.m_Frame = frame;
	m_State.m_Position = position;

	m_State.m_Clock = 0;
	m_State.m_Fps = 30.0f;
	m_State.m_Elapsed = 1.0f / 30.0f;

	setProjection( window, fov, front, back );
}

//-------------------------------------------------------------------------------

// this function returns true if any part of the AABB is on the negative side of the plane.
static bool boxTest( const Plane & plane, const BoxHull & hull )
{
	float fDist = ((plane.normal.x < 0.0f) ? hull.min.x : hull.max.x) * plane.normal.x +
		((plane.normal.y < 0.0f) ? hull.min.y : hull.max.y) * plane.normal.y +
		((plane.normal.z < 0.0f) ? hull.min.z : hull.max.z) * plane.normal.z + plane.distance;

	return fDist >= 0.0f;
}

bool RenderContext::boxVisible( const BoxHull & hull ) const
{
	for(int i=0;i<FULSTRUM_COUNT;i++)
		if (! boxTest( m_State.m_Fulstrum[i], hull ) )
			return false;

	return true;
}

bool RenderContext::boxVisible( const BoxHull &hull, const Matrix33 &rotate, const Vector3 &translate ) const
{
	return boxVisible( BoxHull( hull, rotate, translate ) );
}


bool RenderContext::sphereVisible( const Vector3 &center,				// center in view space
							float radius ) const
{
	for(int p=FRONT;p<FULSTRUM_COUNT;p++)
		if (m_State.m_Fulstrum[p].dot(center) < -radius)
			return false;

	return true;
}

bool RenderContext::clip( Vector3 & clip )
{
	bool clipped = false;

	float midZ = (m_State.m_Fulstrum[FRONT].distance + m_State.m_Fulstrum[BACK].distance) * 0.5f;

	for(int p=FRONT;p<FULSTRUM_COUNT;p++)
		if (m_State.m_Fulstrum[p].dot( clip ) < 0)
		{
			if ( p != BACK )
			{
				Vector3 origin( 0.0f, 0.0f, midZ );
				m_State.m_Fulstrum[p].intersect( origin, clip - origin, clip );
			}
			else
			{
				Vector3 origin( 0.0f, 0.0f, 0.0f );
				m_State.m_Fulstrum[p].intersect( origin, clip - origin, clip );
			}

			clipped = true;
		}

	return clipped;
}

void RenderContext::project( Vector3 & threeD )
{
	if ( threeD.z > 0.0f )
	{
		threeD.x = (( threeD.x / threeD.z ) * m_State.m_D) + m_State.m_Center.x;
		threeD.y = (( -threeD.y / threeD.z ) * m_State.m_Q) + m_State.m_Center.y;
		threeD.z = threeD.z * m_State.m_W;
	}
	else
		threeD = Vector3( 0.0f );
}

void RenderContext::unProject( Vector3 &twoD )
{
	twoD.x = ( twoD.z * (twoD.x - m_State.m_Center.x) ) / m_State.m_D;
	twoD.y = ( twoD.z * -(twoD.y - m_State.m_Center.y) ) / m_State.m_Q;
}

void RenderContext::unProjectFloat( Vector3 & twoD )
{
	// convert to pixels
	twoD.x *= m_State.m_Window.width();
	twoD.y *= m_State.m_Window.height();
	// unproject into viewspace
	unProject( twoD );
}

Vector3 RenderContext::worldToView( const Vector3 & position )
{
	return m_State.m_Frame * (position - m_State.m_Position);
}

Matrix33 RenderContext::worldToView( const Matrix33 & frame )
{
	return m_State.m_Frame * frame;
}

BoxHull RenderContext::worldToView( const BoxHull & hull )
{
	return BoxHull( worldToView( hull.min ), worldToView( hull.max ) );
}

Vector3 RenderContext::viewToWorld( const Vector3 & position )
{
	return m_State.m_Position + (m_State.m_Frame % position);
}

Matrix33 RenderContext::viewToWorld( const Matrix33 & frame )
{
	return m_State.m_Frame % frame;
}

//----------------------------------------------------------------------------

void RenderContext::setDisplay( DisplayDevice * pDisplay )
{
	m_Display = pDisplay;
}

void RenderContext::setAudio( AudioDevice * pAudio )
{
	m_Audio = pAudio;
}

void RenderContext::setTime( float time )
{
	m_State.m_Time = time;
}

void RenderContext::setBits( dword bits )
{
	m_State.m_Bits = bits;
}

void RenderContext::setInstanceKey( qword key )
{
	m_State.m_nInstanceKey = key;
}

RenderContext::InstanceData * RenderContext::instanceData( const ClassKey & a_ClassKey )
{
	InstanceData * pData = NULL;

	InstanceDataMap::iterator iData = m_InstanceDataMap.find( m_State.m_nInstanceKey );
	if ( iData != m_InstanceDataMap.end() )
		pData = iData->second;

	if (! pData || !pData->isType( a_ClassKey ) )
	{
		pData = WidgetCast<InstanceData>( Factory::createWidget( a_ClassKey ) );
		if (! pData )
		{
			TRACE( "Failed to create instance data!" );
			return NULL;
		}

		// update the map with the new data object..
		m_InstanceDataMap[ m_State.m_nInstanceKey ] = pData;
	}

	pData->setTouched( true );
	return pData;
}

void RenderContext::setAlpha( float fAlpha )
{
	m_State.m_fAlpha = Clamp( fAlpha, 0.0f, 1.0f );
}

void RenderContext::setDetail( float fDetail )
{
	m_State.m_fDetail = Clamp( fDetail, 0.0f, 1.0f );
}

void RenderContext::setFrame( const Matrix33 & frame )
{
	m_State.m_Frame = frame;
}

void RenderContext::setPosition( const Vector3 & position )
{
	m_State.m_Position = position;
}

void RenderContext::setProjection( const RectInt & window, 
						float fov, float front, float back )
{
	ASSERT( m_Display.valid() );

	if ( window.valid() )
		m_State.m_Window = window;
	else
		m_State.m_Window = m_Display->renderWindow();

	float fAspect = (float)m_State.m_Window.width() / (float)m_State.m_Window.height();
	m_State.m_Fov = Clamp( fov * fAspect, 0.0f, PI4 * 2.0f );
	m_State.m_Front = front;
	m_State.m_Back = back;

	float fH = 2.0f * sin( m_State.m_Fov / 2.0f );
	float fW = (1.0f / fAspect) * fH;
	float fF = 2.0f * cos( m_State.m_Fov / 2.0f );

	m_State.m_Fulstrum[FRONT].set(Vector3(0,0,1),m_State.m_Front);
	m_State.m_Fulstrum[BACK].set(Vector3(0,0,-1),m_State.m_Back);
	m_State.m_Fulstrum[RIGHT].set( Vector3(-fF,0.0f,fH).normalize(), 0.0f );
	m_State.m_Fulstrum[LEFT].set( Vector3(fF,0.0f,fH).normalize(), 0.0f );
	m_State.m_Fulstrum[TOP].set( Vector3(0.0f,-fF,fW).normalize(), 0.0f );
	m_State.m_Fulstrum[BOTTOM].set( Vector3(0.0f,fF,fW).normalize(), 0.0f );

	// used by project and unProject
	m_State.m_Center = PointFloat( (m_State.m_Window.right + m_State.m_Window.left) >> 1, (m_State.m_Window.top + m_State.m_Window.bottom) >> 1 );
	m_State.m_D = (m_State.m_Window.width() * 0.5f) / tan( m_State.m_Fov * 0.5f );
	m_State.m_Q = (m_State.m_Window.height() * 0.5f) / tan( (m_State.m_Fov * (1.0f / fAspect)) * 0.5f );
	m_State.m_W = 1.0f / (m_State.m_Back - m_State.m_Front);
}

void RenderContext::setMaxShadowLights( int a_nLights )
{
	m_State.m_nMaxShadowLights = a_nLights;
}

void RenderContext::setShadowFocus( const Vector3 & a_vFocus, float a_fRadius )
{
	m_State.m_vShadowFocus = a_vFocus;
	m_State.m_fShadowRadius = a_fRadius;
}

void RenderContext::setShadowMap( const SizeInt & a_szShadowMap )
{
	m_State.m_szShadowMap = a_szShadowMap;
}

void RenderContext::setWindow( const RectInt & window )
{
	ASSERT( m_Display.valid() );

	if ( window.valid() )
		m_State.m_Window = window;
	else
		m_State.m_Window = m_Display->renderWindow();

	float fMaxAspect = PI2 / m_State.m_Fov;
	float fAspect = Min( (float)m_State.m_Window.width() / (float)m_State.m_Window.height(), fMaxAspect );

	// used by project and unProject
	m_State.m_Center = PointFloat( (m_State.m_Window.right + m_State.m_Window.left) >> 1, (m_State.m_Window.top + m_State.m_Window.bottom) >> 1 );
	m_State.m_D = (m_State.m_Window.width() * 0.5f) / tan( m_State.m_Fov * 0.5f );
	m_State.m_Q = (m_State.m_Window.height() * 0.5f) / tan( (m_State.m_Fov * (1.0f / fAspect)) * 0.5f );
	m_State.m_W = 1.0f / (m_State.m_Back - m_State.m_Front);
}

void RenderContext::setOffset( const PointInt & offset )
{
	m_State.m_Offset = offset;
}

void RenderContext::updateTransforms()
{
	// set the projection
	m_Display->setProjection( m_State.m_Frame, 
		m_State.m_Position, 
		m_State.m_Window, 
		m_State.m_Fov, 
		m_State.m_Front, 
		m_State.m_Back );
}

void RenderContext::enableOR( float a_fMetersPerUnit /*= 1.0f*/ )
{
}

void RenderContext::disableOR()
{
}

void RenderContext::lock()
{
	if ( m_Display.valid() )
		m_Display->lock();
}

void RenderContext::unlock()
{
	if ( m_Display.valid() )
		m_Display->unlock();
}

void RenderContext::beginRender( Color clearColor, Color ambient )
{
	if ( !m_State.m_Window.valid() )
		setWindow( m_State.m_Window );
	ASSERT( m_Display.valid() );

	// cleared the touched flag from all instance data...
	for( InstanceDataMap::iterator iInstanceData = m_InstanceDataMap.begin();
		iInstanceData != m_InstanceDataMap.end(); ++iInstanceData )
	{
		iInstanceData->second->setTouched( false );
	}

	// set the projection
	m_Display->setProjection( m_State.m_Frame, 
		m_State.m_Position, 
		m_State.m_Window, 
		m_State.m_Fov, 
		m_State.m_Front, 
		m_State.m_Back );
	m_Display->clear( clearColor );
	m_Display->clearZ( 1.0f );
	m_Display->setAmbient( ambient );
}

void RenderContext::render( Scene * pScene )
{
	pScene->render( *this, Matrix33::IDENTITY, Vector3::ZERO );
}

void RenderContext::render( BaseNode * pNode )
{
	pNode->preRender( *this, Matrix33::IDENTITY, Vector3::ZERO );
}

void RenderContext::push( DevicePrimitive * pPrimitive )
{
	if ( pPrimitive != NULL )
		m_Display->push( pPrimitive );
}

void RenderContext::push( Material * pMaterial )
{
	if ( pMaterial != NULL )
		push( pMaterial->material( *this ) );
}

void RenderContext::pushTransform( const Matrix33 & m, const Vector3 & v )
{
	push( PrimitiveSetTransform::create( display(), m, v ) );
}

void RenderContext::pushWorldTransform( const Matrix33 & m, const Vector3 & v )
{
	push( PrimitiveSetTransform::create( display(), worldToView( m ), worldToView( v ) ) );
}

void RenderContext::pushIdentity()
{
	push( PrimitiveSetTransform::create( display() ) );
}

bool RenderContext::beginScene()
{
	if (! m_Display->beginScene() )
		return false;
	
	// clear all lights from the device
	m_Display->clearLights();
	// set our shadow pass parameters
	m_Display->setShadowPass( m_State.m_nMaxShadowLights,
		m_State.m_vShadowFocus, 
		m_State.m_fShadowRadius, 
		m_State.m_szShadowMap );
	// update our view transforms
	updateTransforms();

	return true;
}

bool RenderContext::beginShadowPass()
{
	DisplayDevice::Transform lightTransform;
	if (! m_Display->beginShadowPass( lightTransform ) )
		return false;

	pushState();

	m_State.m_bShadowPass = true;
	m_State.m_Frame = lightTransform.m_mFrame;
	m_State.m_Position = lightTransform.m_vTranslate;
	m_Display->setProjection( m_State.m_Frame, 
		m_State.m_Position, 
		m_State.m_Window, 
		m_State.m_Fov,
		m_State.m_Front, 
		m_State.m_Back );

	return true;
}

void RenderContext::endShadowPass()
{
	m_Display->endShadowPass();
	popState();
}

bool RenderContext::endScene()
{
	if (! m_Display->endScene() )
		return false;

	return true;
}

bool RenderContext::abortScene()
{
	m_Display->abortScene();
	return true;
}

void RenderContext::endRender()
{
	// render the scene
	m_Display->endScene();
	// display the new render
	m_Display->present();
	// remove all lights
	m_Display->clearLights();

	if ( m_State.m_Clock != 0 )
	{
		dword current = Time::milliseconds();
		if ( current > m_State.m_Clock )
		{
			m_State.m_Elapsed = float( current - m_State.m_Clock ) / 1000.0f;
			m_State.m_Fps = 1.0f / m_State.m_Elapsed;
		}
		else
		{
			m_State.m_Fps = 128.0f;		// time time elapsed since last render is 0, very fast!
			m_State.m_Elapsed = 1.0f / 128.0f;
		}
		m_State.m_Clock = current;
	}
	else
		m_State.m_Clock = Time::milliseconds();

	// removed untouched instance data
	for( InstanceDataMap::iterator iInstanceData = m_InstanceDataMap.begin();
		iInstanceData != m_InstanceDataMap.end(); )
	{
		if (! iInstanceData->second->isTouched() )
			m_InstanceDataMap.erase( iInstanceData++ );
		else
			++iInstanceData;
	}
	
}

void RenderContext::flushInstanceData()
{
	m_InstanceDataMap.clear();
}

bool RenderContext::pushState()
{
	m_StateList.push_back( m_State );
	return true;
}

bool RenderContext::popState()
{
	if ( m_StateList.size() > 0 )
	{
		m_State = m_StateList.back();
		m_StateList.pop_back();
		updateTransforms();

		return true;
	}
	return false;
}

//----------------------------------------------------------------------------

void RenderContext::renderBox( BoxHull &hull, Color color ) 
{
	Vector3 min( hull.min );
	Vector3 max( hull.max );

	Vertex lineStrip[] = 
	{
		Vertex( Vector3( min.x, min.y, min.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( min.x, max.y, min.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( max.x, max.y, min.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( max.x, min.y, min.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( max.x, min.y, max.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( max.x, max.y, max.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( min.x, max.y, max.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( min.x, min.y, max.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( min.x, min.y, min.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( max.x, min.y, min.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( max.x, max.y, min.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( max.x, max.y, max.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( max.x, min.y, max.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( min.x, min.y, max.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( min.x, max.y, max.z ), Vector3(1,0,0), 0, 0 ),	
		Vertex( Vector3( min.x, max.y, min.z ), Vector3(1,0,0), 0, 0 ),	
	};

	// push a material of the proper color
	push( Material( color, true, PrimitiveMaterial::NONE ).material( *this ) );
	// push the line strip
	push( PrimitiveLineStrip::create( m_Display, 16, lineStrip ) );
}

void RenderContext::renderPlane( const Plane & plane, Color color )
{
	const float PLANE_SIZE = 10.0f;

	Matrix33 planeFrame( ~plane.matrix() );
	Vector3 corners[] = 
	{
		planeFrame * Vector3( -PLANE_SIZE, -PLANE_SIZE, -plane.distance ),
		planeFrame * Vector3( PLANE_SIZE, -PLANE_SIZE, -plane.distance ),
		planeFrame * Vector3( PLANE_SIZE, PLANE_SIZE, -plane.distance ),
		planeFrame * Vector3( -PLANE_SIZE, PLANE_SIZE, -plane.distance ),
	};

	Vector3 normal(1,0,0);
	Vertex lineStrip[] =
	{
		Vertex( corners[0], normal, 0,0 ),
		Vertex( corners[1], normal, 0,0 ),
		Vertex( corners[2], normal, 0,0 ),
		Vertex( corners[3], normal, 0,0 ),
		Vertex( corners[0], normal, 0,0 ),
	};

	// push a material of the proper color
	push( Material( color, true, PrimitiveMaterial::ALPHA ).material( *this ) );
	// push the primitive
	push( PrimitiveLineStrip::create( m_Display, 5, lineStrip ) );
}

// creates a circle composed of a line strip along the XZ plane
//

void RenderContext::renderCircle( float radius, Color color, int segments )
{
	const int		MAX_SEGMENTS = 32;
	const Vector3	CIRCLE_NORMAL(0,1,0);
	const Color		CIRCLE_DIFFUSE(255,255,255);

	ASSERT( segments < MAX_SEGMENTS );
	static Vertex	lineStrip[ MAX_SEGMENTS ];

	const float		SEGMENT_PI = (PI * 2) / segments;

	int i = 0;
	for(;i<segments;i++)
	{
		lineStrip[i].normal = lineStrip[i].position = 
			Vector3( cosf( SEGMENT_PI * i) * radius, 0, sinf( SEGMENT_PI * i ) * radius );
		lineStrip[i].normal.normalize();
	}
	lineStrip[i] = lineStrip[0];

	// push a material of the proper color
	push( Material( color ).material( *this ) );
	// push the line strip
	push( PrimitiveLineStrip::create( m_Display, segments + 1, lineStrip ) );
}

//-------------------------------------------------------------------------------
// EOF

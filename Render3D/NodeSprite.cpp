/*
	NodeSprite.cpp
	(c)2005 Palestar, Richard Lyle
*/

#pragma warning( disable : 4786 ) // identifier was truncated to '255' characters in the browser information

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Standard/Limits.h"
#include "Display/PrimitiveSetTransform.h"
#include "Display/PrimitiveTriangleFan.h"
#include "Render3D/NodeSprite.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeSprite, NodeTransform );
REGISTER_FACTORY_KEY(  NodeSprite, 3974553774174497927LL );

BEGIN_PROPERTY_LIST( NodeSprite, NodeTransform )
	ADD_PROPERTY( m_Scale );
	ADD_PROPERTY( m_ScaleLock );
	ADD_PROPERTY( m_ClampBack );
	ADD_PROPERTY( m_EnableAlpha );
	ADD_PROPERTY( m_Alpha );
	ADD_PROPERTY( m_AlphaV );
	ADD_PROPERTY( m_AlphaA );
	ADD_ENUM_PROPERTY( m_AxisLock );
		ADD_ENUM_OPTION( m_AxisLock, LOCKNONE );
		ADD_ENUM_OPTION( m_AxisLock, LOCKX );
		ADD_ENUM_OPTION( m_AxisLock, LOCKY );
		ADD_ENUM_OPTION( m_AxisLock, LOCKZ );
		ADD_ENUM_OPTION( m_AxisLock, LOCKXY );
		ADD_ENUM_OPTION( m_AxisLock, LOCKXZ );
		ADD_ENUM_OPTION( m_AxisLock, LOCKYZ );
		ADD_ENUM_OPTION( m_AxisLock, LOCKXYZ );
	ADD_PROPERTY( m_Material );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeSprite::NodeSprite()
{
	m_Scale = 1.0f;
	m_ScaleLock = false;
	m_ClampBack = false;
	m_EnableAlpha = false;
	m_AxisLock = LOCKNONE;
}

//----------------------------------------------------------------------------

BoxHull NodeSprite::hull() const
{
	return( BoxHull( Vector3(-0.5f,-0.5f,-0.5f) * m_Scale, Vector3(0.5f,0.5f,0.5f) * m_Scale ) );
}

void NodeSprite::render( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	DisplayDevice * pDisplay = context.display();
	ASSERT( pDisplay );

	float frameScale = frame.scale();
	// get the position
	Vector3 positionVS( context.worldToView( position ) );
	// if the scale is locked, then scale the quad based on the distance from the viewer so
	// it stays the same size
	float scale = m_ScaleLock ? (m_Scale * positionVS.z) : m_Scale;

	// sprites can be clamped so they don't go behind the back clipping plane, check for this option
	if ( m_ClampBack )
	{
		// bring the back clipping plane forward by the scale of the object, otherwise parts still may get clipped
		float back = context.back();
		// if any of the axis are locked, then bring the backplane in by the scale of the object, otherwise parts
		// may get clipped by the backplane
		if ( (m_AxisLock & (LOCKX|LOCKY|LOCKZ)) != 0 )
			back -= m_Scale;

		if ( positionVS.z > back ) //&& back > context.front() )
		{
			scale *= back / positionVS.z;

			Plane backPlane( Vector3(0,0,-1), back );
			backPlane.intersect( Vector3(0,0,0), positionVS, positionVS );
		}
	}
	if (! context.sphereVisible( positionVS, scale * frameScale ) )
		return;
	if (! m_Material.valid( true, false ) )
		return;

	// by default the sprite is opaque
	float fAlpha = 1.0f;
	if ( m_EnableAlpha )
	{
		float t = context.time();
		float t2 = t * t;

		fAlpha = Clamp( m_Alpha + (m_AlphaV * t) + (( 0.5f * m_AlphaA) * t2), 0.0f, 1.0f );
	}

	const Color DIFFUSE( 255, 255, 255, (u8)((context.alpha() * fAlpha) * 255) );

	PrimitiveTriangleFanDL::Ref pTriangleFan = 
		PrimitiveTriangleFanDL::create( pDisplay, 4 );

	const Vector3	N( 0, 0, 0 );

	VertexL * pVerts = pTriangleFan->lock();
	pVerts[ 0 ] = VertexL( Vector3( -0.5f, -0.5f, 0 ) * scale, N, DIFFUSE, 0.0f, 1.0f );
	pVerts[ 1 ] = VertexL( Vector3( -0.5f, 0.5f, 0 ) * scale, N, DIFFUSE, 0.0f, 0.0f );
	pVerts[ 2 ] = VertexL( Vector3( 0.5f, 0.5f, 0 ) * scale, N, DIFFUSE, 1.0f, 0.0f );
	pVerts[ 3 ] = VertexL( Vector3( 0.5f, -0.5f, 0 ) * scale, N, DIFFUSE, 1.0f, 1.0f );
	pTriangleFan->unlock();

	// calculate the frame of the quad based the locks
	Matrix33 frameVS( context.worldToView( frame ) );
	Matrix33 quadFrame( Matrix33( true ) * frameScale );
	if ( m_AxisLock & LOCKX)
		quadFrame.i = frameVS.i;
	if ( m_AxisLock & LOCKY)
		quadFrame.j = frameVS.j;
	if ( m_AxisLock & LOCKZ)
		quadFrame.k = frameVS.k;

	// push the primitives
	context.push( m_Material );
	context.pushTransform( context.viewToWorld( quadFrame ), position );
	context.push( pTriangleFan );

	// call the base class
	NodeTransform::render( context, frame, position );
}

//----------------------------------------------------------------------------

float NodeSprite::scale() const
{
	return( m_Scale );
}

bool NodeSprite::scaleLock() const
{
	return( m_ScaleLock );
}

NodeSprite::AxisLock NodeSprite::axisLock() const
{
	return( m_AxisLock );
}

Material::Link NodeSprite::material() const
{
	return( m_Material );
}

//----------------------------------------------------------------------------

void NodeSprite::setScale( float scale )
{
	m_Scale = scale;
}

void NodeSprite::setScaleLock( bool scaleLock )
{
	m_ScaleLock = scaleLock;
}

void NodeSprite::setClampBack( bool clampBack )
{
	m_ClampBack = clampBack;
}

void NodeSprite::setAlpha( bool enable, float alpha, float alphaV, float alphaA )
{
	m_EnableAlpha = enable;
	m_Alpha = alpha;
	m_AlphaV = alphaV;
	m_AlphaA = alphaA;
}

void NodeSprite::setAxisLock( AxisLock lock )
{
	m_AxisLock = lock;
}

void NodeSprite::setMaterial( Material::Link material )
{
	m_Material = material;
}

//----------------------------------------------------------------------------
// EOF

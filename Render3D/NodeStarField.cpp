/*
	NodeStarField.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Math/Helpers.h"
#include "Render3D/NodeStarField.h"
#include "Display/PrimitiveSetTransform.h"

//----------------------------------------------------------------------------

Color NodeStarField::s_StarColor( 255, 255, 255, 255 );
float NodeStarField::s_StarSize = 0.001f;
float NodeStarField::s_DopplerVelocity = 500.0f;

const Color BLUE_SHIFT( 128,128,255,128 );
const Color RED_SHIFT( 255,128,128,128 );

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeStarField, BaseNode );
REGISTER_FACTORY_KEY(  NodeStarField, 4400228791173987159LL );

BEGIN_PROPERTY_LIST( NodeStarField, BaseNode )
	ADD_PROPERTY( m_ParticleCount );
	ADD_PROPERTY( m_Front );
	ADD_PROPERTY( m_Back );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeStarField::NodeStarField() : m_bActive( true ), m_bTrailActive( true ), m_bJumpActive( false )
{
	m_ParticleCount = 150;
	m_Front = 1.0f;
	m_Back = 500.0f;
	m_LastTime = 0.0f;
}

//----------------------------------------------------------------------------

void NodeStarField::render( RenderContext &context, 
		const Matrix33 & frame, 
		const Vector3 & position )
{
	if (! m_bActive )
		return;

	DisplayDevice * pDisplay = context.display();
	ASSERT( pDisplay );

	if ( m_Stars.size() != m_ParticleCount )
	{
		int nPreviousCount = m_Stars.size();
		m_Stars.realloc( m_ParticleCount );
		for(int i=nPreviousCount;i<m_ParticleCount;i++)
			createParticle( context, i );
	}

	const Vector3 N(0,0,-1.0f);

	// get the star color
	Color color( s_StarColor );

	// allocate a triangle list
	PrimitiveTriangleListDL::Ref pTriangleList = 
		PrimitiveTriangleListDL::create( pDisplay, m_ParticleCount );

	Vector3 vDelta( context.position() - m_LastPosition );
	m_LastPosition = context.position();

	// update the triangle list
	VertexL * pVertex = (VertexL *)pTriangleList->lock();
	for(int i=0;i<m_Stars.size();i++)
	{
		Star & star = m_Stars[ i ];

		color = s_StarColor;
		// transform the position into view space
		Vector3 head( context.worldToView( star.m_vHead ) );
		Vector3 tail( star.m_vTail );
		star.m_vTail = head;

		if ( m_bJumpActive )
		{
			// do doppler effect per star...
			float deltaZ = (head.z - tail.z) / context.elapsed();
			if ( deltaZ < 0.0f )
				color.iterpolate( BLUE_SHIFT, Clamp<float>( -deltaZ / s_DopplerVelocity, 0.0f, 1.0f ) );
			else if ( deltaZ > 0.0f )
				color.iterpolate( RED_SHIFT, Clamp<float>( deltaZ / s_DopplerVelocity, 0.0f, 1.0f ) );
		}
		
		float size = s_StarSize * head.z;
		float alpha = 1.0f - (head.z / m_Back);
		color.a = (u8)( 255.0f * alpha );

		pVertex->position = head + Vector3( 0, size, 0 );
		pVertex->diffuse = color;
		pVertex->normal = N;
		pVertex->u = 0.0f;
		pVertex->v = 0.0f;
		pVertex++;

		pVertex->position = head + Vector3( size, 0, 0 );
		pVertex->diffuse = color;
		pVertex->normal = N;
		pVertex->u = 0.0f;
		pVertex->v = 0.0f;
		pVertex++;

		if ( m_bTrailActive || m_bJumpActive )
		{
			pVertex->position = tail;
			pVertex->diffuse = BLACK;
			pVertex->normal = N;
			pVertex->u = 0.0f;
			pVertex->v = 0.0f;
			pVertex++;
		}
		else
		{
			pVertex->position = head + Vector3( -size, 0, 0 );
			pVertex->diffuse = color;
			pVertex->normal = N;
			pVertex->u = 0.0f;
			pVertex->v = 0.0f;
			pVertex++;
		}

		// has the particle left the field of view
		if ( fabs( head.x ) > head.z || fabs( head.y ) > head.z || head.z > m_Back || head.z < m_Front )
		{
			Vector3 vRandomDirection( RandomVector( Vector3( -1.0f, -1.0f, -1.0f ), Vector3( 1.0f, 1.0f, 1.0f ) ) );
			vRandomDirection.normalize();

			// convert back to world space
			star.m_vHead = context.position() + (vRandomDirection * RandomFloat( m_Back * 0.5f, m_Back ));
			star.m_vTail = context.worldToView( star.m_vHead );
		}
	}

	pTriangleList->unlock();

	// push the primitives
	context.push( PrimitiveMaterial::create( pDisplay, PrimitiveMaterial::ADDITIVE, false, true ) );
	context.pushTransform( context.frame(), context.position() );
	context.push( pTriangleList );

	BaseNode::render( context, frame, position );
}

//----------------------------------------------------------------------------

void NodeStarField::setActive( bool bActive )
{
	m_bActive = bActive;
}

void NodeStarField::setTrailActive( bool bTrailActive )
{
	m_bTrailActive = bTrailActive;
}

void NodeStarField::setJumpActive( bool bDopplerActive )
{
	m_bJumpActive = bDopplerActive;
}

void NodeStarField::initialize( int particles, float front, float back )
{
	m_bActive = true;
	m_ParticleCount = particles;
	m_Front = front;
	m_Back = back;
}

void NodeStarField::setParticleCount( int particles )
{
	m_ParticleCount = particles;
}

//----------------------------------------------------------------------------

void NodeStarField::createParticle( RenderContext & context, int n )
{
	// create particle somewhere in the viewspace of the context
	Vector3 vsp( RandomFloat( -0.5f, 0.5f ), RandomFloat( -0.5f, 0.5f ), 1.0f );
	vsp *= RandomFloat( m_Front, m_Back );

	// transform from viewspace to objectspace
	Star & star = m_Stars[ n ];
	star.m_vTail = star.m_vHead = context.viewToWorld( vsp );
}

//----------------------------------------------------------------------------
// EOF

/*
	NodePath.cpp

	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Render3D/NodePath.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodePath, NodeTransform );
REGISTER_FACTORY_KEY(  NodePath, 4015879153176217782LL );

BEGIN_PROPERTY_LIST( NodePath, NodeTransform )
	ADD_PROPERTY(m_Velocity);
	ADD_PROPERTY(m_Acceleration);
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodePath::NodePath()
{
	m_Velocity = Vector3( true );
	m_Acceleration = Vector3( true );
}

//-------------------------------------------------------------------------------

void NodePath::preRender( RenderContext & context, const Matrix33 & frame, const Vector3 & position )
{
	float t = context.time();
	float t2 = t * t;

	m_Position = Vector3( (m_Velocity.x * t) + (( 0.5f * m_Acceleration.x ) * t2),
		(m_Velocity.y * t) + (( 0.5f * m_Acceleration.y ) * t2) ,
		(m_Velocity.z * t) + (( 0.5f * m_Acceleration.z ) * t2) );

	// call the base class
	NodeTransform::preRender( context, frame, position );
}

//-------------------------------------------------------------------------------

const Vector3 & NodePath::velocity() const
{
	return( m_Velocity );
}

const Vector3 & NodePath::acceleration() const
{
	return( m_Acceleration );
}

void NodePath::setPath( const Vector3 & velocity, const Vector3 & acceleration )
{
	m_Velocity = velocity;
	m_Acceleration = acceleration;
}

//-------------------------------------------------------------------------------
// EOF

/*
	NodeSpin.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Render3D/NodeSpin.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeSpin, NodeTransform );
REGISTER_FACTORY_KEY(  NodeSpin, 3937521656325190842LL );

BEGIN_PROPERTY_LIST( NodeSpin, NodeTransform )
	ADD_PROPERTY( m_Yaw );
	ADD_PROPERTY( m_Pitch );
	ADD_PROPERTY( m_Roll );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeSpin::NodeSpin()
{
	m_Yaw = m_Pitch = m_Roll = 0.0f;
}

//-------------------------------------------------------------------------------

void NodeSpin::preRender( RenderContext & context, const Matrix33 & frame, const Vector3 & position )
{
	const float t = context.time();
	// set the rotation of this object based on the current context time
	m_Frame = Matrix33( true ).rotate( 
		m_Pitch * t, 
		m_Yaw * t,
		m_Roll * t );

	// call the base class
	NodeTransform::preRender( context, frame, position );
}

//-------------------------------------------------------------------------------

float NodeSpin::pitch() const
{
	return( m_Pitch );
}

float NodeSpin::yaw() const
{
	return( m_Yaw );
}

float NodeSpin::roll() const
{
	return( m_Roll );
}

//-------------------------------------------------------------------------------

void NodeSpin::setSpin( float pitch, float yaw, float roll )
{
	m_Pitch = pitch;
	m_Yaw = yaw;
	m_Roll = roll;
}

//-------------------------------------------------------------------------------
// EOF

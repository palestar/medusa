/*
	NodeScale.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Render3D/NodeScale.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeScale, NodeTransform );
REGISTER_FACTORY_KEY(  NodeScale, 4012818636223375058LL );

BEGIN_PROPERTY_LIST( NodeScale, NodeTransform )
	ADD_PROPERTY( m_XV );
	ADD_PROPERTY( m_XA );
	ADD_PROPERTY( m_YV );
	ADD_PROPERTY( m_YA );
	ADD_PROPERTY( m_ZV );
	ADD_PROPERTY( m_ZA );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeScale::NodeScale()
{
	m_XV = m_XA = m_YV = m_YA = m_ZV = m_ZA = 0.0f;
}

void NodeScale::preRender( RenderContext & context, const Matrix33 & frame, const Vector3 & position )
{
	float t = context.time();
	float t2 = t * t;

	float x = (m_XV * t) + (( 0.5f * m_XA ) * t2) + 1.0f;
	float y = (m_YV * t) + (( 0.5f * m_YA ) * t2) + 1.0f;
	float z = (m_ZV * t) + (( 0.5f * m_ZA ) * t2) + 1.0f;

	m_Frame = Matrix33( Vector3(1,0,0) * x,
		Vector3(0,1,0) * y,
		Vector3(0,0,1) * z );

	// call the base class
	NodeTransform::preRender( context, frame, position );
}

//-------------------------------------------------------------------------------

float NodeScale::XV() const
{
	return( m_XV );
}

float NodeScale::XA() const
{
	return( m_XA );
}

float NodeScale::YV() const
{
	return( m_YV );
}

float NodeScale::YA() const
{
	return( m_YA );
}

float NodeScale::ZV() const
{
	return( m_ZV );
}

float NodeScale::ZA() const
{
	return( m_ZA );
}

//-------------------------------------------------------------------------------

void NodeScale::setScale( float xv, float xa, float yv, float ya, float zv, float za )
{
	m_XV = xv;
	m_XA = xa;
	m_YV = yv;
	m_YA = ya;
	m_ZV = zv;
	m_ZA = za;
}

//-------------------------------------------------------------------------------
// EOF

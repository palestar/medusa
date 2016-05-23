/*
	NodeBlur.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Render3D/NodeBlur.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeBlur, BaseNode );
REGISTER_FACTORY_KEY(  NodeBlur, 4013906789616206450LL );

BEGIN_PROPERTY_LIST( NodeBlur, BaseNode )
	ADD_PROPERTY( m_Time );
	ADD_PROPERTY( m_Steps );
	ADD_PROPERTY( m_Blending );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeBlur::NodeBlur()
{
	m_Time = 1.0f;	
	m_Steps = 4;
	m_Blending = PrimitiveMaterial::ADDITIVE;
}

//-------------------------------------------------------------------------------

void NodeBlur::preRender( RenderContext & context, const Matrix33 & frame, const Vector3 & position )
{
	// render the objects without blur
	//BaseNode::preRender( context, frame, position );

	float currentTime = context.time();
	float time = currentTime;
	float timeStep = m_Time / m_Steps;

	for(int i=0;i<m_Steps;i++)
	{
		// render the children
		BaseNode::preRender( context, frame, position );

		time -= timeStep;
		if ( time < 0.0f )
			break;

		context.setTime( time );
	}

	// restore the time
	context.setTime( currentTime );
}

//-------------------------------------------------------------------------------

float NodeBlur::time() const
{
	return( m_Time );
}

int NodeBlur::steps() const
{
	return( m_Steps );
}

NodeBlur::Blending NodeBlur::blending() const
{
	return( m_Blending );
}

//-------------------------------------------------------------------------------

void NodeBlur::setBlur( float time, int steps, Blending blending )
{
	m_Time = time;
	m_Steps = steps;
	m_Blending = blending;
}

//-------------------------------------------------------------------------------
// EOF

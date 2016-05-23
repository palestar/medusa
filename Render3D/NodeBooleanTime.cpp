/*
	NodeBooleanTime.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Standard/Bits.h"
#include "Render3D/Scene.h"
#include "Render3D/NodeBooleanTime.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeBooleanTime, BaseNode );
REGISTER_FACTORY_KEY(  NodeBooleanTime, 4012817650362092086LL );

BEGIN_PROPERTY_LIST( NodeBooleanTime, BaseNode )
	ADD_PROPERTY( m_Begin )
	ADD_PROPERTY( m_End )
	ADD_PROPERTY( m_Invert )
	ADD_PROPERTY( m_WarpTime )
	ADD_PROPERTY( m_Loop )
	ADD_PROPERTY( m_LoopEnd )
END_PROPERTY_LIST()

//----------------------------------------------------------------------------

NodeBooleanTime::NodeBooleanTime()
{
	m_Begin = m_End = 0.0f;
	m_Invert = false;
	m_WarpTime = true;
	m_Loop = false;
	m_LoopEnd = 0.0f;
}

//----------------------------------------------------------------------------

void NodeBooleanTime::preRender( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	float currentTime = context.time();
	float time = m_Loop ? fmod( currentTime, m_LoopEnd ) : currentTime;

	if ( (!m_Invert && (time < m_Begin || time > m_End)) ||
		(m_Invert && time > m_Begin && time < m_End ) )
		return;

	// offset the current time by this nodes beginning time
	context.setTime( m_WarpTime ? time - m_Begin : time );
	// call base class, which renders children of this node
	BaseNode::preRender( context, frame, position );

	// restore the time
	context.setTime( currentTime );
}

//----------------------------------------------------------------------------
// EOF

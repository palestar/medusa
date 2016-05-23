/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODESOUNDPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"

#include "Render3d/NodeSound.h"
#include "Tools/ScenePort/ChildFrame.h"
#include "Tools/ResourcerDoc/Port.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeSoundPort, BaseNodePort );
REGISTER_FACTORY_KEY(  NodeSoundPort, 3974612490862532462 );

BEGIN_PROPERTY_LIST( NodeSoundPort, BaseNodePort );
	ADD_PORT_PROPERTY( m_SoundPort, SoundPort );
	ADD_PROPERTY( m_Falloff );
	ADD_PROPERTY( m_Loop );
END_PROPERTY_LIST();

NodeSoundPort::NodeSoundPort() : BaseNodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeSound);
	m_Name = "NodeSound";
	m_Falloff = 1.0f / 50.0f;	// sound is out to a distance of 50 meters
	m_Loop = false;
}

//-------------------------------------------------------------------------------

const int VERSION_042999 = 1;
const int VERSION_081399 = 2;
const int VERSION_111599 = 111599;
const int VERSION_021800 = 21800;

bool NodeSoundPort::read( const InStream & input )
{
	if (! BaseNodePort::read( input ) )
	{
		int version;
		input >> version;

		int m_Type;	
		float m_TriggerTime;

		switch( version )
		{
		case VERSION_021800:
			input >> m_SoundPort;
			input >> m_Falloff;
			input >> m_Loop;
			break;
		case VERSION_111599:
			input >> m_SoundPort;
			input >> m_Falloff;
			m_Loop = true;
			break;
		case VERSION_081399:
			input >> m_SoundPort;
			input >> m_TriggerTime;
			input >> m_Falloff;
			m_Loop = true;
			break;
		case VERSION_042999:
			input >> m_SoundPort;
			input >> m_Type;
			input >> m_TriggerTime;
			input >> m_Falloff;
			m_Loop = true;
			break;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------

void NodeSoundPort::dependencies( DependentArray & dep )
{
	dep.push( m_SoundPort );
}

CFrameWnd * NodeSoundPort::createView()
{
	return BaseNodePort::createView();
}

BaseNode * NodeSoundPort::createNode()
{
	return BaseNodePort::createNode();
}

void NodeSoundPort::initializeNode( BaseNode * pNode )
{
	BaseNodePort::initializeNode( pNode );

	NodeSound * pSound = WidgetCast<NodeSound>( pNode );
	if ( pSound != NULL )
	{
		// get the sound resource
		Sound::Link sound = 
			WidgetCast<Sound>( Port::portResource( m_SoundPort ) );

		pSound->setSound( sound );
		pSound->setFalloff( m_Falloff );
		pSound->setLoop( m_Loop );
	}
}


//-------------------------------------------------------------------------------
// EOF


/*
	NodeSound.h
	(c)2005 Palestar, Richard Lyle
*/

#pragma warning( disable : 4786 ) // identifier was truncated to '255' characters in the browser information

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Standard/Limits.h"
#include "Render3D/Scene.h"
#include "Render3D/NodeSound.h"

//-------------------------------------------------------------------------------

bool NodeSound::sm_bDisable = false;		// disable sound nodes

//---------------------------------------------------------------------------------------------------

IMPLEMENT_NAMED_FACTORY( FACTORY_SoundInstance, NodeSound::SoundInstance, RenderContext::InstanceData, true );

NodeSound::SoundInstance::SoundInstance() : m_bInitialized( false )
{}

NodeSound::SoundInstance::~SoundInstance()
{
	// if we have a valid buffer, stop the sound..
	if ( m_pBuffer.valid() )
		m_pBuffer->stop();
}

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeSound, BaseNode );
REGISTER_FACTORY_KEY(  NodeSound, 3974628642175910903LL );

BEGIN_PROPERTY_LIST( NodeSound, BaseNode )
	ADD_PROPERTY( m_Sound );
	ADD_PROPERTY( m_Falloff );
	ADD_PROPERTY( m_Loop );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeSound::NodeSound()
{
	m_Falloff = 1.0f / 50.0f;
	m_Loop = false;
}

NodeSound::~NodeSound()
{}

//----------------------------------------------------------------------------

void NodeSound::render( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	if (! context.audioDevice() )
		return;	// no valid audio device
	if ( context.isShadowPass() )
		return;	// no sounds on shadow passes
	if (! m_Sound.valid( true, false ) )
		return;
	if ( sm_bDisable )
		return;

	SoundInstance * pInstance = context.instanceData<SoundInstance>();
	if ( m_Loop || !pInstance->m_bInitialized )
	{
		pInstance->m_bInitialized = true;

		Vector3 positionVS( context.worldToView( position ) );
		float soundDistance = positionVS.magnitude();
		float soundVolume = Clamp(1.0f - (soundDistance * m_Falloff),0.0f,1.0f);
		float soundPan = Clamp(positionVS.x / Max(positionVS.z, 1.0f ),-1.0f,1.0f);

		if (! pInstance->m_pBuffer.valid() )
		{
			if ( soundVolume > 0 )
				pInstance->m_pBuffer = m_Sound->play( context.audioDevice(), soundVolume, soundPan, m_Loop );
		}
		else if ( pInstance->m_pBuffer.valid() )
		{
			pInstance->m_pBuffer->setVolume( soundVolume );
			pInstance->m_pBuffer->setPan( soundPan );
		}
	}
}

//----------------------------------------------------------------------------

Sound *	NodeSound::sound() const
{
	return( m_Sound );
}

float NodeSound::falloff() const
{
	return( m_Falloff );
}

bool NodeSound::loop() const
{
	return( m_Loop );
}

//----------------------------------------------------------------------------

void NodeSound::setSound( Sound * pSound )
{
	m_Sound = pSound;
}

void NodeSound::setFalloff( float falloff )
{
	m_Falloff = falloff;
}

void NodeSound::setLoop( bool loop )
{
	m_Loop = loop;
}

//----------------------------------------------------------------------------
// EOF

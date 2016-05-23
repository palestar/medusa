/*
	NodeSound.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_SOUND_H
#define NODE_SOUND_H

#include "Audio/Sound.h"
#include "Render3D/BaseNode.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeSound : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	struct SoundInstance : public RenderContext::InstanceData
	{
		DECLARE_WIDGET_CLASS();

		SoundInstance();
		virtual ~SoundInstance();

		bool				m_bInitialized;
		AudioBuffer::Ref	m_pBuffer;
	};

	typedef Reference<NodeSound>			Ref;

	// Construction
	NodeSound();
	virtual ~NodeSound();

	// Node Interface
	void				render( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );

	// Accessors
	Sound *				sound() const;				// sound resource linked to this node
	float				falloff() const;			// sound falloff rate
	bool				loop() const;

	// Mutators
	void				setSound( Sound * pSound );
	void				setFalloff( float falloff );
	void				setLoop( bool loop );

	// Static
	static bool			sm_bDisable;				// disable sound nodes

private:
	// Data
	Sound::Link			m_Sound;
	float				m_Falloff;			
	bool				m_Loop;
};

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF

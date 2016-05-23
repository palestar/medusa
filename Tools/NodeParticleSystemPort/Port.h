/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEPARTICLESYSTEMPORT_H
#define NODEPARTICLESYSTEMPORT_H

#include "Standard/Dll.h"
#include "Tools/ScenePort/NodePort.h"
#include "Render3d/NodeParticleSystem.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEPARTICLESYSTEMPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeParticleSystemPort : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeParticleSystemPort>	Ref;

	// Construction
	NodeParticleSystemPort();
	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	// Data
	CharString					m_MaterialPort;

    float                       m_SystemDuration;
    bool                        m_bContinuous;
	bool						m_bLocalSpace;
    float                       m_ParticleRadius;
    float                       m_percentRadiusRandomness;

    dword                       m_maxNumParticles;                   
    //
    float                       m_MeanInitialSpeed;
    float                       m_InitialPositionMaxRadius;
    Vector3						m_percentInitialPositionRandomness;
    float                       m_percentInitialSpeedRandomness;
    Vector3						m_percentInitialDirectionRandomness;

    float                       m_ParticleEmissionRate;
    float                       m_percentEmissionRateRandomness;
    float                       m_ParticleLifeRandomness;

    float                       m_ParticleLifetime;
    float                       m_FadeInDuration;
    float                       m_FadeOutDuration;

    float                       m_ParticleMass;                 
    float                       m_BouyantForceMagnitude;
    Vector3						m_WindAcceleration;

    bool                        m_bAlphaFadeIn;
    bool                        m_bAlphaFadeOut;
    bool                        m_bScaleIn;
    bool                        m_bScaleOut;

    bool                        m_bRadialVelocity;
    Vector3						m_InitVelocity;

    bool                        m_bAnimateColor;
    dword			            m_ColorGradStart;
    dword			            m_ColorGradEnd;
	bool						m_bCreateOnSurface;

	Vector3						m_BoxDiagonal;
	bool						m_bCreateInBox;
	bool						m_bLiveInBox;

};


#endif

//-------------------------------------------------------------------------------
// EOF

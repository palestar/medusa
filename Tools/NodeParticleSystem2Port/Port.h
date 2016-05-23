/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEPARTICLESYSTEM2PORT_H
#define NODEPARTICLESYSTEM2PORT_H

#include "Standard/Dll.h"
#include "Render3d/NodeParticleSystem2.h"
#include "Tools/ScenePort/NodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEPARTICLESYSTEM2PORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeParticleSystem2Port : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeParticleSystem2Port>	Ref;
	typedef NodeParticleSystem2::Particle		Particle;

	enum OriginType
	{
		BOX,
		SPHERE,
		LINE,
		CIRCLE,
	};

	// Construction
	NodeParticleSystem2Port();
	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	// Data
	dword						m_Seed;						// random number seed
	int							m_ParticleCount;			// number of particles
	int							m_OriginType;
	Vector3						m_MinOrigin;
	Vector3						m_MaxOrigin;
	Vector3						m_MinVelocity;
	Vector3						m_MaxVelocity;
	Vector3						m_MinAcceleration;
	Vector3						m_MaxAcceleration;
	float						m_MinLife;
	float						m_MaxLife;
	bool						m_ReverseTime;
	bool						m_bLocalSpace;

	float						m_Life;						// what is the life span of this system
	CharString					m_MaterialPort;

	float						m_Visible;					// number of particles rendered, 0.0 - 1.0f
	float						m_VisibleV;					// change in visible particles over time
	float						m_VisibleA;					// change in velocity over time
	float						m_Scale;					// particle scale
	float						m_ScaleV;					// change in scale over time
	float						m_ScaleA;					// change in velocity over time
	float						m_ScaleVar;					// scale variation
	float						m_Alpha;					// particle alpha, 0.0 - 1.0f
	float						m_AlphaV;					// change in alpha over time
	float						m_AlphaA;					// change in velocity over time
};


#endif

//-------------------------------------------------------------------------------
// EOF

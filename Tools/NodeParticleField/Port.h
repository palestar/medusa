/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEPARTICLEFIELD_H
#define NODEPARTICLEFIELD_H

#include "Standard/Dll.h"
#include "Tools/ScenePort/BaseNodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODEPARTICLEFIELD_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeParticleFieldPort : public BaseNodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeParticleFieldPort>	Ref;

	// Construction
	NodeParticleFieldPort();
	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	CharString					m_MaterialPort;
	int							m_ParticleCount;
	float						m_Scale;
	float						m_Front;
	float						m_Back;
	Vector3						m_Velocity;
};


#endif

//-------------------------------------------------------------------------------
// EOF

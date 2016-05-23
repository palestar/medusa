/*
	NodePort.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_PORT_H
#define NODE_PORT_H
 
#include "Render3d/NodeTransform.h"
#include "Tools/ScenePort/BaseNodePort.h"
#include "Tools/ScenePort/ScenePortDll.h"

//----------------------------------------------------------------------------

class DLL NodePort : public BaseNodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodePort>	Ref;

	// Construction
	NodePort();

	// Widget Interface
	bool						read( const InStream & input );
	// BaseNodePort Interface
	void						initializeNode( BaseNode * pNode );

	bool						canScale();
	void						scale( int x, int y );
	bool						canMove();
	void						move( Vector3 & movexy );
	bool						canPitch();
	void						pitch( float r );
	bool						canYaw();
	void						yaw( float r );
	bool						canRoll();
	void						roll( float r );
					
	void						resetPosition( Vector3 & pos );
	void						resetFrame();

	// Data
	Vector3						m_Position;				// position and frame
	Matrix33					m_Frame;
	float						m_fFrameScale;			// scale of the node
};

//----------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------
// EOF

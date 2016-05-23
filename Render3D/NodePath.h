/*
	NodePath.h

	This node type moves with a velocity and acceleration linear through time
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_PATH_H
#define NODE_PATH_H

#include "Render3D/NodeTransform.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodePath : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodePath>			Ref;

	// Construction
	NodePath();
	// NodeTransform Interface
	void				preRender( RenderContext & context, 
							const Matrix33 & frame, const Vector3 & position );
	// Accessors
	const Vector3 &		velocity() const;
	const Vector3 &		acceleration() const;
	// Mutators
	void				setPath( const Vector3 & velocity, const Vector3 & acceleration );

private:
	// Data
	Vector3				m_Velocity;
	Vector3				m_Acceleration;
};

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

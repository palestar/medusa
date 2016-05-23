/*
	NodeSpin.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_SPIN_H
#define NODE_SPIN_H

#include "Render3D/NodeTransform.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodeSpin : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeSpin>			Ref;

	// Construction
	NodeSpin();

	// NodeTransform Interface
	void				preRender( RenderContext & context, 
							const Matrix33 & frame, const Vector3 & position );
	// Accessors
	float				pitch() const;
	float				yaw() const;
	float				roll() const;

	// Mutators
	void				setSpin( float pitch, float yaw, float roll );

private:
	// Data
	float				m_Pitch, m_Yaw, m_Roll;
};

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

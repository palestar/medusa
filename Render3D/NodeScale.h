/*
	NodeScale.h

	This node is used to scale the transformation over time
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_SCALE_H
#define NODE_SCALE_H

#include "Render3D/NodeTransform.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodeScale : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeScale>			Ref;

	// Construction
	NodeScale();

	// NodeTransform Interface
	void				preRender( RenderContext & context, 
							const Matrix33 & frame, const Vector3 & position );
	// Accessors
	float				XV() const;
	float				XA() const;
	float				YV() const;
	float				YA() const;
	float				ZV() const;
	float				ZA() const;

	// Mutators
	void				setScale( float xv, float xa, float yv, float ya, float zv, float za );

private:
	// Data
	float				m_XV, m_XA;
	float				m_YV, m_YA;
	float				m_ZV, m_ZA;
};

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

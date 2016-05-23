/*
	NodeBlur.h

	This node is used to produce a blur effect on it's child nodes, it 
	does this by rendering the child nodes several times over time.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_BLUR_H
#define NODE_BLUR_H

#include "Display/PrimitiveMaterial.h"
#include "Render3D/BaseNode.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodeBlur : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeBlur>			Ref;
	typedef PrimitiveMaterial::Blending	Blending;

	// Construction
	NodeBlur();

	// BaseNode Interface
	void				preRender( RenderContext & context, 
							const Matrix33 & frame, 
							const Vector3 & position );
	// Accessors
	float				time() const;
	int					steps() const;
	Blending			blending() const;

	// Mutators
	void				setBlur( float time, int steps, Blending blending );

private:
	// Data
	float				m_Time;		// how far back in time to blur
	int					m_Steps;	// number of steps back in time
	Blending			m_Blending;	// what blending mode to use
};

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

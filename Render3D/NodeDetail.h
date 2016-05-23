/*
	NodeDetail.h

	This node renders only one child based on the distance from the viewer,
	the distance from the viewer is divided by the nodes 'distance', the 
	integer of that result is the child that is rendered.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_DETAIL_H
#define NODE_DETAIL_H

#include "Render3D/BaseNode.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodeDetail : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeDetail>			Ref;

	// Construction
	NodeDetail();

	// Node Interface
	void				preRender( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );
	// Accessors
	float				distance() const;
	// Mutators
	void				setDistance( float distance );

private:
	// Data
	float				m_Distance;
};

//-------------------------------------------------------------------------------

inline float NodeDetail::distance() const
{
	return( m_Distance );
}

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

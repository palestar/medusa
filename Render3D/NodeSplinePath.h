/*
	NodeSplinePath.h

	Like NodePath, except this node follows a uniform non-rational b-spline curve
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_SPLINE_PATH_H
#define NODE_SPLINE_PATH_H

#include "Render3D/NodeTransform.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodeSplinePath : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeSplinePath>			Ref;

	// Construction
	NodeSplinePath();

	// NodeTransform Interface
	void				preRender( RenderContext & context, 
							const Matrix33 & frame, const Vector3 & position );
	// Accessors
	int					pointCount() const;
	const Vector3 &		point( int n ) const;
	// Mutators
	void				setPath( const Array< Vector3 > & points );
private:
	// Data
	Array< Vector3 >	m_Points;
};

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

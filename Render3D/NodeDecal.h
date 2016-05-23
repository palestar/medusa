/*
	NodeDecal.h

	This node displays a texture on the surface of it's parent mesh
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef NODEDECAL_H
#define NODEDECAL_H

#include "Display/PrimitiveTriangleList.h"
#include "Render3D/NodeTransform.h"
#include "Render3D/NodeComplexMesh2.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeDecal : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeDecal>			Ref;
	
	// Construction
	NodeDecal();

	// BaseNode interface
	BoxHull 			hull() const;

	void				preRender( RenderContext &context, 
							const Matrix33 & frame, const Vector3 & position );

	// NodeTransform Interface
	void				setPosition( const Vector3 &position );
	void				setFrame( const Matrix33 &frame );

	// Accessors
	Material *			material() const;			// get the assigned material

	// Mutators
	void				setMaterial( Material * pMaterial );
	void				invalidate();				// invalidate the decal

private:
	// Data
	float				m_Scale;
	Material::Link		m_Material;

	// non-serialized
	NodeComplexMesh2::Ref 
						m_Decal;					// the decal geometry

};



#endif

//----------------------------------------------------------------------------
// EOF


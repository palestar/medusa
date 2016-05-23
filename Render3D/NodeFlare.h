/*
	NodeFlare.h

	This object renders a len's flare from it's location
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_FLARE_H
#define NODE_FLARE_H

#include "Render3D/NodeTransform.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeFlare : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Static
	static bool			sm_bFlareEnabled;			// when HDR is enabled, lens flares are turned off..

	// Construction
	NodeFlare();

	// NodeTransform interface
	void				render( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );
	// Accessors
	Material *			material() const;
	float				scale() const;
	// Mutators
	void				setFlare( Material * pMaterial, float scale );

private:
	// Data
	Material::Link		m_Material;
	float				m_Scale;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

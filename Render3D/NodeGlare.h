/*
	NodeGlare.h

	This class is used to display a glare in the render, it can be used
	for bright lights, stars, or explosions.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_GLARE_H
#define NODE_GLARE_H

#include "Render3D/NodeTransform.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeGlare : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Construction
	NodeGlare();

	// NodeTransform interface
	void				render( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );
	// Mutators
	void				setColor( Color color );
	void				setIntensity( float intensity, float intensityV, float intensityA );
	void				setFocus( float focus );
	void				setFalloff( float falloff );

private:
	// Data
	Color				m_Color;
	float				m_Intensity;
	float				m_IntensityV;
	float				m_IntensityA;
	float				m_Focus;
	float				m_Falloff;
};

//----------------------------------------------------------------------------

inline void NodeGlare::setColor( Color color )
{
	m_Color = color;
}

inline void NodeGlare::setIntensity( float intensity, float intensityV, float intensityA )
{
	m_Intensity = intensity;
	m_IntensityV = intensityV;
	m_IntensityA = intensityA;
}

inline void NodeGlare::setFocus( float focus )
{
	m_Focus = focus;
}

inline void NodeGlare::setFalloff( float falloff )
{
	m_Falloff = falloff;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

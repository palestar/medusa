/*
	PrimitiveWindowD3D.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_TRIANGLE_LIST_D3D_H 
#define PRIMITIVE_TRIANGLE_LIST_D3D_H

#include "Display/PrimitiveWindow.h"
#include "DisplayD3D/DisplayDeviceD3D.h"



//------------------------------------------------------------------------------------

class PrimitiveWindowD3D : public PrimitiveWindow
{
public:
	// Construction
	PrimitiveWindowD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleList interface
	bool			initialize( const RectInt & window, const RectFloat & uv, Color diffuse );

private:
	// Data
	VertexTL		m_Quad[ 4 ];
};

//----------------------------------------------------------------------------



#endif

//------------------------------------------------------------------------------------
// EOF

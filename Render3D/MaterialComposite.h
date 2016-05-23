/*
	MaterialComposite.h

	This material class uses the render bits to activate / deactivate decals on the diffuse mapping.
	A decal is a 2D image, drawn into the texture at a specified point.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef MATERIAL_COMPOSITE_H
#define MATERIAL_COMPOSITE_H

#include "Render3D/Material.h"
#include "Render3D/Render3dDll.h"



//----------------------------------------------------------------------------

class DLL MaterialComposite : public Material
{
public:
	DECLARE_WIDGET_CLASS();

	struct Decal
	{
		dword			bits;
		Image::Link		image;
		PointInt		point;
	};

	// Construction
						MaterialComposite();
	// Widget interface
	bool				read( InStream & );
	bool				write( OutStream & );
	// Material interface
	void				push( DisplayDevice * pDevice, float time, dword bits );
	// Accessors
	int					decalCount() const;
	const Decal &		decal( int n ) const;
	// Mutators
	int					addDecal( const Decal & decal );

protected:
	// Data
	Array< Decal >		m_Decals;

	// non-serialized
	Hash< dword, Array< PrimitiveSurface::Ref > >
						m_Surfaces;			
	// Mutators
	void				createDevicePrimitives( DisplayDevice::Ref device, dword bits );
};

//----------------------------------------------------------------------------

inline InStream & operator>>(InStream & input, MaterialComposite::Decal & decal )
{
	input >> decal.bits;
	input >> decal.image;
	input >> decal.point;
	return input;
}

inline OutStream & operator<<(OutStream & output, const MaterialComposite::Decal & decal )
{
	output << decal.bits;
	output << decal.image;
	output << decal.point;
	return output;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

	
/*
	PrimitiveSurface.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_SURFACE_H
#define PRIMITIVE_SURFACE_H

#include "Display/DisplayDevice.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL PrimitiveSurface : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveSurface>		Ref;
	typedef ColorFormat::Format				Format;

	// surface types
	enum Type {
		DIFFUSE,			// diffuse texture
		LIGHTMAP,			// lightmap surface
		BUMPMAP,			// bumpmap surface
		DARKMAP,
		DETAILMAP,
		GLOSSMAP,
		NORMALMAP,
		PARALLAXMAP,
		DECALMAP,
		SHADERMAP,

		LAST = SHADERMAP
	};
	enum TextureMode {
		TM_WRAP,		
		TM_MIRROR,
		TM_CLAMP,
		TM_BORDER,
		TM_MIRRORONCE,

		TM_LAST = TM_MIRRORONCE
	};
	enum {
		MAX_TEXTURE_PARAMS			= 6,
	};
	
	// Accessors
	virtual bool			mipmap() const = 0;					// is this surface mipmapped..
	virtual int				levels() const = 0;					// number of mipmap levels fo this surface
	virtual SizeInt			size() const = 0;					// get the surface size
	virtual int				pitch() const = 0;					// surface pitch
	virtual RectInt			rectangle() const = 0;				// build a surface rectangle
	virtual Format		 	colorFormat() const = 0;			// pixel format
	virtual TextureMode		textureMode() const = 0;			// texture address mode

	// Mutators
	virtual bool			initialize( int width, int height, Format eFormat, bool bMipMap = true, TextureMode eMode = TM_WRAP ) = 0;
	virtual void			set( Type eType, int nIndex, int nUV, bool bFiltered, float * pParams = NULL ) = 0;

	virtual byte *			lock( int nLevel = 0 ) = 0;			// returns pixel data pointer
	virtual bool			unlock() = 0;

	// Static
	static bool				sm_bEnableLightMaps;				// are lightmaps enabled
	static int				sm_nAnisotropy;						// level of anisotropic filtering
	static bool				sm_bEnableBumpMaps;					// are bumpmaps enabled
	static const char *		typeText( Type eType );				// get the string version of a given type

	// Helpers
	static Ref create( DisplayDevice * pDevice, int width, int height, 
						Format eFormat, bool bMipMap = true, TextureMode eMode = TM_WRAP )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( width, height, eFormat, bMipMap, eMode );

		return pPrimitive;
	}
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

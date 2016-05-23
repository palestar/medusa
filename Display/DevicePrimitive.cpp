/**
	@file DevicePrimitive.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 2/13/2011 12:23:26 AM
*/

#include "DevicePrimitive.h"

// Include all primitive types, so that we can implement primitiveKey()
//
#include "PrimitiveMaterial.h"
#include "PrimitiveSetTransform.h"
#include "PrimitiveTriangleList.h"
#include "PrimitiveTriangleFan.h"
#include "PrimitiveTriangleStrip.h"
#include "PrimitiveLineList.h"
#include "PrimitiveLineStrip.h"
#include "PrimitiveWindow.h"

//---------------------------------------------------------------------------------------------------

bool							PrimitiveSurface::sm_bEnableLightMaps = true;					// are lightmaps enabled
bool							PrimitiveSurface::sm_bEnableBumpMaps = true;
int								PrimitiveSurface::sm_nAnisotropy = 1;							// default to 1 AF
bool							PrimitiveMaterial::sm_bEnableSpecular = true;					// enable specular highlighting
int								PrimitiveMaterial::sm_nMaxLights = 8;

//---------------------------------------------------------------------------------------------------

IMPLEMENT_DEVICE_PRIMITIVE( DevicePrimitive );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveMaterial );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveSurface );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveSetTransform );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleList );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleListD );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleListI );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleListL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleListDL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleListTL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleListDTL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleFan );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleFanD );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleFanL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleFanDL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleFanDTL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleStrip );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleStripL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleStripDL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveTriangleStripDTL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveLineList );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveLineListL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveLineListDTL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveLineStrip );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveLineStripL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveLineStripDL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveLineStripTL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveLineStripDTL );
IMPLEMENT_DEVICE_PRIMITIVE( PrimitiveWindow );

//----------------------------------------------------------------------------

void DevicePrimitive::push( DisplayDevice * pDevice, DevicePrimitive * pPrimitive )
{
	pDevice->push( pPrimitive );
}

//---------------------------------------------------------------------------------------------------

const char * PrimitiveSurface::typeText( Type eType )
{
	static const char * TYPE_TEXT[] = 
	{
		"DIFFUSE",			// diffuse texture
		"LIGHTMAP",			// lightmap surface
		"BUMPMAP",			// bumpmap surface
		"DARKMAP",
		"DETAILMAP",
		"GLOSSMAP",
		"NORMALMAP",
		"PARALLAXMAP",
		"DECALMAP",
		"SHADERMAP"
	};

	return TYPE_TEXT[ eType ];
}

//---------------------------------------------------------------------------------------------------
//EOF

/*
	PrimitiveSurfaceD3D.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_SURFACE_D3D_H
#define PRIMITIVE_SURFACE_D3D_H

#include "Display/PrimitiveSurface.h"
#include "DisplayD3D/DisplayDeviceD3D.h"

//----------------------------------------------------------------------------

class PrimitiveSurfaceD3D : public PrimitiveSurface
{
public:
	// Types
	typedef Reference< PrimitiveSurfaceD3D >	Ref;

	// Construction
	PrimitiveSurfaceD3D();

	// DevicePrimitive interface
	virtual bool			execute();
	virtual void			clear();
	virtual void			release();

	// PrimitiveSurface interface
	virtual bool			mipmap() const;		
	virtual int				levels() const;
	virtual SizeInt			size() const;
	virtual int				pitch() const;
	virtual RectInt			rectangle() const;
	virtual Format			colorFormat() const;
	virtual TextureMode		textureMode() const;			// texture address mode

	// Mutators
	virtual bool			initialize( int width, int height, Format eFormat, bool bMipMap = true, TextureMode eMode = TM_WRAP );
	virtual void			set( Type eType, int nIndex, int nUV, bool bFiltered, float * pParams = NULL );

	virtual byte *			lock( int nLevel = 0 );			
	virtual bool			unlock();

	// Data
	Type					m_eType;						// surface type
	int						m_nIndex;
	int						m_nUV;
	bool					m_bFiltered;
	float					m_fParams[ MAX_TEXTURE_PARAMS ];

	SizeInt					m_Size;							// surface size
	Format					m_eFormat;						// format of the surface
	bool					m_bMipMap;
	TextureMode				m_eMode;
	int						m_nLevels;
	int						m_Pitch;						// surface pitch

	LPDIRECT3DTEXTURE9 		m_Texture;
	LPDIRECT3DSURFACE9		m_pLockedSurface;
};

//----------------------------------------------------------------------------



#endif

//------------------------------------------------------------------------------------
// EOF

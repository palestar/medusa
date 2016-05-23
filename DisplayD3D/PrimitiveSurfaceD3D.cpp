/*
	PrimitiveSurfaceD3D.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Trace.h"
#include "Standard/Bits.h"
#include "PrimitiveSurfaceD3D.h"
#include "PrimitiveMaterialD3D.h"
#include "PrimitiveFactory.h"

#include "d3dx9.h"

#define ENABLE_TEXTURE_COMPRESSION		0

inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }

//---------------------------------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveSurfaceD3D );

PrimitiveSurfaceD3D::PrimitiveSurfaceD3D()
{
	m_eType = DIFFUSE;
	m_nIndex = 0;
	m_nUV = 0;
	m_bFiltered = true;
	memset( m_fParams, 0, sizeof(m_fParams) );

	m_Size = SizeInt( 0, 0 );
	m_eFormat = ColorFormat::INVALID;
	m_bMipMap = true;
	m_eMode = TM_WRAP;
	m_nLevels = 0;
	m_Pitch = 0;
	m_Texture = NULL;
	m_pLockedSurface = NULL;
}

//------------------------------------------------------------------------------------

bool PrimitiveSurfaceD3D::execute()
{
	DisplayDeviceD3D * pDisplayD3D = (DisplayDeviceD3D *)m_pDevice;
	if (! pDisplayD3D )
		return false;
	LPDIRECT3DDEVICE9 pD3DD = pDisplayD3D->m_pD3DD;

	int nTextureStage = 0;
	if ( pDisplayD3D->m_bUsingFixedFunction )
	{
		switch( m_eType )
		{
		case DIFFUSE:
			{
				nTextureStage = pDisplayD3D->m_nTextureStage++;
				if ( pD3DD->SetTexture( nTextureStage, m_Texture ) != D3D_OK)
					return false;

				pD3DD->SetTextureStageState(nTextureStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );
				pD3DD->SetTextureStageState(nTextureStage, D3DTSS_COLOROP, D3DTOP_MODULATE );
				pD3DD->SetTextureStageState(nTextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				pD3DD->SetTextureStageState(nTextureStage, D3DTSS_COLORARG2, D3DTA_CURRENT );
				pD3DD->SetTextureStageState(nTextureStage, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
				pD3DD->SetTextureStageState(nTextureStage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
				pD3DD->SetTextureStageState(nTextureStage, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
			}
			break;
		case LIGHTMAP:
			{
				// check if lightmaps have been disabled
				if ( !sm_bEnableLightMaps )
					break;
				// Set the diffuse light map.
				nTextureStage = pDisplayD3D->m_nTextureStage++;
				if ( pD3DD->SetTexture( nTextureStage, m_Texture ) != D3D_OK)
					return false;

				// Set the base texture operation and args
				pD3DD->SetTextureStageState( nTextureStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );
				pD3DD->SetTextureStageState( nTextureStage, D3DTSS_COLOROP,   D3DTOP_ADD );
				pD3DD->SetTextureStageState( nTextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				pD3DD->SetTextureStageState( nTextureStage, D3DTSS_COLORARG2, D3DTA_CURRENT );
				pD3DD->SetTextureStageState( nTextureStage, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
				pD3DD->SetTextureStageState( nTextureStage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
				pD3DD->SetTextureStageState( nTextureStage, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
			}
			break;
		case BUMPMAP:
		case DARKMAP:
		case DETAILMAP:
		case GLOSSMAP:
		case NORMALMAP:
		case PARALLAXMAP:
		case DECALMAP:
		case SHADERMAP:
			break;
		}
	}
	else
	{
		if (! pDisplayD3D->m_pMatShader.valid() )
			return false;

		switch( m_eType )
		{
		case DIFFUSE:
			{
				nTextureStage = 0;
				if ( pD3DD->SetTexture( nTextureStage, m_Texture ) != D3D_OK)
					return false;
				pDisplayD3D->m_pMatShader->setConstant( "bEnableDiffuse", true );
			}
			break;
		case LIGHTMAP:
			{
				// check if lightmaps have been disabled
				if ( !sm_bEnableLightMaps )
					break;
				// Set the diffuse light map.
				nTextureStage = 1;
				if ( pD3DD->SetTexture( nTextureStage, m_Texture ) != D3D_OK)
					return false;
				pDisplayD3D->m_pMatShader->setConstant( "bEnableLightMap", true );
			}
			break;
		case BUMPMAP:
			{
				if ( !sm_bEnableBumpMaps )
					break;
				nTextureStage = 2;
				if ( pD3DD->SetTexture( nTextureStage, m_Texture ) != D3D_OK)
					return false;

				pDisplayD3D->m_pMatShader->setConstant( "bEnableBumpMap", true );
				if ( m_fParams[0] > 0.0f )
					pDisplayD3D->m_pMatShader->setConstant( "fBumpDepth", m_fParams[0] );
				else
					pDisplayD3D->m_pMatShader->clearConstant( "fBumpDepth" );		// make sure we use the default
			}
			break;
		case DARKMAP:
		case DETAILMAP:
		case GLOSSMAP:
		case NORMALMAP:
		case PARALLAXMAP:
		case DECALMAP:
		case SHADERMAP:
			break;
		}
	}

	static DWORD nTextureModes[] = 
	{
		D3DTADDRESS_WRAP,		// TM_WRAP,		
		D3DTADDRESS_MIRROR,		// TM_MIRROR,
		D3DTADDRESS_CLAMP,		// TM_CLAMP,
		D3DTADDRESS_BORDER,		// TM_BORDER,
		D3DTADDRESS_MIRRORONCE,	// TM_MIRRORONCE,
	};
	pD3DD->SetSamplerState( nTextureStage, D3DSAMP_ADDRESSU, nTextureModes[ m_eMode ] );
	pD3DD->SetSamplerState( nTextureStage, D3DSAMP_ADDRESSV, nTextureModes[ m_eMode ] );

	// set the sampling mode for this stage..
	if ( m_bFiltered )
	{
		DWORD nFilterMode = pDisplayD3D->getD3DFilterMode();
		pD3DD->SetSamplerState( nTextureStage, D3DSAMP_MAXANISOTROPY, sm_nAnisotropy );
		pD3DD->SetSamplerState( nTextureStage, D3DSAMP_MAGFILTER, nFilterMode );
		pD3DD->SetSamplerState( nTextureStage, D3DSAMP_MINFILTER, nFilterMode );
		pD3DD->SetSamplerState( nTextureStage, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC );
	}
	else
	{
		pD3DD->SetSamplerState( nTextureStage, D3DSAMP_MAXANISOTROPY, sm_nAnisotropy );	
		pD3DD->SetSamplerState( nTextureStage, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		pD3DD->SetSamplerState( nTextureStage, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		pD3DD->SetSamplerState( nTextureStage, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC );
	}

	return true;
}

void PrimitiveSurfaceD3D::clear()
{}

void PrimitiveSurfaceD3D::release()
{
	__try {
		RELEASEDD( m_pLockedSurface );
		RELEASEDD( m_Texture );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		LOG_WARNING( "PrimitiveSurfaceD3D", "Caught exception while releasing texture." );
	}
}

//------------------------------------------------------------------------------------

bool PrimitiveSurfaceD3D::mipmap() const
{
	return m_bMipMap;
}

int PrimitiveSurfaceD3D::levels() const
{
	return m_nLevels;
}

SizeInt PrimitiveSurfaceD3D::size() const
{ 
	return m_Size; 
}

int PrimitiveSurfaceD3D::pitch() const
{ 
	return m_Pitch; 
}

RectInt PrimitiveSurfaceD3D::rectangle() const
{ 
	return RectInt( 0, 0, m_Size); 
}

ColorFormat::Format PrimitiveSurfaceD3D::colorFormat() const
{ 
	return m_eFormat; 
}

PrimitiveSurface::TextureMode PrimitiveSurfaceD3D::textureMode() const
{
	return m_eMode;
}

//------------------------------------------------------------------------------------

inline int getMipLevels( int width, int height )
{
	return GetLastBit( Min( width, height ) );
}

bool PrimitiveSurfaceD3D::initialize( int width, int height, Format eFormat, bool bMipMap /*= true*/, TextureMode eMode /*= TM_WRAP*/  )
{
	// release the previous surface
	release();

	// check for bad parameters
	if ( width <= 0 || height <= 0 )
		return false;

	DisplayDeviceD3D * pDeviceD3D = (DisplayDeviceD3D *)m_pDevice;

	// initialize the new surface
	m_Size = SizeInt( width, height );
	m_eFormat = eFormat;
	m_bMipMap = bMipMap;
	m_eMode = eMode;
	m_Pitch = 0;

	// create the texture
	if ( D3D_OK != pDeviceD3D->m_pD3DD->CreateTexture( 
		width,														// Width
		height,														// Height
		m_bMipMap ? 0 : 1,											// Mipmap levels
		0,															// Usage
		DisplayDeviceD3D::findFormat( m_eFormat ),					// Format
		D3DPOOL_MANAGED,											// Pool
		&m_Texture,													// IDirect3DTexture9 **
		NULL ))														// Handle
	{
		TRACE( "PrimitiveSurfaceD3D::initialize() - Failed to create texture!" );
		return false;
	}

	m_nLevels = m_Texture->GetLevelCount();

	return true;
}

void PrimitiveSurfaceD3D::set( Type eType, int nIndex, int nUV, bool bFiltered, float * pParams /*= NULL*/ )
{
	m_eType = eType;
	m_nIndex = nIndex;
	m_nUV = nUV;
	m_bFiltered = bFiltered;

	if ( pParams != NULL )
		memcpy( m_fParams, pParams, sizeof(m_fParams) );
}

byte * PrimitiveSurfaceD3D::lock( int nLevel /*= 0*/ )
{
	if (! m_Texture )
	{
		TRACE( "PrimitiveSurfaceD3D::lock() - Can't lock surface, m_Texture is NULL!" );
		return NULL;
	}
	if ( m_pLockedSurface != NULL )
		unlock();

	if ( m_Texture->GetSurfaceLevel( nLevel, &m_pLockedSurface ) != D3D_OK )
		return NULL;
	D3DLOCKED_RECT locked;
	if ( m_pLockedSurface->LockRect( &locked, NULL, 0 ) != D3D_OK)
		return NULL;
	m_Pitch = locked.Pitch;

	return (byte *)locked.pBits;
}

bool PrimitiveSurfaceD3D::unlock()
{
	if (! m_pLockedSurface )
	{
		TRACE( "PrimitiveSurfaceD3D::unlock() - Can't unlock surface, m_Texture is NULL!" );
		return NULL;
	}

	m_pLockedSurface->UnlockRect();
	RELEASEDD( m_pLockedSurface );

	return true;
}

//------------------------------------------------------------------------------------
// EOF


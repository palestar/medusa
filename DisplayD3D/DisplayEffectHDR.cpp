/**
	@file DisplayEffectHDR.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/5/2010 5:10:39 PM
*/

#define MEDUSA_TRACE_ON

#include "DisplayEffectHDR.h"
#include "DisplayDeviceD3D.h"
#include "PrimitiveWindowD3D.h"
#include "PrimitiveMaterialD3D.h"

#include "Standard/Settings.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_LIGHT_FACTORY( DisplayEffectHDR, DisplayEffect );

BEGIN_PROPERTY_LIST( DisplayEffectHDR, DisplayEffect );
	ADD_PROPERTY( m_nBloomLevels );
	ADD_PROPERTY( m_nBloomSize );
END_PROPERTY_LIST();

#if defined(_DEBUG)
const char * CLIENT_SETTINGS = "ClientD";
#else
const char * CLIENT_SETTINGS = "Client";
#endif

//---------------------------------------------------------------------------------------------------

DisplayEffectHDR::DisplayEffectHDR() : 
	m_nBloomLevels( 1 ),
	m_nBloomSize( 4 ),
	m_pBloomRenderTarget( NULL ),
	m_pRenderTarget( NULL ),
	m_pStateBlock( NULL ),
	m_bBloomInitialized( false ),
	m_bBloomFailed( false )
{
	for(int i=0;i<2;++i)
	{
		m_pBloomTextures[i] = NULL;
		m_pBloomSurfaces[i] = NULL;
	}
}

DisplayEffectHDR::~DisplayEffectHDR()
{
	release();
}

bool DisplayEffectHDR::preRender( DisplayDevice * pDevice )
{
	if ( m_bBloomFailed || DisplayDevice::sm_bUseFixedFunction )
		return true;
	DisplayDeviceD3D * pDeviceD3D = WidgetCast<DisplayDeviceD3D>( pDevice );
	if (! pDeviceD3D )
		return false;
	LPDIRECT3DDEVICE9 pD3DD = pDeviceD3D->m_pD3DD;
	if (! pD3DD )
		return false;

	RELEASEDD( m_pRenderTarget );
	RELEASEDD( m_pStateBlock );

	if ( pD3DD->TestCooperativeLevel() != D3D_OK )
		return false;		// device is lost or something, just return for now..
	if ( pD3DD->GetRenderTarget( 0, &m_pRenderTarget ) != D3D_OK )
		return false;
	D3DSURFACE_DESC sd;
	if ( m_pRenderTarget->GetDesc( &sd ) != D3D_OK )
		return false;

	// assume we will fail, this will get set to false if everything succeeds
	m_bBloomFailed = true;	

	if (! m_bBloomInitialized || m_RenderSize != SizeInt( sd.Width, sd.Height) )
	{
		// release our previous buffers if we have them..
		RELEASEDD( m_pBloomRenderTarget );
		for(int i=0;i<2;++i)
		{
			RELEASEDD( m_pBloomTextures[i] );
			RELEASEDD( m_pBloomSurfaces[i] );
		}

		m_RenderSize = SizeInt( sd.Width, sd.Height );
		m_BloomSize = m_RenderSize / m_nBloomSize;
		
		if ( pDeviceD3D->pixelShaderVersion() < 2.0f || pDeviceD3D->vertexShaderVersion() < 2.0f )
			return false;	// pixel/vertex shader 2.0 required

		// first, load our bloom shaders..
		if (! (m_pBrightPass = pDeviceD3D->getShader( "Shaders/PS_BrightPass.fx" )) || !m_pBrightPass->valid() )
			return false;
		if (! (m_pHorzBlur = pDeviceD3D->getShader( "Shaders/PS_HorzBlur.fx" )) || !m_pHorzBlur->valid() )
			return false;
		if (! (m_pVertBlur = pDeviceD3D->getShader( "Shaders/PS_VertBlur.fx" )) || !m_pVertBlur->valid() )
			return false;
		if (! (m_pCombine = pDeviceD3D->getShader( "Shaders/PS_Combine.fx" )) || !m_pCombine->valid() )
			return false;

		// create new render target for rendering the world into..
		D3DFORMAT eFormat = D3DFMT_A16B16G16R16F;
		if ( pD3DD->CreateTexture( m_RenderSize.width, m_RenderSize.height, 1, D3DUSAGE_RENDERTARGET, eFormat, 
			D3DPOOL_DEFAULT, &m_pBloomRenderTarget, NULL ) != D3D_OK )
			return false;

		// create our buffers for the bloom effect..
		for(int i=0;i<2;++i)
		{
			if ( pD3DD->CreateTexture( m_BloomSize.width, m_BloomSize.height, 1, D3DUSAGE_RENDERTARGET, eFormat, 
				D3DPOOL_DEFAULT, &m_pBloomTextures[i], NULL ) != D3D_OK )
				return false;
			if ( m_pBloomTextures[i]->GetSurfaceLevel( 0, &m_pBloomSurfaces[i] ) != D3D_OK )
				return false;
		}	
		
		m_bBloomInitialized = true;

		// give our pixel shaders the size of our bloom buffer, so they can calculate UV's correctly..
		m_pHorzBlur->setConstant( "nDiffuseWidth", m_BloomSize.width );
		m_pHorzBlur->setConstant( "nDiffuseHeight", m_BloomSize.height );
		m_pVertBlur->setConstant( "nDiffuseWidth", m_BloomSize.width );
		m_pVertBlur->setConstant( "nDiffuseHeight", m_BloomSize.height );
		m_pCombine->setConstant( "nBloomWidth", m_BloomSize.width );
		m_pCombine->setConstant( "nBloomHeight", m_BloomSize.height );

		Settings settings( CLIENT_SETTINGS );
		m_pCombine->setConstant( "fScale", settings.get( "bloomScale", 100 ) / 100.0f );
	}

	if ( pD3DD->CreateStateBlock( D3DSBT_ALL, &m_pStateBlock ) != D3D_OK )
	{
		TRACE( "ERROR: Failed to create state block!" );
		return false;
	}
	// setup eveyything to be rendered into our bloom surface..
	IDirect3DSurface9 * pRenderSurface = NULL;
	if ( m_pBloomRenderTarget->GetSurfaceLevel( 0, &pRenderSurface ) != D3D_OK )
	{
		TRACE( "ERROR: Failed to get surface level!" );
		return false;
	}
	if ( pD3DD->SetRenderTarget( 0, pRenderSurface ) != D3D_OK )
	{
		TRACE( "ERROR: SetRenderTarget( 0, pRenderSurface ) failed!" );
		return false;
	}

	// clear our render target surface to black - TODO: Perhaps, we should use the previous render target surface
	// to initialize our surface instead.
	pD3DD->Clear( 0, NULL, D3DCLEAR_TARGET, BLACK.BGRA(), 0, 0);
	// release our render surface pointer, we no longer need it..
	RELEASEDD( pRenderSurface );

	m_bBloomFailed = false;

	return true;
}

bool DisplayEffectHDR::postRender( DisplayDevice * pDevice )
{
	if ( m_bBloomFailed || DisplayDevice::sm_bUseFixedFunction )
		return true;
	DisplayDeviceD3D * pDeviceD3D = WidgetCast<DisplayDeviceD3D>( pDevice );
	if (! pDeviceD3D )
		return false;
	LPDIRECT3DDEVICE9 pD3DD = pDeviceD3D->m_pD3DD;
	if (! pD3DD )
		return false;
	if (! m_pStateBlock )
		return false;

	// assume the bloom has failed unless it reaches the end of this function and we clear this flag again
	m_bBloomFailed = true;

	// set sampler states
	pD3DD->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	pD3DD->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	pD3DD->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	pD3DD->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pD3DD->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	pD3DD->SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	pD3DD->SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	pD3DD->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	pD3DD->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pD3DD->SetSamplerState( 1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	// turn off z-buffer while doing this effect..
	pD3DD->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );

	// first step - render our scene at 1/4 it's original size into our first bloom buffer, using the bright pass pixel
	// shader... this will darken the image.
	PrimitiveMaterialD3D pMat;
	pMat.setDevice( pDevice );
	pMat.setMaterial( WHITE, WHITE, BLACK, BLACK, 0.0f );
	pMat.setBlending( PrimitiveMaterial::NONE );
	pMat.setDoubleSided( false );
	pMat.setLightEnable( false );
	pMat.setFilterMode( PrimitiveMaterial::FILTER_ON );
	pMat.setShader( m_pBrightPass );

	if (! pMat.execute() )
		return false;

	if ( pD3DD->SetTexture( 0, m_pBloomRenderTarget ) != D3D_OK )
	{
		TRACE( "ERROR: SetTexture( 0, m_pBloomRenderTarget ) failed!" );
		return false;
	}
	if ( pD3DD->SetRenderTarget( 0, m_pBloomSurfaces[0] ) != D3D_OK )
	{
		TRACE( "ERROR: SetRenderTarget( 0, m_pBloomSurfaces[0] ) failed!" );
		return false;
	}

	PrimitiveWindowD3D pQuad;
	pQuad.setDevice( pDevice );
	pQuad.initialize( RectInt( PointInt(-1,-1), m_BloomSize + SizeInt(2,2) ), RectFloat( 0.0f, 0.0f, 1.0f, 1.0f ), WHITE );
	pQuad.execute();

	// now blur the result N times...
	for(int i=0;i<m_nBloomLevels;++i)
	{
		pMat.setShader( m_pHorzBlur );
		if (! pMat.execute() )
			return false;

		if ( pD3DD->SetRenderTarget( 0, m_pBloomSurfaces[1] ) != D3D_OK )
		{
			TRACE( "ERROR: SetRenderTarget( 0, m_pBloomSurfaces[1] ) failed!" );
			return false;
		}

		if ( pD3DD->SetTexture( 0, m_pBloomTextures[0] ) != D3D_OK )
		{
			TRACE( "ERROR: SetTexture( 0, m_pBloomTextures[0] ) failed!" );
			return false;
		}
		pQuad.execute();

		pMat.setShader( m_pVertBlur );
		if (! pMat.execute() )
			return false;

		if ( pD3DD->SetRenderTarget( 0, m_pBloomSurfaces[0] ) != D3D_OK )
		{
			TRACE( "ERROR: SetRenderTarget( 0, m_pBloomSurfaces[0] ) failed!" );
			return false;
		}

		if ( pD3DD->SetTexture( 0, m_pBloomTextures[1] ) != D3D_OK )
		{
			TRACE( "ERROR: SetTexture( 0, m_pBloomTextures[1] ) failed!" );
			return false;
		}

		pQuad.execute();
	}

	// restore the render target
	if ( pD3DD->SetRenderTarget( 0, m_pRenderTarget ) != D3D_OK )
	{
		TRACE( "ERROR: Failed to restore render target!" );
		return false;
	}

	// combine our original scene and our bloom texture..
	pMat.setShader( m_pCombine );
	if (! pMat.execute() )
		return false;

	if ( pD3DD->SetTexture( 0, m_pBloomRenderTarget ) != D3D_OK )
	{
		TRACE( "ERROR: Failed to set bloom render target!" );
		return false;
	}
	if ( pD3DD->SetTexture( 1, m_pBloomTextures[0] ) != D3D_OK )
	{
		TRACE( "ERROR: Failed to set bloom texture!" );
		return false;
	}

	pQuad.initialize( RectInt( PointInt(0,0), m_RenderSize + SizeInt(1,1) ), RectFloat( 0.0f, 0.0f, 1.0f, 1.0f ), WHITE );
	pQuad.execute();

	// restore the device state
	if ( m_pStateBlock->Apply() != D3D_OK )
		TRACE( "ERROR: m_pStateBlock->Apply() failed!" );
	RELEASEDD( m_pStateBlock );

	// no problems, clear failure flag..
	m_bBloomFailed = false;

	return true;
}

void DisplayEffectHDR::release()
{
	RELEASEDD( m_pRenderTarget );
	RELEASEDD( m_pStateBlock );
	RELEASEDD( m_pBloomRenderTarget );

	for(int i=0;i<2;++i)
	{
		RELEASEDD( m_pBloomTextures[i] );
		RELEASEDD( m_pBloomSurfaces[i] );
	}

	m_bBloomInitialized = false;
	m_bBloomFailed = false;
}

//---------------------------------------------------------------------------------------------------
//EOF

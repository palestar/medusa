/**
	@file DisplayEffectBlur.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 9/21/2010 9:41:26 PM
*/

#include "DisplayEffectBlur.h"
#include "DisplayDeviceD3D.h"
#include "PrimitiveWindowD3D.h"
#include "PrimitiveMaterialD3D.h"

#include "Standard/Settings.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_LIGHT_FACTORY( DisplayEffectBlur, DisplayEffect );

BEGIN_PROPERTY_LIST( DisplayEffectBlur, DisplayEffect );
	ADD_PROPERTY( m_fBlur );
END_PROPERTY_LIST();

//---------------------------------------------------------------------------------------------------

DisplayEffectBlur::DisplayEffectBlur() : 
	m_fBlur( 0.95f ),
	m_pBlurRenderTarget( NULL ),
	m_pPreviousTexture( NULL ),
	m_pPreviousSurface( NULL ),
	m_pRenderTarget( NULL ),
	m_pStateBlock( NULL ),
	m_bBlurInitialized( false ),
	m_bBlurFailed( false )
{}

DisplayEffectBlur::~DisplayEffectBlur()
{
	release();
}

bool DisplayEffectBlur::preRender( DisplayDevice * pDevice )
{
	if ( m_bBlurFailed )
		return false;
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

	if (! m_bBlurInitialized || m_BufferSize != SizeInt( sd.Width, sd.Height) )
	{
		// release our previous buffers if we have them..
		RELEASEDD( m_pBlurRenderTarget );
		RELEASEDD( m_pPreviousSurface );
		RELEASEDD( m_pPreviousTexture );

		// assume we will fail, this will get set to false if everything succeeds
		m_bBlurFailed = true;	
		m_BufferSize = SizeInt( sd.Width, sd.Height );
		
		if ( pDeviceD3D->pixelShaderVersion() < 2.0f || pDeviceD3D->vertexShaderVersion() < 2.0f )
			return false;	// pixel/vertex shader 2.0 required

		// create new render target for rendering the world into..
		if ( pD3DD->CreateTexture( m_BufferSize.width, m_BufferSize.height, 1, D3DUSAGE_RENDERTARGET, sd.Format, 
			D3DPOOL_DEFAULT, &m_pBlurRenderTarget, NULL ) != D3D_OK )
			return false;
		if ( pD3DD->CreateTexture( m_BufferSize.width, m_BufferSize.height, 1, D3DUSAGE_RENDERTARGET, sd.Format, 
			D3DPOOL_DEFAULT, &m_pPreviousTexture, NULL ) != D3D_OK )
			return false;
		if ( m_pPreviousTexture->GetSurfaceLevel( 0, &m_pPreviousSurface ) != D3D_OK )
			return false;

		// load our shaders..
		if (! (m_pCombine = pDeviceD3D->getShader( "Shaders/PS_Blur.fx" )) )
			return false;

		m_bBlurFailed = false;
		m_bBlurInitialized = true;
	}

	if ( pD3DD->CreateStateBlock( D3DSBT_ALL, &m_pStateBlock ) != D3D_OK )
		return false;
	// setup eveyything to be rendered into our bloom surface..
	IDirect3DSurface9 * pRenderSurface = NULL;
	if ( m_pBlurRenderTarget->GetSurfaceLevel( 0, &pRenderSurface ) != D3D_OK )
		return false;
	pD3DD->SetRenderTarget( 0, pRenderSurface );
	pD3DD->Clear( 0, NULL, D3DCLEAR_TARGET, BLACK.BGRA(), 0, 0);

	RELEASEDD( pRenderSurface );

	return true;
}

bool DisplayEffectBlur::postRender( DisplayDevice * pDevice )
{
	DisplayDeviceD3D * pDeviceD3D = WidgetCast<DisplayDeviceD3D>( pDevice );
	if (! pDeviceD3D )
		return false;
	LPDIRECT3DDEVICE9 pD3DD = pDeviceD3D->m_pD3DD;
	if (! pD3DD )
		return false;
	if (! m_pStateBlock )
		return false;

	// update our blur constant
	m_pCombine->setConstant( "fScale", m_fBlur );

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

	PrimitiveMaterialD3D pMat;
	pMat.setDevice( pDevice );
	pMat.setMaterial( WHITE, WHITE, BLACK, BLACK, 0.0f );
	pMat.setBlending( PrimitiveMaterial::NONE );
	pMat.setDoubleSided( false );
	pMat.setLightEnable( false );
	pMat.setFilterMode( PrimitiveMaterial::FILTER_ON );
	pMat.setShader( m_pCombine );
	pMat.execute();

	pD3DD->SetTexture( 0, m_pBlurRenderTarget );
	pD3DD->SetTexture( 1, m_pPreviousTexture );
	pD3DD->SetRenderTarget( 0, m_pPreviousSurface );

	PrimitiveWindowD3D pQuad;
	pQuad.setDevice( pDevice );
	pQuad.initialize( RectInt( PointInt(0,0), m_BufferSize + SizeInt(1,1) ), RectFloat( 0.0f, 0.0f, 1.0f, 1.0f ), WHITE );
	pQuad.execute();

	// now update our previous frame from the new frame..
	pMat.clearShaders();
	pMat.setBlending( PrimitiveMaterial::ADDITIVE );
	pMat.execute();

	pD3DD->SetTexture( 0, m_pPreviousTexture );
	pD3DD->SetRenderTarget( 0, m_pRenderTarget );
	pQuad.execute();

	// restore the device state
	m_pStateBlock->Apply();
	RELEASEDD( m_pStateBlock );

	pD3DD->SetRenderTarget( 0, m_pRenderTarget );

	return true;
}

void DisplayEffectBlur::release()
{
	RELEASEDD( m_pRenderTarget );
	RELEASEDD( m_pStateBlock );
	RELEASEDD( m_pBlurRenderTarget );
	RELEASEDD( m_pPreviousTexture );
	RELEASEDD( m_pPreviousSurface );

	m_bBlurInitialized = false;
	m_bBlurFailed = false;
}

//---------------------------------------------------------------------------------------------------
//EOF

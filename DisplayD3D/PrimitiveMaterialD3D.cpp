/*
	PrimitiveMaterialD3D.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "PrimitiveMaterialD3D.h"
#include "PrimitiveFactory.h"
#include "PrimitiveSetTransformD3D.h"

//------------------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveMaterialD3D );

PrimitiveMaterialD3D::PrimitiveMaterialD3D()
{
	m_nPass = DisplayDevice::PRIMARY;
	m_Blending = NONE;
	m_DoubleSided = false;
	m_LightEnable = true;
	m_bPushed = false;
	m_nFilterMode = FILTER_ON;
	m_bUpdateShaders = false;

	// setup the material to some default vlaues
	setMaterial( Color(255,255,255,255), Color(255,255,255), 
		Color(0,0,0), Color(255,255,255), 0.0f );
}

//------------------------------------------------------------------------------------

bool PrimitiveMaterialD3D::execute()
{
	DisplayDeviceD3D * pDeviceD3D = (DisplayDeviceD3D *)m_pDevice;
	if (! pDeviceD3D )
		return false;
	LPDIRECT3DDEVICE9 pD3DD = pDeviceD3D->m_pD3DD;
	if (! pD3DD )
		return false;
	// disable culling if doubleSided flag is true, enable CCW culling if false
	if ( pD3DD->SetRenderState( D3DRS_CULLMODE, m_DoubleSided ? D3DCULL_NONE : D3DCULL_CCW ) != D3D_OK)
		return false;

	setupBlending();

	if ( m_nPass == DisplayDevice::PRIMARY )
	{
		// write enabled for primary render pass
		pD3DD->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_TRUE );			
		pD3DD->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	}
	else
	{
		// no write for secondary and background passes
		pD3DD->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_FALSE );			
		pD3DD->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	}

	DisplayDeviceD3D::LightMap & lights = pDeviceD3D->m_Lights;

	// we use fixed function if lighting if disabled..
	if ( DisplayDevice::sm_bUseFixedFunction 
		|| !m_LightEnable 
		|| lights.size() == 0 
		|| m_Blending == PrimitiveMaterial::ADDITIVE )
	{
		pDeviceD3D->m_bUsingFixedFunction = true;
		// enable/disable lighting
		if ( pD3DD->SetRenderState( D3DRS_LIGHTING, m_LightEnable && lights.size() > 0 ) != D3D_OK )
			return false;
		// enable/disable specular highlighting
		if ( pD3DD->SetRenderState( D3DRS_SPECULARENABLE, (m_LightEnable && m_SpecularPower > 0.0f && sm_bEnableSpecular) ? TRUE : FALSE ) != D3D_OK)
			return false;

		D3DMATERIAL9 d3dm;
		d3dm.Diffuse = makeD3DCOLORVALUE( m_Diffuse );
		d3dm.Ambient = makeD3DCOLORVALUE( m_Ambient );
		d3dm.Emissive = makeD3DCOLORVALUE( m_Emissive );
		d3dm.Specular = makeD3DCOLORVALUE( m_Specular );
		d3dm.Power = m_SpecularPower;
		
		// apply shader if one has been set manually, needed for the HDR bloom effects..
		if ( m_pShader.valid() && m_pShader != pDeviceD3D->m_pDefaultShader )
			m_pShader->apply( pDeviceD3D );
		else
			pDeviceD3D->clearShaders();

		// set the material properties
		if ( pD3DD->SetMaterial( &d3dm ) != D3D_OK )
			return false;
		if (! setupTextureStatges() )
			return false;
		if (! executeChildren() )
			return false;
	}
	else
	{
		// turn off fixed function lighting..
		if ( pD3DD->SetRenderState( D3DRS_LIGHTING, FALSE ) != D3D_OK )
			return false;

		// update our pixel/vertex shaders if needed..
		if ( m_bUpdateShaders || !m_pShader.valid() || m_pShader->released() )
		{
			m_bUpdateShaders = false;

			m_pShader = NULL;
			if ( m_sShader.length() > 0 )
				m_pShader = pDeviceD3D->getShader( m_sShader );
			if (! m_pShader.valid() )
				m_pShader = pDeviceD3D->m_pDefaultShader;
		}

		if ( !m_pShader.valid() || m_pShader->released() )
			return false;

		pDeviceD3D->m_bUsingFixedFunction = false;
		pDeviceD3D->m_pMatShader = m_pShader;

		// setup shader constants for this material..
		m_pShader->setConstant( "vMatAmbient", m_Ambient );
		m_pShader->setConstant( "vMatDiffuse", m_Diffuse );
		m_pShader->setConstant( "vMatEmissive", m_Emissive );
		m_pShader->setConstant( "vMatSpecular", m_Specular );
		m_pShader->setConstant( "fMatSpecularPower", m_SpecularPower );
		// set global ambient light for the first lighting pass..
		m_pShader->setConstant( "vGlobalAmbient", pDeviceD3D->m_cAmbientLight );
		m_pShader->setConstant( "bEnableAmbient", true );
		m_pShader->setConstant( "szShadowMap", pDeviceD3D->m_szShadowMap );

		if (! setupTextureStatges() )
			return false;

		DisplayDeviceD3D::ShadowPassList::iterator iShadowPass = pDeviceD3D->m_ShadowPassList.begin();

		// set to alpha blending for the first pass...
		pD3DD->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		pD3DD->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		pD3DD->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

		// we need to make a render pass for each light in the scene
		int nLightCount = 0;
		for( DisplayDeviceD3D::LightMap::iterator iLight = lights.begin(); 
			iLight != lights.end(); ++iLight, ++nLightCount )
		{
			if ( nLightCount >= sm_nMaxLights )
				break;
			D3DLIGHT9 & light = iLight->second;

			m_pShader->setConstant( "nLightType", (int)light.Type );
			if ( light.Type != D3DLIGHT_DIRECTIONAL )
				m_pShader->setConstant( "vLightPosition", light.Position );
			if ( light.Type != D3DLIGHT_POINT )
				m_pShader->setConstant( "vLightDirection", light.Direction );
			m_pShader->setConstant( "vLightDiffuse", light.Diffuse );
			m_pShader->setConstant( "vLightSpecular", light.Specular );
			m_pShader->setConstant( "vLightAmbient", light.Ambient );
			
			if ( light.Type == D3DLIGHT_POINT )
			{
				Vector3 vAttenation( light.Attenuation0, light.Attenuation1, light.Attenuation2 );
				m_pShader->setConstant( "vAttenuation", vAttenation );
			}

			if ( iShadowPass != pDeviceD3D->m_ShadowPassList.end() )
			{
				DisplayDeviceD3D::ShadowPass & pass = *iShadowPass;

				// yes we have a shadow map..
				m_pShader->setConstant( "bEnableShadowMap", true );
				m_pShader->setConstant( "mLightView", pass.m_LightView );
				m_pShader->setConstant( "mLightProj", pass.m_LightProj );
				// put our shadow map into slot 7, reserved specially for shadow maps..
				pD3DD->SetTexture( 7, pass.m_pShadowMap );
				pD3DD->SetSamplerState( 7, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
				pD3DD->SetSamplerState( 7, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
				pD3DD->SetSamplerState( 7, D3DSAMP_MAGFILTER, D3DTEXF_POINT );			// IMPORTANT: Turn off all filtering on the shadow map, the shader does the filtering..
				pD3DD->SetSamplerState( 7, D3DSAMP_MINFILTER, D3DTEXF_POINT );
				pD3DD->SetSamplerState( 7, D3DSAMP_MIPFILTER, D3DTEXF_POINT );

				++iShadowPass;
			}
			else
				m_pShader->setConstant( "bEnableShadowMap", false );

			// apply the shader and upload all constants for rendering..
			if (! m_pShader->apply( pDeviceD3D ) )
				return false;
			// render geometry..
			if (! executeChildren() )
				return false;

			if ( nLightCount == 0 )
			{
				// after the first light, switch to additive blending mode..
				pD3DD->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
				pD3DD->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE );
				// disable ambient in the shader, so the colors don't accumulate per light..
				m_pShader->setConstant( "bEnableAmbient", false );
			}
		}
	}

	return true;
}


void PrimitiveMaterialD3D::clear()
{
	m_Children.release();
	m_TopTransform = NULL;
	m_bPushed = false;
}

void PrimitiveMaterialD3D::release()
{
	m_Children.release();
	m_Surfaces.release();
	m_TopTransform = NULL;
	m_bPushed = false;

	m_nPass = DisplayDevice::PRIMARY;
	m_Blending = NONE;
	m_DoubleSided = false;
	m_LightEnable = true;
	m_nFilterMode = FILTER_ON;
	m_bUpdateShaders = false;
	m_pShader = NULL;
	m_sShader = "";

	// setup the material to some default vlaues
	setMaterial( Color(255,255,255,255), Color(255,255,255), 
		Color(0,0,0), Color(255,255,255), 0.0f );
}

//------------------------------------------------------------------------------------

int PrimitiveMaterialD3D::pass() const
{
	return m_nPass;
}

Color PrimitiveMaterialD3D::diffuse() const
{
	return( m_Diffuse );
}

Color PrimitiveMaterialD3D::ambient() const
{
	return( m_Ambient );
}

Color PrimitiveMaterialD3D::emissive() const
{
	return( m_Emissive );
}

Color PrimitiveMaterialD3D::specular() const
{
	return( m_Specular );
}

float PrimitiveMaterialD3D::specularPower() const
{
	return( m_SpecularPower );
}

PrimitiveMaterial::Blending PrimitiveMaterialD3D::blending() const
{
	return( m_Blending );
}

bool PrimitiveMaterialD3D::doubleSided() const
{
	return( m_DoubleSided );
}

bool PrimitiveMaterialD3D::lightEnabled() const
{
	return( m_LightEnable );
}

PrimitiveMaterial::FilterMode PrimitiveMaterialD3D::filterMode() const
{
	return m_nFilterMode;
}

const char * PrimitiveMaterialD3D::shader() const
{
	return m_sShader;
}

//------------------------------------------------------------------------------------

void PrimitiveMaterialD3D::setPass( int nPass )
{
	m_nPass = nPass;
}

void PrimitiveMaterialD3D::setMaterial( Color diffuse, Color ambient, 
							Color emissive, Color specular, float specularPower )
{
	m_Diffuse = diffuse;
	m_Ambient = ambient;
	m_Emissive = emissive;
	m_Specular = specular;
	m_SpecularPower = specularPower;
}

void PrimitiveMaterialD3D::setBlending( Blending blending )
{
	m_Blending = blending;
}

void PrimitiveMaterialD3D::setDoubleSided( bool doubleSided )
{
	m_DoubleSided = doubleSided;
}

void PrimitiveMaterialD3D::setLightEnable( bool enable )
{
	m_LightEnable = enable;
}

void PrimitiveMaterialD3D::setFilterMode( FilterMode nMode )
{
	m_nFilterMode = nMode;
}

void PrimitiveMaterialD3D::setShader( const char * pShader )
{
	m_sShader = pShader != NULL ? pShader : "";
	m_bUpdateShaders = true;
}

int PrimitiveMaterialD3D::addSurface( PrimitiveSurface * pSurface,
								SurfaceType eType, int nIndex, int nUV, float * pParams )
{
	Surface & surface = m_Surfaces.push();
	surface.m_pSurface	= pSurface;
	surface.m_eType		= eType;
	surface.m_nIndex	= nIndex;
	surface.m_nUV		= nUV;

	if ( pParams != NULL )
		memcpy( surface.m_fParams, pParams, sizeof(surface.m_fParams) );
	
	m_Surfaces.qsort( sortSurfaces );
	return m_Surfaces.size() - 1;
}

void PrimitiveMaterialD3D::removeSurface( int n )
{
	if ( m_Surfaces.isValid( n ) )
		m_Surfaces.remove( n );
}

void PrimitiveMaterialD3D::clearSurfaces()
{
	m_Surfaces.release();
}

int PrimitiveMaterialD3D::addChild( DevicePrimitive * pPrimitive )
{
	// TODO: If this is a transparent material, sort incoming primitives based on depth, so we can
	// render them back to front and fix the transparency issues.

	// if this is a transform, and it's not our cur
	if ( pPrimitive->primitiveKey() == PrimitiveSetTransform::staticPrimitiveKey() )
	{
		if ( m_TopTransform == pPrimitive )
			return -1;		// transform is already pushed onto this material..

		m_TopTransform = (PrimitiveSetTransform *)pPrimitive;

		// avoid pushing tons on transforms into the child list, if this primitive is a transform and our last primitive
		// is another transform, just replace it..
		if ( m_Children.size() > 0 )
		{
			DevicePrimitive * pLastPrimitive = m_Children.last();
			if ( pLastPrimitive->primitiveKey() == PrimitiveSetTransform::staticPrimitiveKey() )
			{
				// last primitive is also a transform, so just replace it..
				m_Children[ m_Children.size() - 1 ] = pPrimitive;
				return m_Children.size() - 1;
			}
		}
	}

	m_Children.push( pPrimitive );
	return m_Children.size() - 1;
}

void PrimitiveMaterialD3D::removeChild( int n )
{
	if ( m_Children.isValid( n ) )
		m_Children.remove( n );
}

void PrimitiveMaterialD3D::clearChildren()
{
	m_Children.release();
}

void PrimitiveMaterialD3D::shadowPass()
{
	// execute all attached primitives
	for(int i=0;i<m_Children.size();i++)
	{
		DevicePrimitive * pPrimitive = m_Children[i];
		if (! pPrimitive )
			continue;
		// skip any textures or materials
		if ( pPrimitive->primitiveKey() == PrimitiveSurface::staticPrimitiveKey() ||
			pPrimitive->primitiveKey() == PrimitiveMaterial::staticPrimitiveKey() )
			continue;

		pPrimitive->setDevice( device() );
		if (! pPrimitive->execute() )
			TRACE( "PrimitiveMaterialD3D::execute() - Child primitive failed to execute!" );
	}
}

//---------------------------------------------------------------------------------------------------

void PrimitiveMaterialD3D::setShader( Shader::Ref pShader )
{
	m_pShader = pShader;
	m_sShader = pShader.valid() ? pShader->shaderName() : "";
}

void PrimitiveMaterialD3D::clearShaders()
{
	m_pShader = NULL;
	m_sShader = "";
}

//---------------------------------------------------------------------------------------------------

void PrimitiveMaterialD3D::setupBlending()
{
	DisplayDeviceD3D * pDeviceD3D = (DisplayDeviceD3D *)m_pDevice;

	// setup the blending mode
	switch( m_Blending )
	{
	case PrimitiveMaterial::NONE:
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		break;
	case PrimitiveMaterial::ALPHA:
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		break;
	case PrimitiveMaterial::ALPHA_INV:
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_INVSRCALPHA);
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_SRCALPHA);
		break;
	case PrimitiveMaterial::ADDITIVE:
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE );
		break;
	case PrimitiveMaterial::ADDITIVE_INV:
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_INVSRCALPHA );
		pDeviceD3D->m_pD3DD->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE );
		break;
	}
}

void PrimitiveMaterialD3D::clearTextureStages()
{
	DisplayDeviceD3D * pDeviceD3D = (DisplayDeviceD3D *)m_pDevice;

	pDeviceD3D->m_pD3DD->SetTexture( 0, NULL );
	pDeviceD3D->m_pD3DD->SetTexture( 1, NULL );
	pDeviceD3D->m_pD3DD->SetTexture( 2, NULL );
	pDeviceD3D->m_pD3DD->SetTexture( 3, NULL );
	
	// zero the texture stage
	pDeviceD3D->m_nTextureStage = 0;

	if ( m_pShader.valid() )
	{
		// turn off all texturing options by default..
		m_pShader->setConstant( "bEnableDiffuse" , false );
		m_pShader->setConstant( "bEnableLightMap" , false );
		m_pShader->setConstant( "bEnableBumpMap" , false );
	}
}

bool PrimitiveMaterialD3D::setupTextureStatges()
{
	clearTextureStages();

	// execute all attached surfaces
	for(int i=0;i<m_Surfaces.size();i++)
	{
		Surface & surface = m_Surfaces[i];
		surface.m_pSurface->setDevice( device() );
		surface.m_pSurface->set( surface.m_eType, 
			surface.m_nIndex, 
			surface.m_nUV,
			m_nFilterMode == FILTER_ON,
			surface.m_fParams );

		if (! surface.m_pSurface->execute() )
		{
			TRACE( "PrimitiveMaterialD3D::execute() - Surface failed to execute!" );
			return false;
		}
	}

	return true;
}

bool PrimitiveMaterialD3D::executeChildren()
{
	DisplayDeviceD3D * pDeviceD3D = (DisplayDeviceD3D *)m_pDevice;

	// execute all attached primitives
	for(int i=0;i<m_Children.size();i++)
	{
		DevicePrimitive * pPrimitive = m_Children[i];
		if (! pPrimitive )
			continue;

		// if we hit any surfaces, reset the texture stage..
		if ( pPrimitive->primitiveKey() == PrimitiveSurface::staticPrimitiveKey() )
			clearTextureStages();

		pPrimitive->setDevice( device() );
		if (! pPrimitive->execute() )
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------

D3DCOLORVALUE PrimitiveMaterialD3D::makeD3DCOLORVALUE( const Color &src )
{
	D3DCOLORVALUE result;

	const float resp = 1.0f / 255.0f;
	result.r = src.m_R * resp;
	result.g = src.m_G * resp;
	result.b = src.m_B * resp;
	result.a = src.m_A * resp;

	return( result );
}

Color PrimitiveMaterialD3D::makeColor( const D3DCOLORVALUE &src )
{
	return( Color( src.r * 255.0f, src.g * 255.0f, src.b * 255.0f, src.a * 255.0f ) );
}

int PrimitiveMaterialD3D::sortSurfaces( Surface p1, Surface p2 )
{
	// this array is used to sort surfaces in the order they should be executed(), lower numbers are applied first..
	static int SURFACE_SORT_ORDER[] = 
	{
		1, // DIFFUSE
		2, // LIGHTMAP
		0, // BUMPMAP
		3, // DARKMAP
		4, // DETAILMAP
		5, // GLOSSMAP
		6, // NORMALMAP
		7, // PARALLAXMAP
		8, // DECALMAP
		9, // SHADERMAP
	};

	return SURFACE_SORT_ORDER[ p1.m_eType ] - SURFACE_SORT_ORDER[ p2.m_eType ];
}

//------------------------------------------------------------------------------------
// EOF


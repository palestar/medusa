/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "stdafx.h"
#include "Port.h"
#include "PortView.h"
#include "ChildFrame.h"

#include "File/FileDisk.h"
#include "File/Path.h"
#include "Render3d/Material.h"

#include "Tools/ResourcerDoc/ResourcerDoc.h"
#include "Tools/ImagePort/Port.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( MaterialPort, Port );
REGISTER_FACTORY_KEY(  MaterialPort, 3921697333639447760 );

BEGIN_PROPERTY_LIST( MaterialPort, Port );
	ADD_PROPERTY( m_Flammable );
	ADD_PROPERTY( m_Hardness );
	ADD_PROPERTY( m_Strength );
	ADD_PROPERTY( m_Friction );
	ADD_PROPERTY( m_nPass );
	ADD_PROPERTY( m_Width );
	ADD_PROPERTY( m_Height );
	ADD_COLOR_PROPERTY( m_Diffuse );
	ADD_COLOR_PROPERTY( m_Ambient );
	ADD_COLOR_PROPERTY( m_Emissive );
	ADD_COLOR_PROPERTY( m_Specular );
	ADD_PROPERTY( m_SpecularPower );
	ADD_PROPERTY( m_LightEnable );
	ADD_ENUM_PROPERTY( m_Blending );
		ADD_ENUM_OPTION( m_Blending, PrimitiveMaterial::NONE );
		ADD_ENUM_OPTION( m_Blending, PrimitiveMaterial::ALPHA );
		ADD_ENUM_OPTION( m_Blending, PrimitiveMaterial::ALPHA_INV );
		ADD_ENUM_OPTION( m_Blending, PrimitiveMaterial::ADDITIVE );
		ADD_ENUM_OPTION( m_Blending, PrimitiveMaterial::ADDITIVE_INV );
	ADD_PROPERTY( m_DoubleSided );
	ADD_PROPERTY( m_Fps );
	ADD_PROPERTY( m_Frames );
	ADD_PROPERTY( m_Textures );
	ADD_PROPERTY( m_sShader );

	// deprecated
	ADD_PROPERTY( m_MipMap );
	ADD_PROPERTY( m_DiffusePort );
	ADD_PROPERTY( m_AlphaPort );
	ADD_PROPERTY( m_LightMap );
END_PROPERTY_LIST();

MaterialPort::MaterialPort() : Port()
{
	m_Flammable = 0.5f;
	m_Hardness = 0.5f;
	m_Strength = 0.5f;
	m_Friction = 0.5f;
	m_nPass = 1;
	m_Width = 10.0f;
	m_Height = 10.0f;

	m_Diffuse = Color(128,128,128,255);				// lighting properties
	m_Ambient = Color(128,128,128,255);
	m_Emissive = Color(0,0,0,255);
	m_Specular = Color(255,255,255,255);
	m_SpecularPower = 0.0f;
	m_LightEnable = true;

	m_Blending = PrimitiveMaterial::NONE;
	m_DoubleSided = false;
	m_MipMap = false;							// MipMap enable flag
	m_Fps = 15.0f;
	m_Frames = 1;
}

//-------------------------------------------------------------------------------

bool MaterialPort::read( const InStream & input )
{
	if (! Port::read( input ) )
		return false;

	return true;
}

//----------------------------------------------------------------------------

void MaterialPort::dependencies( DependentArray & dep )
{
	for(int i=0;i<m_Textures.size();++i)
		dep.push( m_Textures[i].m_sImagePort );
}

Resource::Ref MaterialPort::createResource()
{
	Material * pMaterial = new Material;
	
	// handle deprecated variables...
	bool bDirty = false;
	if ( m_DiffusePort.length() > 0 )
	{
		m_Textures.push( Texture( PrimitiveSurface::DIFFUSE, m_DiffusePort, m_MipMap ) );
		m_DiffusePort.empty();
		bDirty = true;
	}
	if ( m_LightMap.length() > 0 )
	{
		m_Textures.push( Texture( PrimitiveSurface::LIGHTMAP, m_LightMap, m_MipMap ) );
		m_LightMap.empty();
		bDirty = true;
	}
	if ( m_AlphaPort.length() > 0 )
	{
		report( CharString().format("Deprecated alpha mask found, adding '%s' as a shader map!", m_AlphaPort ) );
		m_Textures.push( Texture( PrimitiveSurface::SHADERMAP, m_AlphaPort, m_MipMap ) );
		m_AlphaPort.empty();
		bDirty = true;
	}

	pMaterial->setPhysical( m_Flammable, m_Hardness, m_Strength, m_Friction, m_Width, m_Height );
	pMaterial->setPass( m_nPass );
	pMaterial->setLighting( m_Diffuse, m_Ambient, m_Emissive, m_Specular, m_SpecularPower );
	pMaterial->setLightEnable( m_LightEnable );
	pMaterial->setBlending( m_Blending );
	pMaterial->setDoubleSided( m_DoubleSided );
	pMaterial->setFps( m_Fps );
	pMaterial->setFrames( m_Frames );

	for(int i=0;i<m_Textures.size();++i)
	{
		Texture & texture = m_Textures[i];

		Material::Texture addTexture;
		addTexture.m_eType		= texture.m_eType;
		addTexture.m_pImage		= Image::Link( portResource( texture.m_sImagePort ) );
		addTexture.m_bMipMap	= texture.m_bMipMap;
		addTexture.m_nIndex		= texture.m_nIndex;
		addTexture.m_nUV		= texture.m_nUV;

		for(int k=0;k<PrimitiveSurface::MAX_TEXTURE_PARAMS;++k)
			addTexture.m_fParams[k] = texture.m_fParams[k];

		if (! pMaterial->addTexture( addTexture ) )
			report( CharString().format( "Failed to add texture '%s' to material!", texture.m_sImagePort ) );
	}

	pMaterial->setShader( m_sShader );

	if ( bDirty )
		UpdateAllViews( NULL );

	return pMaterial;
}

CFrameWnd * MaterialPort::createView()
{
	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

bool MaterialPort::importFile( const char * fileName )
{
	Path diffusePath = (const char *)GetPathName();
	diffusePath.setName( diffusePath.name() + "_diffuse" );

	ImagePort * pDiffusePort = new ImagePort;
	pDiffusePort->SetPathName( diffusePath.path() , false );

	if (! pDiffusePort->importFile( fileName ) )
	{
		delete pDiffusePort;
		return false;
	}

	// save the image port to disk
	try {
		OutStream output( new FileDisk( CharString(diffusePath.path()), FileDisk::WRITE ) );
		output << pDiffusePort;
	}
	catch( File::FileError )
	{
		MessageBox( NULL, _T("Failed to save port"), diffusePath.path(), MB_OK );
		delete pDiffusePort;

		return false;
	}

	// release the diffuse port
	delete pDiffusePort;

	// link this port to the new image port
	m_Textures.push( Texture( PrimitiveSurface::DIFFUSE, diffusePath.path(), true ) );

	return true; 
}

void MaterialPort::shellOpen() const
{
	Port::Ref uncastedImage = Port::portLoad( m_DiffusePort );
	if ( uncastedImage.valid() )
		uncastedImage->shellOpen();
}

//-------------------------------------------------------------------------------
// EOF


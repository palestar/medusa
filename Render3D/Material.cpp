/*
	Material.cpp

	Material Resource
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#pragma warning( disable : 4786 ) // identifier was truncated to '255' characters in the browser information

#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Standard/Bits.h" 
#include "Standard/Limits.h" 
#include "Draw/Draw.h"
#include "Render3D/Material.h"

//---------------------------------------------------------------------------------------------------

CriticalSection				Material::sm_SurfaceHashLock;
Material::SurfaceHash		Material::sm_SurfaceHash;
SizeInt						Material::sm_MaxTextureSize( 4096, 4096 );			// max allowable texture size

//----------------------------------------------------------------------------

BEGIN_PROPERTY_LIST( Material, Resource )
	ADD_PROPERTY( m_Flammable );
	ADD_PROPERTY( m_Hardness );
	ADD_PROPERTY( m_Strength );
	ADD_PROPERTY( m_Friction );
	ADD_PROPERTY( m_nPass );
	ADD_PROPERTY( m_Width );
	ADD_PROPERTY( m_Height );
	ADD_PROPERTY( m_Diffuse );
	ADD_PROPERTY( m_Ambient );
	ADD_PROPERTY( m_Emissive );
	ADD_PROPERTY( m_Specular );
	ADD_PROPERTY( m_SpecularPower );
	ADD_PROPERTY( m_LightEnable );
	ADD_PROPERTY( m_Blending );
	ADD_PROPERTY( m_DoubleSided );
	ADD_PROPERTY( m_Fps );
	ADD_PROPERTY( m_Frames );
	ADD_PROPERTY( m_sShader );
	ADD_PROPERTY( m_Textures );
	// deprecated properties
	ADD_PROPERTY( m_DiffuseTexture );
	ADD_PROPERTY( m_LightMap );
	ADD_PROPERTY( m_MipMap );
END_PROPERTY_LIST()

//----------------------------------------------------------------------------

IMPLEMENT_RESOURCE_FACTORY( Material, Resource );

Material::Material()
{
	initialize();
}

Material::Material( Blending blend /*= PrimitiveMaterial::NONE*/ )
{
	initialize();

	m_Blending = blend;
	m_nPass = (blend == PrimitiveMaterial::NONE) ? 1 : 2;
}

Material::Material( Image * pDiffuse, Blending blend /*= PrimitiveMaterial::NONE*/ )
{
	initialize();

	m_Textures.push( Texture( PrimitiveSurface::DIFFUSE, pDiffuse ) );
	m_Blending = blend;
	m_nPass = (blend == PrimitiveMaterial::NONE) ? 1 : 2;
}

Material::Material( Color color, bool emissive /*= true*/, Blending blend /*= PrimitiveMaterial::NONE*/ )
{
	initialize();

	m_Diffuse = color;				// lighting properties
	m_Ambient = color;
	m_Emissive = emissive ? color : Color(0,0,0);
	m_Specular = color;
	m_Blending = blend;
	m_nPass = (blend == PrimitiveMaterial::NONE) ? 1 : 2;
}

Material::Material( Color diffuse, Color ambient, Color emissive, Blending blend /*= PrimitiveMaterial::NONE*/ )
{
	initialize();

	m_Diffuse = diffuse;				// lighting properties
	m_Ambient = ambient;
	m_Emissive = emissive;
	m_Blending = blend;
	m_nPass = (blend == PrimitiveMaterial::NONE) ? 1 : 2;
}

Material::~Material()
{}

//---------------------------------------------------------------------------------------------------

bool Material::read( const InStream & input )
{
	if (! Resource::read( input ) )
		return false;

	// handle deprecated properties..
	if ( m_DiffuseTexture.valid() )
	{
		m_Textures.push( Texture( PrimitiveSurface::DIFFUSE, m_DiffuseTexture ) );
		m_DiffuseTexture = NULL;
	}
	if ( m_LightMap.valid() )
	{
		m_Textures.push( Texture( PrimitiveSurface::LIGHTMAP, m_LightMap ) );
		m_LightMap = NULL;
	}

	if ( DisplayDevice::sm_pCacheDevice != NULL )
		createDeviceSurfaces( DisplayDevice::sm_pCacheDevice );

	return true;
}

//----------------------------------------------------------------------------

void Material::setPhysical( float flammable, float hardness, float strength, float friction,
						   float width, float height )
{
	m_Flammable = flammable;
	m_Hardness = hardness;
	m_Strength = strength;
	m_Friction = friction;
	m_Width = width;
	m_Height = height;
}

void Material::setPass( int nPass )
{
	m_nPass = nPass;
}

void Material::setLighting( Color diffuse, Color ambient, Color emissive, Color specular, float specularPower )
{
	m_Diffuse = diffuse;
	m_Ambient = ambient;
	m_Emissive = emissive;
	m_Specular = specular;
	m_SpecularPower = specularPower;

	m_Material = NULL;
}

void Material::setLightEnable( bool enable )
{
	m_LightEnable = enable;
	m_Material = NULL;
}

void Material::setBlending( Blending blending )
{
	m_Blending = blending;
	m_Material = NULL;
}

void Material::setDoubleSided( bool doubleSided )
{
	m_DoubleSided = doubleSided;
	m_Material = NULL;
}

void Material::setFps( float fps )
{
	m_Fps = fps;
}

void Material::setFrames( int frames )
{
	ASSERT( frames >= 1 );
	m_Frames = frames;
}

void Material::setShader( const char * pShader )
{
	m_sShader = pShader;
	m_Material = NULL;
}

bool Material::addTexture( const Texture & texture )
{
	if (! texture.m_pImage.valid() )
		return false;

	m_Textures.push( texture );
	return true;
}

bool Material::removeTexture( int n )
{
	if ( n < 0 || n >= m_Textures.size() )
		return false;
	m_Textures.remove( n );
	return true;
}

DevicePrimitive * Material::material( RenderContext & context )
{
	// check for the primitives being initialized
	if (! m_Material.valid() || m_fLastAlpha != context.alpha() || !m_bSurfaceReady )
		createDevicePrimitives( context );

	// handle animating surfaces
	if ( m_Frames > 1 )
	{
		int nFrame = int(context.time() * m_Fps) % m_Frames;
		if ( nFrame < 0 )
			nFrame = -nFrame;

		if ( nFrame != m_nLastFrame )
		{
			// remove the old textures from the material
			m_Material->clearSurfaces();

			if ( m_Surfaces.size() > 0 )
			{
				Array< PrimitiveSurface::Ref > & surfaces = m_Surfaces[ nFrame % m_Surfaces.size() ];
				ASSERT( surfaces.size() == m_Textures.size() );

				for(int i=0;i<surfaces.size();++i)
				{
					Texture & texture = m_Textures[ i ];
					m_Material->addSurface( surfaces[i], 
						texture.m_eType,
						texture.m_nIndex, 
						texture.m_nUV, 
						texture.m_fParams );
				}
			}

			m_nLastFrame = nFrame;
		}
	}

	return m_Material;
}

void Material::flush()
{
	m_Material = NULL;
	m_Surfaces.release();
}

void Material::flushMaterial()
{
	m_Material = NULL;
}

//-------------------------------------------------------------------------------

void Material::push( RenderContext & context, Material * pMaterial )
{
	if ( pMaterial != NULL )
		context.push( pMaterial->material( context ) );
}

void Material::push( RenderContext & context, Color color )
{
	context.push( Material( color ).material( context ) );
}

void Material::push( RenderContext & context, Color color, bool emissive, Blending blend )
{
	context.push( Material( color, emissive, blend ).material( context ) );
}

void Material::push( RenderContext & context, Blending blend )
{
	context.push( Material( blend ).material( context ) );
}

ColorFormat::Format Material::findBestFormat( DisplayDevice::Ref pDisplay, Image * pImage, bool bAllowCompressed )
{
	if (! pImage )
		return ColorFormat::INVALID;

	ColorFormat::Format eBestFormat = ColorFormat::INVALID;
	int bestScore = -1;

	for(int i=0;i<pDisplay->surfaceFormatCount();++i)
	{
		ColorFormat::Format eFormat = pDisplay->surfaceFormat(i);
		if ( !bAllowCompressed && (eFormat == ColorFormat::DXT1 || eFormat == ColorFormat::DXT3 || eFormat == ColorFormat::DXT5) )
			continue;				// no compressed format, skip..
		if ( eFormat == pImage->format() )
			return eFormat;			// found an exact match, always use those..

		int score = 0;
		switch( eFormat )
		{
		case ColorFormat::RGB8888:
			score = 9;
			break;
		case ColorFormat::RGB888:
		case ColorFormat::RGB888E:
			score = 8;
			break;
		case ColorFormat::RGB565:
		case ColorFormat::RGB565E:
			score = 7;
			break;
		case ColorFormat::RGB555:
			score = 6;
			break;
		case ColorFormat::RGB5551:
			score = 5;
			break;
		case ColorFormat::RGB4444:
			score = 1;
			break;
		case ColorFormat::RGB332:
			score = 0;
			break;
		}

		if ( score > bestScore || eBestFormat == ColorFormat::INVALID )
		{
			bestScore = score;
			eBestFormat = eFormat;
		}
	}

	return eBestFormat;
}


const WidgetKey MIPMAP_KEY( (qword)LL(4750620958941334377) );

PrimitiveSurface * Material::getSurface( DisplayDevice * pDisplay, Image::Link pImage, int nFrame, bool bMipMap )
{
	// generate a unique key from the components
	WidgetKey nKey = pImage.key() + nFrame;
	if ( bMipMap )
		nKey += MIPMAP_KEY;

	AutoLock lock( &sm_SurfaceHashLock );
	SurfaceHash::Iterator it = sm_SurfaceHash.find( nKey );
	if ( it.valid() )
		return *it;
	lock.release();

	// surface not found, create and cache the surface
	PrimitiveSurface::Ref pSurface;
	if ( pImage.valid() && pImage->frameCount() > 0 )
	{
		// find a format for our surface first, findBestFormat() will always try to use the images current format
		// if supported by the hardware.
		ColorFormat::Format eFormat = findBestFormat( pDisplay, pImage, true );
		if ( eFormat == ColorFormat::INVALID )
		{
			TRACE( "ERROR: Failed to find a suitable surface format for texture!" );
			return NULL;
		}

		SizeInt imageSize( pImage->size() );
		SizeInt maxSize( pDisplay->textureMaxSize() );
		SizeInt minSize( pDisplay->textureMinSize() );

		// use the smaller max size
		maxSize.width = Min( sm_MaxTextureSize.width, maxSize.width );
		maxSize.height = Min( sm_MaxTextureSize.height, maxSize.height );

		// validate the texture dimensions
		SizeInt validSize( Max( Min( imageSize.width, maxSize.width ), minSize.width), 
			Max( Min( imageSize.height, maxSize.height ), minSize.height) );
		if ( pDisplay->textureP2() && !pImage->isP2() )
			validSize = SizeInt( 1 << GetLastBit( validSize.width ), 1 << GetLastBit( validSize.height) );
		if ( pDisplay->textureSquare() && validSize.width != validSize.height )
			validSize = SizeInt( Max( validSize.width, validSize.height ), Max( validSize.width, validSize.height) );

		if ( validSize != imageSize || eFormat != pImage->format() )
		{
			// image is the wrong size or format, make a copy so we don't modify the original..
			pImage = new Image( *pImage );

			if ( pImage->size() != validSize )
			{
				// image has to be resized - switch the format to an uncompressed format, DXT compression is too slow
				// to do on the fly..
				eFormat = findBestFormat( pDisplay, pImage, false );
				if ( eFormat != pImage->format() )
				{
					TRACE( CharString().format( "Image: Reformat %s -> %s", ColorFormat::formatText( pImage->format() ),
						ColorFormat::formatText( eFormat ) ) );
					if (! pImage->setFormat( eFormat ) )
						return NULL;
				}

				TRACE( CharString().format( "Image: Resize %dx%d -> %dx%d", imageSize.width, imageSize.height, 
					validSize.width, validSize.height ) );

				pImage->resample( validSize );
				if ( bMipMap && pImage->mipMap() != NULL )
					pImage->createMipMaps();
			}
			else if ( pImage->format() != eFormat )
			{
				TRACE( CharString().format( "Image: Reformat %s -> %s", ColorFormat::formatText( pImage->format() ),
					ColorFormat::formatText( eFormat ) ) );
				if (! pImage->setFormat( eFormat ) )
					return NULL;		// failed to convert to the desired format..
			}
		}

		if ( bMipMap && pImage->mipMap() == NULL )
			bMipMap = false;		// no mipmap levels in image, disable mipmaps

		// create the primitives, note having neither a diffuse or alpha texture is also a valid material
		pDisplay->create( pSurface );

		// create the surface
		SizeInt surfaceSize( pImage->size() );
		pSurface->initialize( 
			surfaceSize.width,
			surfaceSize.height, 
			eFormat,
			bMipMap );

		nFrame = nFrame % pImage->frameCount();

		Image * pMipMap = pImage;
		for(int i=0;i<pSurface->levels();++i)
		{
			if (! pMipMap )
			{
				TRACE( CharString().format("ERROR: Mipmap level %d is missing!", i ) );
				break;
			}

			void * pPixels = pSurface->lock( i );
			if (! pPixels )
			{
				TRACE( "ERROR: Failed to lock surface!" );
				break;
			}

			memcpy( pPixels, pMipMap->frame( nFrame ), pMipMap->frameSize( nFrame ) );
			pSurface->unlock();

			// next mipmap level of our image..
			pMipMap = pMipMap->mipMap();
		}
	}

	// cache the surface now
	lock.set( &sm_SurfaceHashLock );
	sm_SurfaceHash[ nKey ] = pSurface;
	return pSurface;
}

void Material::flushSurfaceCache()
{
	AutoLock lock( &sm_SurfaceHashLock );
	sm_SurfaceHash.release();
}

//-------------------------------------------------------------------------------

void Material::initialize()
{
	m_Flammable = 0.5f;
	m_Hardness = 0.5f;
	m_Strength = 0.5f;
	m_Friction = 0.5f;
	m_nPass = 1;
	m_Width = 10.0f;
	m_Height = 10.0f;

	m_Diffuse = Color(128,128,128,255);			// lighting properties
	m_Ambient = Color(128,128,128);
	m_Emissive = Color(0,0,0);
	m_Specular = Color(255,255,255);
	m_SpecularPower = 0.0f;
	m_LightEnable = true;

	m_Fps = 15.0f;
	m_Frames = 1;
	m_Blending = PrimitiveMaterial::NONE;
	m_DoubleSided = false;
	m_MipMap = false;							// MipMap enable flag

	m_fLastAlpha = 1.0f;
	m_nLastFrame = -1;
	m_bSurfaceReady = false;
}

void Material::createDevicePrimitives( RenderContext & context )
{
	DisplayDevice * pDisplay = context.display();
	ASSERT( pDisplay );

	m_Material = NULL;		// clear any previous primitive

	pDisplay->create( m_Material );
	if ( context.alpha() < 1.0f )
	{
		m_Material->setMaterial( Color( m_Diffuse.r, m_Diffuse.g, m_Diffuse.b, (u8)(context.alpha() * m_Diffuse.a) ), 
			m_Ambient, m_Emissive, m_Specular, m_SpecularPower );
		m_Material->setPass( DisplayDevice::SECONDARY );

		if ( m_Blending == PrimitiveMaterial::NONE )
			m_Material->setBlending( PrimitiveMaterial::ALPHA );
		else
			m_Material->setBlending( m_Blending );
	}
	else
	{
		m_Material->setMaterial( m_Diffuse, m_Ambient, m_Emissive, m_Specular, m_SpecularPower );
		m_Material->setPass( m_nPass );
		m_Material->setBlending( m_Blending );
	}

	m_Material->setLightEnable( m_LightEnable );
	m_Material->setDoubleSided( m_DoubleSided );
	if ( m_sShader.length() > 0 )
		m_Material->setShader( m_sShader );

	if (! m_bSurfaceReady )
		createDeviceSurfaces( context.display() );

	// attach the surfaces to the material now
	if ( m_Surfaces.size() > 0 )
	{
		Array< PrimitiveSurface::Ref > & surfaces = m_Surfaces[ 0 ];
		ASSERT( surfaces.size() == m_Textures.size() );

		for(int i=0;i<surfaces.size();++i)
		{
			Texture & texture = m_Textures[ i ];
			m_Material->addSurface( surfaces[i], 
				texture.m_eType,
				texture.m_nIndex, 
				texture.m_nUV, 
				texture.m_fParams );
		}
	}

	m_nLastFrame = 0;
	m_fLastAlpha = context.alpha();
}

void Material::createDeviceSurfaces( DisplayDevice * pDevice )
{
	// create the primitives, note having no textures is valid..
	if ( m_Textures.size() > 0 )
	{
		m_Surfaces.allocate( m_Frames );
		for(int f=0;f<m_Frames;++f)
		{
			Array< PrimitiveSurface::Ref > & surfaces = m_Surfaces[f];
			surfaces.allocate( m_Textures.size() );

			for(int t=0;t<m_Textures.size();++t)
			{
				Texture & texture = m_Textures[t];
				surfaces[t] = getSurface( pDevice, texture.m_pImage, f, texture.m_bMipMap );
			}
		}
	}
	m_bSurfaceReady = true;
}

//----------------------------------------------------------------------------
// EOF

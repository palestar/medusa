/*
	MaterialComposite.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Standard/Bits.h"
#include "Draw/Draw.h"
#include "Render3D/MaterialComposite.h"



//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( Source, Resource );
REGISTER_FACTORY_KEY(  MaterialComposite, 4015371580030912458 );

MaterialComposite::MaterialComposite()
{}

//----------------------------------------------------------------------------

const int VERSION_081799 = 1;

bool MaterialComposite::read( InStream & input )
{
	Material::read( input );

	int version;
	input >> version;

	switch( version )
	{
	case VERSION_081799:
		input >> m_Decals;
		break;
	default:
		throw BadWidget();
	}
}

bool MaterialComposite::write( OutStream & output )
{
	Material::write( output );

	output << VERSION_081799;
	output << m_Decals;
}

//----------------------------------------------------------------------------

void MaterialComposite::push( DisplayDevice * pDevice, float time, dword bits )
{
	// check for the primitives being initialized
	if (! m_Surfaces.find( bits ).valid() )
		createDevicePrimitives( pDevice, bits );

	// push the material
	pDevice->push( m_Material );
	// push the surface
	Array< PrimitiveSurface::Ref > & surfaces = m_Surfaces[ bits ];
	if ( surfaces.size() > 0 )
		pDevice->push( surfaces[ int(time * m_Fps) % m_Frames ] );
}

//----------------------------------------------------------------------------

int MaterialComposite::decalCount() const
{
	return( m_Decals.size() );
}

const MaterialComposite::Decal & MaterialComposite::decal( int n ) const
{
	return( m_Decals[ n ] );
}

//----------------------------------------------------------------------------

int MaterialComposite::addDecal( const Decal & decal )
{
	Decal & newDecal = m_Decals.push();
	newDecal.bits = decal.bits;
	newDecal.image = decal.image;
	newDecal.point = decal.point;

	return( m_Decals.size() - 1 );
}

//----------------------------------------------------------------------------

void MaterialComposite::createDevicePrimitives( DisplayDevice::Ref device, dword bits )
{
	// create the material 
	//
	if ( m_Material == NULL )
	{
		device->create( m_Material );
		m_Material->setMaterial( m_Diffuse, m_Ambient, m_Emissive, m_Specular, m_SpecularPower );
		m_Material->setBlending( m_Blending );
	}

	if ( m_DiffuseTexture.valid() )
	{
		Image::Ref diffuseTexture = m_DiffuseTexture.valid() ? new Image( *m_DiffuseTexture ) : NULL;
		Image::Ref alphaTexture = m_AlphaTexture.valid() ? new Image( *m_AlphaTexture ) : NULL;

		float xScale = 1.0f;
		float yScale = 1.0f;

		// check the sizes of the textures
		SizeInt diffuseSize( diffuseTexture->size() );
		SizeInt maxSize( device->textureMaxSize() );

		// validate the texture dimensions
		SizeInt validSize( Min( diffuseSize.width, maxSize.width ), 
			Min( diffuseSize.height, maxSize.height ) );

		if ( device->textureP2() && !diffuseTexture->isP2() )
			validSize = SizeInt( 1 << GetLastBit( validSize.width ), 1 << GetLastBit( validSize.height) );
		if ( device->textureSquare() && validSize.width != validSize.height )
			validSize = SizeInt( Max( validSize.width, validSize.height ), Max( validSize.width, validSize.height) );
		if ( validSize != diffuseSize )
		{
			xScale = float(validSize.width ) / float( diffuseSize.width );
			yScale = float(validSize.height ) / float( diffuseSize.height );

			diffuseTexture->resample( validSize );
		}

		if ( alphaTexture.valid() && alphaTexture->size() != diffuseTexture->size() )
			alphaTexture->resample( diffuseTexture->size() );

		// make a decal list, and rescale the images and points
		Array< Image::Ref > decals;
		Array< PointInt > decalPoint;

		for(int d=0;d<m_Decals.size();d++)
		{
			Decal & decal = m_Decals[d];
			if ( decal.bits & bits )
			{
				if ( xScale != 1.0f || yScale != 1.0f )
				{
					Image * pDecal = new Image( *decal.image );
					pDecal->resample( SizeInt( xScale * pDecal->size().width, 
						yScale * pDecal->size().height ) );

					decals.push( pDecal );
				}
				else
					decals.push( decal.image );

				decalPoint.push( PointInt( xScale * decal.point.x, yScale * decal.point.y ) );
			}
		}
		ASSERT( decals.size() == decalPoint.size() );

		// do we need an alpha capible surface
		bool alphaSurface = alphaTexture.valid() ? true : false;
		// choose a color format
		ColorFormat::Format format = alphaSurface ? findBestAlphaFormat( device ) : findBestFormat( device );
		// allocate a format object
		ColorFormat * pFormat = ColorFormat::allocateFormat( format, NULL );

		// create the primitives, note having neither a diffuse or alpha texture is also a valid material
		m_Surfaces[bits].allocate( m_Frames );
		for(int f=0;f<m_Frames;f++)
		{
			PrimitiveSurface::Ref surface;

			// create the surface
			device->create( surface );
			surface->enableColorKey( m_ColorKeyEnable );
			surface->setColorKey( m_ColorKey );

			// create the surface
			surface->initialize( diffuseTexture->size().width, diffuseTexture->size().height, pFormat, 
				m_MipMap ? PrimitiveSurface::MIPMAP : PrimitiveSurface::TEXTURE );

			// create a draw object for the surface
			Draw::Ref draw = Draw::create( surface );
			// draw the texture into the surface
			draw->draw( PointInt(0,0), diffuseTexture, (f * diffuseTexture->frameCount()) / m_Frames );

			// draw the decals onto the surface
			for(int d=0;d<decals.size();d++)
				draw->draw( decalPoint[d], decals[d], (f * decals[d]->frameCount()) / m_Frames ); 

			// draw the alpha texture if valid
			if ( alphaTexture.valid() )
				draw->drawAlpha( PointInt(0,0), alphaTexture, (f * alphaTexture->frameCount()) / m_Frames );

			// optimize the surface
			surface->optimize();

			m_Surfaces[bits][f] = surface.pointer();
		}

		// release the format object
		ColorFormat::deleteFormat( pFormat );
	}
}

//----------------------------------------------------------------------------
// EOF

	
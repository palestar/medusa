/*
	Material.h

	Material Resource
	(c)2005 Palestar, Richard Lyle
*/

#ifndef MATERIAL_H
#define MATERIAL_H

#include "Standard/Hash.h"
#include "Draw/Image.h"
#include "Display/PrimitiveMaterial.h"
#include "Display/PrimitiveSurface.h"
#include "Resource/Resource.h"
#include "Render3D/RenderContext.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL Material : public Resource
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef ResourceLink<Material>			Link;
	typedef Reference< Material >			Ref;
	typedef PrimitiveMaterial::Blending		Blending;
	typedef PrimitiveSurface::Type			Type;

	struct Texture {
		Type					m_eType;			// type of surface to create
		Image::Link				m_pImage;			// image resouce
		bool					m_bMipMap;			// true if this texture should be mipmapped

		int						m_nIndex;			// all textures of the same type must have a unique index
		int						m_nUV;				// UV coordinates for this texture
		float					m_fParams[PrimitiveSurface::MAX_TEXTURE_PARAMS];		// parameters for the texture

		Texture() : m_eType( PrimitiveSurface::DIFFUSE ),
			m_nIndex( 0 ),
			m_bMipMap( true ),
			m_nUV( 0 )
		{
			for(int i=0;i<sizeof(m_fParams)/sizeof(m_fParams[0]);++i)
				m_fParams[i] = 0.0f;
		}
		Texture( Type eType, Image::Link pImage ) : m_eType( eType ),
			m_nIndex( 0 ),
			m_pImage( pImage ),
			m_bMipMap( true ),
			m_nUV( 0 )
		{
			for(int i=0;i<sizeof(m_fParams)/sizeof(m_fParams[0]);++i)
				m_fParams[i] = 0.0f;
		}
	};

	// Construction
	Material();
	Material( Blending blend );
	Material( Image * pDiffuse, Blending blend = PrimitiveMaterial::NONE );
	Material( Color color, bool emissive = true, Blending blend = PrimitiveMaterial::NONE );
	Material( Color diffuse, Color ambient, Color emissive, Blending blend = PrimitiveMaterial::NONE );
	virtual	~Material();

	// Widget interface
	bool			read( const InStream & input );

	// Accessors
	float			flammable() const;
	float			hardness() const;
	float			strength() const;
	float			friction() const;
	int				pass() const;
	float			width() const;
	float			height() const;

	Color			diffuse() const;
	Color			ambient() const;
	Color			emissive() const;
	Color			specular() const;
	float			specularPower() const;
	bool			lightEnable() const;

	Blending		blending() const;
	bool			doubleSided() const;
	float			fps() const;
	int				frames() const;

	const char *	shader() const;

	int				textureCount() const;
	const Texture&	texture(int n) const;

	int				findTexture( Type eType, int nIndex = 0 ) const;	// returns -1 if no texture found..

	// Mutators
	void			setPhysical( float flammable, float hardness, 
						float strength, float friction,
						float width, float height );
	void			setPass( int nPass );
	void			setLighting( Color diffuse, Color ambient, 
							Color emissive, Color specular, 
							float specularPower );
	void			setLightEnable( bool enable );

	void			setBlending( Blending blending );
	void			setDoubleSided( bool doubleSided );
	void			setFps( float fps );
	void			setFrames( int frames );

	void			setShader( const char * pShader );

	bool			addTexture( const Texture & texture );
	bool			removeTexture( int n );

	DevicePrimitive *	
					material( RenderContext & context );			// get the material primitive

	void			flush();					// clear any cached display primitives
	void			flushMaterial();			// flush the material, keep allocated surfaces

	// Static
	static SizeInt	sm_MaxTextureSize;			// max allowable texture size

	static void		push( RenderContext & context, Material * pMaterial );
	static void		push( RenderContext & context, Color color ); 
	static void		push( RenderContext & context, Color color, bool emissive, Blending blend );
	static void		push( RenderContext & context, Blending blend );

	static ColorFormat::Format	
					findBestFormat( DisplayDevice::Ref device, Image * pImage, bool bAllowCompressed );
	static PrimitiveSurface *
					getSurface( DisplayDevice * pDisplay, Image::Link pImage, int nFrame, bool bMipMap );
	static void		flushSurfaceCache();		// flush all cached surface primitives

protected:
	// Types
	typedef Array< Texture >							Textures;
	typedef Hash< WidgetKey, PrimitiveSurface::Ref >	SurfaceHash;
	typedef SurfaceHash::Iterator						SurfaceHashIt;

	// Data
	float			m_Flammable;			// physical properties
	float			m_Hardness;
	float			m_Strength;
	float			m_Friction;
	int				m_nPass;				// render pass
	float			m_Width;				// dimensions in meters
	float			m_Height;				

	Color			m_Diffuse;				// lighting properties
	Color			m_Ambient;
	Color			m_Emissive;
	Color			m_Specular;
	float			m_SpecularPower;
	bool			m_LightEnable;

	Blending		m_Blending;
	bool			m_DoubleSided;
	float			m_Fps;
	int				m_Frames;

	CharString		m_sShader;
	Textures		m_Textures;

	// deprecated 
	Image::Link		m_DiffuseTexture;
	Image::Link		m_AlphaTexture;
	Image::Link		m_LightMap;
	bool			m_MipMap;			

	// non-serialized
	int				m_nLastFrame;			// current frame
	float			m_fLastAlpha;			// last alpha value

	PrimitiveMaterial::Ref
					m_Material;

	bool			m_bSurfaceReady;
	Array< Array< PrimitiveSurface::Ref > >
					m_Surfaces;			

	// Mutators
	void			initialize();
	void 			createDevicePrimitives( RenderContext & context );
	void			createDeviceSurfaces( DisplayDevice * pDevice );

	// Static
	static CriticalSection
					sm_SurfaceHashLock;
	static SurfaceHash
					sm_SurfaceHash;
};

IMPLEMENT_RAW_STREAMING( Material::Blending );
IMPLEMENT_RAW_STREAMING( Material::Type );

//---------------------------------------------------------------------------------------------------

const u8 TEXTURE_VERSION_1 = 1;

inline const InStream & operator>>( const InStream & input, Material::Texture & texture )
{
	u8 nVersion;
	input >> nVersion;

	switch( nVersion )
	{
	case TEXTURE_VERSION_1:
		{
			input >> texture.m_eType;
			input >> texture.m_pImage;
			input >> texture.m_bMipMap;
			input >> texture.m_nIndex;
			input >> texture.m_nUV;
			for(int i=0;i<PrimitiveSurface::MAX_TEXTURE_PARAMS;++i)
				input >> texture.m_fParams[i];
		}
		break;
	}

	return input;
}

inline const OutStream & operator<<( const OutStream & output, Material::Texture & texture )
{
	output << TEXTURE_VERSION_1;
	output << texture.m_eType;
	output << texture.m_pImage;
	output << texture.m_bMipMap;
	output << texture.m_nIndex;
	output << texture.m_nUV;
	for(int i=0;i<PrimitiveSurface::MAX_TEXTURE_PARAMS;++i)
		output << texture.m_fParams[i];

	return output;
}

//----------------------------------------------------------------------------

inline float Material::flammable() const
{
	return( m_Flammable );
}

inline float Material::hardness() const
{
	return( m_Hardness );
}

inline float Material::strength() const
{
	return( m_Strength );
}

inline float Material::friction() const
{
	return( m_Friction );
}

inline int Material::pass() const
{
	return( m_nPass );
}

inline float Material::width() const
{
	return( m_Width );
}

inline float Material::height() const
{
	return( m_Height );
}

inline Color Material::diffuse() const
{
	return( m_Diffuse );
}

inline Color Material::ambient() const
{
	return( m_Ambient );
}

inline Color Material::emissive() const
{
	return( m_Emissive );
}

inline Color Material::specular() const
{
	return( m_Specular );
}

inline float Material::specularPower() const
{
	return( m_SpecularPower );
}

inline bool Material::lightEnable() const
{
	return( m_LightEnable );
}

inline Material::Blending Material::blending() const
{
	return( m_Blending );
}

inline bool Material::doubleSided() const
{
	return( m_DoubleSided );
}

inline float Material::fps() const
{
	return( m_Fps );
}

inline int Material::frames() const
{
	return( m_Frames );
}

inline const char * Material::shader() const
{
	return m_sShader;
}

inline int Material::textureCount() const
{
	return m_Textures.size();
}

inline const Material::Texture & Material::texture(int n) const
{
	return m_Textures[ n ];
}

inline int Material::findTexture( Type eType, int nIndex /*= 0*/ ) const
{
	for(int i=0;i<m_Textures.size();++i)
	{
		const Texture & texture = m_Textures[i];
		if ( texture.m_eType == eType && texture.m_nIndex == nIndex )
			return i;
	}
	return -1;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

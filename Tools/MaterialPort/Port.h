/*
	Port.h

	Port object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef MATERIALPORT_H
#define MATERIALPORT_H

#include "Render3d/Material.h"
#include "Tools/ResourcerDoc/Port.h"
#include "Tools/MaterialPort/PortDll.h"

//-------------------------------------------------------------------------------

class DLL MaterialPort : public Port
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<MaterialPort>		Ref;
	typedef Material					Material;
	typedef Material::Blending			Blending;
	typedef Color						Color;
	typedef Material::Type				Type;

	struct Texture {
		Type			m_eType;
		CharString		m_sImagePort;
		bool			m_bMipMap;
		int				m_nIndex;
		int				m_nUV;
		float			m_fParams[ PrimitiveSurface::MAX_TEXTURE_PARAMS ];

		bool			m_bRemove;

		Texture() : m_eType( PrimitiveSurface::DIFFUSE ),
			m_bMipMap( true ),
			m_nIndex( 0 ),
			m_nUV( 0 ),
			m_bRemove( false )
		{
			memset( m_fParams, 0, sizeof(m_fParams) );
		}
		Texture( Type eType, const char * pImagePort, bool bMipMap ) : m_eType( eType ),
			m_sImagePort( pImagePort ),
			m_bMipMap( bMipMap ),
			m_nIndex( 0 ),
			m_nUV( 0 ),
			m_bRemove( false )
		{
			memset( m_fParams, 0, sizeof(m_fParams) );
		}
	};

	// Construction
	MaterialPort();

	// Widget Interface
	bool						read( const InStream & input );
	// Port interface
	void						dependencies( DependentArray & dep );		// get the depdendencies
	Resource::Ref				createResource();							// create the resource 
	CFrameWnd *					createView();								// create a view of the port

	bool						importFile( const char * fileName );		// import from file
	void						shellOpen() const;							// open the source object through a shell program

	// Data
	float						m_Flammable;			// physical properties
	float						m_Hardness;
	float						m_Strength;
	float						m_Friction;
	int							m_nPass;
	float						m_Width;				// width of material in meters
	float						m_Height;				// height of material in meters

	Color						m_Diffuse;				// lighting properties
	Color						m_Ambient;
	Color						m_Emissive;
	Color						m_Specular;
	float						m_SpecularPower;
	bool						m_LightEnable;
	Blending					m_Blending;
	bool						m_DoubleSided;

	float						m_Fps;
	int							m_Frames;

	Array< Texture >			m_Textures;
	CharString					m_sShader;

	// deprecated
	CharString					m_DiffusePort;			// diffuse texture
	CharString					m_AlphaPort;			// alpha texture
	CharString					m_LightMap;				// lightmap texture
	bool						m_MipMap;				// MipMap enable flag
};

//----------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, MaterialPort::Texture & texture )
{
	u8 nVersion;
	input >> nVersion;

	switch( nVersion )
	{
	case 1:
		{
			input >> texture.m_eType;
			input >> texture.m_sImagePort;
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

inline const OutStream & operator<<( const OutStream & output, MaterialPort::Texture & texture )
{
	u8 version = 1;
	output << version;

	output << texture.m_eType;
	output << texture.m_sImagePort;
	output << texture.m_bMipMap;
	output << texture.m_nIndex;
	output << texture.m_nUV;
	for(int i=0;i<PrimitiveSurface::MAX_TEXTURE_PARAMS;++i)
		output << texture.m_fParams[i];

	return output;
}

//---------------------------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------
// EOF

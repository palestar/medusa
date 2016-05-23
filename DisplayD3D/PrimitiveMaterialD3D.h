/*
	PrimitiveMaterialD3D.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_MATERIAL_D3D_H
#define PRIMITIVE_MATERIAL_D3D_H

#include "Display/PrimitiveMaterial.h"
#include "Display/PrimitiveSetTransform.h"
#include "DisplayDeviceD3D.h"

//----------------------------------------------------------------------------

class PrimitiveMaterialD3D : public PrimitiveMaterial
{
public:
	// Constructions
	PrimitiveMaterialD3D();

	// DevicePrimitive interface
	bool					execute();
	void					clear();
	void					release();

	// PrimitiveMaterial interface
	virtual int				pass() const;							// render pass
	virtual Color			diffuse() const;						// Material properties
	virtual Color			ambient() const;
	virtual Color			emissive() const;
	virtual Color			specular() const;
	virtual float			specularPower() const;

	virtual Blending		blending() const;

	virtual bool			doubleSided() const;
	virtual bool			lightEnabled() const;
	virtual FilterMode		filterMode() const;
	virtual const char *	shader() const;

	virtual void			setPass( int nPass );
	virtual void			setMaterial( Color diffuse, Color ambient, 
								Color emissive, Color specular, 
								float specularPower );
	virtual void			setBlending( Blending blending );
	virtual void			setDoubleSided( bool doubleSided );
	virtual void			setLightEnable( bool enable );
	virtual void			setFilterMode( FilterMode nMode );
	virtual void			setShader( const char * pShader );

	virtual int				addSurface( PrimitiveSurface * pSurface,
								SurfaceType eType, int nIndex, int nUV, float * pParams );
	virtual void			removeSurface( int n );
	virtual void			clearSurfaces();				// remove all surfaces

	virtual int				addChild( DevicePrimitive * pPrimitive );
	virtual void			removeChild( int n );
	virtual void			clearChildren();				// remove all child primitives

	virtual void			shadowPass();

	//! Mutators
	void					setShader( Shader::Ref pShader );
	void					clearShaders();

	//! Types
	struct Surface 
	{
		PrimitiveSurface::Ref		m_pSurface;
		SurfaceType					m_eType;
		int							m_nIndex;
		int							m_nUV;
		float						m_fParams[ PrimitiveSurface::MAX_TEXTURE_PARAMS ];
	};

	// Data
	int						m_nPass;
	Blending				m_Blending;
	Color					m_Diffuse;
	Color					m_Ambient;
	Color					m_Emissive;
	Color					m_Specular;
	float					m_SpecularPower;
	bool					m_DoubleSided;
	bool					m_LightEnable;
	FilterMode				m_nFilterMode;

	bool					m_bUpdateShaders;
	CharString				m_sShader;
	Shader::Ref				m_pShader;

	Array< Surface >		m_Surfaces;
	Array< DevicePrimitive::Ref >
							m_Children;
	PrimitiveSetTransform::Ref	
							m_TopTransform;

	bool					m_bPushed;

	void					setupBlending();
	void					clearTextureStages();
	bool					setupTextureStatges();
	bool					executeChildren();

	// Helpers
	static D3DCOLORVALUE	makeD3DCOLORVALUE( const Color &src );
	static Color			makeColor( const D3DCOLORVALUE &src );
	static int				sortSurfaces( Surface p1, Surface p2 );
};

//----------------------------------------------------------------------------



#endif

//------------------------------------------------------------------------------------
// EOF

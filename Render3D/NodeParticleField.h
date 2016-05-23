/*
	NodeParticleField.h

	This node can be used to produce rain, snow, or star fields.. this type of particle system
	always remains the camera view

	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_PARTICLE_FIELD_H
#define NODE_PARTICLE_FIELD_H

#include "Render3D/BaseNode.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeParticleField : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Construction
	NodeParticleField();

	// Node interface
    void				render( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );
	// Accessors
	Material *			material() const;
	int					particleCount() const;
	float				scale() const;
	float				front() const;
	float				back() const;
	Vector3				velocity() const;

	// Mutators
	void				initialize( Material * pMaterial, int particles, float scale, 
							float front, float back, const Vector3 & velocity );
	void				update( float t );

protected:
	// Data
	Material::Link		m_Material;					// particle material
	int					m_ParticleCount;			// number of particles
	float				m_Scale;					// particle scale
	float				m_Front;
	float				m_Back;
	Vector3				m_Velocity;					// velocity of particles in worldspace

	// non-serialized
	Array< Vector3 >	m_Particles;				// worldspace positions of particles

	// Mutators
	void				createParticle( RenderContext & context, int n );

	// Helpers
	static float		randomFloat( float low, float high );
};

//----------------------------------------------------------------------------

inline Material * NodeParticleField::material() const
{
	return( m_Material );
}

inline int NodeParticleField::particleCount() const
{
	return( m_ParticleCount );
}

inline float NodeParticleField::scale() const
{
	return m_Scale;
}

inline float NodeParticleField::front() const
{
	return( m_Front );
}

inline float NodeParticleField::back() const
{
	return( m_Back );
}

inline Vector3 NodeParticleField::velocity() const
{
	return( m_Velocity );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

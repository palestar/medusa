/*
	NodeParticleSystem2.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_PARTICLE_SYSTEM2_H
#define NODE_PARTICLE_SYSTEM2_H

#include "Render3D/NodeTransform.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodeParticleSystem2 : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	struct Particle
	{
		Vector3		origin;							// starting position
		Vector3		velocity;						// particle velocity
		Vector3		acceleration;					// particle change in velocity over time
		float		life;							// life of particle
		float		scale;							// scale variation of particle
	};

	struct ParticleInstance
	{
		Vector3			m_vOrigin;
		Vector3			m_vVelocity;
		Vector3			m_vAcceleration;
		float			m_fLastTime;

		ParticleInstance() : 
			m_vOrigin( Vector3::ZERO ), 
			m_vVelocity( Vector3::ZERO ), 
			m_vAcceleration( Vector3::ZERO ),
			m_fLastTime( 1000000000.0f )
		{}
	};
	struct ParticleInstanceData : public RenderContext::InstanceData
	{
		DECLARE_WIDGET_CLASS();

		Array< ParticleInstance >
						m_Particles;
	};


	// Construction
	NodeParticleSystem2();

	//! Widget Interface
	bool				read( const InStream & );
	//! BaseNode interface
	virtual BoxHull		hull() const;
    virtual void		render( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );

	// Accessors
	int					particleCount() const;
	const Particle &	particle( int n ) const;
	Material *			material() const;

	// Mutators
	int					addParticle( const Particle & p );
	void				removeParticle( int n );

	void				setLife( float life );
	void				setReverseTime( bool reverseTime );
	void				setLocalSpace( bool a_bLocalSpace );
	void				setMaterial( Material * pMaterial );
	void				setVisible( float visible, float visibleV, float visibleA );
	void				setScale( float scale, float scaleV, float scaleA );
	void				setAlpha( float alpha, float alphaV, float alphaA );

private:
	//! Data
	Array< Particle >	m_Particles;

	float				m_Life;						// what is the life span of this system
	bool				m_ReverseTime;
	Material::Link		m_Material;					// particle material

	float				m_Visible;					// number of particles rendered, 0.0 - 1.0f
	float				m_VisibleV;					// change in visible particles over time
	float				m_VisibleA;					// change in velocity over time
	float				m_Scale;					// particle scale
	float				m_ScaleV;					// change in scale over time
	float				m_ScaleA;					// change in velocity over time
	float				m_Alpha;					// particle alpha, 0.0 - 1.0f
	float				m_AlphaV;					// change in alpha over time
	float				m_AlphaA;					// change in velocity over time
	bool				m_bLocalSpace;				// if true, then all particles are NOT emitted into world space
	BoxHull				m_Hull;		
};

IMPLEMENT_RAW_STREAMING( NodeParticleSystem2::Particle );

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF


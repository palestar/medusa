/*
	NodeParticleSystem.h

	This inherits from NodeTransform to give each NodeParticleSystem a position()
    and orientation (frame()) and to allow a particle system to be attached
    as a child to another NodeTransform in a Scene.

    The convention will be that a particle system, if it has direction (is not
    totally randomized), will emit directed particles along its own positive Z
    axis, with whatever spread is determined by the chaos in initial
    velocities.

	(c)2005 Palestar
*/

#ifndef NODE_PARTICLE_SYSTEM_H
#define NODE_PARTICLE_SYSTEM_H

#include "Display/PrimitiveTriangleList.h"
#include "Render3D/NodeTransform.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeParticleSystem : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< NodeParticleSystem >	        Ref;
	typedef ResourceLink< NodeParticleSystem >	    Link;

	enum Constants {
		NUM_PARTICLE_RANDOM_VALUES = 128,
		NUM_RANDOM_EMISSION_RATES = 16,
		MAX_PARTICLES = 1024,
		MIN_PARTICLES = 1,
	};

	struct Particle
	{
		Particle() : m_vPosition( Vector3::ZERO ),
			m_vVelocity( Vector3::ZERO ),
			m_fAge( 0.0f ),
			m_fEndAge( 0.0f ),
			m_fSolidity( 1.0f ),
			m_fBaseScale( 1.0f ),
			m_fScale( 1.0f ),
			m_Color( WHITE ),
			m_bAlive( false )
		{}

        Vector3     m_vPosition;
        Vector3     m_vVelocity;
        float       m_fAge;
        float       m_fEndAge;
        float       m_fSolidity;
		float		m_fBaseScale;
		float		m_fScale;
		Color		m_Color;
        bool        m_bAlive;
	};

	//! This structure holds all instance speicifc data..
	struct ParticleInstanceData : public RenderContext::InstanceData
	{
		DECLARE_WIDGET_CLASS();

		typedef Reference< ParticleInstanceData >	Ref;

		ParticleInstanceData() : 
			m_fTime( 0.0f ),
			m_pParticles( NULL ), 
			m_ParticleEmissionAccumulator( 0.0f ),
			m_nVisibleParticles( 0 )
		{}
		virtual ~ParticleInstanceData()
		{
			if ( m_pParticles )
				delete [] m_pParticles;
		}

		float				m_fTime;
	    Particle *			m_pParticles;
		float               m_ParticleEmissionAccumulator;
		int					m_nVisibleParticles;
	};

	// Data
	static float            sm_fDetail;
	static float			sm_fGravity;

	//! Construction
	NodeParticleSystem();
	virtual~NodeParticleSystem();

	//! Widget Interface
	bool					read( const InStream & );
	//! BaseNode interface
	virtual BoxHull			hull() const;

    virtual void			render( RenderContext &context, 
                                    const Matrix33 & frame, const Vector3 & position );

	// Accessors
	Type					type() const;
	Material *				material() const;

	// Mutators
	void					setMaterial( Material * pMaterial );
    void                    setDuration( float duration );
    void                    setContinuous( bool continuous );
	void					setLocalSpace( bool a_bLocalSpace );
    void                    setRadius( float radius );
    void                    setRadiusRand( float radius_rand );
    void                    setMaxNum( unsigned int max_num );                   
   
    void                    setSpeed( float speed );
    void                    setPosRadius( float pos_radius );
    void                    setPosRand( const Vector3 & pos_rand );
    void                    setSpeedRand( float speed_rand );
    void                    setDirRand( const Vector3 & dir_rand );

    void                    setRate( float rate );
    void                    setRateRand( float rate_rand );
    void                    setLifeRand( float life_rand );

    void                    setLife( float life );
    void                    setFadeInTime( float fade_in_time );
    void                    setFadeOutTime( float fade_out_time );

    void                    setMass( float mass );                 
    void                    setBouyant( float bouyant );
    void                    setWindAccel( const Vector3 & wind_accel );
    void                    setTerminalSpeed( float terminal_speed );

    void                    setFadeIn( bool fade_in );
    void                    setFadeOut( bool fade_out );
    void                    setScaleIn( bool scale_in );
    void                    setScaleOut( bool scale_out );

    void                    setRadialVelocity( bool radial_vel );

    void                    setInitVelocity( const Vector3 & init_vel );

    void                    setAnimateColor( bool animate_color );
    void                    setColorStart( const Color & color_start );
    void                    setColorEnd( const Color & color_end );

    void                    setCreateOnSurface( bool create_on_surface );
    void                    setBoxValues( const Vector3 & box_diagonal,
										  bool create_in_box,
										  bool live_in_box );

protected:
	//! Mutators
	ParticleInstanceData *	updateParticles( RenderContext & context,
								const Matrix33 & frame, const Vector3 & position );

	// Data
	Material::Link			m_Material;
	BoxHull					m_Hull;

    float                   m_SystemDuration;
    bool                    m_bContinuous;
	bool					m_bLocalSpace;
    float                   m_ParticleRadius;
    float                   m_percentRadiusRandomness;

    unsigned int            m_maxNumParticles;                   
    float                   m_MeanInitialSpeed;
    float                   m_InitialPositionMaxRadius;
    Vector3                 m_percentInitialPositionRandomness;
    float                   m_percentInitialSpeedRandomness;
    Vector3                 m_percentInitialDirectionRandomness;

    float                   m_ParticleEmissionRate;
    float                   m_percentEmissionRateRandomness;
    float                   m_ParticleLifeRandomness;

    float                   m_ParticleLifetime;
    float                   m_FadeInDuration;
    float                   m_FadeOutDuration;

    float                   m_percentRandomBrownianMotion;
    float                   m_ParticleMass;                 
    float                   m_BouyantForceMagnitude;
    Vector3                 m_WindAcceleration;

    bool                    m_bAlphaFadeIn;
    bool                    m_bAlphaFadeOut;
    bool                    m_bScaleIn;
    bool                    m_bScaleOut;

    bool                    m_bAnimateColor;
    Color                   m_ColorGradStart;
    Color                   m_ColorGradEnd;

    Vector3                 m_InitVelocity;

    bool                    m_bRadialVelocity;
    bool                    m_bDetectCollision;
    bool                    m_bElasticCollision;
    bool                    m_bEndOnCollision;
	bool					m_bCreateOnSurface;

	Vector3					m_BoxDiagonal;
	bool					m_bCreateInBox;
	bool					m_bLiveInBox;
};

//-------------------------------------------------------------------------------

inline Material * NodeParticleSystem::material() const
{
	return( m_Material );
}

//---------------------------------------------------------------------------------------------------

inline void NodeParticleSystem::setMaterial( Material * pMaterial )
{
	m_Material = pMaterial;
}

inline void NodeParticleSystem::setDuration( float duration )
{
	m_SystemDuration = duration;
}

inline void NodeParticleSystem::setContinuous( bool continuous )
{
	m_bContinuous = continuous;
}

inline void NodeParticleSystem::setLocalSpace( bool a_bLocalSpace )
{
	m_bLocalSpace = a_bLocalSpace;
}

inline void NodeParticleSystem::setRadius( float radius )
{
	m_ParticleRadius = radius;
}

inline void NodeParticleSystem::setRadiusRand( float radius_rand )
{
	m_percentRadiusRandomness = radius_rand;
}

inline void NodeParticleSystem::setMaxNum( unsigned int max_num )
{
	m_maxNumParticles = max_num;

	if ( m_maxNumParticles > MAX_PARTICLES )
		m_maxNumParticles = MAX_PARTICLES;
	else if ( m_maxNumParticles < MIN_PARTICLES )
		m_maxNumParticles = MIN_PARTICLES;
}

inline void NodeParticleSystem::setSpeed( float speed )
{
	m_MeanInitialSpeed = speed;
}

inline void NodeParticleSystem::setPosRadius( float pos_radius )
{
	m_InitialPositionMaxRadius = pos_radius;
}

inline void NodeParticleSystem::setPosRand( const Vector3 & pos_rand )
{
	m_percentInitialPositionRandomness = pos_rand;
}

inline void NodeParticleSystem::setSpeedRand( float speed_rand )
{	
	m_percentInitialSpeedRandomness = speed_rand;
}

inline void NodeParticleSystem::setDirRand( const Vector3 & dir_rand )
{
	m_percentInitialDirectionRandomness = dir_rand;
}

inline void NodeParticleSystem::setRate( float rate )
{
	m_ParticleEmissionRate = rate;
}

inline void NodeParticleSystem::setRateRand( float rate_rand )
{
	m_percentEmissionRateRandomness = rate_rand;
}

inline void NodeParticleSystem::setLifeRand( float life_rand )
{
	m_ParticleLifeRandomness = life_rand;
}

inline void NodeParticleSystem::setLife( float life )
{
	m_ParticleLifetime = life;
}

inline void NodeParticleSystem::setFadeInTime( float fade_in_time )
{
	m_FadeInDuration = fade_in_time;
}

inline void NodeParticleSystem::setFadeOutTime( float fade_out_time )
{
	m_FadeOutDuration = fade_out_time;
}

inline void NodeParticleSystem::setMass( float mass )
{
	m_ParticleMass = mass;
}

inline void NodeParticleSystem::setBouyant( float bouyant )
{
	m_BouyantForceMagnitude = bouyant;
}

inline void NodeParticleSystem::setWindAccel( const Vector3 & wind_accel )
{
	m_WindAcceleration = wind_accel;
}

inline void NodeParticleSystem::setFadeIn( bool fade_in )
{
	m_bAlphaFadeIn = fade_in;
}

inline void NodeParticleSystem::setFadeOut( bool fade_out )
{
	m_bAlphaFadeOut = fade_out;
}

inline void NodeParticleSystem::setScaleIn( bool scale_in )
{
	m_bScaleIn = scale_in;
}

inline void NodeParticleSystem::setScaleOut( bool scale_out )
{
	m_bScaleOut = scale_out;
}

inline void NodeParticleSystem::setRadialVelocity( bool radial_vel )
{
	m_bRadialVelocity = radial_vel;
}

inline void NodeParticleSystem::setInitVelocity( const Vector3 & init_vel )
{
	m_InitVelocity = init_vel;
	m_InitVelocity.normalize();
}

inline void NodeParticleSystem::setAnimateColor( bool animate_color )
{
    m_bAnimateColor = animate_color;
}

inline void NodeParticleSystem::setColorStart( const Color & color_start )
{
    m_ColorGradStart = color_start;
}

inline void NodeParticleSystem::setColorEnd( const Color & color_end )
{
    m_ColorGradEnd = color_end;
}

inline void NodeParticleSystem::setCreateOnSurface( bool create_on_surface )
{
	m_bCreateOnSurface = create_on_surface;
}

inline void NodeParticleSystem::setBoxValues( const Vector3 & box_diagonal,
									   bool create_in_box,
									   bool live_in_box )
{
	m_BoxDiagonal = box_diagonal;
	m_bCreateInBox = create_in_box;
	m_bLiveInBox = live_in_box;
}

#endif

//----------------------------------------------------------------------------
// EOF

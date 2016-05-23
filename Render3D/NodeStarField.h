/*
	NodeStarField.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef NODESTARFIELD_H
#define NODESTARFIELD_H

#include "Render3D/BaseNode.h"
#include "Display/PrimitiveTriangleList.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeStarField : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Construction
	NodeStarField();

	// Node interface
    void				render( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );
	// Accessors
	bool				active() const;
	bool				isTrailActive() const;
	bool				isJumpActive() const;
	int					particleCount() const;
	float				front() const;
	float				back() const;

	// Mutators
	void				setActive( bool bActive );
	void				setTrailActive( bool bTrailActive );
	void				setJumpActive( bool bDopplerActive );
	void				initialize(int particles, float front, float back );
	void				setParticleCount( int particles );

	// Static
	static Color		s_StarColor;
	static float		s_StarSize;
	static float		s_DopplerVelocity;

protected:
	// Types
	struct Star
	{
		Vector3			m_vHead;
		Vector3			m_vTail;
	};

	// Data
	int					m_ParticleCount;			// number of particles
	float				m_Front;
	float				m_Back;

	// non-serialized
	bool				m_bActive;
	bool				m_bTrailActive;
	bool				m_bJumpActive;
	Array< Star >		m_Stars;					
	float				m_LastTime;
	Vector3				m_LastPosition;

	// Mutators
	void				createParticle( RenderContext & context, int n );
};

//----------------------------------------------------------------------------

inline bool NodeStarField::active() const
{
	return m_bActive;
}

inline bool NodeStarField::isTrailActive() const
{
	return m_bTrailActive;
}

inline bool NodeStarField::isJumpActive() const
{
	return m_bJumpActive;
}

inline int NodeStarField::particleCount() const
{
	return( m_ParticleCount );
}

inline float NodeStarField::front() const
{
	return( m_Front );
}

inline float NodeStarField::back() const
{
	return( m_Back );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

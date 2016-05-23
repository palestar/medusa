/*
	NodeBoolean.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_BOOLEAN_H
#define NODE_BOOLEAN_H

#include "Render3D/BaseNode.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodeBoolean : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeBoolean>			Ref;

	enum DetailLevel
	{
		LOW			= 0,
		MEDIUM		= 1,
		HIGH		= 2
	};
	enum Type
	{
		AND,			// (a & b) != 0
		AND_ALL,		// (a & b) == a
		NOT,			// (a & b) == 0
		EQUAL,			// (a == b) != 0
		NOT_EQUAL,		// (a == b) == 0
	};

	// Construction
	NodeBoolean();

	// Node Interface
	void				preRender( RenderContext &context, const Matrix33 & frame, const Vector3 & position );

	// Accessors
	dword				bits() const;
	Type				type() const;
	float				minDistance() const;			// distance >= minDistance && distance <= maxDistance
	float				maxDistance() const;
	float				fadePercent() const;			// at what percent to the max distance do we begin fading alpha, default is 0.1 (10%)
	DetailLevel			minDetailLevel() const;			// detail >= minDetailLevel && detail <= maxDetailLevel
	DetailLevel			maxDetailLevel() const;
	bool				isZeroTime() const;

	// Mutators
	void				setBits( dword bits );
	void				setType( Type type );
	void				setDistance( float min, float max );
	void				setFadePercent( float a_Percent );
	void				setDetailLevel( DetailLevel min, DetailLevel max );
	void				setZeroTime( bool a_bZeroTime );

	// Static
	static DetailLevel	s_DetailLevel;					// global detail level

private:
	// Data
	dword				m_Bits;
	Type				m_Type;
	float				m_MinDistance, m_MaxDistance;
	float				m_fFadePercent;
	DetailLevel			m_MinDetailLevel, m_MaxDetailLevel;
	bool				m_bZeroTime;					// if true, then time will be 0.0f when we start rendering 
};

IMPLEMENT_RAW_STREAMING( NodeBoolean::Type );
IMPLEMENT_RAW_STREAMING( NodeBoolean::DetailLevel );

//-------------------------------------------------------------------------------

inline dword NodeBoolean::bits() const
{
	return( m_Bits );
}

inline NodeBoolean::Type NodeBoolean::type() const
{
	return( m_Type );
}

inline float NodeBoolean::minDistance() const
{
	return m_MinDistance;
}

inline float NodeBoolean::maxDistance() const
{
	return m_MaxDistance;
}

inline float NodeBoolean::fadePercent() const
{
	return m_fFadePercent;
}

inline NodeBoolean::DetailLevel NodeBoolean::minDetailLevel() const
{
	return m_MinDetailLevel;
}

inline NodeBoolean::DetailLevel NodeBoolean::maxDetailLevel() const
{
	return m_MaxDetailLevel;
}

inline bool NodeBoolean::isZeroTime() const
{
	return m_bZeroTime;
}

//---------------------------------------------------------------------------------------------------

inline void NodeBoolean::setBits( dword bits )
{
	m_Bits = bits;
}

inline void NodeBoolean::setType( Type type )
{
	m_Type = type;
}

inline void NodeBoolean::setDistance( float min, float max )
{
	if ( min < max )
	{
		m_MinDistance = min;
		m_MaxDistance = max;
	}
	else
	{
		m_MinDistance = max;
		m_MaxDistance = min;
	}
}

inline void NodeBoolean::setFadePercent( float a_fFadePercent )
{
	m_fFadePercent = Clamp<float>( a_fFadePercent, 0.0f, 1.0f );
}

inline void NodeBoolean::setDetailLevel( DetailLevel min, DetailLevel max )
{
	if ( min < max )
	{
		m_MinDetailLevel = min;
		m_MaxDetailLevel = max;
	}
	else
	{
		m_MinDetailLevel = max;
		m_MaxDetailLevel = min;
	}
}

inline void NodeBoolean::setZeroTime( bool a_bZeroTime )
{
	m_bZeroTime = a_bZeroTime;
}

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

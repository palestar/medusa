/*
	NodeLight.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_LIGHT_H
#define NODE_LIGHT_H

#include "Render3D/NodeTransform.h"
#include "Render3D/Render3dDll.h"

//-------------------------------------------------------------------------------

class DLL NodeLight : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeLight>			Ref;
	
	enum Type
	{
		DIRECTIONAL,			// D = frame.k
		POINT,					// P = position
		PARALLEL_POINT,			// P = position
		SPOT					// D = frame.k, P = position
	};
	enum Effect
	{
		NONE,
		GLOW,
		FLICKER,
		STROBE,
	};

	// Construction
	NodeLight();

	// NodeTransform Interface
	BoxHull				hull() const;
	void				render( RenderContext &context, const Matrix33 & frame, const Vector3 & position );

	// Accessors
	bool				disabled() const;
	Color				color() const;
	Type				type() const;
	float				distance() const;

	Effect				effect() const;
	float				effectInterval() const;

	Color				illuminate( const Vector3 & vertexPosition, const Vector3 & vertexNormal,
							const Matrix33 & frame, const Vector3 & position ) const;

	// Mutators
	void				setDisabled( bool a_bDisabled );
	void				setColor( Color color );
	void				setType( Type type );
	void				setDistance( float distance );
	void				setSpot( float theta, float phi );
	void				setEffect( Effect effect, float interval );

	// Static
	static bool			sm_bDisable;			// globally disable all lights
	static void			setDisabled( BaseNode * a_pNode, bool a_bDisabled );		// disable/enable all lights in the given heirarchy

private:
	// Data
	bool				m_bDisabled;
	Type				m_Type;
	Color				m_Color;
	float				m_Distance;
	float				m_Theta, m_Phi;
	Effect				m_Effect;
	float				m_EffectInterval;
};

IMPLEMENT_RAW_STREAMING( NodeLight::Type );
IMPLEMENT_RAW_STREAMING( NodeLight::Effect );

//----------------------------------------------------------------------------

inline bool	NodeLight::disabled() const
{
	return m_bDisabled;
}

inline Color NodeLight::color() const
{
	return( m_Color );
}

inline NodeLight::Type NodeLight::type() const
{
	return( m_Type );
}

inline float NodeLight::distance() const
{
	return( m_Distance );
}

inline NodeLight::Effect NodeLight::effect() const
{
	return( m_Effect );
}

inline float NodeLight::effectInterval() const
{
	return( m_EffectInterval );
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

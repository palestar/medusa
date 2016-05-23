/*
	NodeLight.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL

#include "Debug/Assert.h"
#include "Render3D/NodeLight.h"
#include "Math/Helpers.h"

//----------------------------------------------------------------------------

bool NodeLight::sm_bDisable = false;		// when true this object doesn't push it's light into the display device

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeLight, NodeTransform );
REGISTER_FACTORY_KEY(  NodeLight, 3964222246870077790LL );

BEGIN_PROPERTY_LIST( NodeLight, NodeTransform )
	ADD_PROPERTY( m_bDisabled );
	ADD_PROPERTY( m_Color );
	ADD_ENUM_PROPERTY( m_Type );
		ADD_ENUM_OPTION( m_Type, DIRECTIONAL );
		ADD_ENUM_OPTION( m_Type, POINT );
		ADD_ENUM_OPTION( m_Type, PARALLEL_POINT );
		ADD_ENUM_OPTION( m_Type, SPOT );
	ADD_PROPERTY( m_Distance );
	ADD_PROPERTY( m_Theta );
	ADD_PROPERTY( m_Phi );
	ADD_ENUM_PROPERTY( m_Effect );
		ADD_ENUM_OPTION( m_Effect, NONE );
		ADD_ENUM_OPTION( m_Effect, GLOW );
		ADD_ENUM_OPTION( m_Effect, FLICKER );
		ADD_ENUM_OPTION( m_Effect, STROBE );
	ADD_PROPERTY( m_EffectInterval );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeLight::NodeLight() : m_bDisabled( false ), m_Color(128,128,128), m_Type(DIRECTIONAL), m_Distance(100.0f)
{
	m_Effect = NodeLight::NONE;
	m_EffectInterval = 1.0f;
	m_Theta = 1.0f;
	m_Phi = 3.0f;
}

//-------------------------------------------------------------------------------

BoxHull NodeLight::hull() const
{
	static BoxHull sHull( Vector3(-1,-1,-1), Vector3(1,1,1) );
	return( sHull );
}

void NodeLight::render( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	if ( context.isShadowPass() )
		return;	

	NodeTransform::render( context, frame, position );

	if ( sm_bDisable || m_bDisabled )
		return;		// lights disabled, don't push the light

	Color lightColor( m_Color );
	switch( m_Effect )
	{
	case NONE:
		break;
	case GLOW:
		{
			float colorScale = fmod( context.time(), m_EffectInterval * 2.0f );
			if ( colorScale > m_EffectInterval )
				colorScale = (m_EffectInterval * 2.0f) - colorScale;

			lightColor = lightColor * colorScale;
		}
		break;
	case FLICKER:
		lightColor = lightColor * RandomFloat( 0.0f, 1.0f );
		break;
	case STROBE:
		if ( fmod( context.time(), m_EffectInterval * 2.0f) > m_EffectInterval )
			lightColor = Color(0,0,0,0);
		break;
	}

	DisplayDevice * pDisplay = context.display();
	switch( m_Type )
	{
		case DIRECTIONAL:
			pDisplay->addDirectionalLight( 0x7fffffff, lightColor, frame.k );
			break;
		case POINT:
		case PARALLEL_POINT:
			{
				int nPriority = Clamp<int>( m_Distance - context.worldToView( position ).magnitude(), INT_MIN, INT_MAX );
				pDisplay->addPointLight( nPriority,	lightColor, position, m_Distance );
			}
			break;
		case SPOT:
			// not supported currently..
			break;
	}
}

//-------------------------------------------------------------------------------

Color NodeLight::illuminate( const Vector3 & vertexPosition, const Vector3 & vertexNormal,
				const Matrix33 & frame, const Vector3 & position ) const
{
	switch( m_Type )
	{
	case SPOT:
	case DIRECTIONAL:
		return( m_Color * Max(-frame.k | vertexNormal,0.0f) );
	case POINT:
		return( m_Color * Max( (position - vertexPosition).normalize() | vertexNormal,0.0f) );
		break;
	}

	return( Color(0,0,0,0) );
}

//-------------------------------------------------------------------------------

void NodeLight::setDisabled( bool a_bDisabled )
{
	m_bDisabled = a_bDisabled;
}

void NodeLight::setColor( Color color )
{
	m_Color = color;
}

void NodeLight::setType( Type type )
{
	m_Type = type;
}

void NodeLight::setDistance( float distance )
{
	m_Distance = distance;
}

void NodeLight::setSpot( float theta, float phi )
{
	ASSERT( theta <= phi );
	m_Theta = theta;
	m_Phi = phi;
}

void NodeLight::setEffect( Effect effect, float interval )
{
	m_Effect = effect;
	m_EffectInterval = interval;
	if ( m_EffectInterval == 0.0f )
		m_EffectInterval = 1.0f;
}

void NodeLight::setDisabled( BaseNode * a_pNode, bool a_bDisabled )
{
	if ( WidgetCast<NodeLight>( a_pNode ) )
		((NodeLight *)a_pNode)->setDisabled( a_bDisabled );

	for(int i=0;i<a_pNode->childCount();++i)
		setDisabled( a_pNode->child(i), a_bDisabled );
}

//----------------------------------------------------------------------------
// EOF

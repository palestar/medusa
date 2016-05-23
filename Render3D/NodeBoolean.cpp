/*
	NodeBoolean.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Standard/Bits.h"
#include "Render3D/Scene.h"
#include "Render3D/NodeBoolean.h"

//----------------------------------------------------------------------------

// default global detail to high
NodeBoolean::DetailLevel	NodeBoolean::s_DetailLevel = NodeBoolean::MEDIUM;	

//---------------------------------------------------------------------------------------------------

struct ZeroTimeData : public RenderContext::InstanceData
{
	DECLARE_WIDGET_CLASS();

	ZeroTimeData() : m_bInitialized( false ), m_fBaseTime( 0.0f )
	{}

	bool				m_bInitialized;
	float				m_fBaseTime;
};

IMPLEMENT_LIGHT_FACTORY( ZeroTimeData, RenderContext::InstanceData );

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeBoolean, BaseNode );
REGISTER_FACTORY_KEY(  NodeBoolean, 3932631853185873148LL );

BEGIN_PROPERTY_LIST( NodeBoolean, BaseNode )
	ADD_PROPERTY( m_Bits )
	ADD_ENUM_PROPERTY( m_Type )
		ADD_ENUM_OPTION( m_Type, AND );
		ADD_ENUM_OPTION( m_Type, AND_ALL );
		ADD_ENUM_OPTION( m_Type, NOT );
		ADD_ENUM_OPTION( m_Type, EQUAL );
		ADD_ENUM_OPTION( m_Type, NOT_EQUAL );
	ADD_PROPERTY( m_MinDistance );
	ADD_PROPERTY( m_MaxDistance );
	ADD_PROPERTY( m_fFadePercent );
	ADD_ENUM_PROPERTY( m_MinDetailLevel );
		ADD_ENUM_OPTION( m_MinDetailLevel, LOW );
		ADD_ENUM_OPTION( m_MinDetailLevel, MEDIUM );
		ADD_ENUM_OPTION( m_MinDetailLevel, HIGH );
	ADD_ENUM_PROPERTY( m_MaxDetailLevel )
		ADD_ENUM_OPTION( m_MaxDetailLevel, LOW );
		ADD_ENUM_OPTION( m_MaxDetailLevel, MEDIUM );
		ADD_ENUM_OPTION( m_MaxDetailLevel, HIGH );
	ADD_PROPERTY( m_bZeroTime );
END_PROPERTY_LIST()

//----------------------------------------------------------------------------

NodeBoolean::NodeBoolean() :
	m_Bits( 0 ),
	m_Type( AND ),
	m_MinDistance( 0.0f ),
	m_MaxDistance( 32768.0f ),
	m_MinDetailLevel( LOW ),
	m_MaxDetailLevel( HIGH ),
	m_fFadePercent( 0.1f ),
	m_bZeroTime( false )
{}

//-------------------------------------------------------------------------------

void NodeBoolean::preRender( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	float fScale = frame.scale();
	if ( fScale <= 0.00001f )
		return;
	Vector3 positionVS( context.worldToView( position ) );
	float fDistance = positionVS.magnitude() / fScale;
	if (! context.isShadowPass() && (fDistance < m_MinDistance || fDistance > m_MaxDistance) )
		return;
	if ( s_DetailLevel < m_MinDetailLevel || s_DetailLevel > m_MaxDetailLevel )
		return;

	dword bits = context.bits();

	bool bCondition = false;
	switch( m_Type )
	{
	case AND:
		bCondition = (bits & m_Bits) != 0;
		break;
	case AND_ALL:
		bCondition = (bits & m_Bits) == m_Bits;
		break;
	case NOT:
		bCondition = (bits & m_Bits) == 0;
		break;
	case EQUAL:
		bCondition = (bits == m_Bits);
		break;
	case NOT_EQUAL:
		bCondition = bits != m_Bits;
		break;
	}

	// call base class, which renders children of this node
	if ( bCondition )
	{
		float fTime = context.time();

		if ( m_bZeroTime )
		{
			ZeroTimeData * pData = context.instanceData<ZeroTimeData>();
			if (! pData->m_bInitialized || fTime < pData->m_fBaseTime )
			{
				pData->m_bInitialized = true;
				pData->m_fBaseTime = fTime;
				context.setTime( 0.0f );
			}
			else if ( fTime > pData->m_fBaseTime )
				context.setTime( fTime - pData->m_fBaseTime );
		}

		if ( !context.isShadowPass() && m_fFadePercent > 0.0f && m_MaxDistance > 0.0f )
		{
			float fFadeDistance = m_MaxDistance * m_fFadePercent;
			float fBeginFade = m_MaxDistance - fFadeDistance;
			if ( fDistance > fBeginFade )
			{
				float fContextAlpha = context.alpha();
				context.setAlpha( fContextAlpha * (1.0f - ((fDistance - fBeginFade) / fFadeDistance)) );
				BaseNode::preRender( context, frame, position );
				context.setAlpha( fContextAlpha );

				bCondition = false;
			}
		}

		if ( bCondition )
			BaseNode::preRender( context, frame, position );

		if ( m_bZeroTime )
			context.setTime( fTime );		// restore the time value
	}
	else if ( m_bZeroTime )		// && !bCondition
	{
		// reset our instance data if we are not rendering our children..
		ZeroTimeData * pData = context.instanceData<ZeroTimeData>();
		pData->m_bInitialized = false;
	}
}

//----------------------------------------------------------------------------
// EOF

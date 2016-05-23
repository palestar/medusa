/*
	NounContext.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Standard/StringHash.h"
#include "NounContext.h"
#include "WorldContext.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_NAMED_WIDGET_TYPES( TYPES_NounContextData, NounContext::Data );
IMPLEMENT_NAMED_FACTORY( FACTORY_NounContextData, NounContext::Data, Widget, true );

BEGIN_NAMED_PROPERTY_LIST( PLIST_NounContextData, NounContext::Data, Widget );
	ADD_PROPERTY( m_sID );
END_PROPERTY_LIST();

IMPLEMENT_NAMED_WIDGET_TYPES( TYPES_NounContextResourceData, NounContext::ResourceData );
IMPLEMENT_NAMED_FACTORY( FACTORY_NounContextResourceData, NounContext::ResourceData, NounContext::Data, true );

BEGIN_NAMED_PROPERTY_LIST( PLIST_NounContextResourceData, NounContext::ResourceData, NounContext::Data );
	ADD_PORT_PROPERTY( m_sPort, Port );
END_PROPERTY_LIST();


//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NounContext, Resource );
REGISTER_FACTORY_KEY( NounContext, 4283348678626169331LL );

BEGIN_PROPERTY_LIST( NounContext, Resource )
	ADD_PROPERTY( m_Type );
	ADD_PROPERTY( m_Name );
	ADD_PROPERTY( m_Description );
	ADD_PROPERTY( m_Scene );
	ADD_PROPERTY( m_Hull );
		ADD_PROPERTY_FLAGS( m_Hull, PF_NOEDIT );		// don't let the user edit the hull
	ADD_PROPERTY( m_Mass );
	ADD_PROPERTY( m_Flags );
	ADD_PROPERTY( m_Data );
END_PROPERTY_LIST();

NounContext::NounContext()
	: m_Type( CLASS_KEY(Noun) ), m_Mass( 1.0f ), m_Hull( true ), m_Flags( 0 )
{}

NounContext::NounContext( Scene * pScene ) 
	: m_Type( CLASS_KEY(Noun) ), m_Mass( 1.0f ), 
	m_Hull( pScene != NULL ? pScene->calculateHull() : BoxHull( true ) ), m_Flags( 0 )
{
	setScene( pScene );
}

NounContext::NounContext( bool isStatic ) 
	: m_Type( CLASS_KEY(Noun) ), m_Mass( 1.0f ), m_Hull( true ), m_Flags( 0 )
{
	if ( isStatic )
		grabReference();		// don't let this object ever get deleted
}

//----------------------------------------------------------------------------

Noun * NounContext::createNoun() const
{
	Widget * pWidget = Factory::createWidget( m_Type );
	if ( pWidget )
	{
		if ( WidgetCast<Noun>( pWidget ) )
		{
			((Noun *)pWidget)->setNounContext( const_cast<NounContext *>( this ) );
			return (Noun *)pWidget;
		}
		delete pWidget;
	}

	return NULL;
}

//----------------------------------------------------------------------------

void NounContext::addData( Data * a_pData )
{
	m_Data.push( a_pData );
}

//---------------------------------------------------------------------------------------------------

IMPLEMENT_NAMED_WIDGET_TYPES( TYPES_NounContextDrop, NounContext::Drop );
IMPLEMENT_NAMED_FACTORY( FACTORY_NounContextDrop, NounContext::Drop, Widget, true );

BEGIN_NAMED_PROPERTY_LIST( PLIST_NounContextDrop, NounContext::Drop, Widget );
	ADD_CLASSKEY_PROPERTY( m_nDropClassKey, NounEnhancement );
	ADD_PROPERTY( m_nBaseAmount );
	ADD_PROPERTY( m_nRandomAmount );
	ADD_PROPERTY( m_nChance );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------
//EOF

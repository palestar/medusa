/**
	@file InterfaceContextScript.cpp

	(c)2012 Palestar Inc
	@author Richard Lyle @date 4/8/2012 3:03:21 PM
*/

#include "WindowText.h"
#include "WindowButton.h"
#include "NodeWindow.h"
#include "InterfaceContext.h"

//---------------------------------------------------------------------------------------------------

#include "World/LuaHeaders.h"
#ifdef USE_PLAIN_LUA
	#ifdef _DEBUG
		#pragma comment( lib, "../../Medusa/ThirdParty/Lua51/bin/Lua51D.lib")
	#else
		#pragma comment( lib, "../../Medusa/ThirdParty/Lua51/bin/Lua51.lib")
	#endif
#else
	#include "../../Medusa/ThirdParty/LuaJIT/jit/opt.h"
	#include "../../Medusa/ThirdParty/LuaJIT/jit/opt_inline.h"

	#ifdef _DEBUG
		#pragma comment( lib, "../../Medusa/ThirdParty/LuaJIT/bin/Lua51D.lib")
	#else
		#pragma comment( lib, "../../Medusa/ThirdParty/LuaJIT/bin/Lua51.lib")
	#endif
#endif

//---------------------------------------------------------------------------------------------------

void InterfaceContext::registerFunctions( lua_State * pScript )
{
	lua_register(pScript, "messageBox", messageBox );
	lua_register(pScript, "highlightWindow", highlightWindow );
	lua_register(pScript, "hideWindow", hideWindow );
	lua_register(pScript, "showWindow", showWindow );
	lua_register(pScript, "disableWindow", disableWindow );
	lua_register(pScript, "enableWindow", enableWindow );
}

//----------------------------------------------------------------------------

int InterfaceContext::messageBox( lua_State * pScript )
{
	int n = lua_gettop( pScript );

	const char * pText = n >= 1 ? lua_tostring( pScript, 1 ) : "" ;
	const char * pOkCall = n >= 2 ? lua_tostring( pScript, 2 ) : "";
	const char * pCancelCall = n >= 3 ? lua_tostring( pScript, 3 ) : "";

	WindowText * pMessageBox = WidgetCast<WindowText>( InterfaceContext::findNode( "MessageBox" ) );
	if ( pMessageBox == NULL )
	{
		WorldContext::Alert( pScript, "messageBox() failed to find window!");
		return 0;
	}

	WindowButton * pButtonOK = WidgetCast<WindowButton>( InterfaceContext::findNode( "MessageBoxOK" ) );
	if ( pButtonOK == NULL )
	{
		WorldContext::Alert( pScript, "messageBox() failed to find OK button!" );
		return 0;
	}

	WindowButton * pButtonCancel = WidgetCast<WindowButton>( InterfaceContext::findNode( "MessageBoxCANCEL" ) );
	if ( pButtonCancel == NULL )
	{
		WorldContext::Alert( pScript, "messageBox() failed to find cancel button!" );
		return 0;
	}

	if ( strlen( pText ) > 0 )
	{
		// show the message box and set the text
		pMessageBox->showWindow();
		pMessageBox->setText( pText );

		// enable/disable OK button
		if ( strlen( pOkCall ) > 0 )
		{
			pButtonOK->showWindow();
			pButtonOK->setScript( pOkCall );
		}
		else
			pButtonOK->hideWindow();

		// enable/disable cancel button
		if ( strlen( pCancelCall ) > 0 )
		{
			pButtonCancel->showWindow();
			pButtonCancel->setScript( pCancelCall );
		}
		else
			pButtonCancel->hideWindow();
	}
	else
		pMessageBox->hideWindow();

	return 0;
}

int InterfaceContext::highlightWindow( lua_State * pScript )
{
	int n = lua_gettop( pScript );
	if ( n == 1 && lua_isstring( pScript, 1 ) )
	{
		const char * pName = lua_tostring( pScript, 1 );

		BaseNode * pWindow = InterfaceContext::findNode( CharString(pName) );
		if ( pWindow == NULL )
		{
			WorldContext::Alert( pScript, CharString().format("highlightWindow() error! %s not found", pName) );
			return 0;
		}

		BaseNode::s_ShowSelected = true;
		BaseNode::s_pSelectedNode = pWindow;
	}
	else
		BaseNode::s_ShowSelected = false;

	return 0;
}

int InterfaceContext::hideWindow( lua_State * pScript )
{
	int n = lua_gettop( pScript );
	if ( n == 1 && lua_isstring( pScript, 1 ) )
	{
		const char * pName = lua_tostring( pScript, 1 );

		NodeWindow * pWindow = WidgetCast<NodeWindow>( InterfaceContext::findNode( CharString(pName) ) );
		if ( pWindow == NULL )
		{
			WorldContext::Alert( pScript, CharString().format("hideWindow() error! %s not found", pName) );
			return 0;
		}

		pWindow->hideWindow();
	}

	return 0;
}

int InterfaceContext::showWindow( lua_State * pScript )
{
	int n = lua_gettop( pScript );
	if ( n == 1 && lua_isstring( pScript, 1 ) )
	{
		const char * pName = lua_tostring( pScript, 1 );

		NodeWindow * pWindow = WidgetCast<NodeWindow>( InterfaceContext::findNode( CharString(pName) ) );
		if ( pWindow == NULL )
		{
			WorldContext::Alert( pScript, CharString().format("showWindow() error! %s not found", pName) );
			return 0;
		}

		pWindow->showWindow();
	}

	return 0;
}

int InterfaceContext::disableWindow( lua_State * pScript )
{
	int n = lua_gettop( pScript );
	if ( n == 1 && lua_isstring( pScript, 1 ) )
	{
		const char * pName = lua_tostring( pScript, 1 );

		NodeWindow * pWindow = WidgetCast<NodeWindow>( InterfaceContext::findNode( CharString(pName) ) );
		if ( pWindow == NULL )
		{
			WorldContext::Alert( pScript, CharString().format("disableWindow() error! %s not found", pName) );
			return 0;
		}

		pWindow->setEnable( false );
	}

	return 0;
}


int InterfaceContext::enableWindow( lua_State * pScript )
{
	int n = lua_gettop( pScript );
	if ( n == 1 && lua_isstring( pScript, 1 ) )
	{
		const char * pName = lua_tostring( pScript, 1 );

		NodeWindow * pWindow = WidgetCast<NodeWindow>( InterfaceContext::findNode( CharString(pName) ) );
		if ( pWindow == NULL )
		{
			WorldContext::Alert( pScript, CharString().format("enableWindow() error! %s not found", pName) );
			return 0;
		}

		pWindow->setEnable( true );
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------
//EOF

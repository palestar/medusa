/*
	WorldContext.cpp

	This object is used to hold the the worldContext state
	(c)1999 Palestar Development, Richard Lyle
*/

//#define PROFILE_OFF		// turn off profiling for this object

#include "Debug/Assert.h"
#include "Math/Helpers.h"
#include "System/CommandTarget.h"
#include "Noun.h"
#include "NounZoneTarget.h"
#include "VerbCallScript.h"
#include "VerbChat.h"
#include "WorldContext.h"
#include "Standard/StringBuffer.h"
#include "Standard/Queue.h"

#include "LuaHeaders.h"
//#include "ThirdParty/LuaJIT/src/lua.hpp"
#ifdef USE_PLAIN_LUA
	#ifdef _DEBUG
		#pragma comment( lib, "../../Medusa/ThirdParty/Lua51/bin/Lua51D.lib")
	#else
		#pragma comment( lib, "../../Medusa/ThirdParty/Lua51/bin/Lua51.lib")
	#endif
#else
	#include "../../Medusa/ThirdParty/LuaJIT/src/jit/opt.h"
	#include "../../Medusa/ThirdParty/LuaJIT/src/jit/opt_inline.h"

	#ifdef _DEBUG
		#pragma comment( lib, "../../Medusa/ThirdParty/LuaJIT/bin/Lua51D.lib")
	#else
		#pragma comment( lib, "../../Medusa/ThirdParty/LuaJIT/bin/Lua51.lib")
	#endif
#endif

//---------------------------------------------------------------------------------------------------

const UniqueNumber NULL_SCRIPT( (qword)0 );

//---------------------------------------------------------------------------------------------------

static bool s_AlertsEnabled = false;

// ScriptTimer object for script timers...
class ScriptTimer
{
public:
	// Construction
	ScriptTimer( lua_State * pScript, dword time, const char * pCall, const Array< CharString > & arguments )
		: m_pScript( pScript ), m_Time( time ), m_Call( pCall ), m_Arguments( arguments )
	{
		s_Timers.insert( this );
	}
	~ScriptTimer()
	{
		s_Timers.remove( this );
	}

	// Static
	static void		updateTimers()
	{
		List<ScriptTimer *>::Iterator it = s_Timers.head();
		while( it.valid() )
		{
			ScriptTimer * pTimer = (*it);
			it.next();

			pTimer->m_Time--;
			if ( pTimer->m_Time <= 0 )
			{
				int result;
				if ( pTimer->m_Arguments.size() > 0 )	// functionname with parameters or codesnippet ?
					result = WorldContext::DoCall( pTimer->m_pScript, pTimer->m_Call, pTimer->m_Arguments );
				else
					result = WorldContext::DoString( pTimer->m_pScript, pTimer->m_Call );

				if( result != 0 )
				{
					WorldContext * pContext = WorldContext::GetContext( pTimer->m_pScript );
					if( pContext )
						pContext->pushChat( CharString("Error running script from timer: ") + WorldContext::TranslateLuaError( result ) );
				}

				delete pTimer;	// timers are are one-time usage only
			}
		}
	}
	static void		deleteAllTimers( lua_State * pScript = NULL )
	{
		List<ScriptTimer *>::Iterator it = s_Timers.head();
		while( it.valid() )
		{
			ScriptTimer * pTimer = (*it);
			it.next();
			
			if ( pScript == NULL || pScript == pTimer->m_pScript )
				delete pTimer;
		}
	}
	static int		timerCount( lua_State * pScript )
	{
		int count = 0;

		List<ScriptTimer *>::Iterator it = s_Timers.head();
		while( it.valid() )
		{
			ScriptTimer * pTimer = (*it);
			it.next();
			
			if ( pTimer->m_pScript == pScript )
				count++;
		}

		return count;
	}

private:
	// Data
	lua_State *	m_pScript;
	int			m_Time;
	CharString	m_Call;

	Array< CharString >
				m_Arguments;

	// Static
	static List< ScriptTimer * > 
				s_Timers;
};

// declare the static timer list
List< ScriptTimer * > ScriptTimer::s_Timers;

//---------------------------------------------------------------------------------------------------

class KeyHandler : public CommandTarget		// this class is used by installKey() to hook a key to a function in the script
{
public:
	// Construction
	KeyHandler( lua_State * pScript, dword key, const char * pCall, bool trap ) 
		: m_Key( key ), m_pScript( pScript ), m_Call( pCall ), m_Trap( trap )
	{
		s_Keys.insert( this );
	}
	~KeyHandler()
	{
		s_Keys.remove( this );
	}

	// CommandTarget interface
	bool onMessage( const Message & msg )
	{
		if ( (msg.message == HM_KEYDOWN || msg.message == HM_KEYUP || msg.message == HM_CHAR) && msg.wparam == m_Key )
		{
			// either key down or key up, call function only on key up
			if ( msg.message == HM_KEYUP )
			{
				// move some data to local vars, since the script call may delete this object
				lua_State * pScript = m_pScript;
				bool		trap = m_Trap;

				int result = WorldContext::DoString( pScript, m_Call );
				if( result != 0 )
				{
					WorldContext * pContext = WorldContext::GetContext( pScript );
					if( pContext )
						pContext->pushChat( CharString("Error running script from KeyHandler: ") + WorldContext::TranslateLuaError( result ) ); 
				}

				return trap;
			}

			return m_Trap;
		}

		// nope, not our message, pass it onto the next handler
		return false;
	}

	static void installKey( lua_State * pScript, dword key, const char * pCall, bool bTrap )
	{
		//TRACE( CharString().format("KeyHandler, key = %u, call = %s", key, pCall) );

		List<KeyHandler *>::Iterator it = s_Keys.head();
		while( it.valid() )
		{
			KeyHandler * pKey = (*it);
			it.next();

			if ( pKey->m_Key == key )
			{
				// already have a handler, update this handler
				pKey->m_pScript = pScript;
				pKey->m_Call = pCall;
				pKey->m_Trap = bTrap;
				return;
			}
		}

		// no existing handler found, create a new handler
		new KeyHandler( pScript, key, pCall, bTrap );
	}

	static void deleteKey( dword key )
	{
		List<KeyHandler *>::Iterator it = s_Keys.head();
		while( it.valid() )
		{
			KeyHandler * pKey = (*it);
			it.next();

			if ( pKey->m_Key == key )
				delete pKey;
		}
	}

	static void deleteAllKeys( lua_State * pScript = NULL )
	{
		List<KeyHandler *>::Iterator it = s_Keys.head();
		while( it.valid() )
		{
			KeyHandler * pKey = (*it);
			it.next();
			
			if ( pScript == NULL || pKey->m_pScript == pScript )
				delete pKey;
		}
	}

	static int keyCount( lua_State * pScript )
	{
		int count = 0;

		List<KeyHandler *>::Iterator it = s_Keys.head();
		while( it.valid() )
		{
			KeyHandler * pKey = (*it);
			it.next();
			
			if ( pKey->m_pScript == pScript )
				count++;
		}

		return count;
	}

private:
	// Data
	lua_State *	m_pScript;
	dword		m_Key;
	CharString		m_Call;
	bool		m_Trap;

	// Static
	static List< KeyHandler * > 
				s_Keys;
};

// declare the static timer list
List< KeyHandler * > KeyHandler::s_Keys;

//---------------------------------------------------------------------------------------------------

void WorldContext::registerFunctions( lua_State * pScript )
{
	// General or Noun
	lua_register(pScript, "scriptAlert", ScriptAlert );
	lua_register(pScript, "enableAlerts", enableAlerts );
	lua_register(pScript, "enableTrace", enableTrace );
	lua_register(pScript, "randomNumber", randomNumber );
	lua_register(pScript, "isClient", isClient);
	lua_register(pScript, "isServer", isServer);
	lua_register(pScript, "teamCount", teamCount);
	lua_register(pScript, "teamId", teamId );
	lua_register(pScript, "getTeamFaction", getTeamFaction);
	lua_register(pScript, "getTeamName", getTeamName);
	lua_register(pScript, "getName", getName);
	lua_register(pScript, "getKey", getKey);
	lua_register(pScript, "getClass", getClass);
	lua_register(pScript, "nodeCount", nodeCount);
	lua_register(pScript, "getNode", getNode);
	lua_register(pScript, "isNoun", isNoun );
	lua_register(pScript, "getTeamId", getTeamId );
	lua_register(pScript, "setTeamId", setTeamId );
	lua_register(pScript, "isFaction", isFaction );
	lua_register(pScript, "getFactionTeam", getFactionTeam );
	lua_register(pScript, "getDistance", getDistance );
	lua_register(pScript, "getPosition", getPosition );
	lua_register(pScript, "setPosition", setPosition );
	lua_register(pScript, "getFaction", getFaction );
	lua_register(pScript, "createSpawn", createSpawn );
	lua_register(pScript, "remoteCall", remoteCall );
	lua_register(pScript, "spawnNoun", spawnNoun );
	lua_register(pScript, "detachNoun", detachNoun );
	lua_register(pScript, "startTimer", startTimer );
	lua_register(pScript, "getRadius", getRadius );
	lua_register(pScript, "getUserId", getUserId );
	lua_register(pScript, "getConstants", getConstants );

	// Interface
	lua_register(pScript, "flushChat", flushChat);
	lua_register(pScript, "pushChat", pushChat);
	lua_register(pScript, "sendChat", sendChat);
	lua_register(pScript, "trapKey", trapKey );
	lua_register(pScript, "watchKey", watchKey );

	for( ScriptHook::HookList::iterator iHook = ScriptHook::GetHookList().begin(); 
		iHook != ScriptHook::GetHookList().end(); ++iHook )
	{
		(*iHook)->registerFunctions( pScript );
	}
}

// Data for loadLuaLibs
static const luaL_Reg lualibsDS[] = {
	{"", luaopen_base},
	{LUA_TABLIBNAME, luaopen_table},
	{LUA_STRLIBNAME, luaopen_string},
	{LUA_MATHLIBNAME, luaopen_math},
	{LUA_LOADLIBNAME, luaopen_package},
#ifndef USE_PLAIN_LUA
	{LUA_JITLIBNAME, luaopen_jit},
#endif
	{NULL, NULL}
};

// Not called directly, but by Lua itself
int WorldContext::loadLuaLibs( lua_State * pScript )
{
	const luaL_Reg *lib = lualibsDS;
	for (; lib->func; lib++)
	{
		lua_pushcfunction(pScript, lib->func);
		lua_pushstring(pScript, lib->name);
		lua_call(pScript, 1, 0);
	}
	return 0;
}

// Lua code optimizer. Only used if LuaJIT is used
void WorldContext::ActivateJITOpt( lua_State * pScript )
{
	// Load the JIT optimizer
	lua_getglobal(pScript, "package");
	lua_pushliteral(pScript, "preload");
	lua_gettable(pScript, -2);

	lua_pushliteral(pScript, "jit.opt");
	luaL_loadbuffer( pScript,
		(const char*)LuaJITOptBin, sizeof(LuaJITOptBin), "jit.opt");
	lua_settable(pScript, -3);

	lua_pushliteral(pScript, "jit.opt_inline");
	luaL_loadbuffer( pScript,
		(const char*)LuaJITOptBinInline, sizeof(LuaJITOptBinInline), "jit.opt_inline");
	lua_settable(pScript, -3);

	lua_pop(pScript, 2);

	// Activate the JIT optimizer
	lua_getglobal(pScript, "require");
	lua_pushliteral(pScript, "jit.opt");
	lua_pcall(pScript, 1, 1, 0);
	lua_getfield(pScript, -1, "start");
	lua_remove(pScript, -2);
	lua_pcall(pScript, 0, 0, 0);
}


//----------------------------------------------------------------------------

bool WorldContext::scriptCall( const char * pCall )
{
	return scriptCall( (qword)0, pCall, false );
}

bool WorldContext::scriptCall( UniqueNumber nScriptId, const char * pCall, bool bAllowNewContext /*= false*/ )
{
	int nCount = 0;
	for(int i=0;i<m_Scripts.size();i++)
	{
		ScriptContext & context = m_Scripts[i];
		if ( context.nScriptId == nScriptId || nScriptId == NULL_SCRIPT )	// must match the script or be broadcast to execute
		{
			int result = DoString( context.pScript, pCall );
			if( result != 0 && s_AlertsEnabled )				// prevents errorspam for broadcasted remotecalls
				pushChat( CharString("Error executing scriptCall: ") + TranslateLuaError( result ) );
			
			if( nScriptId != NULL_SCRIPT )
				return true;		// done, return now

			nCount++;
		}
	}

	if( nScriptId == NULL_SCRIPT && nCount == 0 && bAllowNewContext )				// broadcast but no script running ?
		return runScript( nScriptId, "Default", pCall );
	
	return nCount > 0;
}

bool WorldContext::runScript( UniqueNumber nScriptId, const char * pName, const char * pCall )
{
	// kill any script that is already using this ID
	killScript( nScriptId );

	ScriptContext & context = m_Scripts.push();
	context.nScriptId = nScriptId;
	context.sName = pName;

	// allocate the script state
	context.pScript = lua_open();
	// open standard libs
	lua_cpcall( context.pScript, &WorldContext::loadLuaLibs, NULL );

#ifndef USE_PLAIN_LUA
	WorldContext::ActivateJITOpt(context.pScript);
#endif

	// register our functions
	registerFunctions( context.pScript );
	// set the context
	lua_pushlightuserdata( context.pScript, this );
	lua_setglobal( context.pScript, "context" );

	ResetExecutionDepth( context.pScript );

	int result = DoString( context.pScript, pCall );
	if( result != 0 )
		pushChat( CharString("Error running script: ") + TranslateLuaError( result ) );

	scriptCall( context.nScriptId, "onInitialize()" );

	return true;
}

bool WorldContext::killScript( UniqueNumber nScriptId )
{
	for(int i=0;i<m_Scripts.size();i++)
	{
		ScriptContext & context = m_Scripts[i];
		if ( context.nScriptId == nScriptId )
		{
			scriptCall( nScriptId, "onRelease()" );

			// delete any timers assigned to this script
			ScriptTimer::deleteAllTimers( context.pScript );
			// delete any key handlers
			KeyHandler::deleteAllKeys( context.pScript );
			// delete the LUA state object
			lua_close( context.pScript );

			m_Scripts.remove( i );
			return true;
		}
	}

	// script not found
	return false;
}

bool WorldContext::findScriptId( lua_State * pScript, UniqueNumber & nScriptId )
{
	for(int i=0;i<m_Scripts.size();i++)
	{
		ScriptContext & context = m_Scripts[i];
		if ( context.pScript == pScript )
		{
			nScriptId = context.nScriptId;
			return true;
		}
	}

	return false;
}

CharString WorldContext::scriptStatus()
{
	CharString sStatus;
	sStatus.format( "%d Active Scripts\n", m_Scripts.size());

	for(int i=0;i<m_Scripts.size();i++)
	{
		ScriptContext & context = m_Scripts[i];

		sStatus += CharString().format( "%d: SID: %s, Name: %s, Status: %d / %d\n", 
			i, context.nScriptId.string().cstr(), context.sName.cstr(),  
			ScriptTimer::timerCount( context.pScript ), 
			KeyHandler::keyCount( context.pScript ) );
	}

	return sStatus;
}

bool WorldContext::isScriptActive( lua_State * pScript )
{	
	if ( ScriptTimer::timerCount( pScript ) > 0 || KeyHandler::keyCount( pScript ) )
		return true;
	return false;
}

void WorldContext::updateScripts()	
{
	// update script timers
	ScriptTimer::updateTimers();
}

void WorldContext::releaseScripts()
{
	ScriptTimer::deleteAllTimers();
	KeyHandler::deleteAllKeys();

	while( m_Scripts.size() > 0 )
		killScript( m_Scripts[0].nScriptId );
}

//----------------------------------------------------------------------------

const char * WorldContext::loadChunk( lua_State * pScript, void * ud, size_t * size )
{
	CharString * pText = *((CharString **)ud);
	if( pText == NULL )
		return NULL;

	*(CharString **)ud = NULL;
	*size = (*pText).length();
	return (*pText).buffer();
} 

int WorldContext::DoString( lua_State * pScript, CharString sText )
{
	CharString * pText = &sText;
	int top = lua_gettop( pScript );					// remember size of stack
    lua_getglobal( pScript, "scriptAlert" );			// push scriptAlert address on top of the stack, thus to top + 1

	int status = lua_load( pScript, loadChunk, &pText, sText );	// load string as code using "loadChunk"
	if( status == 0 )
		status = lua_pcall( pScript, 0, 0, top + 1 );	// no arguments, discard results, scriptalert is at position top + 1

	lua_settop( pScript, top);							// reduce stack to old size after the call to prevent a stack overflow

	return status;
}

int WorldContext::DoCall( lua_State * pScript, CharString sFunc, Array<CharString> & arArguments )
{
	int top = lua_gettop( pScript );
	lua_getglobal( pScript, "scriptAlert" );
	lua_getglobal( pScript, sFunc );
	
	for( int i = 0 ; i < arArguments.size() ; i++ )
		lua_pushstring( pScript, arArguments[i] );
	
	int status = lua_pcall( pScript, arArguments.size(), 0, top + 1 );
	lua_settop( pScript, top);  

	return status;
}

CharString WorldContext::TranslateLuaError( int error )
{
	switch( error )
	{
		case 0: return "No error";
		case LUA_ERRSYNTAX: return "Syntax error during pre-compilation";
 		case LUA_ERRMEM: return "Memory allocation error";
 		case LUA_ERRRUN: return "A runtime error";
 		case LUA_ERRERR: return "Error while running the error handler function";
		default: return "Unknown error";
	}
}

WorldContext * WorldContext::GetContext( lua_State * pScript )
{
	lua_getglobal( pScript, "context" );
	
	WorldContext * pContext = (WorldContext *)lua_touserdata( pScript, -1 );
	ASSERT( pContext );

	lua_pop( pScript, 1 );
	return pContext;
}

Noun * WorldContext::GetNoun( lua_State * pScript, const char * pId )
{
	if ( pId != NULL )
	{
		// first attempt to find the object by it's widget key
		Noun * pNoun = WidgetCast<Noun>( Widget::findWidget( WidgetKey( pId ) ) );

		// if object not found by it's key, attempt to find the object by name
		if ( pNoun == NULL )
		{
			WorldContext * pContext = GetContext( pScript );
			pNoun = pContext->findNoun( pId );
		}

		return pNoun;
	}

	return NULL;
}

// dump all local variables at a given function level
CharString WorldContext::dumpLocals( lua_State * pScript, lua_Debug * ar, int level )
{
	CharString result;

	const char * pName;
    int i = 1;
    while( (pName = lua_getlocal( pScript, ar, i++ ) ) != NULL)
	{
		CharString value;  
		int type = lua_type( pScript, i );	// usage of the already incremeted i is intentional here
		// TODO: Embedded zeros inside strings aren't handled correctly yet.
		// So strings containing embedded zeros will appear shorter than they really are
		switch( type )
		{
			case LUA_TNUMBER:			// numbers are automatically converted to string
			case LUA_TSTRING:			value = lua_tostring( pScript, i );						break;
			case LUA_TBOOLEAN:			value = lua_toboolean( pScript, i ) ? "true" : "false"; break;
			case LUA_TNIL:				// use the native name for all other types instead of the value		
			case LUA_TTABLE:
			case LUA_TFUNCTION:
			case LUA_TUSERDATA:
			case LUA_TTHREAD:
			case LUA_TLIGHTUSERDATA:	value = lua_typename( pScript, type ); break;
			default: value = "Unknown type !";
		}
		
		result += CharString().format( ", %s = %s", pName, value.cstr() );
    }

	return result;
}

// Output a stack trace and local variables for debugging
void WorldContext::Alert( lua_State * pScript, const char * pMessage, const char * pDetails /*= NULL */)
{
	CharString alert;
	alert.format("SCRIPT: %s", pMessage );
	if ( pDetails != NULL )
		alert += CharString().format( ": %s", pDetails );
	alert += "\n";

	TRACE( alert );
	if ( s_AlertsEnabled )
	{
		WorldContext * pContext = GetContext( pScript );
		if ( pContext != NULL )
		{
			// get additional info on the code which executed scriptAlert()
			lua_Debug ar;
			if( lua_getstack( pScript, 1, &ar ) == 1 )
			{
				lua_getinfo( pScript, "Sln", &ar );
				int nCurrLine = ar.currentline;
				int nLineDefined = ar.linedefined;
				CharString sWhat( ar.what ? ar.what : "(null)" );
				CharString sName( ar.name ? ar.name : "(null)" );
				CharString sSource( ar.source ? ar.source : "(null)" );

				if( sWhat == "main" && sSource.length() < 256 )			// usually a function directly called by the C code
					alert += CharString().format( " *** Source: %s", sSource.cstr() );	
				else 
					if( sWhat == "Lua" )	// Error directly within the Lua code or within the C code via scriptAlert()
						alert += CharString().format( " *** In line %d, in function %s(), defined in line %d."
										, nCurrLine, sName.cstr(), nLineDefined );
				
				// append local variables
				alert += CharString().format( " Locals: %s", dumpLocals( pScript, &ar, 1 ).cstr() );
					
				// append callstack
				for( int i = 2 ; lua_getstack( pScript, i, &ar ) == 1 ; i++ )
				{
					lua_getinfo( pScript, "ln", &ar );
					if( i == 2 )
						alert += " *** Callstack:";
					
					alert += CharString().format( ", %d : %s()", ar.currentline, ar.name ? ar.name : "(null)" );
				}
			}

			pContext->pushChat( alert );
		}
	}
}

void WorldContext::ResetExecutionDepth( lua_State * pScript )
{
	lua_pushnumber( pScript, 0 );
	lua_setglobal( pScript, "ExecutionDepth" );
}

int WorldContext::GetExecutionDepth( lua_State * pScript )
{
	lua_getglobal( pScript, "ExecutionDepth" );
	int ExecutionDepth = lua_tonumber( pScript, -1 );
	lua_pop( pScript, 1 );

	return ExecutionDepth;
}

void WorldContext::IncreaseExecutionDepth( lua_State * pScript )
{
	lua_pushnumber( pScript, GetExecutionDepth( pScript ) + 1 );
	lua_setglobal( pScript, "ExecutionDepth" );
}

void WorldContext::DecreaseExecutionDepth( lua_State * pScript )
{
	int newIdent = GetExecutionDepth( pScript ) - 1;
	if( newIdent < 0 )
		newIdent = 0;	// if tracing was enabled inside a function the occuring returns we didn't receive the call
						// would make the ident negative.
	lua_pushnumber( pScript, newIdent );
	lua_setglobal( pScript, "ExecutionDepth" );
}

void WorldContext::LuaHook( lua_State * pScript, lua_Debug * ar )
{
	WorldContext * pContext = GetContext( pScript );
	if ( pContext != NULL )
	{
		CharString sWhat, sName;
		int nLineDefined;
		if( ar->event != LUA_HOOKLINE )	// save some time if tracing the execution of each line
		{
			lua_getinfo( pScript, "Sln", ar );
			nLineDefined = ar->linedefined;
			sWhat = ar->what ? ar->what : "(null)";
			sName = ar->name ? ar->name : "(null)";
		}
		int nCurrLine = ar->currentline;

		CharString result;
		switch( ar->event )
		{
			case LUA_HOOKCALL:
				result.format("Call: %s() W: %s LD: %d", sName.cstr(), sWhat.cstr(), nLineDefined );
				break;
			case LUA_HOOKRET:
				result.format("Retn: %s() W: %s LD: %d", sName.cstr(), sWhat.cstr(), nLineDefined );
				DecreaseExecutionDepth( pScript );
				break;
			case LUA_HOOKLINE:
				result.format("Line: %d", nCurrLine );
				break;
			case LUA_HOOKCOUNT:
				result += "Unsupported hook method";
				break;
			case LUA_HOOKTAILRET:
				result += "Unsupported hook method";
				break;
			default:
				result += "Unknown hook method";
		}
		
		CharString formatString;
		formatString.format( "SCRIPT: TRACE: %%%ds %%s", GetExecutionDepth( pScript ) * 2 );

		pContext->pushChat( CharString().format( formatString, "", result.cstr() ) );

		if( ar->event == LUA_HOOKCALL )
			IncreaseExecutionDepth( pScript );
	}
}

//---------------------------------------------------------------------------------------------------
// STANDARD FUNCTIONS

int WorldContext::ScriptAlert( lua_State * pScript )
{
	if ( lua_isstring( pScript, 1 ) )
		Alert( pScript, "ScriptAlert()", lua_tostring( pScript, 1 ) );
	else
		Alert( pScript, "ScriptAlert() error", "ScriptAlert() error! Function needs 1 parameter." );
	return 0;
}

int WorldContext::enableAlerts( lua_State * pScript )
{
	if ( lua_gettop( pScript ) == 1 )
		s_AlertsEnabled = lua_tonumber( pScript, 1 ) != 0;
	else
		Alert( pScript, "enableAlerts() error!", "Function needs 1 parameter." );
	return 0;
}

int WorldContext::enableTrace( lua_State * pScript )
{
	if ( lua_gettop( pScript ) == 1 )
	{
		int tracemode = lua_tonumber( pScript, 1 );
		switch( tracemode )	// select trace mode, and reset ExecutionDepth if call & return are not hooked
		{
			
			case 0:	// disable hooks
				lua_sethook( pScript, LuaHook, 0, 0 );
				ResetExecutionDepth( pScript );
				break;
			case 1:	// hook function calls and returns
				lua_sethook( pScript, LuaHook, LUA_MASKCALL | LUA_MASKRET, 0 );	break;
			case 2:	// hook execution of each line
				lua_sethook( pScript, LuaHook, LUA_MASKLINE, 0 );
				ResetExecutionDepth( pScript );
				break;
			case 3:	// hook function calls, returns and execution of each line
				lua_sethook( pScript, LuaHook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0 ); break;
			default:
				Alert( pScript, "enableTrace() error!", "Invalid mode." );
		}
	}
	else
		Alert( pScript, "enableTrace() error!", "Function needs 1 parameter." );

	return 0;
}

int WorldContext::randomNumber( lua_State * pScript )
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 2 )
	{
		int low = lua_tonumber( pScript, 1 );
		int high = lua_tonumber( pScript, 2 );

		if ( low <= high )
		{
			lua_pushnumber( pScript, low + (rand() % ((high - low) + 1)) );
			return 1;
		}
		else
			sErrorDetail = "'low' must be <= 'high'.";
	}
	else
		sErrorDetail = "Function needs 2 parameters.";

	Alert( pScript, "randomNumber() error! ", sErrorDetail );
	return 0;
}

int WorldContext::isClient( lua_State * pScript )
{
	WorldContext * pContext = GetContext( pScript );
	lua_pushboolean( pScript, pContext->isClient() );
								
	return 1;
}

int WorldContext::isServer( lua_State * pScript )
{
	WorldContext * pContext = GetContext( pScript );
	lua_pushboolean( pScript, pContext->isServer() );
	return 1;
}

int WorldContext::teamCount( lua_State * pScript )
{
	WorldContext * pContext = GetContext( pScript );
	lua_pushnumber( pScript, pContext->teamCount() );
	return 1;
}

int WorldContext::teamId( lua_State * pScript )
{
	CharString sError;

	WorldContext * pContext = GetContext( pScript );
	if ( lua_gettop( pScript ) == 1 )
	{
		int n = lua_tonumber( pScript, 1 );
		if ( n >= 0 && n < pContext->teamCount() )
		{
			int nTeamId = pContext->teamId( n );
			lua_pushnumber( pScript, nTeamId );
			return 1;
		}
		else
			sError = "Index out of range.";
	}
	else
		sError = "This functin needs 1 parameter.";

	Alert( pScript, "teamId() error! ", sError );
	return 0;
}

int WorldContext::getTeamFaction( lua_State * pScript )
{
	CharString sError;
	WorldContext * pContext = GetContext( pScript );
	if ( lua_gettop( pScript ) == 1 )
	{
		int n = lua_tonumber( pScript, 1 );
		if ( n >= 0 && n < pContext->teamCount() )
		{
			lua_pushnumber( pScript, pContext->teamN( n ).factionId );
			return 1;
		}
		else
			sError = "Index out of range.";
	}
	else
		sError = "This function needs 1 parameter.";

	Alert( pScript, "getTeamFaction() error! ", sError );
	return 0;
}

int WorldContext::getTeamName( lua_State * pScript )
{
	CharString sError;
	WorldContext * pContext = GetContext( pScript );
	if ( lua_gettop( pScript ) == 1 )
	{
		int n = lua_tonumber( pScript, 1 );
		if ( n >= 0 && n < pContext->teamCount() )
		{
			lua_pushstring( pScript, CharString(pContext->teamN( n ).name) );
			return 1;
		}
		else
			sError = "Index out of range.";
	}
	else
		sError = "This function needs 1 parameter.";

	Alert( pScript, "getTeamName() error! ", sError );
	return 0;
}

int WorldContext::getName( lua_State * pScript )
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 1 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1) );
		if ( pNoun != NULL )
		{
			lua_pushstring( pScript, CharString(pNoun->name()) );
			return 1;
		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 1 parameter.";

	Alert( pScript, "getName() error! ", sErrorDetail );
	return 0;
}

int WorldContext::getKey( lua_State * pScript )
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 1 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1) );
		if ( pNoun != NULL )
		{
			lua_pushstring( pScript, CharString(pNoun->key().string()) );
			return 1;
		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 1 parameter.";

	Alert( pScript, "getKey() error! ", sErrorDetail );
	return 0;
}

int WorldContext::getClass( lua_State * pScript )
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 1 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1) );
		if ( pNoun != NULL )
		{
			lua_pushstring( pScript, CharString(pNoun->factory()->className()) );
			return 1;
		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 1 parameter.";

	Alert( pScript, "getClass() error! ", sErrorDetail );
	return 0;
}

int WorldContext::nodeCount( lua_State * pScript )
{
	if ( lua_gettop( pScript ) == 1 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1) );
		if ( pNoun != NULL )
		{
			lua_pushnumber( pScript, pNoun->childCount() );
			return 1;
		}

		Alert( pScript, "nodeCount() error! Failed to find parent!" );
		return 0;
	}

	WorldContext * pContext = GetContext( pScript );
	int count = 0;
	for(int j=0;j<pContext->zoneCount();j++)
	{
		count += pContext->zone( j )->childCount();
	}

	lua_pushnumber( pScript, count );

	return 1;
}

int WorldContext::getNode( lua_State * pScript )
{
	if ( lua_gettop( pScript ) == 2 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 2) );
		if ( pNoun != NULL )
		{
			int n = lua_tonumber( pScript, 1 );
			if ( n >= 0 && n < pNoun->childCount() )
			{
				lua_pushstring( pScript, CharString(pNoun->child( n )->key().string()) );
				return 1;
			}

			Alert( pScript, "getNode() error! Index out of range!" );
			return 0;
		}

		Alert( pScript, "getNode() error! Failed to find parent!" );
		return 0;
	}
	else if ( lua_gettop( pScript ) == 1 )
	{
		WorldContext * pContext = GetContext( pScript );

		int n = lua_tonumber( pScript, 1 );
		int index = 0;
		for(int j=0;j<pContext->zoneCount();j++)
		{
			NodeZone * pRoot = pContext->zone( j );
			int nZoneCount = pRoot->childCount();

			if( index + nZoneCount > n )	// index is within this zone ?
			{
				lua_pushstring( pScript, CharString(pRoot->child( n - index )->key().string()) );
				return 1;
			}
			else
				index += nZoneCount;
		}

		Alert( pScript, "getNode() error! Index out of range!" );
		return 0;
	}

	Alert( pScript, "getNode() error! Function needs 1 or 2 parameters." );
	return 0;
}

int WorldContext::isNoun( lua_State * pScript )		// isNoun( name )
{
	if ( lua_gettop( pScript ) == 1 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1 ) );
		lua_pushboolean( pScript, pNoun != NULL );
		return 1;
	}

	Alert( pScript, "isNoun() error! Function needs 1 parameter." );
	return 0;
}

int WorldContext::getTeamId( lua_State * pScript )
{
	if ( lua_gettop( pScript ) == 1 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1 ) );
		if( pNoun != NULL )
		{
			lua_pushnumber( pScript, pNoun->teamId() );
			return 1;
		}

		Alert( pScript, "getTeamId() error! Parameter is not Noun." );
		return 0;
	}

	Alert( pScript, "getTeamId() error! Function needs 1 parameter." );
	return 0;
}

int WorldContext::setTeamId( lua_State * pScript )
{
	CharString sErrorDetail;
	WorldContext * pContext = GetContext( pScript );
	if ( lua_gettop( pScript ) == 2 && pContext->isServer() )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1 ) );
		if ( pNoun != NULL )
		{
			int nTeamId = lua_tonumber( pScript, 2 );
			if ( pContext->isTeamValid( nTeamId ) )
			{
				pNoun->setTeamId( nTeamId );
				pNoun->resync();

				lua_pushboolean( pScript, true );
				return 1;
			}
			else
				sErrorDetail = "TeamId invalid.";
		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 2 parameters and be to called serverside.";

	Alert( pScript, "setTeamId() error! ", sErrorDetail );
	return 0;
}

int WorldContext::isFaction( lua_State * pScript )		// isFaction( name, faction )
{	
	if ( lua_gettop( pScript ) == 2 )
	{
		const char *	pName = lua_tostring( pScript, 1 );
		int				faction = lua_tonumber( pScript, 2 );

		Noun * pNoun = GetNoun( pScript, pName );
		if( pNoun != NULL )
		{
			lua_pushboolean( pScript, pNoun->factionId() == faction );
			return 1;
		}

		Alert( pScript, "isFaction() error! NounBody not found." );
		return 0;
	}

	Alert( pScript, "isFaction() error! This function needs 1 parameter." );
	return 0;
}

int WorldContext::getFactionTeam( lua_State * pScript )
{
	WorldContext * pContext = GetContext( pScript );

	if ( lua_gettop( pScript ) == 1 )
	{
		int faction = lua_tonumber( pScript, 1 );
		lua_pushnumber( pScript, pContext->factionTeam( faction ) );
		return 1;
	}

	Alert( pScript, "getFactionTeam() error! This function needs 1 parameter." );
	return 0;
}

Verb::Scope WorldContext::getVerbScope( int nNumber )
{
	switch( nNumber )
	{
	case 0:
		return Verb::LOCAL;
	case 1:
		return Verb::GLOBAL;
	case 2:
		return Verb::FLEET;
	case 3:
		return Verb::FACTION;
	case 4:
		return Verb::PRIVATE;
	case 5:
		return Verb::SERVER;
	}

	return Verb::LOCAL;
}

int WorldContext::remoteCall( lua_State * pScript )	// remoteCall( target, scope, call )
{
	WorldContext * pContext = GetContext( pScript );

	int n = lua_gettop( pScript );
	if ( n != 3 )
	{
		Alert( pScript, "remoteCall() error! This function needs 3 parameters."  );
		return 0;
	}

	// get our script id
	UniqueNumber nScriptId;
	if ( ! pContext->findScriptId( pScript, nScriptId ) )
	{
		Alert( pScript, "remoteCall() error, failed to find own script ID!" );
		return 0;
	}

	// find the target
	Noun * pTarget = GetNoun( pScript, lua_tostring( pScript, 1 ) );
	
	// get the scope
	Verb::Scope scope = getVerbScope( lua_tonumber( pScript, 2 ) );

	// push the verb, a remotecall usually executes within an existing context
	Verb::Ref( new VerbCallScript( pTarget, scope, nScriptId, lua_tostring( pScript, 3 ), false ) );
	lua_pushboolean( pScript, true );
	return 1;
}

int WorldContext::getDistance( lua_State * pScript )	// getDistance( obj1, obj2 )
{
	if ( lua_gettop( pScript ) == 2 )
	{
		Noun * pNoun1 = GetNoun( pScript, lua_tostring( pScript, 1 ) );
		Noun * pNoun2 = GetNoun( pScript, lua_tostring( pScript, 2 ) );
		if ( pNoun1 == NULL || pNoun2 == NULL )
			return 0;	// no alert message here, this is intentional

		float distance = (pNoun1->worldPosition() - pNoun2->worldPosition()).magnitude();
		lua_pushnumber( pScript, distance );
		return 1;
	}

	Alert( pScript, "getDistance() error! Function needs 2 parameters." );
	return 0;
}

int WorldContext::getFaction( lua_State * pScript )
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 1 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1 ) );
		if ( pNoun != NULL )
		{
			lua_pushnumber( pScript, pNoun->factionId() );
			return 1;
		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 1 parameter.";

	Alert( pScript, "getFaction() error! ", sErrorDetail );

	return 0;
}


int WorldContext::createSpawn( lua_State * pScript )
{
	WorldContext * pContext = GetContext( pScript );

	if ( lua_gettop( pScript ) == 4 )
	{
		const char * pName = lua_tostring( pScript, 1 );
		Vector3 vWorldposition( lua_tonumber( pScript, 2 ),
			lua_tonumber( pScript, 3 ),
            lua_tonumber( pScript, 4 ) );

		Noun * pNoun = new NounZoneTarget( vWorldposition, pContext );
		pNoun->setName( CharString( pName ) );

		pContext->attachNoun( pNoun );

		lua_pushstring( pScript, CharString(pNoun->key().string()) );
		return 1;
	}

	Alert( pScript, "createSpawn() error! This function needs 4 parameters." );
	return 0;
}

int WorldContext::getPosition( lua_State * pScript )	// getPosition( object )
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 1 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1 ) );
		if ( pNoun != NULL )
		{
			Vector3 position( pNoun->worldPosition() );
			lua_pushnumber( pScript, position.x );
			lua_pushnumber( pScript, position.y );
			lua_pushnumber( pScript, position.z );
			return 3;
		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 1 parameter.";

	Alert( pScript, "getPosition() error! ", sErrorDetail );
	return 0;
}

int WorldContext::setPosition( lua_State * pScript )	// setPosition( object, x, y, z )
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 4 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1 ) );
		if ( pNoun != NULL )
		{
			Vector3 vWorldPosition( lua_tonumber( pScript, 2 ),
				lua_tonumber( pScript, 3 ), 
				lua_tonumber( pScript, 4 ) );

			pNoun->setWorldPosition( vWorldPosition );
			pNoun->resync();

			lua_pushboolean( pScript, true );
			return 1;
		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 4 parameters.";

	Alert( pScript, "getPosition() error! ", sErrorDetail );
	return 0;
}

int WorldContext::spawnNoun( lua_State * pScript )
{
	WorldContext * pContext = GetContext( pScript );
	if (! pContext->isServer() )
	{
		Alert( pScript, "spawnNoun() error! Server side function only!" );
		return 0;
	}

	if ( lua_gettop( pScript ) >= 3 )
	{
		// load the ship context
		NounContext::Link nc;
		if (! loadResource( CharString( lua_tostring( pScript, 1 ) ), nc ) )
		{
			Alert( pScript, "spawnNoun() error! Failed to load template"  );
			return 0;
		}

		// find the location
		Noun * pWhere = GetNoun( pScript, lua_tostring( pScript, 2 ) );
		if ( pWhere == NULL )
		{
			Alert( pScript, "spawnNoun() error! Failed to find location"  );
			return 0;
		}

		const char * pName = lua_tostring( pScript, 3 );
		if ( pName == NULL )
		{
			Alert( pScript, "spawnNoun() error! Invalid name"  );
			return 0;
		}
		
		// get the parent object, if any
		Noun * pParent = NULL;
		if ( lua_gettop( pScript ) > 3 )
			pParent = GetNoun( pScript, lua_tostring( pScript, 4 ) );

		// create a copy of the object
		Noun * pSpawn = nc->createNoun();
		if ( pSpawn == NULL )
		{
			Alert( pScript, "spawnNoun() error! Failed to spawn noun!"  );
			return 0;
		}

		pSpawn->setup();

		//// if the object is a gadget and we are not attaching it to a ship or structure, convert it into cargo
		//if ( WidgetCast<NounGadget>( pSpawn ) != NULL && 
		//	WidgetCast<NounStructure>( pParent ) == NULL &&
		//	WidgetCast<NounShip>( pParent ) == NULL )
		//{
		//	CargoGadget * pCargo = new CargoGadget( (NounGadget *)pSpawn );
		//	ASSERT( pCargo );

		//	delete pSpawn;
		//	pSpawn = pCargo;
		//}

		// generate the spawn location outside the radius of the target object
		Vector3 offset( RandomFloat(-1,1), 0.0f, RandomFloat(-1,1) );
		offset.normalize();

		float radius = pWhere->radius() + pSpawn->radius();
		radius *= 1.2f;		// expand the radius by 20%

		Vector3 position( pWhere->worldPosition() + (offset * radius) );
		if ( pParent != NULL )
			position -= pParent->worldPosition();

		pSpawn->setPosition( position );
		pSpawn->setName( CharString(pName) );

		//// if the parent was a planet, get the item into a hex on the planet
		//if ( WidgetCast<NounPlanet>( pParent ) )
		//{
		//	NounPlanet * pPlanet = (NounPlanet *)pParent;

		//	// find a available hex firstly
		//	if ( WidgetCast<NounStructure>( pSpawn ) )
		//		((NounStructure *)pSpawn)->setHex( pPlanet->findHex( position, true ) );
		//	else if ( WidgetCast<NounUnit>( pSpawn ) )
		//		((NounUnit *)pSpawn)->setHex( pPlanet->findHex( position, false ) );
		//}

		// attach the new object to the world
		pContext->attachNoun( pSpawn, pParent );

		// return the key of the new object
		lua_pushstring( pScript, CharString(pSpawn->key().string()) );
		return 1;
	}

	Alert( pScript, "spawnNoun() error! Function needs 3 or 4 parameters." );
	return 0;
}

int WorldContext::detachNoun( lua_State * pScript )
{
	CharString sErrorDetail;
	WorldContext * pContext = GetContext( pScript );

	if ( lua_gettop( pScript ) == 1 && pContext->isServer() )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1 ) );
		if ( pNoun != NULL )
		{
			pNoun->setDetach();

			lua_pushboolean( pScript, true );
			return 1;
		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 1 parameter and to be executed serverside.";

	Alert( pScript, "detachNoun() error! ", sErrorDetail );
	return 0;
}

int WorldContext::getRadius( lua_State * pScript )
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 1 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1) );
		if ( pNoun != NULL )
		{
			lua_pushnumber( pScript, pNoun->radius() );
			return 1;
		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 1 parameter.";

	Alert( pScript, "getRadius() error! ", sErrorDetail );
	return 0;
}

int WorldContext::getUserId( lua_State * pScript )
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 1 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1) );
		if ( pNoun != NULL )
		{
			lua_pushnumber( pScript, pNoun->userId() );
			return 1;
		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 1 parameter.";

	Alert( pScript, "getUserId() error! ", sErrorDetail );
	return 0;
}

int WorldContext::getAllNouns( lua_State * pScript )	// getAllNouns( nounId, range ) returns a table 1..n
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 2 )
	{
		Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1) );
		if ( pNoun != NULL )
		{
			WorldContext * pContext = GetContext( pScript );
			int range = lua_tonumber( pScript, 2 );
			
			int itemCount = 0;
			lua_newtable( pScript );

			Array< WorldContext::NounCollision > nouns;
			if ( pContext->proximityCheck( pNoun->worldPosition(), range, nouns ) )
			{	
				for(int i=0;i<nouns.size();i++)
				{
					Noun * pNoun = nouns[i].pNoun;

					lua_pushnumber( pScript, ++itemCount );				// index ( 1 .. n )
					lua_pushstring( pScript, CharString(pNoun->key().string()) );	// value ( nounId )
					lua_rawset( pScript, -3 );
				}
			}
			
			// set total number of elements contained
			lua_pushliteral( pScript, "n" );
			lua_pushnumber( pScript, itemCount );
			lua_rawset( pScript, -3 );

			return 1;	// return the table

		}
		else
			sErrorDetail = "Noun not found.";
	}
	else
		sErrorDetail = "This function needs 2 parameters.";

	Alert( pScript, "getAllNouns() error! ", sErrorDetail );
	return 0;
}

int WorldContext::getConstants( lua_State * pScript )	// getConstants() returns a hashtable( name, value )
{
	CharString sErrorDetail;
	if ( lua_gettop( pScript ) == 0 )
	{
		lua_newtable( pScript );

		for( int i=0 ; i<Constant::constantCount() ; i++ )
		{
			Constant * pConstant = Constant::constant( i );
			lua_pushstring( pScript, pConstant->name() );		// index ( name )
			lua_pushnumber( pScript, pConstant->value() );		// value 
			lua_rawset( pScript, -3 );
		}

		// set n to 0, as no numerical index exists
		lua_pushliteral( pScript, "n" );
		lua_pushnumber( pScript, 0 );
		lua_rawset( pScript, -3 );

		return 1;	// return the table
	}
	else
		sErrorDetail = "This function takes no parameters.";

	Alert( pScript, "getConstants() error! ", sErrorDetail );
	return 0;
}

//---------------------------------------------------------------------------------------------------

int WorldContext::listProperties( lua_State * pScript )
{
	//CharString sErrorDetail;
	//if ( lua_gettop( pScript ) == 1 )
	//{
	//	Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1) );
	//	if ( pNoun != NULL )
	//	{	
	//		// get a list of all properties
	//		Array< Property::Find > properties;
	//		pNoun->enumerate( properties );

	//		// Store the property names in a lua table
	//		int itemCount = 0;
	//		lua_newtable( pScript );

	//		for(int i=0;i<properties.size();i++)
	//		{
	//			lua_pushnumber( pScript, ++itemCount );			// index ( 1 .. n )
	//			lua_pushstring( pScript, properties[i].sExpr );	// property expression
	//			lua_rawset( pScript, -3 );
	//		}
	//		
	//		// set total number of elements contained
	//		lua_pushliteral( pScript, "n" );
	//		lua_pushnumber( pScript, itemCount );
	//		lua_rawset( pScript, -3 );
	//		return 1;

	//	}
	//	else
	//		sErrorDetail = "Noun not found.";
	//}
	//else
	//	sErrorDetail = "This function needs 1 parameter.";

	//Alert( pScript, "listProperties() error! ", sErrorDetail );
	return 0;
}

int WorldContext::getProperty( lua_State * pScript )
{
	//CharString sErrorDetail;
	//if ( lua_gettop( pScript ) == 2 )
	//{
	//	Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1) );
	//	if ( pNoun != NULL )
	//	{	
	//		CharString sPropName( lua_tostring( pScript, 2 ) );

	//		CharString sResult;
	//		if ( pNoun->get( sPropName, sResult ) )
	//		{
	//			lua_pushstring( pScript, sResult );
	//			return 1;
	//		}
	//		else
	//			sErrorDetail = "Property not found.";
	//	}
	//	else
	//		sErrorDetail = "Noun not found.";
	//}
	//else
	//	sErrorDetail = "This function needs 2 parameters.";

	//Alert( pScript, "getProperty() error! ", sErrorDetail );
	return 0;
}

int WorldContext::setProperty( lua_State * pScript )
{
	//CharString sErrorDetail;
	//if ( lua_gettop( pScript ) == 3 )
	//{
	//	Noun * pNoun = GetNoun( pScript, lua_tostring( pScript, 1) );
	//	if ( pNoun != NULL )
	//	{	
	//		CharString sExpr = lua_tostring( pScript, 2 );
	//		CharString sValue = lua_tostring( pScript, 3 );

	//		if ( pNoun->set(sExpr, sValue) )
	//		{
	//			lua_pushboolean( pScript, true );
	//			return 1;
	//		}
	//		else
	//			sErrorDetail = "Property not found.";
	//	}
	//	else
	//		sErrorDetail = "Noun not found.";
	//}
	//else
	//	sErrorDetail = "This function needs 3 parameters.";

	//Alert( pScript, "setProperty() error! ", sErrorDetail );
	return 0;
}


//----------------------------------------------------------------------------

int WorldContext::startTimer( lua_State * pScript )
{
	int n = lua_gettop( pScript );
	if ( n >= 2 && lua_isnumber( pScript, 1 ) && lua_isstring( pScript, 2 ) )
	{
		int				iTime = lua_tonumber( pScript, 1 );
		const char *	pCall = lua_tostring( pScript, 2 );

		Array< CharString > arguments;
		for(int i=3;i<=n;i++)
			arguments.push( lua_tostring( pScript, i ) );

		//TRACE( CharString().format("startTimer, time = %d, call = %s, arguments = %d", iTime, pCall, arguments.size() ) );
		new ScriptTimer( pScript, iTime, pCall, arguments );

		lua_pushboolean( pScript, true );
		return 1;
	}

	Alert( pScript, "startTimer() error! Function needs at least 2 arguments. First must be a number, second a string." );
	return 0;
}

//----------------------------------------------------------------------------
// INTERFACE FUNCTIONS

int WorldContext::flushChat( lua_State * pScript )
{
	Alert( pScript, "flushChat() error! This function is currently disabled." );
	return 0;
}

int WorldContext::pushChat( lua_State * pScript )
{
	WorldContext * pContext = GetContext( pScript );
	if ( lua_isstring( pScript, 1 ) )
	{
		pContext->pushChat( CharString(lua_tostring( pScript, 1 )) );
		lua_pushboolean( pScript, true );
		return 1;
	}
	else
		Alert( pScript, "pushChat() error! This function needs 1 string as parameter." );

	return 0;
}

int WorldContext::sendChat( lua_State * pScript )
{
	CharString sErrorDetail;
	WorldContext * pContext = GetContext( pScript );

	if ( lua_gettop( pScript ) == 3 )
	{
		Noun * pTarget = GetNoun( pScript, lua_tostring( pScript, 1 ) );
		// get the scope
		Verb::Scope scope = getVerbScope( lua_tonumber( pScript, 2 ) );

		const char * pMessage = lua_tostring( pScript, 3 );
		if ( pMessage != NULL )
		{
			Verb::Ref( new VerbChat( pTarget, scope, CharString(pMessage) ) );
			lua_pushboolean( pScript, true );
			return 1;
		}
		else
			sErrorDetail = "Invalid message.";
	}
	else
		sErrorDetail = "This function needs 3 parameters.";

	Alert( pScript, "sendChat() error! ", sErrorDetail );
	return 0;
}

//---------------------------------------------------------------------------------------------------

int WorldContext::trapKey( lua_State * pScript )
{
	static Hash< dword, KeyHandler > keys;
	
	int n = lua_gettop( pScript );
	if ( n >= 1 )
	{
		dword key = 0;
		if ( lua_isstring( pScript, 1 ) )
		{
			key = atoi( lua_tostring( pScript, 1 ) );
			if ( key == 0 )
				key = lua_tostring( pScript, 1 )[ 0 ];
		}
		if ( key != 0 && n == 1 )
		{
			KeyHandler::deleteKey( key );
			lua_pushboolean( pScript, false );
			return 1;
		}
		else if ( key != 0 && n == 2 && lua_isstring( pScript, 2 ) )
		{
			const char * pCall = lua_tostring( pScript, 2 );
			KeyHandler::installKey( pScript, key, pCall, true );
			lua_pushboolean( pScript, true );
			return 1;
		}
	}

	WorldContext::Alert( pScript, "trapKey() error! Function needs 1 or 2 arguments." );
	return 0;
}

int WorldContext::watchKey( lua_State * pScript )
{
	static Hash< dword, KeyHandler > keys;
	
	int n = lua_gettop( pScript );
	if ( n >= 1 )
	{
		dword key = 0;
		if ( lua_isstring( pScript, 1 ) )
		{
			key = atoi( lua_tostring( pScript, 1 ) );
			if ( key == 0 )
				key = lua_tostring( pScript, 1 )[ 0 ];
		}
		if ( key != 0 && n == 1 )
		{
			KeyHandler::deleteKey( key );
			lua_pushboolean( pScript, false );
			return 1;
		}
		else if ( key != 0 && n == 2 && lua_isstring( pScript, 2 ) )
		{
			const char * pCall = lua_tostring( pScript, 2 );
			KeyHandler::installKey( pScript, key, pCall, false );
			lua_pushboolean( pScript, true );
			return 1;
		}
	}

	WorldContext::Alert( pScript, "watchKey() error! Function needs 1 or 2 arguments." );
	return 0;
}

//----------------------------------------------------------------------------
// EOF

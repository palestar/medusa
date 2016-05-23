/*
	LuaHeaders.h

	This header includes all required Lua headers and also adds the static Lua lib for linking.
	There is a define USE_PLAIN_LUA to use Lua 5.1 instead of LuaJIT 5.1
	(c)2009 Palestar Development, Richard Lyle
*/

#ifndef LUA_HEADERS_H
#define LUA_HEADERS_H

//----------------------------------------------------------------------------

#ifdef USE_PLAIN_LUA
	#pragma include_alias( "lua.h",      "../../Medusa/ThirdParty/Lua51/src/lua.h" )
	#pragma include_alias( "lualib.h",   "../../Medusa/ThirdParty/Lua51/src/lualib.h" )
	#pragma include_alias( "lauxlib.h",  "../../Medusa/ThirdParty/Lua51/src/lauxlib.h" )
#else
	#pragma include_alias( "lua.h",      "../../Medusa/ThirdParty/LuaJIT/src/lua.h" )
	#pragma include_alias( "lualib.h",   "../../Medusa/ThirdParty/LuaJIT/src/lualib.h" )
	#pragma include_alias( "lauxlib.h",  "../../Medusa/ThirdParty/LuaJIT/src/lauxlib.h" )
	#pragma include_alias( "luajit.h",   "../../Medusa/ThirdParty/LuaJIT/src/luajit.h" )
#endif

//----------------------------------------------------------------------------

extern "C" {
	#ifndef USE_PLAIN_LUA
		#include "luajit.h"
	#endif
	#include "lua.h"		// "Lua" scripting language - www.lua.org
	#include "lualib.h"
	#include "lauxlib.h"
};

#endif

//----------------------------------------------------------------------------
// EOF

/*
	WorldClientCommand.cpp
	(c)2003 Palestar Inc, Richard Lyle
*/


#include "Standard/CommandLine.h"
#include "File/Path.h"
#include "File/FileDisk.h"
#include "WorldClient.h"

//----------------------------------------------------------------------------

const char *	META_CHAT_PREFIX = "<color;00aaaa>";
const char *	META_CHAT_SUFFIX = "</color>";

enum Command
{
	CMD_INVALID,

	CMD_HELP,		// help
	CMD_SERVERS,	// get a list of servers
	CMD_CONNECT,	// connect to a different server
	CMD_FLUSH,		// flush chat buffer
	CMD_META,
	CMD_METAC,
	CMD_ECHO,
	
	CMD_RUN,		// run a script locally
	CMD_PS,			// list running scripts
	CMD_KILL,		// kill a running script
	CMD_FORWARD,	// forward command onto server
};

struct CommandLookup
{
	const char *	pPattern;
	Command			command;
	dword			flags;
	const char *	pHelp;
	dword			color;
};

static CommandLookup COMMAND_LOOKUP[] = 
{
	{ "/?",				CMD_HELP,			0,							"Display help",											GREY.RGB() },
	{ "/serverlist",	CMD_SERVERS,		MetaClient::MODERATOR,		"List all game servers",								GREY.RGB() },
	{ "/flush",			CMD_FLUSH,			0,							"Flush chat buffer",									GREY.RGB() },
	{ "/meta",			CMD_META,			0,							"Send command to MetaServer",							GREY.RGB() },
	{ "/echo",			CMD_ECHO,			0,							"Echo local chat",										GREY.RGB() },

	{ "/run",			CMD_RUN,			MetaClient::DEVELOPER,		"Run a script",											GREY.RGB() },
	{ "/kill",			CMD_KILL,			MetaClient::DEVELOPER,		"Kill a script",										GREY.RGB() },
	{ "/ps",			CMD_PS,				MetaClient::DEVELOPER,		"List running scripts",									GREY.RGB() },

	{ "/forward",		CMD_FORWARD,		0,							"Forward command to server",							GREY.RGB() },

	{ "/connect",		CMD_CONNECT,		MetaClient::MODERATOR,		"Connect to a game server",								GREY.RGB() },

	{ "/room",			CMD_METAC,			0,							"List/Join/Create/Find Rooms",							GREY.RGB()  },
	{ "/status",		CMD_METAC,			0,							"Set status",											GREY.RGB()  },
	{ "/find",			CMD_METAC,			0,							"Find another user",									GREY.RGB()  },
	{ "/who",			CMD_METAC,			0,							"Show profile information",								GREY.RGB()  },
	{ "/clan",			CMD_METAC,			0,							"List/Join/Create/Find clans",							GREY.RGB()  },
	{ "/friend",		CMD_METAC,			0,							"List/Add/Remove friends",								GREY.RGB()  },
	{ "/ignore",		CMD_METAC,			0,							"List/Add/Remove ignores",								GREY.RGB()  },
	{ "/say",			CMD_METAC,			0,							"Send message to players in current room",				GREY.RGB()  },
	{ "/whisper",		CMD_METAC,			0,							"Whisper message to another player in the room",		GREY.RGB()  },

	{ "/me",			CMD_METAC,			0,							"Emotion",												GREY.RGB()   },
	{ "/send",			CMD_METAC,			0,							"Send message to another user/friend/clan",				GREY.RGB()   },
	{ "/report",		CMD_METAC,			0,							"Send message to moderators",							GREY.RGB()   },
	{ "/away",			CMD_METAC,			0,							"Set status as away",									GREY.RGB()   },
	{ "/back",			CMD_METAC,			0,							"Remove away status",									GREY.RGB()   },

	{ "/broadcast",		CMD_METAC,			MetaClient::MODERATOR,		"Broadcast a message",									GREY.RGB()   },
	{ "/modtalk",		CMD_METAC,			MetaClient::MODERATOR,		"Send a message to all moderators in the room",			GREY.RGB()   },
	{ "/devtalk",		CMD_METAC,			MetaClient::DEVELOPER,		"Send a message to all developers in the room",			GREY.RGB()   },
	{ "/moderate",		CMD_METAC,			MetaClient::MODERATOR,		"Toggle moderate status for the current room",			GREY.RGB()   },
	{ "/modsend",		CMD_METAC,			MetaClient::MODERATOR,		"Send an official message to another player",			GREY.RGB()   },
	{ "/modsay",		CMD_METAC,			MetaClient::MODERATOR,		"Send an official message to the room",					GREY.RGB()   },
	{ "/staffsend",		CMD_METAC,			MetaClient::STAFF,			"Send a message to all moderators",						GREY.RGB()   },
	{ "/mute",			CMD_METAC,			MetaClient::MODERATOR,		"Set mute status on another player",					GREY.RGB()   },
	{ "/unmute",		CMD_METAC,			MetaClient::MODERATOR,		"Remove mute status",									GREY.RGB()   },
	{ "/check",			CMD_METAC,			MetaClient::MODERATOR,		"Quickchecks a player for Watchlist entrys",			GREY.RGB()   },
	{ "/watch",			CMD_METAC,			MetaClient::MODERATOR,		"Add player to the watchList",							GREY.RGB()   },
	{ "/push",			CMD_METAC,			MetaClient::MODERATOR,		"Ban player for 30 seconds",							GREY.RGB()   },
	{ "/kick",			CMD_METAC,			MetaClient::MODERATOR,		"Ban player for 10 minutes",							GREY.RGB()   },
	{ "/ban",			CMD_METAC,			MetaClient::MODERATOR,		"Ban player for 24 hours",								GREY.RGB()   },
	{ "/banned",		CMD_METAC,			MetaClient::MODERATOR,		"List all bans which expire within 24 hours",			GREY.RGB()   },
	{ "/bannedlong",	CMD_METAC,			MetaClient::MODERATOR,		"List all banned users",								GREY.RGB()   },
	{ "/unban",			CMD_METAC,			MetaClient::MODERATOR,		"Unban a player",										GREY.RGB()   },
	{ "/reban",			CMD_METAC,			MetaClient::ADMINISTRATOR,	"Extend a ban",											GREY.RGB()   },
	{ "/clones",		CMD_METAC,			MetaClient::MODERATOR,		"Display additional accounts a player is currently using",	GREY.RGB()   },
	{ "/servers",		CMD_METAC,			MetaClient::MODERATOR,		"Find users + servers with same MID or IP",				GREY.RGB()   },
	{ "/logroom",		CMD_METAC,			MetaClient::MODERATOR,		"Display chat logs",									GREY.RGB()   },
	{ "/loguser",		CMD_METAC,			MetaClient::ADMINISTRATOR,	"Display all chat send and received by a user, including private messages",		GREY.RGB()   },
	{ "/session",		CMD_METAC,			MetaClient::MODERATOR,		"Display session information for a user",				GREY.RGB()   },
	{ "/hide",			CMD_METAC,			MetaClient::ADMINISTRATOR,	"Set status to hidden",									GREY.RGB()   },
	{ "/unhide",		CMD_METAC,			MetaClient::ADMINISTRATOR,	"Remove hidden status",									GREY.RGB()   },
};
static int COMMAND_LOOKUP_COUNT = sizeof(COMMAND_LOOKUP)/sizeof(COMMAND_LOOKUP[0]);

//----------------------------------------------------------------------------

bool WorldClient::sendChat( const char * pChat )
{
	if ( pChat == NULL || pChat[ 0 ] == 0 )
		return false;

	if ( pChat[0] == '/' )
		processCommand( pChat );
	else
		sendChatReal( pChat );

	return true;
}

void WorldClient::sendChatReal( const char * pChat )
{
	send( SERVER_RECV_CHAT ) << CharString( pChat );
}

bool WorldClient::receiveChat( const char * pChat, dword nFromId /*= 0*/ )
{
	if ( pChat == NULL || pChat[ 0 ] == 0 )
		return false;

	lock();

	if ( pChat[0] == '/' )
		processCommand( pChat, nFromId );
	else
		pushChat( pChat, nFromId );
	
	unlock();

	return true;
}

bool WorldClient::processCommand( const char * pChat, dword nFromId /*= 0*/ )
{
	if ( pChat == NULL || pChat[ 0 ] == 0 )
		return false;

	// copy into a buffer so we can tokenize it
	CommandLine arguments( pChat, true );
	if ( arguments.argumentCount() < 1 )
		return false;

	// get the command
	CharString pattern( arguments.argument( 0 ) );

	Command command = CMD_INVALID;
	Color color( GREY );

	// search the commands
	for(int i=0;i<COMMAND_LOOKUP_COUNT;++i)
	{
		// allow /aw to match against /away command by using strnicmp
		if ( pattern.compareCommand( COMMAND_LOOKUP[i].pPattern ) == 0 )
		{
			// found a matching string, check the access rights
			if ( COMMAND_LOOKUP[i].flags != 0 && (profile()->flags() & COMMAND_LOOKUP[i].flags) == 0 )
			{
				pushChat( "<color;808080>Error, command is restricted!</color>" );
				return false;
			}

			command = COMMAND_LOOKUP[i].command;
			color = COMMAND_LOOKUP[i].color;
			break;
		}
	}

	switch( command )
	{
	case CMD_INVALID:
		// forward all unkown commands onto the server
		sendChatReal( pChat );
		break;
	case CMD_HELP:
		{
			CharString sHelp;
			sHelp += "<color;ffffff>Client Commands:\n";
			
			for(int i=0;i<COMMAND_LOOKUP_COUNT;++i)
			{
				sHelp += CharString().format("%s ... %s\n", COMMAND_LOOKUP[i].pPattern, 
					COMMAND_LOOKUP[i].pHelp );
			}
			pushChat( sHelp );

			// forward message onto the server
			sendChatReal( pChat );
		}
		break;
	case CMD_SERVERS:
		{
			if ( metaClient()->connected() )
			{
				Array< MetaClient::Server > servers;
				if ( metaClient()->getServers( "", metaClient()->gameId(), MetaClient::GAME_SERVER, servers) > 0 )
				{
					CharString text;
					text += CharString().format("<color;ffffff>Found %d servers...\n", servers.size() );

					for(int i=0;i<servers.size();i++)
						text += CharString().format("%u : %s : %u / %u\n", 
							servers[i].id, servers[i].name.cstr(),
							servers[i].clients, servers[i].maxClients );

					pushChat( text );
				}
				else
					pushChat( "<color;808080>Error, failed to get server list..." );
			}
			else
				pushChat( "<color;808080>Error, no connection to the meta-server..." );
		}
		break;
	case CMD_CONNECT:
		{
			if ( arguments.argumentCount() == 2 )
			{
				dword serverId = strtol( arguments.argument( 1 ), NULL, 10 );
				if ( metaClient()->connected() )
				{
					Array< MetaClient::Server > servers;
					if ( metaClient()->getServers( "", metaClient()->gameId(), MetaClient::GAME_SERVER, servers) > 0 )
					{
						for(int i=0;i<servers.size();i++)
							if (servers[i].id == serverId )
							{
								if ( connect( servers[i].address, servers[i].port ) && login( sessionId() ) )
									pushChat( CharString().format("<color;808080>Connected to '%s'...", servers[i].name.cstr()) );
								else
									pushChat( "<color;808080>Failed to connect..." );

								break;
							}
					}
					else
						pushChat( "<color;808080>Error, failed to get server list..." );
				}
				else
					pushChat( "<color;808080>Error, no connection to the meta-server..." );
			}
			else
				pushChat( "<color;808080>Usage: /connect [server id]" );
		}
		break;
	case CMD_FLUSH:
		flushChat();
		break;
	case CMD_META:
		metaClient()->sendChat( m_nDefaultRoomId, arguments.restring( 1 ) );
		break;
	case CMD_METAC:
		metaClient()->sendChat( m_nDefaultRoomId, pChat );
		break;
	case CMD_ECHO:
		pushChat( arguments.restring( 1 ) );
		break;
	case CMD_RUN:
		{
			if ( arguments.argumentCount() > 1 )
			{
				char * pScript = FileDisk::loadTextFile( CharString("Scripts/") + arguments.argument( 1 ) );
				if ( pScript != NULL )
				{
					// get the name of the script
					CharString sName( Path( arguments.argument( 1 ) ).file() );
					// create a scriptId
					UniqueNumber nScriptId;

					bool bLocal = false;
					bool bServer = false;

					// optionally specify the scope to run the script
					if ( arguments.argumentCount() > 2  )
					{
						if ( CharString( "SERVER" ).compareCommand( arguments.argument(2) ) == 0 )
							bServer = true;
						else if ( CharString( "GLOBAL" ).compareCommand( arguments.argument(2) ) == 0 )
							bLocal = bServer = true;

						// optionally specify the SID to use
						if ( arguments.argumentCount() > 3 )
							nScriptId = arguments.argument( 3 );
					}
					else
						bLocal = true;

					pushChat( CharString().format("<color;ffffff>Running script %s / %s (%s%s).", 
						sName.cstr(), nScriptId.string().cstr(), bLocal ? " LOCAL " : "", bServer ? " SERVER " : "") );

					// run the script locally
					if ( bLocal )
					{
						if (! context()->runScript( nScriptId, CharString(sName), pScript ) )
							pushChat( CharString().format("<color;ffffff>Failed to run script %s.", sName.cstr()) );
					}

					if ( bServer )
						send( CONTEXT_RUN_SCRIPT ) << nScriptId << CharString(sName) << CharString(pScript);

					// delete the memory allocated for the script
					delete [] pScript;
				}
				else
					pushChat( CharString().format("<color;808080>Error, failed to load script from %s.", arguments.argument(1)) );
			}
			else
				pushChat( "<color;808080>Usage: /run [filename] | [local|server|global] | [sid]" );
		}
		break;
	case CMD_KILL:
		{
			if ( arguments.argumentCount() > 1 )
			{
				bool bLocal = false;
				bool bServer = false;

				if ( arguments.argumentCount() > 2  )
				{
					if ( CharString( "SERVER" ).compareCommand( arguments.argument(2) ) == 0 )
						bServer = true;
					else if ( CharString( "GLOBAL" ).compareCommand( arguments.argument(2) ) == 0 )
						bLocal = bServer = true;
				}
				else
					bLocal = bServer = true;

				for(int i=context()->scriptCount()-1;i>=0;i--)
				{
					const WorldContext::ScriptContext & script = context()->script( i );

					bool bKill = CharString( "*" ).compare( arguments.argument(1) ) == 0 || 
						script.nScriptId.string().compareCommand( arguments.argument(1) ) == 0;

					if ( bKill )
					{
						UniqueNumber nScriptId( script.nScriptId );

						pushChat( CharString().format("<color;ffffff>Killing script %s (%s%s).", 
							nScriptId.string().cstr(), bLocal ? " LOCAL " : "", bServer ? " SERVER " : "") );
						if ( bLocal )
						{
							if (! context()->killScript( nScriptId ) )
								pushChat( CharString().format("<color;ffffff>Failed to kill script %s.", nScriptId.string().cstr()) );
						}

						if ( bServer )
							send( CONTEXT_KILL_SCRIPT ) << nScriptId;
					}
				}
			}
			else
				pushChat( "<color;808080>Usage: /kill [*|sid] | [local|server|global]" );
		}
		break;
	case CMD_PS:
		pushChat( context()->scriptStatus() );
		break;
	case CMD_FORWARD:
		sendChatReal( arguments.restring( 1 ) );
		break;
	}

	return true;
}

bool WorldClient::pushMetaChat( const MetaClient::Chat & a_Chat )
{
	if ( a_Chat.text.length() > 0 )
	{
		if ( a_Chat.text[ 0 ] == '/' )
		{
			CharString message;
			message.format( "%s* %s%s", META_CHAT_PREFIX, filterMetaChat( a_Chat.text.buffer() + 1 ).cstr(), META_CHAT_SUFFIX );
			pushChat( message );
		}
		else
		{
			CharString message;
			message.format( "%s%s %s: \"%s\"%s", 
				META_CHAT_PREFIX, Time::format( a_Chat.time, "%H:%M:%S").cstr(),
				a_Chat.author.cstr(), filterMetaChat( a_Chat.text ).cstr(), META_CHAT_SUFFIX );
			pushChat( message );
		}
	}

	return true;
}

bool WorldClient::pushChat( const char * pChat, dword nFromId /*= 0*/)
{
	if ( pChat == NULL || pChat[ 0 ] == 0 )
		return false;

	AutoLock lock( &m_Lock );
	if ( m_pMetaClient != NULL && m_pMetaClient->isIgnored( nFromId ) )
	{
		TRACE( CharString().format( "Ignoring chat %s from user %u", pChat, nFromId ) );
		return false;
	}

	// save the chat in the log
	m_Chat.push( pChat );
	// notify the user
	user()->onChat( pChat, nFromId );

	return true;
}

void WorldClient::flushChat()
{
	lock();
	
	// notify the user before we clear the chat buffer
	user()->onFlushChat();
	// clear the chat buffer
	m_Chat.release();

	unlock();
}


//----------------------------------------------------------------------------
//EOF

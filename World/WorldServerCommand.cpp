/*
	WorldServerCommand.cpp
	(c)2003 Palestar Inc, Richard Lyle
*/


#include "Standard/CommandLine.h"
#include "Standard/RegExpM.h"
#include "File/Path.h"
#include "Standard/StringBuffer.h"
#include "Standard/StringHelpers.h"
#include "Resource/Resource.h"
#include "File/FileDisk.h"
#include "WorldServer.h"

//----------------------------------------------------------------------------

enum Command
{
	CMD_INVALID,	// invalid command

	CMD_HELP,		// help
	CMD_BPS,		// set bytes per second
	CMD_FOCUS,		// set players focus area

	CMD_MODERATE,	// turn moderator mode on or off
	CMD_CLIENTS,	// show all clients connected to this server
	CMD_BROADCAST,	// send message to all players on the server
	CMD_PUSH,		// push a player

	CMD_RUN,		// run a script
	CMD_KILL,		// kill running script
	CMD_PS,			// display running scripts
	CMD_SLAVES,		// display slaves for this server
	CMD_BOUNCE,		// redirect a command to another client/server
	CMD_WIDGETS,	// dump allocated widgets to log file
	CMD_TRACK,		// command to enable/disable widget tracking
	CMD_CRASH,		// command to crash the server (for testing core dumps)
};

struct CommandLookup
{
	const char *	pPattern;
	Command			nCommand;
	dword			flags;
	const char *	pHelp;
	dword			color;
};

static CommandLookup COMMAND_LOOKUP[] = 
{
	{ "/?",			CMD_HELP,			0,						"Display help",						GREY.RGB() },
	{ "/focus",		CMD_FOCUS,			0,						"Set focus area",					GREY.RGB() },

	{ "/moderate",	CMD_MODERATE,		MetaClient::MODERATOR,	"Set moderator mode",				YELLOW.RGB() },
	{ "/clients",	CMD_CLIENTS,		MetaClient::MODERATOR,	"Display clients",					YELLOW.RGB() },
	{ "/scream",	CMD_BROADCAST,		MetaClient::MODERATOR,	"Send message to all clients",		MAGENTA.RGB() },
	{ "/push",		CMD_PUSH,			MetaClient::MODERATOR,	"Disconnect a client",				YELLOW.RGB() },

	{ "/run",		CMD_RUN,			MetaClient::DEVELOPER,	"Run a script",						GREY.RGB() },
	{ "/kill",		CMD_KILL,			MetaClient::DEVELOPER,	"Kill a script",					GREY.RGB() },
	{ "/ps",		CMD_PS,				MetaClient::DEVELOPER,	"List running scripts",				GREY.RGB() },
	{ "/widgets",	CMD_WIDGETS,		MetaClient::DEVELOPER,	"Dump allocated widgets to log",	GREY.RGB() },
	{ "/track",		CMD_TRACK,			MetaClient::DEVELOPER,	"Enable/Disable tracking on widgets", GREY.RGB() },

	{ "/slaves",	CMD_SLAVES,			MetaClient::ADMINISTRATOR, "Display slave servers",			GREY.RGB() },
	{ "/bounce",	CMD_BOUNCE,			MetaClient::ADMINISTRATOR, "Bounce a Command to a client",	GREY.RGB() },
	{ "/crash",		CMD_CRASH,			MetaClient::ADMINISTRATOR, "Crash the server",				GREY.RGB() },
};
static int COMMAND_LOOKUP_COUNT = sizeof(COMMAND_LOOKUP)/sizeof(COMMAND_LOOKUP[0]);

static CharString		MUTED_STRING("[MUTED]");

//----------------------------------------------------------------------------

bool WorldServer::processCommand( ClientContext & context, const char * pChat )
{
	if ( !pChat || pChat[ 0 ] == 0 )
		return false;
	if ( pChat[ 0 ] != '/' )
		return processTeamChat( context, pChat, GOLD );

	dword nClientId = context.nClientId;
	dword nUserId = context.nUserId;

	// copy into a buffer so we can tokenize it
	CommandLine arguments( pChat, true );
	CharString pattern( arguments.argument( 0 ) );

	Command nCommand = CMD_INVALID;
	Color color( WHITE );

	// search the commands
	int n = pattern.length();			
	for(int c=0;c<COMMAND_LOOKUP_COUNT;++c)
	{
		// allow /aw to match against /away command by using strnicmp
		if ( strnicmp<char>( pattern, COMMAND_LOOKUP[c].pPattern, n ) == 0 )
		{
			// found a matching string, check the access rights
			if ( COMMAND_LOOKUP[c].flags != 0 && (context.MetaProfile.flags & COMMAND_LOOKUP[c].flags) == 0 )
			{
				sendChat( nClientId, "<color;808080>You do not have the rights to use this command!", 0 );
				return false;
			}

			nCommand = COMMAND_LOOKUP[c].nCommand;
			color = COMMAND_LOOKUP[c].color;
			break;
		}
	}

	switch( nCommand )
	{
	case CMD_INVALID:
		// fall through to the help command
		sendUserChat( nUserId, CharString().format( "<color;ffffff>Invalid command '%s', type \"/?\" for help...", pChat ) );
		return true;
	case CMD_HELP:
		{
			CharString help;
			help += "<color;ffffff>Server Commands:\n";
			
			for(int i=0;i<COMMAND_LOOKUP_COUNT;++i)
			{
				// only show commands the player has the rights to use
				if ( COMMAND_LOOKUP[ i ].flags == 0 || (COMMAND_LOOKUP[ i ].flags & context.MetaProfile.flags) != 0 )
					help += CharString().format("%s ... %s\n", COMMAND_LOOKUP[i].pPattern, COMMAND_LOOKUP[i].pHelp );
			}

			sendUserChat( nUserId, help );
		}
		return true;
	case CMD_FOCUS:
		{
			if ( arguments.argumentCount() == 2 )
			{
				float fFocus = Clamp<float>( strtod( arguments.argument(1), NULL), MIN_FOCUS, MAX_FOCUS );
				LOG_STATUS( "WorldServer", CharString().format("Client %u setting focus to %.1f...", nClientId, fFocus) );

				setClientFocusArea( 0, nClientId, fFocus );
				sendChat( nClientId, CharString().format( "<color;ffffff>SETTINGS: Focus area set to %.1f...", fFocus ) );
			}
			else
				sendUserChat( nUserId, "<color;808080>Usage: /focus [focus area]" );
		}
		return true;
	case CMD_MODERATE:
		{
			if ( arguments.argumentCount() == 2 )
				setModerate( nClientId, strtol( arguments.argument(1), NULL, 10 ) != 0 );
			else
				sendUserChat( nUserId, "<color;808080>Usage: /moderate [0|1]" );
		}
		return true;
	case CMD_CLIENTS:
		{
			//sendUserChat( nUserId, "<color;808080>Not implemented.." );
			CharString sClients = "<color;808080>Clients:\n";

			for(int i=0;i<clientCount();i++)
			{
				// get the client ID
				dword nId = client(i);
				// get the clients IP address
				CharString sAddress = clientAddress( nId );

				ClientContextHash::iterator iContext = m_ClientContext.find( nId );
				if ( iContext != m_ClientContext.end() )
				{
					ClientContext & context = iContext->second;

					sClients += CharString().format("ID: %u, IP: %s : User: %s @%u, P: %u ms\n", 
						nId, sAddress.cstr(), context.MetaProfile.name.cstr(), context.nUserId, context.nLatency );
				}
				else
					sClients += CharString().format("ID: %u, IP: %s : No Login\n", nId, sAddress.cstr());
			}

			sendUserChat( nUserId, sClients );
		}
		return true;
	case CMD_BROADCAST:
		{
			if ( arguments.argumentCount() > 1 )
			{
				CharString message( arguments.restring( 1 ) );
				filterChat( context, message );

				CharString chat;
				chat.format( "<color;%6.6x>%s %s: \"%s\"", color.RGB(), 
					chatTime().cstr(), context.MetaProfile.name.cstr(), message.cstr() );

				// send chat message to all players
				for(int i=0;i<clientCount();i++)
					sendChat( client(i), chat );
			}
			else
				sendUserChat( nUserId, "<color;808080>Usage: /scream [message]" );
		}
		return true;
	case CMD_PUSH:
		{
			if ( arguments.argumentCount() == 2 )
			{
				dword nPushId = strtol( arguments.argument(1), NULL, 10 );

				if ( clientPort( nPushId ) != 0 )
				{
					// disconnect the client
					Server::removeClient( nPushId );
					// send back message
					sendUserChat( nUserId, CharString().format("<color;808080>Client %u disconnected...", nPushId) );
				}
				else
					sendUserChat( nUserId, CharString().format( "<color;808080>Error, client %u not found!", nPushId) );

			}
			else
				sendUserChat( nUserId, "<color;808080>Usage: /push [ClientId]" );
		}
		return true;
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
					bool bClients = false;

					// optionally specify the scope to run the script
					if ( arguments.argumentCount() > 2  )
					{
						if ( CharString( "GLOBAL" ).compareCommand( arguments.argument(2) ) == 0 )
							bLocal = bClients = true;
						else if ( CharString( "CLIENTS" ).compareCommand( arguments.argument(2)) == 0 )
							bClients = true;
						else if ( CharString( "LOCAL" ).compareCommand( arguments.argument(2)) == 0 )
							bLocal = true;

						// optionally specify the SID to use
						if ( arguments.argumentCount() > 3 )
							nScriptId = arguments.argument( 3 );
					}
					else
						bLocal = true;

					LOG_STATUS( "WorldServer", "Client %u (%s) running script %s:", context.nClientId, context.MetaProfile.name.cstr(), sName.cstr() );
					LOG_STATUS( "WorldServer", pScript );
					
					sendUserChat( nUserId, CharString().format("<color;ffffff>Running script %s / %s (%s%s).</color>", 
						sName.cstr(), nScriptId.string().cstr(), bLocal ? " LOCAL " : "", bClients ? " CLIENTS " : ""));

					if ( bLocal )
					{
						// run the script locally
						if (! worldContext()->runScript( nScriptId, CharString(sName), pScript ) )
							sendUserChat( nUserId, CharString().format("<color;ffffff>Failed to run script %s.</color>", sName.cstr()) );
					}

					if ( bClients )
					{
						for(int i=0;i<clientCount();i++)
							if ( isClient( client(i) ) )
								send( client(i), WorldClient::CONTEXT_RUN_SCRIPT ) << nScriptId << CharString( sName ) << CharString( pScript );
					}

					// delete the memory allocated for the script
					FileDisk::deleteFile( pScript );
				}
				else
					sendUserChat( nUserId, CharString().format("<color;808080>Error, failed to load script from %s.</color>", arguments.argument(1)) );
			}
			else
				sendUserChat( nUserId, "<color;808080>Usage: /run [filename] | [local|clients|global] | [sid]</color>" );
		}
		return true;
	case CMD_KILL:
		{
			if ( arguments.argumentCount() > 1 )
			{
				bool bLocal = false;
				bool bClients = false;

				if ( arguments.argumentCount() > 2  )
				{
					if ( CharString( "GLOBAL" ).compareCommand( arguments.argument(2) ) == 0 )
						bLocal = bClients = true;
					else if ( CharString( "CLIENTS" ).compareCommand( arguments.argument(2)) == 0 )
						bClients = true;
					else if ( CharString( "LOCAL" ).compareCommand( arguments.argument(2)) == 0 )
						bLocal = true;
				}
				else
					bLocal = bClients = true;

				for(int i=worldContext()->scriptCount()-1;i>=0;i--)
				{
					const WorldContext::ScriptContext & script = worldContext()->script( i );

					bool bKill = CharString( "*" ).compare( arguments.argument(1) ) == 0 || 
						script.nScriptId.string().compareCommand( arguments.argument(1) ) == 0;

					if ( bKill )
					{
						UniqueNumber nScriptId( script.nScriptId );

						sendUserChat( nUserId, CharString().format("<color;ffffff>Killing script %s (%s%s).", 
							nScriptId.string().cstr(), bLocal ? " LOCAL " : "",  bClients ? " CLIENTS " : "") );

						if ( bLocal )
						{
							if (! worldContext()->killScript( nScriptId ) )
								sendUserChat( nUserId, CharString().format("<color;ffffff>Failed to kill script %s.", nScriptId.string().cstr()) );
						}

						if ( bClients )
						{
							for(int i=0;i<clientCount();i++)
								if ( isClient( client(i) ) )
									send( client(i), WorldClient::CONTEXT_KILL_SCRIPT ) << nScriptId;
						}
					}
				}
			}
			else
				sendUserChat( nUserId, "<color;808080>Usage: /kill [sid|*] | [local|clients|global]" );
		}
		return true;
	case CMD_PS:
		sendUserChat( nUserId, worldContext()->scriptStatus() );
		return true;
	case CMD_BOUNCE:
		{
			if ( arguments.argumentCount() >= 3 )
			{
				CharString sCommand( arguments.restring( 2 ) );

				bool bClients = false;

				if ( CharString( "GLOBAL" ).compareCommand( arguments.argument(1) ) == 0 )
					bClients = true;
				else if ( CharString( "CLIENTS" ).compareCommand( arguments.argument(1)) == 0 )
					bClients = true;

				if ( bClients )
				{
					// send command to all clients
					for(int i=0;i<clientCount();i++)
						if ( isClient( client(i) ) )
							sendChat( client(i), sCommand );

					sendUserChat( nUserId, CharString().format("<color;808080>Bounced command \"%s\" to clients...", sCommand.cstr()) );
				}
				else
				{
					dword nSendId = strtol( arguments.argument( 1 ), NULL, 10 );
					if ( nSendId != 0 && clientPort( nSendId ) != 0 )
					{
						sendChat( nSendId, sCommand );
						sendUserChat( nUserId, CharString().format("<color;808080>Bounced command \"%s\" to client %u...", sCommand.cstr(), nSendId) );
					}
					else
						sendUserChat( nUserId, CharString().format("<color;808080>Error, failed to bounce command to client %u", nSendId) );
				}

			}
			else
				sendUserChat( nUserId, "<color;808080>Usage: /bounce [clientId|clients|servers|global] [Command ...]" );
		}
		return true;
	case CMD_WIDGETS:
		{
			sendUserChat( nUserId, "Dumping widgets to log..." );
			LOG_STATUS( "WorldServer", Widget::dumpWidgets() );
		}
		return true;
	case CMD_TRACK:
		{
			Widget::sm_bEnableTracking = !Widget::sm_bEnableTracking;
			sendUserChat( nUserId, Widget::sm_bEnableTracking ? "Tracking enabled..." : "Tracking disabled..." );
		}
		return true;
	case CMD_CRASH:
		THROW_EXCEPTION( "Server Crash Command" );
		return true;
	}

	return false;
}

bool WorldServer::processGlobalChat( ClientContext & context, const char * pChat, Color color )
{
	if ( context.nUserId == 0  )
		return false;

	// the default is to send team chat if no command is specified
	CharString message( pChat );
	if (! filterChat( context, message ) )
		return false;

	CharString chat;
	chat.format( "<color;%6.6x>%s %s: \"%s\"", color.RGB(), chatTime().cstr(), context.MetaProfile.name.cstr(), message.cstr() );

	if ( checkMuted( context, chat ) )
		return true;

	for(int i=0;i<clientCount();++i)
		sendChat( client(i), chat, context.nUserId );

	return true;
}

bool WorldServer::processTeamChat( ClientContext & context, const char * pChat, Color color )
{
	if ( context.nUserId == 0 || context.nTeamId == 0 )
		return false;

	// the default is to send team chat if no command is specified
	CharString message( pChat );
	if (! filterChat( context, message ) )
		return false;

	CharString chat;
	chat.format( "<color;%6.6x>%s %s: \"%s\"", color.RGB(), chatTime().cstr(), context.MetaProfile.name.cstr(), message.cstr() );

	if ( checkMuted( context, chat ) )
		return true;

	ClientArray & clients = m_TeamClientHash[ context.nTeamId ];
	for( ClientArray::const_iterator iClient = clients.begin(); iClient != clients.end(); ++iClient )
		sendChat( *iClient, chat, context.nUserId );

	return true;
}

bool WorldServer::processFactionChat( ClientContext & context, const char * pChat, Color color )
{
	if ( context.nUserId == 0 || context.nFactionId == 0 )
		return false;

	// the default is to send team chat if no command is specified
	CharString message( pChat );
	if (! filterChat( context, message ) )
		return false;

	CharString chat;
	chat.format( "<color;%6.6x>%s %s: \"%s\"", color.RGB(), chatTime().cstr(), context.MetaProfile.name.cstr(), message.cstr() );

	if ( checkMuted( context, chat ) )
		return true;

	ClientArray & clients = m_FactionClientHash[ context.nFactionId ];
	for( ClientArray::const_iterator iClient = clients.begin(); iClient != clients.end(); ++iClient )
		sendChat( *iClient, chat, context.nUserId );

	return true;
}

bool WorldServer::processTargetChat( ClientContext & context, const char * pChat, Color color )
{
	if ( context.nUserId == 0 || !context.pTarget.valid() || !context.pTarget->userId() )
		return false;

	// find the other player
	CharString message( pChat );
	if (! filterChat( context, message ) )
		return false;

	CharString chat;
	chat.format( "<color;%6.6x>%s %s: \"%s\"", color.RGB(), 
		chatTime().cstr(), context.MetaProfile.name.cstr(), message.cstr() );

	sendUserChat( context.pTarget->userId(), chat, context.nUserId );
	sendChat( context.nClientId, chat, context.nUserId );

	return true;
}

bool WorldServer::processAreaChat( ClientContext & context, const char * pChat, Color color )
{
	if ( context.nUserId == 0 || !context.pSelf.valid() )
		return false;

	// get all players in the local area of the senders ship
	ClientArray * pClients = getNounClients( context.pSelf );
	if (! pClients )
		return false;

	CharString message( pChat );
	if (! filterChat( context, message ) )
		return false;

	CharString chat;
	chat.format( "<color;%6.6x>%s %s: \"%s\"", color.RGB(), chatTime().cstr(), context.MetaProfile.name.cstr(), message.cstr() );

	if ( checkMuted( context, chat ) )
		return true;

	for( ClientArray::const_iterator iClient = (*pClients).begin(); iClient != (*pClients).end(); ++iClient )
		sendChat( *iClient, chat, context.nUserId );

	return true;
}

bool WorldServer::sendChat( dword nClientId, const char * pChat, dword nFromId /*= 0*/ )
{
	send( nClientId, WorldClient::CLIENT_RECV_CHAT ) << CharString( pChat ) << nFromId;
	return true;
}

bool WorldServer::sendUserChat( dword nUserId, const char * pChat, dword nFromId /*= 0*/)
{
	UserClientHash::iterator iClient = m_UserClientHash.find( nUserId );
	if ( iClient == m_UserClientHash.end() )
		return false;

	return sendChat( iClient->second, pChat, nFromId );
}

bool WorldServer::sendUserChat( const Array< dword > & players, const char * pChat, dword nFromId )
{
	if ( players.size() <= 0 )
		return false;

	for(int i=0;i<players.size();i++)
	{
		UserClientHash::iterator iClient = m_UserClientHash.find( players[i] );
		if ( iClient != m_UserClientHash.end() )
			sendChat( iClient->second, pChat, nFromId );
	}

	return true;
}

bool WorldServer::checkMuted( ClientContext & context, const char * pChat )
{
	if ( (context.MetaProfile.flags & MetaClient::MUTED) == 0 ) 
		return false;		// NOT MUTED

	sendChat( context.nClientId, "<color;808080>SERVER: You are muted. No other players will see your messages." );
	sendModeratorChat( MUTED_STRING + pChat, context.nUserId );
	return true;
}

bool WorldServer::sendModeratorChat( const char * pChat, dword nFromId /*= 0*/ )
{
	for(int i=0;i<clientCount();++i)
	{
		dword nClientId = client(i);
		if ( isModerator( nClientId ) )
			sendChat( nClientId, pChat, nFromId );
	}
	return true;
}

bool WorldServer::filterChat( ClientContext & context, CharString & a_sChat )
{
	// make sure any < character is escaped correctly..
	for( int nEscape = a_sChat.find( "<" ); nEscape >= 0; 
		nEscape = a_sChat.find( "<", nEscape + 1 ) )
	{
		if ( a_sChat[nEscape + 1] == '<' )
			++nEscape;							// < is properly escaped, skip this character, we will do another find after our << characters
		else 
			a_sChat.insert( '<', nEscape++ );		// escape the character and skip
	}

	// replace any %t with the current target name..
	CharString targetName;
	if ( context.pTarget.valid() )
		targetName = context.pTarget->name();
	else
		targetName = "[no target]";
	a_sChat.replace( "%t", targetName );

	return true;
}

CharString WorldServer::chatTime()
{
	dword nTick = worldContext()->tick();

	CharString time;
	time.format("%2.2u:%2.2u:%2.2u", 
		nTick / TICKS_PER_HOUR, 
		(nTick / TICKS_PER_MINUTE) % 60, 
		(nTick / TICKS_PER_SECOND) % 60 );

	return time;
}

//----------------------------------------------------------------------------
//EOF

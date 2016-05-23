/*
	MetaClientCommand.cpp
	(c)2003 Palestar Inc, Richard Lyle
*/

#include "Debug/Assert.h"
#include "Standard/Time.h"
#include "Standard/CommandLine.h"
#include "Standard/StringHelpers.h"
#include "GCQ/MetaClient.h"

#pragma warning( disable : 4996 )	// The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name: _strnicmp. See online help for details.

//----------------------------------------------------------------------------

enum Commands
{
	CMD_FIRST,	
	CMD_HELP = CMD_FIRST,			// display help
	
	CMD_ROOM,						// list rooms
	CMD_STATUS,						// set/view status
	CMD_FIND,						// find another user by name
	CMD_WHO,						// show profile information
	CMD_CLAN,						// list clans
	CMD_FRIEND,						// show/add/remove friends
	CMD_IGNORE,						// show/add/remove ignores
	CMD_WHISPER,					// send private message to anther user in the room

	CMD_INVALID,					// invalid command

	// place new commands here

	CMD_COUNT,
	CMD_LAST = CMD_COUNT - 1
};

struct CommandLookup
{
	const char *	pCommand;
	Commands		command;
	dword			flags;
	const char *	pHelp;
};

static CommandLookup COMMAND_LOOKUP[] =
{
	{ "/?",			CMD_HELP,		0,							"Display chat help" },
	{ "/room",		CMD_ROOM,		0,							"List/Join/Create/Find Rooms" },
	{ "/status",	CMD_STATUS,		0,							"Set status" },
	{ "/find",		CMD_FIND,		0,							"Find another user" },
	{ "/who",		CMD_WHO,		0,							"Show profile information" },
	{ "/clan",		CMD_CLAN,		0,							"List/Join/Create/Find clans" },
	{ "/friend",	CMD_FRIEND,		0,							"List/Add/Remove friends" },
	{ "/ignore",	CMD_IGNORE,		0,							"List/Add/Remove ignores" },
	{ "/whisper",	CMD_WHISPER,	0,							"Whisper message to another player in the room" },
};
static int COMMAND_LOOKUP_COUNT = sizeof(COMMAND_LOOKUP)/sizeof(COMMAND_LOOKUP[0]);

//----------------------------------------------------------------------------

/**
 *  roomId: ID of the room to send the message to, if used set recpId = 0
 *  recpId: ID of the player to send the message to, if used set roomId = 0
 *  pMessage: The message itself
 *  sendLocal: True by default if not specified, if false no local echo is sent
 *  
 *  If roomId and recpId are 0 the message is treated as a broadcast (sendGlobalChat)
 *  If roomId and recpId are both set behavior is unspecified.
 *  If setLocal is set also set roomId and recpId to 0
 **/
void MetaClient::sendChat( dword roomId, dword recpId, const char * pMessage, bool sendLocal /*= true*/ )
{
	send( SERVER_RECV_CHAT ) << roomId << recpId << CharString(pMessage);

	if ( sendLocal )
		sendLocalChat( roomId, recpId, pMessage );
}

void MetaClient::sendPrivateChat( dword recpId, const char * pMessage )
{
	sendChat( 0, recpId, pMessage );
}

/**
 *  Sends the Message to the current Room.
 *  Also handles all Client/Serverside commands i.e.
 *  /away /mute /friend /kick etc...
 **/
void MetaClient::sendChat( dword roomId, const char * pMessage )
{
	if ( pMessage[ 0 ] == '/' )
	{
		Commands command = CMD_INVALID;
		
		CommandLine arguments( pMessage );
		if ( arguments.argumentCount() > 0 )
		{
			CharString keyword( arguments.argument( 0 ) );
			
			int n = keyword.length();
			for(int i=0;i<COMMAND_LOOKUP_COUNT;++i)
			{
				if ( strnicmp<char>( COMMAND_LOOKUP[ i ].pCommand, keyword, n ) == 0 )
				{
					// found keyword match, check rights
					if ( COMMAND_LOOKUP[ i ].flags != 0 && (m_Profile.flags & COMMAND_LOOKUP[ i ].flags) == 0 )
					{
						sendLocalChat("/Invalid command; you do not have the rights to use this command..." );
						return;
					}

					command = COMMAND_LOOKUP[ i ].command;
					break;
				}
			}
		}

		switch( command )
		{
		case CMD_HELP:
			{
				CharString help;
				
				help += "/<b>GCQ Client Commands</b>\n";
				
				for(int i=0;i<COMMAND_LOOKUP_COUNT;++i)
					help += CharString().format("<b>%s</b> ... %s\n", COMMAND_LOOKUP[i].pCommand, COMMAND_LOOKUP[i].pHelp);

				sendLocalChat( help );

				// forward message onto server, so it can add to the help, don't local echo
				sendChat( roomId, 0, pMessage, false );
			}
			break;
		case CMD_ROOM:						// list rooms
			{
				bool usage = true;

				if ( arguments.argumentCount() >= 2 )
				{
					CharString sub( arguments.argument( 1 ) );
					if ( strnicmp<char>( sub, "list", sub.length() ) == 0 )
					{
						Array< Room > rooms;
						if ( getRooms( rooms ) > 0 )
						{
							CharString text;
							
							text += CharString().format("/Found %d rooms...\n", rooms.size() );
							for(int i=0;i<rooms.size();i++)
							{
								text += CharString().format("<b>%s</b> <i>@%u</i>, %u members%s%s\n", 
									rooms[i].name.cstr(), rooms[i].roomId, rooms[i].members, 
									(rooms[i].flags & FLAG_ROOM_PASSWORD) ? ", password protected" : "",
									(rooms[i].flags & FLAG_ROOM_MODERATED) ? ", moderated" : "" );
							}

							sendLocalChat( text );
							usage = false;
						}
						else
							sendLocalChat( "/Error, failed to get room list..." );
					}
					else if ( strnicmp<char>( sub, "join", sub.length() ) == 0 )
					{
						if ( arguments.argumentCount() >= 3 )
						{
							usage = false;

							Room found;
							if ( findRoom( arguments.argument( 2 ), found ) )
							{
								CharString password;
								if ( arguments.argumentCount() > 3 )
									password = arguments.argument( 3 );

								if ( joinRoom( found.roomId, password ) > 0 )
									sendLocalChat( CharString().format("/Joined '%s' chat room...", found.name.cstr() ) );
								else
									sendLocalChat( "/Error, failed to join room..." );
							}
							else
								sendLocalChat( "/Error, failed to find room..." );
						}
						else
							sendLocalChat( "/Usage: /room join [room] | [password]" );
					}
					else if ( strnicmp<char>( sub, "create", sub.length() ) == 0 )
					{
						if ( arguments.argumentCount() >= 3 )
						{
							usage = false;

							CharString name( arguments.argument( 2 ) );
							
							CharString password;
							if ( arguments.argumentCount() > 3 )
								password = arguments.argument( 3 );

							if ( createRoom( name, password ) > 0 )
								sendLocalChat( CharString().format("/Chat Room '%s' created...", name.cstr()) );
							else
								sendLocalChat( "/Error, failed to create room..." );
						}
						else
							sendLocalChat( "/Usage: /room create [room] | [password]" );
					}
					else if ( strnicmp<char>( sub, "leave", sub.length() ) == 0 )
					{
						usage = false;
						if ( leaveRoom() < 0 )
							sendLocalChat( "/Error, failed to leave room..." );
					}
					else if ( strnicmp<char>( sub, "members", sub.length() ) == 0 )
					{
						dword roomId = 0;
						if ( arguments.argumentCount() >= 3 )
						{
							Room found;
							if ( findRoom( arguments.argument( 2 ), found ) )
								roomId = found.roomId;
						}

						if ( roomId != 0 )
						{
							Array< ShortProfile > members;
							if ( getPlayers( roomId, members ) > 0 )
							{
								CharString text;
								text += CharString().format("/Found %d members in room @%u...\n", members.size(), roomId );
								text += profileText( members );

								sendLocalChat( text );
								usage = false;
							}
							else
								sendLocalChat( "/Error, failed to get room members..." );
						}
						else
							sendLocalChat( "/Error, failed to find room..." );
					}
				}

				if ( usage )
					sendLocalChat( "/Usage: /room [list|join|create|leave|members]" );
			}
			break;
		case CMD_STATUS:					// set status
			{
				bool usage = true;

				if ( arguments.argumentCount() > 1 )
				{
					CharString status( arguments.restring( 1 ) );
					sendLocalChat( CharString().format("/Status is \"<b>%s</b>\"...", status.cstr()) );
					usage = false;

					sendStatus( status );
				}

				if ( usage )
					sendLocalChat( "/Usage: /status [message]" );
			}
			break;
		case CMD_FIND:						// find another user by name
			{
				bool usage = true;

				if ( arguments.argumentCount() > 1 )
				{
					CharString pattern( arguments.argument( 1 ) );

					usage = false;

					Array< ShortProfile > found;
					if ( findUser( pattern, found ) )
					{
						CharString text;
						text += CharString().format("/Found %d matches for '%s'...\n", found.size(), pattern.cstr() );
						text += profileText( found );

						sendLocalChat( text );
					}
					else
						sendLocalChat( CharString().format("/Found no matches for '%s'...", pattern.cstr()) );
				}

				if ( usage )
					sendLocalChat( "/Usage: /find [user name|@user id]" );
			}
			break;
		case CMD_WHO:						// show profile information
			{
				bool usage = true;
				if ( arguments.argumentCount() > 1 )
				{
					CharString pattern( arguments.argument( 1 ) );

					usage = false;
					Array< ShortProfile > found;
					if ( findUserExactFirst( pattern, found ) )
					{
						if ( found.size() == 1 )
						{
							Profile profile;
							if ( getProfile( found[0].userId, profile ) > 0 )
							{
								CharString text;
								text += CharString().format("/<b>Who:</b> %s @%u\n", profile.name.cstr(), profile.userId );
								text += CharString().format("<b>Clan:</b> @%u\n", profile.clanId );
								text += CharString().format("<b>Status:</b> %s\n", profile.status.cstr() );
								text += CharString().format("<b>Email:</b> %s\n", profile.email.cstr() );
								text += CharString().format("<b>Rank:</b> %s\n", profile.rank.cstr() );
								text += CharString().format("<b>Score:</b> %f\n", profile.score );

								//for(int i=0;i<profile.fields.size();i++)
								//	text += CharString().format("<tr><td>%s:</td><td><b>%s</b></td></tr>", profile.fields[i].name, profile.fields[i].value);

								sendLocalChat( text );
							}
							else
								sendLocalChat( "/Error, failed to get profile..." );
						}
						else
							sendLocalChat( CharString().format("/Found %d matches for '%s'...", found.size(), pattern.cstr()) );
					}
					else
						sendLocalChat( CharString().format("/Found no matches for '%s'...", pattern.cstr()) );
				}

				if ( usage )
					sendLocalChat( "/Usage: /who [user name|@user id]" );
			}
			break;
		case CMD_CLAN:						// list clans
			{
				bool usage = true;

				if ( arguments.argumentCount() > 1)
				{
					CharString sub( arguments.argument( 1 ) );
					if ( strnicmp<char>( sub, "list", sub.length() ) == 0 )
					{
						usage = false;

						CharString pattern;
						if ( arguments.argumentCount() > 2 )
							pattern = arguments.argument( 2 );

						Array< ShortClan > clans;
						if ( findClan( pattern, clans ) )
						{
							CharString text;
							
							text += CharString().format("/Found %d clans...\n", clans.size() );
							for(int i=0;i<clans.size();i++)
								text += CharString().format("<b>%s</b> <i>@%u</i>\n", clans[i].name.cstr(), clans[i].clanId);

							sendLocalChat( text );
						}
						else
							sendLocalChat( "/Error, failed to get clan list..." );
					}
					else if ( strnicmp<char>( sub, "join", sub.length() ) == 0 )
					{
						usage = false;
						if ( profile().clanId == 0 )
						{
							if ( arguments.argumentCount() > 2 )
							{
								CharString pattern( arguments.argument( 2 ) );

								Array< ShortClan > clan;
								if ( findClan( pattern, clan ) )
								{
									if ( clan.size() == 1 )
									{
										if ( joinClan( clan[0].clanId ) > 0 )
										{
											m_Profile.clanId = clan[0].clanId;
											sendLocalChat( CharString().format("/Joined clan <b>%s</b>...", clan[0].name.cstr()) );
										}
										else
											sendLocalChat( "/Error, failed to join clan. Make sure the clan is currently accepting new recruits..." );
									}
									else
										sendLocalChat( CharString().format("/Error, %d matches found for '%s'...", clan.size(), pattern.cstr()) );
								}
							}
							else
								sendLocalChat( "/Usage: /clan join [clan name:@clan id]" );
						}
						else
							sendLocalChat( "/Error, you must leave your current clan before you can join one." );
					}
					else if ( strnicmp<char>( sub, "create", sub.length() ) == 0 )
					{
						usage = false;
						if ( profile().clanId == 0 )
						{
							if ( arguments.argumentCount() == 6 )
							{
								Clan newClan;
								newClan.flags = 0;
								newClan.faction = 0;
								newClan.name = arguments.argument( 2 );
								newClan.longName = arguments.argument( 3 );
								newClan.motto = arguments.argument( 4 );
								newClan.home = arguments.argument( 5 );

								if ( createClan( newClan ) > 0 )
									sendLocalChat( "/Clan created..." );
								else
									sendLocalChat( "/Error, failed to create clan..." );
							}
							else
								sendLocalChat( "/Usage: /clan create [name] [long name] [motto] [home]" );
						}
						else
							sendLocalChat( "/Error, you must leave your current clan before you can create one." );
					}
					else if ( strnicmp<char>( sub, "leave", sub.length() ) == 0 )
					{
						usage = false;
						if ( profile().clanId != 0 )
						{
							if ( joinClan( 0 ) > 0 )
							{
								m_Profile.clanId = 0;
								sendLocalChat( "/Disconnected from clan..." );
							}
							else
								sendLocalChat( "/Error, failed to leave clan..." );
						}
						else
							sendLocalChat( "/Error, you are not in a clan." );
					}
					else if ( strnicmp<char>( sub, "info", sub.length() ) == 0 )
					{
						usage = false;
						dword clanId = m_Profile.clanId;
						if ( arguments.argumentCount() > 2 )
						{
							CharString pattern = arguments.argument( 2 );

							Array< ShortClan > clans;
							if ( findClan( pattern, clans ) )
								if ( clans.size() > 1 )
								{
									sendLocalChat( CharString().format("/Error, found %d matches for '%s'...", clans.size(), pattern.cstr()) );
									clanId = 0;
								}
								else
									clanId = clans[0].clanId;
							
						}
						else if ( clanId == 0 )
							sendLocalChat( "/Usage: /clan info [clan name|@clan_id]" );

						if ( clanId != 0 )
						{
							Clan clan;
							if ( getClan( clanId, clan ) > 0 )
							{
								CharString text;
								text += CharString().format("/<b>Name:</b> %s @%u\n", clan.name.cstr(), clan.clanId );
								text += CharString().format("<b>Long Name:</b> %s\n", clan.longName.cstr() );
								text += CharString().format("<b>Motto:</b> %s\n", clan.motto.cstr() );
								text += CharString().format("<b>Home:</b> %s\n", clan.home.cstr() );
								text += CharString().format("<b>Faction:</b> %d\n", clan.faction );
								text += CharString().format("<b>Score:</b> %f\n", clan.score );

								sendLocalChat( text );
							}
							else
								sendLocalChat( "/Error, failed to get clan information..." );
						}

					}
					else if ( strnicmp<char>( sub, "members", sub.length() ) == 0 )
					{
						usage = false;
						dword clanId = m_Profile.clanId;
						if ( arguments.argumentCount() > 2 )
						{
							CharString pattern = arguments.argument( 2 );

							Array< ShortClan > clans;
							if ( findClan( pattern, clans ) )
								if ( clans.size() > 1 )
								{
									sendLocalChat( CharString().format("/Error, found %d matches for '%s'...", clans.size(), pattern.cstr()) );
									clanId = 0;
								}
								else
									clanId = clans[0].clanId;
							
						}
						else if ( clanId == 0 )
							sendLocalChat( "/Usage: /clan members [clan name|@clan_id]" );

						if ( clanId != 0 )
						{
							Array< ShortProfile > members;
							if ( getClan( clanId, members ) > 0 )
							{
								CharString text;
								text += CharString().format("/Found %d members...\n", members.size() );
								text += profileText( members );

								sendLocalChat( text );
							}
							else
								sendLocalChat( "/Error, failed to get clan members..." );
						}
					}
					else if( strnicmp<char>(sub, "recruit", sub.length() ) == 0)
					{
						usage = false;
						if(profile().clanId != 0)
						{
							if(arguments.argumentCount() > 2)
							{
								if(recruitClanMember(profile().clanId, arguments.argumentDWORD(2)) > 0)
								{
									sendLocalChat("/Suceeded in recruiting!");
								}
								else
								{
									sendLocalChat("/Error, failed to recruit player ...");
								}
							}
							else
							{
								sendLocalChat("/Error, promotee id not suppllied");
							}
						}
						else
						{
							sendLocalChat("/Error, you are not in a clan ...");
						}
					}
					else if( strnicmp<char>(sub, "promote", sub.length() ) == 0)
					{
						usage = false;
						if(profile().clanId != 0)
						{
							if(arguments.argumentCount() > 2)
							{
								if(promoteClanMember(profile().clanId, arguments.argumentDWORD(2)) > 0)
								{
									sendLocalChat("/Suceeded in recruiting!");
								}
								else
								{
									sendLocalChat("/Error, failed to admin player ...");
								}
							}
							else
							{
								sendLocalChat("/Error, promotee id not suppllied");
							}
						}
						else
						{
							sendLocalChat("/Error, you are not in a clan ...");
						}
					}
				}
			
					if ( usage )
						sendLocalChat( "/Usage: /clan [list|join|create|leave|info|members|recruit|promote] [clan name|@clan id|@user id]" );
			}
			break;
		case CMD_FRIEND:					// show/add/remove friends
			{
				bool usage = true;

				if ( arguments.argumentCount() > 1 )
				{
					CharString sub( arguments.argument( 1 ) );
					if ( strnicmp<char>( sub, "list", sub.length() ) == 0 )
					{
						usage = false;
						Array< ShortProfile > friends;
						if ( getFriends( friends ) > 0 )
						{
							CharString text;
							text += CharString().format( "/Found %d friends...\n", friends.size() );
							text += profileText( friends );

							sendLocalChat( text );
						}
						else
							sendLocalChat( "/Error, failed to get friends..." );
					}
					else if ( strnicmp<char>( sub, "add", sub.length() ) == 0 )
					{
						usage = false;
						if ( arguments.argumentCount() == 3 )
						{
							CharString pattern( arguments.argument( 2 ) );

							Array< ShortProfile > found;
							if ( findUserExactFirst( pattern, found ) )
							{
								if ( found.size() == 1 )
								{
									if ( addFriend( found[0].userId ) > 0 )
										sendLocalChat( CharString().format("/Added '%s' to friends...", found[0].name.cstr()) );
									else
										sendLocalChat( "/Error, failed to add to friends..." );
								}
								else
									sendLocalChat( CharString().format( "/Error, found %d matches for '%s'...", found.size(), pattern.cstr() ) );
							}
							else
								sendLocalChat( CharString().format("/Error, failed to find user '%s'...", pattern.cstr()) );
						}
						else
							sendLocalChat( "/Usage: /friend add [user name|@user id] " );
					}
					else if ( strnicmp<char>( sub, "delete", sub.length() ) == 0 )
					{
						usage = false;
						if ( arguments.argumentCount() == 3 )
						{
							CharString pattern( arguments.argument( 2 ) );
							pattern.lower();

							Array< ShortProfile > friends;
							if ( getFriends( friends ) > 0 )
							{
								dword userId = 0;
								if ( pattern[0] != '@' )
								{
									for(int i=0;i<friends.size();i++)
									{
										CharString name( friends[i].name );
										name.lower();

										if ( strstr<char>( name, pattern ) != NULL )
										{
											userId = friends[i].userId;
											break;
										}
									}
								}
								else
									userId = strtoul( pattern.buffer() + 1, NULL, 10 );

								if ( userId != 0 && deleteFriend( userId ) > 0 )
									sendLocalChat( "/Friend deleted..." );
								else
									sendLocalChat( "/Error, failed to delete friend..." );
							}
							else
								sendLocalChat( "/Error, failed to get friends..." );
						}
						else
							sendLocalChat( "/Usage: /friend delete [user name|@user id]" );
					}
					else if ( strnicmp<char>( sub, "removeselffrom", sub.length() ) == 0 )
					{
						usage = false;
						if ( arguments.argumentCount() == 3 )
						{
							CharString pattern( arguments.argument( 2 ) );

							if ( deleteSelfFromFriend( pattern ) > 0 )
								sendLocalChat( "/You are no longer on this users friendlist..." );
							else
								sendLocalChat( "/Error, command failed..." );
						}
						else
							sendLocalChat( "/Usage: /friend removeselffrom [user name|@user id]" );
					}
				}

				if ( usage )
					sendLocalChat( "/Usage: /friend [list|add|delete|removeselffrom] [user name|@user id]" );
			}
			break;
		case CMD_IGNORE:					// show/add/remove ignores
			{
				bool usage = true;
				if ( arguments.argumentCount() > 1 )
				{
					CharString sub( arguments.argument( 1 ) );
					if ( strnicmp<char>( sub, "list", sub.length() ) == 0 )
					{
						usage = false;
						Array< ShortProfile > ignores;
						if ( getIgnores( ignores ) > 0 )
						{
							CharString text;
							text += CharString().format( "/Found %d ignores...\n", ignores.size() );
							text += profileText( ignores );

							sendLocalChat( text );
						}
						else
							sendLocalChat( "/Error, failed to get ignores..." );
					}
					else if ( strnicmp<char>( sub, "add", sub.length() ) == 0 )
					{
						usage = false;
						if ( arguments.argumentCount() == 3 )
						{
							CharString pattern( arguments.argument( 2 ) );

							Array< ShortProfile > found;
							if ( findUserExactFirst( pattern, found ) )
							{
								if ( found.size() == 1 )
								{
									if ( addIgnore( found[0].userId ) > 0 )
										sendLocalChat( CharString().format("/Added '%s' to ignores...", found[0].name.cstr()) );
									else
										sendLocalChat( "/Error, failed to add to ignores..." );
								}
								else
									sendLocalChat( CharString().format( "/Error, found %d matches for '%s'...", found.size(), pattern.cstr() ) );
							}
							else
								sendLocalChat( CharString().format("/Error, failed to find user '%s'...", pattern.cstr()) );
						}
						else
							sendLocalChat( "/Usage: /ignore add [user name|@user id] " );
					}
					else if ( strnicmp<char>( sub, "delete", sub.length() ) == 0 )
					{
						usage = false;
						if ( arguments.argumentCount() == 3 )
						{
							CharString pattern( arguments.argument( 2 ) );
							pattern.lower();

							Array< ShortProfile > ignores;
							if ( getIgnores( ignores ) > 0 )
							{
								dword userId = 0;
								if ( pattern[0] != '@' )
								{
									for(int i=0;i<ignores.size();i++)
									{
										CharString name( ignores[i].name );
										name.lower();

										if ( strstr<char>( name, pattern ) != NULL )
										{
											userId = ignores[i].userId;
											break;
										}
									}
								}
								else
									userId = strtoul( pattern.buffer() + 1, NULL, 10 );

								if ( userId != 0 && deleteIgnore( userId ) > 0 )
									sendLocalChat( "/Ignore deleted..." );
								else
									sendLocalChat( "/Error, failed to delete ignore..." );
							}
							else
								sendLocalChat( "/Error, failed to get ignores..." );
						}
						else
							sendLocalChat( "/Usage: /ignore delete [user name|@user id]" );
					}
				}

				if ( usage )
					sendLocalChat( "/Usage: /ignore [list|add|delete] [user name|@user id]" );
			}
			break;
		case CMD_WHISPER:					// send private message to anther user in the room
			{
				if ( arguments.argumentCount() > 2 )
				{
					CharString who( arguments.argument( 1 ) );

					Array< ShortProfile > found;
					if ( findRoomUser( who, found ) )
					{
						if ( found.size() == 1 )
						{
							if( ( found[0].flags & MetaClient::HIDDEN ) == 0 || ( profile().flags & MetaClient::ADMINISTRATOR ) != 0 )
							{
								CommandLine withQuotes( pMessage, true );
								sendChatWhisper( withQuotes.restring( 2 ), found[0] );
							}
							else	// user is hidden, can't be seen. Give fake error message to prevent "whisper scanning" for hidden users
								sendLocalChat( CharString().format("/Error, failed to find '%s' in room...", who.cstr()) );
						}
						else
							sendLocalChat( CharString().format("/Error, found %d matches for '%s'...", found.size(), who.cstr()) );
					}
					else
						sendLocalChat( CharString().format("/Error, failed to find '%s' in room...", who.cstr()) );
				}
				else
					sendLocalChat( "/Usage: /whisper [user name|@user id] [message]" );
			}
			break;
		case CMD_INVALID:
			
			if ( arguments.argumentCount() > 0 )
			{
				CharString keyword( arguments.argument( 0 ) );
				if ( strnicmp<char>( "/me", keyword, 3 ) == 0 )	 		
					if ( profile().flags & MUTED )
						sendLocalChat( "/You are muted. No other players will see your messages." );
			}
			// no commands client side found, forward message onto server, don't local echo
			sendChat( roomId, 0, pMessage, false );
			break;
		}
	}
	else
	{
		if ( profile().flags & MUTED )
			sendLocalChat( "/You are muted. No other players will see your messages." );

		// normal chat
		sendChat( roomId, 0, pMessage );
	}
}

/**
 *  Broadcasts a message
 **/
void MetaClient::sendGlobalChat( const char * pMessage )
{
	sendChat( 0, 0, pMessage );
}

/**
 *  Sends a message to the players local chatscreen. (It´s not being sent to the server)
 **/
void MetaClient::sendLocalChat( const char * pMessage )
{
	sendLocalChat( 0, 0, pMessage );
}

/**
 *  Sends a message to the players local chatscreen. (It´s not being sent to the server)
 *  Set roomId and recpId to 0
 **/
void MetaClient::sendLocalChat( dword roomId, dword recpId, const char * pMessage )
{
	AutoLock lock( &m_Lock );

	Chat sentChat;
	sentChat.author = m_Profile.name;
	sentChat.authorId = m_Profile.userId;
	sentChat.roomId = roomId;
	sentChat.recpId = recpId;
	sentChat.time = Time::seconds() + m_ServerTimeDiff;	// So local timestamps fit those received from the server
	sentChat.text = pMessage;

	m_Chat.push( sentChat );
	m_ChatReceived.signal();
}


void MetaClient::sendChatWhisper( const char * pMessage, const ShortProfile & pSProfReceiver )
{
	CharString localMessageText;
	localMessageText.format( "/You whispered to <b>%s</b> : \"<b>%s</b>\"", 
			pSProfReceiver.name.cstr(), pMessage ) ;

	CharString remoteMessageText;
	remoteMessageText.format( "/<b>%s</b> whispered \"<b>%s</b>\"", 
			this->profile().name.cstr(), pMessage ) ;
	
	
	sendLocalChat( localMessageText );
	sendChat( 0, pSProfReceiver.userId, remoteMessageText, false );
}


void MetaClient::sendChatSend(const char *pMessage, const ShortProfile &pSProfReceiver)
{
	CharString localMessageText;
	localMessageText.format( "/You sent to <b>%s</b> : \"<b>%s</b>\"", 
			pSProfReceiver.name.cstr(), pMessage ) ;

	CharString remoteMessageText;
	remoteMessageText.format( "/<b>%s</b> sent \"<b>%s</b>\"", 
			this->profile().name.cstr(), pMessage ) ;
	
	
	sendLocalChat( localMessageText );
	sendChat( 0, pSProfReceiver.userId, remoteMessageText, false );
}


/**
 *  Clears the Chatmessages
 **/
void MetaClient::flushChat()
{
	AutoLock lock( &m_Lock );
	m_Chat.release();
}

/**
 *  Checks/Waits for new received Chatmessages
 *  block: blocking/nonblocking mode
 **/
bool MetaClient::waitChat( bool block /*= true*/)
{
	if ( block )
	{
		m_ChatReceived.wait();
		return true;
	}

	return m_ChatReceived.signaled();
}

//----------------------------------------------------------------------------
//EOF

/**
	@file WorldClientMessages.cpp

	(c)2006 Palestar Inc
	@author Richard Lyle @date 10/14/2006 3:15:44 PM
*/

#define MEDUSA_TRACE_ON			// force trace messages to stay on

#include "WorldClient.h"
#include "Debug/Assert.h"
#include "Standard/Time.h"

//! Any messages that take longer than this amount in time in milliseconds generate an warning to the log..
const dword MESSAGE_WARNING_TIME = 50;

//---------------------------------------------------------------------------------------------------

void WorldClient::receiveMessage( bool bUDP, u8 nMessage, const InStream & input )
{
	dword nStart = Time::milliseconds();

	switch( nMessage )
	{
	case CLIENT_RECV_VERSION:
		{
			dword job;
			input >> job;
			dword version;
			input >> version;
			dword clientId;
			input >> clientId;

			LOG_STATUS( "WorldClient", "CLIENT_RECV_VERSION, version = %u, clientId = %u", version, clientId );
			signalJob( job, version );

			// install a unique number provided based on our client ID now..
			UniqueNumber::setProvider( new PersistedUniqueNumberProvider( UNT_CLIENT, clientId ) );
		}
		break;
	case CLIENT_RECV_CK_DICT:
		{
			lock();
			input >> m_pDictionary->getStorage<ClassKey>( true );
			unlock();
		}
		break;
	case CLIENT_RECV_SERVER_STATUS:
		{
			ServerStatus status;
			input >> status;

			TRACE( CharString().format("CLIENT_RECV_SERVER_STATUS, status.name = %s", status.name.cstr() ) );

			lock();

			// notify the user
			user()->onServerStatus( status );
			// save the server status
			m_ServerStatus = status;

			unlock();
		}
		break;
	case CLIENT_SERVER_TRANSFER:
		{
			TransferRequest req;
			input >> req;
			
			if (! transfer( req ) )
				LOG_ERROR(  "WorldClient", "Failed to redirect to new server!" );
		}
		break;
	case CLIENT_LOGIN:
		{
			bool bSuccess;
			input >> bSuccess;
			
			TRACE( CharString().format("CLIENT_LOGIN, login = %s", bSuccess ? "Yes" : "No" ) );

			lock();

			m_LoggedIn = bSuccess;
			m_LoginEvent.signal();

			user()->onLogin( bSuccess );
			
			if ( m_bServerTransfer )
			{
				TRACE( "Login completed for server transfer..." );

				bool bSuccess = false;

				Noun * pSpawn = context()->findNoun( m_TransferReq.m_sSpawnPoint );
				if ( pSpawn )
					bSuccess = selectSelf( pSpawn->key(), m_TransferReq.m_nSelf, m_TransferReq.m_Items );

				if ( !bSuccess )
				{
					TRACE( "Failed to complete server transfer!" );
					user()->onDisconnected();
				}
			}

			unlock();
		}
		break;
	case CLIENT_RECV_QUEUEPOS:
		{
			int nQueuePosition;
			input >> nQueuePosition;
			int nQueueSize;
			input >> nQueueSize;

			TRACE( "CLIENT_RECV_QUEUEPOS" );

			lock();

			m_nQueuePosition = nQueuePosition;
			m_nQueueSize = nQueueSize;

			user()->onUpdateQueuePosition();

			unlock();
		}
		break;
	case CLIENT_SET_PROFILE:
		{
			Profile::Ref pProfile;
			input >> pProfile;
			
			if ( pProfile != NULL )
			{
				TRACE( CharString().format("CLIENT_SET_PROFILE, profile.alias = %s, profile.userId = %u", 
					pProfile->name(), pProfile->userId() ) );
			}
			else
			{
				TRACE( "CLIENT_SET_PROFILE received NULL profile!" );
			}

			lock();
			
			// notify the user
			user()->onProfile( pProfile );
			// save the player profile
			m_pProfile = pProfile;

			unlock();
		}
		break;
	case CLIENT_SET_FOCUS:
		{
			WidgetKey nFocus;
			input >> nFocus;

			TRACE( "CLIENT_SET_FOCUS" );

			lock();
			
			m_pFocus = nFocus;
			user()->onSetFocus( m_pFocus );

			unlock();
		}
		break;
	case CLIENT_SET_FOCUS_AREA:
		{
			float fFocusArea;
			input >> fFocusArea;

			TRACE( "CLIENT_SET_FOCUS_AREA" );

			lock();

			m_fFocusArea = fFocusArea;
			user()->onSetFocusArea( fFocusArea );

			unlock();
		}
		break;
	case CLIENT_SET_TARGET:
		{
			WidgetKey nTarget;
			input >> nTarget;

			TRACE( "CLIENT_SET_TARGET" );

			lock();
			
			m_pTarget = nTarget;
			user()->onSetTarget( m_pTarget );

			unlock();
		}
		break;
	case CLIENT_SET_SELF:
		{
			WidgetKey nSelf;
			input >> nSelf;

			TRACE( CharString().format("CLIENT_SET_SELF, nSelf = %s", nSelf.string().cstr()) );

			lock();
			
			Noun::wRef pSelf( nSelf );

			// turn on AI on old ship if any..
			if ( m_pSelf.valid() )
				m_pSelf->setUserId( 0 );

			// we may not have the noun yet, so setup a pointer and wait for the
			// pointers to change
			m_pSelf = nSelf;

			// notify our user of our new selves...
			if ( m_pSelf.valid() )
				m_pSelf->setUserId( userId() );

			onSetSelf( m_pSelf );
			user()->onSetSelf( m_pSelf );

			// ship finally in world, clear the server transfer flag if set..
			if ( m_bServerTransfer )
				m_bServerTransfer = false;

			unlock();
		}
		break;
	case CLIENT_SET_TEAM:
		{
			int nTeamId;
			input >> nTeamId;

			TRACE( CharString().format("CLIENT_SET_TEAM, nTeamId = %u", nTeamId ) );

			lock();

			// set the nTeamId for the player
			m_nTeamId = nTeamId;
			user()->onSetTeam( nTeamId );

			unlock();
		}
		break;
	case CLIENT_RECV_CHAT:
		{
			CharString message;
			input >> message;
			dword nFromId;
			input >> nFromId;

			// filter any incoming chat using the word filter from the meta client..
			if ( m_pMetaClient != NULL )
				m_pMetaClient->filterWords( message );

			LOG_DEBUG_HIGH( "WorldClient", "CLIENT_RECV_CHAT, message = %s, from = %u", message.cstr(), nFromId );
			receiveChat( message, nFromId );
		}
		break;
	case CLIENT_RECV_STORAGE:
		{
			WidgetWrap storage;
			input >> storage;

			TRACE( "CLIENT_RECV_STORAGE" );

			lock();

			m_pStorage = WidgetCast<UserStorage>( storage.unwrap() );
			if (! m_pStorage )
				m_pStorage = new UserStorage();		// we must ALWAYS have a storage object
			m_pStorage->postReceive();

			user()->onReceiveStorage( m_pStorage );

			unlock();
		}
		break;
	case CLIENT_ADD_STORAGE:
		{
			WidgetKey nKey;
			input >> nKey;
			Storage storage;
			input >> storage;

			TRACE( "CLIENT_ADD_STORAGE" );

			lock();

			if ( m_pStorage )
			{
				storage.instance.clearDictionary();

				m_pStorage->add( nKey, storage );
				// inform the user
				user()->onAddStorage( nKey );
			}

			unlock();
		}
		break;
	case CLIENT_DEL_STORAGE:			// delete item from storage
		{
			WidgetKey nKey;
			input >> nKey;

			TRACE( "CLIENT_DEL_STORAGE" );

			lock();

			user()->onDeleteStorage( nKey );
			// remove item from storage
			m_pStorage->remove( nKey );

			unlock();
		}
		break;

	//----------------------------------------------------------------------------

	case GROUP_UPDATE:		// sent by the server to add a group member
		{
			WidgetWrap group;
			input >> group;

			lock();

			m_pStorage->setGroup( WidgetCast<Group>( group.unwrap() ) );
			user()->onGroupUpdate( m_pStorage->group() );
			
			unlock();
		}
		break;
	case GROUP_DEL:		// sent by the server to remove the players group information
		{
			lock();
			
			user()->onGroupLeave();
			m_pStorage->setGroup( NULL );
			
			unlock();
		}
		break;
	case GROUP_INVITE:	// sent by the server/client to invite a player into a group
		{
			Noun::wRef pWho;
			input >> pWho;

			lock();

			user()->onGroupInvite( pWho );

			unlock();
		}
		break;

	//----------------------------------------------------------------------------

	case CONTEXT_INIT:
		{
			WidgetWrap worldContext;
			input >> worldContext;

			TRACE( CharString().format("CONTEXT_INIT, worldContext.size() = %u", worldContext.size()) );

			lock();

			// clear some pointers
			m_nTeamId = 0;

			Widget::Ref pUncasted = worldContext.unwrap();
			if ( pUncasted.valid() )
			{
				WorldContext * pUniverse = WidgetCast<WorldContext>( pUncasted.pointer() );
				if ( pUniverse != NULL )
					setContext( pUniverse );

				// de-reference the object before we unlock
				pUncasted = NULL;
			}

			// inform our user that the context is ready...
			user()->onContextReceived();
			// notify any blocking threads
			m_ContextReceived.signal();

			unlock();
		}
		break;
	case CONTEXT_END:
		{
			CharString sEndMessage;
			input >> sEndMessage;

			TRACE( "CONTEXT_END" );

			lock();
			
			// notify the user the game has ended
			user()->onContextEnd( sEndMessage );
			// stop and release the context
			m_pWorldContext->stop();

			// clear some pointers
			m_pSelf = NULL;
			m_pTarget = NULL;
			m_pFocus = NULL;
			m_nTeamId = 0;
			//m_Storage.release();

			unlock();
		}
		break;
	case CONTEXT_SET_NAME:
		{
			CharString sName;
			input >> sName;

			TRACE( "CONTEXT_SET_NAME" );
			lock();
			context()->setName( sName );
			unlock();
		}
		break;
	case CONTEXT_SET_DESC:
		{
			CharString sDesc;
			input >> sDesc;

			TRACE( "CONTEXT_SET_DESC" );

			lock();
			context()->setDescription( sDesc );
			unlock();
		}
		break;
	case CONTEXT_SET_TYPE:
		{
			int nType;
			input >> nType;

			TRACE( "CONTEXT_SET_DESC" );

			lock();
			context()->setType( nType );
			unlock();
		}
		break;
	case CONTEXT_SET_SCRIPT:
		{
			CharString pScript;
			input >> pScript;

			lock();
			context()->setScript( pScript );
			unlock();
		}
		break;
	case CONTEXT_ADD_TEAM:
		{
			WorldContext::Team team;
			input >> team;

			TRACE( "CONTEXT_ADD_TEAM" );

			lock();
			context()->addTeam( team );
			unlock();
		}
		break;
	case CONTEXT_SET_FLEET_FLAGS:
		{
			int nTeamId;
			input >> nTeamId;
			dword nFlags;
			input >> nFlags;

			TRACE( "CONTEXT_SET_FLEET_FLAGS" );

			lock();
			context()->setTeamFlags( nTeamId, nFlags );
			unlock();
		}
		break;
	case CONTEXT_DEL_FLEET:
		{
			int nTeamId;
			input >> nTeamId;

			TRACE( "CONTEXT_DEL_FLEET" );

			lock();
			context()->deleteTeam( nTeamId );
			unlock();
		}
		break;
	case CONTEXT_ADD_CONDITION:
		{
			WorldContext::Condition condition;
			input >> condition;

			TRACE( "CONTEXT_ADD_CONDITION" );

			lock();
			context()->addCondition( condition );
			unlock();
		}
		break;
	case CONTEXT_DEL_CONDITION:
		{
			int nConditionId;
			input >> nConditionId;

			TRACE( "CONTEXT_DEL_CONDITION" );

			lock();
			context()->deleteCondition( nConditionId );
			unlock();
		}
		break;
	case CONTEXT_SET_TIME_LIMIT:
		{
			dword nTimeLimit;
			input >> nTimeLimit;

			TRACE( "CONTEXT_SET_TIME_LIMIT" );

			lock();
			context()->setTimeLimit( nTimeLimit );
			unlock();
		}
		break;
	case CONTEXT_ADD_WORLD:
		{
			WidgetWrap world;
			input >> world;

			TRACE( "CONTEXT_ADD_WORLD" );

			lock();

			Widget::Ref uncasted = world.unwrap();
			if ( uncasted.valid() )
			{
				WorldContext * pWorld = WidgetCast<WorldContext>( uncasted.pointer() );
				if ( pWorld != NULL )
					context()->addWorld( pWorld );

				uncasted = NULL;
			}
			unlock();
		}
		break;
	case CONTEXT_DEL_WORLD:
		{
			WidgetKey nWorld;
			input >> nWorld;

			TRACE( "CONTEXT_DEL_WORLD" );

			lock();

			WorldContext::wRef pWorld( nWorld );
			if ( pWorld.valid() )
			{
				context()->deleteWorld( pWorld );
				pWorld = NULL;
			}

			unlock();
		}
		break;
	case CONTEXT_ADD_ZONE:
		{
			WidgetWrap zone;
			input >> zone;

			TRACE( "CONTEXT_ADD_ZONE" );

			lock();

			Widget::Ref uncasted = zone.unwrap();
			if ( uncasted.valid() )
			{
				NodeZone * pZone = WidgetCast<NodeZone>( uncasted.pointer() );
				if ( pZone != NULL )
					context()->addZone( pZone );

				uncasted = NULL;
			}

			unlock();
		}
		break;
	case CONTEXT_DEL_ZONE:
		{
			WidgetKey nZone;
			input >> nZone;

			TRACE( "CONTEXT_DEL_ZONE" );

			lock();

			NodeZone::wRef pZone = nZone;
			if ( pZone.valid() )
			{
				context()->deleteZone( pZone );
				pZone = NULL;
			}

			unlock();
		}
		break;
	case CONTEXT_ADD_NOUN:
		{
			WidgetKey nParent;
			input >> nParent;
			WidgetKey nNoun;
			input >> nNoun;
			WidgetWrap noun;
			input >> noun;

			lock();

			// Find any old version of the noun we are going to replace.. 
			Noun::Ref pOldNoun = Noun::wRef( nNoun ).pointer();

			// unwrap the noun from it's package
			Widget::Ref pUncasted = noun.unwrap();
			if ( pUncasted.valid() )
			{
				BaseNode::wRef pParent = nParent;
				if ( pParent.valid() )
				{
					Noun * pNoun = WidgetCast<Noun>( pUncasted.pointer() );
					if ( pNoun != NULL )
					{
						LOG_DEBUG_HIGH( "WorldClient", "CONTEXT_ADD_NOUN, Noun = %s, Class = %s, Parent = %s, Bytes = %d", 
							pNoun->name(), pNoun->propertyList()->className(), pParent->name(), noun.size() );

						pNoun->setSyncronized( true );
						pParent->attachNode( pNoun );

						// detach the old noun AFTER we've replaced it with a new noun..
						if ( pOldNoun.valid() )
						{
							pOldNoun->setSyncronized( false );
							pOldNoun->detachSelf();
						}
					}
					else
						LOG_ERROR( "WorldClient", "Was expecting a Noun class, but recieved something different (%s)", pUncasted->className() );
				}
				else	
				{
					LOG_STATUS( "WorldClient", "CONTEXT_ADD_NOUN NULL parent, requesting parent noun. nParent=%llu, nNoun=%llu, Class=%s", 
						nParent.m_Id, nNoun.m_Id, pUncasted->className() );
					send( SERVER_SEND_NOUN ) << nParent;
				}
			}
			else
				LOG_ERROR( "WorldClient", "CONTEXT_ADD_NOUN - Failed to unwrap noun!" );

			unlock();
		}
		break;
	case CONTEXT_UPDATE_NOUN:
		{
			WidgetKey nParent;
			input >> nParent;
			WidgetKey nNoun;
			input >> nNoun;
			WidgetWrap update;
			input >> update;

			lock();

			// find the existing object by ID..
			Noun::wRef pNoun = nNoun;
			if ( pNoun.valid() )
			{
				BaseNode::wRef pParent = nParent;
				if ( pParent.valid() )
				{
					if ( pNoun->parent() != pParent )
					{
						BaseNode * pOldParent = pNoun->parent();

						LOG_DEBUG_LOW( "WorldClient", "Transfering noun %p from parent %p to parent %p",
							pNoun.pointer(), pOldParent, pParent.pointer() );

						if ( pNoun->grabReference() )
						{
							if ( pOldParent != NULL )
							{
								// parent has changed, attach noun to it's new parent...
								pNoun->setSyncronized( false );
								pNoun->setTransfer( true );
								pOldParent->detachNodeSwap( pNoun );
							}

							pNoun->setSyncronized( true );
							pParent->attachNode( pNoun );
							pNoun->setTransfer( false );
							pNoun->releaseReference();
						}
						else
						{
							LOG_ERROR( "WorldClient", "Failed to grab reference to object for transfer." );
						}
					}
					else
					{
						// make sure the sync flag is true..
						pNoun->setSyncronized( true );
					}

					LOG_DEBUG_HIGH( "WorldClient", "CONTEXT_UPDATE_NOUN, Noun = %s, Class = %s, Parent = %s, Bytes = %d", 
						pNoun->name(), pNoun->propertyList()->className(), pParent->name(), update.size() );
					// unwrap the noun from it's package
					if ( update.unwrap( pNoun ) == NULL )
					{
						LOG_ERROR( "WorldClient", "Failed to unwrap noun, requesting noun %llu", nNoun.m_Id );
						// failed to unwrap, send a request to re-send the noun then..
						send( SERVER_SEND_NOUN ) << nNoun;
					}

					// release the reference on the parent while still inside the lock
					pParent = NULL;
				}
				else
				{
					LOG_DEBUG_LOW( "WorldClient", "CONTEXT_UPDATE_NOUN - NULL parent, requesting parent noun %llu!", nParent.m_Id );
					send( SERVER_SEND_NOUN ) << nParent;
				}
			}
			else
			{
				LOG_DEBUG_LOW( "WorldClient", "CONTEXT_UPDATE_NOUN - Failed to find noun to update, requesting noun %llu!", nNoun.m_Id );
				send( SERVER_SEND_NOUN ) << nNoun;
			}

			unlock();
		}
		break;
	case CONTEXT_DESYNC_NOUN:
		{
			WidgetKey nNoun;
			input >> nNoun;

			lock();

			TRACE( "CONTEXT_DESYNC_NOUN" );

			Noun::wRef pNoun = nNoun;
			if ( pNoun.valid() )
			{
				pNoun->setSyncronized( false );
				pNoun = NULL;
			}

			unlock();
		}
		break;
	case CONTEXT_DEL_NOUN:
		{
			WidgetKey nNoun;
			input >> nNoun;
			dword nLastTick;
			input >> nLastTick;

			lock();

			Noun::wRef pNoun = nNoun; //WidgetCast<Noun>( Widget::findWidget( nNoun ) );
			if ( pNoun.valid() )
			{
				TRACE( CharString().format("CONTEXT_DEL_NOUN, Noun = %s, Class = %s", 
					pNoun->name(), pNoun->propertyList()->className()) );

				pNoun->setTick( nLastTick, false );
				pNoun->setSyncronized( false );
				pNoun->detachVerbs();
				pNoun->detachSelf();

				pNoun = NULL;
			}
			else
			{
				TRACE( "CONTEXT_DEL_NOUN - Noun not found!" );
			}

			unlock();
		}
		break;
	case CONTEXT_ADD_VERB:
		{
			WidgetKey nTarget;
			input >> nTarget;
			WidgetWrap verb;
			input >> verb;

			//TRACE( CharString().format("CONTEXT_ADD_VERB, nTarget = %s, verb.size() = %d", nTarget.string(), verb.size()) );

			lock();

			Noun::wRef pNoun = nTarget; //WidgetCast<Noun>( Widget::findWidget( nTarget ) );
			if ( pNoun.valid() && pNoun->context() == context() )
			{
				Widget::Ref pUncasted = verb.unwrap();
				if ( pUncasted.valid() )
				{
					Verb * pVerb = WidgetCast<Verb>( pUncasted.pointer() );
					if ( pVerb != NULL )
					{
						LOG_DEBUG_LOW( "WorldClient", "CONTEXT_ADD_VERB, Target = %s, Verb = %s, Bytes = %d", 
							pNoun->name(), pVerb->propertyList()->className(), verb.size() );

						// set the clientId to 1, so canAttachVerb() allows this verb to be attached
						pVerb->setClientId( 1 );
						// attach the verb now
						pNoun->attachVerb( pVerb );
					}
					else
					{
						TRACE( "CONTEXT_ADD_VERB - object is not a verb!" );
					}
				}
				else
				{
					TRACE( "CONTEXT_ADD_VERB - Failed to unwrap verb!" );
				}

				pUncasted = NULL;
			}
			else
			{
#if defined(_DEBUG)
				Widget::Ref pWidget = verb.unwrap();
				LOG_DEBUG_MED( "WorldClient", "CONTEXT_ADD_VERB - Target not found for verb (%s)!", pWidget->factory()->className() );
#endif
			}
			pNoun = NULL;

			unlock();
		}
		break;
	case CONTEXT_CRON:
		{
			dword nTick;
			input >> nTick;

			//TRACE( "CONTEXT_CRON" );

			lock();
			
			// syncronize the worldContext tick
			context()->cron( nTick );
			
			unlock();
		}
		break;
	case CONTEXT_RUN_SCRIPT:
		{
			UniqueNumber nScriptId;
			input >> nScriptId;
			CharString sName;
			input >> sName;
			CharString sScript;
			input >> sScript;

			lock();

			TRACE( "CONTEXT_RUN_SCRIPT" );
			context()->runScript( nScriptId, sName, sScript );

			unlock();
		}
		break;
	case CONTEXT_KILL_SCRIPT:
		{
			UniqueNumber nScriptId;
			input >> nScriptId;

			lock();

			TRACE( "CONTEXT_KILL_SCRIPT" );
			context()->killScript( nScriptId );

			unlock();
		}
		break;

	//---------------------------------------------------------------------------------------------------

	case UDP_START_CONNECT:
		{
			dword nClientId;
			input >> nClientId;

			TRACE( CharString().format("UDP_START_CONNECT - nClientID = %u", nClientId) );
			if ( m_bEnableUDP )
				sendUDP( SERVER_CLIENTID, UDP_ACK_CONNECT ) << nClientId;
		}
		break;

	//----------------------------------------------------------------------------

	case PING:
		{
			dword nTime;
			input >> nTime;

			//TRACE( CharString().format("PING, time = %u", nTime) );

			if ( bUDP )
			{
				if ( m_bEnableUDP )
					sendUDP( SERVER_CLIENTID, PONG ) << nTime;
			}
			else
			{
				send( PONG ) << nTime;
			}
		}
		break;
	case PONG:
		{
			dword nTime;
			input >> nTime;

			// caluclate the latency, divide by 2 because we only want the one-way time
			if ( bUDP )
				m_nLatencyUDP = (Time::milliseconds() - nTime ) / 2;
			else
				m_nLatency = (Time::milliseconds() - nTime) / 2;
		}
		break;
	default:
		onMessage( bUDP, nMessage, input );
		break;
	}

	dword nElapsed = Time::milliseconds() - nStart;
	if ( nElapsed > MESSAGE_WARNING_TIME )
		LOG_WARNING( "WorldClient", CharString().format( "WARNING: Message 0x%0.2x from server took %u ms", nMessage, nElapsed ) );
}

void WorldClient::onMessage( bool bUDP, u8 nMessage, const InStream & input )
{}

//---------------------------------------------------------------------------------------------------
//EOF

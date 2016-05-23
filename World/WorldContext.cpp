/*
	WorldContext.cpp

	This object is used to hold the the worldContext state
	(c)1999 Palestar Development, Richard Lyle
*/


#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "File/FileDisk.h"
#include "Standard/Queue.h"
#include "Render3D/NodeFlare.h"
#include "Display/PrimitiveSetTransform.h"
#include "Network/LogClient.h"		// LogLevels
#include "Verb.h"
#include "Constants.h"
#include "WorldTimer.h"
#include "WorldContext.h"

#include <list>
#include <set>
#include <algorithm>

//#define BSP_RENDER			// option to use the BSP to render the worldContext
//#define RENDER_BSP_HULLS	// debug option to display the hulls of the BSP tree

//----------------------------------------------------------------------------

static Constant MAX_ZONE_SIZE( "MAX_ZONE_SIZE", 150000.0f );
static Constant ZONE_MIN_MAX_Y( "ZONE_MIN_MAX_Y", 20000.0f );
static Constant SLEEP_MODE_UPDATE_RATE( "SLEEP_MODE_UPDATE_RATE", 5.0f );

//---------------------------------------------------------------------------------------------------

bool		WorldContext::sm_bEnableHDR = true;
bool		WorldContext::sm_bUpdateHDR = false;
bool		WorldContext::sm_bEnableShadows = true;
int			WorldContext::sm_nMaxShadowLights = 4;

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WorldContext, Resource );
REGISTER_FACTORY_KEY( WorldContext, 4037375705062179934LL );

BEGIN_PROPERTY_LIST( WorldContext, Resource )
	ADD_TRANSMIT_PROPERTY( m_Tick );
	ADD_TRANSMIT_PROPERTY( m_nType );
	ADD_TRANSMIT_PROPERTY( m_Name );
	ADD_TRANSMIT_PROPERTY( m_Description );
	ADD_TRANSMIT_PROPERTY( m_sScript );
	ADD_TRANSMIT_PROPERTY( m_WorldExtents );
	ADD_TRANSMIT_PROPERTY( m_vPosition );
	ADD_TRANSMIT_PROPERTY( m_sAddress );
	ADD_TRANSMIT_PROPERTY( m_nPort );
	//ADD_TRANSMIT_PROPERTY( m_Worlds );
	ADD_TRANSMIT_PROPERTY( m_ZoneArray );
	ADD_PROPERTY_FLAGS( m_ZoneArray, PF_OBJECT );
	ADD_TRANSMIT_PROPERTY( m_ActiveZone );
	ADD_TRANSMIT_PROPERTY( m_ZoneTransform );
	ADD_TRANSMIT_PROPERTY( m_Conditions );
	ADD_TRANSMIT_PROPERTY( m_TimeLimit );
	ADD_TRANSMIT_PROPERTY( m_Teams );
	// deprecated
	ADD_PROPERTY( m_Zones );
END_PROPERTY_LIST();

WorldContext::WorldContext() : 
	m_Tick( 0 ), 
	m_nType( 0 ), 
	m_nNextTeamId( 1 ),
	m_ZoneTransform( 0,0,0 ), 
	m_pUser( nullUser() ), 
	m_TickSecond( 0 ), 
	m_NextConditionId( 1 ), 
	m_TimeLimit( TICKS_PER_HOUR * 2 ),
	m_vPosition( Vector3::ZERO )
{}

WorldContext::~WorldContext()
{
	stop();
}

//----------------------------------------------------------------------------

bool WorldContext::read( const InStream & input )
{
	if (! Resource::read( input ) )
		return false;
	
	// move zones from old HashArray<> into the new array..
	if ( m_Zones.size () > 0 )
	{
		TRACE("Coverting old zone hash into zone array!" );
		for(int i=0;i<m_Zones.size();++i)
			m_ZoneArray.push( m_Zones[i] );
		m_Zones.release();
	}

	for(int i=0;i<m_ZoneArray.size();)
	{
		NodeZone * pZone = m_ZoneArray[i];
		if (! pZone )
		{
			// zones may be NULL if this was serialized with the WorldContext::FF_ZONES flag
			m_ZoneArray.remove( i );
			continue;
		}
		++i;

		// BUG FIX: update the hulls on the Y axis
		BoxHull hull( pZone->hull() );
		hull.min.y = -ZONE_MIN_MAX_Y;
		hull.max.y = ZONE_MIN_MAX_Y;
		// update the node
		pZone->setHull( hull );
		// set the context
		pZone->setContext( this );
	}

	// set the next ID's
	for(int i=0;i<m_Teams.size();i++)
		m_nNextTeamId = Max( m_nNextTeamId, m_Teams[i].nTeamId + 1 );
	return true;
}

bool WorldContext::write( const OutStream & output )
{
	return Resource::write( output );
}

//---------------------------------------------------------------------------------------------------

void WorldContext::onAttachNoun( Noun * pNoun )
{
	if ( user() != NULL )
		user()->onAttachNoun( pNoun );
}

void WorldContext::onUpdateNoun( Noun * pNoun )
{
	if ( user() != NULL )
		user()->onUpdateNoun( pNoun );
}

void WorldContext::onDetachNoun( Noun * pNoun )
{
	if ( user() != NULL )
		user()->onDetachNoun( pNoun );
}

void WorldContext::onTransferNoun( Noun * pNoun, 
								NodeZone * pNewZone,
								NodeZone * pOldZone )
{
	if ( user() != NULL )
		user()->onTransferNoun( pNoun, pNewZone, pOldZone );
}

// this function is called usually by the WorldClient class to keep the worldContext centered near the camera
// see WorldClient::render()
void WorldContext::onWorldTransform( const Vector3 & vAdd )
{
	// inform the our user, so they can update any stored world positions
	if ( user() != NULL )
		user()->onWorldTransform( vAdd );
	// keep track of the total position moved
	m_ZoneTransform += vAdd;

	// update all zone positions
	for(int i=0;i<zoneCount();i++)
		zone( i )->onWorldTransform( vAdd );
	// transform proxy worlds as well
	for(int i=0;i<worldCount();++i)
		world( i )->onWorldTransform( vAdd );
}

bool WorldContext::onMergeUpdate( Noun * a_pUpdate, Noun * a_pTemplate )
{
	// merge data from pUpdateNoun into pNoun...
	a_pUpdate->setName( a_pTemplate->name() );
	a_pUpdate->setNounContext( a_pTemplate->nounContext() );
	a_pUpdate->setWorldPosition( a_pTemplate->worldPosition() );
	a_pUpdate->setWorldFrame( a_pTemplate->worldFrame() );
	
	a_pUpdate->detachAllTraits();
	for(int i=0;i<a_pTemplate->traitCount();++i)
		a_pUpdate->attachTrait( (Trait *)a_pTemplate->trait(i)->copy() );

	return true;
}

bool WorldContext::onMergeAttach( Noun * a_pAttach )
{
	// new noun, attach a clone (same key) into our context...
	return attachNoun( a_pAttach );
}

bool WorldContext::onMergeRemove( Noun * a_pDetach )
{
	TRACE( CharString().format("WorldContext::onMergeRemove() - Detaching noun %s", a_pDetach->name() ) );
	a_pDetach->setDetach( true );
	return true;
}

//---------------------------------------------------------------------------------------------------

bool WorldContext::start()
{
	if ( m_Timer.active() )
		return false;		// timer is already running!

	// load faciton ships
	initializeNounTemplates();
	// initialize the zones
	initializeZones();
	// set the timers current tick
	m_Timer.setTick( m_Tick );
	// start the timer
	m_Timer.start();
	m_TickSecond = 0;

	// start up the main script
	if ( m_sScript.length() > 0 )
		runScript( (qword)0, "Default", m_sScript );
	for(int i=0;i<m_Worlds.size();++i)
		m_Worlds[i]->start();

	if ( user()->isServer() )
	{
		// lock all zones with objects so they will all get updated...
		for(int k=0;k<zoneCount();++k)
		{
			NodeZone * pZone = zone( k );
			if ( pZone->childCount() > 0 )
				lockZone( zone( k ), true );
		}
	}

	return true;
}

bool WorldContext::stop()
{
	if (! m_Timer.active() )
		return false;		// timer isn't running
	
	// stop the timer
	m_Timer.stop();
	// reset the tick to 0
	m_Tick = 0;
	// release all running scripts
	releaseScripts();
	// release all zones
	for(int i=0;i<zoneCount();i++)
		zone(i)->releaseZone();
	// reset all conditions
	for(int i=0;i<m_Conditions.size();++i)
		m_Conditions[i].bActive = false;

	m_Worlds.release();
	m_Verbs.release();
	m_LockedZones.release();

	return true;
}

bool WorldContext::update()
{
	PROFILE_START( "WorldContext::update()" );

	if ( m_Timer.active() )
	{
		// reference this object, as checkCondition() may execute a condition and force a new WorldContext object
		// this way this object doesn't get deleted until we're done
		if (! grabReference() )
			return false;

		dword nCurrentTick = m_Timer.tick();
		if (! user()->isSleeping() )
		{
			while( nCurrentTick > m_Tick )
			{
				// update the current tick
				++m_Tick;
				// simulate all locked zones
				for(int i=0;i<m_LockedZones.size();i++)
					m_LockedZones[i]->simulate( m_Tick );
				// update the following once per second
				if ( (m_Tick - m_TickSecond) >= TICKS_PER_SECOND )
				{
					m_TickSecond = m_Tick;
					updateSecond();
				}
			}

			// simulate proxy worlds
			for(int i=0;i<m_Worlds.size();++i)
				m_Worlds[i]->update();
		}
		else
		{
			// sleep mode, update less often..
			dword nElaspedTicks = nCurrentTick - m_Tick;
			if ( nElaspedTicks > (dword)(TICKS_PER_SECOND * SLEEP_MODE_UPDATE_RATE) )
			{
				m_Tick += nElaspedTicks;
				m_TickSecond = m_Tick;

				// simulate all locked zones
				for(int i=0;i<m_LockedZones.size();i++)
					m_LockedZones[i]->simulate( m_Tick );

				updateSecond();

				// simulate proxy worlds
				for(int i=0;i<m_Worlds.size();++i)
					m_Worlds[i]->update();
			}

		}

		releaseReference();
	}

	PROFILE_END();

	return true;
}

void WorldContext::updateSecond()
{
	// update the script once per second
	updateScripts();

	// server side updates
	if ( isServer() )
	{
		// check for an end condition
		checkConditions();
	}
}

bool WorldContext::merge( WorldContext * a_pContext )
{
	if (! a_pContext )
		return false;
	if ( a_pContext->zoneCount() < 1 )
		return false;

	// get the new scenery for all zones from the first zone..
	Scene::Ref pScenery = a_pContext->zone( 0 )->scenery();

	// copy all the zones from the static context into this one.. so we have identical positions, hulls, and keys..
	mergeZones( a_pContext );

	setKey( a_pContext->key() );			// transfer the key as well -- this is IMPORTANT for the proxy system to work!
	setType( a_pContext->type() );
	setName( a_pContext->name() );
	setDescription( a_pContext->description() );
	setScript( a_pContext->script() );
	setPosition( a_pContext->position() );

	deleteAllConditions();
	for(int i=0;i<a_pContext->conditionCount();++i)
	{
		Condition condition( a_pContext->condition( a_pContext->conditionId( i ) ) );
		addCondition( condition );
	}

	// we don't delete teams.. we just merge the teams into this context..
	for(int i=0;i<a_pContext->teamCount();++i)
	{
		Team team( a_pContext->teamN( i ) );
		addTeam( team );
	}

	// make sure we match
	std::set< WidgetKey > NounKeys;
	for(int z=0;z<a_pContext->zoneCount();++z)
	{
		NodeZone * pZone = a_pContext->zone( z );
		for(int i=0;i<pZone->childCount();++i)
		{
			Noun * pNoun = WidgetCast<Noun>( pZone->child(i) );
			if (! pNoun )
				continue;

			NounKeys.insert( pNoun->key() );
			
			TRACE( CharString().format( "Merging noun %s", pNoun->name() ) );
			Noun * pUpdateNoun = findNounByKey( pNoun->key() );
			if (! pUpdateNoun )
			{
				Noun::Ref pClone = (Noun *)pNoun->clone();
				pClone->setWorldPosition( pNoun->worldPosition() );		// make sure clones world position is correct..

				// new noun, attach a clone (same key) into our context...
				if (! onMergeAttach( pClone ) )
				{
					LOG_STATUS( "WorldContext", CharString().format("Merge failed to attach noun %s into map!", pClone->name()) );
					return false;
				}
				continue;
			}

			// merge data from pUpdateNoun into pNoun...
			if (! onMergeUpdate( pUpdateNoun, pNoun ) )
			{
				LOG_STATUS( "WorldContext", "Merge failed to update noun %s!", pUpdateNoun->name() );
				return false;
			}
		}
	}

	// remove nouns...
	for(int z=0;z<zoneCount();++z )
	{
		NodeZone * pZone = zone( z );
		for(int i=0;i<pZone->childCount();++i)
		{
			Noun * pNoun = WidgetCast<Noun>( pZone->child(i) );
			if (! pNoun )
				continue;

			if ( NounKeys.find( pNoun->key() ) == NounKeys.end() )
			{
				//LOG_STATUS( "WorldContext", CharString().format( "Merge is removing noun %s!", pNoun->name()) );
				if (! onMergeRemove( pNoun ) )
				{
					LOG_STATUS( "WorldContext", CharString().format("Merge failed to remove noun %s!", pNoun->name()) );
					return false;
				}
			}
		}
	}

	return true;
}

void WorldContext::render( RenderContext & context, const Matrix33 & frame, const Vector3 & translate, bool bProxy /*= false*/ )
{
	DisplayDevice * pDisplay = context.display();
	ASSERT( pDisplay );

	// no zones in this context, so nothing to render
	if ( zoneCount() < 1 )
		return;	

	if (! bProxy )
	{
		if ( sm_bEnableHDR )
		{
			if ( sm_bUpdateHDR )
			{
				m_pHDR = NULL;
				sm_bUpdateHDR = false;
			}

			if (! m_pHDR.valid() )
				m_pHDR = pDisplay->createEffect( "HDR" );
		
			// push our bloom effect
			if ( m_pHDR.valid() )
				pDisplay->push( m_pHDR );
			else
				sm_bEnableHDR = false;		// failed to create the HDR effect, disable it..
		}
		else
		{
			if ( m_pHDR.valid() )
				m_pHDR = NULL;		
		}

		// disable lens flares if HDR is enabled..
		NodeFlare::sm_bFlareEnabled = !sm_bEnableHDR;

		PROFILE_START( "Rendering scenery" );

		NodeZone * pZone = activeZone();
		if ( pZone != NULL )
		{
			// render the scenery first
			Scene * pScenery = pZone->scenery( false );
			if ( pScenery != NULL )
			{
				// disable the fog
				pDisplay->disableFog();
				pScenery->render( context, Matrix33::IDENTITY, context.position() ); 
			}
		}

		PROFILE_END();

		context.setMaxShadowLights( WorldContext::sm_bEnableShadows ? WorldContext::sm_nMaxShadowLights : 0 );
		if ( context.beginScene() )
		{
			PROFILE_START( "Rendering Zones" );

			renderZones( context, frame, translate );

			// render other worlds attached to this world..
			PROFILE_START( "Rendering Proxies" );
			for(int i=0;i<m_Worlds.size();++i)
				m_Worlds[i]->render( context, frame, translate, true );
			PROFILE_END();

			// enumerate each light, render the scene from the perspective of the light..
			while( context.beginShadowPass() )
			{
				renderZones( context, frame, translate );
				context.endShadowPass();
			}

			context.endScene();
		}
	}
	else
	{
		// proxy render, just render the zones..
		renderZones( context, frame, translate );
	}

	PROFILE_END();
}

void WorldContext::renderZones( RenderContext & context, const Matrix33 & frame, const Vector3 & translate )
{
	for(int i=0;i<m_ZoneArray.size();i++)
	{
		NodeZone * pZone = m_ZoneArray[i];
		if ( pZone->childCount() > 0 )
			pZone->preRender( context, frame, translate );
	}
}

//---------------------------------------------------------------------------------------------------

void WorldContext::cron( dword nTick )
{
	// inform the timer of the current tick, it will adjust over time
	m_Timer.cron( nTick );
}


bool WorldContext::lockZone( NodeZone * pZone, bool bSetTick /*= true*/ )
{
	if ( pZone == NULL )
		return false;		// invalid pointer
	if ( pZone->context() != this )
		return false;		// not attached to this context
	if ( pZone->locked() )
		return false;		// zone is already locked
	
	// set the zone to locked
	pZone->setLocked( true );

	// set the tick of all nouns in the zone to the current tick, otherwise the CPU will get slammed
	// if it tries to update a zone with a bunch of very old objects
	if ( bSetTick )
		pZone->setTick( tick() );

	// add the zone to our locked zones
	m_LockedZones.push( pZone );
	return true;
}

bool WorldContext::unlockZone( NodeZone * pZone )
{
	if ( pZone == NULL )
		return false;		// invalid pointer
	if ( pZone->context() != this )
		return false;		// not attached to this context
	if (! pZone->locked() )
		return false;		// zone already unlocked

	// turn of the locked flag
	pZone->setLocked( false );
	// remove the zone from our list
	m_LockedZones.removeSearchSwap( pZone );

	return true;
}

//----------------------------------------------------------------------------

void WorldContext::setUser( User * pUser )
{
	if ( pUser != NULL )
	{
		m_pUser = pUser;
		m_pUser->m_wpContext = this;
	}
	else
		m_pUser = nullUser();
}

void WorldContext::setTick( dword nTick )
{
	m_Tick = nTick;
	m_TickSecond = 0;
	// set the tick on the timer
	m_Timer.setTick( m_Tick );
	// set the tick on all zones
	for(int i=0;i<zoneCount();i++)
		zone( i )->setTick( nTick );
}

void WorldContext::setType( int nType )
{
	m_nType = nType;
}

void WorldContext::setName( const char * name )
{
	m_Name = name;
}

void WorldContext::setDescription( const char * description )
{
	m_Description = description;
}

void WorldContext::setScript( const char * pScript )
{
	releaseScripts();

	m_sScript = pScript;
	if ( m_sScript.length() > 0 )
		runScript( (qword)0, "Default", m_sScript );
}

void WorldContext::calculateWorldExtents()
{
	m_WorldExtents = BoxHull();
	for(int i=0;i<worldCount();++i)
		m_WorldExtents.addBox( world(i)->worldExtents() );
	for(int i=0;i<m_ZoneArray.size();i++)
		m_WorldExtents.addBox( m_ZoneArray[i]->worldHull() );
}

void WorldContext::setPosition( const Vector3 & a_vPosition )
{
	m_vPosition = a_vPosition;
}

void WorldContext::setAddress( const char * pAddress, u16 nPort )
{
	m_sAddress = pAddress;
	m_nPort = nPort;
}

void WorldContext::clearAddress()
{
	m_sAddress.empty();
	m_nPort = 0;
}

//----------------------------------------------------------------------------

int WorldContext::addWorld( WorldContext * pContext )
{
	if ( pContext == this )
		return -1;
	if ( m_Worlds.search( pContext ) >= 0 )
		return -1;
	if ( pContext->m_Worlds.search( this ) >= 0 )
		return -1;		// don't allow circular relationships..

	m_Worlds.push( pContext );
	if ( pContext->m_pParent )
		pContext->m_pParent->deleteWorld( pContext );		// remove from any previous context
	pContext->m_pParent = this;

	user()->onAddWorld( pContext );

	// start the world 
	pContext->start();
	// update our world extents
	calculateWorldExtents();

	return m_Worlds.size() - 1;
}

bool WorldContext::deleteWorld( WorldContext * pContext )
{
	int nRemove = m_Worlds.search( pContext );
	if ( nRemove < 0 )
		return false;

	ASSERT( this == pContext->m_pParent );
	user()->onDeleteWorld( pContext );

	pContext->m_pParent = NULL;
	m_Worlds.removeSwap( nRemove );
	
	return true;
}

void WorldContext::clearWorlds()
{
	m_Worlds.release();
}

NodeZone * WorldContext::createZone()
{
	return new NodeZone();
}

bool WorldContext::createZones( const BoxHull & world, Scene * a_pScenery )
{
	typedef std::list< NodeZone::Ref >		ZoneList;

	ZoneList RemoveZones;
	for(int i=0;i<m_ZoneArray.size();i++)
		RemoveZones.push_back( m_ZoneArray[i] );

	// create the zones based on the size of the entire universe
	// build the new zones
	Queue< BoxHull > hulls;
	// push the root hull, using a queue instead of recursion to sub-divide the hull until they are below the zoneSize
	hulls.push( world );
	while( hulls.valid() )
	{
		BoxHull & hull = *hulls;

		// make sure all zones are symmetric on the Y, so the center is 0 on the Y...
		hull.max.y = ZONE_MIN_MAX_Y;
		hull.min.y = -ZONE_MIN_MAX_Y;

		// get the lengths of the hull on the X and Z axis
		float x = hull.lengthX();
		float z = hull.lengthZ();

		if ( x > MAX_ZONE_SIZE && x > z )
		{
			// split this hull along the x axis
			Vector3 center( hull.center() );

			BoxHull left( hull );
			left.max.x = center.x;
			BoxHull right( hull );
			right.min.x = center.x;

			hulls.push( left );
			hulls.push( right );
		}
		else if ( z > MAX_ZONE_SIZE )
		{
			// split this hull along the z axis
			Vector3 center( hull.center() );
			
			BoxHull top( hull );
			top.min.z = center.z;
			BoxHull bottom( hull );
			bottom.max.z = center.z;

			hulls.push( top );
			hulls.push( bottom );
		}
		else
		{
			TRACE( CharString().format( "WorldContext::createZones() - Adding zone at %s, hull = %s/%s",
				ConvertType<CharString>( hull.center() ).cstr(), 
				ConvertType<CharString>( hull.min ).cstr(), 
				ConvertType<CharString>( hull.max ).cstr() ) );

			// add the zone
			NodeZone::Ref pZone = createZone();
			pZone->setScenery( a_pScenery );
			pZone->setPosition( hull.center() );
			pZone->setHull( BoxHull( hull, -hull.center() ) );

			addZone( pZone );
		}

		// next hull
		hulls.pop();
	}

	// now remove all the old zones... all nouns will be moved into the new zones..
	for ( ZoneList::iterator iZone = RemoveZones.begin(); iZone != RemoveZones.end(); ++iZone )
		deleteZone( *iZone );

	return true;
}

bool WorldContext::mergeZones( WorldContext * a_pContext )
{
	// make a copy of our current zones for removal after we added all the new ones..
	ZoneArray removeZones( m_ZoneArray );

	for(int i=0;i<a_pContext->zoneCount();++i)
	{
		NodeZone * pZone = a_pContext->zone(i);

		NodeZone::Ref pZoneClone = createZone();
		pZoneClone->setKey( pZone->key() );
		pZoneClone->setScenery( pZone->scenery() );
		pZoneClone->setPosition( pZone->position() );
		pZoneClone->setHull( pZone->hull() );

		addZone( pZoneClone );
	}

	// now remove all the old zones... all nouns will be moved into the new zones..
	for(int i=0;i<removeZones.size();++i)
		deleteZone( removeZones[i] );

	return true;
}

bool WorldContext::addZone( NodeZone * pZone )
{
	if ( pZone == NULL )
		return false;		// invaid pointer
	if ( pZone->context() == this )
		return false;		// already attached to this context

	m_ZoneArray.push( pZone );
	pZone->initializeZone( this );
	pZone->setPosition( pZone->position() + m_ZoneTransform );		// offset the position by the current zone shift
	pZone->setTick( tick() );

	// update the worldContext extents
	calculateWorldExtents();
	// inform the user of the new zone
	user()->onAddZone( pZone );

	LOG_DEBUG_HIGH( "WorldContext::addZone, name = %s, key = %s", pZone->name(), pZone->key().string().cstr() );

	return true;
}

bool WorldContext::deleteZone( NodeZone * pZone )
{
	LOG_DEBUG_HIGH( "WorldContext::deleteZone, name = %s, key = %s", pZone->name(), pZone->key().string().cstr() );

	if ( pZone == NULL )
		return false;		// invaid pointer
	if ( pZone->context() != this )
		return false;		// already not attached to this context

	// close the zone, which will move any contained nouns into any zone containing the same area...
	pZone->close();
	// unlock the zone first, if it's already locked
	if ( pZone->locked() )
		unlockZone( pZone );
	// release the zone now
	pZone->releaseZone();

	// inform our user we are removing a zone
	user()->onDeleteZone( pZone );
	// clear the context
	pZone->setContext( NULL );
	// remove from the zone list
	m_ZoneArray.removeSearch( pZone );
	
	// clear the active zone if it's just been deleted
	if ( pZone == m_ActiveZone )
		m_ActiveZone = NULL;

	return true;
}

bool WorldContext::setActiveZone( const Vector3 & worldPosition )
{
	NodeZone * pZone = activeZone();
	if ( pZone == NULL || !pZone->worldHull().inBox( worldPosition ) )
	{
		NodeZone * pNewZone = NULL;

		// either no active zone or we are outside the active zone
		for(int i=0;i<zoneCount() && pNewZone == NULL;i++)
		{
			NodeZone * pCheck = zone( i );
			if ( pCheck->worldHull().inBox( worldPosition ) )
				pNewZone = pCheck;
		}

		if ( pNewZone != NULL )
		{
			// found correct zone
			pZone = pNewZone;
			// set the new active zone
			setActiveZone( pZone );

			// zone changed
			return true;
		}
	}

	// zone not changed
	return false;
}

void WorldContext::setActiveZone( NodeZone * pZone )
{
	if ( pZone != NULL )
	{
		// assert this zone is from this context
		ASSERT( pZone->context() == this );

		if ( pZone != m_ActiveZone )
		{
			// set the active zone
			m_ActiveZone = pZone;
			// update the zone positions, so our current zone's position is at 0,0,0
			onWorldTransform( -pZone->position() );
		}
	}
	else
		m_ActiveZone = NULL;
}

bool WorldContext::attachNoun( Noun * pNoun, BaseNode * pParent /*= NULL*/ )
{
	ASSERT( pNoun );					// check for valid pointer

	NodeZone * pZone = NULL;
	if ( pParent == NULL )
	{
		// no parent is provided, noun position should be in world space so we can determine which zone to attach the noun
		Vector3 worldPosition( pNoun->position() );
	
		// first check the currently active zone to save some time since 95% of all nouns will be attached to the currently active zone
		if ( m_ActiveZone.valid() && m_ActiveZone->worldHull().inBox( worldPosition ) )
		{
			// set the zone pointer
			pZone = m_ActiveZone;
			// transform into space of the zone
			pNoun->setPosition( pNoun->position() - pZone->position() );
		}

		// find which zone to attach this noun based on which zone hull it resides
		for(int i=0;pZone == NULL && i<zoneCount();i++)
		{
			NodeZone * pCheck = zone( i );
			ASSERT( pCheck );

			if ( pCheck->worldHull().inBox( worldPosition ) )
			{
				// found the zone, stop searching
				pZone = pCheck;
				// transform the nouns position
				pNoun->setPosition( pNoun->position() - pZone->position() );
			}
		}
	}
	else if ( WidgetCast<NodeZone>( pParent ) )
		pZone = (NodeZone *)pParent;
	else if ( WidgetCast<Noun>( pParent ) )
		pZone = ((Noun *)pParent)->zone();

	if ( pZone != NULL )
	{
		pZone->attachNoun( pNoun, pParent );
		return true;
	}

	return false;
}

bool WorldContext::replaceNoun( Noun * pReplace, Noun * pWith )
{
	ASSERT( pReplace );
	ASSERT( pWith );

	NodeZone * pZone = pReplace->zone();
	if ( pZone != NULL )
		return pZone->replaceNoun( pReplace, pWith );
	return false;
}

void WorldContext::detachNoun( Noun * pNoun )
{
	ASSERT( pNoun );

	NodeZone * pZone = pNoun->zone();
	if ( pZone != NULL )
		pZone->detachNoun( pNoun );
}

bool WorldContext::canAttachVerb( Noun * pNoun, Verb * pVerb )
{
	return user()->canAttachVerb( pNoun, pVerb );
}

void WorldContext::onAttachVerb( Verb * pVerb )
{
	user()->onAttachVerb( pVerb );
}

//----------------------------------------------------------------------------

bool WorldContext::addCondition( Condition & condition )
{
	if ( condition.conditionId == 0 )
		condition.conditionId = m_NextConditionId++;

	m_Conditions.insert( condition.conditionId, condition );
	user()->onAddCondition( condition );

	return true;
}

bool WorldContext::deleteCondition( int conditionId )
{
	if (! isConditionValid( conditionId ) )
		return false;

	user()->onDeleteCondition( conditionId );
	m_Conditions.removeByKey( conditionId );

	return true;
}

bool WorldContext::deleteAllConditions()
{
	for( int i=0;i<m_Conditions.size(); ++i)
		user()->onDeleteCondition( m_Conditions.key( i ) );
	m_Conditions.release();
	return true;
}

void WorldContext::setTimeLimit( dword limit )
{
	m_TimeLimit = limit;
}

int WorldContext::checkConditions()
{
	int bestCondition = -1;
	int bestObjectives = -1;

	for(int i=0;i<m_Conditions.size();i++)
	{
		Condition & condition = m_Conditions[ i ];

		int objectiveCount = 0;

		for(int j=0;j<condition.objectives.size();j++)
			if ( checkObjective( condition.objectives[j] ) )
				objectiveCount++;

		if ( objectiveCount == condition.objectives.size() )
		{
			executeCondition( condition.conditionId );				// all objectives for this condition are true!
			return condition.conditionId;
		}
		
		if ( objectiveCount > bestObjectives )
		{
			bestCondition = condition.conditionId;
			bestObjectives = objectiveCount;
		}
	}

	// if time limit is exceeded, then execute the best condition
	if ( m_TimeLimit > 0 && m_Tick > m_TimeLimit && bestCondition >= 0 )
	{
		executeCondition( bestCondition );
		return bestCondition;
	}

	return 0;
}

void WorldContext::executeCondition( int conditionId )
{
	if ( isConditionValid( conditionId ) )
	{
		Condition & execute = m_Conditions.hash()[ conditionId ]; //condition( conditionId );
		if (! execute.bActive )
		{
			// set this condition active flag so we don't try to activate it again..
			execute.bActive = true;

			// execute script for condition
			if ( execute.script.length() > 0 )
				runScript( (qword)0, "Default", execute.script );

			// notify the user the condition is true
			user()->onCondition( conditionId );
		}
	}
}

bool WorldContext::checkObjective( const Objective & objective )
{
	return false;
}

bool WorldContext::addTeam( Team & team )
{
	if ( team.factionId < 0 || team.factionId > 31 )
		return false;
	if ( team.nTeamId == 0 )
		team.nTeamId = m_nNextTeamId++;

	// add the team
	m_Teams.insert( team.nTeamId, team );
	// make sure all templates are loaded
	if ( team.pTemplates.valid() )
	{
		Collection::Link pCollection = team.pTemplates;
		pCollection->load();		// load all resources

		for(int i=0;i<pCollection->resourceCount();i++)
		{
			NounTemplate::Link pContext( pCollection->resource( i ) );
			if ( pContext.valid() )
				pContext->noun();		// force Scene containing the Noun to get loaded
		}
	}
	user()->onAddTeam( team );

	return true;
}

void WorldContext::setTeamFlags( int nTeamId, dword nFlags )
{
	TeamHash::Iterator iFleet = m_Teams.find( nTeamId );
	if ( iFleet.valid() )
		(*iFleet).flags = nFlags;
}

bool WorldContext::deleteTeam( int nTeamId )
{
	if (! isTeamValid( nTeamId ) )
		return false;
	
	user()->onDeleteTeam( nTeamId );
	m_Teams.removeByKey( nTeamId );

	return true;
}

//----------------------------------------------------------------------------

void WorldContext::pushChat( const char * pChat )
{
	user()->onChat( NULL, pChat );
}

//----------------------------------------------------------------------------

bool WorldContext::canJoin( int nTeamId )
{
	return (team( nTeamId ).flags & TF_NOJOIN) == 0;
}

bool WorldContext::canSpawn( int nTeamId )
{
	return (team( nTeamId ).flags & TF_NOSPAWN) == 0;
}

bool WorldContext::proximityCheck( const Vector3 & vOrigin, float fRadius, Array< NounCollision > & nouns )
{
	for(int i=0;i<m_ZoneArray.size();i++)
	{
		NodeZone * pZone = m_ZoneArray[i];
		ASSERT( pZone );

		if ( ! pZone->worldHull().intersect( vOrigin, fRadius ) )
			continue;		// zone does not intersect the area we are checking, continue onto the next zone
		pZone->proximityCheck( vOrigin, fRadius, nouns );
	}

	// return true if any nouns are in the area
	return nouns.size() > 0;
}

bool WorldContext::proximityCheck( const Vector3 & vOrigin, float fRadius, Array< NounCollision > & nouns, const ClassKey & nType )
{
	for(int i=0;i<m_ZoneArray.size();i++)
	{
		NodeZone * pZone = m_ZoneArray[i];
		ASSERT( pZone );

		if ( ! pZone->worldHull().intersect( vOrigin, fRadius ) )
			continue;		// zone does not intersect the area we are checking, continue onto the next zone
		pZone->proximityCheck( vOrigin, fRadius, nouns, nType );
	}

	// return true if any nouns are in the area
	return nouns.size() > 0;
}

bool WorldContext::proximityCheckLocked( const Vector3 & vOrigin, float fRadius, Array< NounCollision > & nouns )
{
	for(int i=0;i<m_ZoneArray.size();i++)
	{
		NodeZone * pZone = m_ZoneArray[i];
		ASSERT( pZone );
		
		if ( ! pZone->locked() )
			continue;		// zone isn't locked, skip
		if ( ! pZone->worldHull().intersect( vOrigin, fRadius ) )
			continue;		// zone does not intersect the area we are checking, continue onto the next zone

		pZone->proximityCheck( vOrigin, fRadius, nouns );
	}

	// return true if any nouns are in the area
	return nouns.size() > 0;
}

//----------------------------------------------------------------------------

int WorldContext::teamClan( dword nClanId, int nFactionId )
{
	if ( nClanId != 0 )
	{
		for(int i=0;i<teamCount();i++)
			if ( teamN( i ).clanId == nClanId && teamN( i ).factionId == nFactionId )
				return teamId( i );
	}

	// no team found for that clanId
	return 0;
}

int WorldContext::teamFaction( int nTeamId )
{
	if ( isTeamValid( nTeamId ) )
		return team( nTeamId ).factionId;

	return 0;
}

int WorldContext::factionTeam( int factionId )
{
	for(int i=0;i<teamCount();i++)
	{
		const Team & team = teamN( i );
		if ( team.factionId == factionId )
			return team.nTeamId;
	}

	return 0;
}

bool WorldContext::findNouns( Array<Noun::Ref> & a_Nouns,
								ClassKey a_nKey /*= NULL_CLASS*/,
								int a_nTeamID /*= -1*/,
								int a_nFactionID /*= -1*/,
								bool a_bRecursive /*= false*/ )
{
	for(int i=0;i<m_ZoneArray.size();i++)
	{
		NodeZone * pZone = m_ZoneArray[i];

		std::list< BaseNode * > children;
		for(int j=0;j<pZone->childCount();++j)
			children.push_back( pZone->child(j) );

		while( children.size() > 0 )
		{
			BaseNode * pChild = children.front();
			children.pop_front();

			if (! pChild )
				continue;

			if ( a_bRecursive )
			{
				for(int k=0;k<pChild->childCount();++k)
					children.push_back( pChild->child(k) );
			}

			Noun * pNoun = WidgetCast<Noun>( pChild );
			if (! pNoun )
				continue;
			if ( a_nKey != NULL_CLASS && !pNoun->factory()->isType( a_nKey ) )
				continue;
			if ( a_nTeamID >= 0 && pNoun->teamId() != a_nTeamID )
				continue;
			if ( a_nFactionID >= 0 && pNoun->factionId() != a_nFactionID )
				continue;

			a_Nouns.push( pNoun );
		}
	}

	return a_Nouns.size() > 0;
}


Noun * WorldContext::findNoun( const char * pName )
{
	// search zones in this world first..
	for(int i=0;i<m_ZoneArray.size();i++)
	{
		NodeZone * pZone = m_ZoneArray[i];

		Noun * pNoun = WidgetCast<Noun>( pZone->findNode( pName ) );
		if ( pNoun != NULL )
			return pNoun;
	}

	// search proxy worlds as well..
	for(int j=0;j<m_Worlds.size();++j)
	{
		WorldContext * pWorld = m_Worlds[j];
		Noun * pNoun = pWorld->findNoun( pName );
		if ( pNoun != NULL )
			return pNoun;
	}

	return NULL;
}

// NOTE that we are searching for noun by it's key this way rather than using the Widget::findWidget() function... 
// this is done because the merge() routine needs to find a noun in this context that may have been replaced in the hash by another widget with
// the same key.
Noun * WorldContext::findNounByKey( const WidgetKey & a_nKey )
{
	for(int i=0;i<m_ZoneArray.size();i++)
	{
		NodeZone * pZone = m_ZoneArray[i];

		Noun * pNoun = WidgetCast<Noun>( pZone->findNode( a_nKey ) );
		if ( pNoun != NULL )
			return pNoun;
	}

	return NULL;
}

NounTemplate * WorldContext::findNounTemplate( Noun * pNoun )
{
	if (! pNoun )
		return NULL;

	for(int f=0;f<m_Teams.size();f++)
	{
		Collection::Link pCollection = m_Teams[f].pTemplates;
		if ( pCollection.valid() )
		{
			for(int i=0;i<pCollection->resourceCount();i++)
			{
				NounTemplate::Link pContext = pCollection->resource( i );
				if ( pContext.valid() && pContext->noun() != NULL && pContext->noun()->nounContext() == pNoun->nounContext() )
					return pContext;
			}
		}
	}

	return NULL;
}

NodeZone * WorldContext::findZone( const Vector3 & vWorldPosition )
{
	NodeZone * pZone = activeZone();
	if ( pZone != NULL && pZone->worldHull().inBox( vWorldPosition ) )
		return pZone;
	
	for(int i=0;i<zoneCount();i++)
	{
		pZone = zone( i );
		if ( pZone->worldHull().inBox( vWorldPosition ) )
			return pZone;
	}

	return NULL;
}

//----------------------------------------------------------------------------

bool WorldContext::saveContext( const char * pFileName, WorldContext * pContext )
{
	FileDisk::Ref pFile = new FileDisk();
	if ( pFile->open( pFileName, FileDisk::WRITE ) )
	{
		OutStream output( pFile );
		output << pContext;

		return true;
	}
	
	return false;
}

WorldContext * WorldContext::loadContext( const char * pFileName )
{
	WorldContext * pContext = NULL;

	// open the file
	FileDisk::Ref pFile = new FileDisk();
	if ( pFile->open( pFileName ) )
	{
		InStream input( pFile );
		input >> pContext;
	}

	return pContext;
}

WorldContext::User * WorldContext::nullUser()
{
	static WorldContext::User NULL_USER;
	return &NULL_USER;
}

//----------------------------------------------------------------------------

void WorldContext::initializeZones()
{
	for(int i=0;i<m_ZoneArray.size();i++)
		m_ZoneArray[i]->initializeZone( this );
}

void WorldContext::initializeNounTemplates()
{
	for(int i=0;i<m_Teams.size();i++)
	{
		Team & team = m_Teams[ i ];
		if ( team.pTemplates.valid() )
		{
			Collection * pCollection = team.pTemplates;
			pCollection->load();

			for(int i=0;i<pCollection->resourceCount();i++)
			{
				NounTemplate::Link pContext = pCollection->resource( i );
				if ( pContext.valid() )
					pContext->noun();		// force Scene containing the noun to get loaded
			}
		}
	}
}

//----------------------------------------------------------------------------
// EOF

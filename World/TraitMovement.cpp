/**
	@file TraitMovement.cpp

	(c)2006 Palestar Inc
	@author Richard Lyle @date 5/4/2007 8:25:14 PM
*/

#include "VerbMovement.h"
#include "TraitMovement.h"
#include "Constants.h"
#include "Noun.h"
#include "WorldContext.h"

#include "Debug/Assert.h"

#define ENABLE_MOVEMENT_TRACE		0

//---------------------------------------------------------------------------------------------------

const float		SMOOTH_POSITION_TIME = 0.5f;
const float		SMOOTH_ORIENATION_TIME = 0.5f;
// Minimal number of ticks between VerbMovement() packets..
const dword		MIN_SEND_MOVEMENT_TICKS = TICKS_PER_SECOND * 0.25f;
// Maximum amount of time before we will send a movement packet no matter what..
const dword		MAX_SEND_MOVEMENT_TICKS = TICKS_PER_SECOND * 5.0f;		
// use movement prediction if we received two movement messages within this amount of time...
const dword		USE_PREDICTION_TICKS = MAX_SEND_MOVEMENT_TICKS + (TICKS_PER_SECOND * 1.0f);
// Minimal number of ticks between global movement packets
const dword		MIN_GLOBAL_MOVEMENT_TICKS = TICKS_PER_SECOND * 30.0f;
// maximum velocity we will use prediction for... any velocities above this value will be capped...
const float		MAX_PREDICTION_VELOCITY = 50.0f;

// Difference required in predicted position/orientation and actual position or orientation to generate a new VerbMovement()
const float		EPSILON_POSITION = 1.0f;
const float		EPSILON_ORIENTATION = PI / 128.0f;

//---------------------------------------------------------------------------------------------------

IMPLEMENT_FACTORY( TraitMovement, Trait );

BEGIN_PROPERTY_LIST( TraitMovement, Trait )
END_PROPERTY_LIST();

//---------------------------------------------------------------------------------------------------

TraitMovement::TraitMovement( bool a_bServerLocal /*= false*/ ) : 
	m_bServerLocal( a_bServerLocal ),
	m_bInitialized( false ),
	m_bDisabled( false ),
	m_nLastSentMovement( 0 ), 
	m_nLastGlobalMovement( 0 ),
	m_vPosition( true ),
	m_vLastPosition( true ),
	m_vOrientation(),
	m_vLastOrientation(),
	m_vVelocity( true ), 
	m_vRotVelocity(), 
	m_nLastTick( 0 ), 
	m_vSmoothPosition( Vector3( 0, 0, 0 ), Vector3( 0, 0, 0 ) ),
	m_vSmoothOrien( Vector3( 0, 0, 0 ), Vector3( 0, 0, 0 ) )
{}

//---------------------------------------------------------------------------------------------------

void TraitMovement::initialize()
{
	Trait::initialize();

	// don't initialize if noun is getting transfered to a new zone..
	if (! parent()->transfer() || !m_bInitialized )
	{
		// initialize with the parent's position and orientation...
		m_vPosition = m_vLastPosition = parent()->position();
		m_vSmoothPosition.SetValue( m_vPosition );
		m_vOrientation = m_vLastOrientation = parent()->orientation();
		m_vSmoothOrien.SetValue( m_vOrientation );
		m_nLastTick = 0;

		// initialize the last sent movement to a random value... otherwise all AI objects
		// will send their movement on the same frame...
		m_nLastSentMovement = parent()->tick() + (rand() % MIN_SEND_MOVEMENT_TICKS);
		m_bInitialized = true;
	}
}

void TraitMovement::simulate( dword nTick )
{
	if (! m_bDisabled )
	{
		int nElapsed = nTick - parent()->tick();
		if ( nElapsed > 0 )
		{
			// update the position & frame of our parent noun...
			float fDelta = TICK_DURATION_S * nElapsed;
			
			if ( (!m_bServerLocal && parent()->isLocal()) || (m_bServerLocal && parent()->isServer()) )
			{
				if ( nTick >= (m_nLastSentMovement + MIN_SEND_MOVEMENT_TICKS) )
				{
					// we only send global movement packets for nouns that are objects or landmarks, all other types don't send global movement.
					bool bGlobal = (parent()->nounType() == TYPE_OBJECT || parent()->nounType() == TYPE_LANDMARK) ? 
						nTick >= (m_nLastGlobalMovement + MIN_GLOBAL_MOVEMENT_TICKS) : false;

					bool bMaxTicks = nTick >= (m_nLastSentMovement + MAX_SEND_MOVEMENT_TICKS);
					bool bPositionChanged = ( (parent()->position() - m_vPosition ).magnitude2() > EPSILON_POSITION);
					bool bOrienChanged = ( Vector3( parent()->orientation() - m_vOrientation ).magnitude2() > EPSILON_ORIENTATION);
					bool bSend = bGlobal || bMaxTicks || bPositionChanged || bOrienChanged;

					if ( bSend )
					{
		#if ENABLE_MOVEMENT_TRACE
						TRACE( CharString().format("Sending movement, Noun = %p", parent() ) );
		#endif
						sendMovement( bGlobal );
					}
				}
			}
			else
			{
				// TODO: Implement smoothing when applying the calculated position / orientation to the actual object.
				// ... Apply our changes to the parent noun
				parent()->setPosition( m_vSmoothPosition.Update( m_vPosition, SMOOTH_POSITION_TIME, fDelta) );
				parent()->setOrientation( m_vSmoothOrien.Update( m_vOrientation, SMOOTH_ORIENATION_TIME, fDelta) );
			}

			// project position and orientation into the future...
			m_vPosition += m_vVelocity * fDelta;
			m_vOrientation += m_vRotVelocity * fDelta;
		}
	}

	Trait::simulate( nTick );
}

void TraitMovement::onTransferZones( NodeZone * pNewZone, NodeZone * pOldZone )
{
	// adjust our smoothed position value... 
	Vector3 vOffset( pOldZone->position() - pNewZone->position()  );
#if ENABLE_MOVEMENT_TRACE
	TRACE( CharString().format( "TraitMovement::onTransferZones() - %s, %s, %s",
		ConvertType<CharString>( pNewZone->position() ), 
		ConvertType<CharString>( pOldZone->position() ),
		ConvertType<CharString>( vOffset ) ) );
#endif

	m_vSmoothPosition.SetValue( m_vSmoothPosition.GetValue() + vOffset );
	m_vPosition		+= vOffset;
}

//---------------------------------------------------------------------------------------------------

void TraitMovement::setServerLocal( bool a_bServerLocal )
{
	m_bServerLocal = a_bServerLocal;
}

void TraitMovement::setDisabled( bool a_bDisabled )
{
	if ( m_bDisabled != a_bDisabled )
	{
		m_bDisabled = a_bDisabled;
		if (! m_bDisabled )
			initialize();		// re-initialize when enabled..
	}
}

void TraitMovement::setPosition( const Vector3 & a_vPosition )
{
	m_vPosition = a_vPosition;
}

void TraitMovement::setOrientation( const Euler & a_Orientation )
{
	m_vOrientation = a_Orientation;
}

void TraitMovement::receiveMovement( dword nTick, NodeZone * pZone, const Vector3 & vPosition, const Euler & vOrientation, bool bGlobal )
{
	if (! pZone )
		return;		// invalid zone pointer...
	if ( nTick < m_nLastTick )
		return;		// movement is too old, we have already received newer movement information...

	NodeZone * pOldZone = parent()->zone();
	if ( !pOldZone )
		return;		// this can happen if we send out a movement packet, then detach ourselves, then process our own movement verb...
	if ( pZone != pOldZone )
	{
		// if the old zone is locked, then go ahead and just translate the movement into coordinates for the old zone...
		if ( pOldZone->locked() )
		{
			Vector3 vOldPosition( vPosition - (pOldZone->position() - pZone->position()) );
#if ENABLE_MOVEMENT_TRACE
			TRACE( CharString().format( "TraitMovement::receiveMovement() - %s, %s, %s, %s",
				ConvertType<CharString>( pZone->position() ), 
				ConvertType<CharString>( pOldZone->position() ),
				ConvertType<CharString>( vPosition ),
				ConvertType<CharString>( vOldPosition ) ) );
#endif
			receiveMovement( nTick, pOldZone, vOldPosition, vOrientation, bGlobal );
			return;
		}

		// move our parent into their new zone...
		pOldZone->transferNoun( parent(), pZone, true );
	}

	processMovement( nTick, pZone, vPosition, vOrientation, bGlobal );
}

void TraitMovement::processMovement( dword nTick, NodeZone * pZone, const Vector3 & vPosition, const Euler & vOrientation, bool bGlobal )
{
	dword nElapsed = nTick - m_nLastTick;
	if ( nElapsed < USE_PREDICTION_TICKS && parent()->inLockedZone() )		// we need two movement packets before we start predicting velocity/rotation
	{
		// calculate the base velocity and rotational velocity based on our last position/orientation and our new position and orientation..
		float fElapsed = TICK_DURATION_S * nElapsed;
		if ( fElapsed > 0.0f )
		{
			m_vVelocity = (vPosition - m_vLastPosition) / fElapsed;

			// cap our velocity within limits...
			float fVelocity = m_vVelocity.magnitude();
			if ( fVelocity > MAX_PREDICTION_VELOCITY )
				m_vVelocity *= MAX_PREDICTION_VELOCITY / fVelocity;

			m_vRotVelocity = (vOrientation - m_vLastOrientation) / fElapsed;
			float fLag = TICK_DURATION_S * (parent()->tick() - nTick);
			if ( fLag > 0.0f )
			{
				m_vPosition = vPosition + (m_vVelocity * fLag);
				m_vOrientation = vOrientation + (m_vRotVelocity * fLag);
			}
			else
			{
				m_vPosition = vPosition;
				m_vOrientation = vOrientation;
			}
#if ENABLE_MOVEMENT_TRACE
			TRACE( CharString().format("Received movement, Noun = %p, Pos = %s, Orien = %s, Vel = %s, RotVel = %s",
				parent(), CharString( m_vPosition ), CharString( m_vOrientation ), 
				CharString( m_vVelocity ), CharString( m_vRotVelocity ) ) );
#endif
		}
	}
	else
	{
		m_vPosition = vPosition;
		m_vOrientation = vOrientation;
		m_vVelocity.zero();
		m_vRotVelocity.zero();

		// Update the position of the object if the zone is not currently locked and updating...
		if (! parent()->inLockedZone() )
		{
			parent()->setPosition( m_vPosition );
			m_vSmoothPosition.SetValue( m_vPosition );
			m_vSmoothPosition.SetVelocity( Vector3( 0.0f, 0.0f, 0.0f ) );

			parent()->setOrientation( m_vOrientation );
			m_vSmoothOrien.SetValue( m_vOrientation );
			m_vSmoothOrien.SetVelocity( Euler( 0.0f, 0.0f, 0.0f ) );
		}
	}

	m_nLastTick = nTick;
	m_vLastPosition = vPosition;
	m_vLastOrientation = vOrientation;

	// Update our base velocity, so intercept calculations will work correctly..
	parent()->setVelocity( m_vVelocity );
}

void TraitMovement::sendMovement( bool bGlobal )
{
	parent()->attachVerb( new VerbMovement( parent(), bGlobal ) );
	m_nLastSentMovement = parent()->tick();
	if ( bGlobal )
		m_nLastGlobalMovement = m_nLastSentMovement;
}

//---------------------------------------------------------------------------------------------------
//EOF

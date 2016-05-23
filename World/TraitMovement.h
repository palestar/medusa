/**
	@file TraitMovement.h
	@brief This trait replicates or receives the movement of a noun in the world. Attach this trait
	to any noun for which you want to replicate that movement over the network.

	If m_bSendMovement is true, then this trait will replicate the position / orientation
	from the noun parent using VerbMovement packets. The position / orientation in the noun 
	should be getting updated by another source (i.e. ship dynamics).

	If m_bSendMovment is false, then any received movement packets will be applied to the 
	parent noun.

	(c)2006 Palestar Inc
	@author Richard Lyle @date 5/4/2007 7:57:55 PM
*/

#ifndef TRAITMOVEMENT_H
#define TRAITMOVEMENT_H

#include "Standard/Queue.h"
#include "Standard/WeakReference.h"
#include "Math/Euler.h"
#include "Math/Smoother.h"

#include "Trait.h"
#include "WorldDll.h"

//---------------------------------------------------------------------------------------------------

class DLL TraitMovement : public Trait
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< TraitMovement >		Ref;
	typedef WeakReference< TraitMovement >	WeakRef;

	// Construction
	TraitMovement( bool a_bServerLocal = false );

	// Trait interface
	virtual void		initialize();
	virtual void		simulate( dword nTick );					
	virtual void		onTransferZones( NodeZone * pNewZone, NodeZone * pOldZone );

	// Accessors
	bool				serverLocal() const;		// if true, trait is local on the server
	bool				disabled() const;			// if true, then this trait is disabled
	const Vector3 &		position() const;			// get the current position inside the zone
	const Euler &		orientation() const;		// get the current orientation of the noun
	const Vector3 &		velocity() const;			// get the velocity vector
	const Euler &		rotVelocity() const;		// get the rotational velocity vector

	// Mutators
	void				setServerLocal( bool a_bServerLocal );
	void				setDisabled( bool a_bDisabled );
	void				setPosition( const Vector3 & a_vPosition );
	void				setOrientation( const Euler & a_Orientation );

	// Called by VerbMovement()
	void				receiveMovement( dword nTick, NodeZone * pZone, const Vector3 & vPosition, 
							const Euler & vOrientation, bool bGlobal );
	//! push a VerbMovement()
	void				sendMovement( bool bGlobal );

protected:
	// Data
	bool				m_bServerLocal;				// trait is local on the server only..
	bool				m_bInitialized;				// true if this has been initialized..
	bool				m_bDisabled;				// true if disabled
	dword				m_nLastSentMovement;		// tick of last time we send movement information...
	dword				m_nLastGlobalMovement;		// tick of the last time we sent global movement
	Vector3				m_vPosition;				// current position in the zone
	Vector3				m_vLastPosition;
	Euler				m_vOrientation;				// current orientation in the zone
	Euler				m_vLastOrientation;
	Vector3				m_vVelocity;				// current estimated velocity
	Euler				m_vRotVelocity;				// current rotational velocity
	dword				m_nLastTick;				// last received movement...

	Smoother< Vector3 >	m_vSmoothPosition;
	Smoother< Euler >	m_vSmoothOrien;

	// Mutators
	void				processMovement( dword nTick, NodeZone * pZone, const Vector3 & vPosition, const Euler & vOrientation, bool bGlobal );
};

//---------------------------------------------------------------------------------------------------

inline bool TraitMovement::serverLocal() const
{
	return m_bServerLocal;
}

inline bool TraitMovement::disabled() const
{
	return m_bDisabled;
}

inline const Vector3 & TraitMovement::position() const
{
	return m_vPosition;
}

inline const Euler & TraitMovement::orientation() const
{
	return m_vOrientation;
}

inline const Vector3 & TraitMovement::velocity() const
{
	return m_vVelocity;
}

inline const Euler & TraitMovement::rotVelocity() const
{
	return m_vRotVelocity;
}

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF

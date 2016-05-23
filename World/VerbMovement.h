/**
	@file VerbMovement.h
	@brief TODO

	(c)2006 Palestar Inc
	@author Richard Lyle @date 5/15/2007 8:59:12 PM
*/

#ifndef VERBMOVEMENT_H
#define VERBMOVEMENT_H

#include "TraitMovement.h"
#include "NodeZone.h"
#include "Verb.h"

//---------------------------------------------------------------------------------------------------

class VerbMovement : public Verb
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Construction
	VerbMovement();
	VerbMovement( Noun * pTarget, bool bGlobal );

	// Verb Interface
	virtual bool			useUDP() const;						// if true, then verb is sent using UDP
	virtual Priority		priority() const;					// priority of this verb
	virtual Scope			scope() const;
	virtual bool			client() const;						// is this a client verb, meaning okay to receive from a client
	virtual bool			queue() const;						// if true verb is queued to be executed inside simulate() 
	virtual bool			checkNoun() const;

	virtual void			onExecute();

private:
	// Data
	NodeZone::wRef			m_pZone;
	Vector3					m_vPosition;
	Euler					m_vOrientation;
	bool					m_bGlobal;
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF

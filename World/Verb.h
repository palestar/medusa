/*
	Verb.h

	This base class is used to represent a action towards a noun
	(c)1999 Palestar Development, Richard Lyle
*/

#ifndef VERB_H
#define VERB_H

#include "Standard/Queue.h"
#include "Factory/FactoryTypes.h"
#include "WorldDll.h"

//----------------------------------------------------------------------------

class WorldContext;		// forward declare
class Noun;

//-------------------------------------------------------------------------------

class DLL Verb : public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef WorldContext				Context;
	typedef Reference<Verb>				Ref;

	enum Priority
	{
		CRITICAL,
		HIGH,
		MEDIUM,
		LOW,
	};
	enum Scope
	{
		SERVER,				// send to server
		LOCAL,				// send verb to clients local to target
		NOT_LOCAL,			// send verb to clients not local to target
		GLOBAL,				// send verb to all clients
		TEAM,				// send verb to all players of the same team
		FACTION,			// send verb to all players of the same faction
		PRIVATE,			// send verb to server and player for target

		// depreciated...
		FLEET = TEAM,		
	};

	// Construction
	Verb( dword nTick = 0 );

	// Widget interface
	bool					read( const InStream & );
	bool					write( const OutStream & );

	// Accessors
	dword					tick() const;						// time of verb
	Noun *					target() const;						// target of this verb

	dword					clientId() const;					// who sent the verb
	dword					userId() const;	

	virtual bool			useUDP() const;						// if true, then verb is sent using UDP
	virtual bool			checkNoun() const;					// if true, code checks if noun was sent to client.
	virtual Priority		priority() const;					// priority of this verb
	virtual Scope			scope() const;						// what is the scope of this verb
	virtual bool			client() const;						// is this a client verb, meaning okay to receive from a client
	virtual bool			queue() const;						// if true verb is queued to be executed inside simulate() 

	// Mutators
	void					setTick( dword nTick );				// set the time for this verb
	void					setTarget( Noun * pTarget );		// set the verb target, if bSetContext true context is set from the noun
	void					setClientId( dword nClientId );
	void					setUserId( dword nSenderId );	

	virtual bool			canAttach( Noun * pTarget );		// valid to attach this verb to the context
	virtual void			onExecute() = 0;					// execute the verb

	// Helpers
	Context *				context() const;					// get the WorldContext from our target
	bool					validate( Noun * pNoun ) const;		// validate a noun before attempting to use it, returns true if noun is valid
	bool					attachVerb( Noun * pNoun );			// attach this verb to a noun

	int						lag() const;				

protected:
	// Data
	dword					m_nTick;							// time of this verb

	// non-serialized
	Noun *					m_pTarget;							// target of this verb
	dword					m_nClientId;						// routing data, used server-side to route the verb
	dword					m_nUserId;				
};

IMPLEMENT_RAW_STREAMING( Verb::Scope );
IMPLEMENT_RAW_STREAMING( Verb::Priority );

//----------------------------------------------------------------------------

inline dword Verb::tick() const
{
	return m_nTick;
}

inline Noun * Verb::target() const
{
	return m_pTarget;
}

inline dword Verb::clientId() const
{
	return m_nClientId;
}

inline dword Verb::userId() const
{
	return m_nUserId;
}

//----------------------------------------------------------------------------

inline void Verb::setTick( dword nTick )
{
	m_nTick = nTick;
}

inline void Verb::setTarget( Noun * pTarget )
{
	m_pTarget = pTarget;
}

inline void Verb::setClientId( dword nClientId )
{
	m_nClientId = nClientId;
}

inline void Verb::setUserId( dword nUserId )
{
	m_nUserId = nUserId;
}

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

/*
	VerbChat.h

	This verb is sent to display a chat message on the clients.
	(c)1999 Palestar, Richard Lyle
*/

#ifndef VERB_CHAT_H
#define VERB_CHAT_H

#include "Verb.h"
#include "WorldDll.h"

//-------------------------------------------------------------------------------

class DLL VerbChat : public Verb
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Construction
	VerbChat();
	VerbChat( Noun * pTarget, Scope scope, const char * chat );

	// Verb Interface
	Priority				priority() const;	// priority of this verb
	Scope					scope() const;
	bool					client() const;			// is this a client verb, meaning okay to receive from a client

	void					onExecute();

	// Data
	Scope					m_Scope;
	CharString				m_sChat;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

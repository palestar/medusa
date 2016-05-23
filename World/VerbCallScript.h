/*
	VerbCallScript.h
	(c)2000 Palestar Inc, Richard Lyle
*/

#ifndef VERBCALLSCRIPT_H
#define VERBCALLSCRIPT_H

#include "Verb.h"
#include "WorldDll.h"

//-------------------------------------------------------------------------------

class DLL VerbCallScript : public Verb
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Construction
	VerbCallScript();
	VerbCallScript( Noun * pTarget, Scope scope, UniqueNumber nScriptId, const char * pCall, bool bNewContext );

	// Verb Interface
	Priority				priority() const;	// priority of this verb
	Scope					scope() const;
	bool					client() const;	

	void					onExecute();

	// Data
	Scope					m_Scope;
	UniqueNumber			m_nScriptId;
	CharString				m_Call;
	bool					m_CreateNewContext;
};

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF


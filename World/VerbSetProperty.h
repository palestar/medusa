/**
	@file VerbSetProperty.h
	@brief This verb can set a property (aka variable) of the given object.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 11/25/2009 11:46:19 PM
*/

#ifndef VERBSETPROPERTY_H
#define VERBSETPROPERTY_H

#include "Reflection/Value.h"
#include "World/Verb.h"
#include "WorldDll.h"

//-------------------------------------------------------------------------------

class DLL VerbSetProperty : public Verb
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	//! Types
	struct Record {
		u8				m_nProperty;
		Value			m_Value;
	};
	typedef Array< Record >			RecordArray;

	// Construction
	VerbSetProperty();
	//! If a_bGlobal is false, then this verb is sent only to clients local to the target object.
	VerbSetProperty( Noun * pTarget, const char * a_pProperty, const Value & a_Value, bool a_bGlobal = true, bool a_bUDP = false );
	VerbSetProperty( Noun * pTarget, int a_nProperty, const Value & a_Value, bool a_bGlobal = true, bool a_bUDP = false );

	// Verb Interface
	virtual bool			useUDP() const;		// if true, then verb is sent using UDP
	Priority				priority() const;	// priority of this verb
	Scope					scope() const;
	bool					client() const;

	bool					canAttach( Noun * pNoun );
	void					onExecute();

	// Data
	u8						m_nProperty;
	Value					m_Value;
	bool					m_bGlobal;
	bool					m_bUDP;
};


#endif

//---------------------------------------------------------------------------------------------------
//EOF

/*
	Trait.h

	This is the base class for a Noun trait; A noun can have any number of traits 
	attached. A trait adds functionality and/or properties to a noun such as Physics,
	AI, interactions, etc...

	The purpose of this class is so additional properties can be added to nouns 
	without having to inherit from multiple classes and bloat the size of the Noun class.

	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef TRAIT_H
#define TRAIT_H

#include "Factory/FactoryTypes.h"
#include "Math/Vector3.h"
#include "WorldDll.h"

//---------------------------------------------------------------------------------------------------

// forward declare noun
class Noun;
class NodeZone;

class DLL Trait : public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< Trait >		Ref;

	// Construction
	Trait();
	virtual ~Trait();

	// Accessors
	Noun *				parent() const;			// the noun instance to which we are attached
	bool				detach() const;
	// Mutators
	void				setParent( Noun * pParent );
	void				setDetach( bool a_bDetach = true );

	// called when the noun is initialized
	virtual void		initialize();
	// called when the noun is released
	virtual void		release();
	// simulate the trait, called for each game tick
	virtual void		simulate( dword nTick );					
	// called when our parent noun changes zones, update any internal positions if needed..
	virtual void		onTransferZones( NodeZone * pNewzone, NodeZone * pOldZone );

protected:
	// Data
	Noun *				m_pParent;				// noun to which we are attached
	bool				m_bDetach;				// set to true if trait wants to be detached..
};

//----------------------------------------------------------------------------

inline Noun * Trait::parent() const
{
	return m_pParent;
}

inline bool Trait::detach() const
{
	return m_bDetach;
}

//----------------------------------------------------------------------------


#endif

//----------------------------------------------------------------------------
//EOF

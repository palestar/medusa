/**
	@file TraitReplication.h
	@brief This trait replicates data members changes of it's parent object down to clients or up to the server
	using VerbSetProperty.

	(c)2009 Palestar Inc
	@author Richard Lyle @date 8/26/2010 7:33:46 PM
*/

#ifndef TRAITREPLICATION_H
#define TRAITREPLICATION_H

#include "Trait.h"
#include "WorldDll.h"

//---------------------------------------------------------------------------------------------------

class DLL TraitReplication : public Trait
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	//! Construction
	TraitReplication();

	//! Trait interface
	virtual void		initialize();
	virtual void		simulate( dword nTick );					

	//! Accessors
	bool				disabled() const;
	//! Mutators
	void				setDisabled( bool a_bDisabled );

protected:
	//! Types
	struct Replicate : public Referenced
	{
		typedef Reference< Replicate >		Ref;

		int				m_nProperty;
		Property *		m_pProperty;
		Value			m_LastValue;
		dword			m_nInterval;
		dword			m_nNextTick;

		Replicate() : 
			m_nProperty( -1 ), 
			m_pProperty( NULL ), 
			m_nInterval( 20 ), 
			m_nNextTick( 0 )
		{}
		Replicate( int a_nProperty, Property * a_pProperty, const Value & a_Value, dword a_nInterval, dword a_nNextTick ) :
			m_nProperty( a_nProperty ), 
			m_pProperty( a_pProperty ), 
			m_LastValue( a_Value ), 
			m_nInterval( a_nInterval ), 
			m_nNextTick( a_nNextTick )
		{}
	};
	typedef std::list< Replicate::Ref >		ReplicateList;

	//! Data
	bool				m_bDisabled;				// set to true if no replication should occur
	ReplicateList		m_ReplicateList;			// this is a list of the properties getting replicated

	void				insertIntoReplicateList( Replicate * a_pReplicate );
};

//---------------------------------------------------------------------------------------------------

inline bool TraitReplication::disabled() const
{
	return m_bDisabled;
}

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF

/**
	@file TraitReplication.cpp

	(c)2009 Palestar Inc
	@author Richard Lyle @date 8/26/2010 7:48:58 PM
*/

#include "TraitReplication.h"
#include "VerbSetProperty.h"
#include "Noun.h"

#include "Debug/Assert.h"

//---------------------------------------------------------------------------------------------------

BEGIN_PROPERTY_LIST( TraitReplication, Trait );
END_PROPERTY_LIST();

IMPLEMENT_FACTORY( TraitReplication, Trait );

TraitReplication::TraitReplication() : 
	m_bDisabled( false )
{}

void TraitReplication::initialize()
{
	Noun * pParent = parent();
	ASSERT( pParent );

	bool bServer = pParent->isServer();

	dword nTick = pParent->tick();

	// initialize from our parent object..
	m_ReplicateList.clear();

	PropertyList * pPropertyList = pParent->propertyList();
	for(int i=0;i<pPropertyList->propertyCount();++i)
	{
		Property * pProperty = pPropertyList->property( i );
		if ( bServer && (pProperty->flags() & PF_REPLICATE) == 0 )
			continue;		// server needs PF_REPLICATE flag
		else if ( !bServer && (pProperty->flags() & PF_REPLICATE_UP) == 0 )
			continue;		// client needs PF_REPLICATE_UP flag
		if ( !bServer && !pParent->isLocal() )
			continue;		// clients can't only replicate up from local objects
		if ( pProperty->isDynamic() || pParent->propertyElements( i ) > 1 )
			continue;		// currently, no support for replicating arrays of properties
		if ( i > 255 )
			continue;		// replication supports a maximum of 255 properties currently

		dword nInterval = 20;
		Property::Option * pInterval = pProperty->findOption( "_INTERVAL" );
		if ( pInterval != NULL )
			nInterval = strtoul( pInterval->sValue, NULL, 10 );
		if ( nInterval == 0 )
			nInterval = 20;

		dword nNextTick = nTick + (rand() % nInterval);

		Value v;
		if (! pParent->get( pProperty, 0, v ) )
		{
			TRACE( "Failed to get property for replication" );
			continue;
		}

		insertIntoReplicateList( new Replicate( i, pProperty, v, nInterval, nNextTick ) );
	}

	// if we have no data to replicate, then just disable this trait..
	if ( m_ReplicateList.size() == 0 )
		m_bDisabled = true;
}

void TraitReplication::simulate( dword nTick )
{
	if( !m_bDisabled )
	{
		Noun * pParent = parent();
		ASSERT( pParent != NULL );

		dword nTick = pParent->tick();

		// check for changes in replicated propertities, send a VerbSetProperty() if they have changed..
		for( ReplicateList::iterator iReplicate = m_ReplicateList.begin(); 
			iReplicate != m_ReplicateList.end(); )
		{
			Replicate::Ref pReplicate = *iReplicate;
			if ( pReplicate->m_nNextTick > nTick )
				break;		// not time yet, stop enumerating... 

			m_ReplicateList.erase( iReplicate++ );

			Property * pProperty = pReplicate->m_pProperty;
			void * pParentData = pProperty->pointer( pParent, 0 );
			void * pLastData = pReplicate->m_LastValue.data();
			if ( memcmp( pParentData, pLastData, pProperty->size() ) != 0 )
			{
				if ( pParent->get( pProperty, 0, pReplicate->m_LastValue ) )
				{
					// property is different, send a verb out..
					Verb::Ref( new VerbSetProperty( pParent, 
						pReplicate->m_nProperty, 
						pReplicate->m_LastValue, 
						(pProperty->flags() & PF_GLOBAL) != 0,		// send to all clients if PF_GLOBAL flag is on
						(pProperty->flags() & PF_UDP) != 0 ) );		// send UDP if PF_UDP flag is on
				}
			}

			pReplicate->m_nNextTick = nTick + pReplicate->m_nInterval;
			insertIntoReplicateList( pReplicate );
		}
	}
}

void TraitReplication::setDisabled( bool a_bDisabled )
{
	m_bDisabled = a_bDisabled;
}

void TraitReplication::insertIntoReplicateList( Replicate * a_pReplicate )
{
	// insert into the list sorted by the next tick..
	ReplicateList::iterator iInsert = m_ReplicateList.begin();
	while( iInsert != m_ReplicateList.end() )
	{
		if ( (*iInsert)->m_nNextTick > a_pReplicate->m_nNextTick )
		{
			// sooner than iInsert, so insert before..
			m_ReplicateList.insert( iInsert, a_pReplicate );
			break;
		}
		++iInsert;
	}

	// if we reached the end of the list, then push onto the end..
	if ( iInsert == m_ReplicateList.end() )
		m_ReplicateList.push_back( a_pReplicate );
}

//---------------------------------------------------------------------------------------------------
//EOF

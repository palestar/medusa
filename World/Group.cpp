/**
	@file Group.cpp

	(c)2006 Palestar Inc
	@author Richard Lyle @date 1/24/2009 6:02:45 PM
*/

#include "Group.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_FACTORY( GroupMember, Widget );
BEGIN_PROPERTY_LIST( GroupMember, Widget );
	ADD_TRANSMIT_PROPERTY( m_bPending );
	ADD_TRANSMIT_PROPERTY( m_bLeader );
	ADD_TRANSMIT_PROPERTY( m_nUserId );
	ADD_TRANSMIT_PROPERTY( m_sName );
	ADD_TRANSMIT_PROPERTY( m_nObjectId );
	ADD_TRANSMIT_PROPERTY( m_nType );
END_PROPERTY_LIST();

IMPLEMENT_FACTORY( Group, Widget );
BEGIN_PROPERTY_LIST( Group, Widget );
	ADD_TRANSMIT_PROPERTY( m_Members );
	ADD_TRANSMIT_PROPERTY( m_nFactionId );
END_PROPERTY_LIST();

int Group::sm_nMaxGroupMembers = 8;

//---------------------------------------------------------------------------------------------------

GroupMember::GroupMember() :
	m_bPending( false ),
	m_bLeader( false ),
	m_nUserId( 0 ),
	m_nObjectId( NULL_WIDGET ),
	m_nType( 0 )
{}

GroupMember::GroupMember( bool bPending, bool bLeader, dword userId, const char * pName, const WidgetKey & objectId, int nType ) :
	m_bPending( bPending ),
	m_bLeader( bLeader ),
	m_nUserId( userId ),
	m_sName( pName ),
	m_nObjectId( objectId ),
	m_nType( nType )
{}

void GroupMember::setIsPending( bool a_bPending )
{
	m_bPending = a_bPending;
}

void GroupMember::setIsLeader( bool a_bLeader )
{
	m_bLeader = a_bLeader;
}

void GroupMember::setUserId( dword a_nUserId )
{
	m_nUserId = a_nUserId;
}

void GroupMember::setName( const char * a_pName )
{
	m_sName = a_pName;
}

void GroupMember::setObjectId( const WidgetKey & a_nObjectId )
{
	m_nObjectId = a_nObjectId;
}

void GroupMember::setType( int a_nType )
{
	m_nType = a_nType;
}

//---------------------------------------------------------------------------------------------------

Group::Group() : m_nFactionId( 0 )
{}

void Group::add( GroupMember * a_pMember )
{
	m_Members.push( a_pMember );
}

void Group::remove( GroupMember * a_pMember )
{
	m_Members.removeSearchSwap( a_pMember );
}

void Group::remove( int a_n )
{
	m_Members.removeSwap( a_n );
}

void Group::setFactionId( int nFactionId )
{
	m_nFactionId = nFactionId;
}

bool Group::clearPending( dword nUserId )
{
	GroupMember * pMember = findMember( nUserId );
	if (! pMember || !pMember->isPending() )
		return false;

	pMember->setIsPending( false );
	return true;
}

//---------------------------------------------------------------------------------------------------
//EOF

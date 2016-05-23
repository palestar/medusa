/**
	@file Group.h
	@brief This Widget class implements a player group in game, it's stored in each memebers UseStorage object..

	(c)2006 Palestar Inc
	@author Richard Lyle @date 1/24/2009 5:52:53 PM
*/

#ifndef GROUP_H
#define GROUP_H

#include "Factory/FactoryTypes.h"
#include "WorldDll.h"

//---------------------------------------------------------------------------------------------------

class DLL GroupMember : public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< GroupMember >		Ref;

	// Construction
	GroupMember();
	GroupMember( bool bPending, bool bLeader, dword userId, const char * pName, const WidgetKey & objectId, int nType );

	// Accessors
	bool				isPending() const;			// if true, then invite has been sent, waiting on reply..
	bool				isLeader() const;			// true if this member is the leader of the group
	dword				userId() const;				// user ID of the group member, if 0 then member is an AI
	const char *		name() const;				// name of the group member
	const WidgetKey &	objectId() const;			// ID of the object, this is updated if the user changes self
	int					type() const;				// type of object, use to display the correct icon on the UI since we may not have the object

	// Mutators
	void				setIsPending( bool a_bPending );
	void				setIsLeader( bool a_bLeader );
	void				setUserId( dword a_nUserId );
	void				setName( const char * a_pName );
	void				setObjectId( const WidgetKey & a_nObjectId );
	void				setType( int a_nType );

protected:
	// Data
	bool				m_bPending;
	bool				m_bLeader;
	dword				m_nUserId;
	CharString			m_sName;
	WidgetKey			m_nObjectId;
	int					m_nType;
};

//---------------------------------------------------------------------------------------------------

class DLL Group : public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Configurations
	static int			sm_nMaxGroupMembers;

	// Types
	typedef Reference< Group >			Ref;
	typedef WidgetReference< Group >	wRef;

	// Construction
	Group();

	// Accessors
	int					memberCount() const;
	int					activeCount() const;				// returns the number of non-pending members
	GroupMember *		member( int a_n ) const;
	GroupMember *		findMember( dword nUserId ) const;
	bool				isLeader( dword nUserId ) const;
	bool				isPending( dword nUserId ) const;
	int					factionId() const;			// faction of this group

	// Mutators
	void				add( GroupMember * a_pMember );
	void				remove( GroupMember * a_pMember );
	void				remove( int a_n );
	void				setFactionId( int nFactionId );

	bool				clearPending( dword nUserId );

protected:
	// Data
	Array< GroupMember::Ref >
						m_Members;
	int					m_nFactionId;
};

//---------------------------------------------------------------------------------------------------

inline bool GroupMember::isPending() const
{
	return m_bPending;
}

inline bool GroupMember::isLeader() const
{
	return m_bLeader;
}

inline dword GroupMember::userId() const
{
	return m_nUserId;
}

inline const char * GroupMember::name() const
{
	return m_sName;
}

inline const WidgetKey & GroupMember::objectId() const
{
	return m_nObjectId;
}

inline int GroupMember::type() const
{
	return m_nType;
}

//---------------------------------------------------------------------------------------------------

inline int Group::memberCount() const
{
	return m_Members.size();
}

inline int Group::activeCount() const
{
	int nActive = 0;
	for(int i=0;i<memberCount();++i)
		if (! member(i)->isPending() )
			++nActive;
	return nActive;
}

inline GroupMember * Group::member( int a_n ) const
{
	return m_Members[ a_n ];
}

inline GroupMember * Group::findMember( dword nUserId ) const
{
	for(int i=0;i<m_Members.size();++i)
	{
		if ( m_Members[i]->userId() == nUserId )
			return m_Members[i];
	}
	return NULL;
}

inline bool Group::isLeader( dword nUserId ) const
{
	GroupMember * pMember = findMember( nUserId );
	if (! pMember )
		return false;
	return pMember->isLeader();
}

inline bool Group::isPending( dword nUserId ) const
{
	GroupMember * pMember = findMember( nUserId );
	if (! pMember )
		return false;
	return pMember->isPending();
}

inline int Group::factionId() const
{
	return m_nFactionId;
}

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF

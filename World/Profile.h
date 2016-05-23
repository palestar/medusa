/*
	Profile.h
	(c)2000 Palestar Inc, Richard Lyle
*/

#ifndef GAME_PROFILE_H
#define GAME_PROFILE_H

#include "GCQ/MetaClient.h"
#include "File/Stream.h"
#include "Factory/FactoryTypes.h"
#include "Standard/CharString.h"
#include "Standard/Tree.h"
#include "Constants.h"
#include "WorldDll.h"

//----------------------------------------------------------------------------

class DLL Profile : public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< Profile >			Ref;

	// Construction
	Profile();		
	Profile( const Profile & copy );
	virtual ~Profile();

	// Accessors
	bool				isModified() const;
	dword				timeModified() const;								// how many seconds since this profile was modified

	bool				isSubscriber() const;								// returns true if player is subscribed
	bool				isModerator() const;								// is this player a moderator
	bool				isAdmin() const;
	bool				isDeveloper() const;

	dword				flags() const;
	dword				userId() const;
	dword				clanId() const;
	dword				sessionId() const;

	const char *		email() const;
	const char *		name() const;

	float				operator[]( const char * pFieldName ) const;

	// Mutators
	void				setName( const CharString & a_sName );
	float &				operator[]( const char * pFieldName );

	void				addStatistic( const char * pFieldName, float value );		// sets the time modified
	void				setModified();		
	void				clearModified();

	Profile &			operator=( const Profile & copy );

	virtual void		load( const MetaClient::Profile & profile );		// clears the time modified
	virtual void		save( MetaClient::Profile & profile);				// clears the time modified
	virtual int			spawnScore();										// value of this player in regards to spawner/balance calculations

protected:
	// Types
	typedef Tree< CharString, float >			StatTree;

	// Data
	dword				m_nFlags;				// user flags
	dword				m_nUserId;				// Users Id / Alias
	dword				m_nClanId;				// users clan, 0 if none
	dword				m_nSessionId;			// session for this user
	CharString			m_sEmail;				// users email address
	CharString			m_sName;				// user name

	StatTree			m_Stats;
	dword				m_nLastTimeModified;	// Time in seconds since this profile was modified
};

//---------------------------------------------------------------------------------------------------

inline void Profile::setName( const CharString & a_sName )
{
	m_sName = a_sName;
}

//---------------------------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF

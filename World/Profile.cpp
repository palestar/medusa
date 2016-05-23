/*
	Profile.cpp
	(c)2000 Palestar Inc, Richard Lyle
*/


#include "Debug/Assert.h"
#include "Standard/Time.h"
#include "Profile.h"
#include "WorldContext.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( Profile, Widget );

BEGIN_PROPERTY_LIST( Profile, Widget )
	ADD_TRANSMIT_PROPERTY( m_nFlags );
	ADD_TRANSMIT_PROPERTY( m_nUserId );
	ADD_TRANSMIT_PROPERTY( m_nClanId );
	ADD_TRANSMIT_PROPERTY( m_nSessionId );
	ADD_TRANSMIT_PROPERTY( m_sEmail );
	ADD_TRANSMIT_PROPERTY( m_sName );
	ADD_TRANSMIT_PROPERTY( m_Stats );
	ADD_TRANSMIT_PROPERTY( m_nLastTimeModified );
END_PROPERTY_LIST();

Profile::Profile() : m_nFlags( 0 ), m_nUserId( 0 ), m_nClanId( 0 ), m_nSessionId( 0 ), m_nLastTimeModified( 0 )
{}

Profile::Profile( const Profile & copy )
{
	operator=( copy );
}

Profile::~Profile()
{}

//----------------------------------------------------------------------------

bool Profile::isModified() const
{
	return m_nLastTimeModified != 0;
}

dword Profile::timeModified() const
{
	if ( isModified() )
		return Time::seconds() - m_nLastTimeModified;
	return 0;
}

bool Profile::isSubscriber() const
{
	return (m_nFlags & MetaClient::SUBSCRIBED) != 0;
}

bool Profile::isModerator() const
{
	return (m_nFlags & MetaClient::MODERATOR) != 0;
}

bool Profile::isAdmin() const
{
	return (m_nFlags & MetaClient::ADMINISTRATOR) != 0;
}

bool Profile::isDeveloper() const
{
	return (m_nFlags & MetaClient::DEVELOPER) != 0;
}

dword Profile::flags() const
{
	return m_nFlags;
}

dword Profile::userId() const
{
	return m_nUserId;
}

dword Profile::clanId() const
{
	return m_nClanId;
}

dword Profile::sessionId() const
{
	return m_nSessionId;
}

const char * Profile::email() const
{
	return m_sEmail;
}

const char * Profile::name() const
{
	return m_sName;
}

float Profile::operator[]( const char * pFieldName ) const
{
	StatTree::Iterator iStat = m_Stats.find( pFieldName );
	if ( iStat.valid() )
		return *iStat;
	return 0.0f;
}

//----------------------------------------------------------------------------

float & Profile::operator[]( const char * pFieldName )
{
	setModified();
	return m_Stats[ pFieldName ];
}

void Profile::addStatistic( const char * pFieldName, float fAmount )
{
	m_Stats[ pFieldName ] += fAmount;
	setModified();
}

void Profile::setModified()
{
	if ( m_nLastTimeModified == 0 )
		m_nLastTimeModified = Time::seconds();
}

void Profile::clearModified()
{
	m_nLastTimeModified = 0;
}

Profile & Profile::operator=( const Profile & copy )
{
	m_nFlags = copy.m_nFlags;
	m_nUserId = copy.m_nUserId;
	m_nClanId = copy.m_nClanId;
	m_nSessionId = copy.m_nSessionId;
	m_sEmail = copy.m_sEmail;
	m_sName = copy.m_sName;
	m_nLastTimeModified = copy.m_nLastTimeModified;
	m_Stats = copy.m_Stats;
	return *this;
}

//----------------------------------------------------------------------------

void Profile::load( const MetaClient::Profile & profile )
{
	m_nFlags = profile.flags;
	m_nUserId = profile.userId;
	m_nClanId = profile.clanId;
	m_nSessionId = profile.sessionId;
	m_sEmail = profile.email;
	m_sName = profile.name;

	m_Stats.release();
	for(int i=0;i<profile.fields.size();++i)
	{
		const MetaClient::Field & field = profile.fields[ i ];
		m_Stats[ field.name ] = atof( field.value );
	}

	clearModified();
}

void Profile::save( MetaClient::Profile & profile )
{
	StatTree::Iterator iStat = m_Stats.head();
	while( iStat.valid() )
	{
		CharString sKey = iStat.key();
		profile[ sKey ] = CharString().format("%f", (*iStat) );

		iStat++;
	}

	clearModified();
}

int Profile::spawnScore()
{
	return 1;
}

//----------------------------------------------------------------------------
//EOF

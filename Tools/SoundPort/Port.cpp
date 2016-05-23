/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define SOUNDPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"
#include "ChildFrame.h"

#include "Audio/Sound.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( SoundPort, Port );
REGISTER_FACTORY_KEY(  SoundPort, 3922248957742011880 );

BEGIN_PROPERTY_LIST( SoundPort, Port );
	ADD_PROPERTY( m_pSound );
END_PROPERTY_LIST();

SoundPort::SoundPort() : Port()
{}

//-------------------------------------------------------------------------------

const int VERSION_042999 = 1;
const int VERSION_20040701 = 20040701;

bool SoundPort::read( const InStream & input )
{
	if (! Port::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_20040701:
			input >> m_pSound;
			break;
		case VERSION_042999:
			{
				CharString sWaveFile;
				input >> sWaveFile;

				try {
					m_pSound = new Sound( sWaveFile );
				}
				catch(Sound::FailedToLoadWave)
				{}
			}
			break;
		}
	}

	return true;
}

//----------------------------------------------------------------------------

Resource::Ref SoundPort::createResource()
{
	if ( m_pSound.valid() )
	{
		Widget * pUncasted = m_pSound->copy();
		if ( dynamic_cast<Sound *>( pUncasted ) != NULL )
			return (Sound *)pUncasted;

		delete pUncasted;
	}

	return NULL;
}

CFrameWnd * SoundPort::createView()
{
	static CMultiDocTemplate	docTemplate( IDR_PORT,NULL,
							RUNTIME_CLASS(CChildFrame),RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

bool SoundPort::importFile( const char * pFileName )
{
	try {
		m_pSound = new Sound( pFileName );
		UpdateAllViews(NULL);
		return true;
	}
	catch(Sound::FailedToLoadWave)
	{}

	return false;
}

void SoundPort::shellOpen() const
{}

//-------------------------------------------------------------------------------
// EOF


/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define MUSICPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"
#include "ChildFrame.h"
#include "Resource.h"

#include "Standard/StringHash.h"

//-------------------------------------------------------------------------------

extern AFX_EXTENSION_MODULE					g_PortDLL;

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( MusicPort, Port );
REGISTER_FACTORY_KEY(  MusicPort, 3964190993112938318 );

BEGIN_PROPERTY_LIST( MusicPort, Port );
	ADD_FILE_PROPERTY( m_WaveFile );
	ADD_PROPERTY( m_SegmentId );
	ADD_PROPERTY( m_Segments );
	ADD_PROPERTY( m_Branches );
	ADD_PROPERTY( m_Triggers );
	ADD_PROPERTY( m_BufferSize );
END_PROPERTY_LIST();

MusicPort::MusicPort() : Port()
{
	m_BufferSize = 176400;
}

//-------------------------------------------------------------------------------

const int VERSION_040599 = 1;
const int VERSION_102000 = 102000;

bool MusicPort::read( const InStream & input )
{
	if (! Port::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_102000:
			input >> m_WaveFile;
			input >> m_SegmentId;
			input >> m_Segments;
			input >> m_Branches;
			input >> m_Triggers;
			input >> m_BufferSize;
			return true;
		case VERSION_040599:
			input >> m_WaveFile;
			input >> m_SegmentId;
			input >> m_Segments;
			input >> m_Branches;
			input >> m_Triggers;
			m_BufferSize = 176400;
			return true;
		}
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------

void MusicPort::dependencies( DependentArray & dep )
{}

Resource::Ref MusicPort::createResource()
{
	Music * pMusic = new Music( m_WaveFile );

	for(int i=0;i<m_Segments.size();i++)
	{
		pMusic->addSegment( m_Segments[i] );

		Array< Branch > & branches = m_Branches[ i ];
		for(int j=0;j<branches.size();j++)
		{
			Branch newBranch;
			newBranch.trigger = StringHash( (const char *)m_Triggers[ branches[j].trigger ] );
			newBranch.condition = branches[j].condition;
			newBranch.segment = branches[j].segment;

			pMusic->addBranch( i, newBranch );
		}
	}

	pMusic->setBufferSize( m_BufferSize );

	return pMusic;
}

CFrameWnd * MusicPort::createView()
{
	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

bool MusicPort::importFile( const char * fileName )
{
	m_WaveFile = fileName;
	return true;
}

void MusicPort::shellOpen() const
{
    if ( m_WaveFile.length() > 0 )
        ShellExecute(0, "open", m_WaveFile, NULL, NULL, SW_SHOWNORMAL);
}

//-------------------------------------------------------------------------------
// EOF


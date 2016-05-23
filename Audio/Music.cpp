/*
	Music.cpp

	This class is used to play an interactive digital music track
	(c)2005 Palestar, Richard Lyle
*/

#define AUDIO_DLL
#include "Audio/Music.h" 

#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Standard/StringHash.h"
#include "Audio/Sound.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( Music, Resource );
REGISTER_FACTORY_KEY( Music, 3965769673947648449LL );

Music::Music() 
{
	m_Playing = false;
	m_pFile = NULL;
	m_WaveSize = 0;
	m_pWave = NULL;
	m_ActiveSegment = 0;
	m_ActiveTrigger = 0;
	m_SegmentPos = 0;
	m_BufferSize = 176400;		// 2 seconds of 22050, 16, 2
}

Music::Music( const char * fileName ) 
{
	m_WaveFile = fileName;

	m_Playing = false;
	m_pFile = NULL;
	m_WaveSize = 0;
	m_pWave = NULL;
	m_ActiveSegment = 0;
	m_ActiveTrigger = 0;
	m_SegmentPos = 0;
	m_BufferSize = 176400;		// 2 seconds of 22050, 16, 2
}

Music::~Music()
{
	stop();
}

//-------------------------------------------------------------------------------

const int VERSION_040599 = 1;
const int VERSION_102000 = 102000;

bool Music::read( const InStream & input )
{
	if (! Resource::read( input ) )
		return false;

	int version;
	input >> version;

	switch( version )
	{
	case VERSION_102000:
		input >> m_WaveFile;
		input >> m_Segments;
		input >> m_Branches;
		input >> m_BufferSize;
		return true;
	case VERSION_040599:
		input >> m_WaveFile;
		input >> m_Segments;
		input >> m_Branches;
		m_BufferSize = 176400;		// 2 seconds of 22050, 16, 2
		return true;
	}

	return false;
}

bool Music::write( const OutStream & output )
{
	if (! Resource::write( output ) )
		return false;

	output << VERSION_102000;
	output << m_WaveFile;
	output << m_Segments;
	output << m_Branches;
	output << m_BufferSize;
	return true;
}

//-------------------------------------------------------------------------------

int Music::request( void * pDst, int dstSize )
{
	ASSERT( m_Playing );

	//TRACE(String().format("Music::request, pDst = %p, dstSize = %d", pDst, dstSize) );

	const int	sampleSize = (m_Bits >> 3 ) * m_Channels;
	int			bytesSent = 0;

	while( dstSize > 0 )
	{
		if ( m_ActiveSegment < 0 )
			break;

		Segment & segment = m_Segments[ m_ActiveSegment ];
		dword segmentSize = (segment.end - segment.begin) * sampleSize;

		byte * pBegin = ((byte *)m_pWave) + (segment.begin * sampleSize);
		byte * pEnd = pBegin + segmentSize;

		ASSERT( m_SegmentPos <= segmentSize );
		pBegin += m_SegmentPos;

		int copySize = pEnd - pBegin;
		if ( copySize > dstSize )
			copySize = dstSize;

		memcpy( pDst, pBegin, copySize );

		dstSize -= copySize;
		bytesSent += copySize;
		m_SegmentPos += copySize;
		pDst = ((byte *)pDst) + copySize;
		pBegin += copySize;

		ASSERT( pBegin <= pEnd );

		if ( pBegin == pEnd )
		{
			m_SegmentPos = 0;

			// advance to next segment
			//
			bool branched = false;

			Array< Branch > & branches = m_Branches[ m_ActiveSegment ];
			for(int i=0;i<branches.size();i++)
			{
				Branch & branch = branches[ i ];

				if ( (branch.condition && branch.trigger == m_ActiveTrigger) ||
					(!branch.condition && branch.trigger != m_ActiveTrigger) )
				{
					branched = true;
					m_ActiveSegment = branch.segment;
					break;
				}
			}

			// if we didn't branch, just advance to next segment, then loop
			if ( !branched )
			{
				m_ActiveSegment++;
				if ( m_ActiveSegment >= m_Segments.size() )
					m_ActiveSegment = 0;
			}
		}
	}

	return bytesSent;
}

//-------------------------------------------------------------------------------

void Music::setWaveFile( const char * file )
{
	m_WaveFile = file;
}

int Music::addSegment( const Segment & segment )
{
	m_Segments.push( segment );
	m_Branches.push();

	return( segmentCount() - 1 );
}

void Music::removeSegment( int n )
{
	m_Segments.remove( n );
	m_Branches.remove( n );
}

int Music::addBranch( int segment, const Branch & branch )
{
	m_Branches[ segment ].push( branch );
	return( branchCount( segment ) - 1 );
}

void Music::removeBranch( int segment, int n )
{
	m_Branches[ segment ].remove( n );
}

void Music::setTrigger( dword trigger )
{
	m_ActiveTrigger = trigger;
}

void Music::setTrigger( const char * trigger )
{
	m_ActiveTrigger = StringHash( trigger );
}

void Music::setSegment( int segment )
{
	m_ActiveSegment = segment;
	m_SegmentPos = 0;
}

void Music::setBufferSize( int bufferSize )
{
	m_BufferSize = bufferSize & 0x3 ? (bufferSize | 0x3) + 1 : bufferSize;
}

const int BUFFER_SECONDS = 2;

AudioBuffer * Music::play( AudioDevice * pDevice, float volume )
{
	if (! m_Playing )
	{
		m_Playing = true;
		
		// memory map the wave file
		m_pFile = Sound::mapWave( m_WaveFile, &m_Rate, &m_Channels, &m_Bits, &m_WaveSize, &m_pWave );
		if ( ! m_pFile )
			return NULL;
		
		TRACE( CharString().format( "Music::play, m_WaveFile = %s, m_Rate = %d, m_Channels = %d, mBits = %d, m_WaveSize = %d", 
			m_WaveFile.cstr(), m_Rate, m_Channels, m_Bits, m_WaveSize ) );

		if ( m_Segments.size() < 1 )
		{
			// if no segments defined, play the entire file as one single segment
			//
			const int sampleSize = ((m_Bits >> 3) * m_Channels);
			Segment newSegment;
			newSegment.begin = 0;
			newSegment.end = m_WaveSize / sampleSize;

			addSegment( newSegment );
			m_Branches.allocate( 1 );
		}

		int bufferSize = m_BufferSize;
		if ( bufferSize > m_WaveSize )
			bufferSize = m_WaveSize;
				
		// initialize the audio buffer
		//
		m_Buffer = pDevice->createBuffer();
		ASSERT( m_Buffer.valid() );

		m_Buffer->initializeBuffer( bufferSize, m_Channels, m_Bits, m_Rate );
		m_Buffer->setVolume( volume );
		m_Buffer->play( this );
	}

	return( m_Buffer );
}

void Music::stop()
{
	if ( m_Playing )
	{
		if ( m_Buffer.valid() )
		{
			m_Buffer->stop();
			m_Buffer = NULL;
		}
		if ( m_pFile )
		{
			delete m_pFile;
			m_pFile = NULL;
		}

		m_Playing = false;
	}
}

//-------------------------------------------------------------------------------
// EOF

/*
	Music.h

	This class is used to play an interactive digital music track
	(c)2005 Palestar, Richard Lyle
*/

#ifndef MUSIC_H
#define MUSIC_H

#include "Audio/AudioDevice.h"
#include "Audio/AudioBuffer.h"
#include "Resource/Resource.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Music : public Resource, private AudioBuffer::Stream
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef ResourceLink< Music >	Link;

	struct Segment
	{
		dword				begin;				// beginning sample
		dword				end;				// ending sample
	};
	struct Branch
	{
		dword				trigger;			// StringHash of the trigger name
		bool				condition;			// true ==, false !=
		int					segment;			// jump to which segment
	};

	// Construction
	Music();
	Music( const char * );
	virtual ~Music();

	// Widget Inteface
	bool					read( const InStream & );
	bool					write( const OutStream & );
	// AudioBuffer::Stream Interface
	int						request( void * pDst, int dstSize );

	// Accessors
	const char *			waveFile() const;
	int						segmentCount() const;
	const Segment &			segment( int n ) const;
	int						branchCount( int segment ) const;
	const Branch &			branch( int segment, int n ) const;

	int						activeSegment() const;
	dword					activeTrigger() const;

	int						bufferSize() const;
	bool					playing() const;

	// Mutators
	void					setWaveFile( const char * file );
	int						addSegment( const Segment & segment );
	void					removeSegment( int n );
	int						addBranch( int segment, const Branch & branch );
	void					removeBranch( int segment, int n );

	void					setTrigger( dword trigger );			// control the music with these functions
	void					setTrigger( const char * trigger );
	void					setSegment( int segment );

	void					setBufferSize( int bufferSize );

	AudioBuffer *			play( AudioDevice * pDevice, float volume ); 
	void					stop();

private:
	// Data
	CharString				m_WaveFile;			// filename of the wave file
	Array< Segment >		m_Segments;			// segments of the wave file
	Array< Array< Branch > >
							m_Branches;			// branches for each segments

	bool					m_Playing;			// is this music playing
	File *					m_pFile;			// memory mapped wave file
	int						m_WaveSize;			// size of wave data
	void *					m_pWave;			// wave data pointer
	int						m_Channels;
	int						m_Bits;
	int						m_Rate;
	int						m_BufferSize;		// buffer size in bytes of the audio buffer
	AudioBuffer::Ref		m_Buffer;			// current audio buffer

	int						m_ActiveSegment;	// currently active segment
	dword					m_ActiveTrigger;	// current trigger
	dword					m_SegmentPos;		// current position in the active segment
};

IMPLEMENT_RAW_STREAMING( Music::Segment );
IMPLEMENT_RAW_STREAMING( Music::Branch );

//-------------------------------------------------------------------------------

inline const char * Music::waveFile() const
{
	return( m_WaveFile );
}

inline int Music::segmentCount() const
{
	return( m_Segments.size() );
}

inline const Music::Segment & Music::segment( int n ) const
{
	return( m_Segments[ n ] );
}

inline int Music::branchCount( int segment ) const
{
	return( m_Branches[ segment ].size() );
}

inline const Music::Branch & Music::branch( int segment, int n ) const
{
	return( m_Branches[ segment ][ n ] );
}

inline int Music::activeSegment() const
{
	return( m_ActiveSegment );
}

inline dword Music::activeTrigger() const
{
	return( m_ActiveTrigger );
}

inline int Music::bufferSize() const
{
	return m_BufferSize;
}

inline bool Music::playing() const
{
	return( m_Playing );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

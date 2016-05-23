/*
	Sound.cpp

	Sound class, used to contain a waveform
	(c)2005 Palestar, Richard Lyle
*/

#define USE_SEMAPHORE

#define AUDIO_DLL
#include "Debug/Assert.h"
#include "File/FileMap.h"
#include "File/FileDisk.h"
#include "Audio/Sound.h"

//----------------------------------------------------------------------------

List< Sound * >		Sound::s_SoundList;
CriticalSection		Sound::s_SoundListLock;

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( Sound, Resource );
REGISTER_FACTORY_KEY(  Sound, 3964272773223354185LL );

BEGIN_PROPERTY_LIST( Sound, Resource );
	ADD_PROPERTY( m_Rate );
	ADD_PROPERTY( m_Channels );
	ADD_PROPERTY( m_Bits );
END_PROPERTY_LIST();

Sound::Sound() : m_Rate(-1), m_Channels(-1), m_Bits(-1), 
	m_WaveSize(-1), m_pWave(NULL)
{
#ifdef USE_SEMAPHORE
	s_SoundListLock.lock();
	s_SoundList.insert( this );
	s_SoundListLock.unlock();
#else
	s_SoundList.insert( this );
#endif
}

Sound::Sound( const char * waveFile ) : m_Rate(-1), m_Channels(-1), m_Bits(-1), 
	m_WaveSize(-1), m_pWave(NULL)
{
	if ( !loadWave( waveFile, &m_Rate, &m_Channels, &m_Bits, &m_WaveSize, &m_pWave ) )
		throw FailedToLoadWave();

#ifdef USE_SEMAPHORE
	s_SoundListLock.lock();
	s_SoundList.insert( this );
	s_SoundListLock.unlock();
#else
	s_SoundList.insert( this );
#endif
}

Sound::~Sound()
{
	release();

	// remove from linked list
#ifdef USE_SEMAPHORE
	s_SoundListLock.lock();
	s_SoundList.remove( this );
	s_SoundListLock.unlock();
#else
	s_SoundList.remove( this );
#endif
}

//----------------------------------------------------------------------------

const int VERSION_040199 = 1;
const int VERSION_20060520 = 20060520;

bool Sound::read( const InStream & input )
{
	if (! Resource::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_040199:
			input >> m_Rate;
			input >> m_Channels;
			input >> m_Bits;
			input >> m_WaveSize;

			m_pWave = new byte[ m_WaveSize ];
			ASSERT( m_pWave );

			input.read( m_pWave, m_WaveSize );
			break;
		default:
			return false;
		}

		return true;
	}

	int version;
	input >> version;

	switch( version )
	{
	case VERSION_20060520:
		input >> m_WaveSize;

		m_pWave = new byte[ m_WaveSize ];
		if (! m_pWave )
			return false;
		input.read( m_pWave, m_WaveSize );
		break;
	}

	return true;
}

bool Sound::write( const OutStream & output )
{
	if (! Resource::write( output ) )
		return false;

	output << VERSION_20060520;
	output << m_WaveSize;
	output.write( m_pWave, m_WaveSize );

	return true;
}

//----------------------------------------------------------------------------

void Sound::initializeSound( int rate, int channels, int bits, 
				int waveSize, const void * pWave )
{
	release();

	m_Rate = rate;
	m_Channels = channels;
	m_Bits = bits;
	m_WaveSize = waveSize;

	m_pWave = new byte[ m_WaveSize ];
	ASSERT( m_pWave );
}

void Sound::release()
{
	m_Buffer = NULL;

	if ( m_pWave )
	{
		delete (byte *)m_pWave;
		m_pWave = NULL;
	}
}

AudioBuffer * Sound::play( AudioDevice * pDevice, bool looped /*= false*/ )
{
	return( play( pDevice, 0.5f, 0.5f, looped ) );
}

AudioBuffer * Sound::play( AudioDevice * pDevice, float volume, float pan, bool looped /*= false*/ )
{
	if (! pDevice )
		return NULL;

	if ( ! m_Buffer.valid() )
		initializeAudioBuffer( pDevice );

	AudioBuffer::Ref dupBuffer = pDevice->duplicateBuffer( m_Buffer );
	if ( dupBuffer.valid() )
	{
		dupBuffer->setVolume( volume );
		dupBuffer->setPan( pan );
		if ( ! dupBuffer->play( looped ) )
		{
			// out of voices, return NULL
			return NULL;
		}
	}

	return( dupBuffer );
}

bool Sound::exportWave( const char * pFileName )
{
	return saveWave( pFileName, m_Rate, m_Channels, m_Bits, m_WaveSize, m_pWave );
}

//----------------------------------------------------------------------------

typedef dword	IdTag;

struct IFF_FORM_CHUNK
{
	IdTag		form_id;
	dword		form_length;
	IdTag		id;
};

struct IFF_CHUNK
{
	IdTag		id;
	dword		length;
};

struct WAV_PCM
{
	word  format_tag;
	word  channels;
	long  sample_rate;
	long  average_data_rate;
	word  alignment;
	word  bits_per_sample;
};

struct WAV_CUE
{
	dword	number;					// cue number
	dword	start;					// starting sample frame
	dword	id;						// cue id
	dword	reserved1,reserved2;	// ?
	dword	end;					// ending frame
};

struct WAV_CUE_LABEL
{
	IFF_CHUNK	chunk;
	dword		number;					// cue number
	dword		reserved;
	char		name;					// cue name, length of string to end of chunk + chunk.length
};

#define CHUNKID(string)		(*((IdTag *)(string)))
#define IDTAG(a,b,c,d)		((IdTag)((d)<<24)|((c)<<16)|((b)<<8)|(a))

bool Sound::loadWave( const char * pFilename, int * pRate, int * pChannels, int *pBits, 
			int * pWaveSize, void ** pWave )
{
	int		fileSize = 0;
	byte *	pFile = NULL;

	FileDisk file;
	if ( !file.open( pFilename, FileDisk::READ ) )
		return false;

	fileSize = (int)file.size();
	pFile = new byte[ fileSize ];
	ASSERT( pFile );

	try {
		file.read( pFile, fileSize );
	}
	catch( FileDisk::FileError )
	{
		delete [] pFile;
		return false;
	}

	IFF_FORM_CHUNK	*pForm = (IFF_FORM_CHUNK *)pFile;
	if ( pForm->form_id != CHUNKID("RIFF") || pForm->id != CHUNKID("WAVE") )
	{
		delete [] pFile;
		return false;
	}

	IFF_CHUNK *pEnd = (IFF_CHUNK *)(pFile + fileSize);
	IFF_CHUNK *pChunk = (IFF_CHUNK *)(pFile + sizeof(IFF_FORM_CHUNK));

	while (pChunk < pEnd)
	{
		switch(pChunk->id)
		{
			case IDTAG('f','m','t',' '):
				{
					WAV_PCM *pPCM = (WAV_PCM *)(pChunk + 1);

					if ( pRate )
						*pRate = pPCM->sample_rate;
					if ( pChannels )
						*pChannels = pPCM->channels;
					if ( pBits )
						*pBits = pPCM->bits_per_sample;
				}
				break;
			case IDTAG('d','a','t','a'):
				{
					if ( pWaveSize )
						*pWaveSize = pChunk->length;
					if ( pWave )
					{
						*pWave = new byte[ pChunk->length ];
						ASSERT( *pWave );

						memcpy( *pWave, pChunk + 1, pChunk->length );
					}
				}
				break;
			default:
				break;
		}

		int length = pChunk->length & 0x1 ? (pChunk->length + 1) : pChunk->length;
		pChunk = (IFF_CHUNK *)(((byte *)(pChunk + 1)) + length);
	}

	delete [] pFile;
	return true;
}


File * Sound::mapWave( const char * pFilename, int * pRate, int * pChannels, int *pBits, 
			int * pWaveSize, void ** pWave )
{
	FileMap *	pFileMap = NULL;

	dword		fileSize = 0;
	byte *		pFile = NULL;

	pFileMap = new FileMap;
	if ( !pFileMap->open( pFilename ) )
	{
		delete pFileMap;
		return NULL;
	}

	fileSize = (dword)pFileMap->size();
	pFile = (byte *)pFileMap->view();

	IFF_FORM_CHUNK	*pForm = (IFF_FORM_CHUNK *)pFile;
	if ( pForm->form_id != CHUNKID("RIFF") || pForm->id != CHUNKID("WAVE") )
	{
		delete pFileMap;
		return NULL;
	}

	IFF_CHUNK *pEnd = (IFF_CHUNK *)(pFile + fileSize);
	IFF_CHUNK *pChunk = (IFF_CHUNK *)(pFile + sizeof(IFF_FORM_CHUNK));

	while (pChunk < pEnd)
	{
		switch(pChunk->id)
		{
			case IDTAG('f','m','t',' '):
				{
					WAV_PCM *pPCM = (WAV_PCM *)(pChunk + 1);

					if ( pRate )
						*pRate = pPCM->sample_rate;
					if ( pChannels )
						*pChannels = pPCM->channels;
					if ( pBits )
						*pBits = pPCM->bits_per_sample;
				}
				break;
			case IDTAG('d','a','t','a'):
				{
					if ( pWaveSize )
						*pWaveSize = pChunk->length;
					if ( pWave )
						*pWave = pChunk + 1;
				}
				break;
			default:
				break;
		}

		int length = pChunk->length & 0x1 ? (pChunk->length + 1) : pChunk->length;
		pChunk = (IFF_CHUNK *)(((byte *)(pChunk + 1)) + length);
	}

	return pFileMap;
}

bool Sound::saveWave( const char * pFileName, int nRate, int nChannels, int nBits, int nWaveSize, void * pWave )
{
	IFF_FORM_CHUNK form;
	form.form_id = CHUNKID("RIFF");
	form.form_length = sizeof(IFF_CHUNK) + sizeof(WAV_PCM) + 
		sizeof(IFF_CHUNK) + nWaveSize + sizeof(form.id);
	form.id = CHUNKID("WAVE");

	IFF_CHUNK format;
	format.id = CHUNKID("fmt ");
	format.length = sizeof(WAV_PCM);

	WAV_PCM pcm;
	pcm.format_tag = 0x1;
	pcm.channels = nChannels;
	pcm.sample_rate = nRate;
	pcm.average_data_rate = (nRate * nBits * nChannels) / 8;
	pcm.alignment = 0x2;
	pcm.bits_per_sample = nBits;

	IFF_CHUNK data;
	data.id = CHUNKID("data");
	data.length = nWaveSize;

	FileDisk fd;
	if ( !fd.open( pFileName, FileDisk::WRITE ) )
		return false;

	try {
		fd.write( &form, sizeof(form) );
		fd.write( &format, sizeof(format) );
		fd.write( &pcm, sizeof(pcm) );
		fd.write( &data, sizeof(data) );
		fd.write( pWave, nWaveSize );
		fd.close();
	}
	catch(File::FileError)
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------

int sortWavCues( const void *arg1, const void *arg2 )
{
	return( ((WAV_CUE *)arg1)->start - ((WAV_CUE *)arg2)->start );
}

bool Sound::loadRegions( const char * pFilename, Array< Region > & regions )
{
	FileMap *	pFileMap = NULL;

	dword		fileSize = 0;
	byte *		pFile = NULL;

	pFileMap = new FileMap;
	if ( !pFileMap->open( pFilename ) )
	{
		delete pFileMap;
		return false;
	}

	fileSize = (dword)pFileMap->size();
	pFile = (byte *)pFileMap->view();

	IFF_FORM_CHUNK	*pForm = (IFF_FORM_CHUNK *)pFile;
	if ( pForm->form_id != CHUNKID("RIFF") || pForm->id != CHUNKID("WAVE") )
	{
		delete pFileMap;
		return false;
	}

	IFF_CHUNK *pEnd = (IFF_CHUNK *)(pFile + fileSize);
	IFF_CHUNK *pChunk = (IFF_CHUNK *)(pFile + sizeof(IFF_FORM_CHUNK));

	int			cueCount = 0;
	WAV_CUE *	cueList = NULL;
	//WAV_CUE_LABEL * cueLabels = NULL;

	while (pChunk < pEnd)
	{
		switch(pChunk->id)
		{
			case IDTAG('c','u','e',' '):
				{
					dword * pCueCount = (dword *)(pChunk + 1);
					cueCount = *pCueCount;
					
					cueList = new WAV_CUE[ cueCount ];
					memcpy( cueList, pCueCount + 1, sizeof(WAV_CUE) * cueCount );
					
					// sort the cue's by starting sample
					qsort( cueList, cueCount, sizeof(WAV_CUE), sortWavCues );
				}
				break;
				/*
			case IDTAG('L','I','S','T'):	// this chunk contains the cue names
				{
					dword * pListId = (dword *)(pChunk + 1);

					if (*pListId == CHUNKID("adtl"))
						cueLabels = (WAV_CUE_LABEL *)(pListId + 1);
				}
				break;
				*/
			default:
				break;
		}

		int length = pChunk->length & 0x1 ? (pChunk->length + 1) : pChunk->length;
		pChunk = (IFF_CHUNK *)(((byte *)(pChunk + 1)) + length);
	}

	if ( cueList != NULL )
	{
		regions.allocate( cueCount );
		
		dword frame = 0;
		for(int i=0;i<cueCount;i++)
		{
			regions[i].name = String().format(STR("%3.3d"), i );	// generate default name

			/*
			// locate label for current region
			//
			if ( cueLabels )
			{
				WAV_CUE_LABEL * pLabel = cueLabels;

				for(int k=0;k<cueCount;k++)
				{
					if ( pLabel->number == cueList[i].number )
					{
						regions[i].name = &pLabel->name;
						break;
					}

					// advance to the next label
					int length = cueLabels->chunk.length + (cueLabels->chunk.length & 0x1);
					pLabel = (WAV_CUE_LABEL *)((byte *)pLabel + (length + sizeof(IFF_CHUNK)));
				}
				ASSERT( k < cueCount );
			}
			*/

			regions[i].start = frame;
			regions[i].end = cueList[i].start;
			frame = cueList[i].end;
		}

		delete [] cueList;
	}

	delete pFileMap;
	return( regions.size() > 0 ? true : false );
}

void Sound::releaseAudioBuffers()
{
	TRACE( "Sound::releaseAudioBuffers" );

	s_SoundListLock.lock();
	List< Sound * >::Iterator it = s_SoundList.head();
	while( it.valid() )
	{
		(*it)->m_Buffer = NULL;
		it++;
	}
	s_SoundListLock.unlock();
}

//----------------------------------------------------------------------------

void Sound::initializeAudioBuffer( AudioDevice * pDevice )
{
	// create the audio buffer
	//
	m_Buffer = pDevice->createBuffer();
	ASSERT( m_Buffer.valid() );

	// initialize the audio buffer
	//
	m_Buffer->initializeBuffer( m_WaveSize, m_Channels, m_Bits, m_Rate );

	// copy in the waveform
	//
	void * pWave = m_Buffer->lockBuffer();
	ASSERT( pWave );

	memcpy( pWave, m_pWave, m_WaveSize );
	m_Buffer->unlockBuffer();
}


//----------------------------------------------------------------------------
// EOF

/*
	AudioCodecSpeech.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Debug/Assert.h"
#include "AudioDS/AudioCodecSpeech.h"

//----------------------------------------------------------------------------

#ifdef HYDRA_STATIC
// allow programs to include this class by add /INCLUDE:AUDIO_CODEC_SPEECH to their linker options
extern "C" int AUDIO_CODEC_SPEECH = 1;
#endif

IMPLEMENT_FACTORY( AudioCodecSpeech, AudioCodec );

AudioCodecSpeech::AudioCodecSpeech() : m_hDriver( NULL ), m_PreEncodeStream( NULL ), m_EncodeStream( NULL ), 
	m_DecodeStream( NULL ), m_FinalDecodeStream( NULL ), m_PreEncodeWF( NULL ), m_EncodedWF( NULL )
{}

AudioCodecSpeech::~AudioCodecSpeech()
{
	release();
}

//----------------------------------------------------------------------------

struct DriverInfo
{
	String					name;		// driver name
	HACMDRIVERID			id;			// driver id
	Array< WAVEFORMATEX * >	formats;	// formats supported

	DriverInfo & operator=( const DriverInfo & copy )
	{
		name = copy.name;
		id = copy.id;
		formats = copy.formats;
		return *this;
	}
};

static BOOL CALLBACK FormatEnum(HACMDRIVERID hadid, LPACMFORMATDETAILS pafd, DWORD dwInstance, DWORD fdwSupport)
{
	DriverInfo & driver = *((DriverInfo *)dwInstance);

	WAVEFORMATEX * pwf = (WAVEFORMATEX*)new byte[ pafd->cbwfx ];
	memcpy( pwf, pafd->pwfx, pafd->cbwfx );

	driver.formats.push( pwf );   

	//TRACE( String("FormatEnum - %s", pafd->szFormat ) );

	return TRUE; // continue enumerating
}

static BOOL CALLBACK DriverEnum(HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport)
{
	Array< DriverInfo > & drivers = *((Array< DriverInfo > *)dwInstance);

	ACMDRIVERDETAILS dd;
	memset( &dd, 0, sizeof(dd) );
	dd.cbStruct = sizeof(dd);

	if ( acmDriverDetails(hadid, &dd, 0 ) )
		return TRUE;
	if ( dd.wPid != MM_DSP_GROUP_TRUESPEECH || dd.wMid != MM_DSP_GROUP )
		return TRUE;		// This keeps us from loading lots of DLLs

	// open the driver
	HACMDRIVER had = NULL;
	if ( acmDriverOpen(&had, hadid, 0) )
		return TRUE;	// error

	DriverInfo & newDriver = drivers.push();
	newDriver.name = dd.szShortName;
	newDriver.id = hadid;

	//TRACE( String("DriverEnum - %s", newDriver.name ) );

	// enumerate the formats it supports
	DWORD dwSize = 0;
	if ( acmMetrics((HACMOBJ)had, ACM_METRIC_MAX_SIZE_FORMAT, &dwSize) )
		return TRUE;	// error
	if (dwSize < sizeof(WAVEFORMATEX)) 
		dwSize = sizeof(WAVEFORMATEX); // for MS-PCM

	WAVEFORMATEX * pwf = (WAVEFORMATEX*)malloc( dwSize );
	memset(pwf, 0, dwSize);

	pwf->cbSize = LOWORD(dwSize) - sizeof(WAVEFORMATEX);
	pwf->wFormatTag = WAVE_FORMAT_UNKNOWN;
	
	ACMFORMATDETAILS fd;
	memset(&fd, 0, sizeof(fd));

	fd.cbStruct = sizeof(fd);
	fd.pwfx = pwf;
	fd.cbwfx = dwSize;
	fd.dwFormatTag = WAVE_FORMAT_UNKNOWN;

	acmFormatEnum(had, &fd, FormatEnum, (DWORD)&newDriver, 0);

	// release and close
	free(pwf);
	acmDriverClose(had, 0);

	return TRUE; // continue enumeration
}

static void EnumDrivers( Array< DriverInfo > & drivers )
{
	acmDriverEnum( DriverEnum, (DWORD)&drivers, 0);
}

//----------------------------------------------------------------------------

bool AudioCodecSpeech::initialize( PCM & format )
{
	release();

	memset( &m_DecodedWF, 0 , sizeof( m_DecodedWF ) );
	m_DecodedWF.wFormatTag = WAVE_FORMAT_PCM; 
	m_DecodedWF.nChannels = format.channels;
	m_DecodedWF.nSamplesPerSec = format.rate;
	m_DecodedWF.wBitsPerSample = format.bits;
	m_DecodedWF.nBlockAlign = m_DecodedWF.nChannels * m_DecodedWF.wBitsPerSample / 8;
	m_DecodedWF.nAvgBytesPerSec = m_DecodedWF.nSamplesPerSec * m_DecodedWF.nBlockAlign;
	
	// enumerate all the available ACM drivers and their formats!
	Array< DriverInfo > drivers;
	EnumDrivers( drivers );

	// find the TrueSpeech driver and initialize that driver
	for(int i=0;i<drivers.size() && m_hDriver == NULL;i++)
		for(int j=0;j<drivers[i].formats.size() && m_hDriver == NULL;j++)
			if ( drivers[i].formats[j]->wFormatTag == WAVE_FORMAT_DSPGROUP_TRUESPEECH )
			{
				// save the encoded format
				m_EncodedWF = drivers[i].formats[j];

				// assume we will have to pre-encode unless we find a exact match, then
				// the driver will take our PCM waveform directly
				bool doPreEncode = true;

				// found the driver, look for the closest pcm format it takes
				int bestScore = -1;
				for(int k=0;k<drivers[i].formats.size();k++)
				{
					WAVEFORMATEX * pCheck = drivers[i].formats[k];
					if ( pCheck->wFormatTag == m_DecodedWF.wFormatTag )
					{
						// pick the closest PCM format to our input/output format
						int score = 0;
						if ( pCheck->nChannels == m_DecodedWF.nChannels )
							score++;
						if ( pCheck->nSamplesPerSec == m_DecodedWF.nSamplesPerSec )
							score++;
						if ( pCheck->wBitsPerSample == m_DecodedWF.wBitsPerSample )
							score++;
						if ( pCheck->cbSize == m_DecodedWF.cbSize )
							score++;

						if ( score > bestScore )
						{
							bestScore = score;
							m_PreEncodeWF = drivers[i].formats[k];

							if ( score == 4 )
							{
								// found perfect match, our driver will take our format directly!
								doPreEncode = false;
								break;
							}
						}
					}
				}

				if ( bestScore < 0 )
					return false;		// driver takes no valid PCM format

				// open the driver
				if ( acmDriverOpen( &m_hDriver, drivers[i].id, 0 ) )
					return false;

				if ( doPreEncode )
				{
					if ( acmStreamOpen( &m_PreEncodeStream,
						NULL,				// any driver
						&m_DecodedWF,		// source format
						m_PreEncodeWF,		// destination format
						NULL,				// no filter
						NULL,				// no callback
						0,					// instance data
						ACM_STREAMOPENF_NONREALTIME) )
							return false;	// failed to open the stream

					if ( acmStreamOpen( &m_FinalDecodeStream,
						NULL,				// any driver
						m_PreEncodeWF,		// source format
						&m_DecodedWF,		// destination format
						NULL,				// no filter
						NULL,				// no callback
						0,					// instance data
						ACM_STREAMOPENF_NONREALTIME) )
							return false;	// failed to open the stream
				}

				if ( acmStreamOpen( &m_EncodeStream,
					m_hDriver,			// driver
					m_PreEncodeWF,		// source format
					m_EncodedWF,		// destination format
					NULL,				// no filter
					NULL,				// no callback
					0,					// instance data
					ACM_STREAMOPENF_NONREALTIME) )
						return false;	// failed to open the stream

				if ( acmStreamOpen( &m_DecodeStream,
					m_hDriver,			// driver
					m_EncodedWF,		// source format
					m_PreEncodeWF,		// destination format
					NULL,				// no filter
					NULL,				// no callback
					0,					// instance data
					ACM_STREAMOPENF_NONREALTIME) )
						return false;	// failed to open the stream

			}

	if ( m_hDriver == NULL )
		return false;

	return true;
}

bool AudioCodecSpeech::release()
{
	if ( m_PreEncodeStream != NULL )
	{
		acmStreamClose( m_PreEncodeStream, 0 );
		m_PreEncodeStream = NULL;
	}
	if ( m_EncodeStream != NULL )
	{
		acmStreamClose( m_EncodeStream, 0 );
		m_EncodeStream = NULL;
	}
	if ( m_DecodeStream != NULL )
	{
		acmStreamClose( m_DecodeStream, 0 );
		m_DecodeStream = NULL;
	}
	if ( m_FinalDecodeStream != NULL )
	{
		acmStreamClose( m_FinalDecodeStream, 0 );
		m_FinalDecodeStream = NULL;
	}
	if ( m_hDriver != NULL )
	{
		acmDriverClose(m_hDriver, 0);
		m_hDriver = NULL;
	}

	return false;
}

bool AudioCodecSpeech::encode(const Buffer & input, Buffer & output )
{
	const Buffer *	pEncode = &input;		// pointer to data to encode, changed if we're preEncoding the data to another format
	dword			encodeSize = input.bufferSize();

	Buffer			preEncoded;				// buffer for data that might need to be pre-encoded before sending to the real driver
	if ( m_PreEncodeStream != NULL )
	{
		dword preEncodeSize;
		acmStreamSize( m_PreEncodeStream, encodeSize, &preEncodeSize, ACM_STREAMSIZEF_SOURCE );
		preEncoded.allocate( preEncodeSize );
		
		ACMSTREAMHEADER preEncodeHeader;
		memset(&preEncodeHeader, 0, sizeof(preEncodeHeader));
		preEncodeHeader.cbStruct = sizeof(preEncodeHeader);
		preEncodeHeader.pbSrc = (unsigned char *)input.buffer();
		preEncodeHeader.cbSrcLength = input.bufferSize();
		preEncodeHeader.pbDst = (unsigned char *)preEncoded.buffer();
		preEncodeHeader.cbDstLength = preEncoded.bufferSize();

		// prep the header
		if ( acmStreamPrepareHeader(m_PreEncodeStream, &preEncodeHeader, 0) )
			return false;
		// convert the PCM data
		if ( acmStreamConvert(m_PreEncodeStream, &preEncodeHeader, 0) )
			return false;
		acmStreamUnprepareHeader(m_PreEncodeStream, &preEncodeHeader, 0 );

		// change the encode pointer to our preEncoded buffer
		pEncode = &preEncoded;
		// set the size of the encoded data
		encodeSize = preEncodeHeader.cbDstLengthUsed;
	}

	// dermine the size of the encoded data
	dword encodedSize;
	acmStreamSize( m_EncodeStream, encodeSize, &encodedSize, ACM_STREAMSIZEF_SOURCE );

	// allocate the output buffer
	Buffer encoded;
	encoded.allocate( encodedSize );

	ACMSTREAMHEADER encodeHeader;
	memset(&encodeHeader, 0, sizeof(encodeHeader));
	encodeHeader.cbStruct = sizeof(encodeHeader);
	encodeHeader.pbSrc = (unsigned char *)pEncode->buffer();
	encodeHeader.cbSrcLength = encodeSize;
	encodeHeader.pbDst = (unsigned char *)encoded.buffer();
	encodeHeader.cbDstLength = encoded.bufferSize();

	// prep the header
	if ( acmStreamPrepareHeader(m_EncodeStream, &encodeHeader, 0) )
		return false;
	// convert the PCM data
	if ( acmStreamConvert(m_EncodeStream, &encodeHeader, 0 ) )
		return false;

	// allocate the output buffer and copy the results
	output.allocate( encodeHeader.cbDstLengthUsed );
	memcpy( output.buffer(), encoded.buffer(), encodeHeader.cbDstLengthUsed );

	acmStreamUnprepareHeader(m_EncodeStream, &encodeHeader, 0 );

	return true;
}

bool AudioCodecSpeech::decode( const Buffer & input, Buffer & output )
{
	dword decodeSize;
	acmStreamSize( m_DecodeStream, input.bufferSize(), &decodeSize, ACM_STREAMSIZEF_SOURCE );

	Buffer decoded;
	decoded.allocate( decodeSize + 5000 );

	ACMSTREAMHEADER decodeHeader;
	memset(&decodeHeader, 0, sizeof(decodeHeader));
	decodeHeader.cbStruct = sizeof(decodeHeader);
	decodeHeader.pbSrc = (unsigned char *)input.buffer();
	decodeHeader.cbSrcLength = input.bufferSize();
	decodeHeader.pbDst = (unsigned char *)decoded.buffer();
	decodeHeader.cbDstLength = decoded.bufferSize();

	// prep the header
	if ( acmStreamPrepareHeader(m_DecodeStream, &decodeHeader, 0) )
		return false;
	// convert from the encoded format to PCM
	if ( acmStreamConvert(m_DecodeStream, &decodeHeader, 0) )
		return false;

	acmStreamUnprepareHeader( m_DecodeStream, &decodeHeader, 0 );

	if ( m_FinalDecodeStream != NULL )
	{
		dword finalSize;
		acmStreamSize( m_FinalDecodeStream, decodeHeader.cbDstLengthUsed, &finalSize, ACM_STREAMSIZEF_SOURCE );

		Buffer final;
		final.allocate( finalSize );
		
		ACMSTREAMHEADER finalDecodeHeader;
		memset(&finalDecodeHeader, 0, sizeof(finalDecodeHeader));
		finalDecodeHeader.cbStruct = sizeof(finalDecodeHeader);
		finalDecodeHeader.pbSrc = (unsigned char *)decoded.buffer();
		finalDecodeHeader.cbSrcLength = decodeHeader.cbDstLengthUsed;
		finalDecodeHeader.pbDst = (unsigned char *)final.buffer();
		finalDecodeHeader.cbDstLength = final.bufferSize();

		// prep the header
		if ( acmStreamPrepareHeader(m_FinalDecodeStream, &finalDecodeHeader, 0) )
			return false;
		// convert the PCM data
		if ( acmStreamConvert(m_FinalDecodeStream, &finalDecodeHeader, 0 ) )
			return false;

		output.allocate( finalDecodeHeader.cbDstLengthUsed );
		memcpy( output.buffer(), final.buffer(), finalDecodeHeader.cbDstLengthUsed );

		acmStreamUnprepareHeader( m_FinalDecodeStream, &finalDecodeHeader, 0 );
	}
	else
	{
		output.allocate( decodeHeader.cbDstLengthUsed );
		memcpy( output.buffer(), decoded.buffer(), decodeHeader.cbDstLengthUsed );
	}


	return true;
}

//----------------------------------------------------------------------------
//EOF


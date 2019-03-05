/*
	Time.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "String.h"
#include "Time.h"
#include "Bits.h"

#include "Debug/SafeThread.h"

#include <time.h>
#include <sys/timeb.h>

//! Define to non-zero to enable the use of QueryPerformanceCount() for Time::milliseconds()..
#define ENABLE_QPC				1

//! Define to non-zero to enable the use of winmm for Time::milliseconds()...
#define ENABLE_WINMM			0

#if defined(_WIN32)
#include <windows.h>
#endif

#pragma warning( disable : 4996 )	// warning C4996: 'localtime' was declared deprecated

#if ENABLE_QPC && defined(_WIN32)

class QPCThread : public SafeThread
{
public:
	QPCThread() : SafeThread( CRITICAL ), m_bActive( true ), m_nMilliseconds( 0 )
	{}
	virtual ~QPCThread()
	{
		m_bActive = false;
	}

	virtual int run()
	{
		DWORD nProcessAffin = 0;
		DWORD nSystemAffin = 0;
		if (! GetProcessAffinityMask( GetCurrentProcess(), &nProcessAffin, &nSystemAffin ) )
			THROW_EXCEPTION( "Call failed to GetProcessAffinityMask()" );

		// force this thread to run on the first core assigned to our process..
		DWORD nProcessor = GetFirstBit( (nProcessAffin & nSystemAffin) );
		if (! SetThreadAffinityMask( GetCurrentThread(), 1 << nProcessor ) )
			THROW_EXCEPTION( "Call failed to SetThreadAffinityMask()" );

		while( m_bActive )
		{
			LARGE_INTEGER f, t;
			if (!QueryPerformanceFrequency( &f ) )
				THROW_EXCEPTION( "Call failed to QueryPerformanceFrequency()" );

			if (! QueryPerformanceCounter( &t ) )
				THROW_EXCEPTION( "Call failed to QueryPerformanceCounter()" );

			m_nMilliseconds = (dword)((1000LL * t.QuadPart) / f.QuadPart);

			// sleep for 1 millisecond ..
			Sleep( 1 );
		}

		return 0;
	}

	volatile bool		m_bActive;
	volatile dword		m_nMilliseconds;
};

static QPCThread		s_QPCThread;

#endif

//----------------------------------------------------------------------------

const int MAX_TIME_FORMAT_STRING = 256;

//----------------------------------------------------------------------------

qword Time::CPU()
{
#if defined(_WIN32) || defined(_XBOX)
	qword cpu_clock;

	__asm
	{
		push	eax
		push	edx

		_emit	0xf	
		_emit	0x31		// rdtsc
	
		mov		dword ptr [cpu_clock],eax
		mov		dword ptr [cpu_clock + 4],edx

		pop		edx
		pop		eax
	}	

	return cpu_clock;
#else
	qword x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
#endif
}

dword Time::seconds()
{
#if defined(_WIN32)
	return (dword)::_time32( NULL );
#else
	return (dword)::time( NULL );
#endif
}

dword Time::milliseconds()
{
#if defined(_WIN32)
#if ENABLE_QPC 
	if ( s_QPCThread.suspended() )
	{
		s_QPCThread.resume();
		Sleep( 1 );		// let the first calculation occur..
	}

	return s_QPCThread.m_nMilliseconds;
#elif ENABLE_WINMM
	static DWORD (WINAPI * pTimeGetTime)() = (DWORD (WINAPI *)())GetProcAddress( LoadLibrary( "winmm" ), "timeGetTime" );;
	if ( pTimeGetTime != NULL )
		return pTimeGetTime();
#endif
	return GetTickCount();
#else

	static dword nStartTime = Time::seconds();

	timeb t;
	ftime( &t );

	return ((t.time - nStartTime) * 1000) + t.millitm;

#endif	// #if defined(_WIN32)
}

WideString Time::format( dword nTime, const wchar * pFormat )
{
	wchar sOut[ MAX_TIME_FORMAT_STRING ];
#if defined(_WIN32)
	wcsftime( sOut, MAX_TIME_FORMAT_STRING, pFormat, _localtime32( (__time32_t *)&nTime ) );
#else
	wcsftime( sOut, MAX_TIME_FORMAT_STRING, pFormat, localtime( (time_t *)&nTime ) );
#endif
	return WideString( sOut );
}

CharString Time::format( dword nTime, const char * pFormat )
{
	char sOut[ MAX_TIME_FORMAT_STRING ];
#if defined(_WIN32)
	strftime( sOut, MAX_TIME_FORMAT_STRING, pFormat, _localtime32( (__time32_t *)&nTime ) );
#else
	strftime( sOut, MAX_TIME_FORMAT_STRING, pFormat, localtime( (time_t *)&nTime ) );
#endif
	return CharString( sOut );
}

static bool CheckMaskPart( const char * pMask, int n )
{
	if ( pMask[0] == '*' )
		return true;
	if ( pMask[0] == 0 )
		return false;

	const char * pSplit = strchr( pMask, '-' );
	if ( pSplit != NULL && pSplit > pMask )
	{
		// range k-j
		int k = atoi( pMask );
		int j = atoi( pSplit + 1 );
		return n >= k && n <= j;
	}

	int k = atoi( pMask );
	if ( pMask[0] == '+' && k != 0 )
		return (n % k) == 0;
	else
		return k == n;
}

// mask is formated as "MON/DAY/YEAR HOUR:MIN:SEC"
bool Time::isTime( dword nSeconds, const char * pMask )
{
	CharString sMask = pMask;
	if ( sMask.length() == 0 )
		return false;

	// get the mask parts
	CharString sParts[ 6 ];
	for(int i=0;i<6 && sMask.tokenize( sParts[i], " /:");i++);

#if defined(_WIN32)
	tm * pTime = _localtime32( (const __time32_t *)&nSeconds );
#else
	tm * pTime = localtime( (const time_t *)&nSeconds );
#endif
	if (! CheckMaskPart( sParts[0], (pTime->tm_mon + 1) ) )
		return false;
	if (! CheckMaskPart( sParts[1], pTime->tm_mday ) )
		return false;
	if (!CheckMaskPart( sParts[2], pTime->tm_year + 1900) && !CheckMaskPart( sParts[2], pTime->tm_year - 100) )
		return false;
	if (! CheckMaskPart( sParts[3], pTime->tm_hour ) )
		return false;
	if (! CheckMaskPart( sParts[4], pTime->tm_min ) )
		return false;
	if (! CheckMaskPart( sParts[5], pTime->tm_sec ) )
		return false;

	return true;
}

CharString Time::time( dword nSeconds )
{
#if defined(_WIN32)
	return _ctime32( (__time32_t *)&nSeconds );
#else
	return ctime( (time_t *)&nSeconds );
#endif
}

bool Time::setTime( dword nSeconds )
{
#if defined(_WIN32)
	tm * gm = _gmtime32( (__time32_t *)&nSeconds );

	SYSTEMTIME time;
	time.wYear = gm->tm_year + 1900;
	time.wMonth = gm->tm_mon + 1;
	time.wDayOfWeek = 0;		// this is ignored by SetSystemTime()
	time.wDay = gm->tm_mday;
	time.wHour = gm->tm_hour;
	time.wMinute = gm->tm_min;
	time.wSecond = gm->tm_sec;
	time.wMilliseconds = 0; 

	return SetSystemTime( &time ) != 0;
#else
	return false;
#endif
}

//----------------------------------------------------------------------------
// EOF


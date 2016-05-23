/*
	Profile.h

	Code profiling system...

	Use the PROFILE_START and PROFILE_END macros so that profiling can be removed if wanted. You profile a section
	of code using PROFILE_START and use PROFILE_END to stop profiling. These CAN be nested, but make sure you 
	match every start() with a call to end().
	
	IMPORTANT NOTE: The string you pass to Profile::start() must be a static string, such as PROFILE_START( L"Profile" ); 
	this is because the address of the string is used in an internal hash.

	DO NOT pass a dynamic string such as PROFILE_START( CharString(L"Profile") );

	(c)2005 Palestar, Richard Lyle
*/

#ifndef DEBUG_PROFILE_H
#define DEBUG_PROFILE_H

#include "Standard/CharString.h"
#include "Standard/HashArray.h"
#include "Standard/Types.h"
#include "Standard/CriticalSection.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

#ifndef PROFILE_OFF
	#define PROFILE_FUNCTION()					ProfileFunction pf( __FUNCTION__ );
	#define PROFILE_START( name )				Profiler::start( name );
	#define PROFILE_END()						Profiler::end();
	#define PROFILE_MESSAGE( string )			Profiler::message( 0, string );
	#define PROFILE_LMESSAGE( line, string )	Profiler::message( line, string );
	#define PROFILE_STOP()						Profiler::stop();
#else
	#define PROFILE_FUNCTION()
	#define PROFILE_START( string )			
	#define PROFILE_END()					
	#define PROFILE_MESSAGE( string )
	#define PROFILE_LMESSAGE( line, string )
#endif

//-------------------------------------------------------------------------------

#pragma warning( disable: 4251 ) // needs to have dll-interface to be used by clients of class 'Profile'

class DLL Profiler
{
public:
	// Types
	struct Profile	
	{
		Profile();

		const char *		pName;			// name of the profile entry
		qword				nCPU;				
		dword				nHits;		
		qword				nAvCPU;			// CPU time used per second
		dword				nAvHits;		// number of hits per second
		int					nBytes;			// memory allocated
	};

	// begin profiling
	static void				start( const char * pName );	
	// end profiling the specified section
	static void				end();			
	// clears all sections for the calling thread, should be called before a thread exits
	static void				stop();		
	// add/update user message
	static void				message( dword nLine, const char * pMessage );
	// remove message line
	static void				removeMessage( dword nLine );
	// remove all messages
	static void				clearMessages();	

	// lock the profiler before calling any of the below functions
	static void				lock( bool bLock );	

	// none of the below functions are thread-safe, use the lock() function before calling these functions
	static int				threadCount();		// number of threads being profiles
	static dword			thread( int n );	// get thread_id n
	static int				profileCount( dword nThread );
	static Profile &		profile( dword nThread, int n );
	static int				messageCount();
	static const char *		message( int n );
	static qword			totalCPU();			// total number of CPU cycles per second
	static qword			peakCPU();
	static float			CPUused();			// returns the percent of cpu load (0.0f - 1.0f)

private:
	// Types
	struct Timer
	{
		const char *		pName;
		qword				nTime;
		int					nBytes;
	};

	typedef HashArray< dword, Profile >				ProfileHash;
	typedef HashArray< dword, ProfileHash >			ThreadHash;
	typedef HashArray< dword, CharString >			MessageHash;
	typedef Hash< dword, Array< Timer > >			StackHash;

	static CriticalSection	sm_Lock;
	static ThreadHash		sm_ThreadHash;
	static MessageHash		sm_MessageHash;
	static StackHash		sm_StackHash;			// timing stack for each thread, array contains the profile id
	static dword			sm_nLastUpdate;
	static qword			sm_nLastCPU;
	static qword			sm_nTotalCPU;
	static qword			sm_nPeakCPU;
	static float			sm_fLoad;

	static void				updateProfiles();
};

//----------------------------------------------------------------------------

inline Profiler::Profile::Profile() : pName( NULL ), nCPU( 0 ), 
	nHits( 0 ), nAvCPU( 0 ), nAvHits( 0 ), nBytes( 0 )
{}

//----------------------------------------------------------------------------

//! This object is used by the PROFILE_FUNCTION macro to profile a function, it constructs an object on the stack which ends
//! the profiling when the object is destroyed.
class DLL ProfileFunction
{
public:
	ProfileFunction( const char * a_pFunctionName )
	{
		Profiler::start( a_pFunctionName );
	}
	~ProfileFunction()
	{
		Profiler::end();
	}
};

//---------------------------------------------------------------------------------------------------


#endif

//----------------------------------------------------------------------------
// EOF

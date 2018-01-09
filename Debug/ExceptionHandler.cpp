/*
	ExceptionHandler.cpp

	Exception handler original code by Matt Pietrek, Microsoft Systems Journal, May 1997
	(c)2005 Palestar Inc, Richard Lyle
*/

#define DEBUG_DLL
#include "Log.h"
#include "ExceptionHandler.h"
#include "Error.h"

#include "File/Path.h"
#include "Standard/Thread.h"
#include "Standard/CharString.h"
#include "Standard/Settings.h"

#if defined(_WIN32) || defined(_XBOX)
#define _WIN32_WINNT    0x0400        
#include <windows.h>
#include <Excpt.h>
#include <dbghelp.h>
#endif

//---------------------------------------------------------------------------------------------------

static bool s_bVersionLoaded = false;
static dword s_nProgramVersion = 1000;
static dword s_nProgramBuild = 0;

//---------------------------------------------------------------------------------------------------

void LoadProgramVersion()
{
	if (! s_bVersionLoaded )
	{
		CharString sVersionFile( "." PATH_SEPERATOR "Version.ini" );

#if defined(_WIN32)
		char szExePath[ _MAX_PATH];
		if ( GetModuleFileName( NULL, szExePath, _MAX_PATH ))
		{
		   Path p( szExePath );
		   sVersionFile = p.directory() + "Version.ini";
		}
#endif

		Settings version( "Version", sVersionFile );
		s_nProgramVersion = version.get( "Version", s_nProgramVersion );
		s_nProgramBuild = version.get( "Build", s_nProgramBuild );
		s_bVersionLoaded = true;
	}
}

dword & GetProgramVersion()
{
	if (! s_bVersionLoaded )
		LoadProgramVersion();
	return s_nProgramVersion;
}

dword & GetProgramBuild()
{
	if (! s_bVersionLoaded )
		LoadProgramVersion();
	return s_nProgramBuild;
}

int GetStack( void * pContext, dword * pStack, int nMax )
{
#if defined(_WIN32) || defined(_XBOX)
	int nCount = 0;

	dword * pFrame = (dword *)((CONTEXT *)pContext)->Ebp;
	dword * pPrevFrame = NULL;

	try {
		DWORD pc = ((CONTEXT *)pContext)->Eip;

		while( nCount < nMax )
		{
			if ( pFrame <= pPrevFrame )
				break;
			if ( IsBadReadPtr(pFrame, sizeof(PVOID)*2) )
				break;
			if ( ((dword)pFrame & 3) != 0 )	
				break;

			pStack[ nCount++ ] = pc;

			pc = pFrame[1];
			pPrevFrame = pFrame;
			pFrame = (dword *)pFrame[0]; // proceed to next higher frame on stack

			if ( pc == 0 )
				break;
		}
	}
	catch( ... )
	{}

	// null terminate the stack..
	if ( nCount < nMax )
		pStack[ nCount ] = 0;

	return nCount;
#else
	return 0;
#endif
}

int GetStack( dword * pStack, int nMax )
{
#if defined(_WIN32) || defined(_XBOX)
   int nCount = 0;
   // due to release build optimizations, we don't want to miss any critical functions, so we don't skip
   // any entries in the stack.
#if defined(_DEBUG)
   int nSkip = 3;
#else
   int nSkip = 1;
#endif

   dword * pFrame = 0;
   dword * pPrevFrame = 0;

   __asm {
      mov [pFrame],ebp
   };

   try {
      while( nCount < nMax )
      {
         if ( pFrame <= pPrevFrame )
            break;
         if ( IsBadReadPtr(pFrame, sizeof(PVOID)*2) )
            break;
         if ( ((dword)pFrame & 3) != 0 )	
            break;

         dword pc = pFrame[1];         // get the return address from the frame
         pPrevFrame = pFrame;          // save off th previous frame
         pFrame = (dword *)pFrame[0];  // proceed to next higher frame on stack

         if ( nSkip <= 0 )
            pStack[ nCount++ ] = pc;
         else
            --nSkip;

         if ( pc == 0 )
            break;
      }
   }
   catch( ... )
   {}

   if ( nCount < nMax )
	   pStack[ nCount ] = 0;		

   return nCount;
#else
	return 0;
#endif
}

bool GetLogicalAddress( void * addr,char * szModule, dword len,dword & section, dword & offset )
{
#if defined(_WIN32) || defined(_XBOX)
    MEMORY_BASIC_INFORMATION mbi;

    if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
        return false;

    DWORD hMod = (DWORD)mbi.AllocationBase;

    if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
        return false;

    // Point to the DOS header in memory
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;
    if ( IsBadReadPtr( pDosHdr, sizeof(IMAGE_DOS_HEADER) ) )
		return false;
    
    // From the DOS header, find the NT (PE) header
    PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);
	if ( IsBadReadPtr( pNtHdr, sizeof(IMAGE_NT_HEADERS) ) )
		return false;

    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

    DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

    // Iterate through the section table, looking for the one that encompasses
    // the linear address.
    for ( unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++ )
    {
        DWORD sectionStart = pSection->VirtualAddress;
        DWORD sectionEnd = sectionStart
                    + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

        // Is the address in this section???
        if ( (rva >= sectionStart) && (rva <= sectionEnd) )
        {
            // Yes, address is in the section.  Calculate section and offset,
            // and store in the "section" & "offset" params, which were
            // passed by reference.
            section = i+1;
            offset = rva - sectionStart;
            return true;
        }
    }

    return false;   // Should never get here!
#else
    return false; 
#endif
}

void DumpStack( CharString & a_sDump, dword * pStack, int nMax )
{
#if defined(_WIN32) || defined(_XBOX)
	static char sLine[ 1024 ];
	static char szModule[MAX_PATH];

	a_sDump += "Address   Logical addr  Module\n";

	for(int i=0;i<nMax;++i)
	{
		DWORD section = 0, offset = 0;

		DWORD pc = pStack[ i ];
		if ( pc == 0 )
			break;

		szModule[ 0 ] = 0;
		if ( GetLogicalAddress( (void *)pc, szModule,sizeof(szModule),section,offset ) )
			sprintf( sLine,  "%08X  %04X:%08X %s\n", pc, section, offset, szModule );
		else
			sprintf( sLine, "%08X  ????:???????? %s\n", pc, szModule );

		a_sDump += sLine;
	}
#endif
}

void DumpContextStack( CharString & a_sDump, void * pContext )
{
#if defined(_WIN32) || defined(_XBOX)
	static char sLine[ 1024 ];
	static char szModule[MAX_PATH];

	a_sDump += "Address   Logical addr  Module\n";

	dword stack[ 128 ];
	int nStackSize = GetStack( pContext, stack, 128 );

	for(int i=0;i<nStackSize;i++)
	{
		DWORD section = 0, offset = 0;

		DWORD pc = stack[ i ];
		szModule[ 0 ] = 0;
		if ( GetLogicalAddress( (void *)pc, szModule,sizeof(szModule),section,offset ) )
			sprintf_s( sLine, sizeof(sLine), "%08X  %04X:%08X %s", pc, section, offset, szModule );
		else
			sprintf_s( sLine, sizeof(sLine), "%08X  ????:???????? %s", pc, szModule );

		a_sDump += sLine;
	}
#endif
}

#if defined(_WIN32) || defined(_XBOX)

static LPTSTR GetExceptionString( DWORD dwCode )
{
    #define EXCEPTION( x ) case EXCEPTION_##x: return #x;

    switch ( dwCode )
    {
        EXCEPTION( ACCESS_VIOLATION )
        EXCEPTION( DATATYPE_MISALIGNMENT )
        EXCEPTION( BREAKPOINT )
        EXCEPTION( SINGLE_STEP )
        EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
        EXCEPTION( FLT_DENORMAL_OPERAND )
        EXCEPTION( FLT_DIVIDE_BY_ZERO )
        EXCEPTION( FLT_INEXACT_RESULT )
        EXCEPTION( FLT_INVALID_OPERATION )
        EXCEPTION( FLT_OVERFLOW )
        EXCEPTION( FLT_STACK_CHECK )
        EXCEPTION( FLT_UNDERFLOW )
        EXCEPTION( INT_DIVIDE_BY_ZERO )
        EXCEPTION( INT_OVERFLOW )
        EXCEPTION( PRIV_INSTRUCTION )
        EXCEPTION( IN_PAGE_ERROR )
        EXCEPTION( ILLEGAL_INSTRUCTION )
        EXCEPTION( NONCONTINUABLE_EXCEPTION )
        EXCEPTION( STACK_OVERFLOW )
        EXCEPTION( INVALID_DISPOSITION )
        EXCEPTION( GUARD_PAGE )
        EXCEPTION( INVALID_HANDLE )
    }

    // If not one of the "known" exceptions, try to get the string
    // from NTDLL.DLL's message table.

    static char szBuffer[512] = "";

    FormatMessage(  FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                    GetModuleHandle( "NTDLL.DLSTR(" ),
                    dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

    return szBuffer;
}

#endif

static void dumpExceptionReport( void * pException )
{
#if defined(_WIN32) || defined(_XBOX)
	static char sLine[ 1024 ];
	PEXCEPTION_POINTERS pExceptionInfo = (PEXCEPTION_POINTERS)pException;

    // Start out with a banner
    LOG_ERROR( "EXCEPTION", "=====================================================" );

    PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

    // First print information about the type of fault
    sprintf_s( sLine, sizeof(sLine), "Exception code: %08X %s",pExceptionRecord->ExceptionCode,
                GetExceptionString(pExceptionRecord->ExceptionCode) );
	LOG_ERROR( "EXCEPTION",  sLine );

    // Now print information about where the fault occured
    char szFaultingModule[MAX_PATH] ="";
    DWORD section, offset;

	if ( GetLogicalAddress( pExceptionRecord->ExceptionAddress, szFaultingModule, sizeof( szFaultingModule ), section, offset ) )
		sprintf_s( sLine, sizeof(sLine), "Fault address:  %08X %02X:%08X %s", unsigned(pExceptionRecord->ExceptionAddress),section, offset, szFaultingModule );
	else
		sprintf_s( sLine, sizeof(sLine), "Fault address:  %08X ??:???????? %s", unsigned(pExceptionRecord->ExceptionAddress), szFaultingModule );
	LOG_ERROR( "EXCEPTION",  sLine );

	// Show the registers
    PCONTEXT pCtx = pExceptionInfo->ContextRecord;
    LOG_ERROR( "EXCEPTION",  "Registers:" );
    sprintf_s( sLine, sizeof(sLine), "EAX:%08X EBX:%08X ECX:%08X EDX:%08X ESI:%08X EDI:%08X",pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi );
	LOG_ERROR( "EXCEPTION",  sLine );
    sprintf_s( sLine, sizeof(sLine), "CS:EIP:%04X:%08X", pCtx->SegCs, pCtx->Eip );
	LOG_ERROR( "EXCEPTION",  sLine );
    sprintf_s( sLine, sizeof(sLine), "SS:ESP:%04X:%08X  EBP:%08X",pCtx->SegSs, pCtx->Esp, pCtx->Ebp );
	LOG_ERROR( "EXCEPTION",  sLine );
    sprintf_s( sLine, sizeof(sLine), "DS:%04X  ES:%04X  FS:%04X  GS:%04X",pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs );
	LOG_ERROR( "EXCEPTION",  sLine );
    sprintf_s( sLine, sizeof(sLine), "Flags:%08X", pCtx->EFlags );
	LOG_ERROR( "EXCEPTION",  sLine );

	CharString sDump;
	DumpContextStack( sDump, pCtx );

	LOG_ERROR( "EXCEPTION",  sDump );

    LOG_ERROR( "EXCEPTION",  "=====================================================" );
#endif
}

//---------------------------------------------------------------------------------------------------

#if defined(_WIN32) || defined(_XBOX)

// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                         CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                         CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                         CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
                                         );


bool WriteDump( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
   bool bSuccess = false;

   // Firstly see if dbghelp.dll is around and has the function we need look next to the EXE first, as the one in 
   // System32 might be old (e.g. Windows 2000)
   HMODULE hDll = NULL;
   
   hDll = ::LoadLibrary( "DBGHELP.DLL" );
   if (! hDll )
      return false;

   MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
   if (! pDump )
   {
      ::FreeLibrary( hDll );
      return false;
   }

   // Base the name of our dump file on the EXE's name.
   char szExePath[ _MAX_PATH];
   if (! GetModuleFileName( NULL, szExePath, _MAX_PATH ))
   {
      ::FreeLibrary( hDll );
      return false;
   }

   Path p( szExePath );

   // Generate the file name of the dump file with the correct path
   CharString sDumpFileName = p.directory();
   sDumpFileName += p.name();
   sDumpFileName += CharString().format("_%u_%u_%u", GetProgramVersion(), GetProgramBuild(), GetCurrentProcessId() );
   sDumpFileName += ".dmp";

   // create the file
   HANDLE hFile = ::CreateFile( sDumpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL, NULL );

   if (hFile!=INVALID_HANDLE_VALUE)
   {
      _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

      ExInfo.ThreadId = ::GetCurrentThreadId();
      ExInfo.ExceptionPointers = pExceptionInfo;
      ExInfo.ClientPointers = NULL;

      // Use MiniDumpNormal for minimal information & size.  Use MiniDumpWithDataSegs to get variable values.  But
      // the dump file will be much larger
      if ( pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, 
         MiniDumpNormal, &ExInfo, NULL, NULL ) )
	  {
         bSuccess = true;
	  }
      ::CloseHandle(hFile);
   }
   ::FreeLibrary( hDll );

   if ( bSuccess )
   {
	   LOG_ERROR( "EXCEPTION", "Exception dumped:" );
	   LOG_ERROR( "EXCEPTION", sDumpFileName );
   }

   return bSuccess;
}

#endif

//---------------------------------------------------------------------------------------------------

int ProcessException( void * pException )
{
#if defined(_WIN32) || defined(_XBOX)
	static char sLine[ 1024 ];
	int i;

	Thread::threadLock( true );

	try {
		if (! WriteDump( (_EXCEPTION_POINTERS *)pException ) )
			LOG_ERROR( "EXCEPTION", "Failed to dump exception!" );
	}
	catch(...)
	{
		LOG_ERROR( "EXCEPTION", "Exception occurred while dumping exception!" );
	}

	dumpExceptionReport( pException );

	// additionally, dump the a stack trace for each thread, except this thread
	for(i=0;i<Thread::threadCount();i++)
	{
		Thread * pThread = Thread::thread( i );
		if ( pThread->inThread() )
			continue;		// skip our thread and suspended threads..

		sprintf_s( sLine, sizeof(sLine), "Thread %u Stack:", pThread->id() );
		LOG_ERROR( "EXCEPTION", sLine );

		DumpThreadStack( pThread->handle() );
		LOG_ERROR( "EXCEPTION", "=====================================================" );
	}
	Thread::threadLock( false );

#endif

	//ProcessError( "Exception has occured!" );
	return 1; //EXCEPTION_EXECUTE_HANDLER;
}

int HandleException( int (* pFunction)( void * pArg ), void * pArg )
{
#if defined(_WIN32) || defined(_XBOX)

#ifndef _DEBUG
	__try {
#endif
		return pFunction( pArg );
#ifndef _DEBUG
	}
	__except( ProcessException( GetExceptionInformation() ) )
	{}
	return -1;
#endif

#else
	return pFunction( pArg );
#endif
}

void DumpThreadStack( void * pThread )
{
#if defined(_WIN32) || defined(_XBOX)
	if ( pThread == NULL )
		pThread = GetCurrentThread();

	bool bResumeThread = false;
	if ( pThread != GetCurrentThread() )
	{
		if ( SuspendThread( (HANDLE)pThread ) == 0 )
			bResumeThread = true;		
	}

	CONTEXT context;
	memset( &context, 0, sizeof(context) );
	context.ContextFlags = CONTEXT_FULL;

	CharString sDump;
	if ( GetThreadContext( (HANDLE)pThread, &context ) )
		DumpContextStack( sDump, &context );

	LOG_ERROR( "EXCEPTION", sDump );

	if ( bResumeThread )
		ResumeThread( (HANDLE)pThread );
#endif
}

//----------------------------------------------------------------------------
//EOF

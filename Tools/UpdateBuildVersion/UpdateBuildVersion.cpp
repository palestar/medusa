/**
	@file UpdateVersionBuild.cpp

	(c)2012 Palestar Inc
	@author Richard Lyle @date 9/27/2012 8:00:28 PM
*/

#include "Standard/Settings.h"
#include "Debug/Log.h"

#include <windows.h>
#include <string>

//---------------------------------------------------------------------------------------------------

int RunCommand( const std::string & a_sCommand, std::string * a_pOutput /*= NULL*/ )
{
   std::string sCommand( a_sCommand );

   SECURITY_ATTRIBUTES sa;
   memset( &sa, 0, sizeof(sa) );
   sa.nLength = sizeof(SECURITY_ATTRIBUTES);
   sa.bInheritHandle = TRUE;
   sa.lpSecurityDescriptor = NULL;

   // Create the child sOutput pipe...
   HANDLE hOutputWrite, hOutputRead, hOutputReadTmp;
   if (! CreatePipe( &hOutputReadTmp, &hOutputWrite, &sa, 0 ) )
      LOG_ERROR( "UpdateBuildVersion", "Failed to create sOutput pipe!" );

   // create a duplicate handle of the sOutput write handle for the std error write handle. This is necessary
   // in case the child application closes one of it's std sOutput handles.
   HANDLE hErrorWrite;
   if (! DuplicateHandle( GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hErrorWrite, 0, TRUE,DUPLICATE_SAME_ACCESS ) )      
      LOG_ERROR( "UpdateBuildVersion", "Failed to duplicate handle!" );
   
   // Create the child input pipe
   HANDLE hInputRead, hInputWriteTmp, hInputWrite;
   if (! CreatePipe( &hInputRead, &hInputWriteTmp, &sa, 0 ) )
      LOG_ERROR( "UpdateBuildVersion", "Failed to create input pipe!" );

   if (! DuplicateHandle( GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(), &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS ) )
      LOG_ERROR( "UpdateBuildVersion", "Failed to duplicate handle!" );
   if (! DuplicateHandle( GetCurrentProcess(), hInputWriteTmp, GetCurrentProcess(), &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS ) )
      LOG_ERROR( "UpdateBuildVersion", "Failed to duplicate handle!" );

   // close inheritable copies of the handles you do not want to be inherited.
   CloseHandle( hOutputReadTmp );
   CloseHandle( hInputWriteTmp );

   PROCESS_INFORMATION pi;
   STARTUPINFO si;

   memset( &si, 0, sizeof(si) );
   si.cb = sizeof(si);
   si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
   si.hStdOutput = hOutputWrite;
   si.hStdInput = hInputRead;
   si.hStdError = hErrorWrite;
   si.wShowWindow = SW_HIDE;

   if ( CreateProcess( NULL, &sCommand[0], NULL, NULL, TRUE, 
      CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi ) )
   {
      CloseHandle( pi.hThread );

      // Close pipe handles (do not continue to modify the parent). 
      // You need to make sure that no handles to the write end of the 
      // sOutput pipe are maintained in this process or else the pipe will
      // not close when the child process exits and the ReadFile will hang.
      CloseHandle( hOutputWrite);
      CloseHandle( hInputRead );
      CloseHandle( hErrorWrite );

      char sBuffer[ 1024 ];
      while( true )
      {
         DWORD nBytesRead = 0;
         if ( !ReadFile( hOutputRead, sBuffer, sizeof(sBuffer)-1, &nBytesRead, NULL ) || nBytesRead == 0 )
            break;

         sBuffer[ nBytesRead ] = 0;
         if ( a_pOutput )
            (*a_pOutput) += sBuffer;
      }

      CloseHandle( hOutputRead );
      CloseHandle( hInputWrite );

      if ( WaitForSingleObject( pi.hProcess, 10 * 1000 ) != WAIT_OBJECT_0 )
         LOG_ERROR( "UpdateBuildVersion", "Failed to wait for process to exit!" );

      DWORD nExitCode;
      if (! GetExitCodeProcess( pi.hProcess, &nExitCode ) )
         LOG_ERROR( "UpdateBuildVersion", "Failed to process exit code!" );

      CloseHandle( pi.hProcess );

      return static_cast<int>( nExitCode );
   }

  CloseHandle( hOutputWrite);
  CloseHandle( hInputRead );
  CloseHandle( hErrorWrite );
  CloseHandle( hOutputRead );
  CloseHandle( hInputWrite );

  return -1;
}

int main(int argc,char **argv)
{
	StandardOutReactor r;

	std::string sDirectory( ".\\" );
	std::string sVersionFile( ".\\Version.ini" );
	if ( argc > 1 )
	{
		sDirectory = argv[1];
		for(size_t i=0;i<sDirectory.length();++i)
			if ( sDirectory[i] == '/' )
				sDirectory[i] = '\\';
		if ( sDirectory.length() > 0 && sDirectory[ sDirectory.length() - 1 ] != '/' )
			sDirectory += '\\';		// make sure it ends with a slash..

		if ( argc > 2 )
			sVersionFile = argv[2];
		else
			sVersionFile = sDirectory + "Version.ini";
	}

	std::string sOutput;
	if ( RunCommand( "svn info " + sDirectory, &sOutput ) != 0 )
	{
		printf( "ERROR: Failed to run svn info command. (%s)", sOutput.c_str() );
		return 1;
	}

	const std::string REVISION_ID( "Revision:" );

	size_t nRevisionIndex = sOutput.find( REVISION_ID );
	if ( nRevisionIndex == std::string::npos )
	{
		printf( "ERROR: Failed to find revision number in svn info. (%s)", sOutput.c_str() );
		return 1;
	}

	dword nBuild = strtoul( sOutput.c_str() + nRevisionIndex + REVISION_ID.size(), NULL, 10 ) + 1;		// add 1 so it matches the commit revision!
	printf( "Setting Version.ini to build %u...\n", nBuild );

	Settings version( "Version", sVersionFile.c_str() );
	version.put( "Build", nBuild );

	return 0;
}

//---------------------------------------------------------------------------------------------------
//EOF

/**
	@file Error.h
	@brief Error handler

	(c)2006 Palestar Inc
	@author Richard Lyle @date 7/19/2006 8:28:35 PM
*/

#ifndef ERROR_H
#define ERROR_H

#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

enum ErrorMode
{
	EM_DISABLED,			// all errors are ignored
	EM_DEFAULT,				// errors are logged, then the application is aborted()
	EM_DIALOG,				// errors are logged then the user is given the choice to exit, retry (Debug), or ignore the error
};

//---------------------------------------------------------------------------------------------------

DLL void						SetProcessErrorMode( ErrorMode nMode );
DLL void						ProcessError( const char * pMessage, const char * a_pFile, int a_nLine );

#undef ERROR_ON
#define ERROR_ON( EXP )			if ( EXP ) ProcessError( #EXP, __FILE__,__LINE__ )

#endif

//---------------------------------------------------------------------------------------------------
//EOF

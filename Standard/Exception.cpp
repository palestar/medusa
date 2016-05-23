/**
	@file Exception.cpp

	(c)2006 Palestar Inc
	@author Richard Lyle @date 7/18/2006 11:56:03 PM
*/

#include "Exception.h"
#include "Debug/Error.h"

//---------------------------------------------------------------------------------------------------

ExceptionBase::ExceptionBase() : m_pMessage( "?" ), m_pFile( 0 ), m_nLine( -1 )
{}

ExceptionBase::ExceptionBase( const char * pMsg, const char * pFile, int nLine )
		: m_pMessage( pMsg ), m_pFile( pFile ), m_nLine( nLine )
{}

ExceptionBase::ExceptionBase( const char * pMsg )
		: m_pMessage( pMsg ), m_pFile( 0 ), m_nLine( -1 )
{}


//---------------------------------------------------------------------------------------------------
//EOF

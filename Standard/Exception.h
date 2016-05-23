/*
	ExceptionBase.h

	Base exception class
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <typeinfo>
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL ExceptionBase
{
public:
	// Construction
	ExceptionBase();
	ExceptionBase( const char * pMsg );
	ExceptionBase( const char * pMsg, const char * pFile, int nLine );

	// Accessors
	const char *	message() const;
	const char *	file() const;
	int				line() const;

private:
	// Data
	const char *	m_pMessage;
	const char *	m_pFile;
	int				m_nLine;
};

//----------------------------------------------------------------------------

inline const char *	ExceptionBase::message() const
{
	return m_pMessage;
}

inline const char *	ExceptionBase::file() const
{
	return m_pFile;
}

inline int ExceptionBase::line() const
{
	return m_nLine;
}

//----------------------------------------------------------------------------

// Exception macros
#define THROW_EXCEPTION_ERROR()		throw ExceptionBase( "Unspecified Error", __FILE__, __LINE__ )
#define THROW_EXCEPTION( MSG )		throw ExceptionBase( MSG, __FILE__, __LINE__ )
// This macro should be used to conditionally throw a macro, do not define this one out..
#define THROW_EXCEPTION_ON( COND )	if ( COND ) throw ExceptionBase( #COND, __FILE__, __LINE__ )

#define DECLARE_EXCEPTION( NAME )	class NAME : public Exception<NAME> {}

//----------------------------------------------------------------------------

template<typename T>
class Exception : public ExceptionBase
{
public:
	Exception() : ExceptionBase( typeid(T).name() )
	{}
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF

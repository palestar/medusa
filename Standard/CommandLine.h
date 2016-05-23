/*
	CommandLine.h

	This class is used to parse a single string command line into seperate arguments. 

	NOTE: This class will also handle arguments with spaces if their are contained with 
	either double or single quotes.

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include "Standard/CharString.h"
#include "Standard/Array.h"
#include "Standard/StringHelpers.h"

#include <stdlib.h>

//----------------------------------------------------------------------------

class CommandLine
{
public:
	// Construction
	CommandLine();
	CommandLine( const char * pCommandLine );
	CommandLine( const char * pCommandLine, bool keepQuotes );

	// Accessors
	bool			isValid( int n ) const;						// returns true if argument index is valid
	int				argumentCount() const;
	const char *	argument( int n ) const;
	int				argumentInt( int n ) const;
	dword			argumentDWORD( int n ) const;
	CharString		restring( int n ) const;					// Creates a single string beginning at argument n to the last argument
	
	int				find( const char * pArg, int nArgs = 0 );	// find an argument (case-insenstive), returns -1 if not found

	// Mutators
	void			initialize( const char * pCommandLine );
	void			initialize( const char * pCommandLine, bool keepQuotes );
	void			append( const CommandLine & append );

	CommandLine &	operator=( const CommandLine & copy );
	CommandLine &	operator=( const char * pCommandLine );

private:
	// Data
	Array< CharString >	
					m_Arguments;
};

//----------------------------------------------------------------------------

inline CommandLine::CommandLine()
{}

inline CommandLine::CommandLine( const char * pCommandLine)
{
	initialize( pCommandLine );
}

inline CommandLine::CommandLine( const char * pCommandLine, bool keepQuotes )
{
	initialize( pCommandLine, keepQuotes );
}

inline bool CommandLine::isValid( int n ) const
{
	return m_Arguments.isValid( n );
}

inline int CommandLine::argumentCount() const
{
	return m_Arguments.size();
}

inline const char * CommandLine::argument( int n ) const
{
	return m_Arguments[ n ];
}

inline int CommandLine::argumentInt( int n ) const
{
	return atoi( m_Arguments[ n ] );
}

inline dword CommandLine::argumentDWORD( int n ) const
{
	return strtoul( m_Arguments[ n ], NULL, 10 );
}

inline CharString CommandLine::restring( int n ) const
{
	CharString string;
	for(int i=n;i<m_Arguments.size();i++)
	{
		string += m_Arguments[ i ];
		if ( (i + 1) < m_Arguments.size() )
			string += " ";
	}

	return string;
}

inline int CommandLine::find( const char * pArg, int nArgs /*= 0*/ )
{
	// we subtract the number of arguments expected from the total count, this means
	// we should never find an Argument thats doesn't have enough expected following symbols
	for(int i=0;i<(m_Arguments.size() - nArgs);i++)
		if ( stricmp<char>( pArg, (const char *)m_Arguments[i] ) == 0 )
			return i;
	return -1;
}

//----------------------------------------------------------------------------

inline void CommandLine::initialize( const char * pCommandLine )
{
	initialize( pCommandLine, false );
}

inline void CommandLine::initialize( const char * pCommandLine, bool keepQuotes )
{
	// flag if we're inside quotes or not
	bool	inQuote = false;
	// keep track of the quote character, so we can embed quotes within each other
	char	quoteChar = 0;

	// current argument
	CharString	argument;

	int	length = (int)strlen( pCommandLine );
	for(int i=0;i<length;i++)
	{
		char c = pCommandLine[ i ];

		if ( c == ' ' && !inQuote )
		{
			// if argument isn't zero length then push the argument
			if ( argument.length() > 0 )
			{
				m_Arguments.push( argument );
				argument = "";
			}
		}
		else
		{
			if ( inQuote && c == quoteChar )
			{
				inQuote = false;
				if ( keepQuotes )
					argument += c;
			}
			else if ( !inQuote && (c == '\'' || c == '\"') )
			{
				inQuote = true;
				quoteChar = c;

				if ( keepQuotes )
					argument += c;
			}
			else
				argument += c;
		}
	}

	// push last remaining argument if any
	if ( argument.length() > 0 )
	{
		m_Arguments.push( argument );
		argument = "";
	}
}

inline void CommandLine::append( const CommandLine & append )
{
	for(int i=0;i<append.m_Arguments.size();i++)
		m_Arguments.push( append.m_Arguments[i] );
}

inline CommandLine & CommandLine::operator=( const CommandLine & copy )
{
	m_Arguments = copy.m_Arguments;
	return *this;
}

inline CommandLine & CommandLine::operator=( const char * pCommandLine )
{
	initialize( pCommandLine, false );
	return *this;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF

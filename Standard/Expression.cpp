/*
	Expression.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Expression.h"
#include <ctype.h>

//----------------------------------------------------------------------------

struct TokenLookup
{
	Expression::TokenType	nType;
	char					cBegin;
	char					cEnd;
};

static TokenLookup TOKENS[] =
{
	{	Expression::TOKEN_STRING,		'\"',	'\"'	},
	{	Expression::TOKEN_STRING,		'\'',	'\''	},
	{	Expression::TOKEN_ELEMENT,		'[',	']'		},
	{	Expression::TOKEN_POINTER,		'-',	'>'		},
	{	Expression::TOKEN_FUNCTION,		'(',	')'		},

	{	Expression::TOKEN_MEMBER,		'.',	0		},
	{	Expression::TOKEN_ADDITION,		'+',	0		},
	{	Expression::TOKEN_SUBTRACTION,	'-',	0		},
	{	Expression::TOKEN_MULTIPLY,		'*',	0		},
	{	Expression::TOKEN_DIVIDE,		'/',	0		},
	{	Expression::TOKEN_ASSIGNMENT,	'=',	0		},
};

//----------------------------------------------------------------------------

inline bool IsSymbol( int c )
{
	return isalnum( c ) || c == '_';
}

static const char * CopyToken( const char * pParse, char cEnd, Expression::Token & token )
{
	while( *pParse && *pParse != cEnd )
		token.sToken += *pParse++;
	if ( *pParse == cEnd )
		pParse++;
	return pParse;
}

//----------------------------------------------------------------------------

Expression::Expression() : m_nTop( 0 )
{}

Expression::Expression( const char * pParse ) : m_nTop( 0 )
{
	parse( pParse );
}

Expression::Expression( const Expression & copy ) : m_nTop( copy.m_nTop ), m_Tokens( copy.m_Tokens )
{}

//----------------------------------------------------------------------------

CharString Expression::restring( int n /*= 0*/ ) const
{
	CharString s;
	for(;n < (int)m_Tokens.size();n++)
	{
		const Token & token = m_Tokens[ n ];
		if ( token.nType == TOKEN_SYMBOL )
		{
			s += m_Tokens[ n ].sToken;
			continue;
		}

		for(unsigned int i=0;i<sizeof(TOKENS)/sizeof(TOKENS[0]);i++)
		{
			TokenLookup & lookup = TOKENS[i];
			if ( lookup.nType == token.nType )
			{
				if ( lookup.cBegin != 0 )
					s += lookup.cBegin;
				s += token.sToken;
				if ( lookup.cEnd != 0 )
					s += lookup.cEnd;

				break;
			}
		}
	}

	return s;
}

//----------------------------------------------------------------------------

void Expression::setTop( int nTop )
{
	m_nTop = nTop;
}

int Expression::parse( const char * pParse )
{
	release();

	const char * pStart = pParse;
	const char * pEnd = pStart + strlen( pParse );
	while( *pParse && pParse < pEnd )
	{
		// skip whitespaces
		if( *pParse && isspace( *pParse ) )
		{ 
			pParse++;
			continue;
		}
		
		Token token;
		token.nType = TOKEN_ERROR;

		char ch = *pParse++;
		if ( IsSymbol( ch ) )
		{
			token.nType = TOKEN_SYMBOL;
			token.sToken += ch;

			while( IsSymbol( *pParse ) )
				token.sToken += *pParse++;
		}
		else 
		{
			for(unsigned int i=0;i<sizeof(TOKENS)/sizeof(TOKENS[0]);i++)
			{
				if ( TOKENS[i].cBegin == ch )
				{
					token.nType = TOKENS[i].nType;
					if ( TOKENS[i].cEnd != 0 )
						pParse = CopyToken( pParse, TOKENS[i].cEnd, token );
					break;
				}
			}
		}

		// push the new token
		m_Tokens.push( token );
	}

	return (int)m_Tokens.size();
}

void Expression::remove( int n )
{
	if ( m_Tokens.isValid( n ) )
		m_Tokens.remove( n );
}

void Expression::release()
{
	m_Tokens.release();
	m_nTop = 0;
}

void Expression::first()
{
	m_nTop = 0;
}

bool Expression::next()
{
	return ++m_nTop < ((int)m_Tokens.size());
}

bool Expression::pop()
{
	remove( m_nTop );
	return m_nTop < ((int)m_Tokens.size());
}

//----------------------------------------------------------------------------
//EOF

/*
	Expression.h

	This class is used to parse a language expression, such as "x = y". Currently, this class is fairly limited 
	because it's only needed by the Property System.

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "Array.h"
#include "String.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Expression
{
public:
	// Types
	enum TokenType {

		TOKEN_ERROR = -1,			// error

		TOKEN_SYMBOL,				// Symbol
		TOKEN_STRING,				// " or '
		TOKEN_ELEMENT,				// [ ]
		TOKEN_MEMBER,				// .
		TOKEN_POINTER,				// ->
		TOKEN_ASSIGNMENT,			// =
		TOKEN_FUNCTION,				// ()

		TOKEN_ADDITION,				// +
		TOKEN_SUBTRACTION,			// -
		TOKEN_MULTIPLY,				// *
		TOKEN_DIVIDE,				// /
	};

	struct Token
	{
		TokenType		nType;
		CharString		sToken;

		Token & operator=( const Token & copy )
		{
			nType = copy.nType;
			sToken = copy.sToken;
			return *this;
		}
	};


	// Construction
	Expression();
	Expression( const char * pParse );
	Expression( const Expression & copy );

	// Accessors
	int					top() const;					// current token index

	int					remain() const;					// number of tokens remaining, 0 if none
	TokenType			type() const;					// top token type
	const char *		string() const;					// top token string

	int					tokenCount() const;
	const Token &		token( int n ) const;

	CharString			restring( int n = 0 ) const;	// build a string from the current tokens, starting at n

	// Mutators
	void				setTop( int nTop );

	int					parse( const char * pParse );	// parse a string, returns the number of tokens parsed
	void				remove( int n );				// remove token n
	void				release();						// remove all tokens

	void				first();						// goto the first token
	bool				next();							// next token, return false if none
	bool				pop();							// pop the curren token

private:
	// Data
	int					m_nTop;
	Array< Token >		m_Tokens;
};

//----------------------------------------------------------------------------

inline int Expression::remain() const
{
	if ( m_nTop >= 0 && m_nTop < (int)m_Tokens.size() )
		return (int)m_Tokens.size() - m_nTop;
	return 0;
}

inline Expression::TokenType Expression::type() const
{
	if ( m_nTop >= 0 && m_nTop < (int)m_Tokens.size() )
		return m_Tokens[ m_nTop ].nType;

	return TOKEN_ERROR;
}

inline const char * Expression::string() const
{
	if ( m_nTop >= 0 && m_nTop < (int)m_Tokens.size() )
		return m_Tokens[ m_nTop ].sToken;
	return NULL;
}

inline int Expression::tokenCount() const
{
	return (int)m_Tokens.size();
}

inline const Expression::Token & Expression::token( int n ) const
{
	return m_Tokens[ n ];
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF

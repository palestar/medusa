/*
	RegExpM.h

	Wrapper class for regular expression code
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef REGEXPM_H
#define REGEXPM_H

//----------------------------------------------------------------------------

#include "Standard/Types.h"
#include "Standard/String.h"
#include "Standard/CharString.h"

#include "MedusaDll.h"
	
//----------------------------------------------------------------------------

struct tag_regexp;
typedef struct tag_regexp regexp;
struct regmatch;

class DLL RegExpM
{
public:
	// Construction
	RegExpM();
	RegExpM( const wchar * pExp );
	RegExpM( const char * pExp );
	~RegExpM();

	// Mutators
	RegExpM *	regComp( const char * pExp );
	RegExpM *	regComp( const wchar * pExp );
	int			regFind( const char * pStr );
	int			regFind( const wchar * pStr );
	int			getFindLen();

	String		getReplaceString( const wchar * sReplaceExp );
	void		release();

	static int	regSearchReplace( String & s, 
					const wchar * pSearchExp, 
					const wchar * pReplaceExp );
	static int	regSearchReplace( CharString & s, 
					const char * pSearchExp, 
					const char * pReplaceExp );
private:
	regexp *	m_pCompiled;
	String		m_sFindText;
	String		m_sFoundText;
	int			m_nMatches;
	regmatch *	m_pMatches;
};

//----------------------------------------------------------------------------

#endif 

//----------------------------------------------------------------------------
//EOF

/*
	Path.h

	This object is used to break up or construct a file path
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PATH_H
#define PATH_H

#include "Standard/String.h"
#include "MedusaDll.h"



//----------------------------------------------------------------------------

class DLL Path
{
public:
	// Construction
	Path();
	Path( const CharString & path );
	Path( const char * pPath );

	// Accessors
	CharString		path() const;				// c:\example\file.ext
	CharString 		directory() const;			// c:\example
	CharString 		file() const;				// file.ext
	CharString 		name() const;				// file
	CharString		extension() const;			// .ext

	// Mutators
	void			setPath( const char * pPath );
	void			setDirectory( const char * pDir );
	void			setFile( const char * pFile );
	void			setName( const char * pName );
	void			setExtension( const char * pExt );

private:
	CharString		m_Directory;
	CharString		m_Name;
	CharString		m_Extension;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

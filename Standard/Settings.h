/*
	Settings.h

	This class is used to read/write settings to a ini file. If you want the file to save/load in the current directory
	you must put a ./ before your filename. (i.e. ./settings.ini ), otherwise if you just put the filename it will
	be stored in the windows directory.

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include "Standard/CharString.h"
#include "Standard/Types.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

class DLL Settings
{
public:
	// Types
	enum {
		MAX_SIZE = 512,			// largest string that can be handled
	};

	// Construction
	Settings();
	Settings( const char * pSection, const char * pFile = NULL );		

	// Accessors
	const char *		file() const;
	const char *		section() const;

	// Mutators
	void				setFile( const char * pFile );
	void				setSection( const char * pSection );

	dword				get( const char * pKey, dword defaultValue );
	CharString			get( const char * pKey, const char * pDefaultValue );
	bool				put( const char * pKey, dword value );
	bool				put( const char * pKey, const char * pValue );

private:
	CharString			m_sFile;
	CharString			m_sSection;
};

//----------------------------------------------------------------------------

inline const char * Settings::file() const
{
	return m_sFile;
}

inline const char * Settings::section() const
{
	return m_sSection;
}

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

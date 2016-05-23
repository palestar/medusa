/*
	Registry.h

	This object provides access to the windows registry for storing program configuration values
	(c)2005 PaleStar Development, Richard Lyle
*/

#ifndef REGISTRY_H
#define REGISTRY_H

#include "Standard/CharString.h"
#include "Standard/Types.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

class DLL Registry
{
public:
	// Types
	enum  {
		MAX_STRING = 1024,		// maximum size of a string returned by get()
	};
	enum RootKey {
		USER,					// user specific information
		LOCAL,					// machine specific information
	};

	// Construction
	Registry( RootKey root, const char * pApplication );	
	virtual	~Registry();

	// Accessors
	RootKey				root() const;
	const char *		application() const;

	// Mutators
	dword				get( const char * pKey, dword nDefault );					// get a dword from the registry
	CharString			get( const char * pKey, const char * pDefault );			// get a string from the registry
	bool				put( const char * pKey, dword nValue );
	bool				put( const char * pKey, const char * pValue );

private:
	bool				m_bUser;
	CharString			m_sApplication;
};

//----------------------------------------------------------------------------

inline Registry::RootKey Registry::root() const				
{ 
	return m_bUser ? USER : LOCAL; 
}

inline const char * Registry::application() const
{ 
	return m_sApplication; 
}

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF


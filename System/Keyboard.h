/*
	Keyboard.h

	Keyboard interface
	(c)2005 Palestar, Richard Lyle
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "Standard/Types.h"
#include "Standard/Hash.h"
#include "Standard/CharString.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

#pragma warning(disable:4251)	// needs to have dll-interface to be used by clients of class

class DLL Keyboard
{
public:
	// Static
	static bool				shiftDown();
	static bool				ctrlDown();
	static bool				altDown();

	static void				enableKeyMap();									// initialize the default keymap
	static void				disableKeyMap();								// disable the key remapping
	static void				resetKeyMap();									// reset keymapping to default
	static bool				loadKeyMap( const char * pFileName );			// load the keymapping from a file
	static bool				saveKeyMap( const char * pFileName );			// save the keymapping to a file

	static dword			map( dword realKey );							// map a real keycode to the mapped version
	static dword			unmap( dword key );								// reverse map a key
	static void				remap( dword oldKey, dword newKey );			// remap a key to return a different value

	static const char *		keyTextRaw( dword key );						// get text description of key, use only the lower 8 bits
	static CharString		keyText( dword key );							// get text description of key
	static dword			textKey( const CharString & sKey );				// convert test description to key
	static CharString		keyShortText( dword key );						// get short text description of key

private:
	// Types
	typedef Hash< dword, dword >		KeyHash;
	typedef KeyHash::Iterator			KeyHashIt;

	// Data
	static bool				s_KeyMapEnabled;
	static KeyHash			s_KeyMap;
	static KeyHash			s_KeyUnmap;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

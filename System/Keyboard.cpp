/*
	Keyboard.cpp

	Keyboard interface
	(c)2005 Palestar, Richard Lyle
*/

#define SYSTEM_DLL
#include "File/FileDisk.h"
#include "File/Stream.h"
#include "System/Keyboard.h"
#include "System/Messages.h"

#if defined(_WIN32)
#include <windows.h>
#endif

//----------------------------------------------------------------------------

bool					Keyboard::s_KeyMapEnabled = false;
Keyboard::KeyHash		Keyboard::s_KeyMap;
Keyboard::KeyHash		Keyboard::s_KeyUnmap;

//----------------------------------------------------------------------------

bool Keyboard::shiftDown()
{
#if defined(_WIN32)
	return( GetAsyncKeyState( VK_SHIFT ) < 0 );
#else
	return false;
#endif
}

bool Keyboard::ctrlDown()
{
#if defined(_WIN32)
	return( GetAsyncKeyState( VK_CONTROL ) < 0 );
#else
	return false;
#endif
}

bool Keyboard::altDown()
{
#if defined(_WIN32)
	return( GetAsyncKeyState( VK_MENU ) < 0 );
#else
	return false;
#endif
}

//----------------------------------------------------------------------------

void Keyboard::enableKeyMap()
{
	s_KeyMapEnabled = true;
}

void Keyboard::disableKeyMap()
{
	s_KeyMapEnabled = false;
}

void Keyboard::resetKeyMap()
{
	s_KeyMap.release();
	s_KeyUnmap.release();
}

const int VERSION_042402 = 42402;
const int VERSION_20040204 = 20040204;

bool Keyboard::loadKeyMap( const char * pFileName )
{
	try {
		InStream input( new FileDisk( pFileName ) );
		
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_20040204:
			{
				input >> s_KeyMap;
				input >> s_KeyUnmap;
			}
			break;
		case VERSION_042402:
			{
				unsigned int i = 0;
				for(;i<s_KeyMap.treeCount();i++)
					input >> s_KeyMap.tree( i );
				for(i=0;i<s_KeyUnmap.treeCount();i++)
					input >> s_KeyUnmap.tree( i );
			}
			break;
		default:
			return false;		// invalid version!
		}

		return true;
	}
	catch( File::FileError )
	{}

	return false;
}

bool Keyboard::saveKeyMap( const char * pFileName )
{
	try {
		OutStream output( new FileDisk( pFileName, FileDisk::WRITE) );

		output << VERSION_20040204;
		output << s_KeyMap;
		output << s_KeyUnmap;

		return true;
	}
	catch( File::FileError )
	{}

	return false;
}

dword Keyboard::map( dword key )
{
	if ( s_KeyMapEnabled )
	{
		KeyHashIt find = s_KeyMap.find( key );
		if ( find.valid() )
			return *find;

	}
	return key;
}

dword Keyboard::unmap( dword key )
{
	if ( s_KeyMapEnabled )
	{
		KeyHashIt find = s_KeyUnmap.find( key );
		if ( find.valid() )
			return *find;
	}

	return key;
}

void Keyboard::remap( dword oldKey, dword newKey )
{
	if ( oldKey == newKey )
	{
		s_KeyMap.remove( oldKey );
		s_KeyUnmap.remove( newKey );
	}
	else
	{
		KeyHashIt find = s_KeyUnmap.find( newKey );
		if ( find.valid() )
		{
			// another key was already mapped to this key, remove the mapping
			s_KeyMap.remove( *find );
			s_KeyUnmap.remove( newKey );
		}

		s_KeyMap[ oldKey ] = newKey;
		s_KeyUnmap[ newKey ] = oldKey;
	}
}

const char * Keyboard::keyTextRaw( dword key )
{
	static const char * KEY_TEXT[ 256 ] =	// translate a keycode into a readable text string
	{
        "",				// 0x00
		"0x01",			// 0x01
		"0x02",			// 0x02
		"0x03",			// 0x03
		"0x04",			// 0x04
		"0x05",			// 0x05
		"0x06",			// 0x06
		"0x07",			// 0x07
		"BACK",			// 0x08
		"TAB",			// 0x09
		"0x0a",			// 0x0a
		"0x0b",			// 0x0b
		"CLR",			// 0x0c
		"ENTER",		// 0x0d,
		"0x0e",			// 0x0e
		"0x0f",			// 0x0f
		"SHIFT",		// 0x10
		"CTRL",			// 0x11
		"ALT",			// 0x12
		"PAUSE",		// 0x13
		"CAPS",			// 0x14
		"KANA",			// 0x15
		"0x16",			// 0x16
		"JUNJA",		// 0x17
		"FINAL",		// 0x18
		"HANJA",		// 0x19
		"0x1a",			// 0X1a
		"0x1b",			// 0x1b
		"0x1c",			// 0x1c
		"0x1d",			// 0x1d
		"0x1e",			// 0x1e
		"0x1f",			// 0x1f
		"SPACE",		// 0x20
		"PGUP",			// 0x21
		"PGDN",			// 0x22
		"END",			// 0x23
		"HOME",			// 0x24
		"LEFT",			// 0x25
		"UP",			// 0x26
		"RIGHT",		// 0x27
		"DOWN",			// 0x28
		"SEL",			// 0x29
		"PRT",			// 0x2a
		"EXE",			// 0x2b
		"0x2c",			// 0x2c
		"INS",			// 0x2d
		"DEL",			// 0x2e
		"0x2f",			// 0x2f
		"0",			// 0X30,
		"1",		
		"2",		
		"3",		
		"4",		
		"5",		
		"6",		
		"7",		
		"8",		
		"9",		
		"0x3a",			// 0x3a
		"0x3b",			// 0x3b
		"0x3c",			// 0x3c
		"0x3d",			// 0x3d
		"0x3e",			// 0x3e
		"0x3f",			// 0x3f
		"0x40",			// 0x40
		"A",				// 0x41
		"B",		
		"C",		
		"D",		
		"E",		
		"F",		
		"G",		
		"H",		
		"I",		
		"J",		
		"K",		
		"L",		
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"0x5b",			// 0x5b
		"0x5c",			// 0x5c
		"0x5d",			// 0x5d
		"0x5e",			// 0x5e
		"0x5f",			// 0x5f
		"NP0",			// 0x60
		"NP1",
		"NP2",
		"NP3",
		"NP4",
		"NP5",
		"NP6",
		"NP7",
		"NP8",
		"NP9",
		"MUL",			// 0x6a
		"ADD",			// 0x6b
		"SEP",			// 0x6c
		"SUB",			// 0x6d
		"DEC",			// 0x6e
		"DIV",			// 0x6f
		"F1",			// 0x70
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		"F13",
		"F14",
		"F15",
		"F16",
		"F17",
		"F18",
		"F19",
		"F20",
		"F21",
		"F22",
		"F23",
		"F24",
		"0x88",			// 0x88
		"0x89",			// 0x89
		"0x8a",			// 0x8a
		"0x8b",			// 0x8b
		"0x8c",			// 0x8c
		"0x8d",			// 0x8d
		"0x8e",			// 0x8e
		"0x8f",			// 0x8f
		"NUML",			// 0x90
		"SCRL",			// 0x91
		"0x92",			// 0x92
		"0x93",			// 0x93
		"0x94",			// 0x94
		"0x95",			// 0x95
		"0x96",			// 0x96
		"0x97",			// 0x97
		"0x98",			// 0x98
		"0x99",			// 0x99
		"0x9a",			// 0x9a
		"0x9b",			// 0x9b
		"0x9c",			// 0x9c
		"0x9d",			// 0x9d
		"0x9e",			// 0x9e
		"0x9f",			// 0x9f
		"0xa0",			// 0xa0
		"0xa1",			// 0xa1
		"0xa2",			// 0xa2
		"0xa3",			// 0xa3
		"0xa4",			// 0xa4
		"0xa5",			// 0xa5
		"0xa6",			// 0xa6
		"0xa7",			// 0xa7
		"0xa8",			// 0xa8
		"0xa9",			// 0xa9
		"0xaa",			// 0xaa
		"0xab",			// 0xab
		"0xac",			// 0xac
		"0xad",			// 0xad
		"0xae",			// 0xae
		"0xaf",			// 0xaf
		"0xb0",			// 0xb0
		"0xb1",			// 0xb1
		"0xb2",			// 0xb2
		"0xb3",			// 0xb3
		"0xb4",			// 0xb4
		"0xb5",			// 0xb5
		"0xb6",			// 0xb6
		"0xb7",			// 0xb7
		"0xb8",			// 0xb8
		"0xb9",			// 0xb9
		";",			// 0xba
		"0xbb",			// 0xbb
		"<",			// 0xbc
		"0xbd",			// 0xbd
		">",			// 0xbe
		"/",			// 0xbf
		"0xc0",			// 0xc0
		"0xc1",			// 0xc1
		"0xc2",			// 0xc2
		"0xc3",			// 0xc3
		"0xc4",			// 0xc4
		"0xc5",			// 0xc5
		"0xc6",			// 0xc6
		"0xc7",			// 0xc7
		"0xc8",			// 0xc8
		"0xc9",			// 0xc9
		"0xca",			// 0xca
		"0xcb",			// 0xcb
		"0xcc",			// 0xcc
		"0xcd",			// 0xcd
		"0xce",			// 0xce
		"0xcf",			// 0xcf
		"0xd0",			// 0xd0
		"0xd1",			// 0xd1
		"0xd2",			// 0xd2
		"0xd3",			// 0xd3
		"0xd4",			// 0xd4
		"0xd5",			// 0xd5
		"0xd6",			// 0xd6
		"0xd7",			// 0xd7
		"0xd8",			// 0xd8
		"0xd9",			// 0xd9
		"0xda",			// 0xda
		"[",			// 0xdb
		"0xdc",			// 0xdc
		"]",			// 0xdd
		"0xde",			// 0xde
		"0xdf",			// 0xdf
		"0xe0",			// 0xe0
		"0xe1",			// 0xe1
		"0xe2",			// 0xe2
		"0xe3",			// 0xe3
		"0xe4",			// 0xe4
		"0xe5",			// 0xe5
		"0xe6",			// 0xe6
		"0xe7",			// 0xe7
		"0xe8",			// 0xe8
		"0xe9",			// 0xe9
		"0xea",			// 0xea
		"0xeb",			// 0xeb
		"0xec",			// 0xec
		"0xed",			// 0xed
		"0xee",			// 0xee
		"0xef",			// 0xef
		"0xf0",			// 0xf0
		"0xf1",			// 0xf1
		"0xf2",			// 0xf2
		"0xf3",			// 0xf3
		"0xf4",			// 0xf4
		"0xf5",			// 0xf5
		"0xf6",			// 0xf6
		"0xf7",			// 0xf7
		"0xf8",			// 0xf8
		"0xf9",			// 0xf9
		"0xfa",			// 0xfa
		"0xfb",			// 0xfb
		"0xfc",			// 0xfc
		"0xfd",			// 0xfd
		"0xfe",			// 0xfe
		"0xff",			// 0xff
	};

	return KEY_TEXT[ key & 0xff ];
}

CharString Keyboard::keyText( dword key )
{
	CharString string;
	if ( key & HMK_SHIFT_MASK )
		string += "SHIFT+";
	if ( key & HMK_CONTROL_MASK )
		string += "CTRL+";
	if ( key & HMK_ALT_MASK )
		string += "ALT+";
	string += keyTextRaw( key );

	return string;
}

dword Keyboard::textKey( const CharString & sKey )
{
	Array< CharString > parts;
	sKey.split( parts, "+" );

	dword nHotKey = 0;
	for(int k=0;k<parts.size();++k)
	{
		const CharString & sPart = parts[ k ];
		if ( sPart == "ALT" )
		{
			nHotKey |= HMK_ALT_MASK;
			continue;
		}
		if ( sPart == "CTRL" )
		{
			nHotKey |= HMK_CONTROL_MASK;
			continue;
		}
		if ( sPart == "SHIFT" )
		{
			nHotKey |= HMK_SHIFT_MASK;
			continue;
		}

		// must be the key value, look for it in the table..
		for( int i=0;i<256;++i)
			if ( sPart == keyTextRaw(i) )
			{
				nHotKey |= i;
				break;
			}
	}


	return nHotKey;
}

CharString Keyboard::keyShortText( dword key )
{
	CharString string( keyTextRaw( key ) );
	if ( key & HMK_SHIFT_MASK )
		string = CharString("S+") + string;
	if ( key & HMK_CONTROL_MASK )
		string = CharString("C+") + string;
	if ( key & HMK_ALT_MASK )
		string = CharString("A+") + string;

	return string;
}

//----------------------------------------------------------------------------
// EOF

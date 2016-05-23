/**
	@file TypeName.h
	@brief TODO

	(c)2012 Palestar Inc
	@author Richard Lyle @date 4/22/2012 10:29:08 AM
*/

#ifndef TYPENAME_H
#define TYPENAME_H

#include "Standard/CharString.h"
#include "Standard/Types.h"

//---------------------------------------------------------------------------------------------------

//! This function returns the name of the given type..
template<typename T>
struct TypeName
{
	static const CharString & name()
	{
		static CharString sName;
		if ( sName[0] == 0 )
		{
			sName = typeid(T).name();
	#if defined(_WIN32)
			// strip the type qualifiers off the front of VS generated type names,
			// so our types will be normalized between windows & linux..
			sName.replace( "class ", "" );
			sName.replace( "struct ", "" );
			sName.replace( "enum ", "" );
			sName.replace( "union ", "" );
	#endif
		}

		return sName;
	}
};

//---------------------------------------------------------------------------------------------------
// Use specialization on all the basic types, so our type names will be normalized between platforms.

template<>
struct TypeName<bool>
{
	static const CharString & name() 
	{ 
		static CharString name( "bool" );
		return name; 
	}
};

template<>
struct TypeName<u8>
{
	static const CharString & name() 
	{ 
		static CharString name( "u8" );
		return name; 
	}
};

template<>
struct TypeName<s8>
{
	static const CharString & name() 
	{ 
		static CharString name( "s8" );
		return name; 
	}
};

template<>
struct TypeName<u16>
{
	static const CharString & name() 
	{ 
		static CharString name( "u16" );
		return name; 
	}
};

template<>
struct TypeName<s16>
{
	static const CharString & name() 
	{ 
		static CharString name( "s16" );
		return name; 
	}
};

template<>
struct TypeName<u32>
{
	static const CharString & name() 
	{ 
		static CharString name( "u32" );
		return name; 
	}
};

template<>
struct TypeName<s32>
{
	static const CharString & name() 
	{ 
		static CharString name( "s32" );
		return name; 
	}
};

template<>
struct TypeName<u64>
{
	static const CharString & name() 
	{ 
		static CharString name( "u64" );
		return name; 
	}
};

template<>
struct TypeName<s64>
{
	static const CharString & name() 
	{ 
		static CharString name( "s64" );
		return name; 
	}
};

template<>
struct TypeName<ul32>
{
	static const CharString & name() 
	{ 
		static CharString name( "ul32" );
		return name; 
	}
};

template<>
struct TypeName<sl32>
{
	static const CharString & name() 
	{ 
		static CharString name( "sl32" );
		return name; 
	}
};

template<>
struct TypeName<f32>
{
	static const CharString & name() 
	{ 
		static CharString name( "f32" );
		return name; 
	}
};

template<>
struct TypeName<f64>
{
	static const CharString & name() 
	{ 
		static CharString name( "f64" );
		return name; 
	}
};

template<>
struct TypeName<wchar>
{
	static const CharString & name() 
	{ 
		static CharString name( "wchar" );
		return name; 
	}
};

template<>
struct TypeName<CharString>
{
	static const CharString & name() 
	{ 
		static CharString name( "CharString" );
		return name; 
	}
};

template<>
struct TypeName<WideString>
{
	static const CharString & name() 
	{ 
		static CharString name( "WideString" );
		return name; 
	}
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF

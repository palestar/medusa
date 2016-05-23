/*
	Value.h
	
	The Value class can be used to contain a value of any data type in a generic fashion. The
	ValueTemplate class, implements an automatic method to contain a value of any datatype.
	
	Usage:

		ValueTemplate number( 25 );
		int n = number.value();
		assert( n == 25 );

		ValueTemplate string( std::string("hello world") );
		std::string s = string.value();
		assert( s == "hello world" );

	(c)2005 Palestar, Richard Lyle
*/

#ifndef VALUEBASE_H
#define VALUEBASE_H

#include "Reflection/Type.h"
#include "Reflection/TypeTemplate.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class DLL Value
{
public:
	DECLARE_EXCEPTION( FailedToConvertValue );

	// Constructor
	Value();
	Value( const Value & set );
	Value( Type * pType, const void * pData );
	~Value();

	// handle conversion from any data type into this Value object..
	template<typename T>
	Value( const T & assign ) : m_pType( NULL ), m_pData( NULL )
	{
		if (! set( TypeTemplate<T>::instance(), &assign ) )
			throw FailedToConvertValue();
	}

	// Accessors
	Type *			type() const;
	void *			data() const;

	// Mutators
	Value &			operator=( const Value & copy );
	bool			set( Type * pType, const void * pData );
	void			setNoCopy( Type * pType, void * pData );
	bool			copy( Type * pType, const void * pData );
	void			release();

	bool			read( const InStream & input );
	bool			write( const OutStream & output ) const;

	// implement auto-matic conversion of a Value into any data type using the TypeCopy system...
	template<typename T>
	inline operator T() const
	{
		T dst;
		if (! Type::copy( &dst, TypeTemplate<T>::instance(), data(), type() ) )
			throw FailedToConvertValue();
		return dst;
	}


private:
	// Data
	Type::Ref		m_pType;
	void *			m_pData;
};

//---------------------------------------------------------------------------------------------------

inline Value & Value::operator=( const Value & copy )
{
	set( copy.type(), copy.data() );
	return *this;
}

inline const OutStream & operator<<( const OutStream & output, const Value & value )
{
	if (! value.write( output ) )
		throw File::FileError();
	return output;
}

inline const InStream & operator>>( const InStream & input, Value & value )
{
	if (! value.read( input ) )
		throw File::FileError();
	return input;
}

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF

/*
	TypeTemplate.h
	Template class to implement a concrete TypeBase object.

	(c)2006 Palestar, Richard Lyle
*/

#ifndef TYPETEMPLATE_H
#define TYPETEMPLATE_H

#include "Standard/Reference.h"
#include "Reflection/TypeCopy.h"

//---------------------------------------------------------------------------------------------------

// Template class to create a Type, this type cannot be serialized by read/write
template<typename T>
class TypeTemplate : public Type
{
public:
	// Construction
	TypeTemplate() : Type( TypeName<T>::name(), sizeof(T), TypeName<T>::name() )
	{}

	// Type interface
	virtual void * allocate()
	{
		return new T();
	}
	virtual void free( void * p )
	{
		delete ((T *)p);
	}

	// Static
	static Type * instance()		// This is the prefered method to get a Type, it will create the type if it doesn't exist..
	{
		Type * pType = find( TypeName<T>::name() );
		if ( pType == NULL )
			pType = new TypeTemplate<T>;
		return pType;
	}
};

//---------------------------------------------------------------------------------------------------

// Type template class for abstract classes
template<typename T>
class TypeAbstract : public Type
{
public:
	// Construction
	TypeAbstract() : Type( TypeName<T>::name(), sizeof(T), TypeName<T>::name() )
	{}

	// Type interface
	virtual void * allocate()
	{
		return NULL;
	}
	virtual void free( void * p )
    {}
	// Static
	static Type * instance()		// This is the prefered method to get a Type, it will create the type if it doesn't exist..
	{
		Type * pType = find( TypeName<T>::name() );
		if ( pType == NULL )
			pType = new TypeAbstract<T>;
		return pType;
	}
};

//---------------------------------------------------------------------------------------------------

// Serializable data type...
template<typename T>
class TypeSerialized : public Type
{
public:
	TypeSerialized() : Type( TypeName<T>::name(), sizeof(T), TypeName<T>::name() )
	{}

	// Type interface
	virtual void * allocate()
	{
		return new T();
	}
	virtual void free( void * p )
	{
		delete ((T *)p);
	}

	virtual bool read( const InStream & input, void * pData )
	{
		try {
			input >> *((T *)pData);
			return true;
		}
		catch( File::FileError )
		{}

		return false;
	}
	virtual bool write( const OutStream & output, const void * pData )
	{
		output << *((T *)pData);
		return true;
	}

	// Static
	static Type * instance()		// This is the prefered method to get a Type, it will create the type if it doesn't exist..
	{
		Type * pType = find( TypeName<T>::name() );
		if ( pType == NULL )
			pType = new TypeSerialized<T>;
		return pType;
	}
};

//---------------------------------------------------------------------------------------------------

// Handy template function to convert any datatype to another datatype
template<typename T, typename K>
bool ConvertType( T & dst, const K & src )
{
   return Type::copy( &dst, TypeTemplate<T>::instance(), &src, TypeTemplate<K>::instance() );
}

template<typename T, typename K>
T ConvertType( const K & src )
{
	T dst;
	Type::copy( &dst, TypeTemplate<T>::instance(), &src, TypeTemplate<K>::instance() );

	return dst;
}

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF

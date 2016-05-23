/*
	Type.cpp

	This class can be used to build a generic description of a data type.

	See Standard/Properties.h for an interface which allows a class/structure to expose it's data members
	in a generic method. 

	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/StringHash.h"
#include "Standard/String.h"
#include "Standard/UniqueNumber.h"
#include "Standard/Color.h"
#include "Reflection/Type.h"
#include "Reflection/TypeCopy.h"
#include "Reflection/TypeTemplate.h"

#include <stdio.h>

//----------------------------------------------------------------------------

int	Type::s_nNextTypeId = 0;		// next type id

//----------------------------------------------------------------------------

Type::Type( const char * pName, int nSize, const char * pNote /*= NULL*/ ) 
	: m_nTypeId( -1 ), m_pName( pName ), m_nSize( nSize ), m_pNote( pNote ), m_bIsPointer( false ), m_pList( NULL )
{
	dword nHash = StringHash( pName );

	TypeNameHash::Iterator find = typeNameHash().find( nHash );
	if (! find.valid() )
		typeNameHash()[ nHash ] = this;

	// assign a unique type id
	m_nTypeId = s_nNextTypeId++;
	typeIdHash()[ m_nTypeId ] = this;

	// determine if this type is a pointer or not based on it's name
	int n = (int)strlen( m_pName );
	if ( n > 0 )
		m_bIsPointer = m_pName[ n - 1 ] == '*';		// we are a pointer if the last character of the name is an *
}

Type::~Type()
{}

//----------------------------------------------------------------------------

void * Type::allocate()
{
	return malloc( size() );
}

void Type::free( void * p )
{
	free( p );
}

bool Type::read( const InStream & input, void * pData )
{
	return false;
}

bool Type::write( const OutStream & output, const void * pData )
{
	return false;
}

//---------------------------------------------------------------------------------------------------

dword Type::nameHash() const
{
   return StringHash( name() );
}

//----------------------------------------------------------------------------

Type * Type::find( const char * pName )
{
   return findNameHash( StringHash( pName ) );
}

Type * Type::findNameHash( dword nHash )
{
	TypeNameHash::Iterator iType = typeNameHash().find( nHash );
	if ( iType.valid() )
		return *iType;

	// type not found, return NULL
	return NULL;
}

int	Type::findTypeId( const char * pName )
{
	Type * pType = find( pName );
	if ( pType != NULL )
		return pType->typeId();
	return -1;
}

Type * Type::find( int nTypeId )
{
	TypeIdHash::Iterator iType = typeIdHash().find( nTypeId );
	if ( iType.valid() )
		return *iType;

	// type not found, return NULL
	return NULL;
}


bool Type::copy( void * pLeft, const Type * pLeftType, const void * pRight, const Type * pRightType )
{
	return TypeCopy::copy( pLeft, pLeftType, pRight, pRightType );
}

//---------------------------------------------------------------------------------------------------

void Type::initialize()
{
	TypeSerialized<bool>::instance();
	TypeSerialized<u8>::instance();
	TypeSerialized<s8>::instance();
	TypeSerialized<u16>::instance();
	TypeSerialized<s16>::instance();
	TypeSerialized<u32>::instance();
	TypeSerialized<s32>::instance();
	TypeSerialized<ul32>::instance();
	TypeSerialized<sl32>::instance();
	TypeSerialized<u64>::instance();
	TypeSerialized<s64>::instance();
	TypeSerialized<f32>::instance();
	TypeSerialized<f64>::instance();
	TypeSerialized<WideString>::instance();
	TypeSerialized<CharString>::instance();
	TypeSerialized<Color>::instance();
	TypeSerialized<RectInt>::instance();
	TypeSerialized<RectFloat>::instance();
	TypeSerialized<PointInt>::instance();
	TypeSerialized<PointFloat>::instance();
	TypeSerialized<SizeInt>::instance();
	TypeSerialized<SizeFloat>::instance();
	TypeSerialized<UniqueNumber>::instance();

	TypeTemplate<const char *>::instance();
	TypeTemplate<const wchar *>::instance();
}

void Type::release()
{
	typeNameHash().release();
	typeIdHash().release();
	s_nNextTypeId = 0;
}
	
Type::TypeNameHash & Type::typeNameHash()
{
	static TypeNameHash HASH;
	return HASH;
}

Type::TypeIdHash & Type::typeIdHash()
{
	static TypeIdHash HASH;
	return HASH;
}

//----------------------------------------------------------------------------
//EOF

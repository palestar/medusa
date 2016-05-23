/**
	@file TypeHotKey.h
	@brief Reflection classes for a hotkey.

	(c)2010 Palestar Inc
	@author Richard Lyle @date 5/13/2010 9:34:48 PM
*/

#ifndef TYPEHOTKEY_H
#define TYPEHOTKEY_H

#include "Reflection/TypeCopy.h"
#include "System/Keyboard.h"

//---------------------------------------------------------------------------------------------------

// TypeCopy object for converting a custom HotKey type
template< typename T>
class TypeCopyHotKey : public TypeCopy
{
public:
	TypeCopyHotKey( Type * pType ) : 
	  TypeCopy( pType, pType )
	  {}

	  // TypeCopy interface
	  bool copy( void * pLeft, const void * pRight )
	  {
		  (*((T *)pLeft)) = (*((const T *)pRight));
		  return true;
	  }
};

// Template class to convert a std::string into HotKey
template< typename T >
class TypeCopyStringHotKey : public TypeCopy
{
public:
	TypeCopyStringHotKey( Property * pProperty ) : 
	  TypeCopy( pProperty->type(), TypeSerialized<CharString>::instance() ), m_pProperty( pProperty ) 
	  {}

	  // TypeCopy interface
	  bool copy( void * pLeft, const void * pRight )
	  {
		  (*((T *)pLeft)) = Keyboard::textKey( *((const CharString *)pRight) );
		  return true;
	  }
protected:
	// Data
	Property *	m_pProperty;
};

// Template class to convert bits into a string...
template<typename T>
class TypeCopyHotKeyString : public TypeCopy
{
public:
	TypeCopyHotKeyString( Property * pProperty ) : 
	  TypeCopy( TypeSerialized<CharString>::instance(), pProperty->type() ), m_pProperty( pProperty ) 
	  {}

	  // TypeCopy interface
	  bool copy( void * pLeft, const void * pRight )
	  {
		  T nHotKey = (*(T *)pRight);
		  (*((CharString *)pLeft)) = Keyboard::keyText( nHotKey );
		  return true;
	  }

protected:
	// Data
	Property *	m_pProperty;
};

template<typename T>
void AddHotKeyProperty( PropertyList * pList, const char * pName, const T & member, 
				 int nElements, dword nFlags, const char * pNote )
{
	Type * pType = new TypeSerialized<T>();
	Type * pDWORD = TypeSerialized<dword>::instance();

	Property * pProperty = new Property( new StaticContainer( nElements, pType->size() ), pName, 
		reinterpret_cast<int>( &member ), pType, nFlags, pNote );
	pList->addProperty( pProperty );

	// T -> T
	new TypeCopyHotKey<T>( pType );
	// T -> unsigned int conversion
	new TypeCopyCast<dword,T>( pDWORD, pType );
	// unsigned int -> T conversion
	new TypeCopyCast<T,dword>( pType, pDWORD );
	// CharString -> T
	new TypeCopyStringHotKey<T>( pProperty );
	// T -> CharString
	new TypeCopyHotKeyString<T>( pProperty );
}

#define ADD_HOTKEY_PROPERTY( MEMBER )										\
	AddHotKeyProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_HOTKEY, 0 );


#endif

//---------------------------------------------------------------------------------------------------
//EOF

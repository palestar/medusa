/**
	@file TypeCopyEnum.h
	@brief TypeCopy template classes for converting enumeration types.

	(c)2010 Palestar Inc
	@author Richard Lyle @date 5/13/2010 2:55:38 PM
*/

#ifndef TYPECOPYENUM_H
#define TYPECOPYENUM_H

#include "TypeCopy.h"
#include "Property.h"

//---------------------------------------------------------------------------------------------------

// TypeCopy object for converting a custom enum type
template< typename T>
class TypeCopyEnum : public TypeCopy
{
public:
   TypeCopyEnum( Type * pType ) : TypeCopy( pType, pType )
   {}

	// TypeCopy interface
 	bool copy( void * pLeft, const void * pRight )
	{
		(*((T *)pLeft)) = (*((const T *)pRight));
		return true;
	}
};

// This TypeCopy converts from T to CharString
template<typename T>
class TypeCopyEnumString : public TypeCopy
{
public:
	TypeCopyEnumString( Property * pProperty ) : 
	  TypeCopy( Type::find(TypeName<CharString>::name()), pProperty->type() ), m_pProperty( pProperty )
	{}

	bool copy( void * pLeft, const void * pRight )
	{
		unsigned int nValue = static_cast<unsigned int>( *((T *)pRight) );
		CharString sValue;
		sValue.format( "%u", nValue );

		for(int k=0;k<m_pProperty->optionCount();++k)
		{
			const Property::Option & option = m_pProperty->option( k );
			if ( option.sValue == sValue )
			{
				(*((CharString *)pLeft)) = option.sName;
				return true;
			}
		}

		return false;
	}

protected:
	//! Data
	Property *		m_pProperty;
};

// This TypeCopy converts from an CharString to T
template<typename T>
class TypeCopyStringEnum : public TypeCopy
{
public:
	TypeCopyStringEnum( Property * pProperty ) : 
	  TypeCopy( pProperty->type(), Type::find(TypeName<CharString>::name()) ), m_pProperty( pProperty )
	{}

	bool copy( void * pLeft, const void * pRight )
	{
		CharString & sValue = *((CharString *)pRight);
		for(int k=0;k<m_pProperty->optionCount();++k)
		{
			const Property::Option & option = m_pProperty->option( k );
			if ( option.sName == sValue )
			{
				(*((T *)pLeft)) = (T)StringBase::strdword( option.sValue );
				return true;
			}
		}

		return false;
	}

protected:
	//! Data
	Property *		m_pProperty;
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF

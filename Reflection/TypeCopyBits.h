/**
@file TypeCopyBits.h
@brief TODO

(c)2010 Palestar Inc
@author Richard Lyle @date 5/13/2010 8:39:52 PM
*/

#ifndef TYPECOPYBITS_H
#define TYPECOPYBITS_H

#include "TypeCopy.h"
#include "Property.h"

//---------------------------------------------------------------------------------------------------

// TypeCopy object for converting a custom Bits type
template< typename T>
class TypeCopyBits : public TypeCopy
{
public:
	TypeCopyBits( Type * pType ) : 
	  TypeCopy( pType, pType )
	  {}

	  // TypeCopy interface
	  bool copy( void * pLeft, const void * pRight )
	  {
		  (*((T *)pLeft)) = (*((const T *)pRight));
		  return true;
	  }
};

// Template class to convert a std::string into Bits
template< typename T >
class TypeCopyStringBits : public TypeCopy
{
public:
	TypeCopyStringBits( Property * pProperty ) : 
	  TypeCopy( pProperty->type(), Type::find( TypeName<CharString>::name() ) ), m_pProperty( pProperty ) 
	  {}

	  // TypeCopy interface
	  bool copy( void * pLeft, const void * pRight )
	  {
		  CharString & sValue = *((CharString *)pRight);

		  Array< CharString > bits;
		  sValue.split( bits, "|" );

		  T nBits = 0;
		  for(int k=0;k<bits.size();++k)
		  {
			  const CharString & sBit = bits[ k ];
			  for(int j=0;j<m_pProperty->optionCount();j++)
			  {
				  const Property::Option & option = m_pProperty->option( j );
				  if ( sBit == option.sName )
				  {
					  nBits |= StringBase::strdword( option.sValue );
					  break;
				  }
			  }
		  }

		  (*((T *)pLeft)) = nBits;
		  return true;
	  }
protected:
	// Data
	Property *	m_pProperty;
};

// Template class to convert bits into a string...
template<typename T>
class TypeCopyBitsString : public TypeCopy
{
public:
	TypeCopyBitsString( Property * pProperty ) : 
	  TypeCopy( Type::find(TypeName<CharString>::name()), pProperty->type() ), m_pProperty( pProperty ) 
	  {}

	  // TypeCopy interface
	  bool copy( void * pLeft, const void * pRight )
	  {
		  CharString sValue;

		  T nBits = (*(T *)pRight);
		  for(int k=0;k<m_pProperty->optionCount();k++)
		  {
			  const Property::Option & option = m_pProperty->option( k );
			  T nBit = StringBase::strdword( option.sValue );
			  if ( (nBit & nBits) == nBit )
			  {
				  if ( sValue.length() > 0 )
					  sValue += "|";
				  sValue += option.sName;
			  }
		  }

		  (*((CharString *)pLeft)) = sValue;
		  return true;
	  }

protected:
	// Data
	Property *	m_pProperty;
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF

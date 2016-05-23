/*
	TypeCopy.h

	This class object handles the conversion of one Type to another Type. All TypeCopy
	objects are registered into a static directed graph, this is used to find a conversion
	path from one data type to another type.

	see Type.h

	(c)2006 Palestar, Richard Lyle
*/

#ifndef TYPECOPYBASE_H
#define TYPECOPYBASE_H

#include "Reflection/Type.h"
#include "Standard/Graph.h"
#include "Standard/Reference.h"
#include "Standard/Types.h"

#include <map>	
#ifdef WIN32
#include <typeinfo.h>
#else
#include <typeinfo>
#endif

#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class DLL TypeCopy : public Referenced
{
public:
	// Types
	typedef Reference< TypeCopy >		Ref;

	// Construction
	TypeCopy( Type * pLeft, Type * pRight );
	~TypeCopy();

	// Accessors
	qword				hash() const;										// hash of left and right id
	Type *				left() const;										// destination type
	Type *				right() const;										// source type

	// Interface
	virtual bool		copy( void * pLeft, const void * pRight ) = 0;		// pRight -> pLeft Copy, return false on failure

	// Static
	static TypeCopy *	find( int nLeftId, int nRightId );					// find a TypeCopy object

	// given the right type, and a list of left types, find the closest left type..
	static Type *		findBestType( const Array< Type * > & LeftTypes,
								Type * pRightType );
	// copy from one type to another, handles conversions between multiple types if needed
	static bool			copy( void * pLeft, int nLeftType, 
								const void * pRight, int nRightType );		
	static bool			copy( void * pLeft, const Type * pLeftType, 
								const void * pRight, const Type * pRightType );		

	static void			initialize();			
	static void			release();

private:
	typedef Graph< Type::Ref >						TypeCopyGraph;
	typedef Hash< qword, Ref >						TypeCopyHash;
	typedef Hash< qword, Array< int > >				TypePathHash;

	// Mutators
	static bool			findPath( const Type * pLeftType, 
								const Type * pRightType, Array< int > & path );

	// Data
	qword				m_nHash;
	Type *				m_pLeft;								// destination type
	Type *				m_pRight;								// source type

	static TypeCopyHash &	
						typeCopyHash();		
	static TypeCopyGraph &	
						typeCopyGraph();
	static TypePathHash &	
						typePathHash();
};

//----------------------------------------------------------------------------

inline qword TypeCopy::hash() const
{
	return m_nHash; 
}

inline Type * TypeCopy::left() const
{
	return m_pLeft;
}

inline Type * TypeCopy::right() const
{
	return m_pRight;
}

//----------------------------------------------------------------------------

// abstract template class to make a simple TypeCopy object, the user is still required to implement the copy() method...
template<typename C, typename T, typename K>
class TypeCopyTemplate : public TypeCopy
{
public:
	TypeCopyTemplate() : TypeCopy( Type::find( TypeName<T>::name() ), 
		Type::find( TypeName<K>::name() ) )
	{}
	TypeCopyTemplate( Type * pLeft, Type * pRight ) : TypeCopy( pLeft, pRight )
	{}

	// Static
	static TypeCopy * instance()
	{
		TypeCopy * pInstance = find( Type::findTypeId(TypeName<T>::name()), 
			Type::findTypeId( TypeName<K>::name() ) );
		if ( pInstance == NULL )
			pInstance = new C();
		return pInstance;
	}
};

// TypeCopy object that uses the operator= to perform the copy
template<typename T, typename K>
class TypeCopyDefault : public TypeCopyTemplate<TypeCopyDefault<T,K>,T,K>
{
public:
	TypeCopyDefault() 
	{}
	TypeCopyDefault( Type * pLeft, Type * pRight ) : TypeCopyTemplate<TypeCopyDefault<T,K>,T,K>( pLeft, pRight )
	{}
	~TypeCopyDefault()
	{}

	// TypeCopy interface
	bool copy( void * pLeft, const void * pRight )
	{
		(*((T *)pLeft)) = (*((const K *)pRight));
		return true;
	}

};

template<typename T, typename K>
class TypeCopyCast : public TypeCopyTemplate<TypeCopyCast<T,K>,T,K>
{
public:
	TypeCopyCast() 
	{}
	TypeCopyCast( Type * pLeft, Type * pRight ) : TypeCopyTemplate<TypeCopyCast<T,K>,T,K>( pLeft, pRight )
	{}
	~TypeCopyCast()
	{}

	// TypeCopy interface
	bool copy( void * pLeft, const void * pRight )
	{
		(*((T *)pLeft)) = (T)(*((const K *)pRight));
		return true;
	}
};


#endif

//---------------------------------------------------------------------------------------------------
//EOF

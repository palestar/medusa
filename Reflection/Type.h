/*
	Type.h
	
	This is the base class to contain/describe a data-type in a generic way. This include methods to allocate, free,
	read, and write that data-type (in binary). 

	Additionally, it provides the static interface to convert one Type into another Type.. see Type::copy()

	(c)2006 Palestar, Richard Lyle
*/

#ifndef TYPEBASE_H
#define TYPEBASE_H

#include "Standard/Graph.h"				// Graph template class
#include "Standard/Reference.h"			// smart-pointer
#include "Standard/Types.h"
#include "Standard/Hash.h"
#include "File/Stream.h"

#include "TypeName.h"

#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class PropertyList;						// forward declare
class DLL Type : public Referenced
{
public:
	// Types
	typedef Reference< Type >		Ref;

	// Construction
	Type( const char * pName, int nSize, const char * pNote );
	~Type();

	// Accessors
	int					typeId() const;						// type id
	const char *		name() const;						// type name
	int					size() const;						// size of type in bytes
	const char *		note() const;						// type description, may return NULL
	bool				isPointer() const;					// is this type a pointer
	bool				isReference() const;             // is this type a Reference, if true containers will not delete this object

	PropertyList *		propertyList() const;				// is this type described by a PropertyList

	// Mutators
	void				setIsPointer( bool bIsPointer );
	void				setIsReference( bool bIsReference );
	void				setPropertyList( PropertyList * pList );

	// Interface
	virtual void *		allocate();							// allocate this type, this may call the Type constructor
	virtual void		free( void * p );					// free this type, this may call the desructor

	virtual bool		read( const InStream & input,			// binary serialization
								void * pData );
	virtual bool		write( const OutStream & output, 
								const void * pData );

   // Helpers
   dword				nameHash() const;                // get the hash for the type name

	// Static
	static Type *		find( const char * pName );
	static Type *		findNameHash( dword nNameHash );
	static int			findTypeId( const char * pName );
	static Type *		find( int nTypeId );

	// copy from one type to another, handles conversions between multiple types if needed
	static bool			copy( void * pLeft, const Type * pLeftType, 
							   const void * pRight, const Type * pRightType );		

	static void			initialize();							// call to initialize all the basic types
	static void			release();								// release all types

protected:
	// Types
	typedef Hash< dword, Ref >		TypeNameHash;
	typedef Hash< int, Ref >		TypeIdHash;

	// Data
	int					m_nTypeId;
	const char *		m_pName;
	int					m_nSize;
	const char *		m_pNote;
	bool				m_bIsPointer;
	bool				m_bIsReference;
	PropertyList *		m_pList;

	static int			s_nNextTypeId;							

	static TypeNameHash &	
						typeNameHash();
	static TypeIdHash &	
						typeIdHash();
};

//----------------------------------------------------------------------------

inline int Type::typeId() const
{
	return m_nTypeId;
}

inline const char * Type::name() const
{
	return m_pName;
}

inline int Type::size() const
{
	return m_nSize;
}

inline const char * Type::note() const
{
	return m_pNote;
}

inline bool Type::isPointer() const
{
	return m_bIsPointer;
}

inline bool Type::isReference() const
{
   return m_bIsReference;
}

inline PropertyList * Type::propertyList() const
{
	return m_pList;
}

//---------------------------------------------------------------------------------------------------

inline void Type::setIsPointer( bool bIsPointer )
{
	m_bIsPointer = bIsPointer;
}

inline void Type::setIsReference( bool bIsReference )
{
   m_bIsReference = bIsReference;
}

inline void Type::setPropertyList( PropertyList * pList )
{
	m_pList = pList;
}

//---------------------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------------------
//EOF

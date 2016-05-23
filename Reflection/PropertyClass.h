/*
	PropertyClass.h
	Mixin class for any class that has a PropertyList

	Usage:

	// Firstly, the class (or one of it's parents) must inhert from PropertyClass, see PropertyClass.h
	class MyClass : public PropertyClass
	{
	public:
	DECLARE_PROPERTY_LIST();		// This line must exist in the class definition

	private:
	int		m_nMyInteger;			// some private properties we're going to expose
	String	m_sMyString;
	};

	// In the class implementation, you must have the following lines:
	BEGIN_PROPERTY_LIST( MyClass, PropertyClass )
	ADD_PROPERTY( m_nMyInterger )
	ADD_PROPERTY( m_sMyString )
	END_PROPERTY_LIST()

	// now you can access those data members through the get() and set() template functions...
	PropertyClass * pSomeClass = new MyClass;

	int n = 32;
	pMyClass->set( "m_nMyInteger", n );

	// The properties are automatically converted to whatever data type you provide
	String s;
	pMyClass->get( "m_nMyInteger", s );

	// s == "32"

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef PROPERTYCLASS_H
#define PROPERTYCLASS_H

#include "Reflection/PropertyList.h"
#include "Reflection/Containers.h"
#include "Reflection/TypeTemplate.h"
#include "Reflection/TypeCopyEnum.h"
#include "Reflection/TypeCopyBits.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

template<typename T>
void AddProperty( PropertyList * pList, const char * pName, const T & member, 
				 int nElements, dword nFlags, const char * pNote )
{
	Type * pType = TypeSerialized<T>::instance();
	pList->addProperty( new Property( new StaticContainer( nElements, pType->size() ), pName, 
		reinterpret_cast<int>( &member ), pType, nFlags, pNote ) );
}

template<typename T>
void AddProperty( PropertyList * pList, const char * pName, const Array<T> & member,
				 int nElements, dword nFlags, const char * pNote )
{
	Type * pType = TypeSerialized< T >::instance();
	TypeSerialized< Array<T> >::instance();
	pList->addProperty( new Property( new ArrayContainer<T>(), pName, 
		reinterpret_cast<int>( &member ), pType, nFlags, pNote ) );
}

template<typename T>
void AddProperty( PropertyList * pList, const char * pName, const std::vector<T> & member,
				 int nElements, dword nFlags, const char * pNote )
{
	Type * pType = TypeSerialized< T >::instance();
	TypeSerialized< Array<T> >::instance();
	pList->addProperty( new Property( new VectorContainer<T>(), pName, 
		reinterpret_cast<int>( &member ), pType, nFlags, pNote ) );
}

template<typename T>
void AddProperty( PropertyList * pList, const char * pName, const std::list<T> & member,
				 int nElements, dword nFlags, const char * pNote )
{
	Type * pType = TypeSerialized< T >::instance();
	TypeSerialized< Array<T> >::instance();
	pList->addProperty( new Property( new ListContainer<T>(), pName, 
		reinterpret_cast<int>( &member ), pType, nFlags, pNote ) );
}

template<typename T>
void AddEnumProperty( PropertyList * pList, const char * pName, const T & member, 
				 int nElements, dword nFlags, const char * pNote )
{
	Type * pType = new TypeSerialized<T>();
	Type * pUnsignedInt = TypeSerialized<unsigned int>::instance();

	Property * pProperty = new Property( new StaticContainer( nElements, pType->size() ), pName, 
		reinterpret_cast<int>( &member ), pType, nFlags, pNote );
	pList->addProperty( pProperty );

	// T -> T
	new TypeCopyEnum<T>( pType );
	// T -> unsigned int conversion
	new TypeCopyCast<unsigned int,T>( pUnsignedInt, pType );
	// unsigned int -> T conversion
	new TypeCopyCast<T,unsigned int>( pType, pUnsignedInt );
	// CharString -> T
	new TypeCopyStringEnum<T>( pProperty );
	// T -> CharString
	new TypeCopyEnumString<T>( pProperty );
}

template<typename T>
void AddBitsProperty( PropertyList * pList, const char * pName, const T & member, 
				 int nElements, dword nFlags, const char * pNote )
{
	Type * pType = new TypeSerialized<T>();
	Type * pDWORD = TypeSerialized<dword>::instance();

	Property * pProperty = new Property( new StaticContainer( nElements, pType->size() ), pName, 
		reinterpret_cast<int>( &member ), pType, nFlags, pNote );
	pList->addProperty( pProperty );

	// T -> T
	new TypeCopyBits<T>( pType );
	// T -> unsigned int conversion
	new TypeCopyCast<dword,T>( pDWORD, pType );
	// unsigned int -> T conversion
	new TypeCopyCast<T,dword>( pType, pDWORD );
	// CharString -> T
	new TypeCopyStringBits<T>( pProperty );
	// T -> CharString
	new TypeCopyBitsString<T>( pProperty );
}


//---------------------------------------------------------------------------------------------------

template<typename T>
class PropertyListInitializer
{ 
public:
	PropertyListInitializer()
	{
		TypeCopy::initialize();
		T::staticPropertyList();
	}
};

//---------------------------------------------------------------------------------------------------

#define DECLARE_PROPERTY_LIST()											\
	virtual PropertyList *			propertyList();						\
	static PropertyList *			staticPropertyList();

//---------------------------------------------------------------------------------------------------

#pragma warning( disable : 4312 )	// conversion from 'int' to 'ElysiumContext *' of greater size

#define BEGIN_NAMED_PROPERTY_LIST( NAME, CLASS, BASE )					\
	PropertyListInitializer<CLASS> NAME;								\
	PropertyList * CLASS::propertyList()								\
	{																	\
		return staticPropertyList();									\
	}																	\
	PropertyList * CLASS::staticPropertyList()							\
	{																	\
		static PropertyListTemplate<CLASS> LIST( BASE::staticPropertyList() );	\
		if (! LIST.initialized() )										\
		{																\
			int nOffset = ((int)(CLASS *)(PropertyClass *)1) - 1;		\
			CLASS * pOBJECT = (CLASS *)nOffset;							\
			LIST.initialize();


#define BEGIN_PROPERTY_LIST( CLASS, BASE )								\
	BEGIN_NAMED_PROPERTY_LIST( PLIST_##CLASS, CLASS, BASE )

//---------------------------------------------------------------------------------------------------

#define BEGIN_NAMED_ABSTRACT_PROPERTY_LIST( NAME, CLASS, BASE )			\
	PropertyListInitializer<CLASS> NAME;								\
	PropertyList * CLASS::propertyList()								\
	{																	\
		return staticPropertyList();									\
	}																	\
	PropertyList * CLASS::staticPropertyList()							\
	{																	\
		static PropertyListAbstractTemplate<CLASS> LIST( BASE::staticPropertyList() );	\
		if (! LIST.initialized() )										\
		{																\
			int nOffset = ((int)(CLASS *)(PropertyClass *)1) - 1;		\
			CLASS * pOBJECT = (CLASS *)nOffset;							\
			LIST.initialize();

#define BEGIN_ABSTRACT_PROPERTY_LIST( CLASS, BASE )						\
	BEGIN_NAMED_ABSTRACT_PROPERTY_LIST( PLIST_##CLASS, CLASS, BASE )

//----------------------------------------------------------------------------

#define ADD_PROPERTY( MEMBER )											\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, 0, 0 );
#define ADD_PROPERTY_WITH_FLAGS( MEMBER, FLAGS )						\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, FLAGS, 0 );
#define ADD_NOEDIT_PROPERTY( MEMBER )									\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_NOEDIT, 0 );
#define ADD_NOSAVE_PROPERTY( MEMBER )									\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_NOSAVE, 0 );
#define ADD_TRANSMIT_PROPERTY( MEMBER )									\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_TRANSMIT, 0 );
#define ADD_OBJECT_PROPERTY( MEMBER )								\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_OBJECT, 0 );
#define ADD_UPDATE_PROPERTY( MEMBER )								\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_UPDATE, 0 );
#define ADD_TRANSMIT_UPDATE_PROPERTY( MEMBER )								\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_TRANSMIT|PF_UPDATE, 0 );
#define ADD_REPLICATED_PROPERTY( MEMBER, INTERVAL )							\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_REPLICATE|PF_TRANSMIT|PF_UPDATE, 0 );	\
	LIST.addOption( #MEMBER, "_INTERVAL", ConvertType<CharString,dword>( INTERVAL ) );
#define ADD_REPLICATED_UP_PROPERTY( MEMBER, INTERVAL )							\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_REPLICATE_UP|PF_REPLICATE|PF_TRANSMIT|PF_UPDATE, 0 );	\
	LIST.addOption( #MEMBER, "_INTERVAL", ConvertType<CharString,dword>( INTERVAL ) );

#define ADD_COLOR_PROPERTY( MEMBER )									\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_COLOR, 0 );
#define ADD_FILE_PROPERTY( MEMBER )										\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_FILENAME, 0 );

#define ADD_CLASSKEY_PROPERTY( MEMBER, BASE )							\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_CLASSKEY, 0 );	\
	LIST.addOption( #MEMBER, "_BASE", #BASE );
#define ADD_WIDGETKEY_PROPERTY( MEMBER, BASE )							\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_WIDGETKEY, 0 );	\
	LIST.addOption( #MEMBER, "_BASE", #BASE );
#define ADD_PORT_PROPERTY( MEMBER, BASE )								\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_PORT, 0 );		\
	LIST.addOption( #MEMBER, "_BASE", #BASE );

#define ADD_BITS_PROPERTY( MEMBER )										\
	AddBitsProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_BITS, 0 );
#define ADD_ENUM_PROPERTY( MEMBER )										\
	AddEnumProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, PF_ENUM, 0 );

#define ADD_PROPERTY_NOTE( MEMBER, NOTE )								\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, 0, NOTE );
//#define ADD_PROPERTY_FLAGS( MEMBER, FLAGS )								\
//	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER, 1, FLAGS, 0 );
#define ADD_PROPERTY_ARRAY( MEMBER )									\
	AddProperty( &LIST, #MEMBER, pOBJECT->MEMBER[0], sizeof(pOBJECT->MEMBER)/sizeof(pOBJECT->MEMBER[0]), 0, 0 );

#define ADD_OPTION( PROP, OPTION )										\
	LIST.addOption( #PROP, #OPTION, #OPTION );
#define ADD_ENUM_OPTION( PROP, OPTION )									\
	LIST.addOption( #PROP, #OPTION, ConvertType<CharString,unsigned int>( OPTION ) );
#define ADD_BIT_OPTION( PROP, OPTION )									\
	LIST.addOption( #PROP, #OPTION, ConvertType<CharString,dword>( OPTION ) );
#define ADD_STRING_OPTION( PROP, OPTION )								\
	LIST.addOption( #PROP, OPTION, OPTION );
#define ADD_OPTION_NOTE( PROP, OPTION, NOTE )							\
	LIST.addOption( #PROP, #OPTION, NOTE );

#define SET_PROPERTY_FLAGS( PROP, OPTION )								\
	LIST.setFlags( #PROP, OPTION );
#define ADD_PROPERTY_FLAGS( PROP, OPTION )								\
	LIST.addFlags( #PROP, OPTION );
#define CLEAR_PROPERTY_FLAGS( PROP, OPTION )							\
	LIST.clearFlags( #PROP, OPTION );

#define SET_PROPERTY_NOTE( PROP, OPTION )								\
	LIST.setNote( #PROP, OPTION );

//----------------------------------------------------------------------------

#define END_PROPERTY_LIST()												\
}																		\
	return &LIST;														\
}

//---------------------------------------------------------------------------------------------------

class DLL PropertyClass : public Referenced
{
public:
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< PropertyClass >     Ref;

	// Mutators
	int				propertyCount();              // get the total number of properties for this class
	Property *		property( int n );            // get property n for this class
	int				propertyElements( int n );    // get the number of elements in property n

	// Interface
	virtual bool	copy( void * pLeft, Type * pLeftType, const void * pRight, Type * pRightType, 
						Property * pProperty, int nElement, bool bMutator );

	virtual void	OnPreSave();                   // called before this object is serialized 
	virtual void	OnPostSave();                  // called after this object has been saved
	virtual void	OnPreLoad();                   // called before this object is loaded
	virtual void	OnPostLoad();                  // called after this object has been loaded


	// Helpers
	const char *	className();
	bool			find( const char * pExpr, Property::Find & found );
	bool			enumerate( Array< Property::Find > & expr );

	template<typename T>
	bool get( const char * pName, T & value )
	{
		Property::Find found;
		if (! find( pName, found ) )
			return false;
		return copy( &value, TypeTemplate<T>::instance(), found.pData, found.pType, found.pProperty, found.nElement, false );
	}

	template<typename T>
	bool get( Property * pProperty, int n, T & value )
	{
		return copy( &value, TypeTemplate<T>::instance(), pProperty->pointer( this, n ), pProperty->type(), pProperty, n, false );
	}

	bool get( Property * pProperty, int n, Value & a_Value )
	{
		return a_Value.set( pProperty->type(), pProperty->pointer( this, n ) );
	}

	template<typename T>
	bool set( const char * pName, const T value )
	{
		Property::Find found;
		if (! find( pName, found ) )
			return false;
		return copy( found.pData, found.pType, &value, TypeTemplate<T>::instance(), found.pProperty, found.nElement, true );
	}
	bool set( const char * pName, const Value & value )
	{
		Property::Find found;
		if (! find( pName, found ) )
			return false;
		return copy( found.pData, found.pType, value.data(), value.type(), found.pProperty, found.nElement, true );
	}

	bool set( Property * pProperty, int n, const Value & value )
	{
		return copy( pProperty->pointer( this, n ), pProperty->type(), value.data(), value.type(), pProperty, n, true );
	}

	template<typename T>
	bool set( Property * pProperty, int n, const T value )
	{
		return copy( pProperty->pointer( this, n ), pProperty->type(), &value, TypeTemplate<T>::instance(), pProperty, n, true );
	}
};

//---------------------------------------------------------------------------------------------------

inline int PropertyClass::propertyCount()
{
	return propertyList()->propertyCount();
}

inline Property * PropertyClass::property( int n )
{
	return propertyList()->property( n );
}

inline int PropertyClass::propertyElements( int n )
{
	return property( n )->count( this );
}

inline const char *	PropertyClass::className()
{
	return propertyList()->className();
}

inline bool PropertyClass::find( const char * pExpr, Property::Find & found )
{
	PropertyList * pList = propertyList();
	if ( pList == NULL || pExpr == NULL )
		return false;
	if ( pList->find( Expression( pExpr ), this, found ) )
	{
		found.sExpr = pExpr;
		return true;
	}
	return false;
}

inline bool PropertyClass::enumerate( Array< Property::Find > & expr )
{
	return propertyList()->enumerate( this, expr );
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
//EOF

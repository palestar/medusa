/*
	FactoryTypes.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef FACTORYTYPES_H
#define FACTORYTYPES_H

#include "Factory.h"
#include "WidgetReference.h"
#include "Resource/ResourceLink.h"

#include "FactoryConcrete.h"
#include "FactoryAbstract.h"

//---------------------------------------------------------------------------------------------------

template<class T, typename P>
void InitializePointerType()
{
	Type * pPointerType = TypeSerialized< P >::instance();
	pPointerType->setPropertyList( T::staticPropertyList() );
	pPointerType->setIsPointer( true );

	TypeCopyDefault< P, P >::instance();
	TypeCopyDefault< P, T * >::instance();
	TypeCopyDefault< T *, P >::instance();
}

//---------------------------------------------------------------------------------------------------

template<class T>
class FactoryTypes
{
public:
	FactoryTypes()
	{
		TypeSerialized< T >::instance();
		TypeSerialized< T * >::instance();

		InitializePointerType<T, Reference<T> >();
		InitializePointerType<T, WidgetReference<T> >();
	}
};

//---------------------------------------------------------------------------------------------------

template<class T>
class ResourceTypes
{
public:
	ResourceTypes()
	{
		TypeSerialized< T >::instance();
		TypeSerialized< T * >::instance();

		InitializePointerType<T, Reference<T> >();
		InitializePointerType<T, WidgetReference<T> >();
		InitializePointerType<T, ResourceLink<T> >();
	}
};

//---------------------------------------------------------------------------------------------------

template<class T>
class AbstractFactoryTypes
{
public:
	AbstractFactoryTypes()
	{
		TypeAbstract<T>::instance();
		TypeSerialized< T * >::instance();

		InitializePointerType<T, Reference<T> >();
		InitializePointerType<T, WidgetReference<T> >();
	}
};

//---------------------------------------------------------------------------------------------------

// This class object is declare in the global scope, and forces the registration of a Factory object.
template<typename T>
class RegisterFactory 
{
public:
	RegisterFactory() : m_pFactory( T::staticFactory() )
	{}
	Factory *		m_pFactory;
};

//---------------------------------------------------------------------------------------------------

#define IMPLEMENT_WIDGET_TYPES( CLASS )																		\
	FactoryTypes<CLASS>				TYPES_##CLASS;
#define IMPLEMENT_NAMED_WIDGET_TYPES( NAME, CLASS )															\
	FactoryTypes<CLASS>				NAME;
#define IMPLEMENT_RESOURCE_WIDGET_TYPES( CLASS )															\
	ResourceTypes<CLASS>			TYPES_##CLASS;
#define IMPLEMENT_ABSTRACT_WIDGET_TYPES( CLASS )															\
	AbstractFactoryTypes<CLASS>		TYPES_##CLASS;

//---------------------------------------------------------------------------------------------------

#define IMPLEMENT_NAMED_FACTORY( N, C, B, SK )																\
	Factory * C::staticFactory() { static FactoryConcrete<C> F( #C, B::staticFactory(), SK ); return &F; }	\
	Factory * C::factory() const { return staticFactory(); }												\
	const ClassKey & C::classKey() { return staticFactory()->classKey();  }									\
	RegisterFactory<C> N;																				

#define IMPLEMENT_NAMED_ABSTRACT_FACTORY( N, C, B, SK )														\
	Factory * C::staticFactory() { static FactoryAbstract<C> F( #C, B::staticFactory(), SK ); return &F; }	\
	Factory * C::factory() const { return staticFactory(); }												\
	const ClassKey & C::classKey() { return staticFactory()->classKey();  }									\
	RegisterFactory<C> N;																				

//---------------------------------------------------------------------------------------------------

//! declare a normal factory.
#define IMPLEMENT_FACTORY( CLASS, BASE )																	\
	IMPLEMENT_NAMED_FACTORY( FACTORY_##CLASS, CLASS, BASE, BASE::staticFactory()->serializeKey() );			\
	IMPLEMENT_WIDGET_TYPES( CLASS );

//! declare this factory for types that don't need to serialize the instance key.
#define IMPLEMENT_NOKEY_FACTORY( CLASS, BASE )																\
	IMPLEMENT_NAMED_FACTORY( FACTORY_##CLASS, CLASS, BASE, false );											\
	IMPLEMENT_WIDGET_TYPES( CLASS );																		

//! declare a factory for a Resource based class.
#define IMPLEMENT_RESOURCE_FACTORY( CLASS, BASE )															\
	IMPLEMENT_NAMED_FACTORY( FACTORY_##CLASS, CLASS, BASE, BASE::staticFactory()->serializeKey() );			\
	IMPLEMENT_RESOURCE_WIDGET_TYPES( CLASS );																

// use this macro when you don't need full reflection information for this Widget object, less templates to compile since
// it does not declare IMPLEMENT_WIDGET_TYPES()
#define IMPLEMENT_LIGHT_FACTORY( CLASS, BASE )																\
	IMPLEMENT_NAMED_FACTORY( FACTORY_##CLASS, CLASS, BASE, BASE::staticFactory()->serializeKey() );

#define IMPLEMENT_ABSTRACT_FACTORY( CLASS, BASE )															\
	IMPLEMENT_NAMED_ABSTRACT_FACTORY( FACTORY_##CLASS, CLASS, BASE, BASE::staticFactory()->serializeKey() );\
	IMPLEMENT_ABSTRACT_WIDGET_TYPES( CLASS );															

#define IMPLEMENT_ABSTRACT_NOKEY_FACTORY( CLASS, BASE )														\
	IMPLEMENT_NAMED_ABSTRACT_FACTORY( FACTORY_##CLASS, CLASS, BASE, false );								\
	IMPLEMENT_ABSTRACT_WIDGET_TYPES( CLASS );																

//---------------------------------------------------------------------------------------------------

// Helper class to register a factory under another ID
class RegisterFactoryAlias 
{
public:
	RegisterFactoryAlias( const ClassKey & nKey, Factory * pFactory ) : m_nKey( nKey )
	{
		Factory::registerFactory( m_nKey, pFactory );
	}
	~RegisterFactoryAlias()
	{
		Factory::unregisterFactory( m_nKey );
	}
private:
	// Data
	ClassKey		m_nKey;
};

// macro to allow the user to register another key for a class factory...
#define REGISTER_FACTORY_KEY( CLASS, KEY )																	\
	RegisterFactoryAlias REG_##CLASS( KEY, CLASS::staticFactory() );
// macro to register a factory under a different name
#define REGISTER_FACTORY_ALIAS( CLASS, ALIAS )																\
	RegisterFactoryAlias REG_##CLASS##ALIAS( ClassKey( #ALIAS ), CLASS::staticFactory() );


#endif

//---------------------------------------------------------------------------------------------------
//EOF

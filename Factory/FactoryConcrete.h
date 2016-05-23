/*
	FactoryConcrete.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef FACTORY_CONCRETE_H
#define FACTORY_CONCRETE_H

#include "Debug/Trace.h"
#include "Reflection/TypeTemplate.h"
#include "Factory.h"

#include "MedusaDll.h"

//-------------------------------------------------------------------------------

template<class T>
class FactoryConcrete : public Factory
{
public:
	// Construction
	FactoryConcrete( const char * pName, Factory * a_pBase, bool a_bSerializeKey );

	// Factory interface
	virtual Widget *			createWidget();
	virtual Widget *			createWidget( const InStream &input );
	virtual bool				isAbstract();
};

//---------------------------------------------------------------------------------------------------

template<class T>
inline FactoryConcrete<T>::FactoryConcrete(const char * pName, Factory * a_pBase, bool a_bSerializeKey ) 
	: Factory( pName, a_pBase, a_bSerializeKey, TypeAbstract<T>::instance()->typeId() )
{}

template<class T>
inline Widget * FactoryConcrete<T>::createWidget() 
{
	T * pWidget = new T();
	return pWidget;
}

template<class T>
inline Widget * FactoryConcrete<T>::createWidget( const InStream &input ) 
{
	T * pWidget = new T();
	pWidget->read( input );

	return pWidget;
}

template<class T>
inline bool FactoryConcrete<T>::isAbstract() 
{
	return( false );
}


//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

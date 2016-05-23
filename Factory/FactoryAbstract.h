/*
	FactoryAbstract.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef FACTORY_ABSTRACT_H
#define FACTORY_ABSTRACT_H

#include "Factory.h"
#include "MedusaDll.h"

//-------------------------------------------------------------------------------

template<class T>
class FactoryAbstract : public Factory
{
public:
	// Construction
	FactoryAbstract( const char * pName, Factory * a_pBase, bool a_bSerializeKey );

	// Factory interface
	virtual Widget *			createWidget();
	virtual Widget *			createWidget( const InStream &input );
	virtual bool				isAbstract();
};

//-------------------------------------------------------------------------------

template<class T>
inline FactoryAbstract<T>::FactoryAbstract( const char * pName, Factory * a_pBase, bool a_bSerializeKey ) 
	: Factory( pName, a_pBase, a_bSerializeKey, TypeAbstract<T>::instance()->typeId() )
{}

template<class T>
inline Widget * FactoryAbstract<T>::createWidget() 
{
	return NULL;
}

template<class T>
inline Widget * FactoryAbstract<T>::createWidget( const InStream &input ) 
{
	return NULL;
}

template<class T>
inline bool FactoryAbstract<T>::isAbstract() 
{
	return true;
}

//-------------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF

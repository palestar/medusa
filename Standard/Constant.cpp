/*
	Constant.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/Constant.h"



//----------------------------------------------------------------------------

Array< Constant * >				Constant::s_Constants;
Tree< CharString, Constant * >	Constant::s_ConstantTree;

//----------------------------------------------------------------------------

Constant::Constant( const char * pName, float value ) : m_pName( pName ), m_Value( value )
{
	registerConstant( this );
}

Constant::~Constant()
{
	unregisterConstant( this );
}

//----------------------------------------------------------------------------

const char * Constant::name() const
{
	return m_pName;
}

float Constant::value() const
{
	return m_Value;
}

//----------------------------------------------------------------------------

Constant & Constant::operator=( float value )
{
	m_Value = value;
	return *this;
}

//----------------------------------------------------------------------------

Constant * Constant::find( const char * pName )
{
	Tree< CharString, Constant *>::Iterator it = s_ConstantTree.find( pName );
	if ( it.valid() )
		return *it;

	return NULL;
}

int Constant::constantCount()
{
	return s_Constants.size();
}

Constant * Constant::constant( int n )
{
	return s_Constants[ n ];
}

void Constant::registerConstant( Constant * pConstant )
{
	s_Constants.push( pConstant );
	s_ConstantTree[ pConstant->name() ] = pConstant;
}

void Constant::unregisterConstant( Constant * pConstant )
{
	s_Constants.removeSearchSwap( pConstant );
	s_ConstantTree.remove( pConstant->name() );
}

//----------------------------------------------------------------------------
//EOF

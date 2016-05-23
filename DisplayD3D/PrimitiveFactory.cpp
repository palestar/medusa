/*
	PrimitiveFactory.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Trace.h"
#include "Standard/Hash.h"
#include "PrimitiveFactory.h"


//---------------------------------------------------------------------------------------

typedef Hash<PrimitiveKey,BasePrimitiveFactory *>		PrimitiveFactoryHash;

PrimitiveFactoryHash & primitiveFactoryHash()
{
	static PrimitiveFactoryHash s_PrimitiveFactoryHash;
	return( s_PrimitiveFactoryHash );
}

//---------------------------------------------------------------------------------------

BasePrimitiveFactory::BasePrimitiveFactory( const PrimitiveKey & key ) : m_Key(key)
{
	PrimitiveFactoryHash & hash = primitiveFactoryHash();
	if ( hash.find( key ).valid() )
		throw KeyConflict();

	hash[ key ] = this;
}

BasePrimitiveFactory::~BasePrimitiveFactory()
{
	primitiveFactoryHash().remove( m_Key );
}

//----------------------------------------------------------------------------

BasePrimitiveFactory * BasePrimitiveFactory::findFactory( const PrimitiveKey &key )
{
	PrimitiveFactoryHash & hash = primitiveFactoryHash();

	PrimitiveFactoryHash::Iterator iFactory = hash.find( key );
	if ( iFactory.valid() )
		return *iFactory;

	return NULL;
}

//---------------------------------------------------------------------------------------
// EOF

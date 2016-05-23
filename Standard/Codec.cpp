/*
	Codec.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "Standard/Codec.h"



//----------------------------------------------------------------------------

Codec::Factory::Factory( const char * pName ) : m_sName( pName )
{
	Codec::factoryArray().push( this );
}

Codec::Factory::~Factory()
{
	Codec::factoryArray().removeSearch( this );
}

const char * Codec::Factory::name() const
{
	return m_sName;
}
//----------------------------------------------------------------------------

int Codec::factoryCount()
{
	return factoryArray().size();
}

Codec::FactoryArray & Codec::factoryArray()
{
	static FactoryArray CODEC_FACTORY_ARRAY;
	return CODEC_FACTORY_ARRAY;
}

Codec::Factory * Codec::factory( int n )
{
	return factoryArray()[ n ];
}

Codec * Codec::createNamed( const char * pName )
{
	for(int i=0;i<Codec::factoryCount();i++)
		if ( stricmp( pName, factory( i )->name() ) == 0 )
			return Codec::factory( i )->create();

	return NULL;
}

//---------------------------------------------------------------------------------------------------

Codec::FactoryTemplate<NoCodec>		NONE_FACTORY( "NONE" );

Codec * NoCodec::clone()
{
	return new NoCodec();
}

int NoCodec::encode( const void * pInput, int nInputBytes, 
					void * pOutput, int nOutputBytes, EncodeLevel nLevel )
{
	memcpy( pOutput, pInput, nInputBytes );
	return nInputBytes;
}

int	NoCodec::decode( const void * pInput, int nInputBytes, void * pOutput, int nOutputBytes )
{
	memcpy( pOutput, pInput, nInputBytes );
	return nInputBytes;
}

void NoCodec::reset()
{}

//----------------------------------------------------------------------------
//EOF

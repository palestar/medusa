/*
	ClassKey.cpp
	(c)2006 Palestar Inc, Richard Lyle
*/

#include "ClassKey.h"
#include "Factory.h"
#include "Standard/StringHash64.h"

//---------------------------------------------------------------------------------------------------

ClassKey::ClassKey( const char * pKey ) : UniqueNumber( (qword)0 )
{
	m_Id = StringHash64( pKey ).hash();
}

ClassKey::ClassKey( const CharString & key ) : UniqueNumber( (qword)0 )
{
	m_Id = StringHash64( key ).hash();
}

ClassKey::operator CharString() const
{
	Factory * pFactory = Factory::findFactory( *this );
	if ( pFactory != NULL )
		return pFactory->className();
	else
		return string();
}

//---------------------------------------------------------------------------------------------------
//EOF

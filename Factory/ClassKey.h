/*
	ClassKey.h

	Class Identifier
	(c)2005 Palestar, Richard Lyle
*/

#ifndef CLASS_KEY_H
#define CLASS_KEY_H

#include "Debug/Log.h"
#include "Standard/UniqueNumber.h"
#include "File/Stream.h"
#include "File/CompactNumber.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class DLL ClassKey : public UniqueNumber
{
public:
	ClassKey() : UniqueNumber( (qword)0 )
	{}
	ClassKey( const ClassKey & copy ) : UniqueNumber( copy )
	{}
	ClassKey( qword nid ) : UniqueNumber( nid )
	{}

	ClassKey( const char * pKey );
	ClassKey( const CharString & key );
	operator CharString() const;
};

//---------------------------------------------------------------------------------------------------

inline const InStream & operator>>( const InStream & input, ClassKey & n )
{
	Dictionary * pD = input.dictionary();
	if ( pD != NULL )
	{
		Dictionary::TypedStorage<ClassKey> * pStorage = pD->getStorage<ClassKey>();
		if ( pStorage != NULL )
		{
			CompactNumber id;
			input >> id;

			// get existing data from storage..
			ClassKey * pData = pStorage->findData( id.m_nNumber );
			if (! pData )
				throw File::FileError();

			n = *pData;
			return input;
		}
	}

	input >> (UniqueNumber &)n;
	return input;
}

inline const OutStream & operator<<( const OutStream & output, const ClassKey & n )
{
	Dictionary * pD = output.dictionary();
	if ( pD != NULL )
	{
		Dictionary::TypedStorage<ClassKey> * pStorage = pD->getStorage<ClassKey>();
		if ( pStorage != NULL )
		{
			CompactNumber id;
			if (! pStorage->findID( n, id.m_nNumber ) )
			{
				LOG_ERROR( "ClassKey", "Failed to find id %llu in dictionary!", (qword)n );
				throw File::FileError();		// failed to find a ID in our dictionary storage!
			}

			output << id;
			return output;
		}
	}

	output << (const UniqueNumber &)n;
	return output;
}

//---------------------------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF

/*
	WidgetKey.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef WIDGETKEY_H
#define WIDGETKEY_H

#include "Standard/UniqueNumber.h"
#include "File/Stream.h"
#include "MedusaDll.h"

//---------------------------------------------------------------------------------------------------

class DLL WidgetKey : public UniqueNumber
{
public:
	WidgetKey( bool a_bGenerateKey = true ) : UniqueNumber( a_bGenerateKey )
	{}
	WidgetKey( const WidgetKey & copy ) : UniqueNumber( copy )
	{}
	WidgetKey( const char * pKey ) : UniqueNumber( pKey )
	{}
	WidgetKey( qword nid ) : UniqueNumber( nid )
	{}

	WidgetKey & operator=( const WidgetKey & assign )
	{
		m_Id = assign.m_Id;
		return *this;
	}
};

IMPLEMENT_RAW_STREAMING( WidgetKey );

const WidgetKey NULL_WIDGET( (qword)0 );

//---------------------------------------------------------------------------------------------------

//inline const InStream & operator>>( const InStream & input, WidgetKey & n )
//{
//	Dictionary * pD = input.dictionary();
//	if ( pD != NULL )
//	{
//		Dictionary::TypedStorage<WidgetKey> * pStorage = (Dictionary::TypedStorage<WidgetKey> *)pD->findStorage( "WidgetKey" );
//		if ( pStorage != NULL )
//		{
//			CompactNumber id;
//			input >> id;
//
//			// get existing data from storage..
//			WidgetKey * pData = pStorage->findData( id.m_nNumber );
//			if (! pData )
//				throw File::FileError();
//
//			n = *pData;
//			return input;
//		}
//	}
//
//	input.read( &n, sizeof( n ) );	
//	return input;
//}
//
//inline const OutStream & operator<<( const OutStream & output, const WidgetKey & n )
//{
//	Dictionary * pD = output.dictionary();
//	if ( pD != NULL )
//	{
//		Dictionary::TypedStorage<WidgetKey> * pStorage = (Dictionary::TypedStorage<WidgetKey> *)pD->findStorage( "WidgetKey" );
//		if ( pStorage != NULL )
//		{
//			CompactNumber id;
//			if (! pStorage->findID( n, id.m_nNumber ) )
//				throw File::FileError();		// failed to find a ID in our dictionary storage!
//
//			output << id;
//			return output;
//		}
//	}
//
//	output.write( &n, sizeof( n ) );	
//	return output;
//}

#endif

//---------------------------------------------------------------------------------------------------
//EOF

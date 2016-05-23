/*
	WidgetWrap.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define FACTORY_DLL
#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Standard/UUE.h"
#include "File/FileMemoryBlocked.h"
#include "Factory/WidgetWrap.h"

#pragma warning(disable:4244) // warning C4244: 'initializing' : conversion from 'unsigned __int64' to 'int', possible loss of data

//-------------------------------------------------------------------------------

WidgetWrap::WidgetWrap() : m_Size( 0 ), m_pWidget( 0 ), m_nFilter( 0 )
{
	//TRACE( "WidgetWrap::WidgetWrap" ); 
}

WidgetWrap::WidgetWrap( const WidgetWrap & copy ) : m_Size( 0 ), m_pWidget( 0 ), m_nFilter( 0 )
{
	//TRACE( "WidgetWrap::WidgetWrap" );
	operator=( copy );
}

WidgetWrap::WidgetWrap( Widget * pWidget, dword nFilter, Dictionary * a_pDictionary ) : m_Size( 0 ), m_pWidget( 0 ), m_nFilter( 0 )
{
	wrap( pWidget, nFilter, a_pDictionary );
}

WidgetWrap::WidgetWrap( Widget * pWidget, dword nFilter ) : m_Size( 0 ), m_pWidget( 0 ), m_nFilter( 0 )
{
	//TRACE( "WidgetWrap::WidgetWrap" );
	wrap( pWidget, nFilter );
}

WidgetWrap::WidgetWrap( Widget * pWidget ) : m_Size( 0 ), m_pWidget( 0 ), m_nFilter( 0 )
{
	//TRACE( "WidgetWrap::WidgetWrap" );
	wrap( pWidget, FF_TRANSMIT );
}

WidgetWrap::~WidgetWrap()
{
	//TRACE( "WidgetWrap::~WidgetWrap" );
	release();
}

//----------------------------------------------------------------------------

Widget * WidgetWrap::unwrap( Widget * pWidget /*= NULL*/ ) const
{
	if ( m_pWidget != NULL && m_Size > 0 )
	{
		try {
			InStream input( new FileMemoryBlocked( m_pWidget, m_Size ), m_nFilter, SV_CURRENT, m_pDictionary );
			
			if ( (m_nFilter & FF_TRANSMIT) == 0 )
			{
				int nVersion;
				input >> nVersion;

				switch( nVersion )
				{
				case SV_20090104:
					input.setVersion( nVersion );
					break;
				default:
					input.setPosition( 0 );
					input.setVersion( 0 );
					break;
				}
			}
			else
			{
				// for transmitted widgets, we always assume the version is the latest..
				input.setVersion( SV_CURRENT );
			}

			input >> pWidget;
		}
		catch( File::FileError )
		{
			LOG_ERROR( "WidgetWrap", "Exception while unwrapping widget %p (%s)", 
				pWidget, pWidget != NULL ? pWidget->factory()->className() : "" );
			return NULL;
		}
	}

	return pWidget;
}

CharString WidgetWrap::uuencode() const
{
	// encode all binary data into a text string, very large objects may fail due to the string limits, so keep the sizes down
	return UUE( m_pWidget, m_Size ).encoded();
}

//----------------------------------------------------------------------------

bool WidgetWrap::uudecode( const char * pText )
{
	UUD uud;
	if ( uud.decode( pText ) )
	{
		// decoded, set the binary data, copy is true
		wrap( m_nFilter, m_pDictionary, uud.decodedBytes(), const_cast<byte *>( uud.decoded() ), true );
		return true;
	}

	// failed to decode
	return false;
}

void WidgetWrap::wrap( dword nFilter, Dictionary * a_pDictionary, int size, byte * pWidget, bool copy /*= false*/ )
{
	release();

	m_nFilter = nFilter;
	m_pDictionary = a_pDictionary;
	m_Size = size;

	if ( copy )
	{
		m_pWidget = new byte[ m_Size ];
		memcpy( m_pWidget, pWidget, m_Size );
	}
	else
		m_pWidget = pWidget;
}

void WidgetWrap::wrap( Widget * pWidget, dword nFilter, Dictionary * a_pDictionary )
{
	release();

	m_nFilter = nFilter;
	m_pDictionary = a_pDictionary;

	try {
		int nVersion = SV_CURRENT;
		// store widget in FileMemory object
		OutStream output( new FileMemoryBlocked(), nFilter, nVersion, m_pDictionary );
		if ( (nFilter & FF_TRANSMIT) == 0 )
			output << nVersion;
		output << pWidget;

		m_Size = output.file()->size();
		m_pWidget = new byte[ m_Size ];

		output.file()->rewind();
		output.file()->read( m_pWidget, m_Size );
	}
	catch( File::FileError )
	{
		LOG_ERROR( "WidgetWrap", "Failed to wrap widget (%s)", pWidget != NULL ? pWidget->className() : "NULL" );
	}
}

void WidgetWrap::wrap( Widget * pWidget, dword nFilter )
{
	wrap( pWidget, nFilter, m_pDictionary );
}

void WidgetWrap::wrap( Widget * pWidget )
{
	wrap( pWidget, FF_TRANSMIT );
}

void WidgetWrap::grab( WidgetWrap & grab )
{
	release();

	// take the size and pointer from the other object
	m_Size = grab.m_Size;
	m_nFilter = grab.m_nFilter;
	m_pDictionary = grab.m_pDictionary;
	m_pWidget = grab.m_pWidget;
	// set the pointers in the other object to NULL so it doesn't delete our objects
	grab.m_Size = 0;
	grab.m_pWidget = NULL;
}

void WidgetWrap::release()
{
	if ( m_pWidget != NULL )
	{
		delete [] m_pWidget;
		m_pWidget = NULL;
		m_Size = 0;
		m_nFilter = 0;
	}
}

WidgetWrap & WidgetWrap::operator=( const WidgetWrap & copy )
{
	release();

	m_Size = copy.m_Size;
	m_pDictionary = copy.m_pDictionary;

	if ( m_Size > 0 )
	{
		m_nFilter = copy.m_nFilter;
		m_pWidget = new byte[ m_Size ];
		memcpy( m_pWidget, copy.m_pWidget, m_Size );
	}

	return *this;
}

WidgetWrap & WidgetWrap::operator=( Widget * pWidget )
{
	wrap( pWidget );
	return *this;
}

//----------------------------------------------------------------------------
//EOF

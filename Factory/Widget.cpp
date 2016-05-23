/*
	Widget.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define MEDUSA_TRACE_ON			// needed by dumpWidgets()
#define FACTORY_DLL
#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Standard/StringBuffer.h"
#include "Factory/FactoryTypes.h"
#include "File/FileMemoryBlocked.h"

#include <stdio.h>

//---------------------------------------------------------------------------------------------------

bool Widget::sm_bEnableTracking = false;						// set to true to enable tracking of all widget objects

//----------------------------------------------------------------------------

AbstractFactoryTypes<Widget>	TYPES_Widget;
RegisterFactory<Widget>			FACTORY_Widget;

//---------------------------------------------------------------------------------------------------

Factory * Widget::staticFactory() { static FactoryAbstract<Widget> F( "Widget", NULL, true ); return &F; }
Factory * Widget::factory() const { return staticFactory(); }
const ClassKey & Widget::classKey() { return staticFactory()->classKey(); }	

//---------------------------------------------------------------------------------------------------

BEGIN_PROPERTY_LIST( Widget, PropertyClass );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

Widget::Widget() : m_Key( (qword)0 ), m_nVersion( 0 )
{}

Widget::~Widget()
{
	AutoLock lock( &widgetLock() );
	unregisterKey();

	// we should NEVER have a WidgetHandle for a Widget with an ID of 0..
	if ( m_pWidgetHandle.valid() )
		throw BadWidget();
}

//---------------------------------------------------------------------------------------------------

bool Widget::isType( const ClassKey & nClass ) const
{
	return factory()->isType( nClass );
}

//----------------------------------------------------------------------------

const int VERSION_20060416 = 20060416;
const int VERSION_20080517 = 20080517;

bool Widget::write( const OutStream & output )
{
	bool bCopy = (output.filter() & FF_COPY) != 0;
	bool bTransmit = (output.filter() & FF_TRANSMIT) != 0;
	bool bUpdate = (output.filter() & FF_UPDATE) != 0;
	bool bFilterObjects = (output.filter() & FF_NOOBJECTS) != 0;
	bool bText = (output.filter() & FF_TEXT) != 0;

	if ( bText )
	{
		output.writeLine( "{" );		// mark the start of this object

		PropertyList * pList = propertyList();
		for(int i=0;i<pList->propertyCount();++i)
		{
			Property * pProperty = pList->property( i );

			int nElements = pProperty->count( this );

			dword nFlags = pProperty->flags();
			if ( (nFlags & PF_NOSAVE) )
				continue;
			if ( bCopy && (nFlags & PF_NOCOPY) )
				continue;
			if ( bFilterObjects && (nFlags & PF_OBJECT) )
				continue;
			if ( bUpdate && (nFlags & PF_UPDATE) == 0 )
				continue;

			output.writeLine( "[" );

			output << pProperty->name();
			if ( pProperty->isDynamic() )
				output << nElements;

			for(int j=0;j<nElements;++j)
			{
				if (! pProperty->type()->write( output, pProperty->pointer( this, j ) ) )
				{
					TRACE( "Widget::write() - Failed to serialize property %s!", pProperty->name() );
					return false;
				}
			}
			output.writeLine( "]" );
		}

		output.writeLine( "}" );		// mark the end of an object with a empty string
	}
	else if (! bTransmit )
	{
		PropertyList * pList = propertyList();

		FileMemoryBlocked::Ref pFile = new FileMemoryBlocked();
		OutStream data( pFile, output.filter(), output.version() );

		data << VERSION_20080517;
		data << (dword)0;			// write out object size place holder..

		File::Position nObjectStart = pFile->position();
		for(int i=0;i<pList->propertyCount();++i)
		{
			Property * pProperty = pList->property( i );

			int nElements = pProperty->count( this );

			dword nFlags = pProperty->flags();
			if ( (nFlags & PF_NOSAVE) )
				continue;
			if ( bCopy && (nFlags & PF_NOCOPY) )
				continue;
			if ( bFilterObjects && (nFlags & PF_OBJECT) )
				continue;
			if ( bUpdate && (nFlags & PF_UPDATE) == 0 )
				continue;

			data << (dword)0;

			File::Position nPropertyStart = pFile->position();
			data << pProperty->nameId();
			data << nElements;
			for(int j=0;j<nElements;++j)
			{
				if (! pProperty->type()->write( data, pProperty->pointer( this, j ) ) )
				{
					TRACE( "Widget::write() - Failed to serialize property %s!", pProperty->name() );
					return false;
				}
			}

			dword nPropertySize = pFile->position() - nPropertyStart;
			pFile->setPosition( nPropertyStart - sizeof(dword) );
			data << nPropertySize;
			pFile->setPosition( nPropertyStart + nPropertySize );
		}

		dword nObjectSize = pFile->position() - nObjectStart;
		pFile->setPosition( nObjectStart - sizeof(dword) );
		data << nObjectSize;

		// pipe all data into the real file now...
		pFile->rewind();
		InStream input( pFile );
		output.pipe( input );
	}
	else
	{
		// transmitting this widget, send it as compact as possible...
		PropertyList * pList = propertyList();
		for(int i=0;i<pList->propertyCount();++i)
		{
			Property * pProperty = pList->property( i );
			dword nFlags = pProperty->flags();
			if ( !(nFlags & PF_TRANSMIT) )
				continue;
			if ( (nFlags & PF_NOSAVE) )
				continue;
			if ( bCopy && (nFlags & PF_NOCOPY) )
				continue;
			if ( bFilterObjects && (nFlags & PF_OBJECT) )
				continue;
			if ( bUpdate && (nFlags & PF_UPDATE) == 0 )
				continue;

			int nElements = pProperty->count( this );
			if ( pProperty->isDynamic() )
				output << nElements;

			for(int j=0;j<nElements;++j)
			{
				if (! pProperty->type()->write( output, pProperty->pointer( this, j ) ) )
				{
					TRACE( CharString().format( "Widget::write() - Failed to serialize property %s!", pProperty->name()) );
					return false;
				}
			}
		}
	}

	return true;
}

bool Widget::read( const InStream & input )
{
	bool bCopy = (input.filter() & FF_COPY) != 0;
	bool bTransmit = (input.filter() & FF_TRANSMIT) != 0;
	bool bFilterObjects = (input.filter() & FF_NOOBJECTS) != 0;
	bool bUpdate = (input.filter() & FF_UPDATE) != 0;
	bool bText = (input.filter() & FF_TEXT) != 0;

	if ( bText )
	{
		PropertyList * pList = propertyList();
		
		CharString sObjectStart;
		input >> sObjectStart;

		if ( sObjectStart != "{" )
		{
			LOG_ERROR( "Widget", "Text parsing error near byte %u.", input.file()->position() );
			return false;
		}

		while( true )
		{
			CharString sLine = input.readLine();		// read line without escaping any characters
			if ( sLine == "}" )
				break;

			if ( sLine == "[" )
			{
				// read in property name
				input >> sLine;

				int nProperty = pList->find( sLine );
				if ( nProperty < 0 )
				{
					// property not found, skip until we're out of this property..
					int nPropertyDepth = 1;
					while( nPropertyDepth > 0 )
					{
						sLine = input.readLine();	
						if ( sLine == "{" || sLine == "[" )
							++nPropertyDepth;
						else if ( sLine == "}" || sLine == "]" )
							--nPropertyDepth;
					}
					continue;
				}

				Property * pProperty = pList->property( nProperty );

				int nElements = 1;
				if ( pProperty->isDynamic() )
					input >> nElements;

				pProperty->allocate( this, nElements );
				for(int j=0;j<nElements;++j)
				{
					if (! pProperty->type()->read( input, pProperty->pointer( this, j ) ) )
					{
						LOG_ERROR( "Widget", "Failed to read property %s[%d] (%s)!", pProperty->name(), j, className() );
						return false;
					}
				}

				// we expect an ending ] 
				sLine = input.readLine();
				if ( sLine != "]" )
				{
					LOG_ERROR( "Widget", "Text parsing error near byte %u.", input.file()->position() );
					return false;
				}
			}
		}

	}
	else if (! bTransmit )
	{
		PropertyList * pList = propertyList();
		
		int nVersion;
		input >> nVersion;

		switch( nVersion )
		{
		case VERSION_20080517:
			{
				dword nObjectSize = 0;
				input >> nObjectSize;

				File::Position nObjectEnd = input.file()->position() + nObjectSize;
				while( input.file()->position() < nObjectEnd )
				{
					dword nPropertySize = 0;
					input >> nPropertySize;

					File::Position nPropertyEnd = input.file()->position() + nPropertySize;

					dword nNameId = 0;
					input >> nNameId;
					int nProperty = pList->findByNameId( nNameId );
					if ( nProperty < 0 )
					{
						input.file()->setPosition( nPropertyEnd );
						continue;
					}

					int nElements = 0;
					input >> nElements;

					Property * pProperty = pList->property( nProperty );

					pProperty->allocate( this, nElements );
					for(int j=0;j<nElements;++j)
					{
						if (! pProperty->type()->read( input, pProperty->pointer( this, j ) ) )
						{
							LOG_ERROR( "Widget", "Failed to read property %s[%d] (%s)!", pProperty->name(), j, className() );
							return false;
						}
					}
					// check the number of bytes that got read in...
					if ( input.file()->position() != nPropertyEnd )
					{
						LOG_ERROR( "Widget", "Property size mismatch on %s (%s)!", pProperty->name(), className() );
						return false;
					}
				}

				if ( input.file()->position() != nObjectEnd )
				{
					LOG_ERROR( "Widget", "Object size mismatch on object %s!", className() );
					return false;
				}
			}
			break;
		case VERSION_20060416:
			{
				int nCount;
				input >> nCount;

				for(int i=0;i<nCount;++i)
				{
					CharString sName;
					input >> sName;
					File::Size nBytes;
					input >> nBytes;

					// calculate the end of this properties data, so we can check after reading in this property
					File::Position nEndPosition = input.file()->position() + nBytes;

					int nProp = pList->find( sName );
					if ( nProp < 0 )
					{
						// property not found, skip the data...
						input.file()->setPosition( nEndPosition );
						continue;		
					}

					int nElements;
					input >> nElements;

					Property * pProperty = pList->property( nProp );

					pProperty->allocate( this, nElements );
					for(int j=0;j<nElements;++j)
					{
						if (! pProperty->type()->read( input, pProperty->pointer( this, j ) ) )
						{
							LOG_ERROR( "Widget", "Widget::read() - Failed to serialize property %s!", sName.cstr() );
							return false;
						}
					}

					// check the number of bytes that got read in...
					if ( input.file()->position() != nEndPosition )
					{
						LOG_ERROR( "Widget", "Widget::read() - Data input overrun on property %s!", sName.cstr() );
						return false;
					}
				}
			}
			break;
		default:
			// pre-reflection widget object, return failure so old serialization code may handle this object..
			input.file()->rewind( sizeof(int) );
			return false;
		}
	}
	else
	{
		// transmitting this widget, send it as compact as possible...
		PropertyList * pList = propertyList();
		for(int i=0;i<pList->propertyCount();i++)
		{
			Property * pProperty = pList->property( i );
			dword nFlags = pProperty->flags();
			if ( !(nFlags & PF_TRANSMIT) )
				continue;
			if ( nFlags & PF_NOSAVE )
				continue;
			if ( bCopy && (nFlags & PF_NOCOPY) )
				continue;
			if ( bFilterObjects && (nFlags & PF_OBJECT) )
				continue;
			if ( bUpdate && (nFlags & PF_UPDATE) == 0 )
				continue;

			int nElements = 0;
			if ( bUpdate )
			{
				nElements = pProperty->count( this );
				if ( pProperty->isDynamic() )
				{
					int nReceived = 0;
					input >> nReceived;
					if ( nReceived != nElements )
					{
						LOG_ERROR( "Widget", "Widget::read() - Element count mismatch, received %d but expected %d for property %s!",
							nReceived, nElements, pProperty->name() );
						return false;
					}
				}
			}
			else 
			{
				if ( pProperty->isDynamic() )
					input >> nElements;
				else
					nElements = pProperty->count( this );
			}

			// set the container size..
			pProperty->allocate( this, nElements );
			for(int j=0;j<nElements;j++)
			{
				if (! pProperty->type()->read( input, pProperty->pointer( this, j ) ) )
				{
					LOG_ERROR( "Widget", "Widget::read() - Failed to serialize property %s!", pProperty->name() );
					return false;
				}
			}
		}
	}

	return true;
}

void Widget::setKey( const WidgetKey & key )
{
	AutoLock lock( &widgetLock() );
	if ( key != m_Key )
	{
		unregisterKey();
		m_Key = key;
		registerKey();
	}
}

Widget * Widget::copy()
{
	FileMemoryBlocked::Ref pFile = new FileMemoryBlocked();

	// serialize this object into a FileMemory object
	OutStream output( pFile, FF_COPY );
	output << this;
	pFile->rewind();

	// deserialize this object into a new object
	try {
		Widget * pCopy = NULL;
		InStream input( pFile, FF_COPY );
		input >> pCopy;

		return( pCopy );
	}
	catch( File::FileError )
	{}

	return NULL;
}

Widget * Widget::clone()
{
	FileMemoryBlocked::Ref pFile = new FileMemoryBlocked();

	// serialize this object into a FileMemory object
	OutStream output( pFile );
	output << this;
	pFile->rewind();

	// deserialize this object into a new object
	try {
		Widget * pCopy = NULL;
		InStream input( pFile );
		input >> pCopy;

		return( pCopy );
	}
	catch( File::FileError )
	{}

	return NULL;
}

//----------------------------------------------------------------------------

Widget::WidgetMap & Widget::widgetMap( const WidgetKey & key )
{
	static WidgetMap * pWidgetHash = new WidgetMap[ WIDGET_HASH_SIZE ];
	return pWidgetHash[ key.m_Id & WIDGET_HASH_MASK ];
}

CriticalSection & Widget::widgetLock()
{
	static CriticalSection * pWidgetLock = new CriticalSection();
	return *pWidgetLock;
}

Widget::Ref Widget::findWidget( const WidgetKey & key )
{
	AutoLock lock( &widgetLock() );

	try {
		WidgetMap & map = widgetMap( key );
		WidgetMap::iterator iWidget = map.find( key );
		if ( iWidget != map.end() )
			return iWidget->second;
	}
	catch( Referenced::AlreadyDestroyed )
	{}

	return NULL;
}

Widget::Ref Widget::findWidgetByType( const WidgetKey & key, const ClassKey & ck )
{
	AutoLock lock( &widgetLock() );

	try {
		WidgetMap & map = widgetMap( key );
		WidgetMap::iterator iWidget = map.find( key );
		if ( iWidget != map.end() )
			if ( iWidget->second->isType( ck ) )
				return iWidget->second;
	}
	catch( Referenced::AlreadyDestroyed )
	{}

	return NULL;
}

typedef std::map< CharString, int > ClassCountMap;

CharString Widget::dumpWidgets()
{
	AutoLock lock( &widgetLock() );

	CharString sDump;

	ClassCountMap classCount;
	sDump += "Active Widgets:\n";

	int nTotalWidgets = 0;
	for(int i=0;i<WIDGET_HASH_SIZE;++i)
	{
		WidgetMap & map = widgetMap( WidgetKey( (qword)i ) );
		nTotalWidgets += map.size();

		for( WidgetMap::iterator iWidget = map.begin(); 
			iWidget != map.end(); ++iWidget )
		{
			Widget * pWidget = iWidget->second;
			if (! pWidget || !pWidget->factory() )
				continue;

			classCount[ pWidget->factory()->className() ]++;
			if ( pWidget->isTrackingEnabled() )
			{
				sDump += CharString().format( "Widget %p (%s) Tracking:\n", pWidget, pWidget->factory()->className() );
				pWidget->dumpTracking( sDump );
			}
		}
	}

	sDump += CharString().format("%d widgets allocated, %d classes...\n", nTotalWidgets, classCount.size());
	lock.release();
	
	for( ClassCountMap::iterator iClass = classCount.begin(); iClass != classCount.end(); ++iClass )
		sDump += CharString().format( "%s;%d\n", (const char *)iClass->first, iClass->second );

	return sDump;
}

void Widget::dumpWidgetsTrace()
{
	TRACE( dumpWidgets() );
}

//---------------------------------------------------------------------------------------------------

void Widget::registerKey()
{
	// widgetLock() should be locked before calling this function!
	if ( m_Key != NULL_WIDGET )
	{
		WidgetMap & map = widgetMap( m_Key );

		// check if we are replacing an existing widget, if so take it's WidgetHandle object
		// so they will point to our new Widget object.
		WidgetMap::iterator iReplace = map.find( m_Key );
		if ( iReplace != map.end() )
		{
			Widget * pReplace = iReplace->second;
			if ( pReplace != NULL && pReplace != this && pReplace->m_pWidgetHandle.valid() )
			{
				if ( m_pWidgetHandle.valid() )
					throw BadWidget();					// this should never happen, since we can't have a WidgetHandle until we have a key..

				m_pWidgetHandle = pReplace->m_pWidgetHandle;
				pReplace->m_pWidgetHandle = NULL;
				m_pWidgetHandle->m_pWidget = this;		// point all WidgetReference<> pointers at this object!
			}
		}

		map[ m_Key ] = this;
	}
}

void Widget::unregisterKey()
{
	// widgetLock() should be locked before calling this function!
	if ( m_Key != NULL_WIDGET )
	{
		WidgetMap & map = widgetMap( m_Key );

		WidgetMap::iterator iWidget = map.find( m_Key );
		if ( iWidget != map.end() && iWidget->second == this )
			map.erase( iWidget );

		if ( m_pWidgetHandle != NULL )
		{
			m_pWidgetHandle->m_pWidget = NULL;
			m_pWidgetHandle = NULL;
		}
	}
}


//----------------------------------------------------------------------------
// EOF

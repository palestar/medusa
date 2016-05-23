/*
	Factory.cpp

	Factory class, creates Widget objects
	(c)2005 Palestar, Richard Lyle
*/

// NOTE: The lock is disabled because it was causing lots of overhead with WidgetCast<>
#define ENABLE_FACTORY_LOCK				0		// defined to non-zero to lock when accessing the factory hash

#include "Debug/Assert.h"
#include "Debug/Trace.h"
#include "Debug/ExceptionHandler.h"
#include "Standard/STLHelper.h"

#include "Factory.h"
#include "Widget.h"

//----------------------------------------------------------------------------

Factory::Factory( const char * a_pName, Factory * a_pBase, bool a_bSerializeKey, int a_nTypeId ) 
	: m_pName( a_pName ), m_nKey( a_pName ), m_pBase( a_pBase ), m_bSerializeKey( a_bSerializeKey ), m_nTypeId( a_nTypeId )
{
#if ENABLE_FACTORY_LOCK
	AutoLock lock( &GetFactoryLock() );
#endif

	FactoryHash::iterator i = GetFactoryHash().find( m_nKey );
    if ( i != GetFactoryHash().end() )
    {
        LOG_CRITICAL( "Factory", "Factory key conflict %s -> %s", i->second->className(), m_pName );
        throw FactoryAlreadyExists();
    }

	LOG_DEBUG_MED( "Factory", "Factory for %s (Type: %s, Key: %s) registered", 
		a_pName, m_pBase != NULL ? m_pBase->className() : "[BASE]", m_nKey.string().cstr() );

	GetFactoryHash()[ m_nKey ] = this;
	GetTypeIdHash()[ m_nTypeId ] = this;
	if ( m_pBase != NULL )
		m_pBase->m_Children.push_back( this );
}

Factory::~Factory()
{
#if ENABLE_FACTORY_LOCK
	AutoLock lock( &GetFactoryLock() );
#endif

	LOG_DEBUG_MED( "Factory", "Factory %s unregistered", m_pName );

	GetFactoryHash().erase( m_nKey );

	// remove ourselves from our child factories..
	if ( m_pBase != NULL )
	{
		if (! removeSwap( m_pBase->m_Children, this ) )
			LOG_ERROR( "Factory", "Failed to remove %s as child of %s", m_pName, m_pBase->m_pName );
	}
	for(size_t i=0;i<m_Children.size();++i)
		m_Children[i]->m_pBase = NULL;
}

//-------------------------------------------------------------------------------

bool Factory::loadWidget( const InStream &input, Widget * & pWidget, const ClassKey & type, bool bLoadInPlace )
{
	WidgetKey nKey( (qword)0 );
	bool bSetKey = false;

	if ( bLoadInPlace && pWidget == NULL )
	{
		LOG_WARNING( "Factory", "Load in place can't load into NULL Widget object." );
		return false;
	}

	dword nFilter = input.filter();
	if ( (nFilter & FF_UPDATE) == 0 )
	{
		// get the key of the class we're creating
		ClassKey nClass;
		input >> nClass;
		
		if ( nClass == NULL_WIDGET )
		{
			if ( bLoadInPlace )
			{
				LOG_WARNING( "Factory", "Load in place used with NULL class key." );
				return false;
			}
			pWidget = NULL;
			return true;
		}

		Factory * pFactory = Factory::findFactory( nClass );
		if (! pFactory )
		{
			LOG_ERROR( "Factory", "Failed to find factory for %s!", nClass.string().cstr() );
			return false;
		}
		if (! pFactory->isType( type ) )
		{
			LOG_ERROR( "Factory", "Class factory for %s is not type %s.", pFactory->className(), type.string().cstr() );
			return false;
		}
		if ( pWidget != NULL && pWidget->factory() != pFactory )
		{
			//LOG_WARNING( "Factory", "Factory of Widget %s != %s", pWidget->factory()->className(), pFactory->className() );
			if ( bLoadInPlace )
				return false;
			pWidget = NULL;		// force the creation of a new object using the correct factory..
		}

		int nVersion = input.version();
		if ( (pFactory->serializeKey() || nVersion < SV_20090104) && (nFilter & FF_COPY) == 0 )
			input >> nKey;

		if (! bLoadInPlace )
		{
			// try to find an existing instance with the correct key and the correct type..
			if ( (nFilter & FF_SHARED) != 0 && nKey.m_Id != 0 )
				pWidget = Widget::findWidgetByType( nKey, type );

			// if no widget found or we are forcing creation
			if ( pWidget == NULL || (pWidget != NULL && pWidget->key( false ) != nKey) )
			{
				pWidget = pFactory->createWidget();
				if (! pWidget )
				{
					LOG_ERROR( "Factory", "Factory (%s) returned NULL!", pFactory->className() );
					return false;
				}

				// set the key AFTER we load the object, otherwise another thread may grab a pointer to our 
				// Widget before it's ready to be used.
				if ( nKey.m_Id != 0 )
					bSetKey = true;		
			}
		}
		else
		{
			// load into Widget that is already allocated .. so just set the key if needed.
			if ( nKey.m_Id != 0 )
				bSetKey = true;
		}
	}

	// Read the properties of the object into the Widget..
	if ( pWidget != NULL && !pWidget->read( input ) )
	{
		LOG_ERROR( "Factory", "Failed to read widget %s", pWidget->factory()->className() );
		return false;
	}

	// Lastly, set the key of the loaded object if needed.
	if ( bSetKey )
		pWidget->setKey( nKey );

	return true;
}

bool Factory::saveWidget( const OutStream & output, Widget * pWidget )
{
	if ( pWidget != NULL )
	{
		if ( (output.filter() & FF_UPDATE) == 0 )
		{
			try {
				output << pWidget->factory()->classKey();
			}
			catch( File::FileError )
			{
				LOG_ERROR( "ClassKey", "Failed to save class key for widget %s", pWidget->factory()->className() );
				throw;		// continue processing the exception..
			}

			if ( (pWidget->factory()->serializeKey() || output.version() < SV_20090104) && (output.filter() & FF_COPY) == 0 )
				output << pWidget->key();
		}

		if (! pWidget->write( output ) )
		{
			LOG_ERROR( "Factory", "Failed to write widget %s", pWidget->factory()->className() );
			return false;
		}
	}
	else
	{
		output << NULL_CLASS;
	}

	return true;
}

Factory *Factory::findFactory( const ClassKey &key )
{
#if ENABLE_FACTORY_LOCK
	AutoLock lock( &GetFactoryLock() );
#endif

	FactoryHash::iterator iFactory = GetFactoryHash().find( key );
	if ( iFactory != GetFactoryHash().end() )
		return iFactory->second;

	return NULL;
}

Factory * Factory::findFactoryByName( const char * pName )
{
	return findFactory( ClassKey( pName ) );
}

Factory * Factory::findFactoryByTypeId( int nTypeId )
{
#if ENABLE_FACTORY_LOCK
	AutoLock lock( &GetFactoryLock() );
#endif

	TypeIdHash::iterator iFactory = GetTypeIdHash().find( nTypeId );
	if ( iFactory != GetTypeIdHash().end() )
		return iFactory->second;

	return NULL;
}

Widget * Factory::createWidget( const ClassKey &key )
{
	Factory * pFactory = findFactory( key );
	if ( pFactory != NULL )
	{
		// if the factory is abstract, then find the first concrete factory...
		if ( pFactory->isAbstract() )
		{
			for(int i=0;i< typeCount( key ); ++i)
			{
				Widget * pWidget = Factory::createWidget( type( key, i ) );
				if ( pWidget )
					return pWidget;
			}
		}
		else
		{
			return pFactory->createWidget();
		}
	}

	return NULL;
}

Widget * Factory::createNamedWidget( const char * pName )
{
	return Factory::createWidget( ClassKey( pName ) );
}

const char * Factory::className( const ClassKey &key )
{
	Factory * pFactory = findFactory( key );
	if ( pFactory != NULL )
		return pFactory->className();

	return "NULL";
}

const ClassKey & Factory::classType( const ClassKey & key )
{
	Factory * pFactory = findFactory( key );
	if ( pFactory != NULL )
		return pFactory->typeKey();

	return NULL_CLASS;
}

int Factory::typeCount( const ClassKey &key )
{
	Factory * pFactory = findFactory( key );
	if ( pFactory != NULL )
		return pFactory->childCount();

	return 0;
}

const ClassKey &Factory::type( const ClassKey &key, int n )
{
	Factory * pFactory = findFactory( key );
	if ( pFactory != NULL )
	{
		Factory * pChild = pFactory->child( n );
		if ( pChild != NULL )
			return pChild->classKey();
	}

	return NULL_CLASS;
}

bool Factory::isType( const ClassKey & nType, const ClassKey & nCheck )
{
	Factory * pFactory = findFactory( nCheck );
	while( pFactory != NULL )
	{
		if ( pFactory->classKey() == nType )
			return true;
		pFactory = pFactory->m_pBase;
	}

	return false;
}

//---------------------------------------------------------------------------------------------------

void Factory::registerFactory( const ClassKey & nKey, Factory * pFactory )
{
#if ENABLE_FACTORY_LOCK
	AutoLock lock( &GetFactoryLock() );
#endif

	FactoryHash::iterator i = GetFactoryHash().find( nKey );
    if ( i != GetFactoryHash().end() )
    {
        LOG_CRITICAL( "Factory", "Factory key conflict with %s", i->second->className() );
        throw FactoryAlreadyExists();
    }

	GetFactoryHash()[ nKey ] = pFactory;
}

void Factory::unregisterFactory( const ClassKey & nKey )
{
#if ENABLE_FACTORY_LOCK
	AutoLock lock( &GetFactoryLock() );
#endif
	GetFactoryHash().erase( nKey );
}

bool Factory::initializeClassDictionary( Dictionary * a_pDictionary )
{
#if ENABLE_FACTORY_LOCK
	AutoLock lock( &GetFactoryLock() );
#endif

	Dictionary::TypedStorage<ClassKey> * pStorage = a_pDictionary->getStorage<ClassKey>( true );
	if (! pStorage )
		return false;

	pStorage->clear();

	pStorage->addData( NULL_CLASS );
	for( FactoryHash::iterator i = GetFactoryHash().begin();
		i != GetFactoryHash().end(); ++i )
	{
		Factory * pFactory = i->second;
		if ( pFactory != NULL && !pFactory->isAbstract() )
		{
			//LOG_DEBUG_LOW( "Factory", "Adding class %s to dictionary.", pFactory->className() );
			pStorage->addData( i->first );
		}
	}

	LOG_STATUS( "Factory", "Initialized ClassKey dictionary, %d keys found.", pStorage->m_IDMap.size() );
	return true;
}


//---------------------------------------------------------------------------------------------------

Factory::FactoryHash &	Factory::GetFactoryHash()
{
	static FactoryHash * pHASH = new FactoryHash();
	return *pHASH;
}

CriticalSection & Factory::GetFactoryLock()
{
	static CriticalSection * pLOCK = new CriticalSection();
	return *pLOCK;
}

Factory::TypeIdHash & Factory::GetTypeIdHash()
{
	static TypeIdHash * pHASH = new TypeIdHash();
	return *pHASH;
}

//----------------------------------------------------------------------------------------
// EOF

/*
	PrimitiveFactory.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_FACTORY_H
#define PRIMITIVE_FACTORY_H

#include "Debug/Assert.h"
#include "DisplayD3D/DisplayDeviceD3D.h"

//---------------------------------------------------------------------------------------

class BasePrimitiveFactory 
{
public:
	// Types
	class KeyConflict {};

	// Construction
	BasePrimitiveFactory( const PrimitiveKey &key );
	virtual ~BasePrimitiveFactory();
	
	// Mutators
	virtual DevicePrimitive *		create() = 0;		// get primitive, creating a new one if needed or using a cached one
	virtual void					release() = 0;				// release all cached primitives

	// Static
	static BasePrimitiveFactory *	findFactory( const PrimitiveKey &key );

private:
	// Data
	PrimitiveKey					m_Key;
};

//---------------------------------------------------------------------------------------

template<class T> 
class PrimitiveFactory : public BasePrimitiveFactory
{
public:
	//----------------------------------------------------------------------------

	// This factory actually creates this object
	class PrimitiveProxy : public T		// create a proxy object
	{
	public:
		PrimitiveProxy( PrimitiveFactory * pFactory ) : m_pFactory( pFactory )
		{}

		//! DevicePrimitive interface
		void recache()
		{
			m_pFactory->m_Cache.insert( this );
			release();
		}
	private:
		PrimitiveFactory *	m_pFactory;
	};

	//----------------------------------------------------------------------------

	// Construction
	PrimitiveFactory() : BasePrimitiveFactory( T::staticPrimitiveKey() ) 
	{}
	virtual ~PrimitiveFactory()
	{
		release();
	}

	// Mutators
	DevicePrimitive * create()
	{
		// check for a cached primitive before creating a new primitive
		List< T * >::Iterator primitive = m_Cache.head();
		if ( primitive.valid() )
		{
			DevicePrimitive * pPrimitive = *primitive;
			ASSERT( pPrimitive );

			// remove primitive from cache list
			m_Cache.remove( primitive );
			return( pPrimitive );
		}

		// create a brand new primitive
		return( new PrimitiveProxy( this ) );
	}

	void release()
	{
		// delete all cached primitives
		List< T * >::Iterator primitive = m_Cache.head();
		while( primitive.valid() )
		{
			delete *primitive;
			primitive++;
		}
		// release the linked list
		m_Cache.release();
	}

private:
	// Data
	List< T * >		m_Cache;		// linked list of available primitives

	// Friends
	friend class PrimitiveProxy;
};

//---------------------------------------------------------------------------------------

#define IMPLEMENT_PRIMITIVE_FACTORY( thisClass )					\
	    static PrimitiveFactory<thisClass> g_##thisClass##Factory;

//---------------------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------------------
// EOF

/*
	DevicePrimitive.h
	(c)2005 Palestar, Richard Lyle
*/


#ifndef DEVICE_PRIMITIVE_H
#define DEVICE_PRIMITIVE_H

#include "Standard/Reference.h"
#include "Standard/UniqueNumber.h"
#include "Standard/StringHash64.h"

#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL PrimitiveKey : public UniqueNumber
{
public:
	// Construction
    PrimitiveKey() : UniqueNumber()									{}
	PrimitiveKey( const PrimitiveKey & key ) : UniqueNumber( key )	{}
	PrimitiveKey( const char * pKey ) : UniqueNumber( pKey )		{}
	PrimitiveKey( u64 key ) : UniqueNumber( key )					{}
};

//----------------------------------------------------------------------------

#define DECLARE_DEVICE_PRIMITIVE()							\
	static const PrimitiveKey & staticPrimitiveKey();		\
	virtual const PrimitiveKey & primitiveKey();

#define IMPLEMENT_DEVICE_PRIMITIVE( className )				\
	const PrimitiveKey & className::staticPrimitiveKey()	\
	{														\
		static PrimitiveKey KEY( StringHash64( #className ) );	\
		return( KEY );										\
	}														\
	const PrimitiveKey & className::primitiveKey()			\
	{														\
		return staticPrimitiveKey();						\
	};

//----------------------------------------------------------------------------

class DisplayDevice;	// forward declare

class DLL DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<DevicePrimitive>		Ref;

	//! Construction
	DevicePrimitive();
	virtual ~DevicePrimitive();

	// Accessors
	DisplayDevice *		device() const;

	// Mutators
	bool				grabReference( dword nTrackingId = 0 );
	void				releaseReference( dword nTrackingId = 0 );
	void				setDevice( DisplayDevice * pDevice );

	// Interface
	virtual bool		execute() = 0;					// execute primitive
	virtual void		clear() = 0;					// clear primitive
	virtual void		release() = 0;					// release the primitive

	// when the last reference is released, this is invoked to recache this primitive for reuse
	virtual void		recache() {};

	// Helpers
	static void			push( DisplayDevice * pDevice, DevicePrimitive * pPrimitive );

	DisplayDevice *		m_pDevice;
	volatile int		m_nReferenceCount;
};

//----------------------------------------------------------------------------

inline DevicePrimitive::DevicePrimitive() : 
	m_pDevice( NULL ), m_nReferenceCount( 1 )
{}

inline DevicePrimitive::~DevicePrimitive()
{}

inline DisplayDevice * DevicePrimitive::device() const
{
	return m_pDevice;
}

inline bool DevicePrimitive::grabReference( dword nTrackingId /*= 0*/ )
{
	Atomic::increment( &m_nReferenceCount );
	return true;
}

inline void DevicePrimitive::releaseReference( dword nTrackingId /*= 0*/ )
{
	if ( Atomic::decrement( &m_nReferenceCount ) == 1 )
		recache();
}

inline void DevicePrimitive::setDevice( DisplayDevice * pDevice )
{
	m_pDevice = pDevice;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF


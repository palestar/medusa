/*
	PrimitiveLineList.h
	This primitive renders each set of two vertices as an isolated line

	1--------2  3           6
	             \        /
				  \     /
				   4   5

	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_LINE_LIST_H
#define PRIMITIVE_LINE_LIST_H

#include "Math/Vector3.h"
#include "Display/DisplayDevice.h"
#include "Display/Types.h"
#include "MedusaDll.h"



//----------------------------------------------------------------------------

class DLL PrimitiveLineList : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveLineList>	Ref;

	// Mutators
	virtual bool				initialize( int lineCount ) = 0;
	virtual bool				initialize( int lineCount, const Line * pLines ) = 0;

	virtual Line *				lock() = 0;
	virtual void				unlock() = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int lineCount )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( lineCount );

		return(pPrimitive);
	}
	static Ref					create( DisplayDevice * pDevice,
									int lineCount, const Line * pLines )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( lineCount, pLines );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, int lineCount, const Line * pLines )
	{
		pDevice->push( create( pDevice, lineCount, pLines ) );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveLineListL : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveLineListL>	Ref;

	// Mutators
	virtual bool				initialize( int lineCount ) = 0;
	virtual bool				initialize( int lineCount, const LineL * pLines ) = 0;

	virtual LineL *				lock() = 0;
	virtual void				unlock() = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int lineCount )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( lineCount );

		return(pPrimitive);
	}
	static Ref					create( DisplayDevice * pDevice,
									int lineCount, const LineL * pLines )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( lineCount, pLines );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, int lineCount, const LineL * pLines )
	{
		pDevice->push( create( pDevice, lineCount, pLines ) );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveLineListDTL : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveLineListDTL>	Ref;

	// Mutators
	virtual bool				initialize( int lineCount ) = 0;
	virtual bool				initialize( int lineCount, const LineTL * pLines ) = 0;

	virtual LineTL *			lock() = 0;
	virtual void				unlock() = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int lineCount )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize(lineCount );

		return(pPrimitive);
	}
	static Ref					create( DisplayDevice * pDevice,
									int lineCount, const LineTL * pLines )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( lineCount, pLines );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, int lineCount, const LineTL * pLines )
	{
		pDevice->push( create( pDevice, lineCount, pLines ) );
	}
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

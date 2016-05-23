/*
	PrimitiveTriangleStrip.h
	This primitive renders a triangle strip..

     1---------3---------5 
      \       / \       /
	   \     /   \     /
	    \   /     \   /
		 \ /       \ /
		  2---------4
		  
	(1,2,3), (2,3,4), (3,4,5)

	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_TRIANGLE_STRIP_H
#define PRIMITIVE_TRIANGLE_STRIP_H

#include "Math/Vector3.h"
#include "Display/DisplayDevice.h"
#include "Display/Types.h"
#include "MedusaDll.h"



//----------------------------------------------------------------------------

class DLL PrimitiveTriangleStrip : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleStrip>	Ref;

	// Mutators
	virtual bool				initialize( int vertexCount, const Vertex * pVerticies ) = 0;
	// Helpers
	static Ref					create( DisplayDevice * pDevice,
									int vertexCount, const Vertex * pVerticies )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( vertexCount, pVerticies );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleStripL : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleStripL>	Ref;

	// Mutators
	virtual bool				initialize( int vertexCount, const VertexL * pVerticies ) = 0;
	// Helpers
	static Ref					create( DisplayDevice * pDevice,
									int vertexCount, const VertexL * pVerticies )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( vertexCount, pVerticies );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleStripDL : public DevicePrimitive		// dynamic lit triangle strip
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleStripDL>	Ref;

	// Mutators
	virtual bool				initialize( int vertexCount ) = 0;
	virtual bool				initialize( int vertexCount, const VertexL * pVerticies ) = 0;
	virtual VertexL *			lock() = 0;
	virtual void				unlock() = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice,
									int vertexCount, const VertexL * pVerticies )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( vertexCount, pVerticies );

		return(pPrimitive);
	}
	static Ref					create( DisplayDevice * pDevice, int vertexCount )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( vertexCount );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleStripDTL : public DevicePrimitive		// dynamic transformed lit triangle fan
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleStripDTL>	Ref;

	// Mutators
	virtual bool				initialize( int vertexCount ) = 0;
	virtual bool				initialize( int vertexCount, const VertexTL * pVerticies ) = 0;
	virtual VertexTL *			lock() = 0;
	virtual void				unlock() = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice,
									int vertexCount, const VertexTL * pVerticies )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( vertexCount, pVerticies );

		return(pPrimitive);
	}
	static Ref					create( DisplayDevice * pDevice, int vertexCount )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( vertexCount );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}
	static void					push( DisplayDevice * pDevice, 
									int vertexCount, const VertexTL * pVerts )
	{
		pDevice->push( create( pDevice, vertexCount, pVerts ) );
	}
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

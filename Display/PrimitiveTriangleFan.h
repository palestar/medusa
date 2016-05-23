/*
	PrimitiveTriangleFan.h
	This primitive renders a triangle fan, the first vertex being the anchor for the remaining verticies

			   5--------1---------2
			   \	   / \       /
				\	  /	  \     /
				 \	 /	   \   /
				  \	/	    \ /
                   4---------3
						 
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_TRIANGLE_FAN_H
#define PRIMITIVE_TRIANGLE_FAN_H

#include "Math/Vector3.h"
#include "Display/DisplayDevice.h"
#include "Display/Types.h"
#include "MedusaDll.h"



//----------------------------------------------------------------------------

class DLL PrimitiveTriangleFan : public DevicePrimitive		// triangle fan
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleFan>	Ref;

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

class DLL PrimitiveTriangleFanD : public DevicePrimitive		// dynamic triangle fan
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleFanD>	Ref;

	// Mutators
	virtual bool				initialize( int vertexCount ) = 0;
	virtual bool				initialize( int vertexCount, const Vertex * pVerticies ) = 0;

	virtual Vertex *			lock() = 0;
	virtual void				unlock() = 0;

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
									int vertexCount, const Vertex * pVerts )
	{
		pDevice->push( create( pDevice, vertexCount, pVerts ) );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleFanL : public DevicePrimitive		// lit triangle fan
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleFanL>	Ref;

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

class DLL PrimitiveTriangleFanDL : public DevicePrimitive		// dynamic lit triangle fan
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleFanDL>	Ref;

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
	static void					push( DisplayDevice * pDevice, 
									int vertexCount, const VertexL * pVerts )
	{
		pDevice->push( create( pDevice, vertexCount, pVerts ) );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleFanDTL : public DevicePrimitive		// dynamic transformed lit triangle fan
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleFanDTL>	Ref;

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

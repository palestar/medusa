/*
	PrimitiveTriangleList.h
	This primitive renders each group of 3 verticies as a isolated triangle

				1              4
			   / \            / \
			  /   \          /   \
			 /     \        /     \
			3-------2      6-------5 
		
	(c)2005 Palestar, Richard Lyle
*/

/*
	NOTE: When you initialize this primitive, the 'dynamic' bool controls whether the primitive will optimize
	the triangles internally or not. If you don't expect to touch the triangle list again after creation this
	boolean should be false. If you expect to use 'triangleBuffer()' to grab and modify the triangles on the fly
	then pass true so that the primitive doesn't optimize the triangle list
*/

#ifndef PRIMITIVE_TRIANGLE_LIST_H
#define PRIMITIVE_TRIANGLE_LIST_H

#include "Math/Vector3.h"
#include "Display/DisplayDevice.h"
#include "Display/Types.h"
#include "MedusaDll.h"



//----------------------------------------------------------------------------

class DLL PrimitiveTriangleList : public DevicePrimitive		// triangle list, static untransformed and unlit
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleList>	Ref;

	// Mutators
	virtual bool				initialize( int triangleCount, const Triangle * pTriangles ) = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int triangleCount, 
									const Triangle * pTriangles )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( triangleCount, pTriangles );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}

};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleListD : public DevicePrimitive		// triangle list, dynamic untransformed and unlit
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleListD>	Ref;

	// Mutators
	virtual bool				initialize( int triangleCount ) = 0;
	virtual bool				initialize( int triangleCount, const Triangle * pTriangles ) = 0;
	virtual Triangle *			lock() = 0;
	virtual void				unlock() = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int triangleCount )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( triangleCount );

		return(pPrimitive);
	}
	static Ref					create( DisplayDevice * pDevice, int triangleCount, 
									const Triangle * pTriangles )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( triangleCount, pTriangles );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, int triangleCount, 
									const Triangle * pTriangles )
	{
		pDevice->push( create( pDevice, triangleCount, pTriangles ) );
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}

};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleListI : public DevicePrimitive		// indexed triangle list, faster when drawing triangles that share verticies
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleListI>	Ref;

	// Mutators
	virtual bool				initialize( int triangleCount, const Triangle * pTriangles ) = 0;
	virtual bool				initialize( int triangleCount, const word * pTriangles,
									int vertexCount, const Vertex * pVerticies ) = 0;
	virtual bool				initialize( int triangleCount, const word * pTriangles,
									PrimitiveTriangleListI * pVerts ) = 0;				// share verticies from another primitive

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int triangleCount, 
									const Triangle * pTriangles )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( triangleCount, pTriangles );

		return(pPrimitive);
	}
	static Ref					create( DisplayDevice * pDevice, int triangleCount, const word * pTriangles,
									int vertexCount, const Vertex * pVerticies )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( triangleCount, pTriangles, vertexCount, pVerticies );

		return(pPrimitive);
	}
	static Ref					create( DisplayDevice * pDevice, int triangleCount, const word * pTriangles,
									PrimitiveTriangleListI * pVerts )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( triangleCount, pTriangles, pVerts );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}

};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleListL : public DevicePrimitive	// triangle list, lit verticies
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleListL>	Ref;

	// Mutators
	virtual bool				initialize( int triangleCount, const TriangleL * pTriangles ) = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int triangleCount, 
									const TriangleL * pTriangles )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );
		// setup the primitive
		pPrimitive->initialize( triangleCount, pTriangles );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}

};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleListDL : public DevicePrimitive		// triangle list, dynamic untransformed and lit
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleListDL>	Ref;

	// Mutators
	virtual bool				initialize( int triangleCount ) = 0;
	virtual bool				initialize( int triangleCount, const TriangleL * pTriangles ) = 0;
	virtual TriangleL *			lock() = 0;
	virtual void				unlock() = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int triangleCount )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( triangleCount );

		return(pPrimitive);
	}
	static Ref					create( DisplayDevice * pDevice, int triangleCount, 
									const TriangleL * pTriangles )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( triangleCount, pTriangles );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, int triangleCount, 
									const TriangleL * pTriangles )
	{
		pDevice->push( create( pDevice, triangleCount, pTriangles ) );
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleListTL : public DevicePrimitive		// Triangle List, transformed and lit
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleListTL>	Ref;

	// Mutators
	virtual bool				initialize( int triangleCount, const TriangleTL * pTriangles ) = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int triangleCount, 
									const TriangleTL * pTriangles )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );
		// setup the primitive
		pPrimitive->initialize( triangleCount, pTriangles );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}

};

//----------------------------------------------------------------------------

class DLL PrimitiveTriangleListDTL : public DevicePrimitive		// Triangle list, dynamic transformed and lit verticies
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveTriangleListDTL>	Ref;

	// Mutators
	virtual bool				initialize( int triangleCount ) = 0;
	virtual bool				initialize( int triangleCount, const TriangleTL * pTriangles ) = 0;

	virtual TriangleTL *		lock() = 0;
	virtual void				unlock() = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int triangleCount )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( triangleCount );

		return(pPrimitive);
	}
	static Ref					create( DisplayDevice * pDevice, int triangleCount, 
									const TriangleTL * pTriangles )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( triangleCount, pTriangles );

		return(pPrimitive);
	}
	static void					push( DisplayDevice * pDevice, int triangleCount, 
									const TriangleTL * pTriangles )
	{
		pDevice->push( create( pDevice, triangleCount, pTriangles ) );
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}

};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF


/*
	PrimitiveLineStrip.h
	This primitive renders a series of verticies as a connected line

	1-------2           /5
			 \        /
			  \3-----4

	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_LINE_STRIP_H
#define PRIMITIVE_LINE_STRIP_H

#include "Math/Vector3.h"
#include "Display/DisplayDevice.h"
#include "Display/Types.h"
#include "MedusaDll.h"



//----------------------------------------------------------------------------

class DLL PrimitiveLineStrip : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveLineStrip>	Ref;

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
	static void					push( DisplayDevice * pDevice, int vertexCount, const Vertex * pVerticies )
	{
		pDevice->push( create( pDevice, vertexCount, pVerticies ) );
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveLineStripL : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveLineStripL>	Ref;

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
	static void					push( DisplayDevice * pDevice, int vertexCount, const VertexL * pVerticies )
	{
		pDevice->push( create( pDevice, vertexCount, pVerticies ) );
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveLineStripDL : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveLineStripDL>	Ref;

	// Mutators
	virtual bool				initialize( int vertexCount ) = 0;
	virtual bool				initialize( int vertexCount, const VertexL * pVerticies ) = 0;

	virtual VertexL *			lock() = 0;
	virtual void				unlock() = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int vertexCount )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize( vertexCount );

		return(pPrimitive);
	}
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
	static void					push( DisplayDevice * pDevice, int vertexCount, const VertexL * pVerticies )
	{
		pDevice->push( create( pDevice, vertexCount, pVerticies ) );
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveLineStripTL : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveLineStripTL>	Ref;

	// Mutators
	virtual bool				initialize( int vertexCount, const VertexTL * pVerticies ) = 0;

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
	static void					push( DisplayDevice * pDevice, int vertexCount, const VertexTL * pVerticies )
	{
		pDevice->push( create( pDevice, vertexCount, pVerticies ) );
	}
	static void					push( DisplayDevice * pDevice, Ref primitive )
	{
		pDevice->push( primitive );
	}
};

//----------------------------------------------------------------------------

class DLL PrimitiveLineStripDTL : public DevicePrimitive
{
public:
	DECLARE_DEVICE_PRIMITIVE();

	// Types
	typedef Reference<PrimitiveLineStripDTL>	Ref;

	// Mutators
	virtual bool				initialize( int vertexCount ) = 0;
	virtual bool				initialize( int vertexCount, const VertexTL * pVerticies ) = 0;

	virtual VertexTL *			lock() = 0;
	virtual void				unlock() = 0;

	// Helpers
	static Ref					create( DisplayDevice * pDevice, int vertexCount )
	{
		// create the primitive
		Ref pPrimitive;
		pDevice->create( pPrimitive );

		// setup the primitive
		pPrimitive->initialize(vertexCount );

		return(pPrimitive);
	}
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
	static void					push( DisplayDevice * pDevice, int vertexCount, const VertexTL * pVerticies )
	{
		pDevice->push( create( pDevice, vertexCount, pVerticies ) );
	}
	static void					push( DisplayDevice * pDevice, const RectInt & rect, Color diffuse, float z = 0.0f)
	{
		Ref primitive;
		pDevice->create( primitive );

		// initialize the primitive
		primitive->initialize( 5 );

		// setup the verticies
		VertexTL * pVertex = primitive->lock();
		pVertex[0] = VertexTL( Vector3( rect.m_Left, rect.m_Top, z ), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[1] = VertexTL( Vector3( rect.m_Right, rect.m_Top, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[2] = VertexTL( Vector3( rect.m_Right, rect.m_Bottom, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[3] = VertexTL( Vector3( rect.m_Left, rect.m_Bottom, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[4] = pVertex[0];
		primitive->unlock();

		pDevice->push( primitive );
	}
	static void					createPushRecticle( DisplayDevice * pDevice, const RectInt & rect, Color diffuse1, Color diffuse2, float z = 0.0f )
	{
		Ref primitive;
		pDevice->create( primitive );

		primitive->initialize( 9 );

		// get the rectangle center
		PointInt center( rect.center() );
		// setup the verticies
		VertexTL * pVertex = primitive->lock();
		pVertex[0] = VertexTL( Vector3( rect.left, rect.top, z ), 1.0f, diffuse1, 0.0f, 0.0f );
		pVertex[1] = VertexTL( Vector3( center.x, rect.top, z), 1.0f, diffuse2, 0.0f, 0.0f );
		pVertex[2] = VertexTL( Vector3( rect.right, rect.top, z), 1.0f, diffuse1, 0.0f, 0.0f );
		pVertex[3] = VertexTL( Vector3( rect.right, center.y, z), 1.0f, diffuse2, 0.0f, 0.0f );
		pVertex[4] = VertexTL( Vector3( rect.right, rect.bottom, z), 1.0f, diffuse1, 0.0f, 0.0f );
		pVertex[5] = VertexTL( Vector3( center.x, rect.bottom, z), 1.0f, diffuse2, 0.0f, 0.0f );
		pVertex[6] = VertexTL( Vector3( rect.left, rect.bottom, z), 1.0f, diffuse1, 0.0f, 0.0f );
		pVertex[7] = VertexTL( Vector3( rect.left, center.y, z), 1.0f, diffuse2, 0.0f, 0.0f );
		pVertex[8] = pVertex[0];
		primitive->unlock();

		pDevice->push( primitive );
	}
	static void					createPushTriangle( DisplayDevice * pDevice, const PointInt & center, int size, Color diffuse, float z = 0.0f )
	{
		Ref primitive;
		pDevice->create( primitive );

		// initialize the primitive
		primitive->initialize( 4 );
		VertexTL * pVertex = primitive->lock();
		pVertex[0] = VertexTL( Vector3( center.x, center.y - size, z ), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[1] = VertexTL( Vector3( center.x + size, center.y + size, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[2] = VertexTL( Vector3( center.x - size, center.y + size, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[3] = pVertex[0];
		primitive->unlock();
		
		pDevice->push( primitive );
	}
	static void					createPushDiamond( DisplayDevice * pDevice, const PointInt & center, int size, Color diffuse, float z = 0.0f )
	{
		Ref primitive;
		pDevice->create( primitive );

		// initialize the primitive
		primitive->initialize( 5 );
		VertexTL * pVertex = primitive->lock();
		pVertex[0] = VertexTL( Vector3( center.x, center.y - size, z), 1.0f, diffuse, 0, 0 );
		pVertex[1] = VertexTL( Vector3( center.x + size, center.y, z), 1.0f, diffuse, 0, 0 );
		pVertex[2] = VertexTL( Vector3( center.x, center.y + size, z), 1.0f, diffuse, 0, 0 );
		pVertex[3] = VertexTL( Vector3( center.x - size, center.y, z), 1.0f, diffuse, 0, 0 );
		pVertex[4] = pVertex[0];
		primitive->unlock();
		
		pDevice->push( primitive );
	}
	static void					createPushRightArrow( DisplayDevice * pDevice, const PointInt & center, int size, Color diffuse, float z = 0.0f )
	{
		Ref primitive;
		pDevice->create( primitive );

		// initialize the primitive
		primitive->initialize( 4 );
		VertexTL * pVertex = primitive->lock();
		pVertex[0] = VertexTL( Vector3( center.x + size, center.y, z ), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[1] = VertexTL( Vector3( center.x - size, center.y + size, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[2] = VertexTL( Vector3( center.x - size, center.y - size, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[3] = pVertex[0];
		primitive->unlock();
		
		pDevice->push( primitive );
	}
	static void					createPushLeftArrow( DisplayDevice * pDevice, const PointInt & center, int size, Color diffuse, float z = 0.0f )
	{
		Ref primitive;
		pDevice->create( primitive );

		// initialize the primitive
		primitive->initialize( 4 );
		VertexTL * pVertex = primitive->lock();
		pVertex[0] = VertexTL( Vector3( center.x - size, center.y, z ), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[1] = VertexTL( Vector3( center.x + size, center.y + size, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[2] = VertexTL( Vector3( center.x + size, center.y - size, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[3] = pVertex[0];
		primitive->unlock();
		
		pDevice->push( primitive );
	}
	static void					createPushUpArrow( DisplayDevice * pDevice, const PointInt & center, int size, Color diffuse, float z = 0.0f )
	{
		Ref primitive;
		pDevice->create( primitive );

		// initialize the primitive
		primitive->initialize( 4 );
		VertexTL * pVertex = primitive->lock();
		pVertex[0] = VertexTL( Vector3( center.x, center.y - size, z ), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[1] = VertexTL( Vector3( center.x + size, center.y + size, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[2] = VertexTL( Vector3( center.x - size, center.y + size, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[3] = pVertex[0];
		primitive->unlock();
		
		pDevice->push( primitive );
	}
	static void					createPushDownArrow( DisplayDevice * pDevice, const PointInt & center, int size, Color diffuse, float z = 0.0f )
	{
		Ref primitive;
		pDevice->create( primitive );

		// initialize the primitive
		primitive->initialize( 4 );
		VertexTL * pVertex = primitive->lock();
		pVertex[0] = VertexTL( Vector3( center.x, center.y + size, z ), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[1] = VertexTL( Vector3( center.x + size, center.y - size, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[2] = VertexTL( Vector3( center.x - size, center.y - size, z), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[3] = pVertex[0];
		primitive->unlock();
		
		pDevice->push( primitive );
	}
	static void					createPushY( DisplayDevice * pDevice, const PointInt & src, const PointInt & dst, Color diffuse )
	{
		Ref primitive;
		pDevice->create( primitive );

		// initialize the primitive
		primitive->initialize( 3 );

		// setup the verticies
		VertexTL * pVertex = primitive->lock();
		pVertex[0] = VertexTL( Vector3( src.m_X, src.m_Y, 0.0f ), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[1] = VertexTL( Vector3( src.m_X, dst.m_Y, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[2] = VertexTL( Vector3( dst.m_X, dst.m_Y, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		primitive->unlock();

		pDevice->push( primitive );
	}
	static void					createPushX( DisplayDevice * pDevice, const PointInt & src, const PointInt & dst, Color diffuse )
	{
		Ref primitive;
		pDevice->create( primitive );

		// initialize the primitive
		primitive->initialize( 3 );

		// setup the verticies
		VertexTL * pVertex = primitive->lock();
		pVertex[0] = VertexTL( Vector3( src.m_X, src.m_Y, 0.0f ), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[1] = VertexTL( Vector3( dst.m_X, src.m_Y, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[2] = VertexTL( Vector3( dst.m_X, dst.m_Y, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		primitive->unlock();

		pDevice->push( primitive );
	}
	static void					pushConnector( DisplayDevice * pDevice, const PointInt & src, const PointInt & dst, Color diffuse, int size = 4 )
	{
		Ref primitive;
		pDevice->create( primitive );

		// initialize the primitive
		primitive->initialize( 12 );

		int horzSize = Clamp( abs(src.x - dst.x), 0, 16 );

		VertexTL * pVertex = primitive->lock();
		if ( dst.x > src.x )
		{
			horzSize = -horzSize;
			size = -size;			// flip it around
		}

		pVertex[0] = VertexTL( Vector3( src.x - size, src.y, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[1] = VertexTL( Vector3( src.x, src.y + size, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[2] = VertexTL( Vector3( src.x + size, src.y, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[3] = VertexTL( Vector3( src.x, src.y - size, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[4] = pVertex[ 0 ];
		pVertex[5] = VertexTL( Vector3( src.x - horzSize, src.y, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );

		pVertex[6] = VertexTL( Vector3( dst.x + horzSize, dst.y, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[7] = VertexTL( Vector3( dst.x + size, dst.y, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[8] = VertexTL( Vector3( dst.x, dst.y + size, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[9] = VertexTL( Vector3( dst.x - size, dst.y, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[10] = VertexTL( Vector3( dst.x, dst.y - size, 0.0f), 1.0f, diffuse, 0.0f, 0.0f );
		pVertex[11] = pVertex[ 7 ];

		primitive->unlock();

		pDevice->push( primitive );
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

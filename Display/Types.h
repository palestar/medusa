/*	
	Types.h
	(c)2005 Palestar, RIchard Lyle
*/

#ifndef DISPLAY_TYPES_H
#define DISPLAY_TYPES_H

#include "Math/Vector3.h"
#include "File/Stream.h"

//----------------------------------------------------------------------------

struct Vertex		// untransformed and unlit vertex
{
	Vertex() 
	{}
	Vertex( const Vector3 &p, const Vector3 &n, float uu, float vv) 
		: position(p), normal(n), u(uu), v(vv) 
	{}

	Vector3		position;		// position in model space
	Vector3		normal;			// normal of the vertex
	float		u,v;			// texture coordinates
};

struct VertexL		// untransformed and lit vertex
{
	VertexL() 
	{}
	VertexL( const Vector3 &p, const Vector3 &n, Color d, float uu, float vv) 
		: position(p), normal(n), diffuse(d), u(uu), v(vv) 
	{}

	Vector3		position;		// position in model space
	Vector3		normal;			// normal of the vertex
	Color		diffuse;		// diffuse color
	float		u,v;			// texture coordinates
};


struct VertexTL		// transformed and lit vertex
{
	VertexTL() 
	{}
	VertexTL( const Vector3 & p, float ww, Color d, float uu, float vv ) 
		: position( p ), w( ww ), diffuse( d ), u( uu ), v( vv ) 
	{}

	Vector3		position;		// position in screen coordinates
	float		w;				// 1 divided by the distance from the origin to the object along the z-axis
	Color		diffuse;		// vertex color
	float		u,v;			// texture uv's
};

//----------------------------------------------------------------------------

struct Triangle
{
	Triangle()
	{}
	Triangle( const Vertex &c1, const Vertex &c2, const Vertex &c3 ) 
	{
		v[0] = c1;
		v[1] = c2;
		v[2] = c3;
	}

	Vertex		v[ 3 ];
};

struct TriangleL
{
	TriangleL()
	{}
	TriangleL( const VertexL &c1, const VertexL &c2, const VertexL &c3 ) 
	{
		v[0] = c1;
		v[1] = c2;
		v[2] = c3;
	}

	VertexL		v[ 3 ];
};

struct TriangleTL
{
	TriangleTL()
	{}
	TriangleTL( const VertexTL &c1, const VertexTL &c2, const VertexTL &c3 ) 
	{
		v[0] = c1;
		v[1] = c2;
		v[2] = c3;
	}

	VertexTL		v[ 3 ];
};

//----------------------------------------------------------------------------

struct LineL
{
	LineL()
	{}
	LineL( const VertexL &c1, const VertexL &c2 ) 
	{
		v[0] = c1;
		v[1] = c2;
	}

	VertexL		v[ 2 ];
};

struct LineTL
{
	LineTL()
	{}
	LineTL( const VertexTL &c1, const VertexTL &c2 ) 
	{
		v[0] = c1;
		v[1] = c2;
	}

	VertexTL	v[ 2 ];
};

struct Line
{
	Line()
	{}
	Line( const Vertex &c1, const Vertex &c2 ) 
	{
		v[0] = c1;
		v[1] = c2;
	}

	Vertex		v[ 2 ];
};

//----------------------------------------------------------------------------

struct Sprite
{
	Sprite() 
	{}
	Sprite( const Vector3 &p, float s, Color d ) 
		: position(p), size(s), diffuse(d)
	{}

	Vector3		position;		// sprite position
	float		size;			// sprite size
	Color		diffuse;		// vertex color
};

//----------------------------------------------------------------------------

IMPLEMENT_RAW_STREAMING( Vertex );
IMPLEMENT_RAW_STREAMING( VertexL );
IMPLEMENT_RAW_STREAMING( VertexTL );
IMPLEMENT_RAW_STREAMING( Triangle );
IMPLEMENT_RAW_STREAMING( TriangleL );
IMPLEMENT_RAW_STREAMING( TriangleTL );
IMPLEMENT_RAW_STREAMING( LineL );
IMPLEMENT_RAW_STREAMING( LineTL );
IMPLEMENT_RAW_STREAMING( Line );
IMPLEMENT_RAW_STREAMING( Sprite );

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF


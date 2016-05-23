/*
	Stripper.h

	This class is used to convert a collection of triangles into triangle strips, used by NodeComplexMesh
	(c)2005 Palestar, Richard Lyle
*/

#ifndef STRIPPER_H
#define STRIPPER_H

#include "Standard/Array.h"
#include "Math/Vector3.h"
#include "Display/DisplayDevice.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL Stripper 
{
public:
	// Types
	struct Connection
	{
		int			vertex;			// connected vertex index
		int			edge;			// edge index
	};
	struct Vertex
	{
		Vector3		position;		// vertex position
		Vector3		normal;			// vertex normal
		float		u,v;			// texture coord

		Array< Connection >		
					connections;	// connections to other verticies

		bool		operator==( const Vertex & compare );
		Vertex &	operator=( const Vertex & copy );
	};
	struct Edge
	{
		Array< int >
					triangles;		// triangles using this edge

		Edge &		operator=( const Edge & copy );
	};
	struct Triangle
	{
		int			verts[ 3 ];		// vertex indicies
		int			next;			// next triangle in strip
		int			strip;			// which strip was triangle added too, -1 if none yet

		Array< int >	
					connections;	// adjacent triangles

		Triangle &	operator=( const Triangle & copy );
	};
	struct Strip
	{
		Array< int >	verts;		// vertex index that make up this triangle strip

		Strip &		operator=( const Strip & copy );
	};

	// Accessors
	int					vertexCount() const;
	const Vertex &		vertex( int n ) const;
	int					triangleCount() const;
	const Triangle &	triangle( int n ) const;
	int					stripCount() const;
	const Strip &		strip( int n ) const;

	// Mutators
	int					addVertex( const Vertex & vertex, bool cull = false );
	int					addTriangle( int v1, int v2, int v3 );
	void				buildStrips();	
	void				release();
	void				releaseTriangles();						// release the triangles, keep the verts

	// Helpers
	void				createDevicePrimitives( DisplayDevice * pDisplay, 
							Array< DevicePrimitive::Ref > & primitives, 
							int minTrianglesPerStrip ) const;
private:
	// Data
	Array< Vertex >		m_Verts;
	Array< Edge >		m_Edges;
	Array< Triangle >	m_Triangles;
	
	Array< Strip >		m_Strips;

	// Mutators
	void				addEdge( int t, int v1, int v2);		// add edge for triangle t, with verticies v1 and v2
	int					notSharedVertex(int	t1, int	t2);		// get the vertex that isn't shared between two triangles
	int					nextVertex( int t, int v );			
	int					firstSharedVertex(int t1, int t2);		// get the first shared vertex between two triangles
	int					secondSharedVertex(int t1, int t2);		// get the second shared vertex
	bool				triangleUsesVertex(int t, int v);		// does triangle t use vertex v

	void				followStrip( int last, int stripIndex );
	bool				getNextStrip();							// returns false if all triangles have been converted to strips
};

//----------------------------------------------------------------------------

inline int Stripper::vertexCount() const
{
	return m_Verts.size();
}

inline const Stripper::Vertex & Stripper::vertex( int n ) const
{
	return m_Verts[ n ];
}

inline int Stripper::triangleCount() const
{
	return m_Triangles.size();
}

inline const Stripper::Triangle & Stripper::triangle( int n ) const
{
	return m_Triangles[ n ];
}

inline int Stripper::stripCount() const
{
	return m_Strips.size();
}

inline const Stripper::Strip & Stripper::strip( int n ) const
{
	return m_Strips[ n ];
}

//----------------------------------------------------------------------------

#undef EPS
#define EPS 1e-04f

inline bool Stripper::Vertex::operator==( const Stripper::Vertex & compare )
{
	return( (position - compare.position).magnitude2() < EPS &&
		(normal - compare.normal).magnitude2() < EPS &&
		fabs( u - compare.u) < EPS && fabs( v - compare.v ) < EPS );
}

inline Stripper::Vertex & Stripper::Vertex::operator=( const Stripper::Vertex & copy )
{
	position = copy.position;
	normal = copy.normal;
	u = copy.u;
	v = copy.v;
	connections = copy.connections;
	return *this;
}

inline Stripper::Edge &	Stripper::Edge::operator=( const Stripper::Edge & copy )
{
	triangles = copy.triangles;
	return *this;
}

inline Stripper::Triangle & Stripper::Triangle::operator=( const Stripper::Triangle & copy )
{
	verts[0] = copy.verts[0];
	verts[1] = copy.verts[1];
	verts[2] = copy.verts[2];
	next = copy.next;
	strip = copy.strip;
	connections = copy.connections;
	return *this;
}

inline Stripper::Strip & Stripper::Strip::operator=( const Stripper::Strip & copy )
{
	verts = copy.verts;
	return *this;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

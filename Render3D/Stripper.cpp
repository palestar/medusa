/*
	Stripper.cpp

	This class is used to convert a triangle mesh into triangle strips
	(c)2005 Palestar, Richard Lyle
*/

#define RENDER3D_DLL
#include "Debug/Assert.h"
#include "Display/PrimitiveMaterial.h"
#include "Display/PrimitiveTriangleStrip.h"
#include "Display/PrimitiveTriangleList.h"
#include "Render3D/Stripper.h"

#define SHOW_STRIPS

//----------------------------------------------------------------------------

int Stripper::addVertex( const Vertex & vertex, bool cull /*= false*/ )
{
	if ( cull )
	{
		for(int i=0;i<m_Verts.size();i++)
			if ( m_Verts[i] == vertex )
				return i;

		// matching vertex not found, add vertex
	}

	int vertexIndex = m_Verts.size();

	// add the new vertex
	m_Verts.push( vertex );
	return vertexIndex;		// return the vertex index
}

int Stripper::addTriangle( int v1, int v2, int v3 )
{
	int t = m_Triangles.size();

	Triangle & newTriangle = m_Triangles.push();
	newTriangle.verts[0] = v1;
	newTriangle.verts[1] = v2;
	newTriangle.verts[2] = v3;
	newTriangle.next = -1;
	newTriangle.strip = -1;

	// setup connectivity information
	addEdge( t, v1, v2 );
	addEdge( t, v2, v3 );
	addEdge( t, v3, v1 );

	// return the triangle index
	return t;
}

void Stripper::buildStrips()
{
	// build the strips
	while( getNextStrip() );
	
	LOG_STATUS( "Stripper", "Stripper::buildStrips, triangles = %d, strips = %d, triangles per strip = %.1f", 
		m_Triangles.size(), m_Strips.size(), float(m_Triangles.size()) / m_Strips.size() );
}

void Stripper::release()
{
	m_Verts.release();
	m_Edges.release();
	m_Triangles.release();
	m_Strips.release();
}

void Stripper::releaseTriangles()
{
	m_Edges.release();
	m_Triangles.release();
	m_Strips.release();
	
	for(int i=0;i<m_Verts.size();i++)
		m_Verts[i].connections.release();
}

void Stripper::createDevicePrimitives( DisplayDevice * pDisplay, 
									  Array< DevicePrimitive::Ref > & primitives, 
									  int minTrianglesPerStrip ) const
{
	/*
	// create the device primitives
	Array< int > orphanedTriangles;
	for(int i=0;i<m_Strips.size();i++)
	{
		const Strip & strip = m_Strips[ i ];
		if ( (strip.verts.size() - 2) >= minTrianglesPerStrip )
		{

#ifdef SHOW_STRIPS
			// generate a random color and push a material of that color
			Color color( 128, rand() % 255,0, 255 );
			primitives.push( PrimitiveMaterial::create( pDisplay, color, color, color, color, 0.0f, PrimitiveMaterial::NONE ).pointer() );
#endif
			PrimitiveTriangleStrip::Ref triangleStrip = PrimitiveTriangleStrip::create( pDisplay, strip.verts.size() );
			primitives.push( triangleStrip.pointer() );

			Vertex * dstVertex = triangleStrip->lock();
			for(int v=0;v<strip.verts.size();v++)
			{
				const Vertex & srcVertex = m_Verts[ strip.verts[v] ];

				*dstVertex = Vertex( srcVertex.position,
					srcVertex.normal, srcVertex.u, srcVertex.v );
				dstVertex++;
			}
			triangleStrip->unlock();
		}
		else
		{
			// add triangles belonging to strip i to the triangle list
			for(int j=0;j<m_Triangles.size();j++)
				if ( m_Triangles[j].strip == i )
					orphanedTriangles.push( j );
		}
	}

	// create a triangle list for all orphaned triangles
	if ( orphanedTriangles.size() > 0 )
	{
#ifdef SHOW_STRIPS
		Color color( 0,0,128,255 );
		primitives.push( PrimitiveMaterial::create( pDisplay, color, color, color, color, 0.0f, PrimitiveMaterial::NONE ).pointer() );
#endif

		PrimitiveTriangleList::Ref triangleList = PrimitiveTriangleList::create( pDisplay, orphanedTriangles.size() );
		primitives.push( triangleList.pointer() );

		Vertex * dstVertex = (Vertex *)triangleList->lock();
		for(int k=0;k<orphanedTriangles.size();k++)
		{
			const Triangle & srcTriangle = m_Triangles[ orphanedTriangles[k] ];
			for(int v=0;v<3;v++)
			{
				const Vertex & srcVertex = m_Verts[ srcTriangle.verts[v] ];

				*dstVertex = Vertex( srcVertex.position, 
					srcVertex.normal, srcVertex.u, srcVertex.v );
				dstVertex++;
			}
		}
		triangleList->unlock();
	}
	*/
}

//----------------------------------------------------------------------------

void Stripper::addEdge( int t, int v1, int v2)
{
	Triangle & triangle = m_Triangles[ t ];

	// search for an existing edge
	for(int i=0;i<m_Verts[ v1 ].connections.size();i++)
		if ( m_Verts[ v1 ].connections[i].vertex == v2 )
		{
			// found existing edge
			Edge & edge = m_Edges[ m_Verts[ v1 ].connections[i].edge ];

			// enumerate all the other triangles on this edge
			for(int j=0;j<edge.triangles.size();j++)
			{
				m_Triangles[ edge.triangles[j] ].connections.push( t );
				triangle.connections.push( edge.triangles[j] );
			}

			// add this triangle to the list of triangles using this edge
			edge.triangles.push( t );
			// updated the edge and triangles, returning
			return;
		}

	// create a new edge
	int edgeIndex = m_Edges.size();
	Edge & newEdge = m_Edges.push();
	newEdge.triangles.push( t );

	Connection newConnection;
	newConnection.edge = edgeIndex;

	// connect the two verticies
	newConnection.vertex = v2;
	m_Verts[ v1 ].connections.push( newConnection );
	newConnection.vertex = v1;
	m_Verts[ v2 ].connections.push( newConnection );

	// done
}

int Stripper::notSharedVertex(int t1, int t2)
{
    if((m_Triangles[t1].verts[0] != m_Triangles[t2].verts[0])  &&
        (m_Triangles[t1].verts[0] != m_Triangles[t2].verts[1])   &&
        (m_Triangles[t1].verts[0] != m_Triangles[t2].verts[2]))
    {
        return(m_Triangles[t1].verts[0]);
    }
    if((m_Triangles[t1].verts[1] != m_Triangles[t2].verts[0])  &&
        (m_Triangles[t1].verts[1] != m_Triangles[t2].verts[1])   &&
        (m_Triangles[t1].verts[1] != m_Triangles[t2].verts[2]))
    {
        return(m_Triangles[t1].verts[1]);
    }

	return(m_Triangles[t1].verts[2]);
}

int Stripper::nextVertex( int t, int v )
{
	if ( m_Triangles[t].verts[0] == v )
		return m_Triangles[t].verts[1];
	if ( m_Triangles[t].verts[1] == v )
		return m_Triangles[t].verts[2];
	
	ASSERT(m_Triangles[t].verts[2] == v );
	return m_Triangles[t].verts[0];
}

int Stripper::firstSharedVertex(int t1, int t2)
{
    if((m_Triangles[t1].verts[0] == m_Triangles[t2].verts[0])  ||
        (m_Triangles[t1].verts[0] == m_Triangles[t2].verts[1])   ||
        (m_Triangles[t1].verts[0] == m_Triangles[t2].verts[2]))
    {
        return(m_Triangles[t1].verts[0]);
    }
    if((m_Triangles[t1].verts[1] == m_Triangles[t2].verts[0])  ||
        (m_Triangles[t1].verts[1] == m_Triangles[t2].verts[1])   ||
        (m_Triangles[t1].verts[1] == m_Triangles[t2].verts[2]))
    {
        return(m_Triangles[t1].verts[1]);
    }

    // firstSharedVertex: only one shared? 
	ASSERT( 0 );
	return( m_Triangles[t1].verts[2] );
}

int Stripper::secondSharedVertex(int t1, int t2)
{
    if((m_Triangles[t1].verts[2] == m_Triangles[t2].verts[0])  ||
        (m_Triangles[t1].verts[2] == m_Triangles[t2].verts[1])   ||
        (m_Triangles[t1].verts[2] == m_Triangles[t2].verts[2]))
    {
        return(m_Triangles[t1].verts[2]);
    }
    if((m_Triangles[t1].verts[1] == m_Triangles[t2].verts[0])  ||
        (m_Triangles[t1].verts[1] == m_Triangles[t2].verts[1])   ||
        (m_Triangles[t1].verts[1] == m_Triangles[t2].verts[2]))
    {
        return(m_Triangles[t1].verts[1]);
    }

    // firstSharedVertex: only one shared? 
	ASSERT( 0 );
	return( m_Triangles[t1].verts[0] );
}

bool Stripper::triangleUsesVertex(int t, int v)
{
	return( m_Triangles[t].verts[0] == v || m_Triangles[t].verts[1] == v || m_Triangles[t].verts[2] == v );
}

void Stripper::followStrip( int last, int stripIndex )
{
	Strip & strip = m_Strips[ stripIndex ];

	// continue finding more triangles
	while( true )
	{
		int next = -1;
		if ( strip.verts.size() == 0 )
		{
			// for second triangle, just pick one, TODO: use a better method of picking the next triangle
			Triangle & triangle = m_Triangles[ last ];
			triangle.strip = stripIndex;

			// find next triangle to add
			int min = -1;
			for(int i=0;i<triangle.connections.size();i++)
				if ( m_Triangles[ triangle.connections[i] ].strip < 0 
					&& m_Triangles[ triangle.connections[i] ].connections.size() > min )
				{
					next = triangle.connections[i];
					min = m_Triangles[ next ].connections.size();
				}

			if ( next < 0 )
			{
				// didn't find another triangle, create this strip with this single triangle
				strip.verts.push( triangle.verts[0] );
				strip.verts.push( triangle.verts[1] );
				strip.verts.push( triangle.verts[2] );
			}
			else
			{
				int v1 = notSharedVertex( last, next );
				int v2 = nextVertex( last, v1 );
				int v3 = nextVertex( last, v2 );

				strip.verts.push( v1 );
				strip.verts.push( v2 );
				strip.verts.push( v3 );
			}
		}
		else
		{
			Triangle & triangle = m_Triangles[ last ];

			int min = -1;
			for(int i=0;i<triangle.connections.size();i++)
				if ( m_Triangles[ triangle.connections[i] ].strip < 0 &&
					m_Triangles[ triangle.connections[i] ].connections.size() > min && 
					triangleUsesVertex( triangle.connections[i], strip.verts[ strip.verts.size() - 2 ]) &&
					triangleUsesVertex( triangle.connections[i], strip.verts[ strip.verts.size() - 1 ]) )
				{
					next = triangle.connections[i];
					min = m_Triangles[ next ].connections.size();
				}
		}

		if ( next < 0 )
			break;

		// push the next vertex
		strip.verts.push( notSharedVertex( next, last ) );
		// assign the triangle to this strip
		m_Triangles[ next ].strip = stripIndex;
		// find next triangle
		last = next;
	}
}

bool Stripper::getNextStrip()
{
	// find the least connected triangle to begin the strip
	int minConnections = INT_MAX;
	int minTriangle = -1;

	for(int i=0;i<m_Triangles.size();i++)
		if ( m_Triangles[i].strip < 0 && m_Triangles[i].connections.size() < minConnections )
		{
			minTriangle = i;
			minConnections = m_Triangles[i].connections.size();
		}

	if ( minTriangle < 0 )
		return false;		// no more triangles, all have been added to strips

	// create a new strip
	int stripIndex = m_Strips.size();
	m_Strips.push();

	// enumerate the triangles of this strip beginning with minTriangle
	followStrip( minTriangle, stripIndex );

	// strip done
	return true;
}

/*
Result of two passes with followStrip..

Trace:NodeComplexMesh::optimize(), culled 2132 verts, merged 18 triangles
Trace:NodeComplexMesh::optimize(), culled 0 verts, merged 0 triangles
Trace:Stripper::buildStrips, triangles = 86, strips = 13, triangles per strip = 6.6
Trace:Stripper::buildStrips, triangles = 20, strips = 7, triangles per strip = 2.9
Trace:Stripper::buildStrips, triangles = 24, strips = 7, triangles per strip = 3.4
Trace:Stripper::buildStrips, triangles = 24, strips = 7, triangles per strip = 3.4
Trace:Stripper::buildStrips, triangles = 48, strips = 15, triangles per strip = 3.2
Trace:Stripper::buildStrips, triangles = 4, strips = 1, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 4, strips = 2, triangles per strip = 2.0
Trace:Stripper::buildStrips, triangles = 4, strips = 2, triangles per strip = 2.0
Trace:Stripper::buildStrips, triangles = 4, strips = 1, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 50, strips = 18, triangles per strip = 2.8
Trace:Stripper::buildStrips, triangles = 50, strips = 22, triangles per strip = 2.3
Trace:Stripper::buildStrips, triangles = 138, strips = 40, triangles per strip = 3.5
Trace:Stripper::buildStrips, triangles = 138, strips = 39, triangles per strip = 3.5
Trace:Stripper::buildStrips, triangles = 198, strips = 49, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 86, strips = 13, triangles per strip = 6.6


Result of single pass with followStrip

Trace:NodeComplexMesh::optimize(), culled 2132 verts, merged 18 triangles
Trace:NodeComplexMesh::optimize(), culled 0 verts, merged 0 triangles
Trace:Stripper::buildStrips, triangles = 86, strips = 13, triangles per strip = 6.6
Trace:Stripper::buildStrips, triangles = 20, strips = 7, triangles per strip = 2.9
Trace:Stripper::buildStrips, triangles = 24, strips = 7, triangles per strip = 3.4
Trace:Stripper::buildStrips, triangles = 24, strips = 7, triangles per strip = 3.4
Trace:Stripper::buildStrips, triangles = 48, strips = 15, triangles per strip = 3.2
Trace:Stripper::buildStrips, triangles = 4, strips = 1, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 4, strips = 2, triangles per strip = 2.0
Trace:Stripper::buildStrips, triangles = 4, strips = 2, triangles per strip = 2.0
Trace:Stripper::buildStrips, triangles = 4, strips = 1, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 50, strips = 18, triangles per strip = 2.8
Trace:Stripper::buildStrips, triangles = 50, strips = 22, triangles per strip = 2.3
Trace:Stripper::buildStrips, triangles = 138, strips = 40, triangles per strip = 3.5
Trace:Stripper::buildStrips, triangles = 138, strips = 39, triangles per strip = 3.5
Trace:Stripper::buildStrips, triangles = 198, strips = 49, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 86, strips = 13, triangles per strip = 6.6


Choosing by minimum connections..

Trace:NodeComplexMesh::optimize(), culled 2132 verts, merged 18 triangles
Trace:NodeComplexMesh::optimize(), culled 0 verts, merged 0 triangles
Trace:Stripper::buildStrips, triangles = 86, strips = 13, triangles per strip = 6.6
Trace:Stripper::buildStrips, triangles = 20, strips = 7, triangles per strip = 2.9
Trace:Stripper::buildStrips, triangles = 24, strips = 7, triangles per strip = 3.4
Trace:Stripper::buildStrips, triangles = 24, strips = 9, triangles per strip = 2.7
Trace:Stripper::buildStrips, triangles = 48, strips = 12, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 4, strips = 1, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 4, strips = 2, triangles per strip = 2.0
Trace:Stripper::buildStrips, triangles = 4, strips = 2, triangles per strip = 2.0
Trace:Stripper::buildStrips, triangles = 4, strips = 1, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 50, strips = 18, triangles per strip = 2.8
Trace:Stripper::buildStrips, triangles = 50, strips = 22, triangles per strip = 2.3
Trace:Stripper::buildStrips, triangles = 138, strips = 44, triangles per strip = 3.1
Trace:Stripper::buildStrips, triangles = 138, strips = 40, triangles per strip = 3.5
Trace:Stripper::buildStrips, triangles = 198, strips = 49, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 86, strips = 13, triangles per strip = 6.6

Trace:NodeComplexMesh::optimize(), culled 2132 verts, merged 18 triangles
Trace:NodeComplexMesh::optimize(), culled 0 verts, merged 0 triangles
Trace:Stripper::buildStrips, triangles = 86, strips = 13, triangles per strip = 6.6
Trace:Stripper::buildStrips, triangles = 20, strips = 7, triangles per strip = 2.9
Trace:Stripper::buildStrips, triangles = 24, strips = 7, triangles per strip = 3.4
Trace:Stripper::buildStrips, triangles = 24, strips = 7, triangles per strip = 3.4
Trace:Stripper::buildStrips, triangles = 48, strips = 15, triangles per strip = 3.2
Trace:Stripper::buildStrips, triangles = 4, strips = 1, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 4, strips = 2, triangles per strip = 2.0
Trace:Stripper::buildStrips, triangles = 4, strips = 2, triangles per strip = 2.0
Trace:Stripper::buildStrips, triangles = 4, strips = 1, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 50, strips = 17, triangles per strip = 2.9
Trace:Stripper::buildStrips, triangles = 50, strips = 24, triangles per strip = 2.1
Trace:Stripper::buildStrips, triangles = 138, strips = 40, triangles per strip = 3.5
Trace:Stripper::buildStrips, triangles = 138, strips = 39, triangles per strip = 3.5
Trace:Stripper::buildStrips, triangles = 198, strips = 49, triangles per strip = 4.0
Trace:Stripper::buildStrips, triangles = 86, strips = 13, triangles per strip = 6.6
*/

//----------------------------------------------------------------------------
// EOF

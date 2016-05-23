/*
	NodeComplexMesh.cpp

	This is the high level polygon object
	(c)2005 Palestar, Richard Lyle
*/

#pragma warning( disable : 4786 ) // identifier was truncated to '255' characters in the browser information

#define RENDER3D_DLL

#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Standard/Limits.h"
#include "Display/PrimitiveSetTransform.h"
#include "Render3D/NodeComplexMesh.h"

//-------------------------------------------------------------------------------

//#define SHOW_MATERIALS

struct ObsoleteVertex
{
	Vector3			position;
	Color			diffuse;	
	Vector3			normal;
};

IMPLEMENT_RAW_STREAMING( ObsoleteVertex );

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeComplexMesh, NodeTransform );
REGISTER_FACTORY_KEY(  NodeComplexMesh, 4008780388213769326LL );

BEGIN_PROPERTY_LIST( NodeComplexMesh, NodeTransform )
	ADD_PROPERTY( m_Frames )
	ADD_PROPERTY( m_Verts )
	ADD_PROPERTY( m_Materials )
	ADD_PROPERTY( m_Triangles )
	ADD_PROPERTY( m_Hull )
	ADD_PROPERTY( m_Fps )
END_PROPERTY_LIST()

//----------------------------------------------------------------------------

NodeComplexMesh::NodeComplexMesh()
{
	m_Fps = 15.0f;
}

NodeComplexMesh::NodeComplexMesh( const NodeMesh & mesh )
{
	add( mesh, Matrix33( true ), Vector3( 0.0f ) );
	m_Fps = 15.0f;
}

NodeComplexMesh::NodeComplexMesh( const NodeComplexMesh & mesh ) : NodeTransform()
{
	add( mesh, Matrix33( true ), Vector3( 0.0f ) );
	m_Fps = 15.0f;
}

//----------------------------------------------------------------------------

BoxHull NodeComplexMesh::hull() const
{
	return( m_Hull );
}

void NodeComplexMesh::render( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	if ( m_Triangles.size() < 1 )
		return;
	Vector3 positionVS( context.worldToView( position ) );
	if (! context.sphereVisible( positionVS, m_Hull.radius() ) )
		return;

	DisplayDevice * pDisplay = context.display();
	ASSERT( pDisplay );

	// make sure the primitives are built
	if ( m_Primitives.size() == 0 )
		createPrimitives( pDisplay );

	// set the transform
	context.pushTransform( frame, position );
	// push the primitives for each material in this mesh
	for(int i=0;i<m_Materials.size();i++)
	{
		if (! m_Materials[i].valid( true, false ) )
			continue;

		// push the material
		context.push( m_Materials[i] );
		context.push( m_Primitives[i] );
	}

	// call the base class
	NodeTransform::render( context, frame, position );
}

void NodeComplexMesh::preRender( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	// animate this object if it has an animation
	if ( m_Frames.size() > 0 )
	{
		int frameIndex = (int)(context.time() * m_Fps);
		frameIndex = frameIndex % m_Frames.size();

		Frame & f = m_Frames[ frameIndex ];
		m_Frame = f.m_Frame;
		m_Position = f.m_Position;
	}

	NodeTransform::preRender( context, frame, position );
}

//----------------------------------------------------------------------------

void NodeComplexMesh::calculateHull()
{
	m_Hull.resetBox();
	if ( m_Verts.size() > 0)
	{
		for(int i=0;i<m_Verts.size();i++)
			m_Hull.addVertex( m_Verts[i].position );
	}
}

void NodeComplexMesh::calculateNormals()
{
	Array< Vector3 > triangleNormals;

	int j = 0;
	for(;j<m_Materials.size();j++)
	{
		int normalOffset = triangleNormals.size();
		triangleNormals.grow( m_Triangles[j].size() );

		// calculate the triangle normals and planes
		for(int i=0;i<m_Triangles[j].size();i++)
		{
			Triangle & triangle = m_Triangles[j][i];

			// calculate the normal of the triangle
			Vector3 normal( ((m_Verts[ triangle.verts[0] ].position - m_Verts[ triangle.verts[1] ].position) & 
								(m_Verts[ triangle.verts[0] ].position - m_Verts[ triangle.verts[2] ].position)) );
			normal.normalize();

			triangleNormals[ normalOffset + i ] = normal;
		}
	}

	// update the vertex normals
	for(j=0;j<m_Verts.size();j++)
	{
		Vertex & vertex = m_Verts[j];

		vertex.normal = Vector3(0,0,0);

		int triangleCount = 0;
		int normalOffset = 0;
		for(int i=0;i<m_Materials.size();i++)
		{
			for(int k=0;k<m_Triangles[i].size();k++)
				for(int v=0;v<3;v++)
					if ( m_Triangles[i][k].verts[v] == j )
					{
						vertex.normal += triangleNormals[ normalOffset + k ];
						triangleCount++;
					}

			normalOffset += m_Triangles[i].size();
		}

		if ( triangleCount > 0 )
			vertex.normal = vertex.normal / triangleCount;
		else
			vertex.normal = Vector3(0,1,0);
		vertex.normal.normalize();
	}
}

void NodeComplexMesh::setFps( float fps )
{
	m_Fps = fps;
}

int NodeComplexMesh::addFrame( const Frame & frame )
{
	m_Frames.push( frame );
	return( frameCount() - 1 );
}

int NodeComplexMesh::addVertex( const Vertex & vertex, bool cull /*= false*/ )
{
	if ( cull )
	{
		// search for another vertex that matches this vertex
		for(int i=0;i<m_Verts.size();i++)
			if ( m_Verts[i] == vertex )
				return i;

		// no vertex found, create new vertex
	}

	m_Verts.push( vertex );
	return( vertexCount() - 1 );
}

int NodeComplexMesh::addVertex( const NodeComplexMesh & mesh )
{
	int offset = m_Verts.size();

	m_Verts.grow( mesh.vertexCount() );
	for(int i=0;i<mesh.vertexCount();i++)
		m_Verts[ offset + i ] = mesh.vertex(i);

	return( offset );
}

int NodeComplexMesh::addMaterial( Material::Link pMaterial )
{
	m_Materials.push( pMaterial );
	m_Triangles.push();

	return( materialCount() - 1 );
}

int NodeComplexMesh::addMaterial( const NodeComplexMesh & mesh )
{
	int offset = m_Materials.size();

	m_Materials.grow( mesh.materialCount() );
	m_Triangles.grow( mesh.materialCount() );
	for(int i=0;i<mesh.materialCount();i++)
		m_Materials[ offset + i ] = mesh.material(i);

	return( offset );
}

int NodeComplexMesh::addTriangle( int material, const Triangle & triangle )
{
	if ( m_Triangles.size() <= material )
		m_Triangles.grow( (material - m_Triangles.size()) + 1 );

	m_Triangles[ material ].push( triangle );
	return( triangleCount(material) - 1 );
}

void NodeComplexMesh::add( const NodeComplexMesh & mesh,
						const Matrix33 & rotation,
						const Vector3 & translation )
{
	invalidate();

	for(int k=0;k<mesh.materialCount();k++)
	{
		// look for the material already in this object
		int materialIndex = -1;
		int j = 0;
		for(;j<m_Materials.size();j++)
			if ( m_Materials[j]->key() == mesh.material(k)->key() )
			{
				materialIndex = j;
				break;
			}

		// if not found, then add the material to this object
		if ( materialIndex < 0 )
			materialIndex = addMaterial( mesh.material(k) );

		for(j=0;j<mesh.triangleCount(k);j++)
		{
			const Triangle & srcTriangle = mesh.triangle( k, j );

			Triangle newTriangle;
			for(int v=0;v<3;v++)
			{
				const Vertex & srcVertex = mesh.vertex( srcTriangle.verts[v] );

				Vertex newVertex;
				newVertex.position = (rotation * srcVertex.position) + translation;
				newVertex.normal = (rotation * srcVertex.normal).normalize();

				newTriangle.verts[v] = addVertex( newVertex, true );
				newTriangle.uv[v].u = srcTriangle.uv[v].u;
				newTriangle.uv[v].v = srcTriangle.uv[v].v;
			}

			addTriangle( materialIndex, newTriangle );
		}
	}

	calculateHull();
}

void NodeComplexMesh::add( const NodeMesh & mesh,
						const Matrix33 & rotation,
						const Vector3 & translation )
{
	invalidate();

	// look for the material already in this object
	int materialIndex = -1;
	int j = 0;
	for(;j<m_Materials.size();j++)
		if ( m_Materials[j].pointer() == mesh.material() )
		{
			materialIndex = j;
			break;
		}

	// if not found, then add the material to this object
	if ( materialIndex < 0 )
		materialIndex = addMaterial( mesh.material() );

	for(j=0;j<mesh.triangleCount();j++)
	{
		const NodeMesh::Triangle & srcTriangle = mesh.triangle( j );

		Triangle newTriangle;
		for(int v=0;v<3;v++)
		{
			const NodeMesh::Vertex & srcVertex = mesh.vertex( srcTriangle.vertex[v] );

			Vertex newVertex;
			newVertex.position = (rotation * srcVertex.position) + translation;
			newVertex.normal = (rotation * srcVertex.normal).normalize();

			newTriangle.verts[v] = addVertex( newVertex, true );
			newTriangle.uv[v].u = srcTriangle.uv[v].u;
			newTriangle.uv[v].v = srcTriangle.uv[v].v;
		}

		addTriangle( materialIndex, newTriangle );
	}

	calculateHull();
}

void NodeComplexMesh::reverse()
{
	invalidate();

	int j = 0;
	for(;j<m_Verts.size();j++)
		m_Verts[j].normal = -m_Verts[j].normal;

	for(j=0;j<m_Materials.size();j++)
	{
		for(int k=0;k<m_Triangles[j].size();k++)
		{
			Triangle & triangle = m_Triangles[j][k];
			
			// swap vertex 0 and 2
			int swap = triangle.verts[2];
			triangle.verts[2] = triangle.verts[0];
			triangle.verts[0] = swap;

			// swap the uv's
			UV swapUV = triangle.uv[2];
			triangle.uv[2] = triangle.uv[0];
			triangle.uv[0] = swapUV;
		}
	}
}

void NodeComplexMesh::optimize()
{
	// invalidate the device objects
	invalidate();

#ifdef _DEBUG
	int culledCount = 0;
#endif

	// snap-cull verticies
	int i = 1;
	for(;i<m_Verts.size();)
	{
		bool vertexCulled = false;

		for(int j=i-1;j>=0;j--)
		{
			// check vertex i against all verticies before it, if we find a previous vertex close enough let's merge it
			if ( m_Verts[i] == m_Verts[j] )
			{
				// removing vertex i, update triangles
				for(int m=0;m<m_Materials.size();m++)
				{
					for(int k=0;k<m_Triangles[m].size();k++)
					{
						Triangle & triangle = m_Triangles[m][k];

						for(int v=0;v<3;v++)
						{
							if ( triangle.verts[v] == i )
								triangle.verts[v] = j;
							else if ( triangle.verts[v] > i)
								triangle.verts[v]--;
						}
					}
				}
#ifdef _DEBUG
				culledCount++;
#endif
				m_Verts.remove( i );
				vertexCulled = true;
				break;
			}
		}

		if (! vertexCulled )
			i++;
	}

#ifdef _DEBUG
	int mergeCount = 0;
#endif

	// find triangles that share an edge, then check if we can merge them
	for(i=0;i<m_Materials.size();)
	{
		bool trianglesMerged = false;
		for(int k=0;k<m_Triangles[i].size();)
		{
			if ( optimizeTriangle( i, k ) )
			{
				trianglesMerged = true;
#ifdef _DEBUG
				mergeCount++;
#endif
			}
			else
				k++;
		}

		// repeat the merge pass until we have no more triangles to merge
		if (! trianglesMerged )
			i++;
	}

	// remove orphan verticies
	for(i=0;i<m_Verts.size();i++)
	{
		bool orphaned = true;
		for(int k=0;k<m_Materials.size() && orphaned;k++)
			for(int j=0;j<m_Triangles[k].size() && orphaned;j++)
			{
				Triangle & triangle = m_Triangles[k][j];
				for(int v=0;v<3;v++)
					if ( triangle.verts[v] == i )
					{
						orphaned = false;
						break;
					}
			}

		if ( orphaned )
		{
			// remove vertex i
			m_Verts.remove( i );

#ifdef _DEBUG
			culledCount++;
#endif
			// update triangles
			for(int k=0;k<m_Materials.size() && orphaned;k++)
				for(int j=0;j<m_Triangles[k].size() && orphaned;j++)
				{
					Triangle & triangle = m_Triangles[k][j];
					for(int v=0;v<3;v++)
						if ( triangle.verts[v] > i )
							triangle.verts[v]--;
				}
		}
	}

#ifdef _DEBUG
	TRACE( CharString().format( "NodeComplexMesh::optimize(), culled %d verts, merged %d triangles", culledCount, mergeCount ) );
#endif
}

void NodeComplexMesh::invalidate()
{
	m_Primitives.release();
}

void NodeComplexMesh::scale( float scale )
{
	for(int i=0;i<m_Verts.size();i++)
		m_Verts[i].position = m_Verts[i].position * scale;

	calculateHull();
}

void NodeComplexMesh::release()
{
	invalidate();

	m_Frames.release();
	m_Verts.release();
	m_Materials.release();
	m_Triangles.release();
	m_Hull.resetBox();
}

//-------------------------------------------------------------------------------

#undef EPS
#define EPS 1e-04f

bool NodeComplexMesh::optimizeTriangle( int material, int k )
{
	Triangle & triangleA = m_Triangles[material][k];

	for(int j=0;j<m_Triangles[material].size();j++)
	{
		if ( j == k )
			continue;

		Triangle & triangleB = m_Triangles[material][j];

		// check edges on triangleA and triangleB, if any use the same verticies then 
		// we have a candiate for merging
		for(int a1=0;a1<3;a1++)
			for(int b1=0;b1<3;b1++)
			{
				int a2 = (a1 + 1) % 3;
				int b2 = (b1 + 1) % 3;

				if ( triangleA.verts[a1] == triangleB.verts[b2] &&
					triangleA.verts[a2] == triangleB.verts[b1] )
				{
					int a3 = (a2 + 1) % 3;
					int b3 = (b2 + 1) % 3;

					// check the edges a3-a1 and b2-b3, if they form a straight line
					// then we can collapse triangleA into triangleB
					Vector3 edge1( m_Verts[triangleA.verts[a3]].position - m_Verts[triangleA.verts[a1]].position );
					edge1.normalize();
					Vector3 edge2( m_Verts[triangleB.verts[b2]].position - m_Verts[triangleB.verts[b3]].position );
					edge2.normalize();

					float dot = edge1 | edge2;
					if ( dot < (1.0f + EPS) && dot > (1.0f - EPS) )
					{
						// this two edges form a straight line, collapse triangleA into triangleB
						triangleB.verts[b2] = triangleA.verts[a3];
						m_Triangles[material].removeSwap( k );
						return true;
					}

					// also check edges a3-a2 and b1-b3
					edge1 = m_Verts[triangleA.verts[a3]].position - m_Verts[triangleA.verts[a2]].position;
					edge1.normalize();
					edge2 = m_Verts[triangleB.verts[b1]].position - m_Verts[triangleB.verts[b3]].position;
					edge2.normalize();

					dot = edge1 | edge2;
					if ( dot < (1.0f + EPS) && dot > (1.0f - EPS) )
					{
						triangleB.verts[b1] = triangleA.verts[a3];
						m_Triangles[material].removeSwap( k );
						return true;
					}
				}
			}
	}

	return false;
}

//----------------------------------------------------------------------------

void NodeComplexMesh::createPrimitives( DisplayDevice * pDisplay )
{
	m_Primitives.allocate( m_Materials.size() );
	for(int i=0;i<m_Materials.size();i++)
	{
		int triangleCount = m_Triangles[ i ].size();
		if ( triangleCount > 0 )
		{
			::Triangle * pTriangles = new ::Triangle[ triangleCount ];
			::Vertex * dstVertex = (::Vertex *)pTriangles;
			for(int k=0;k<triangleCount;k++)
			{
				Triangle & srcTriangle = m_Triangles[ i ][ k ];
				for(int v=0;v<3;v++)
				{
					Vertex & srcVertex = m_Verts[ srcTriangle.verts[v] ];

					*dstVertex = ::Vertex( srcVertex.position, 
						srcVertex.normal, 
						srcTriangle.uv[v].u, 
						srcTriangle.uv[v].v );
					dstVertex++;
				}
			}

			m_Primitives[i] = PrimitiveTriangleListI::create( pDisplay, triangleCount, pTriangles );

			delete [] pTriangles;
		}
	}
}

//----------------------------------------------------------------------------
// EOF


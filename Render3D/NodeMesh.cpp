/*
	NodeMesh.cpp

	This is the high level polygon object
	(c)2005 Palestar, Richard Lyle
*/

#pragma warning( disable : 4786 ) // identifier was truncated to '255' characters in the browser information

#define RENDER3D_DLL

#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Standard/Limits.h"
#include "Display/PrimitiveSetTransform.h"
#include "Render3D/Scene.h"
#include "Render3D/NodeMesh.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeMesh, NodeTransform );
REGISTER_FACTORY_KEY(  NodeMesh, 3934871209353629227LL );

BEGIN_PROPERTY_LIST( NodeMesh, NodeTransform )
	ADD_PROPERTY( m_Frames );
	ADD_PROPERTY( m_Vertex );
	ADD_PROPERTY( m_Triangle );
	ADD_PROPERTY( m_Material );
	ADD_PROPERTY( m_BoxHull );
	ADD_PROPERTY( m_fRadius );
END_PROPERTY_LIST();

//----------------------------------------------------------------------------

NodeMesh::NodeMesh() : m_fRadius( 0.0f )
{}

//----------------------------------------------------------------------------

BoxHull NodeMesh::hull() const
{
	return( m_BoxHull );
}

void NodeMesh::render( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	if ( m_Triangle.size() < 1 || !m_Material.valid( true, false ) )
		return;
	Vector3 positionVS( context.worldToView( position ) );
	if (! context.sphereVisible( positionVS, m_fRadius ) )
		return;

	// make sure the triangle list primitive is built
	if (! m_Primitive.valid() )
	{
		::Triangle * pTriangles = new ::Triangle[ m_Triangle.size() ];
		::Vertex * dstVertex = (::Vertex *)pTriangles;
		for(int k=0;k<m_Triangle.size();k++)
		{
			Triangle & srcTriangle = m_Triangle[ k ];
			for(int v=0;v<3;v++)
			{
				Vertex & srcVertex = m_Vertex[ srcTriangle.vertex[v] ];

				*dstVertex = ::Vertex( srcVertex.position, srcVertex.normal, srcTriangle.uv[v].u, srcTriangle.uv[v].v );
				dstVertex++;
			}
		}
		m_Primitive = PrimitiveTriangleListI::create( context.device(), m_Triangle.size(), pTriangles );

		delete [] pTriangles;
	}

	// set the material
	context.push( m_Material );
	// set the transform
	context.pushTransform( frame, position );
	// push the triangles
	context.push( m_Primitive );

	// call the base class
	NodeTransform::render( context, frame, position );
}

void NodeMesh::preRender( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	if ( m_Frames.size() > 0 )
	{
		Scene * pScene = Scene::currentScene();
		ASSERT( pScene );

		int frameIndex = (int)( context.time() * pScene->fps() );
		frameIndex = frameIndex % m_Frames.size();

		Frame & f = m_Frames[ frameIndex ];
		m_Frame = f.m_Frame;
		m_Position = f.m_Position;
	}

	NodeTransform::preRender( context, frame, position );
}

//----------------------------------------------------------------------------

int	NodeMesh::frameCount() const
{
	return( m_Frames.size() );
}

const NodeMesh::Frame & NodeMesh::frame( int n ) const
{
	return( m_Frames[ n ] );
}

int NodeMesh::vertexCount() const
{
	return( m_Vertex.size() );
}

const NodeMesh::Vertex & NodeMesh::vertex( int n ) const
{
	return( m_Vertex[ n ] );
}

int NodeMesh::triangleCount() const
{
	return( m_Triangle.size() );
}

const NodeMesh::Triangle & NodeMesh::triangle( int n ) const
{
	return( m_Triangle[ n ] );
}

Material * NodeMesh::material() const
{
	return( m_Material );
}

//----------------------------------------------------------------------------

int NodeMesh::addFrame( const Frame & frame )
{
	m_Frames.push( frame );
	return( frameCount() - 1 );
}

void NodeMesh::removeFrame( int n )
{
	m_Frames.remove( n );
}

int NodeMesh::addVertex( const Vector3 & position, const Color & diffuse )
{
	Vertex & newVertex = m_Vertex.push();
	newVertex.position = position;
	newVertex.diffuse = diffuse;

	return( vertexCount() - 1 );
}

int NodeMesh::addVertex( const Vector3 & position, const Color & diffuse, const Vector3 &normal )
{
	Vertex & newVertex = m_Vertex.push();
	newVertex.position = position;
	newVertex.diffuse = diffuse;
	newVertex.normal = normal;

	return( vertexCount() - 1 );
}

void NodeMesh::removeVertex( int n )
{
	m_Vertex.remove( n );
}

int NodeMesh::addTriangle( const int * pVertex, const UV * pUV )
{
	Triangle & newTriangle = m_Triangle.push();
	memcpy( newTriangle.vertex, pVertex, sizeof(newTriangle.vertex) );
	memcpy( newTriangle.uv, pUV, sizeof(newTriangle.uv) );

	return( triangleCount() - 1 );
}

int NodeMesh::addTriangles( int triCount, const int * pVertex, const UV * pUV )
{
	for(int i=0;i<triCount;i++, pVertex += 3, pUV += 3)
		addTriangle( pVertex, pUV );

	return( triangleCount() - triCount );
}

void NodeMesh::removeTriangle( int n )
{
	m_Triangle.remove( n );
}

void NodeMesh::setMaterial( Material::Link material )
{
	m_Material = material;
}

void NodeMesh::update( bool snapVerts /*= true*/, bool DoNormals /*= true*/, bool DoVertexNormals /*= true*/  )
{
	// invalidate the device objects
	invalidate();

#ifdef _DEBUG
	int culledCount = 0;
#endif

	if ( snapVerts )
	{
		// snap-cull verticies
		for(int i=1;i<m_Vertex.size();)
		{
			bool vertexCulled = false;

			for(int j=i-1;j>=0;j--)
			{
				float delta = (m_Vertex[i].position - m_Vertex[j].position).magnitude();

				// check vertex i against all verticies before it, if we find a previous vertex close enough let's merge it
				if ( delta < EPSILON && m_Vertex[i].diffuse == m_Vertex[j].diffuse )
				{
					// removing vertex i, update triangles
					for(int k=0;k<m_Triangle.size();k++)
					{
						Triangle & triangle = m_Triangle[k];

						for(int v=0;v<3;v++)
						{
							if ( triangle.vertex[v] == i )
								triangle.vertex[v] = j;
							else if ( triangle.vertex[v] > i)
								triangle.vertex[v]--;
						}
					}
	#ifdef _DEBUG
					culledCount++;
	#endif
					m_Vertex.remove( i );
					vertexCulled = true;
					break;
				}
			}

			if (! vertexCulled )
				i++;
		}
	}

	if ( DoNormals )
	{
		// calculate the triangle normals and planes
		for(int i=0;i<m_Triangle.size();i++)
		{
			Triangle & triangle = m_Triangle[i];

			// calculate the normal of the triangle
			triangle.normal = ((m_Vertex[ triangle.vertex[0] ].position - m_Vertex[ triangle.vertex[1] ].position) & 
								(m_Vertex[ triangle.vertex[0] ].position - m_Vertex[ triangle.vertex[2] ].position));
			triangle.normal.normalize();

			// get the center of the triangle
			triangle.center = (m_Vertex[ triangle.vertex[0] ].position + 
								m_Vertex[ triangle.vertex[1] ].position + 
								m_Vertex[ triangle.vertex[2] ].position) / 3.0f;

			// calculate the plane of the triangle
			triangle.plane = Plane( triangle.normal, triangle.center );
		}
	}

	if ( DoVertexNormals )
	{
		// update the vertex normals
		for(int i=0;i<m_Vertex.size();i++)
		{
			Vertex & vertex = m_Vertex[i];

			vertex.normal = Vector3(0,0,0);

			int triangleCount = 0;
			for(int j=0;j<m_Triangle.size();j++)
				for(int v=0;v<3;v++)
					if ( m_Triangle[j].vertex[v] == i )
					{
						vertex.normal += m_Triangle[j].normal;
						triangleCount++;
					}

			if ( triangleCount == 0 )
			{
				/// vertex is unused, remove it then
				for(int j=0;j<m_Triangle.size();j++)
					for(int v=0;v<3;v++)
						if (m_Triangle[j].vertex[v] >= i )
						{
							ASSERT( m_Triangle[j].vertex[v] != i );
							m_Triangle[j].vertex[v]--;
						}

	#ifdef _DEBUG
				culledCount++;
	#endif
				m_Vertex.remove( i );
			}
			else
				vertex.normal = vertex.normal / triangleCount;
		}
	}

	// update bounding box
	m_BoxHull.resetBox();
	if ( m_Vertex.size() > 0)
	{
		for(int i=0;i<m_Vertex.size();i++)
			m_BoxHull.addVertex( m_Vertex[i].position );
	}
	
	m_fRadius = m_BoxHull.radius();

	TRACE( CharString().format("NodeMesh::update(), culled %d verticies", culledCount) );
}

void NodeMesh::merge( const Matrix33 &frame, const Vector3 &offset, const NodeMesh & merge )
{
	// merge mesh into this mesh, transform mesh by frame and offset
	int vertexOffset = vertexCount();
	for(int k=0;k<merge.vertexCount();k++)
		addVertex( (frame * merge.vertex(k).position) + offset, merge.vertex(k).diffuse );

	for(int i=0;i<merge.triangleCount();i++)
	{
		const Triangle & triangle = merge.triangle( i );

		int vertex[ 3 ];
		for(int v=0;v<3;v++)
			vertex[ v ] = vertexOffset + triangle.vertex[ v ];

		addTriangle( vertex, triangle.uv );
	}
}

void NodeMesh::invalidate()
{
	m_Primitive = NULL;
}

void NodeMesh::release()
{
	invalidate();
	m_Frames.release();
	m_Triangle.release();
	m_Vertex.release();
}

//----------------------------------------------------------------------------

void NodeMesh::buildCube( const Vector3 &min, const Vector3 &max, bool mapToWorld /*= false*/ )
{
	release();

	float xu, xv, yu, yv, zu, zv;
	xu = xv = yu = yv = zu = zv = 1.0f;

	if ( mapToWorld && m_Material.valid() )
	{
		xu = sqrtf((max.x * max.x) + (min.x * min.x)) / m_Material->width();
		xv = sqrtf((max.y * max.y) + (min.y * min.y)) / m_Material->height();
		yu = sqrtf((max.z * max.z) + (min.z * min.z)) / m_Material->width();
		yv = xv;
		zu = yu;
		zv = xu;
	}
	const NodeMesh::UV cubeUV[] = 
	{
		{ xu, 0.0 }, { xu, xv }, { 0.0, xv },	// Front
		{ 0.0, xv }, { 0.0, 0.0 }, { xu, 0.0 },
		{ yu, 0.0 }, { yu, yv }, { 0.0, yv },	// Right
		{ 0.0, yv }, { 0.0, 0.0 }, { yu, 0.0 },
		{ xu, 0.0 }, { xu, xv }, { 0.0, xv },	// Back
		{ 0.0, xv }, { 0.0, 0.0 }, { xu, 0.0 },
		{ yu, 0.0 }, { yu, yv }, { 0.0, yv },	// Left
		{ 0.0, yv }, { 0.0, 0.0 }, { yu, 0.0 },
		{ zu, 0.0 }, { zu, zv }, { 0.0, zv },	// Bottom
		{ 0.0, zv }, { 0.0, 0.0 }, { zu, 0.0 },
		{ zu, 0.0 }, { zu, zv }, { 0.0, zv },	// Top
		{ 0.0, zv }, { 0.0, 0.0 }, { zu, 0.0 },
	};
	const int cubeVertex[] =
	{
		2, 1, 0,		// Front
		0, 3, 2, 
		6, 5, 1,		// Right
		1, 2, 6,
		7, 4, 5,		// Back
		5, 6, 7,
		3, 0, 4,		// Left
		4, 7, 3,
		0, 1, 5,		// Bottom
		5, 4, 0,
		6, 2, 3,		// Top
		3, 7, 6
	};

	addVertex( Vector3( min.x,min.y,min.z ), Color(255,255,255,255) );
	addVertex( Vector3( max.x,min.y,min.z ), Color(255,255,255,255) );
	addVertex( Vector3( max.x,max.y,min.z ), Color(255,255,255,255) );
	addVertex( Vector3( min.x,max.y,min.z ), Color(255,255,255,255) );
	addVertex( Vector3( min.x,min.y,max.z ), Color(255,255,255,255) );
	addVertex( Vector3( max.x,min.y,max.z ), Color(255,255,255,255) );
	addVertex( Vector3( max.x,max.y,max.z ), Color(255,255,255,255) );
	addVertex( Vector3( min.x,max.y,max.z ), Color(255,255,255,255) );
	addTriangles( 12, cubeVertex, cubeUV );

	update();
}

void NodeMesh::buildPyramid( const Vector3 &top, const Vector3 &bottomCenter, float base, bool mapToWorld /*= false*/ )
{
	ASSERT( 0 );
}

void NodeMesh::buildCylinder( const Vertex &center, float radius, float height, int detail, bool mapToWorld /*= false*/ )
{
	ASSERT( 0 );
}

//----------------------------------------------------------------------------

/*

Icosahedron
  0  a -1     a  1  0    -a  1  0
  0  a  1    -a  1  0     a  1  0
  0  a  1     0 -a  1    -1  0  a
  0  a  1     1  0  a     0 -a  1
  0  a -1     0 -a -1     1  0 -a
  0  a -1    -1  0 -a     0 -a -1
  0 -a  1     a -1  0    -a -1  0
  0 -a -1    -a -1  0     a -1  0
 -a  1  0    -1  0  a    -1  0 -a
 -a -1  0    -1  0 -a    -1  0  a
  a  1  0     1  0 -a     1  0  a
  a -1  0     1  0  a     1  0 -a
  0  a  1    -1  0  a    -a  1  0
  0  a  1     a  1  0     1  0  a
  0  a -1    -a  1  0    -1  0 -a
  0  a -1     1  0 -a     a  1  0
  0 -a -1    -1  0 -a    -a -1  0
  0 -a -1     a -1  0     1  0 -a
  0 -a  1    -a -1  0    -1  0  a
  0 -a  1     1  0  a     a -1  0

Where a = 2 / (1 + sqrt(5)) = 1 / phi 

*/

struct Face
{
	Vector3		p1, p2, p3;
};

// build a sphere using surface refinement - starting shape is a icosahedron

void NodeMesh::buildSphere( const Vector3 &center, float radius, int detail )
{
	release();

	const float a = 2.0f / (1.0f + sqrtf(5));
	const Vector3 icosahedron[] =
	{
		Vector3(0,a,-1),	Vector3(a,1,0),		Vector3(-a,1,0),
		Vector3(0,a,1),		Vector3(-a,1,0),	Vector3(a,1,0),
		Vector3(0,a,1),		Vector3(0,-a,1),	Vector3(-1,0,a),
		Vector3(0,a,1),		Vector3(1,0,a),		Vector3(0,-a,1),
		Vector3(0,a,-1),	Vector3(0,-a,-1),	Vector3(1,0,-a),
		Vector3(0,a,-1),    Vector3(-1,0,-a),	Vector3(0,-a,-1),
		Vector3(0,-a,1),	Vector3(a,-1,0),	Vector3(-a,-1,0),
		Vector3(0,-a,-1),	Vector3(-a,-1,0),	Vector3(a,-1,0),
		Vector3(-a,1,0),	Vector3(-1,0,a),	Vector3(-1,0,-a),
		Vector3(-a,-1,0),	Vector3(-1,0,-a),	Vector3(-1,0,a),
		Vector3(a,1,0),		Vector3(1,0,-a),	Vector3(1,0,a),
		Vector3(a,-1,0),	Vector3(1,0,a),		Vector3(1,0,-a),
		Vector3(0,a,1),		Vector3(-1,0,a),	Vector3(-a,1,0),
		Vector3(0,a,1),		Vector3(a,1,0),		Vector3(1,0,a),
		Vector3(0,a,-1),	Vector3(-a,1,0),	Vector3(-1,0,-a),
		Vector3(0,a,-1),	Vector3(1,0,-a),	Vector3(a,1,0),
		Vector3(0,-a,-1),	Vector3(-1,0,-a),	Vector3(-a,-1,0),
		Vector3(0,-a,-1),	Vector3(a,-1,0),	Vector3(1,0,-a),
		Vector3(0,-a,1),	Vector3(-a,-1,0),	Vector3(-1,0,a),
		Vector3(0,-a,1),	Vector3(1,0,a),		Vector3(a,-1,0)
	};

	const int vertexCount = sizeof(icosahedron) / sizeof(Vector3);
	const int faceCount = vertexCount / 3;

	// initialize the initial set of faces
	Array< Face > faces;
	int i = 0;
	for(;i<faceCount;i++)
	{
		Face & newFace = faces.push();

		const int v = i * 3;
		newFace.p1 = icosahedron[ v ];
		newFace.p2 = icosahedron[ v + 1 ];
		newFace.p3 = icosahedron[ v + 2 ];
	}

	for (i=1;i<detail;i++) 
	{
		int nstart = faces.size();
		for (int j=0;j<nstart;j++) 
		{
			// Create initially copies for the new facets
			Face & face1 = faces.push( faces[j] );
			Face & face2 = faces.push( faces[j] );
			Face & face3 = faces.push( faces[j] );

			// Calculate the midpoints
			Vector3 p1( (faces[j].p1 + faces[j].p2) * 0.5f );
			Vector3 p2( (faces[j].p2 + faces[j].p3) * 0.5f );
			Vector3 p3( (faces[j].p3 + faces[j].p1) * 0.5f );

			// Replace the current facet
			faces[j].p2 = p1;
			faces[j].p3 = p3;

			// Create the changed vertices in the new facets
			face1.p1 = p1;
			face1.p3 = p2;
			face2.p1 = p3;
			face2.p2 = p2;
			face3.p1 = p1;
			face3.p2 = p2;
			face3.p3 = p3;
		}
	}

	// normalize the faces
	for(i=0;i<faces.size();i++)
	{
		faces[i].p1.normalize();
		faces[i].p2.normalize();
		faces[i].p3.normalize();
	}

	const Color D( 255,255,255,255 );
	const UV	ZUV = { 0, 0 };

	for(i=0;i<faces.size();i++)
	{
		Triangle & newTriangle = m_Triangle.push();
		newTriangle.vertex[0] = addVertex( faces[i].p1 * radius, D );
		newTriangle.vertex[1] = addVertex( faces[i].p3 * radius, D );
		newTriangle.vertex[2] = addVertex( faces[i].p2 * radius, D );

		const float TPI = PI * 2;
		newTriangle.uv[0].u = (atan2( faces[i].p1.x, faces[i].p1.z ) + PI) / TPI;
		newTriangle.uv[0].v = 1.0f - ((faces[i].p1.y * 0.5f) + 0.5f);
		newTriangle.uv[1].u = (atan2( faces[i].p3.x, faces[i].p3.z ) + PI) / TPI;
		newTriangle.uv[1].v = 1.0f - ((faces[i].p3.y * 0.5f) + 0.5f);
		newTriangle.uv[2].u = (atan2( faces[i].p2.x, faces[i].p2.z ) + PI) / TPI;
		newTriangle.uv[2].v = 1.0f - ((faces[i].p2.y * 0.5f) + 0.5f);
	}

	update();
}

//----------------------------------------------------------------------------

const NodeMesh::UV		g_PlaneUV[] = 
{
	{ 1.0, 0.0 }, { 1.0, 1.0 }, { 0.0, 1.0 },
	{ 0.0, 1.0 }, { 0.0, 0.0 }, { 1.0, 0.0 },
};

const int				g_PlaneVertex[] =
{
	2, 1, 0,	
	0, 3, 2, 
};

void NodeMesh::buildPlane( const Vector3 &size, int width, int height )
{
	release();

	for(int y=0;y<height;y++)
		for(int x=0;x<width;x++)
		{
			Vector3 offset( size.x * (x - (width / 2)), size.y, size.z * (y - (height / 2)) );
			int vertexOffset = addVertex( Vector3( offset.x, offset.y, offset.z ), Color(255,255,255,255) );
			addVertex( Vector3( offset.x + size.x, offset.y, offset.z), Color(255,255,255,255) );
			addVertex( Vector3( offset.x + size.x, offset.y, offset.z + size.z ), Color(255,255,255,255) );
			addVertex( Vector3( offset.x, offset.y, offset.z + size.z ), Color(255,255,255,255) );

			int planeVerts[6];
			for(int i=0;i<6;i++)
				planeVerts[i] = g_PlaneVertex[i] + vertexOffset;

			addTriangles( 2, planeVerts, g_PlaneUV );
		}

	update();
}

//----------------------------------------------------------------------------
// EOF

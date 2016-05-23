/*
	NodeComplexMesh2.cpp

	This is the high level polygon object
	(c)2005 Palestar, Richard Lyle
*/

#pragma warning( disable : 4786 ) // identifier was truncated to '255' characters in the browser information

#define RENDER3D_DLL
//#define PROFILE_OFF

//! if defined to non-zero, then allow the animation data to be used for LOD levels..
#define ENABLE_LOD_ANIMATION			0

#include "Debug/Trace.h"
#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "Standard/Limits.h"
#include "Display/PrimitiveLineList.h"
#include "Display/PrimitiveSetTransform.h"

#include "NodeComplexMesh2.h"
#include "NodeBoolean.h"

//----------------------------------------------------------------------------

bool NodeComplexMesh2::sm_bShowVertexNormals = false;

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeComplexMesh2, NodeTransform );
REGISTER_FACTORY_KEY(  NodeComplexMesh2, 4191705967834383792LL );

BEGIN_PROPERTY_LIST( NodeComplexMesh2, NodeTransform )
	ADD_PROPERTY( m_NodeFrames )
	ADD_PROPERTY( m_Verts )
	ADD_PROPERTY( m_Materials )
	ADD_PROPERTY( m_Triangles )
	ADD_PROPERTY( m_Hull )
	ADD_PROPERTY( m_Fps )
	ADD_PROPERTY( m_EnableLOD )
	ADD_PROPERTY( m_MinLOD )
	ADD_PROPERTY( m_MaxLOD )
END_PROPERTY_LIST()

//----------------------------------------------------------------------------

NodeComplexMesh2::NodeComplexMesh2()
{
	m_Fps = 15.0f;
	m_EnableLOD = false;
	m_MinLOD = 1.0f;
	m_MaxLOD = 750.0f;
}

//----------------------------------------------------------------------------

BoxHull NodeComplexMesh2::hull() const
{
	return( m_Hull );
}

void NodeComplexMesh2::render( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	if ( m_Triangles.size() < 1 )
		return;
	Vector3 positionVS( context.worldToView( position ) );
	if (! context.sphereVisible( positionVS, BoxHull( m_Hull, frame, Vector3( 0.0f ) ).radius() ) )
		return;

	DisplayDevice * pDisplay = context.display();
	ASSERT( pDisplay );

	// make sure the primitives are built
	if ( m_Primitives.size() == 0 )
		createPrimitives( pDisplay );

	// get the current frame to render
	int fc = frameCount();
	int f = 0;

	if ( m_EnableLOD )
	{
		f = 0;								// highest level of detail - frame 0
#if ENABLE_LOD_ANIMATION
		if ( ! context.isShadowPass() )
		{
			// get the range from the view, scale the range by the object scale
			float range = positionVS.z * (1.0f - context.detail());
			if ( range > m_MinLOD )
			{
				f = (int)(((range - m_MinLOD) * fc) / (m_MaxLOD - m_MinLOD));
				if ( f >= fc )
					f = fc - 1;	// lowest level of detail
			}
		}
#endif
	}
	else
	{
		// vertex animation, no LOD
		f = (int)(context.time() * m_Fps);
		f %= fc;
	}

	// set the transform
	context.pushTransform( frame, position );
	// push the primitives for each material in this mesh
	for(int m=0;m<m_Materials.size();m++)
	{
		// background load the material, skip if it's not ready yet..
		if (! m_Materials[m].valid( true, false ) )
			continue;

		if ( m_Primitives[f].size() > m && m_Primitives[f][m].valid() )
		{
			// push the material and triangles
			context.push( m_Materials[m] );
			context.push( m_Primitives[f][m] );
		}
	}

	if ( sm_bShowVertexNormals )
	{
		PrimitiveLineList::Ref pLineList = PrimitiveLineList::create( pDisplay, m_Verts.size() );

		static Vector3 N( 0.0f );

		Line * pLine = pLineList->lock();
		for(int i=0;i<m_Verts.size();i++)
			pLine[i] = Line( Vertex( m_Verts[i].position,N,0,0 ), 
				Vertex( m_Verts[i].position + m_Verts[i].normal,N,0,0 ) );

		pLineList->unlock();

		context.push( new Material( WHITE ) );
		context.push( pLineList );
	}

	// call the base class
	NodeTransform::render( context, frame, position );
}

void NodeComplexMesh2::preRender( RenderContext &context, const Matrix33 & frame, const Vector3 & position )
{
	// NodeTransform based animation
	if ( m_NodeFrames.size() > 0 )
	{
		int f = (int)(context.time() * m_Fps);
		f %= m_NodeFrames.size();

		setFrame( m_NodeFrames[ f ].frame );
		setPosition( m_NodeFrames[ f ].position );
	}

	// call the base class
	NodeTransform::preRender( context, frame, position );
}
//----------------------------------------------------------------------------

void NodeComplexMesh2::calculateHull()
{
	m_Hull.resetBox();
	if ( m_Verts.size() > 0)
	{
		for(int i=0;i<m_Verts.size();i++)
			m_Hull.addVertex( m_Verts[i].position );
	}
}

#undef EPS
#define EPS 0.001f

inline bool CompareVertPosition( const Vector3 & lhs, const Vector3 & rhs )
{
	return( (lhs - rhs).magnitude() < EPS );
}

void NodeComplexMesh2::calculateNormals()
{
	TRACE( "NodeComplexMesh2::calculateNormals, culling verticies..." );

	// cull all verticies down to ones in the same position
	Array< int >				vertexMap( m_Verts.size() );		// map original vertex index to culled verts
	Array< Array< int > >		vertexUnMap;						// maps new vertex back to orignal verticies
	Array< Vector3 >			verts;								// culled verticies

	//vertexUnMap.setGrowSize( 5000 );
	//verts.setGrowSize( 5000 );

	int i = 0;
	for(;i<m_Verts.size();i++)
	{
		vertexMap[ i ] = -1;
		for(int j=0;j<verts.size();j++)
			if ( CompareVertPosition( m_Verts[i].position, verts[j] ) )
			{
				vertexMap[ i ] = j;
				vertexUnMap[ j ].push( i );
				break;
			}

		if ( vertexMap[i] < 0 )
		{
			// no match found, add vertex
			vertexMap[i] = verts.size();

			verts.push( m_Verts[i].position );
			vertexUnMap.push().push( i );
		}

		m_Verts[i].normal = Vector3(0,0,0);
	}
	TRACE( CharString().format( "NodeComplexMesh2::calculateNormals, culled verticies %d -> %d", m_Verts.size(), verts.size()) );

	Array< Vector3 >	vertexNormals( verts.size() );
	Array< int >		vertexTri( verts.size() );
	for(i=0;i<verts.size();i++)
	{
		vertexNormals[i] = Vector3( 0.0f );
		vertexTri[i] = 0;
	}

	TRACE( "NodeComplexMesh2::calculateNormals, triangle normals..." );
	for(int f=0;f<m_Triangles.size();f++)
		for(int j=0;j<m_Triangles[f].size();j++)
			for(int i=0;i<m_Triangles[f][j].size();i++)
			{
				Triangle & triangle = m_Triangles[f][j][i];

				int v1 = vertexMap[ triangle.verts[ 0 ] ];
				int v2 = vertexMap[ triangle.verts[ 1 ] ];
				int v3 = vertexMap[ triangle.verts[ 2 ] ];

				// calculate the normal of the triangle
				Vector3 normal( ((verts[ v1 ] - verts[ v2 ]) & 
									(verts[ v1 ] - verts[ v3 ])) );
				normal.normalize();

				vertexNormals[ v1 ] += normal;
				vertexTri[ v1 ]++;
				vertexNormals[ v2 ] += normal;
				vertexTri[ v2 ]++;
				vertexNormals[ v3 ] += normal;
				vertexTri[ v3 ]++;
			}

	// update the vertex normals
	for(i=0;i<verts.size();i++)
	{
		Vector3 normal( 0, 1, 0 );
		if ( vertexTri[ i ] > 0 )
			normal = vertexNormals[ i ] / float( vertexTri[ i ] );
		//normal.normalize();

		// store the normal back into the orignal verticies
		for(int k=0;k<vertexUnMap[ i ].size();k++)
		{
			int index = vertexUnMap[ i ][ k ];
			//ASSERT( m_Verts[index].normal.magnitude() == 0.0f );
			m_Verts[ index ].normal = normal;
		}
	}

	// renormalize the verts
	for(i=0;i<m_Verts.size();i++)
		m_Verts[i].normal.normalize();
}

void NodeComplexMesh2::setFps( float fps )
{
	m_Fps = fps;
}

void NodeComplexMesh2::setLOD( bool enable, float min, float max )
{
	m_EnableLOD = enable;
	m_MinLOD = min;
	m_MaxLOD = max;
}

int NodeComplexMesh2::addNodeFrame( const NodeComplexMesh2::Frame & frame )
{
	m_NodeFrames.push( frame );
	return m_NodeFrames.size() - 1;
}

int NodeComplexMesh2::addVertex( const Vertex & vertex, bool cull /*= false*/ )
{
	if ( cull )
	{
		// search for another vertex that matches this vertex
		for(int i=0;i<m_Verts.size();i++)
			if ( compareVerts( m_Verts[i], vertex ) )
				return i;

		// no vertex found, create new vertex
	}

	m_Verts.push( vertex );
	return( vertexCount() - 1 );
}

int NodeComplexMesh2::addMaterial( Material::Link pMaterial )
{
	m_Materials.push( pMaterial );
	return( materialCount() - 1 );
}

int NodeComplexMesh2::addTriangle( int f, int m, const Triangle & triangle )
{
	if ( m_Triangles.size() <= f )
		m_Triangles.grow( (f - m_Triangles.size()) + 1 );
	if ( m_Triangles[f].size() <= m )
		m_Triangles[f].grow( (m - m_Triangles[f].size()) + 1 );

	m_Triangles[ f ][ m ].push( triangle );
	return( triangleCount(f, m) - 1 );
}

void NodeComplexMesh2::setVerts( const Array< Vertex > & verts )
{
	m_Verts = verts;
}

void NodeComplexMesh2::setTriangles( int f, int m, const Array< Triangle > & triangles )
{
	if ( m_Triangles.size() <= f )
		m_Triangles.grow( (f - m_Triangles.size()) + 1 );
	if ( m_Triangles[f].size() <= m )
		m_Triangles[f].grow( (m - m_Triangles[f].size()) + 1 );

	m_Triangles[f][m] = triangles;
}

void NodeComplexMesh2::reverse()
{
	invalidate();

	for(int j=0;j<m_Verts.size();j++)
		m_Verts[j].normal = -m_Verts[j].normal;

	for(int f=0;f<m_Triangles.size();f++)
	{
		for(int j=0;j<m_Triangles[f].size();j++)
		{
			for(int k=0;k<m_Triangles[f][j].size();k++)
			{
				Triangle & triangle = m_Triangles[f][j][k];
				
				// swap vertex 0 and 2
				int swap = triangle.verts[2];
				triangle.verts[2] = triangle.verts[0];
				triangle.verts[0] = swap;
			}
		}
	}
}

void NodeComplexMesh2::optimize()
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
			if ( compareVerts( m_Verts[i], m_Verts[j] ) )
			{
				// removing vertex i, update triangles
				for(int f=0;f<m_Triangles.size();f++)
					for(int m=0;m<m_Triangles[f].size();m++)
						for(int k=0;k<m_Triangles[f][m].size();k++)
						{
							Triangle & triangle = m_Triangles[f][m][k];

							for(int v=0;v<3;v++)
							{
								if ( triangle.verts[v] == i )
									triangle.verts[v] = j;
								else if ( triangle.verts[v] > i)
									triangle.verts[v]--;
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

	// remove orphan verticies
	for(i=0;i<m_Verts.size();i++)
	{
		bool orphaned = true;
		for(int f=0;f<m_Triangles.size() && orphaned;f++)
			for(int k=0;k<m_Triangles[f].size() && orphaned;k++)
				for(int j=0;j<m_Triangles[f][k].size() && orphaned;j++)
				{
					Triangle & triangle = m_Triangles[f][k][j];
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
			for(int f=0;f<m_Triangles.size();f++)
				for(int k=0;k<m_Triangles[f].size();k++)
					for(int j=0;j<m_Triangles[f][k].size();j++)
					{
						Triangle & triangle = m_Triangles[f][k][j];
						for(int v=0;v<3;v++)
							if ( triangle.verts[v] > i )
								triangle.verts[v]--;
					}
		}
	}

#ifdef _DEBUG
	TRACE( CharString().format("NodeComplexMesh2::optimize(), culled %d verts", culledCount ) );
#endif

	TRACE( "NodeComplexMesh2::optimize(), sorting triangles..." );

	// sort triangles, try to sort them so the next triangles uses as many verticies of the previous triangle
	// this helps with the low-level draw routines, because verts are more likely to still be in the cache
	int f = 0;
	for(;f<m_Triangles.size();f++)
		for(int m=0;m<m_Triangles[f].size();m++)
		{
			if ( m_Triangles[f][m].size() == 0 )
				continue;		// no triangles to sort

			int n = 0;
			int k = 0;

			Array< bool > bSorted( m_Triangles[f][m].size() );
			for(int j=0;j<bSorted.size();j++)
				bSorted[j] = false;

			Array< Triangle > sorted( m_Triangles[f][m].size() );

			// begin with triangle 0
			sorted[ n++ ] = m_Triangles[f][m][k];
			bSorted[ k ] = true;

			// process all the remaining triangles
			for(;n<sorted.size();n++)
			{
				Triangle & tk = m_Triangles[f][m][k]; 

				// find the best next triangle
				int bestTriangle = -1;
				int bestShare = -1;
				for(int j=0;j<m_Triangles[f][m].size();j++)
				{
					if ( bSorted[ j ] )
						continue;

					Triangle & tj = m_Triangles[f][m][j];

					// how many verticies does triangle k share with triangle j
					int shared = 0;
					for(int v1=0;v1<3;v1++)
						for(int v2=0;v2<3;v2++)
							if ( tk.verts[v1] == tj.verts[v2] )
								shared++;

					if ( shared > bestShare )
					{
						bestTriangle = j;
						bestShare = shared;
					}
				}
				ASSERT( bestTriangle >= 0 );

				k = bestTriangle;
				sorted[ n ] = m_Triangles[f][m][k];
				bSorted[ k ] = true;
			}

			// copy the sorted triangles back into the main array
			m_Triangles[f][m] = sorted;
		}

	TRACE( "NodeComplexMesh2::optimize(), sorting verts..." );

	Array< int > remap( m_Verts.size() );
	for(i=0;i<remap.size();i++)
		remap[ i ] = -1;
	
	int n = 0;
	Array< Vertex > sorted( m_Verts.size() );

	for(f=0;f<m_Triangles.size();f++)
		for(int m=0;m<m_Triangles[f].size();m++)
			for(int k=0;k<m_Triangles[f][m].size();k++)
			{
				Triangle & fmk = m_Triangles[f][m][k]; 
				for(int v=0;v<3;v++)
				{
					if ( remap[ fmk.verts[v] ] < 0 )
					{
						remap[ fmk.verts[v] ] = n;
						sorted[ n++ ] = m_Verts[ fmk.verts[v] ];
					}

					fmk.verts[v] = remap[ fmk.verts[v] ];
				}
			}

	m_Verts = sorted;
}

void NodeComplexMesh2::invalidate()
{
	m_Primitives.release();
}

void NodeComplexMesh2::scale( float scale )
{
	for(int i=0;i<m_Verts.size();i++)
	{
		Vertex & vert = m_Verts[i];
		vert.position = vert.position * scale;
	}

	calculateHull();
}

void NodeComplexMesh2::release()
{
	invalidate();

	m_NodeFrames.release();
	m_Verts.release();
	m_Materials.release();
	m_Triangles.release();
	m_Hull.resetBox();
}

void NodeComplexMesh2::buildDecal( NodeComplexMesh2 * pMesh, const Vector3 & p, const Vector3 & n, const Vector3 & t, 
							float width, float height, float depth, Material * pMaterial )
{
	// release this entire mesh
	release();

	// get the bi-normal
	Vector3 bn( n & t );		

	// generate the 6 planes used to clip the triangles of the source mesh
	Plane clipPlanes[ 6 ];

	float d = p | t;
	clipPlanes[0].set( t, width * 0.5f - d );		// left plane
	clipPlanes[1].set( -t, width * 0.5f + d );		// right plane
	
	d = p | bn;
	clipPlanes[2].set( bn, height * 0.5f - d );		// bottom plane
	clipPlanes[3].set( -bn, height * 0.5f + d );	// top plane

	d = p | n;
	clipPlanes[4].set( -n, depth + d );				// front plane
	clipPlanes[5].set( n, depth - d );				// back plane

	// push the decal material
	m_Materials.push( pMaterial );

	// clip all the triangles of the source mesh against the 6 planes
	m_Triangles.allocate( 1 );
	m_Triangles[0].allocate( 1 );

	for(int m=0;m<pMesh->materialCount();m++)
		for(int t=0;t<pMesh->triangleCount( 0, m );t++)
		{
			const Triangle & triangle = pMesh->triangle( 0, m, t );

			Array< Vertex > verts;
			verts.push( pMesh->vertex( triangle.verts[ 0 ] ) );
			verts.push( pMesh->vertex( triangle.verts[ 1 ] ) );
			verts.push( pMesh->vertex( triangle.verts[ 2 ] ) );

			// check the normal of the triangle
			Vector3 tn( (verts[1].position - verts[0].position) & (verts[2].position - verts[0].position) );

			// ignore this triangle if it's facing the other direction
			if ( (n | tn) > 0.0f )
			{
				// clip this triangle against the planes
				for(int p=0;p<6;p++)
				{
					Array< Vertex > clipped;
					clipAgainstPlane( clipPlanes[p], verts, clipped );

					// copy over array and clip against the next plane
					verts = clipped;

					if ( verts.size() == 0 )
						break;		// all verticies clipped, stop
				}

				// add the triangles to this mesh
				for(int i=1;i<verts.size() - 1;i++)
				{
					Triangle newTriangle;
					newTriangle.verts[0] = addVertex( verts[0] );
					newTriangle.verts[1] = addVertex( verts[i] );
					newTriangle.verts[2] = addVertex( verts[i+1] );

					addTriangle( 0, 0, newTriangle );
				}
			}
		}

	// assign texture mapping coordinates
	float rw = 1.0f / width;
	float rh = 1.0f / height;

	for(int i=0;i<m_Verts.size();i++)
	{
		Vector3 d = m_Verts[i].position - p;
		m_Verts[i].u = (d | t) * rw + 0.5f;
		m_Verts[i].v = (d | bn) * rh + 0.5f;
	}

	calculateHull();
}

//----------------------------------------------------------------------------

void NodeComplexMesh2::clipAgainstPlane( const Plane & p, const Array< Vertex > & verts, Array< Vertex > & clipped )
{
	Array< float > dot( verts.size() );

	// classify all the vertices
	bool negative = true;
	bool positive = true;

	int i = 0;
	for(;i<verts.size();i++)
	{
		if ( (dot[i] = p.dot( verts[i].position )) < 0.0f )
			positive = false;
		else
			negative = false;
	}

	// the polygon is split if verticies found on the negative and positive side of the plane
	if ( !negative && !positive )
	{
		for(i=0;i<verts.size();i++)
		{
			int j = (i != 0) ? i - 1 : verts.size() - 1;

			if ( dot[i] < 0.0f )
			{
				if ( dot[j] > 0.0f )
				{
					// current vertex is on the negative side, previous vertex on the positive side
					float t = fabs( dot[j] ) / (fabs( dot[i] ) + fabs( dot[j] ));

					const Vertex & s = verts[ j ];		// source
					const Vertex & d = verts[ i ];		// destination

					Vertex & v = clipped.push();
					v.position = s.position + ((d.position - s.position) * t);
					v.normal = s.normal + ((d.normal - s.normal) * t);
					v.u = s.u + ((d.u - s.u) * t);
					v.v = s.v + ((d.v - s.v) * t);
				}
			}
			else
			{
				if ( dot[j] < 0.0f )
				{
					// current vertex on the positive side, previous vertex on the negative side
					float t = fabs( dot[j] ) / (fabs( dot[i] ) + fabs( dot[j] ));

					const Vertex & s = verts[ j ];		// source
					const Vertex & d = verts[ i ];		// destination

					Vertex & v = clipped.push();
					v.position = s.position + ((d.position - s.position) * t);
					v.normal = s.normal + ((d.normal - s.normal) * t);
					v.u = s.u + ((d.u - s.u) * t);
					v.v = s.v + ((d.v - s.v) * t);
				}

				// include current vertex
				clipped.push( verts[i] );
			}
		}
	}
	else if ( positive )		// all verticies on the positive side, no clipping
		clipped = verts;	
}

//-------------------------------------------------------------------------------

void NodeComplexMesh2::createPrimitives( DisplayDevice * pDisplay )
{
	PrimitiveTriangleListI * pFirst = NULL;

	m_Primitives.allocate( m_Triangles.size() );
	for(int f=0;f<m_Triangles.size();f++)
	{
		m_Primitives[f].allocate( m_Triangles[f].size() );
		for(int i=0;i<m_Triangles[f].size();i++)
		{
			if ( m_Triangles[f][i].size() > 0 )
			{
				if ( pFirst == NULL )
				{
					word * pTriangles = (word *)m_Triangles[f][i].buffer();
					Vertex * pVerts = m_Verts.buffer();

					pFirst = m_Primitives[f][i] = PrimitiveTriangleListI::create( pDisplay, 
							m_Triangles[f][i].size(), pTriangles, m_Verts.size(), pVerts );

					delete pTriangles;
					delete pVerts;
				}
				else
				{
					word * pTriangles = (word *)m_Triangles[f][i].buffer();

					m_Primitives[f][i] = PrimitiveTriangleListI::create( pDisplay, 
							m_Triangles[f][i].size(), pTriangles, pFirst );

					delete pTriangles;
				}
			}
		}
	}

}

//----------------------------------------------------------------------------
// EOF

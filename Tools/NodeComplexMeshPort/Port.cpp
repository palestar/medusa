/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODECOMPLEXMESHPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"

#include "File/Path.h"
#include "Render3d/NodeComplexMesh.h"

#include "Tools/ScenePort/ChildFrame.h"
#include "Tools/ScenePort/Port.h"
#include "Tools/ResourcerDoc/Port.h"

//----------------------------------------------------------------------------

struct ObsoleteVertex
{
	Vector3	position;
	Color	diffuse;	
	Vector3	normal;
};

IMPLEMENT_RAW_STREAMING( ObsoleteVertex );

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeComplexMeshPort, NodePort );
REGISTER_FACTORY_KEY(  NodeComplexMeshPort, 4009789493766360222 );

BEGIN_PROPERTY_LIST( NodeComplexMeshPort, NodePort );
	ADD_PROPERTY( m_bExportNodeFrames );
	ADD_PROPERTY( m_NodeFrames );
	ADD_PROPERTY( m_Verts );
	ADD_PORT_PROPERTY( m_Materials, MaterialPort );
	ADD_PROPERTY( m_Triangles );
	ADD_PROPERTY( m_Fps );
	ADD_PROPERTY( m_fMeshScale );
	ADD_PROPERTY( m_Reverse );
	ADD_PROPERTY( m_Optimize );
	ADD_PROPERTY( m_CalcNormals );
	ADD_PROPERTY( m_EnableLOD );
	ADD_PROPERTY( m_MinLOD );
	ADD_PROPERTY( m_MaxLOD );
END_PROPERTY_LIST();

NodeComplexMeshPort::NodeComplexMeshPort() : NodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeComplexMesh2);

	m_Fps = 15.0f;
	m_fMeshScale = 1.0f;
	m_Reverse = false;
	m_Optimize = false;
	m_CalcNormals = false;
	m_EnableLOD = false;
	m_MinLOD = 1.0f;
	m_MaxLOD = 750.0f;
	m_bExportNodeFrames = false;
}

//-------------------------------------------------------------------------------

const int VERSION_080299 = 1;
const int VERSION_052500 = 52500;
const int VERSION_060100 = 60100;
const int VERSION_061200 = 61200;
const int VERSION_121200 = 121200;
const int VERSION_032001 = 32001;
const int VERSION_032901 = 32901;
const int VERSION_033001 = 33001;
const int VERSION_20050110 = 20050110;

bool NodeComplexMeshPort::read( const InStream & input )
{
	if (! NodePort::read( input ) )
	{
		int version;
		input >> version;

		// obsolete data
		int m_LOD;
		float m_MaxDetailRange;
		float m_MinDetailRange;

		switch( version )
		{
		case VERSION_20050110:
			input >> m_bExportNodeFrames;
			input >> m_NodeFrames;
			input >> m_Verts;
			input >> m_Materials;
			input >> m_Triangles;
			input >> m_Fps;
			input >> m_fMeshScale;
			input >> m_Reverse;
			input >> m_Optimize;
			input >> m_CalcNormals;
			input >> m_EnableLOD;
			input >> m_MinLOD;
			input >> m_MaxLOD;
			break;
		case VERSION_033001:
			input >> m_NodeFrames;
			input >> m_Verts;
			input >> m_Materials;
			input >> m_Triangles;
			input >> m_Fps;
			input >> m_fMeshScale;
			input >> m_Reverse;
			input >> m_Optimize;
			input >> m_CalcNormals;
			input >> m_EnableLOD;
			input >> m_MinLOD;
			input >> m_MaxLOD;
			break;
		case VERSION_032901:
			input >> m_NodeFrames;
			input >> m_Verts;
			input >> m_Materials;
			input >> m_Triangles;
			input >> m_Fps;
			input >> m_fMeshScale;
			input >> m_Reverse;
			input >> m_Optimize;
			input >> m_EnableLOD;
			input >> m_MinLOD;
			input >> m_MaxLOD;
			m_CalcNormals = false;
			break;
		case VERSION_032001:
			input >> m_NodeFrames;
			input >> m_Verts;
			input >> m_Materials;
			input >> m_Triangles;
			input >> m_Fps;
			input >> m_Reverse;
			input >> m_Optimize;
			input >> m_EnableLOD;
			input >> m_MinLOD;
			input >> m_MaxLOD;
			m_fMeshScale = 1.0f;
			m_CalcNormals = false;
			break;
		case VERSION_121200:
			input >> m_NodeFrames;
			input >> m_Verts;
			input >> m_Materials;
			input >> m_Triangles;
			input >> m_Fps;
			input >> m_Reverse;
			input >> m_Optimize;
			m_EnableLOD = false;
			m_MinLOD = 1.0f;
			m_MaxLOD = 750.0f;
			m_fMeshScale = 1.0f;
			m_CalcNormals = false;
			break;
		case VERSION_061200:
			{
				Array< NodeComplexMesh::Vertex >			verts;
				Array< Array< NodeComplexMesh::Triangle > >	triangles;

				input >> m_NodeFrames;
				input >> verts;
				input >> m_Materials;
				input >> triangles;
				input >> m_Fps;
				input >> m_Reverse;
				input >> m_Optimize;

				import( verts, triangles ); 
			}
			m_EnableLOD = false;
			m_MinLOD = 1.0f;
			m_MaxLOD = 750.0f;
			m_fMeshScale = 1.0f;
			m_CalcNormals = false;
			break;
		case VERSION_060100:
			{
				Array< NodeComplexMesh::Vertex >			verts;
				Array< Array< NodeComplexMesh::Triangle > >	triangles;

				input >> m_NodeFrames;
				input >> verts;
				input >> m_Materials;
				input >> triangles;
				input >> m_Fps;
				m_Reverse = false;
				m_Optimize = false;

				import( verts, triangles ); 
			}
			m_EnableLOD = false;
			m_MinLOD = 1.0f;
			m_MaxLOD = 750.0f;
			m_fMeshScale = 1.0f;
			m_CalcNormals = false;
			break;
		case VERSION_052500:
			{
				Array< NodeComplexMesh::Vertex >			verts;
				Array< Array< NodeComplexMesh::Triangle > >	triangles;

				input >> m_NodeFrames;
				input >> verts;
				input >> m_Materials;
				input >> triangles;
				input >> m_Fps;
				input >> m_LOD;
				input >> m_MaxDetailRange;
				input >> m_MinDetailRange;
				m_Reverse = false;
				m_Optimize = false;

				import( verts, triangles ); 
			}
			m_EnableLOD = false;
			m_MinLOD = 1.0f;
			m_MaxLOD = 750.0f;
			m_fMeshScale = 1.0f;
			m_CalcNormals = false;
			break;
		case VERSION_080299:
			{
				Array< ObsoleteVertex >						oldVerts;
				Array< NodeComplexMesh::Vertex >			verts;
				Array< Array< NodeComplexMesh::Triangle > >	triangles;

				input >> m_NodeFrames;
				input >> oldVerts;
				input >> m_Materials;
				input >> triangles;
				input >> m_Fps;
			
				BoxHull hull;

				verts.allocate( oldVerts.size() );
				for(int j=0;j<verts.size();j++)
				{
					ObsoleteVertex & srcVertex = oldVerts[ j ];
					NodeComplexMesh::Vertex & dstVertex = verts[ j ];

					dstVertex.position = srcVertex.position;
					dstVertex.normal = srcVertex.normal;

					// update the hull
					hull.addVertex( dstVertex.position );
				}

				import( verts, triangles ); 
			}
			m_Reverse = false;
			m_Optimize = false;
			m_EnableLOD = false;
			m_MinLOD = 1.0f;
			m_MaxLOD = 750.0f;
			m_fMeshScale = 1.0f;
			m_CalcNormals = false;
			break;
		default:
			return false;
		}
		
		for(int i=0;i<m_Verts.size();i++)
			m_Verts[i].normal.normalize();

		// cull the materials
		for(int i=m_Materials.size()-1;i>=0;i--)
		{
			bool used = false;		// how many triangles use this material, if any leave the material attached
			for(int f=0;f<m_Triangles.size() && used == false;f++)
				if ( m_Triangles[f][i].size() > 0 )
					used = true;

			if (! used )
			{
				//((ScenePort *)parent())->pushMessage( CharString().format("%s; Removing unused material %s", GetPathName(), m_Materials[ i ]) );

				// material i is not used, remove it
				for(int f=0;f<m_Triangles.size();f++)
					m_Triangles[f].remove( i );
				m_Materials.remove( i );
			}
		}
		return true;
	}

	return true;
}

//-------------------------------------------------------------------------------

bool NodeComplexMeshPort::import( const char * fileName )
{
	Path path( fileName );
	CharString extension( path.extension() );

	if ( extension.compareNoCase(".LWO") == 0 )
		return importLWO( fileName );

	return false;
}

void NodeComplexMeshPort::dependencies( DependentArray & dep )
{
	for(int i=0;i<m_Materials.size();i++)
		dep.push( m_Materials[i] );
}

CFrameWnd * NodeComplexMeshPort::createView()
{
	NodePort::createView();

	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

BaseNode * NodeComplexMeshPort::createNode()
{
	return NodePort::createNode();
}

void NodeComplexMeshPort::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodeComplexMesh2 * pMesh = dynamic_cast<NodeComplexMesh2 *>( pNode );
	if ( pMesh != NULL )
	{
		// add the materials
		for(int i=0;i<m_Materials.size();i++)
		{
			// make sure material is used
			bool used = false;		// how many triangles use this material, if any leave the material attached
			for(int f=0;f<m_Triangles.size() && used == false;f++)
				if ( m_Triangles[f][i].size() > 0 )
					used = true;

			if ( used )
				pMesh->addMaterial( Material::Link( Port::portResource( m_Materials[i] ) ) );
		}

		// add vertices's
		pMesh->setVerts( m_Verts );
		
		// export node animation
		if ( m_bExportNodeFrames )
		{
			for(int k=0;k<m_NodeFrames.size();k++)
				pMesh->addNodeFrame( m_NodeFrames[k] );
		}

		// add triangles
		for(int f=0;f<m_Triangles.size();f++)					// enumerate frames
		{
			int m = 0;
			for(int i=0;i<m_Triangles[f].size();i++)			// enumerate materials
			{
				if ( m_Triangles[f][i].size() > 0 )
				{
					pMesh->setTriangles( f, m, m_Triangles[f][i] );
					m++;
				}
			}
		}

		if ( m_Optimize )
			pMesh->optimize();
		if ( m_Reverse )
			pMesh->reverse();

		pMesh->setFps( m_Fps );
		pMesh->scale( m_fMeshScale );
		pMesh->calculateHull();

		if ( m_CalcNormals )
			pMesh->calculateNormals();

		pMesh->setLOD( m_EnableLOD, m_MinLOD, m_MaxLOD );
	}
}

//----------------------------------------------------------------------------

void NodeComplexMeshPort::import( Array< NodeComplexMesh::Vertex > & verts, 
								Array< Array< NodeComplexMesh::Triangle > > & triangles )
{
	// allocate a single frame, no animation
	m_Triangles.allocate( 1 );								
	
	// allocate material slots
	m_Triangles[0].allocate( triangles.size() );
	for(int m=0;m<triangles.size();m++)
	{
		m_Triangles[0][m].allocate( triangles[m].size() );	// allocate triangles for this material and frame
		for(int i=0;i<triangles[m].size();i++)
		{
			NodeComplexMesh::Triangle & oldTriangle = triangles[m][i];
			NodeComplexMesh2::Triangle & newTriangle = m_Triangles[0][m][i];

			for(int j=0;j<3;j++)
			{
				// allocate a new vertex
				int index = m_Verts.size();
				NodeComplexMesh2::Vertex & newVertex = m_Verts.push();
				// store the vertex index into the new triangle
				newTriangle.verts[j] = index;
				
				// get the old vertex
				NodeComplexMesh::Vertex & oldVertex = verts[ oldTriangle.verts[j] ];
				newVertex.position = oldVertex.position;
				newVertex.normal = oldVertex.normal; //Vector3( 0, 0, 0 );
				newVertex.u = oldTriangle.uv[ j ].u;
				newVertex.v = oldTriangle.uv[ j ].v;
			}
		}
	}

	cullVerticies();
}

#undef EPS
#define EPS 0.001f

inline bool CompareVerts( const Vertex & lhs, const Vertex & rhs )
{
	return( (lhs.position - rhs.position).magnitude() < EPS &&
		fabs( lhs.u - rhs.u) < EPS &&
		fabs( lhs.v - rhs.v) < EPS );
}

void NodeComplexMeshPort::cullVerticies()
{
	Array< int >	vertexMap( m_Verts.size() );

	Array< Vertex > culled;
	for(int i=0;i<m_Verts.size();i++)
	{
		vertexMap[ i ] = -1;

		// compare vertex i to all the verts in the culled array, if any match is found then use that vertex
		for(int j=0;j<culled.size();j++)
			if ( CompareVerts( m_Verts[i], culled[j] ) )
			{
				// found match, stop searching
				vertexMap[i] = j;
				break;
			}

		if ( vertexMap[i] < 0 )
		{
			// no match found, add new vertex
			vertexMap[i] = culled.size();
			culled.push( m_Verts[i] );
		}
	}

	// update all the triangles
	for(int f=0;f<m_Triangles.size();f++)
		for(int m=0;m<m_Triangles[f].size();m++)
			for(int i=0;i<m_Triangles[f][m].size();i++)
				for(int j=0;j<3;j++)
					m_Triangles[f][m][i].verts[j] = vertexMap[ m_Triangles[f][m][i].verts[j] ];


	TRACE( CharString().format("NodeComplexMeshPort::cullVerticies, culled %d, verts = %d", m_Verts.size() - culled.size(), culled.size()) );
	// replace old vertex array
	m_Verts = culled;
}

void NodeComplexMeshPort::transformVerts()
{
	// transform our verts
	for(int i=0;i<m_Verts.size();++i)
		m_Verts[i].position = m_Frame % m_Verts[i].position;
	m_Frame = Matrix33::IDENTITY;
}

//-------------------------------------------------------------------------------
// EOF


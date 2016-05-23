/*
	NodeComplexMesh2.h

	This class is a rewrite of the original NodeComplexMesh class. This class is used to display 
	a triangle mesh with multiple materials. Unlike NodeMesh which can only have a single material 
	for all triangles.

	* Added vertex animation support
	* Triangle indexes are pre-created - faster initialzation of the PrimitiveTriangleListI object
	* TODO: Add progressive mesh support

	This node supports 2 types of animations... vertex animation and node animation. NodeTransform animation
	just affects the position and frame of the entire node.
	
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_COMPLEX_MESH2_H
#define NODE_COMPLEX_MESH2_H

#include "Display/PrimitiveTriangleList.h"
#include "Math/BoxHull.h"

#include "Render3D/NodeTransform.h"
#include "Render3D/Material.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeComplexMesh2 : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeComplexMesh2>			Ref;
	typedef ::Vertex							Vertex;

	struct Frame					// simple node animation information
	{
		Matrix33		frame;		// TODO: use quanternion or euler
		Vector3			position;
	};
	struct Triangle
	{
		word			verts[ 3 ];
	};

	// Construction
	NodeComplexMesh2();

	// NodeTransform Interface
	BoxHull				hull() const;

	void				render( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );
	void				preRender( RenderContext &context, 
							const Matrix33 & frame, 
							const Vector3 & position );

	// Accessors
	float				fps() const;

	int					nodeFrameCount() const;					// number of node animation frames
	const Frame &		nodeFrame( int n ) const;				

	int					vertexCount() const;
	const Vertex &		vertex( int n ) const;

	int					frameCount() const;						// number of vertex animation frames
	int					materialCount() const;
	Material *			material( int n ) const;

	int					triangleCount( int f, int m ) const;
	const Triangle &	triangle( int f, int m, int n ) const;

	int					triangleCount() const;					// get the total triangle count

	bool				enableLOD() const;						// are levels of detail enabled
	float				minLOD() const;							// min lod detail range
	float				maxLOD() const;							// max lod detail range

	// Mutators
	void				calculateHull();						// calculate the bounding box for this node
	void				calculateNormals();						// calculate the vertex normals
	void				setFps( float fps );					// set this nodes animation rate
	void				setLOD( bool enable, float min, float max );

	int					addNodeFrame( const Frame & frame );
	int					addVertex( const Vertex & vertex, bool cull = false );
	int					addMaterial( Material::Link pMaterial );
	int					addTriangle( int f, int m, const Triangle & triangle );
	int					addTriangles( int f, int m, const Array< Triangle > & triangles );

	void				setVerts( const Array< Vertex > & verts );
	void				setTriangles( int f, int m, const Array< Triangle > & triangles );

	void				reverse();
	void				optimize();								// optimize this mesh
	void				invalidate();							// invalidate the primitives
	void				scale( float scale );					// scale this mesh verticies
	void				release();								// release this mesh

	void				buildDecal( NodeComplexMesh2 * pMesh, const Vector3 & p, const Vector3 & n, const Vector3 & t, 
							float width, float height, float depth, Material * pMaterial );

	// Static
	static void			clipAgainstPlane( const Plane & p, const Array< Vertex > & verts, Array< Vertex > & clipped );
	static bool			sm_bShowVertexNormals;

private:
	// Mutators
	void				createPrimitives( DisplayDevice * pDisplay );
	bool				compareVerts( const Vertex & lhs, const Vertex & rhs );

	// Types
	typedef Array< Triangle >				TriangleList;

	// Data
	Array< Frame >		m_NodeFrames;
	Array< Vertex >		m_Verts;

	Array< Material::Link >	
						m_Materials;
	Array< Array< TriangleList > >
						m_Triangles;							// m_Triangles[frame][material]				
	
	BoxHull				m_Hull;
	float				m_Fps;		

	bool				m_EnableLOD;
	float				m_MinLOD, m_MaxLOD;

	// device objects created by createPrimitives()
	Array< Array< PrimitiveTriangleListI::Ref > >
						m_Primitives;							// m_Primitives[frame][material]
};

//----------------------------------------------------------------------------

IMPLEMENT_RAW_STREAMING( NodeComplexMesh2::Frame );
IMPLEMENT_RAW_STREAMING( NodeComplexMesh2::Triangle );

//----------------------------------------------------------------------------

#undef EPS
#define EPS 0.001f

inline bool NodeComplexMesh2::compareVerts( const Vertex & lhs, const Vertex & rhs )
{
	return( (lhs.position - rhs.position).magnitude() < EPS &&
		fabs( lhs.u - rhs.u) < EPS &&
		fabs( lhs.v - rhs.v) < EPS );
}

//----------------------------------------------------------------------------

inline float NodeComplexMesh2::fps() const
{
	return( m_Fps );
}

inline int NodeComplexMesh2::nodeFrameCount() const
{
	return m_NodeFrames.size();
}

inline const NodeComplexMesh2::Frame & NodeComplexMesh2::nodeFrame( int n ) const
{
	return m_NodeFrames[ n ];
}

inline int NodeComplexMesh2::vertexCount() const
{
	return( m_Verts.size() );
}

inline const Vertex & NodeComplexMesh2::vertex( int n ) const
{
	return( m_Verts[ n ] );
}

inline int	NodeComplexMesh2::frameCount() const
{
	return( m_Triangles.size() );
}

inline int NodeComplexMesh2::materialCount() const
{
	return( m_Materials.size() );
}

inline Material * NodeComplexMesh2::material( int n ) const
{
	return( m_Materials[n] );
}

inline int NodeComplexMesh2::triangleCount( int frame, int material ) const
{
	return( m_Triangles[frame][material].size() );
}

inline const NodeComplexMesh2::Triangle & NodeComplexMesh2::triangle( int frame, int material, int n ) const
{
	return( m_Triangles[frame][material][n] );
}

inline int NodeComplexMesh2::triangleCount() const
{
	int count = 0;
	for(int j=0;j<m_Triangles.size();j++)
		for(int i=0;i<m_Triangles[j].size();i++)
			count += m_Triangles[j][i].size();

	return count;
}

inline bool NodeComplexMesh2::enableLOD() const
{
	return m_EnableLOD;
}

inline float NodeComplexMesh2::minLOD() const
{
	return m_MinLOD;
}

inline float NodeComplexMesh2::maxLOD() const
{
	return m_MaxLOD;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

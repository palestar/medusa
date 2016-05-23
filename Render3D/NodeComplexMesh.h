/*
	NodeComplexMesh.h

	This node type is a re-write of the previous NodeMesh object, with multi-material
	capability, and some optimizations.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_COMPLEX_MESH_H
#define NODE_COMPLEX_MESH_H

#include "Display/PrimitiveTriangleList.h"
#include "Math/BoxHull.h"

#include "Render3D/NodeTransform.h"
#include "Render3D/NodeMesh.h"
#include "Render3D/Material.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeComplexMesh : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeComplexMesh>			Ref;

	struct Frame					// animation structure
	{
		Matrix33		m_Frame;	// TODO: use quanternion or euler
		Vector3			m_Position;
	};
	struct Vertex
	{
		Vector3			position;
		Vector3			normal;

		bool operator==( const Vertex & compare );
	};
	struct UV
	{
		float			u,v;
	};
	struct Triangle
	{
		int				verts[ 3 ];
		UV				uv[ 3 ];		// uv's
	};

	// Construction
	NodeComplexMesh();
	NodeComplexMesh( const NodeMesh & mesh );
	NodeComplexMesh( const NodeComplexMesh & mesh );

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

	int					frameCount() const;
	const Frame &		frame( int n ) const;
	int					vertexCount() const;
	const Vertex &		vertex( int n ) const;
	int					triangleCount( int material ) const;
	const Triangle &	triangle( int material, int n ) const;
	int					materialCount() const;
	Material *			material( int n ) const;

	int					triangleCount() const;					// get the total triangle count

	// Mutators
	void				calculateHull();						// calculate the bounding box for this node
	void				calculateNormals();						// calculate the vertex normals
	void				setFps( float fps );					// set this nodes animation rate

	int					addFrame( const Frame & frame );
	int					addVertex( const Vertex & vertex, bool cull = false );
	int					addVertex( const NodeComplexMesh & mesh );
	int					addMaterial( Material::Link pMaterial );
	int					addMaterial( const NodeComplexMesh & mesh );
	int					addTriangle( int material, const Triangle & triangle );

	void				add( const NodeComplexMesh & mesh,
							const Matrix33 & rotation,
							const Vector3 & translation );		// merge another mesh into this one
	void				add( const NodeMesh & mesh,
							const Matrix33 & rotation,
							const Vector3 & translation );	

	void				reverse();
	void				optimize();								// optimize this mesh into triangle strips
	void				invalidate();							// invalidate the primitives
	void				scale( float scale );					// scale this mesh verticies
	void				release();								// release this mesh

private:
	// Mutators
	bool				optimizeTriangle( int material, int k );	// attempt to merge triangle k
	void				createPrimitives( DisplayDevice * pDisplay );
	
	// Data
	Array< Frame >		m_Frames;
	Array< Vertex >		m_Verts;

	Array< Material::Link >	
						m_Materials;
	Array< Array< Triangle > >
						m_Triangles;						
	
	BoxHull				m_Hull;
	float				m_Fps;		

	// device objects created by createPrimitives()
	Array< PrimitiveTriangleListI::Ref >
						m_Primitives;							// m_Primitives.size() == m_Materials.size()
};

//----------------------------------------------------------------------------

IMPLEMENT_RAW_STREAMING( NodeComplexMesh::Frame );
IMPLEMENT_RAW_STREAMING( NodeComplexMesh::Vertex );
IMPLEMENT_RAW_STREAMING( NodeComplexMesh::UV );
IMPLEMENT_RAW_STREAMING( NodeComplexMesh::Triangle );

//----------------------------------------------------------------------------

#undef EPS
#define EPS 0.001f

inline bool NodeComplexMesh::Vertex::operator==( const NodeComplexMesh::Vertex & compare )
{
	return( (position - compare.position).magnitude() < EPS );
}

//----------------------------------------------------------------------------

inline float NodeComplexMesh::fps() const
{
	return( m_Fps );
}

inline int	NodeComplexMesh::frameCount() const
{
	return( m_Frames.size() );
}

inline const NodeComplexMesh::Frame & NodeComplexMesh::frame( int n ) const
{
	return( m_Frames[ n ] );
}

inline int NodeComplexMesh::vertexCount() const
{
	return( m_Verts.size() );
}

inline const NodeComplexMesh::Vertex & NodeComplexMesh::vertex( int n ) const
{
	return( m_Verts[ n ] );
}

inline int NodeComplexMesh::triangleCount( int material ) const
{
	return( m_Triangles[material].size() );
}

inline const NodeComplexMesh::Triangle & NodeComplexMesh::triangle( int material, int n ) const
{
	return( m_Triangles[material][n] );
}

inline int NodeComplexMesh::materialCount() const
{
	return( m_Materials.size() );
}

inline Material * NodeComplexMesh::material( int n ) const
{
	return( m_Materials[n] );
}

inline int NodeComplexMesh::triangleCount() const
{
	int count = 0;
	for(int i=0;i<m_Materials.size();i++)
		count += m_Triangles[i].size();

	return count;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

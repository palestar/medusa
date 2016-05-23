/*
	NodeMesh.h

	This is the high level polygon object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODE_MESH_H
#define NODE_MESH_H

#include "Display/PrimitiveTriangleList.h"
#include "Math/SphericalHull.h"
#include "Math/BoxHull.h"

#include "Render3D/NodeTransform.h"
#include "Render3D/Material.h"
#include "Render3D/Render3dDll.h"

//----------------------------------------------------------------------------

class DLL NodeMesh : public NodeTransform
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeMesh>			Ref;

	struct Frame
	{
		Matrix33		m_Frame;	// TODO: use quanternion or euler
		Vector3			m_Position;
	};

	struct Vertex
	{
		Vector3			position;
		Color			diffuse;	
		Vector3			normal;		// Calculated
	};
	struct UV
	{
		float			u,v;
	};
	struct Triangle
	{
		int				vertex[ 3 ];
		UV				uv[ 3 ];

		Plane			plane;		// Calculated
		Vector3			center;		// Calculated
		Vector3			normal;		// Calculated
	};

	// Construction
	NodeMesh();

	// NodeTransform Interface
	BoxHull				hull() const;
	void				render( RenderContext &context, const Matrix33 & frame, const Vector3 & position );
	void				preRender( RenderContext &context, const Matrix33 & frame, const Vector3 & position );

	// Accessors
	int					frameCount() const;
	const Frame &		frame( int n ) const;
	int					vertexCount() const;
	const Vertex &		vertex( int n ) const;
	int					triangleCount() const;
	const Triangle &	triangle( int n ) const;

	Material *			material() const;

	// Mutators
	int					addFrame( const Frame & frame );
	void				removeFrame( int n );

	int					addVertex( const Vector3 & position, const Color & diffuse );
	int					addVertex( const Vector3 & position, const Color & diffuse, const Vector3 &normal );
	void				removeVertex( int n );

	int					addTriangle( const int * pVertex, const UV * pUV );
	int					addTriangles( int triangleCount, const int * pVertex, const UV * pUV );
	void				removeTriangle( int n );
	
	void				setMaterial( Material::Link material );

	void				update( bool snapVerts = true, bool DoNormals = true, bool DoVertexNormals = true );
	void				merge( const Matrix33 &frame, const Vector3 &offset, const NodeMesh & merge );
	void				invalidate();		// releases device objects
	void				release();

	// Simple objects
	void				buildCube( const Vector3 &min, const Vector3 &max, bool mapToWorld = false );
	void				buildPyramid( const Vector3 &top, const Vector3 &bottomCenter, float base, bool mapToWorld = false );
	void				buildCylinder( const Vertex &center, float radius, float height, int detail, bool mapToWorld = false );
	void				buildSphere( const Vector3 &center, float radius, int detail );
	void				buildPlane( const Vector3 &size, int width, int height );

private:

	// Data
	Array< Frame >		m_Frames;
	Array< Vertex >		m_Vertex;
	Array< Triangle >	m_Triangle;
	Material::Link		m_Material;
	BoxHull				m_BoxHull;
	float				m_fRadius;

	// device object, created when render() is called
	PrimitiveTriangleListI::Ref
						m_Primitive;			// TODO: build triangle strip/fan instead?
};

//----------------------------------------------------------------------------

IMPLEMENT_RAW_STREAMING( NodeMesh::Vertex );
IMPLEMENT_RAW_STREAMING( NodeMesh::UV );
IMPLEMENT_RAW_STREAMING( NodeMesh::Triangle );
IMPLEMENT_RAW_STREAMING( NodeMesh::Frame );

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

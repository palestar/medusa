/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODECOMPLEXMESHPORT_H
#define NODECOMPLEXMESHPORT_H

#include "Standard/Dll.h"
#include "Render3d/NodeComplexMesh.h"
#include "Render3d/NodeComplexMesh2.h"
#include "Tools/ScenePort/NodePort.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef NODECOMPLEXMESHPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeComplexMeshPort : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeComplexMeshPort>	Ref;
	typedef NodeComplexMesh2::Frame			Frame;
	typedef NodeComplexMesh2::Triangle		Triangle;
	typedef NodeComplexMesh2::Vertex		Vertex;

	// Construction
	NodeComplexMeshPort();

	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	bool						import( const char * file );
	void						dependencies( DependentArray & dep );
	
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	// PortImportLWO.cpp
	bool						importLWO( const char * fileName );
	void						decodeLWOB( const char * fileName, byte * begin, byte * end );
	void						decodeLWO2( const char * fileName, byte * begin, byte * end );

	// Types
	typedef Array< Triangle >	TriangleList;

	// Data
	bool						m_bExportNodeFrames;
	Array< Frame >				m_NodeFrames;

	Array< Vertex >				m_Verts;
	Array< CharString >			m_Materials;
	Array< Array< TriangleList > >
								m_Triangles;			// m_Triangles[frame][material]
												
	float						m_Fps;
	float						m_fMeshScale;
	bool						m_Reverse;
	bool						m_Optimize;
	bool						m_CalcNormals;

	bool						m_EnableLOD;
	float						m_MinLOD, m_MaxLOD;

	void						import( Array< NodeComplexMesh::Vertex > & verts, 
									Array< Array< NodeComplexMesh::Triangle > > & triangles );
	void						cullVerticies();	
	void						transformVerts();
};


#endif

//-------------------------------------------------------------------------------
// EOF

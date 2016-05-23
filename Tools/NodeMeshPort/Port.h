/*
	Port.h

	ResourcePort object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef NODEMESHPORT_H
#define NODEMESHPORT_H

#include "Standard/Dll.h"
#include "File/Stream.h"
#include "Render3d/NodeMesh.h"

#include "Tools/ScenePort/NodePort.h"

//----------------------------------------------------------------------------

#undef DLL
#ifdef NODEMESHPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL NodeMeshPort : public NodePort
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<NodeMeshPort>	Ref;
	typedef NodeMesh::Frame			Frame;
	typedef NodeMesh::Triangle		Triangle;
	typedef NodeMesh::Vertex		Vertex;

	// Construction
	NodeMeshPort();
	// Widget Interface
	bool						read( const InStream & input );
	// NodePort Interface
	void						dependencies( DependentArray & dep );
	CFrameWnd *					createView();
	BaseNode *					createNode();
	void						initializeNode( BaseNode * pNode );

	void						createNodeView();

	// Data
	Array< Triangle >			m_Triangles;
	Array< Vertex >				m_Verticies;
	Array< Frame >				m_Frames;

	CharString					m_MaterialPort;

	bool						m_MergeChildren;
	bool						m_SnapVerts;
};

#endif

//-------------------------------------------------------------------------------
// EOF

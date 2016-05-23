/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODEMESHPORT_DLL
#include "stdafx.h"

#include "PortView.h"
#include "Port.h"

#include "Tools/ScenePort/ChildFrame.h"
#include "Tools/ScenePort/Port.h"
#include "Tools/ResourcerDoc/ResourcerDoc.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( NodeMeshPort, NodePort );
REGISTER_FACTORY_KEY(  NodeMeshPort, 3934837122061263856 );

BEGIN_PROPERTY_LIST( NodeMeshPort, NodePort );
	ADD_PROPERTY( m_Frames );
	ADD_PROPERTY( m_Triangles );
	ADD_PROPERTY( m_Verticies );
	ADD_PROPERTY( m_MaterialPort );
	ADD_PROPERTY( m_MergeChildren );
	ADD_PROPERTY( m_SnapVerts );
END_PROPERTY_LIST();

NodeMeshPort::NodeMeshPort() : NodePort()
{
	m_Class = m_Type = CLASS_KEY(NodeMesh);
	m_MergeChildren = false;
	m_SnapVerts = true;
}

//-------------------------------------------------------------------------------

const int VERSION_012299 = 1;

bool NodeMeshPort::read( const InStream & input )
{
	if (! NodePort::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
			case VERSION_012299:
				input >> m_Frames;
				input >> m_Triangles;
				input >> m_Verticies;
				input >> m_MaterialPort;
				input >> m_MergeChildren;
				input >> m_SnapVerts;
				break;
			default:
				return false;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------

void NodeMeshPort::dependencies( DependentArray & dep )
{
	dep.push( m_MaterialPort );
}

CFrameWnd * NodeMeshPort::createView()
{
	// call the base class
	NodePort::createView();

	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

BaseNode * NodeMeshPort::createNode()
{
	return NodePort::createNode();
}

void NodeMeshPort::initializeNode( BaseNode * pNode )
{
	NodePort::initializeNode( pNode );

	NodeMesh * pMesh = dynamic_cast<NodeMesh *>( pNode );
	if ( pMesh != NULL )
	{
		for(int j=0;j<m_Frames.size();j++)
			pMesh->addFrame( m_Frames[j] );

		for(int k=0;k<m_Verticies.size();k++)
			pMesh->addVertex( m_Verticies[k].position, m_Verticies[k].diffuse );

		for(int i=0;i<m_Triangles.size();i++)
			pMesh->addTriangle( m_Triangles[i].vertex, m_Triangles[i].uv );

		pMesh->setMaterial( Material::Link( Port::portResource( m_MaterialPort ) ) );
		pMesh->update( m_SnapVerts );
	}
}

void NodeMeshPort::createNodeView()
{
	NodePort::createView();
}

//-------------------------------------------------------------------------------
// EOF


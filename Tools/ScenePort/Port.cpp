/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "stdafx.h"
#include "Port.h"
#include "ChildFrame.h"
 
#include "File/FileDisk.h"
#include "Standard/StringHash.h"
#include "File/Path.h"

#include "Tools/ImagePort/Port.h"
#include "Tools/MaterialPort/Port.h"
#include "Tools/ResourcerDoc/ResourcerDoc.h"
//#include "Tools/NodeMaterialPort/Port.h"

#include "Resource.h"

//---------------------------------------------------------------------------------------------------

ScenePort::Importer::Importer()
{
	ScenePort::sm_Importers.insert( this );
}

ScenePort::Importer::~Importer()
{
	ScenePort::sm_Importers.erase( this );
}

//----------------------------------------------------------------------------

ScenePort::ImporterSet ScenePort::sm_Importers;

IMPLEMENT_FACTORY( ScenePort, Port );
REGISTER_FACTORY_KEY(  ScenePort, 3925323079376588003 );

BEGIN_PROPERTY_LIST( ScenePort, Port );
	ADD_PROPERTY( m_Root );
	ADD_PROPERTY( m_Fps );
	ADD_PROPERTY( m_Segments );
	ADD_PROPERTY( m_TargetPosition );
	ADD_PROPERTY( m_TargetDistance );
	ADD_PROPERTY( m_TargetFrame );
END_PROPERTY_LIST();

ScenePort::ScenePort() : Port()
{
	m_Root = new NodePort;
	m_Fps = 15.0f;
	m_Bits = 0;

	m_TargetPosition = Vector3(0,0,0);
	m_TargetDistance = 50.0f;
	m_TargetFrame.identity();
	m_pSelectedNode = m_Root;
	m_CurrentSegment = -1;
	m_bKeyConflicts = false;

	m_Root->setParent( this );
	m_Nodes = m_Root->updateNodeCache();
}

//-------------------------------------------------------------------------------

const int VERSION_030999 = 1;
const int VERSION_031199 = 2;
const int VERSION_072999 = 3;

bool ScenePort::read( const InStream & input )
{
	if (! Port::read( input ) )
	{
		int version;
		input >> version;

		// obsolete data members
		float					m_Time;
		Array< Flag >			m_Flags;

		switch( version )
		{
		case VERSION_072999:
			input >> m_Root;
			input >> m_Fps;
			input >> m_Segments;

			input >> m_TargetPosition;
			input >> m_TargetDistance;
			input >> m_TargetFrame;
			break;

		case VERSION_031199:
			input >> m_Flags;
			input >> m_Root;
			input >> m_Bits;
			input >> m_Time;
			input >> m_Fps;

			input >> m_Segments;

			input >> m_TargetPosition;
			input >> m_TargetDistance;
			input >> m_TargetFrame;
			break;

		case VERSION_030999:
			input >> m_Root;
			input >> m_Bits;
			input >> m_Time;
			input >> m_Fps;

			input >> m_Segments;

			input >> m_TargetPosition;
			input >> m_TargetDistance;
			input >> m_TargetFrame;
			break;
		}
	}

	m_Bits = 0;
	m_pSelectedNode = m_Root;
	m_CurrentSegment = -1;

	// convert all root nodes from BaseNodePort to NodePort..
	if ( WidgetCast<NodePort>( m_Root ) == NULL )
	{
		BaseNodePort::Ref pNewRoot = new NodePort();
		pNewRoot->m_NodeKey = m_Root->m_NodeKey;
		pNewRoot->m_Type = m_Root->m_Type;
		if ( m_Root->m_Class != CLASS_KEY(BaseNode) )
			pNewRoot->m_Class = m_Root->m_Class;
		pNewRoot->m_Name = m_Root->m_Name;
		pNewRoot->m_pTemplate = m_Root->m_pTemplate;
		pNewRoot->m_Children = m_Root->m_Children;

		m_Root = pNewRoot;
	}

	m_Root->setParent( this );
	//m_Nodes = m_Root->updateNodeCache();
	return true;
}

//----------------------------------------------------------------------------

void ScenePort::dependencies( DependentArray & dep )
{
	// get the dependencies from the hierarchy
	m_Root->recurseDependencies( dep );
}

Resource::Ref ScenePort::createResource()
{
	Scene * newScene = new Scene;
	//newScene->setKey( key() );
	
	m_bKeyConflicts = false;

	m_Nodes = m_Root->updateNodeCache( false, true );
	newScene->setNode( m_Nodes );
	newScene->setFps( m_Fps );

	// add the segments to the scene
	for(int i=0;i<m_Segments.size();i++)
	{
		Scene::Segment newSegment;
		newSegment.hashId = StringHash( m_Segments[i].id );
		newSegment.beginTime = m_Segments[i].beginTime;
		newSegment.endTime = m_Segments[i].endTime;

		newScene->addSegment( newSegment );
	}

	newScene->setDefaultCamera( m_TargetFrame, m_TargetPosition - (m_TargetFrame.k * m_TargetDistance) );

	// if we had key conflicts, re-save this scene port with the new keys ..
	if( m_bKeyConflicts )
	{
		bool bReleaseLock = false;

		if (! locked() )
		{
			FileDisk * pFile = new FileDisk();
			if ( pFile->open( GetPathName(), FileDisk::READ_WRITE ) )
			{
				setFileLock( pFile );
				bReleaseLock = true;
			}
		}

		if ( locked() )
		{
			report( "Saving due to key/name conflicts." );
			OnFileSave();
		}
		if ( bReleaseLock )
			setFileLock( NULL );
	}

	return newScene;
}

CFrameWnd * ScenePort::createView()
{
	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, RUNTIME_CLASS(CChildFrame), NULL );
    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

bool ScenePort::importFile( const char * fileName )
{
	Path path( fileName );

	for( ImporterSet::iterator iImporter = sm_Importers.begin(); 
		iImporter != sm_Importers.end(); ++iImporter )
	{
		if ( (*iImporter)->import( this, fileName ) )
			return true;
	}

	return false;
}

void ScenePort::shellOpen() const
{
	MessageBox( NULL, _T("shellOpen() is not valid for this port"), NULL, MB_OK);
}

//-------------------------------------------------------------------------------

BaseNode * ScenePort::findCachedNode( const WidgetKey & key ) const
{
	return m_Root->findCachedNode( key );
}

BaseNode * ScenePort::findCachedNode( const char * pName ) const
{
	return m_Root->findCachedNode( pName );
}

//-------------------------------------------------------------------------------
// EOF


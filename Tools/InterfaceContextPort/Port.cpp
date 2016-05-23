/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define INTERFACECONTEXTPORT_DLL
#include "stdafx.h"

#include "Port.h"
#include "PortView.h"
#include "ChildFrame.h"
#include "Resource.h"

#include "Gui3d/InterfaceContext.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( InterfaceContextPort, Port );
REGISTER_FACTORY_KEY(  InterfaceContextPort, 3979389470128564958 );

BEGIN_PROPERTY_LIST( InterfaceContextPort, Port );
	ADD_CLASSKEY_PROPERTY( m_DocumentClass, Document );
	ADD_PROPERTY( m_StartingScene );
	ADD_PROPERTY( m_SceneId );
	ADD_PROPERTY( m_Scenes );
	ADD_PROPERTY( m_Style );
	ADD_PROPERTY( m_Cursor );
END_PROPERTY_LIST();

InterfaceContextPort::InterfaceContextPort() : Port()
{
	m_StartingScene = 0;
	m_DocumentClass = CLASS_KEY( Document );
}

//-------------------------------------------------------------------------------

const int VERSION_051299 = 1;
const int VERSION_102599 = 102599;
const int VERSION_031300 = 31300;

bool InterfaceContextPort::read( const InStream & input )
{
	if (! Port::read( input ) )
	{
		int version;
		input >> version;

		switch( version )
		{
		case VERSION_031300:
			input >> m_DocumentClass;
			input >> m_StartingScene;
			input >> m_SceneId;
			input >> m_Scenes;
			input >> m_Style;
			input >> m_Cursor;
			return true;
		case VERSION_102599:
			input >> m_DocumentClass;
			input >> m_StartingScene;
			input >> m_SceneId;
			input >> m_Scenes;
			return true;
		case VERSION_051299:
			input >> m_StartingScene;
			input >> m_SceneId;
			input >> m_Scenes;
			m_DocumentClass = CLASS_KEY( Document );
			return true;
		}

		return false;
	}

	return true;
}

//----------------------------------------------------------------------------

void InterfaceContextPort::dependencies( DependentArray & dep )
{
	Port::dependencies( dep );

	for(int i=0;i<m_Scenes.size();i++)
		dep.push( m_Scenes[i] );

	dep.push( m_Style );
	dep.push( m_Cursor );
}

Resource::Ref InterfaceContextPort::createResource()
{
	InterfaceContext * pInterface = new InterfaceContext;
	ASSERT( pInterface );

	pInterface->setDocumentClass( m_DocumentClass );
	pInterface->setWindowStyle( WindowStyle::Link( portResource( m_Style ) ) );
	pInterface->setCursor( Material::Link( Port::portResource( m_Cursor ) ) );

	for(int i=0;i<m_Scenes.size();i++)
	{
		if (! pInterface->addScene( m_SceneId[i], Scene::Link( portResource( m_Scenes[i] ) ),
			m_StartingScene == i ? true : false ) )
		{
			report( CharString().format("Failed to add scene %s", (const char *)m_Scenes[i]) );
		}
	}

	return pInterface;
}

CFrameWnd * InterfaceContextPort::createView()
{
	static CMultiDocTemplate docTemplate( IDR_PORT, NULL, 
		RUNTIME_CLASS(CChildFrame), RUNTIME_CLASS(CPortView) );

    CChildFrame *pFrame = dynamic_cast<CChildFrame *>( docTemplate.CreateNewFrame( this, NULL ) );
	ASSERT( pFrame );

	docTemplate.InitialUpdateFrame(pFrame, this);

	return( pFrame );
}

bool InterfaceContextPort::importFile( const char * fileName )
{
	return true;	// return true of successful import, false on failure
}

void InterfaceContextPort::shellOpen() const
{
	MessageBox( NULL, _T("shellOpen() is not valid for this port"), NULL, MB_OK);
}

//-------------------------------------------------------------------------------
// EOF


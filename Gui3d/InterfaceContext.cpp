/*
	InterfaceContext.cpp

	The context under which the entire game interface is ran
	(c)2005 Palestar, Richard Lyle
*/

// uncomment to lose profiling information
//#define PROFILE_OFF

#pragma warning( disable : 4146 ) // c:\Projects\Medusa\Standard\String.h(892) : warning C4146: unary minus operator applied to unsigned type, result still unsigned

#define GUI3D_DLL
#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "File/FileDisk.h"
#include "Draw/Draw.h"
#include "Standard/StringHash.h"
#include "System/Messages.h"
#include "System/Platform.h"
#include "Gui3d/NodeInterfaceClient.h"
#include "Gui3d/NodeWindow.h"
#include "Gui3d/InterfaceContext.h"

#if defined(_WIN32)
#include <windows.h>
#endif

//---------------------------------------------------------------------------------------------------

static float GetPercent( qword nCPU, qword nTotalCPU )
{
	float fPercent = 0;
	if ( nTotalCPU > 0 )
		fPercent = ((float)nCPU * 100.0f) / ((float)nTotalCPU);

	return fPercent;
}

//----------------------------------------------------------------------------

InterfaceContext * InterfaceContext::s_InterfaceContext = NULL;		// current interface context

//-------------------------------------------------------------------------------

IMPLEMENT_RESOURCE_FACTORY( InterfaceContext, Resource );

BEGIN_PROPERTY_LIST( InterfaceContext, Resource );
	ADD_PROPERTY( m_DocumentClass );
	ADD_PROPERTY( m_ActiveScene );
	ADD_PROPERTY( m_Scenes );
	ADD_PROPERTY( m_SceneHash );
	ADD_PROPERTY( m_WindowStyle );
	ADD_PROPERTY( m_Cursor );
END_PROPERTY_LIST();

InterfaceContext::InterfaceContext()
{
	m_DocumentClass = CLASS_KEY( Document );

	m_ActiveScene = -1;
	m_Time = 0.0f;
	m_Bits = 0;
	m_nFPSLimit = 60;
	m_Limitor.setLocked( false );
	m_Limitor.setDuration( 1000 / m_nFPSLimit );

	m_pDocument = NULL;
	
	m_CursorState = POINTER;
	m_CursorPosition = PointInt( 0, 0 );
	m_pHelpWindow = NULL;

	m_MovieCapture = false;
	m_MovieFrame = 0;
	m_DisplayProfile = false;
	m_DisplayWireframe = false;

	// create the root window
	m_pRootWindow = new NodeWindow;
	m_pRootWindow->setContext( this );
	m_MessageQueueInvalid = true;

	// create the document
	createDocument();
	// activate the document
	m_pDocument->onActivate();
}

InterfaceContext::~InterfaceContext()
{
	if ( s_InterfaceContext == this )
		s_InterfaceContext = NULL;

	// release the message queue
	m_MessageQueue.release();

	// release the root window
	m_pRootWindow = NULL;

	// release the document
	deleteDocument();
}

//-------------------------------------------------------------------------------

const int VERSION_051299 = 51299;
const int VERSION_20040204 = 20040204;

bool InterfaceContext::read( const InStream & input )
{
	if (! Resource::read( input ) )
		return false;

	// create the document
	createDocument();
	// activate the document
	m_pDocument->onActivate();
	// set the default cursor state
	setCursorState( POINTER );

	if ( m_Scenes.isValid( m_ActiveScene ) )
	{
		// get the new scene
		Scene * pScene = m_Scenes[ m_ActiveScene ];

		// set the render context frame and position in world space
		m_Context.setPosition( pScene->defaultCameraPosition() );
		m_Context.setFrame( pScene->defaultCameraFrame() );

		// make sure the root of our scene node is detached
		pScene->node()->detachSelf();
		// attach new scene to root window
		m_pRootWindow->detachAllNodes();
		m_pRootWindow->attachNode( pScene->node() );

		// call onActivate for the new windows
		onActivate( m_pRootWindow );
		// build the message queue
		updateMessageQueue();
	}
	return true;
}

//-------------------------------------------------------------------------------

const StringHash	EXIT("EXIT");

// handle incoming messages from the CommandTarget interface, see sceneMessage() for messages from the GUI
bool InterfaceContext::onMessage( const Message & msg )
{
	if ( m_pDocument == NULL || m_pRootWindow == NULL )
		return false;

	// if we have a focus window, it gets first crack at all incoming message..
	if ( m_pFocusWindow.valid() && m_pFocusWindow->onMessage( msg ) )
		return true;
	// send all messages to the document
	if ( m_pDocument->onMessage( msg ) )
		return true;

	switch( msg.message )
	{
	case HM_SYSKEYDOWN:
		switch( msg.wparam )
		{
		case HK_F4:		// standard quit program
#if defined(_WIN32)
			::PostMessage( NULL, WM_QUIT, 0, 0 );
#endif
			return true;
//		case 'M':		// movie capture
//			m_MovieCapture = !m_MovieCapture;
//			return true;
		case 'T':		// screen capture
			capture();
			return true;
#ifdef _DEBUG
		case 'W':		// display wireframe
			m_DisplayWireframe = !m_DisplayWireframe;
			return true;
#endif
		case 'P':		// display profiling information
			m_DisplayProfile = !m_DisplayProfile;
			return true;
		}
		break;
	case IC_EXIT:
#if defined(_WIN32)
		::PostMessage( NULL, WM_QUIT, 0, 0 );
#endif
		return true;
	case IC_SET_SCENE:
		{
			int sceneIndex = findScene( msg.wparam );
			if ( sceneIndex >= 0 )
			{
				setActiveScene( sceneIndex );
				return true;
			}

			TRACE( "InterfaceContext::onMessage, SetScene failed to find scene!" );
		}
		return true;
	}

	// send message to all members of the message queue in reverse order, the last
	// is the top most NodeIntefaceClient
	for(int i=m_MessageQueue.size() - 1;i>=0;i--)
		if ( m_MessageQueue[ i ]->onMessage( msg ) )
			return true;
	
	return false;
}

//----------------------------------------------------------------------------

Platform * InterfaceContext::platform() const
{
	// return back the singleton platform object
	return Platform::sm_pPlatform;
}

const char * InterfaceContext::help() const
{
	if ( m_pHelpWindow != NULL )
		return m_pHelpWindow->help();

	return "";
}

//-------------------------------------------------------------------------------

void InterfaceContext::setDocumentClass( const ClassKey & key )
{
	// set the key
	m_DocumentClass = key;
	// recreate the document
	createDocument();
	// activate the document
	m_pDocument->onActivate();
}

bool InterfaceContext::addScene( const char * pName, Scene::Link pScene, bool activate /*= true*/  )
{
	if (! pScene.valid() )
		return false;

	dword command = StringHash( pName );

	int sceneIndex = m_Scenes.size();
	m_Scenes.push( pScene );
	m_SceneHash[ command ] = sceneIndex;

	// set the context
	setContext( pScene->node(), this );
	if ( activate )
		setActiveScene( sceneIndex );

	return true;
}

void InterfaceContext::setCursor( Material::Link pMaterial )
{
	m_Cursor = pMaterial;
	// reset the state so the hardware cursor will get updated..
	setCursorState( m_CursorState );
}

void InterfaceContext::setCursorState( int state )
{
	// store the cursor state
	m_CursorState = state;

	// disable / enable the cursor motion
	Platform * pPlatform = platform();
	if ( pPlatform != NULL )
	{
		// enable or disable cursor movement based on the state
		pPlatform->enableCursor( m_CursorState != HIDDEN );

		// update the hardware cursor if needed..
		if (! pPlatform->config().bSoftwareCursor )
		{
			if ( m_CursorState != HIDDEN && m_Cursor.valid() )
			{
				int nDiffuse = m_Cursor->findTexture( PrimitiveSurface::DIFFUSE, 0 );
				if ( nDiffuse >= 0 )
					pPlatform->setHardwareCursor( m_Cursor->texture( nDiffuse ).m_pImage, m_CursorState - 1 );

				pPlatform->showHardwareCursor( true );
			}
			else
			{
				pPlatform->showHardwareCursor( false );
			}
		}
	}
}

void InterfaceContext::setActiveScene( int scene )
{
	ASSERT( m_WindowStyle.valid() );

	if ( m_ActiveScene != scene )
	{
		// deactivate the old scene
		onDeactivate(  m_pRootWindow );

		// change the active scene
		m_ActiveScene = scene;
		// get the new scene
		Scene * pScene = m_Scenes[ m_ActiveScene ];

		// set the render context frame and position in world space
		m_Context.setPosition( pScene->defaultCameraPosition() );
		m_Context.setFrame( pScene->defaultCameraFrame() );

		// make sure our node is detached from any parent node
		pScene->node()->detachSelf();
		// attach new scene to root window
		m_pRootWindow->detachAllNodes();
		m_pRootWindow->attachNode( pScene->node() );

		// make sure the context is set
		setContext( m_pRootWindow, this );
		// call onActivate for the new windows
		onActivate( m_pRootWindow );
		// build the message queue
		updateMessageQueue();
	}
}

void InterfaceContext::setTime( float time )
{
	m_Time = time;
}

void InterfaceContext::setBits( dword bits )
{
	m_Bits = bits;
}

void InterfaceContext::setFPSLimit( int a_nFPS )
{
	m_nFPSLimit = a_nFPS;
	if ( m_nFPSLimit > 0 )
		m_Limitor.setDuration( 1000 / m_nFPSLimit );
	else
		m_Limitor.setDuration( 0 );
}

bool InterfaceContext::render()
{
	Platform * pPlatform = platform();
	if ( pPlatform == NULL )
		return false;			// no platform object

	// frame rate may be limited... so wait for timer to expire if needed...
	if ( m_nFPSLimit > 0 )
		m_Limitor.wait();

	PROFILE_START( "InterfaceContext::render()" );

	// set the current interface context
	s_InterfaceContext = this;

	DisplayDevice * pDisplay = pPlatform->display();
	ASSERT( pDisplay );

	// set the display and audio devices
	m_Context.setDisplay( pDisplay );
	m_Context.setAudio( pPlatform->audio() );

	// lock the document, since changes from another thread during the render and update can cause problems
	m_pDocument->onLock();

	// process messages firstly
	if (! pPlatform->update() )
	{
		// quit program
		m_pDocument->onUnlock();
		return false;
	}

	// pre-update the document
	m_pDocument->onPreUpdate();

	// update the current time
	float deltaTime = 1.0f / 30.0f;
	if ( m_Context.fps() > 0 )
		deltaTime = 1.0f / m_Context.fps();
	m_Time += deltaTime;

	// update the document
	m_pDocument->onUpdate( deltaTime );
	// update the interface 
	onUpdate( m_pRootWindow, deltaTime );

	// detach any nodes now
	if ( m_DetachQueue.valid() )
	{
		m_MessageQueueInvalid = true;
		while( m_DetachQueue.valid() )
		{
			BaseNode * pNode = *m_DetachQueue;

			// if pParent is null, then node has already been detached
			BaseNode * pParent = pNode->parent();
			if ( pParent != NULL )
				pParent->detachNode( pNode );

			m_DetachQueue.pop();
		}
	}

	// update the message queue if invalid
	if ( m_MessageQueueInvalid )
		updateMessageQueue();

	// lock the display
	pDisplay->lock();
	// calculate the window size
	RectInt window( pDisplay->clientWindow() );
	window -= window.upperLeft();

	// set the root window size and position
	m_pRootWindow->setWindow( window );
	m_pRootWindow->setWindowStyle( m_WindowStyle );

	// set the fill mode
	pDisplay->setFillMode( m_DisplayWireframe ? DisplayDevice::FILL_WIREFRAME : DisplayDevice::FILL_SOLID );

	// setup the context
	m_Context.setTime( m_Time );
	m_Context.setBits( m_Bits );
	m_Context.setProjection( window, PI / 4, 1.0f, 32768.0f );
	
	// render the scene
	m_Context.beginRender( Color(0,0,0), Color(0,0,0) );
	m_Context.render( m_pRootWindow );

	// render the cursor
	m_Context.beginScene();
	if ( platform() != NULL && m_CursorState > HIDDEN )
	{
		m_CursorPosition = platform()->cursorPosition();

		if ( platform()->config().bSoftwareCursor )
		{
			// software cursor
			const RectFloat	cursorUV( 0, 0, 1, 1 );
			const PointInt	cursorOffset( CURSOR_SIZE2, CURSOR_SIZE2 );
			const SizeInt	cursorSize( CURSOR_SIZE, CURSOR_SIZE );

			// save then set the time so the correct frame will be rendered
			float fContextTime = m_Context.time();
			m_Context.setTime( m_CursorState - 1 );
			// set the cursor material, then push a window primitive to render it
			Material::push( m_Context, m_Cursor );
			PrimitiveWindow::push( pDisplay, RectInt( m_CursorPosition - cursorOffset, cursorSize ), cursorUV, WHITE );
			// restore the context time
			m_Context.setTime( fContextTime );
		}

		// determine which top most window the cursor is current inside and has help information
		m_pHelpWindow = NULL;
		for(int i=m_MessageQueue.size() - 1;i>=0;i--)
		{
			NodeWindow * pWindow = WidgetCast<NodeWindow>( m_MessageQueue[ i ] );
			if ( pWindow != NULL && pWindow->visible() && strlen( pWindow->help() ) > 0 )
			{
				PointInt localPosition( pWindow->screenToWindow( m_CursorPosition ) );
				RectInt localWindow( pWindow->localWindow() );

				if ( localWindow.inRect( localPosition ) )
				{
					m_pHelpWindow = pWindow;
					break;
				}
			}
		}

		// render the tool tip
		if ( m_sCursorTip.length() > 0 )
		{
			Font * pFont = m_WindowStyle->font();
			ASSERT( pFont );

			SizeInt		tipSize( pFont->size( m_sCursorTip ) );
			PointInt	tipPosition( m_CursorPosition );

			if ( tipPosition.x > ((window.left + window.right) / 2) )
				tipPosition.x -= (tipSize.width + CURSOR_SIZE2);	// left justify
			else
				tipPosition.x += CURSOR_SIZE2;						// right justify
			if ( tipPosition.y > ((window.top + window.bottom) / 2) )
				tipPosition.y -= (tipSize.height + CURSOR_SIZE2);	// top justify
			else
				tipPosition.y += CURSOR_SIZE2;						// bottom justify

			Font::push( pDisplay, pFont, tipPosition, m_sCursorTip, WHITE );
		}
	}

	PROFILE_END();

	// display the frames per second
	PROFILE_LMESSAGE( 0, CharString().format("FPS: %.2f", m_Context.fps()) );

#ifndef PROFILE_OFF
	// display profiling information
	if ( m_DisplayProfile )
	{
		Font * pFont = m_WindowStyle->font();
		ASSERT( pFont );

		int y = 0;
		int lineHeight = pFont->size().height;

		// lock the profiler while building a report
		Profiler::lock( true );

		// output messages first
		for(int i=0;i<Profiler::messageCount();i++)
		{
			Vector3 vecPos( 0, y, 0 );
			y += lineHeight;

			WideString sMessage = Profiler::message( i );
			Font::push( pDisplay, pFont, vecPos, sMessage, WHITE );
		}

		qword nTotalCPU = Profiler::totalCPU();

		Vector3 vecPos( 0, y, 0 );
		y += lineHeight;

		WideString sSummary;
		sSummary.format(STR("Total CPU: %s"), FormatNumber<wchar,qword>( nTotalCPU ));

		Font::push( pDisplay, pFont, vecPos, sSummary, WHITE );

		// enumerate all threads, and display the profile information for each thread
		for(int k=0;k<Profiler::threadCount();k++)
		{
			dword nThread = Profiler::thread( k );

			vecPos.y = y;
			y += lineHeight;

			WideString sThread;
			sThread.format(STR("Thread: %u"), nThread);

			Font::push( pDisplay, pFont, vecPos, sThread, WHITE );

			int nCount = Profiler::profileCount( nThread );
			for(int j=0;j<nCount;j++)
			{
				Profiler::Profile & profile = Profiler::profile( nThread, j );

				WideString line;
				line.format(STR("%S - CPU: %s (%.2f%%), HITS: %u"), profile.pName, 
					FormatNumber<wchar,qword>( profile.nAvCPU ), GetPercent( profile.nAvCPU, nTotalCPU), profile.nAvHits);

				Vector3 vecPos( 10, y, 0 );
				y += lineHeight;

				Font::push( pDisplay, pFont, vecPos, line, WHITE );
			}

		}

		// unlock the profiler
		Profiler::lock( false );
	}
#endif
	m_Context.endScene();

	// render all primitives on the stack
	m_Context.endRender();
	// capture movie frames
	if ( m_MovieCapture )
		capture();
	// unlock the display device..
	pDisplay->unlock();
	

	PROFILE_END();

	// unlock the document, we are done with render and update
	m_pDocument->onUnlock();

	return true;
}

void InterfaceContext::capture()
{
	// generate the filename
	while( FileDisk::fileExists( CharString().format(".\\%8.8u.jpg", m_MovieFrame) ) )
		m_MovieFrame++;
	
	DisplayDevice * pDisplay = m_Context.display();
	pDisplay->lock();

	// create the targa
	pDisplay->capture( CharString().format(".\\%8.8u.jpg", m_MovieFrame) );

	pDisplay->unlock();
}

void InterfaceContext::setFocus( BaseNode * pFocus )
{
	if ( pFocus != NULL )
	{
		// recurse down the heirarchy, making sure this node and it's parents are the last children
		BaseNode * pParent = pFocus->parent();
		while( pParent != NULL )
		{
			if ( pParent->child( pParent->childCount() - 1 ) != pFocus )
			{
				// make pFocus the last child of pParent
				if ( pFocus->grabReference() )
				{
					pParent->detachNode( pFocus );
					pParent->attachNode( pFocus );

					pFocus->releaseReference();
				}
			}

			pFocus = pParent;
			pParent = pFocus->parent();
		}
	}

	// rebuild message queue
	setMessageQueueInvalid();
}

bool InterfaceContext::lockFocus( NodeWindow * pFocus )
{
	if ( m_pFocusWindow.valid() && pFocus != m_pFocusWindow )
		return false;
	m_pFocusWindow = pFocus;
	return true;
}

void InterfaceContext::unlockFocus( NodeWindow * pFocus )
{
	if ( pFocus == m_pFocusWindow )
		m_pFocusWindow = NULL;
}

void InterfaceContext::detachNode( BaseNode * pNode )
{
	m_DetachQueue.push( pNode );
}

//----------------------------------------------------------------------------

InterfaceContext * InterfaceContext::interfaceContext()
{
	return s_InterfaceContext;
}

BaseNode * InterfaceContext::findNode( const char * pName )
{
	if ( s_InterfaceContext != NULL )
		if ( s_InterfaceContext->scene() != NULL )
			if ( s_InterfaceContext->scene()->node() != NULL )
				return s_InterfaceContext->scene()->node()->findNode( pName );

	return NULL;
}

//----------------------------------------------------------------------------

void InterfaceContext::updateMessageQueue()
{
	// release the previous message queue
	m_MessageQueue.release();

	// build a list of NodeInterfaceClient objects in the current scene
	// incoming messages are sent to the clients in reverse order
	buildMessageQueue( m_pRootWindow );
	// mark the message queue as valid
	m_MessageQueueInvalid = false;
}

void InterfaceContext::buildMessageQueue( BaseNode * pNode )
{
	if ( WidgetCast<NodeInterfaceClient>( pNode ) )
	{
		NodeInterfaceClient * pInterfaceNode = (NodeInterfaceClient *)pNode;
		if (! pInterfaceNode->enabled() )
			return;		// node not enabled, don't recurse any further

		// add to the queue
		m_MessageQueue.push( pInterfaceNode );
	}

	// recurse into the children
	for(int i=0;i<pNode->childCount();i++)
		buildMessageQueue( pNode->child(i) );
}

void InterfaceContext::createDocument()
{
	deleteDocument();

	// create the document using a factory
	m_pDocument = WidgetCast<Document>( Factory::createWidget( m_DocumentClass ) );
	ASSERT( m_pDocument );

	// assign the document to all the NodeInterfaceClient objects
	for(int i=0;i<m_Scenes.size();i++)
		if ( m_Scenes[i].valid() )
			setContext( m_Scenes[i]->node(), this );
}

void InterfaceContext::deleteDocument()
{
	if ( m_pDocument )
	{
		delete m_pDocument;
		m_pDocument = NULL;
	}
}

//----------------------------------------------------------------------------

void InterfaceContext::onActivate( BaseNode * pNode )
{
	ASSERT( pNode != NULL );

	if ( WidgetCast< NodeInterfaceClient > ( pNode ) )
		((NodeInterfaceClient *)pNode)->onActivate();

	for(int i=0;i<pNode->childCount();i++)
		onActivate( pNode->child(i) );
}

void InterfaceContext::onDeactivate( BaseNode * pNode )
{
	ASSERT( pNode != NULL );

	if ( WidgetCast< NodeInterfaceClient > ( pNode ) )
		((NodeInterfaceClient *)pNode)->onDeactivate();

	for(int i=0;i<pNode->childCount();i++)
		onDeactivate( pNode->child(i) );
}

void InterfaceContext::onUpdate( BaseNode * pNode, float t )
{
	ASSERT( pNode != NULL );

	if ( WidgetCast< NodeInterfaceClient > ( pNode ) )
	{
		if (! ((NodeInterfaceClient *)pNode)->enabled() )
			return;		// node not enabled, don't recurse any further
	
		((NodeInterfaceClient *)pNode)->onUpdate( t );
	}

	for(int i=0;i<pNode->childCount();i++)
		onUpdate( pNode->child(i), t );
}

bool InterfaceContext::onMessage( BaseNode * pNode, const Message & msg )
{
	ASSERT( pNode != NULL );

	if ( WidgetCast< NodeInterfaceClient > ( pNode ) )
		if ( ((NodeInterfaceClient *)pNode)->onMessage( msg ) )
			return true;

	// begin at last child, this is the opposite of the render order because the 
	// last children rendered on the ones on the top
	for(int i=pNode->childCount() - 1;i>=0;i--)
		if ( onMessage( pNode->child(i), msg ) )
			return true;

	return false;
}

void InterfaceContext::setContext( BaseNode * pNode, InterfaceContext * pContext )
{
	ASSERT( pNode != NULL );

	if ( WidgetCast< NodeInterfaceClient > ( pNode ) )
		((NodeInterfaceClient *)pNode)->setContext( pContext );

	for(int i=0;i<pNode->childCount();i++)
		setContext( pNode->child(i), pContext );
}

//----------------------------------------------------------------------------
// EOF

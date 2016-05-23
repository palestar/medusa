/*
	InterfaceContext.h

	The context under which the entire game interface is ran
	(c)2005 Palestar, Richard Lyle
*/

#ifndef INTERFACE_CONTEXT_H
#define INTERFACE_CONTEXT_H

#include "Draw/Font.h"
#include "Resource/Resource.h"
#include "Render3D/Scene.h"
#include "System/CommandTarget.h"
#include "Gui3d/Document.h"
#include "Gui3d/Cursor.h"
#include "Gui3d/WindowStyle.h"
#include "Standard/Timer.h"
#include "World/WorldContext.h"
#include "NodeWindow.h"

#include "Gui3d/GUI3DDll.h"

//-------------------------------------------------------------------------------

class NodeInterfaceClient;	// forward declare
class NodeWindow;
class Platform;

typedef struct lua_State lua_State;
typedef struct lua_Debug lua_Debug;

class DLL InterfaceContext : public Resource, public CommandTarget, public WorldContext::ScriptHook
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef CommandTarget::Message			Message;

	typedef ResourceLink<InterfaceContext>	Link;
	typedef Reference<InterfaceContext>		Ref;

	enum Constants
	{
		CURSOR_SIZE		= 32,	// cursor size in pixels
		CURSOR_SIZE2	= CURSOR_SIZE / 2,
	};

	// Construction
	InterfaceContext();
	virtual ~InterfaceContext();

	// Widget interface
	bool						read( const InStream & );
	// CommandTarget interface
	bool						onMessage( const Message & msg );

	// Accessors
	Platform *					platform() const;
	Document *					document() const;
	DisplayDevice *				display() const;
	AudioDevice *				audio() const;
	RenderContext *				renderContext() const;

	NodeWindow *				rootWindow() const;
	int							activeScene() const;
	int							findScene( const char * pName ) const;
	int							findScene( dword id ) const;

	WindowStyle *				windowStyle() const;				// default window style
	Material *					cursor() const;
	int							cursorState() const;
	const PointInt &			cursorPosition() const;

	NodeWindow * 				helpWindow() const;					// cursor is currently over which window
	const char *				help() const;						// get current help text
	const char *				tip() const;

	Scene *						scene() const;
	float						time() const;
	dword						bits() const;
	int							getFPSLimit() const;					// frames per second limit... used ot reduce CPU usage

	// Mutators
	void						setDocumentClass( const ClassKey & key );
	bool						addScene( const char * id, 
									Scene::Link pScene, bool activate = true );

	void						setWindowStyle( WindowStyle::Link pStyle );
	void						setCursor( Material::Link pMaterial );
	void						setCursorState( int state );
	void						setCursorTip( const char * pTip );

	void						setActiveScene( int scene );
	void						setTime( float time );
	void						setBits( dword bits );
	void						setFPSLimit( int a_nFPS );

	void						setMessageQueueInvalid( bool invalid = true );

	bool						render();				// execute and render interface, returns false to end
	void						capture();				// capture screen shot

	//! This function makes the given node the top-most node
	void						setFocus( BaseNode * pFocus );
	//! This function locks the focus to the given node, all incoming messages are passed to this window FIRST
	//! until unlockFocus() is called.
	bool						lockFocus( NodeWindow * pFocus );	
	void						unlockFocus( NodeWindow * pFocus );

	void						detachNode( BaseNode * pNode );

	// Static
	static InterfaceContext *	interfaceContext();		// returns the current interface context object
	static BaseNode *			findNode( const char * pName );

	// InterfaceContextScript.cpp

	// WorldContext::ScriptHook interface
	virtual void				registerFunctions( lua_State * pScript );
	// Script Functions
	static int					messageBox( lua_State * pScript );
	static int					highlightWindow( lua_State * pScript );
	static int					hideWindow( lua_State * pScript );
	static int					showWindow( lua_State * pScript );
	static int					disableWindow( lua_State * pScript );
	static int					enableWindow( lua_State * pScript );

private:
	// Types
	typedef Hash< dword, int >		SceneHash;
	typedef SceneHash::Iterator		SceneHashIt;

	// Data
	ClassKey					m_DocumentClass;
	int							m_ActiveScene;
	Array< Scene::Link >		m_Scenes;

	SceneHash					m_SceneHash;

	WindowStyle::Link			m_WindowStyle;			// default window style
	Material::Link				m_Cursor;				// the cursor material

	// non-serialized
	CharString					m_sCursorTip;					// the current tool tip
	RenderContext				m_Context;				// the render context object
	Document *					m_pDocument;			// our document object
	NodeWindow::Ref				m_pRootWindow;			// our root window

	float						m_Time;
	dword						m_Bits;
	int							m_nFPSLimit;			// frames per second limit, 0 if disabled...
	Timer						m_Limitor;

	int							m_CursorState;			// cursor state
	PointInt					m_CursorPosition;
	NodeWindow *				m_pHelpWindow;

	NodeWindow::Ref				m_pFocusWindow;			// current focus window if any

	bool						m_MovieCapture;			// true if movie is being captured
	int							m_MovieFrame;
	bool						m_DisplayProfile;		// display profiling information
	bool						m_DisplayWireframe;		// display wireframe

	bool						m_MessageQueueInvalid;	// is message queue invalid and needs to be updated?
	Array< NodeInterfaceClient::Ref >		
								m_MessageQueue;			// the message queue
	Queue< BaseNode::Ref >		m_DetachQueue;			// objects to detach from scene

	// Static
	static InterfaceContext *	s_InterfaceContext;		// current interface context

	// Mutators
	void						updateMessageQueue();
	void						buildMessageQueue( BaseNode * pNode );
	void						createDocument();		// create and hook the document
	void						deleteDocument();

	// Static
	static void					onActivate( BaseNode * pNode );
	static void					onDeactivate( BaseNode * pNode );
	static void					onUpdate( BaseNode * pNode, float t );
	static bool					onMessage( BaseNode * pNode, const Message & msg );

	static void					setContext( BaseNode * pNode, InterfaceContext * pContext );
};

//----------------------------------------------------------------------------

inline Document * InterfaceContext::document() const
{
	return( m_pDocument );
}

inline DisplayDevice * InterfaceContext::display() const
{
	return( m_Context.display() );
}

inline AudioDevice * InterfaceContext::audio() const
{
	return( m_Context.audio() );
}

inline RenderContext * InterfaceContext::renderContext() const
{
	return const_cast<RenderContext *>( &m_Context );
}

inline NodeWindow * InterfaceContext::rootWindow() const
{
	return( m_pRootWindow );
}

inline int InterfaceContext::activeScene() const
{
	return( m_ActiveScene );
}

inline int InterfaceContext::findScene( const char * pName ) const
{
	return( findScene( StringHash( pName ) ) );
}

inline int InterfaceContext::findScene( dword id ) const
{
	SceneHashIt scene = m_SceneHash.find( id );
	if ( scene.valid() )
		return( *scene );

	return( -1 );
}

inline WindowStyle * InterfaceContext::windowStyle() const
{
	return( m_WindowStyle );
}

inline Material * InterfaceContext::cursor() const
{
	return( m_Cursor );
}

inline int InterfaceContext::cursorState() const
{
	return( m_CursorState );
}

inline const PointInt &	InterfaceContext::cursorPosition() const
{
	return m_CursorPosition;
}

inline NodeWindow * InterfaceContext::helpWindow() const
{
	return m_pHelpWindow;
}

inline const char * InterfaceContext::tip() const
{
	return m_sCursorTip;
}

inline Scene *	InterfaceContext::scene() const
{
	if ( m_ActiveScene < 0 )
		return NULL;

	return( m_Scenes[ m_ActiveScene ] );
}

inline float InterfaceContext::time() const
{
	return( m_Time );
}

inline dword InterfaceContext::bits() const
{
	return( m_Bits );
}

inline int InterfaceContext::getFPSLimit() const
{
	return m_nFPSLimit;
}

//----------------------------------------------------------------------------

inline void InterfaceContext::setWindowStyle( WindowStyle::Link pStyle )
{
	m_WindowStyle = pStyle;
}

inline void InterfaceContext::setCursorTip( const char * pTip )
{
	m_sCursorTip = pTip;
}

inline void InterfaceContext::setMessageQueueInvalid( bool invalid /*= true*/ )
{
	m_MessageQueueInvalid = invalid;
}

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

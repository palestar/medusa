/*
	View.h

	This class is used to handle a heirarchy of controls and windows, displaying information
	from the document class. This is modeled after MFC's document/view design.

	(c)2005 Palestar, Richard Lyle

	$ Log: $
*/

#ifndef WINDOW_VIEW_H
#define WINDOW_VIEW_H

#include "Gui3d/NodeWindow.h"
#include "Gui3d/WindowButton.h"
#include "Gui3d/WindowList.h"
#include "Gui3d/WindowText.h"
#include "Gui3d/WindowEdit.h"
#include "Gui3d/WindowSlider.h"
#include "Gui3d/WindowMenu.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowView : public NodeWindow
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowView>		Ref;

	class DLL View : public Widget
	{
	public:
		DECLARE_WIDGET_CLASS();

		typedef Reference< View >		Ref;
		typedef NodeWindow::Message		Message;

		// Construction
		View(); 
		// Client interface
		virtual void		onActivate() = 0;
		virtual void		onDeactivate() = 0;
		virtual void		onUpdate( float t ) = 0;
		virtual	bool		onMessage( const Message & msg ) = 0;
		
		virtual void		onShow() {};
		virtual void		onHide() {};
		virtual void		onRender( RenderContext & context, 
								const RectInt & window )	{}
		virtual bool		onCursorMove( const RectInt & window, 
								const PointInt & position,		// window and position are local coordinates
								const PointInt & delta ) 	{ return false; }
		virtual bool		onLeftDown( const RectInt & window, 
								const PointInt & position ) { return false; }
		virtual bool		onLeftUp( const RectInt & window, 
								const PointInt & position ) { return false; }
		virtual bool		onRightDown( const RectInt & window,
								const PointInt & position ) { return false; }
		virtual bool		onRightUp( const RectInt & window,
								const PointInt & position ) { return false; }

		// Accessors
		InterfaceContext *	context() const;
		Document *			document() const;
		WindowView *		window() const;
		// Mutators
		void				setWindow( WindowView * pWindow );
		// Helpers
		void				setCursorState( CursorState state );
		void				setCursorTip( const char * pTip );

	private:
		WindowView *		m_pWindow;
	};

	// Construction
	WindowView();

	// NodeInterfaceClient interface
	void					onActivate();
	void					onDeactivate();
	void					onUpdate( float t );			
	bool					onMessage( const Message & msg );

	// NodeWindow interface
	void					onShow();
	void					onHide();
	void					onRender( RenderContext & context,
								const RectInt & window );	
	bool					onCursorMove( const RectInt & window, 
								const PointInt & position,		// window and position are local coordinates
								const PointInt & delta );
	bool					onLeftDown( const RectInt & window, 
								const PointInt & position );
	bool					onLeftUp( const RectInt & window, 
								const PointInt & position );
	bool					onRightDown( const RectInt & window,
								const PointInt & position );
	bool					onRightUp( const RectInt & window,
								const PointInt & position );

	// Accessors
	const ClassKey &		viewClass() const;
	View *					view() const;
	// Mutators
	void					setViewClass( const ClassKey & key );

private:
	// Data
	ClassKey				m_ViewClass;			// view class to create
	// non-serialized
	View::Ref				m_View;					// the view class created

	// Mutators
	void					createViewClass();
};

//----------------------------------------------------------------------------

inline const ClassKey & WindowView::viewClass() const
{
	return m_ViewClass;
}

inline WindowView::View * WindowView::view() const
{
	return m_View;
}

inline void WindowView::setViewClass( const ClassKey & key )
{
	m_ViewClass = key;
	createViewClass();
}

//----------------------------------------------------------------------------

inline WindowView::View::View() : m_pWindow( NULL )
{}

inline InterfaceContext * WindowView::View::context() const
{
	if ( m_pWindow != NULL )
		return m_pWindow->context();
	return NULL;
}

inline Document * WindowView::View::document() const
{
	if ( m_pWindow != NULL )
		return m_pWindow->document();
	return NULL;
}

inline WindowView * WindowView::View::window() const
{
	return m_pWindow;
}

inline void WindowView::View::setWindow( WindowView * pWindow )
{
	m_pWindow = pWindow;
}

inline void WindowView::View::setCursorState( CursorState state )
{
	if ( m_pWindow != NULL )
		m_pWindow->setCursorState( state );
}

inline void WindowView::View::setCursorTip( const char * pTip )
{
	if ( m_pWindow != NULL )
		m_pWindow->setCursorTip( pTip );
}


//----------------------------------------------------------------------------

// Helper macros used by classes derived from WindowView::View
#define MESSAGE_MAP( messageId, nodeId, functionName )								\
	if ( msg.message == messageId && msg.origin == nodeId )							\
		return functionName( msg );

#define MESSAGE_MAP_HM( messageId, functionName )									\
	if ( msg.message == messageId )													\
		return functionName( msg );

#ifndef _DEBUG

#define DATA_MAP( member, type, name )												\
	member = WidgetCast<type>( window()->findNode( name ) );

#else

#define DATA_MAP( member, type, name )												\
	if ( (member = WidgetCast<type>( window()->findNode( name ) )) == NULL )		\
		__asm int 3;

#endif

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

/*
	TestView.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef TESTVIEW_H
#define TESTVIEW_H

#include "System/Messages.h"
#include "Gui3d/WindowView.h"

//----------------------------------------------------------------------------

class TestView : public WindowView::View
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference< TestView >	Ref;

	// Construction
						TestView();
	// Widget interface
	bool				read( const InStream & input );
	bool				write( const OutStream &output );
	// View interface
	void				onActivate();
	void				onDeactivate();
	void				onUpdate( float t );
	bool				onMessage( const Message & msg );
	void				onDocumentUpdate();
	void				onRender( RenderContext & context, const RectInt & window );

	// {{BEGIN_DATA
	WindowList *				m_pList1;
	NodeWindow *				m_pWindow2;
	NodeWindow *				m_pWindow1;
	WindowText *				m_pTextWindow;
	// END_DATA}}

	// {{BEGIN_MSG
	bool				onChar(const Message & msg);
	bool				onButton1Over(const Message & msg);
	bool				onButton2(const Message & msg);
	bool				onButton1(const Message & msg);
	// END_MSG}}
};

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF

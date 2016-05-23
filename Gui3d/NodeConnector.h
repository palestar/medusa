/*
	NodeConnector.h

	This node draws a connector line between 2 windows
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef NODECONNECTOR_H
#define NODECONNECTOR_H

#include "Gui3d/NodeWindow.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL NodeConnector : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Construction
	NodeConnector();
	NodeConnector( NodeWindow * pOrigin, NodeWindow * pConnect, Color color );

	// BaseNode interface
	void				render( RenderContext & context, 
							const Matrix33 & frame, 
							const Vector3 & position );

	// Accessors
	NodeWindow *		origin() const;
	NodeWindow *		connect() const;
	Color				color() const;

	// Mutators
	void				setOrigin( NodeWindow * pOrigin );
	void				setConnect( NodeWindow * pConnect );
	void				setColor( Color color );

protected:
	NodeWindow::wRef	m_Origin;
	NodeWindow::wRef	m_Connect;
	Color				m_Color;
};



#endif

//----------------------------------------------------------------------------
//EOF

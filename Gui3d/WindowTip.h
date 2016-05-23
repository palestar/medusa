/*
	WindowTip.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOW_TIP_H
#define WINDOW_TIP_H

#include "Gui3d/WindowText.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class NodeTip;
class DLL WindowTip : public WindowText
{
public:
	DECLARE_WIDGET_CLASS();

	// Types
	typedef Reference<WindowTip>		Ref;
	// Exceptions
	class TipAlreadyAttached {};

	// Construction
	WindowTip();
	WindowTip( NodeWindow * pParent, const char * pTip, int iMaxWidth = 350 );
	virtual ~WindowTip();
	// NodeInterfaceClient interface
	void				onUpdate( float t );			

	// Static
	static dword		s_TipStyle;
	static int			s_TipBorderSize;
	static Color		s_TipBorderColor;
	static Color		s_TipBackColor;
	static Color		s_TipTextColor;

	// Helpers
	static WindowTip *	getWindowTip( NodeWindow * pCheck );

private:
	// Data
	NodeWindow::WeakRef	m_wpParent;
	// Mutators
	void				updateWindowPosition();
};

//---------------------------------------------------------------------------------------------------

class DLL NodeTip : public BaseNode
{
public:
	DECLARE_WIDGET_CLASS();

	NodeTip() : m_pTip( NULL )
	{}
	NodeTip( WindowTip * pTip ) : m_pTip( pTip )
	{}
	virtual ~NodeTip()
	{
		if ( m_pTip != NULL )
			m_pTip->destroy();
	}

	WindowTip *		m_pTip;
};


//-------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF

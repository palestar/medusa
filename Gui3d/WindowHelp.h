/*
	WindowHelp.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef WINDOWHELP_H
#define WINDOWHELP_H

#include "Gui3d/WindowText.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowHelp : public WindowText
{
public:
	DECLARE_WIDGET_CLASS();

	// Construction
	WindowHelp();

	// NodeInterfaceClient interface
	void				onActivate();									
	void				onDeactivate();							
	void				onUpdate( float t );
private:
	// Data
	NodeWindow::Ref		m_TargetWindow;
};

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

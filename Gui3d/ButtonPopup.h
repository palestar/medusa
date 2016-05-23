/*
	ButtonPopup.h

	This button displays it's child objects when active
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef BUTTONPOPUP_H
#define BUTTONPOPUP_H

#include "Gui3d/WindowButton.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL ButtonPopup : public WindowButton
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<ButtonPopup>		Ref;

	enum PopupStyle
	{
		POPDOWN_SIBLINGS	= USER3,		// popdown any sibling popup buttons when this button is pressed
		DISABLE_CHILDREN	= USER4,		// when children are not active, are they disabled
		HIDE_SIBLINGS		= USER5,		// when button is activated, do we hide all sibling windows
	};

	// Construction
	ButtonPopup();

	// NodeInterfaceClient interface
	void				onActivate();									

	// WindowButton Interface
	void				onButtonDown();						// called on button depressed
	void				onButtonUp();
	void				onButtonAbort();

	// Accessors
	bool				up() const;
	// Mutators
	virtual void		popDown();
	virtual void		popUp();
	
private:
	// Data
	bool				m_Up;
};

//----------------------------------------------------------------------------

inline bool ButtonPopup::up() const
{
	return m_Up;
}

//-------------------------------------------------------------------------------


#endif

//----------------------------------------------------------------------------
// EOF


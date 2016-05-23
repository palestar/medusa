/*
	WindowMenu.h

	GUI object for a menu
	(c)2005 Palestar, Richard Lyle
*/

#ifndef WINDOW_MENU_H
#define WINDOW_MENU_H

#include "Gui3d/WindowText.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowMenu : public WindowText
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowMenu>		Ref;

	struct Item
	{
		LocalizedString	text;							// text of command
		dword			key;							// hot key for menu item

		Item & operator=( const Item & copy );
	};

	// Construction
	WindowMenu();

	// NodeInterfaceClient interface
	void				onUpdate( float t );
	bool				onMessage( const Message & msg );
	// NodeWindow interface
	void				onRender( RenderContext & context,
							const RectInt & window );		// client render, window is in screen coordinates
	bool				onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta );
	bool				onLeftDown( const RectInt & window, const PointInt & position );
	bool				onLeftUp( const RectInt & window, const PointInt & position );

	// Accessors
	int					itemCount() const;
	const Item &		item( int n ) const;

	// Mutators
	int					addItem( const char * string, dword key = 0 );
	int					addItem( const Item & item );
	void				setItem( int i, const Item & item );
	void				removeItem( int i );
	void				removeAllItems();

	// client interface
	virtual void		onSelectChange( int item );
	virtual void		onSelect( int item );	

protected:

	// Data
	Array< Item >		m_Items;						// menu items

	// non-serialized
	int					m_Highlighted;					// currently highlighted item
	int					m_Selected;						// currently selected item
	bool				m_MenuInvalid;					// set to true anytime items are added

	// Mutators
	void				updateMenu();					// update the menu
};

//----------------------------------------------------------------------------

inline int WindowMenu::itemCount() const
{
	return m_Items.size();
}

inline const WindowMenu::Item & WindowMenu::item( int n ) const
{
	return m_Items[ n ];
}

//----------------------------------------------------------------------------

inline WindowMenu::Item & WindowMenu::Item::operator=( const WindowMenu::Item & copy )
{
	text = copy.text;
	key = copy.key;

	return *this;
}

inline const InStream & operator>>(const InStream &input, WindowMenu::Item & item )
{
	input >> item.text;
	input >> item.key;
	return input;
}

inline const OutStream & operator<<(const OutStream &output, const WindowMenu::Item & item )
{
	output << item.text;
	output << item.key;
	return output;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

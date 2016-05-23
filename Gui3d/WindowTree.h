/*
	WindowTree.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef WINDOWTREE_H
#define WINDOWTREE_H

#include "Gui3d/NodeWindow.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowTree : public NodeWindow
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowTree>		Ref;

	enum TreeStyle
	{
		SINGLE_EXPAND = NodeWindow::USER1,	// only one branch is expanded at any one time
	};
	enum ItemFlags
	{
		BUTTON			= 0x1,				// display item as a button
		EXPANDED		= 0x2,				// item is current expanded
	};
	struct Item
	{
		Color			cColor;			// color of text
		String			sLabel;			// text displayed for this item
		dword			dwFlags;		// item flags
		dword			dwUser;			// user data
		
		Item *			pParent;		// items parent, 0 if a root item
		Array< Item * >	children;		// children items

		Item & operator=( const Item & copy );
	};

	// Construction
	WindowTree();
	virtual ~WindowTree();

	// NodeWindow interface
	void				onRender( RenderContext & context,
							const RectInt & window );		// client render, window is in screen coordinates
	bool				onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta );
	bool				onLeftUp( const RectInt & window, const PointInt & position );
	bool				onRightUp( const RectInt & window, const PointInt & position );

	// Accessors
	int					indent() const;					// how many pixels to indent each item
	int					itemCount() const;				// get the number of root items
	Item *				item( int n ) const;			// get root item n
	Item *				root() const;					// get the root item

	int					itemDepth( Item * pItem ) const;	// how deep this is this item in the tree

	// Mutators
	void				setIndent( int pixels );
	Item *				insertItem( const wchar * pLabel, Color color, 
							dword flags, dword user, Item * pParent = NULL );
	bool				deleteItem( Item * pItem );		// remove item from the tree
	void				reset();						// release the entire tree

	void				expand( Item * pItem );			// expand the specified item
	void				collapse( Item * pItem );		// collapse the specified item
	void				sort( Item * pItem );			// sort children of the specified item by their labels

	// client interface
	virtual void		onHighlight( Item * pItem );	// called when the currently highlighted item changes
	virtual void		onSelect( Item * pItem );		// called when an item is left clicked on
	virtual void		onMenu( Item * pItem );			// called when an item is right clicked on

	virtual void		onRenderItem( RenderContext & context, const RectInt & window,
							PointInt & pos, Item * pItem );		// called to render a tree item, render code should set pos for the next item

protected:
	// Helpers
	void				rdelete( Item * pItem );		// recursive delete a item
	// static
	static int			compare( Item * pItem1, Item * pItem2 );

	// Data
	int					m_Indent;						// how many pixels to indent each item
	Item *				m_pRoot;						// root item of the tree, never actually displayed
	Item *				m_pCursorOver;					// which item is the cursor currently over
	Item *				m_pSelected;					// which item is currently selected

	bool				m_CursorInWindow;				// true if the cursor is currently in the window
	PointInt			m_LastCursorPosition;			// last known cursor position
};

//----------------------------------------------------------------------------

inline WindowTree::Item & WindowTree::Item::operator=( const Item & copy )
{
	cColor = copy.cColor;
	sLabel = copy.sLabel;
	dwFlags = copy.dwFlags;
	dwUser = copy.dwUser;
	pParent = copy.pParent;
	children = copy.children;

	return *this;
}

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

/*
	WindowList.h

	This class presents a list of items for selection
	(c)2005 PaleStar Development, Richard Lyle
*/

#ifndef WINDOW_LIST_H
#define WINDOW_LIST_H

#include "Gui3d/WindowText.h"
#include "Gui3d/GUI3DDll.h"

//----------------------------------------------------------------------------

class DLL WindowList : public WindowText
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef Reference<WindowList>		Ref;

	struct Column
	{
		int		width;		// width of column
		Color	color;		// color of text in column
	};

	// Construction
	WindowList();

	// NodeInterfaceClient interface
	void				onActivate();
	// NodeWindow interface
	void				onRender( RenderContext & context,
							const RectInt & window );		// client render, window is in screen coordinates

	bool				onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta );
	bool				onLeftDown( const RectInt & window, const PointInt & position );
	bool				onLeftUp( const RectInt & window, const PointInt & position );

	// Accessors
	int					columnCount() const;
	const Column &		column( int n ) const;
	int					itemCount() const;
	const char *		item( int i, int c = 0 ) const;

	bool				sortEnable() const;
	int					sortByColumn() const;
	bool				sortAscending() const;

	int					highlighted() const;		// highlighted() & selected() return an index from the top of the display list, unless you disabled sorting
	int					selected() const;			// int realIndex = sortedItem( selected() );
	int					sortedItem( int n ) const;

	// Mutators
	int					addColumn( const Column & column );
	void				setColumn( int c, const Column & column );
	void				removeColumn( int n );

	int					addItem( const char * pString );
	int					setItem( int i, int c, const char * pString );
	void				removeItem( int i );
	void				swapItems( int i, int n );	// swap items in the list
	void				moveItem( int i, int n );	// move item i to position n
	void				removeAllItems();

	void				removeAll();				// releases all columns and items

	void				setSortEnable( bool sort );
	void				setSort( int column, bool ascending );

	void				setHighlighted( int item );		// doesn't call onSelectChange
	void				setSelected( int item );		// doesn't call onSelect

	void				updateList();					// resort the list

	// Helpers
	int					addColumn( int width, Color color );

	// client interface
	virtual void		onSelectChange( int item );
	virtual void		onSelect( int item );	

protected:
	// Data
	Array< Column >		
						m_Columns;						// list columns
	Array< Array< CharString > >
						m_List;							// list data
	
	bool				m_SortEnable;					// is sorting enabled
	int					m_SortByColumn;					// which column to sort by
	bool				m_SortAscending;				// sort ascending, otherwise decending
	Array< int >		m_SortedItems;					// array of sorted items

	// non-serialized
	int					m_Highlighted;					// currently highlighted item
	int					m_Selected;						// currently selected item
	
	bool				m_ListInvalid;					// set to true anytime items are added
	bool				m_SortInvalid;					// set to true to resort list

private:
	// Static
	static WindowList *	s_pSorting;
 	static int			sortCallback( const void *, const void * );
};

IMPLEMENT_RAW_STREAMING( WindowList::Column );

//----------------------------------------------------------------------------

inline int WindowList::columnCount() const
{
	return( m_Columns.size() );
}

inline const WindowList::Column & WindowList::column( int c ) const
{
	return( m_Columns[ c ] );
}

inline int WindowList::itemCount() const
{
	return( m_List.size() > 0 ? m_List[ 0 ].size() : 0 );
}

inline const char * WindowList::item( int n, int c /*= 0*/ ) const
{
	return( m_List[ c ][ n ] );
}

inline bool WindowList::sortEnable() const
{
	return( m_SortEnable );
}

inline int WindowList::sortByColumn() const
{
	return( m_SortByColumn );
}

inline bool WindowList::sortAscending() const
{
	return( m_SortAscending );
}

inline int WindowList::highlighted() const
{
	return( m_Highlighted );
}

inline int WindowList::selected() const
{
	return( m_Selected );
}

inline int WindowList::sortedItem( int n ) const
{
	if ( n >= 0 && n < m_SortedItems.size() )
		return m_SortedItems[ n ];

	return -1;
}

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

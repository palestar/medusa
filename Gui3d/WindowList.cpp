/*
	WindowList.cpp

	This class presents a list of items for selection
	(c)2005 PaleStar Development, Richard Lyle
*/

#define GUI3D_DLL
#include "Debug/Assert.h"
#include "Draw/Draw.h"
#include "Gui3d/WindowList.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowList, WindowText );
REGISTER_FACTORY_KEY(  WindowList, 4041331871448120586 );

BEGIN_PROPERTY_LIST( WindowList, WindowText );
	ADD_PROPERTY( m_Columns );
	ADD_PROPERTY( m_List );
	ADD_PROPERTY( m_SortEnable );
	ADD_PROPERTY( m_SortByColumn );
	ADD_PROPERTY( m_SortAscending );
	ADD_PROPERTY( m_SortedItems );
END_PROPERTY_LIST();

WindowList::WindowList()
{
	m_SortEnable = true;
	m_SortByColumn = 0;
	m_SortAscending = true;

	m_Highlighted = -1;
	m_Selected = -1;
	m_ListInvalid = m_SortInvalid = false;
}

//----------------------------------------------------------------------------

void WindowList::onActivate()
{
	if ( m_Columns.size() < 1 )
		addColumn( m_Window.width(), Color(255,255,255,255) );
}

//----------------------------------------------------------------------------

const RectFloat WINDOW_UV(0,0,1,1);

void WindowList::onRender( RenderContext & context, const RectInt & window )
{
	// update the list
	updateList();

	// draw highlighted selection
	if ( m_Highlighted >= 0 )
	{
		Font * pFont = windowStyle()->font();
		ASSERT( pFont );

		int height = pFont->size().height;
		int offset = m_Highlighted * height;

		RectInt highWindow( window.m_Left, window.m_Top + offset, window.m_Right, window.m_Top + offset + height );

		PrimitiveMaterial::push( context.display(), PrimitiveMaterial::ADDITIVE );
		PrimitiveWindow::push( context.display(), highWindow, WINDOW_UV, Color(128,128,128,64) );
	}

	// draw highlighted selection
	if ( m_Selected >= 0 )
	{
		Font * pFont = windowStyle()->font();
		ASSERT( pFont );

		int height = pFont->size().height;
		int offset = m_Selected * height;

		RectInt highWindow( window.m_Left, window.m_Top + offset, window.m_Right, window.m_Top + offset + height );

		PrimitiveMaterial::push( context.display(), PrimitiveMaterial::ADDITIVE );
		PrimitiveWindow::push( context.display(), highWindow, WINDOW_UV, Color(128,128,128,128) );
	}

	// call the base class
	WindowText::onRender( context, window );
}

bool WindowList::onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta )
{
	updateList();
	if ( m_Flags & DISABLED )
		return false;

	if ( (m_Flags & VISIBLE) && window.inRect( position ) )
	{
		setCursorState( SELECT );
		// determine highlighted item
		Font * pFont = windowStyle()->font();
		ASSERT( pFont );

		int item = position.m_Y / pFont->size().height;
		if ( item >= 0 && item < itemCount() )
		{
			if ( m_Highlighted != item )
			{
				m_Highlighted = item;
				onSelectChange( m_SortedItems[ m_Highlighted ] ); 
			}
		}
		else if ( m_Highlighted != -1 )
		{
			m_Highlighted = -1;
			onSelectChange( -1 );
		}

		return true;
	}

	return WindowText::onCursorMove( window, position, delta );
}

bool WindowList::onLeftDown( const RectInt & window, const PointInt & position )
{
	if ( m_Flags & DISABLED )
		return false;
	if ( (m_Flags & VISIBLE) && window.inRect( position) )
		return true;

	return WindowText::onLeftDown( window, position );
}

bool WindowList::onLeftUp( const RectInt & window, const PointInt & position )
{
	if ( m_Flags & DISABLED )
		return false;
	if ( (m_Flags & VISIBLE) && window.inRect( position ) )
	{
		if ( m_Highlighted >= 0 )
		{
			m_Selected = m_Highlighted;
			onSelect( m_SortedItems[ m_Selected ] );
		}
		else
		{
			m_Selected = -1;
			onSelect( -1 );
		}
	}

	return WindowText::onLeftUp( window, position );
}

//----------------------------------------------------------------------------

int WindowList::addColumn( const Column & column )
{
	m_Columns.push( column );
	m_List.push();			
	m_ListInvalid = true;

	// return the column index
	return( m_Columns.size() - 1 );
}

void WindowList::setColumn( int c, const Column & column )
{
	m_Columns[ c ] = column;
}

void WindowList::removeColumn( int n )
{
	m_Columns.remove( n );
	m_List.remove( n );

	m_SortByColumn = 0;
	m_ListInvalid = true;
}

int WindowList::addItem( const char * pString )
{
	if ( m_List.size() < 1 )
		addColumn( window().width(), WHITE );

	m_List[ 0 ].push( pString );
	for(int i=1;i<m_Columns.size();i++)
		m_List[ i ].push();
	m_SortInvalid = true;

	return( m_List[ 0 ].size() - 1 );
}

int WindowList::setItem( int i, int c, const char * pString )
{
	m_List[ c ][ i ] = pString;
	if ( c == m_SortByColumn )
		m_SortInvalid = true;
	return i;
}

void WindowList::removeItem( int n )
{
	for(int i=0;i<m_Columns.size();i++)
		m_List[ i ].remove( n );
	m_SortInvalid = true;
}

void WindowList::swapItems( int i, int n )
{
	for(int j=0;j<m_Columns.size();j++)
		m_List[ j ].swap( i, n );

	m_SortEnable = false;		// disable sorting
	m_SortedItems.swap( i, n );
	m_ListInvalid = true;
}

void WindowList::moveItem( int i, int n )
{
	for(int j=0;j<m_Columns.size();j++)
		m_List[ j ].move( i, n );

	m_SortEnable = false;		// disable sorting
	m_SortedItems.move( i, n );
	m_ListInvalid = true;
}

void WindowList::removeAllItems()
{
	for(int i=0;i<m_Columns.size();i++)
		m_List[ i ].release();
	m_Selected = -1;
	m_Highlighted = -1;
	m_SortInvalid = true;
}

void WindowList::removeAll()
{
	m_Columns.release();
	m_List.release();
	m_Selected = -1;
	m_Highlighted = -1;
	m_SortByColumn = 0;
	m_SortInvalid = true;
	m_ListInvalid = true;
}

void WindowList::setSortEnable( bool enable )
{
	m_SortEnable = enable;
	m_SortInvalid = true;
}

void WindowList::setSort( int column, bool ascending )
{
	m_SortByColumn = column;
	m_SortAscending = ascending;
	m_SortInvalid = true;
}

void WindowList::setHighlighted( int item )
{
	m_Highlighted = item;
}

void WindowList::setSelected( int item )
{
	m_Selected = item;
}

void WindowList::updateList()
{
	if ( m_SortInvalid || m_ListInvalid )
	{
		m_SortedItems.allocate( itemCount() );

		int i = 0;
		for(;i<m_SortedItems.size();i++)
			m_SortedItems[ i ] = i;

		if ( m_SortEnable )		// if false, then leave the items in their added order
		{
			// sort the list items
			s_pSorting = this;
			int * pSortBuffer = m_SortedItems.buffer();
			qsort( pSortBuffer, m_SortedItems.size(), sizeof(int), sortCallback );

			// copy results back into array
			for(i=0;i<m_SortedItems.size();i++)
				m_SortedItems[i] = pSortBuffer[i];

			delete [] pSortBuffer;
		}

		m_SortInvalid = false;
		m_ListInvalid = true;
	}

	if ( m_ListInvalid )
	{
		Font * pFont = windowStyle()->font();
		ASSERT( pFont );

		SizeInt listSize( 0, pFont->size().height * m_SortedItems.size() );
		int i = 0;
		for(;i<m_Columns.size();i++)
			listSize.width += m_Columns[ i ].width;

		int y = 0;

		// build the list
		CharString sList;
		for(i=0;i<m_SortedItems.size();i++)
		{
			sList += CharString().format("<Y;%d>", y );		// set the Y position

			int x = 0;
			int item = m_SortedItems[ i ];
			for(int j=0;j<m_Columns.size();j++)
			{
				sList += CharString().format("<X;%d><Color;%x>", x, m_Columns[ j ].color );
				sList += m_List[ j ][ item ];

				x += m_Columns[ j ].width;
			}

			// update the Y
			y += pFont->size().height;
		}

		setText( sList );
		setWindow( RectInt( m_Window.upperLeft(), listSize ) );

		m_ListInvalid = false;
	}
}

//----------------------------------------------------------------------------

int WindowList::addColumn( int width, Color color )
{
	if ( width < 0 )
		width = m_Window.width();

	Column newColumn;
	newColumn.width = width;
	newColumn.color = color;

	return( addColumn( newColumn ) );
}

//----------------------------------------------------------------------------

void WindowList::onSelectChange( int item )
{
	hoverSound();
	postMessage( WL_SELECT_CHANGE, (dword)this, item );

}

void WindowList::onSelect( int item )
{
	selectSound();
	postMessage( WL_SELECT, (dword)this, item );
}

//----------------------------------------------------------------------------

WindowList * WindowList::s_pSorting = NULL;
int WindowList::sortCallback( const void * arg1, const void * arg2 )
{
	int  column = s_pSorting->m_SortByColumn;
	return 	s_pSorting->m_SortAscending ? 
		stricmp<char>( s_pSorting->m_List[ column ][ *((int *)arg1) ],
			s_pSorting->m_List[ column ][ *((int *)arg2) ] ) :
		stricmp<char>( s_pSorting->m_List[ column ][ *((int *)arg2) ],
			s_pSorting->m_List[ column ][ *((int *)arg1) ] );
}


//----------------------------------------------------------------------------
// EOF

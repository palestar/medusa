/*
	WindowTree.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define GUI3D_DLL
#include "Debug/Assert.h"
#include "Standard/Constant.h"
#include "Gui3d/WindowTree.h"

//----------------------------------------------------------------------------

const dword TREE_BUTTON_STYLE = NodeWindow::SMOOTH_LR | NodeWindow::SMOOTH_LL;
const dword TREE_BUTTON_BORDER = 2;
const Color TREE_GLOW_INNER( 255,255,255,255 );
const Color TREE_GLOW_OUTER( 0,0,0,0 );

Constant WINDOW_TREE_SPACING( "WINDOW_TREE_SPACING", 2.0f );
Constant TREE_ITEM_BUFFER( "TREE_ITEM_BUFFER", 4.0f );
Constant TREE_SHADE_COLOR( "TREE_SHADE_COLOR", 0.80f );
Constant TREE_LIGHT_COLOR( "TREE_LIGHT_COLOR", 1.5f );
Constant TREE_GLOW_SIZE( "TREE_GLOW_SIZE", 4.0f );

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowTree, NodeWindow );
REGISTER_FACTORY_KEY(  WindowTree, 4546858653980140250 );

BEGIN_PROPERTY_LIST( WindowTree, NodeWindow );
END_PROPERTY_LIST();

WindowTree::WindowTree() : m_Indent( 5 ), m_pRoot( NULL ), m_pCursorOver( NULL ), m_pSelected( NULL ), m_CursorInWindow( false )
{
	reset();

	// add test data to this tree
	Item * pRoot1 = insertItem( STR("Tree1"), WHITE, BUTTON, 0 );
	insertItem( STR("Child5"), WHITE, 0, 0, pRoot1 );
	insertItem( STR("Child1"), WHITE, 0, 0, pRoot1 );
	insertItem( STR("Child3"), WHITE, 0, 0, pRoot1 );
	insertItem( STR("Child4"), WHITE, 0, 0, pRoot1 );
	insertItem( STR("Child2"), WHITE, 0, 0, pRoot1 );
	sort( pRoot1 );

	Item * pRoot2 = insertItem( STR("Tree2"), WHITE, BUTTON, 0 );
	insertItem( STR("Child2"), WHITE, 0, 0, pRoot2 );
	insertItem( STR("Child1"), WHITE, 0, 0, pRoot2 );
	insertItem( STR("Child4"), WHITE, 0, 0, pRoot2 );
	insertItem( STR("Child5"), WHITE, 0, 0, pRoot2 );
	insertItem( STR("Child3"), WHITE, 0, 0, pRoot2 );
	sort( pRoot2 );

	Item * pRoot3 = insertItem( STR("Tree3"), WHITE, BUTTON, 0 );
	insertItem( STR("Child4"), WHITE, 0, 0, pRoot3 );
	insertItem( STR("Child3"), WHITE, 0, 0, pRoot3 );
	insertItem( STR("Child5"), WHITE, 0, 0, pRoot3 );
	insertItem( STR("Child2"), WHITE, 0, 0, pRoot3 );
	insertItem( STR("Child1"), WHITE, 0, 0, pRoot3 );
	sort( pRoot3 );
}

WindowTree::~WindowTree()
{
	delete m_pRoot;
}

//----------------------------------------------------------------------------

void WindowTree::onRender( RenderContext & context, const RectInt & window )
{
	NodeWindow::onRender( context, window );

	// clear the highlighted item before rendering
	m_pCursorOver = NULL;

	// use a queue to render the tree without recursion
	Queue< Item * > stack;

	int i = 0;
	for(;i<m_pRoot->children.size();i++)
		stack.push( m_pRoot->children[ i ] );

	PointInt pos( window.left, window.top );
	while( stack.valid() )
	{
		Item * pItem = *stack;
		stack.pop();

		onRenderItem( context, window, pos, pItem );

		// if this item is expanded, insert any children into the queue
		if ( pItem->dwFlags & EXPANDED )
		{
			for(i=0;i<pItem->children.size();i++)
				stack.insert( pItem->children[ i ] );
		}
	}

	// update the window height
	setWindowHeight( pos.y - window.top );
}

bool WindowTree::onCursorMove( const RectInt & window, const PointInt & position, const PointInt & delta )
{
	if ( visible() )
	{
		if ( window.inRect( position ) )
		{
			m_CursorInWindow = true;
			m_LastCursorPosition = windowToScreen( position );

			setCursorState( SELECT );
			return true;
		}
		else
		{
			m_CursorInWindow = false;
		}
	}

	return NodeWindow::onCursorMove( window, position, delta );
}

bool WindowTree::onLeftUp( const RectInt & window, const PointInt & position )
{
	if ( m_pCursorOver != NULL )
		onSelect( m_pCursorOver );

	return NodeWindow::onLeftUp( window, position );
}

bool WindowTree::onRightUp( const RectInt & window, const PointInt & position )
{
	if ( m_pCursorOver != NULL )
		onMenu( m_pCursorOver );

	return NodeWindow::onRightUp( window, position );
}

//----------------------------------------------------------------------------

int WindowTree::indent() const
{
	return m_Indent;
}

int WindowTree::itemCount() const
{
	return m_pRoot->children.size();
}

WindowTree::Item * WindowTree::item( int n ) const
{
	if ( m_pRoot->children.isValid( n ) )
		return m_pRoot->children[ n ];

	return NULL;
}

WindowTree::Item * WindowTree::root() const
{
	return m_pRoot;
}

int WindowTree::itemDepth( Item * pItem ) const
{
	int depth = 0;
	while( pItem != m_pRoot && pItem != NULL )
	{
		pItem = pItem->pParent;
		depth++;
	}

	return depth;
}

//----------------------------------------------------------------------------

void WindowTree::setIndent( int pixels )
{
	m_Indent = pixels;
}

WindowTree::Item * WindowTree::insertItem( const wchar * pLabel, Color color, dword flags, dword user, Item * pParent /*= NULL*/ )
{
	if ( pParent == NULL )
		pParent = m_pRoot;

	Item * pItem = new Item;
	pItem->sLabel = pLabel;
	pItem->cColor = color;
	pItem->dwFlags = flags;
	pItem->dwUser = user;
	pItem->pParent = pParent;
	
	// add this item to the parents child list
	pParent->children.push( pItem );
	// return the new item
	return pItem;
}

bool WindowTree::deleteItem( Item * pItem )
{
	if ( pItem == NULL || pItem == m_pRoot )
		return false;

	// remove this item from the parent child list
	pItem->pParent->children.removeSearch( pItem );
	pItem->pParent = NULL;

	// now free the memory
	rdelete( pItem );

	return true;
}

void WindowTree::reset()
{
	// free all items
	rdelete( m_pRoot );
	// create a new root item
	m_pRoot = new Item;
	m_pRoot->sLabel = "ROOT";
}

void WindowTree::expand( Item * pItem )
{
	pItem->dwFlags |= EXPANDED;
}

void WindowTree::collapse( Item * pItem )
{
	pItem->dwFlags &= ~EXPANDED;
}

void WindowTree::sort( Item * pItem )
{
	pItem->children.qsort( compare );
}

//----------------------------------------------------------------------------

void WindowTree::onHighlight( Item * pItem )
{
	m_pCursorOver = pItem;
}

void WindowTree::onSelect( Item * pItem )
{
	m_pSelected = pItem;

	if ( m_pSelected != NULL && m_pSelected->children.size() > 0 )
	{
		if ( m_pSelected->dwFlags & EXPANDED )
			m_pSelected->dwFlags &= ~EXPANDED;
		else
			m_pSelected->dwFlags |= EXPANDED;
	}
}

void WindowTree::onMenu( Item * pItem )
{
}

void WindowTree::onRenderItem( RenderContext & context, const RectInt & window, PointInt & pos, Item * pItem )
{
	WindowStyle * pStyle = windowStyle();
	ASSERT( pStyle );
	Font * pFont = windowStyle()->font();
	ASSERT( pFont );

	// get the depth of this item
	int depth = itemDepth( pItem );
	// determine the X position based on it's depth
	pos.x = window.left + (depth * m_Indent);
	// get the size of the label text
	SizeInt labelSize( pFont->size( pItem->sLabel ) );
	// determine the height of this item
	int height = (int)(labelSize.height + TREE_ITEM_BUFFER);
	RectInt itemRect( window.left, pos.y, window.right, pos.y + height );

	// check if this item is highlighted or not
	if ( m_CursorInWindow && itemRect.inRect( m_LastCursorPosition ) )
		onHighlight( pItem );

	PrimitiveMaterial::push( context.display(), WHITE, PrimitiveMaterial::ALPHA );

	if ( pItem->dwFlags & BUTTON )
	{
		bool expanded = (pItem->dwFlags & EXPANDED) != 0;
		Color backColor( expanded ? pStyle->backColor() * TREE_SHADE_COLOR : pStyle->backColor() );
		Color shadeColor( expanded ? pStyle->borderColor() * TREE_LIGHT_COLOR : pStyle->borderColor() * TREE_SHADE_COLOR );
		Color lightColor( expanded ? pStyle->borderColor() * TREE_SHADE_COLOR : pStyle->borderColor() * TREE_LIGHT_COLOR );

		// render the button
		pushBackground( context, itemRect, backColor, TREE_BUTTON_STYLE, TREE_BUTTON_BORDER );
		pushBorder( context, itemRect, lightColor, shadeColor, TREE_BUTTON_STYLE, TREE_BUTTON_BORDER );

		// draw glow around this object if the mouse is currently over this button
		if ( m_pCursorOver == pItem )
			pushGlow( context, itemRect, TREE_GLOW_SIZE, TREE_GLOW_INNER, TREE_GLOW_OUTER, TREE_BUTTON_STYLE, TREE_BUTTON_BORDER );

		// place the label in the center of the button
		PointInt labelPos( itemRect.center() - PointInt( labelSize.width / 2, labelSize.height / 2 ) );
		// draw the label
		Font::push( context.display(), pFont, labelPos, pItem->sLabel, pItem->cColor );
	}
	else
	{
		if ( m_pSelected == pItem )
			pushBackground( context, itemRect, pStyle->highColor(), 0, 0 );
		if ( m_pCursorOver == pItem )
			pushGlow( context, itemRect, TREE_GLOW_SIZE, TREE_GLOW_INNER, TREE_GLOW_OUTER, 0, 0 );

		PointInt labelPos( pos.x, (int)(pos.y + (TREE_ITEM_BUFFER / 2)) );
		// render the label text`
		Font::push( context.display(), pFont, labelPos, pItem->sLabel, pItem->cColor );

	}


	// move y down
	pos.y += (int)(height + WINDOW_TREE_SPACING);
}

//----------------------------------------------------------------------------

void WindowTree::rdelete( Item * pItem )
{
	if ( pItem != NULL )
	{
		if ( m_pSelected == pItem )
			m_pSelected = NULL;
		if ( m_pCursorOver == pItem )
			m_pCursorOver = NULL;

		for(int i=0;i<pItem->children.size();i++)
			rdelete( pItem->children[ i ] );

		delete pItem;
	}
}

int WindowTree::compare( Item * pItem1, Item * pItem2 )
{
	return pItem2->sLabel.compare( pItem1->sLabel );
}

//----------------------------------------------------------------------------
// EOF

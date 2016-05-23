/*
	WindowLayout.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define GUI3D_DLL
#include "Gui3d/WindowLayout.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( WindowLayout, NodeWindow );
REGISTER_FACTORY_KEY(  WindowLayout, 4186554018689568524 );

BEGIN_PROPERTY_LIST( WindowLayout, NodeWindow );
	ADD_BIT_OPTION( m_Style, H_CENTER );
	ADD_BIT_OPTION( m_Style, H_RIGHT );
	ADD_BIT_OPTION( m_Style, V_BOTTOM );
	ADD_BIT_OPTION( m_Style, H_SPACING_X2 );
	ADD_BIT_OPTION( m_Style, V_SPACING_X2 );
	ADD_BIT_OPTION( m_Style, SORT );
	ADD_BIT_OPTION( m_Style, LAYOUT_VERT );
END_PROPERTY_LIST();

WindowLayout::WindowLayout()
{}

//----------------------------------------------------------------------------

void WindowLayout::onRender( RenderContext & context, const RectInt & screen )
{
	// update the layout every render
	doLayout();
	// call the base class
	NodeWindow::onRender( context, screen );
}

//----------------------------------------------------------------------------

void WindowLayout::doLayout()
{
	if ( childCount() > 0 )
	{
		// sort all child nodes by name
		if ( style() & SORT )
			sortNodes();

		// make a list of visible child windows
		Array< NodeWindow * > children;
		int i = 0;
		for(;i<childCount();i++)
		{
			NodeWindow * pChild = WidgetCast<NodeWindow>( child(i) );
			if ( pChild != NULL && pChild->visible() )
			{
				// add the child to the array
				children.push( pChild );
				// check the placement of the child (it may be docked or locked to the parent height or width)
				pChild->checkPlacement();
			}
		}

		if ( children.size() == 0 )
			return;		// no visible children to layout

		// calculate the spacing between child windows
		int hs = H_SPACING;
		if ( m_Style & H_SPACING_X2 )
			hs *= 2;
		int vs = V_SPACING;
		if ( m_Style & V_SPACING_X2 )
			vs *= 2;

		RectInt localWindow( window() - window().upperLeft() );
		SizeInt localSize( localWindow.size() );

		if ( (m_Style & LAYOUT_VERT) != 0 )
		{
			int x = 0;		// assume left justified
			if ( (m_Style & H_RIGHT) != 0 )
				x = localSize.width;

			for(i=0;i<children.size();)
			{
				int maxWidth = 0;
				int height = 0;
				int j = i;
				for(;j<children.size();j++)
				{
					SizeInt childSize( children[j]->window().size() );
					if ( height > 0 && (height + childSize.height) > localSize.height )
						break;		// note: height > 0, allows a child who is larger than the parent window, otherwise infinite loop

					maxWidth = Max( maxWidth, childSize.width );
					height += (childSize.height + hs);
				}

				int y = 0;	
				for(;i<children.size() && i < j;i++)
				{
					RectInt childWindow( children[i]->window() );
					SizeInt childSize( childWindow.size() );

					int yy = y;
					if ( (m_Style & V_BOTTOM ) != 0 )
						yy = localSize.height - y - childSize.height;
					int xx = x;
					if ( (m_Style & H_RIGHT) != 0 )
						xx -= maxWidth;

					children[i]->setWindow( RectInt( PointInt( xx, yy ), childSize ) );

					y += (childSize.height + vs);
				}

				if ( (m_Style & H_RIGHT) == 0 )
					x += (maxWidth + hs);
				else 
					x -= (maxWidth + hs);
			}
		}
		else
		{
			int y = 0;		// assume top justified
			if ( (m_Style & V_BOTTOM) != 0 )
				y = localSize.height;

			for(i=0;i<children.size();)
			{
				int maxHeight = 0;
				int width = 0;
				int j = i;
				for(;j<children.size();j++)
				{
					SizeInt childSize( children[j]->window().size() );
					if ( width > 0 && (width + childSize.width) > localSize.width )
						break;		// note: width > 0, allows a child who is larger than the parent window, otherwise infinite loop

					maxHeight = Max( maxHeight, childSize.height );
					width += (childSize.width + hs);
				}

				int x = 0;		// assume left justified
				if ( (m_Style & H_CENTER) != 0 )
					x = (localSize.width / 2) - (width / 2);
				else if ( (m_Style & H_RIGHT) != 0 )
					x = localSize.width - width;
				
				for(;i<children.size() && i < j;i++)
				{
					RectInt childWindow( children[i]->window() );
					SizeInt childSize( childWindow.size() );

					int yy = y;
					if ( (m_Style & V_BOTTOM ) )
						yy = y - childSize.height;
					RectInt newWindow( PointInt( x, yy ), childSize );
					children[i]->setWindow( newWindow );

					x += (childSize.width + hs);
				}

				if ( (m_Style & V_BOTTOM) == 0 )
					y += (maxHeight + vs);
				else 
					y -= (maxHeight + vs);
			}
		}
	}
}

//----------------------------------------------------------------------------
//EOF


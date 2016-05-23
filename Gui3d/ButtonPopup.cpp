/*
	ButtonPopup.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#define GUI3D_DLL
#include "Gui3d/ButtonPopup.h"

//----------------------------------------------------------------------------

IMPLEMENT_FACTORY( ButtonPopup, WindowButton );
REGISTER_FACTORY_KEY(  ButtonPopup, 4413945551862954417 );

BEGIN_PROPERTY_LIST( ButtonPopup, WindowButton );
	ADD_BIT_OPTION( m_Style, POPDOWN_SIBLINGS );
	ADD_BIT_OPTION( m_Style, DISABLE_CHILDREN );
	ADD_BIT_OPTION( m_Style, HIDE_SIBLINGS );
END_PROPERTY_LIST();

ButtonPopup::ButtonPopup() : m_Up( false )
{}

//----------------------------------------------------------------------------

void ButtonPopup::onActivate()
{
	WindowButton::onActivate();

	// on activation, popdown or popup our child objects if this button is down or not
	if ( buttonDown() )
		popUp();
	else
		popDown();	
}

//----------------------------------------------------------------------------

void ButtonPopup::onButtonDown()
{
	// call base class
	WindowButton::onButtonDown();

	if ( toggle() )
		popUp();
}

void ButtonPopup::onButtonUp()
{
	// call base class
	WindowButton::onButtonUp();

	if (! toggle() )
	{
		if ( up() )
			popDown();
		else
			popUp();
	}
	else
		popDown();
}

void ButtonPopup::onButtonAbort()
{
	// only hide windows if this isn't a toggle button
	if ( !toggle() && up() )
	{
		// call base class
		WindowButton::onButtonAbort();
		// left click outside of this window occured, hide all child windows
		popDown();
	}
}

//----------------------------------------------------------------------------

void ButtonPopup::popDown()
{
	m_Up = false;

	int i = 0;
	for(;i<childCount();i++)
	{
		NodeWindow * pWindow = WidgetCast<NodeWindow>( child(i) );
		if ( pWindow != NULL )
			pWindow->hideWindow( (m_Style & DISABLE_CHILDREN) != 0 ? false : true );
	}

	if ( (m_Style & HIDE_SIBLINGS) != 0 )
	{
		BaseNode * pParent = parent();
		if ( pParent != NULL )
		{
			for(i=0;i<pParent->childCount();i++)
			{
				NodeWindow * pSibling = WidgetCast<NodeWindow>( pParent->child(i) );
				if ( pSibling != NULL && pSibling != this )
					pSibling->showWindow();
			}
		}
	}
}

void ButtonPopup::popUp()
{
	m_Up = true;

	// show all child nodes
	int i = 0;
	for(;i<childCount();i++)
	{
		NodeWindow * pWindow = WidgetCast<NodeWindow>( child(i) );
		if ( pWindow != NULL )
			pWindow->showWindow();
	}

	if ( (m_Style & POPDOWN_SIBLINGS) != 0 )
	{
		// check for sibling popup nodes, pop them down 
		BaseNode * pParent = parent();
		if ( pParent != NULL )
		{
			for(i=0;i<pParent->childCount();i++)
			{
				ButtonPopup * pPopup = WidgetCast<ButtonPopup>( pParent->child(i) );
				if ( pPopup != NULL && pPopup != this && pPopup->buttonDown() && (pPopup->style() & POPDOWN_SIBLINGS) != 0)
				{
					if ( pPopup->toggle() )
						pPopup->onButtonUp();
					else
						pPopup->onButtonAbort();
				}
			}
		}
	}

	if ( (m_Style & HIDE_SIBLINGS) != 0 )
	{
		BaseNode * pParent = parent();
		if ( pParent != NULL )
		{
			for(i=0;i<pParent->childCount();i++)
			{
				NodeWindow * pSibling = WidgetCast<NodeWindow>( pParent->child(i) );
				if ( pSibling != NULL && pSibling != this )
					pSibling->hideWindow();
			}
		}
	}
}

//----------------------------------------------------------------------------
//EOF

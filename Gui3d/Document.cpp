/*
	Document.cpp

	This base class is created by the InterfaceContext for the NodeInterfaceClient objects
	attached to the context. 
	
	The user should derive a class from this object and implement the onMessage() function 
	which will allow this document to control / monitor all messages received by the 
	InterfaceContext and it's nodes.

	(c)2005 PaleStar Development, Richard Lyle
*/

#define GUI3D_DLL
#include "Gui3d/Document.h"
#include "Gui3d/NodeWindow.h"

//-------------------------------------------------------------------------------

IMPLEMENT_FACTORY( Document, Widget );
REGISTER_FACTORY_KEY(  Document, 4041050079922747562 );

BEGIN_PROPERTY_LIST( Document, Widget );
END_PROPERTY_LIST();

Document::Document()
{}

//----------------------------------------------------------------------------

void Document::onActivate()
{}

void Document::onLock()
{}

void Document::onPreUpdate()
{}

void Document::onUpdate( float t )
{}

void Document::onUnlock()
{}

bool Document::onMessage( const Message & msg )
{
	return false;
}

//----------------------------------------------------------------------------

bool Document::postMessage( dword message, dword wparam, dword lparam )
{
	Message msg;
	msg.message = message;
	msg.wparam = wparam;
	msg.lparam = lparam;

	return( CommandTarget::postMessage( msg ) );
}

bool Document::setScene( const char * pSceneName )
{
	return( postMessage( IC_SET_SCENE, StringHash( pSceneName ), 0 ) );
}

//----------------------------------------------------------------------------
// EOF



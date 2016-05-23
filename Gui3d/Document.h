/*
	Document.h

	This base class is created by the InterfaceContext for the NodeInterfaceClient objects
	attached to the context. 
	
	The user should derive a class from this object and implement the onMessage() function 
	which will allow this document to control / monitor all messages received by the 
	InterfaceContext and it's nodes.

	(c)2005 PaleStar Development, Richard Lyle
*/

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "Factory/Widget.h"
#include "Standard/StringHash.h"
#include "System/CommandTarget.h"
#include "Gui3d/GUI3DDll.h"

//-------------------------------------------------------------------------------

class DLL Document : public Widget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Types
	typedef CommandTarget::Message			Message;

	// Construction
	Document();

	// Mutators
	virtual void				onActivate();
	virtual void				onLock();											// called to lock the document before rendering or sending a message
	virtual void				onPreUpdate();										// called before document is rendered
	virtual void				onUpdate( float t );								// called to update the document, called after rendering is completed
	virtual void				onUnlock();											// called to unlock the document

	virtual bool				onMessage( const Message & msg );

	// Helpers
	static bool					postMessage( dword message, 
									dword wparam, dword lparam );					// this posts a message to the GUI
	static bool					setScene( const char * pSceneName );				// sets the current scene
};

//-------------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

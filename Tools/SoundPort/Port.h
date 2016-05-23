/*
	Port.h

	Port object
	(c)2005 Palestar, Richard Lyle
*/

#ifndef SOUNDPORT_H
#define SOUNDPORT_H

#include "Audio/Sound.h"
#include "Tools/ResourcerDoc/Port.h"
#include "Standard/Dll.h"

//-------------------------------------------------------------------------------

#undef DLL
#ifdef SOUNDPORT_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT
#endif

//----------------------------------------------------------------------------

class DLL SoundPort : public Port
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	// Construction
	SoundPort();
	// Widget Interface
	bool						read( const InStream & input );
	// Port interface
	Resource::Ref				createResource();							// create the resource 
	CFrameWnd *					createView();								// create a view of the port

	bool						importFile( const char * fileName );		// import from file
	void						shellOpen() const;							// open the source object through a shell program

	// Data
	Sound::Ref					m_pSound;
};


#endif

//-------------------------------------------------------------------------------
// EOF

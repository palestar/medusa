/*
	Platform.h

	Platform interface, this class provides an abstract interface to the platform that Medusa is running under. 
	This is a singleton object, their should be only one of this objects per application.

	(c)2005 Palestar, Richard Lyle
*/

#ifndef SYSTEM_PLATFORM_H
#define SYSTEM_PLATFORM_H

#include "Display/DisplayDevice.h"
#include "Audio/AudioDevice.h"
#include "Draw/Image.h"
#include "Standard/Types.h"
#include "Factory/FactoryTypes.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL Platform : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	enum Buttons								// Cursor buttons
	{
		LEFT		= 0x1,
		RIGHT		= 0x2,
		MIDDLE		= 0x4,
	};
	struct Config
	{
		Config() : instance( NULL ), 
			icon( NULL ),
			bWindowed( false ), 
			eFSAA( DisplayDevice::FSAA_NONE ),
			bSoftwareCursor( false ),
			bInvertButtons( false ),
			audioKhz( 44100 ), 
			audioChan( 2 ), 
			audioVoices( 32 ), 
			audioBits( 16 )
		{}

		void *				instance;			// application provided instance (windows hinstance)
		const char *		icon;				// icon resource to load
		CharString			className;			// class name of this application (i.e. DarkSpace)
		String				applicationName;	// name of this application (i.e. DarkSpace v1.527)

		CharString			displayMode;		// display mode to initialize the platform (640x480x16, 1024x768x32, ...)
		bool				bWindowed;			// true to keep display in windowed mode, otherwise fullscreen..
		DisplayDevice::FSAA	eFSAA;				// full screen anti-aliasing mode
		bool				bSoftwareCursor;	// if true, then the game will render the mouse cursor 
		bool				bInvertButtons;		// true to swap right/left cursor buttons
		int					audioKhz;			// audio parameters
		int					audioChan;
		int					audioVoices;
		int					audioBits;
	};
	typedef Reference< Platform >		Ref;
    
	// Construction
	Platform();
	virtual ~Platform();

	// Accessors
	virtual const Config &	config() const = 0;
	virtual DisplayDevice *	display() const = 0;
	virtual AudioDevice *	audio() const = 0;

	virtual bool			cursorEnabled() const = 0;
	virtual PointInt		cursorPosition() const = 0;
	virtual dword			cursorButtons() const = 0;
	virtual bool			buttonsInverted() const = 0;

	// Mutators
	virtual bool			initialize( Config & context ) = 0;			// returns false on failure	
	virtual bool			uninitialize() = 0;
	virtual bool			update() = 0;

	virtual void			setSoftwareCursor( bool bSoftwareCursor ) = 0;
	virtual bool			setHardwareCursor( Image::Link pImage, int nFrame ) = 0;
	virtual bool			showHardwareCursor( bool bShow ) = 0;			// show or hide hardware cursor
	virtual void			enableCursor( bool enable ) = 0;				// enable position updates, doesn't prevent delta messages from being sent
	virtual void			invertButtons( bool bInverted ) = 0;			// true to invert left/right mouse buttons
	virtual void			setCursorPosition( const PointInt & pos ) = 0;

	//! This function returns the name of the given button - this function checks the invertButtons() flag.
	static const char *		buttonText( Buttons a_eButton );

	// Static
	static Platform *		create();	
	// Static Data
	static Platform *		sm_pPlatform;
};

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

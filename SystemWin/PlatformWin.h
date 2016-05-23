/*
	PlatformWin.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PLATFORMWIN_H
#define PLATFORMWIN_H

#include "System/Platform.h"
#include "SystemWin/SystemWin.h"

// Fix for VS2005
#ifndef POINTER_64
#define POINTER_64		__ptr64
#endif

#define _WIN32_WINDOWS  0x501                                     
#define _WIN32_WINNT    0x501
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//----------------------------------------------------------------------------

class DLL PlatformWin : public Platform
{
public:
	DECLARE_WIDGET_CLASS();

	// Construction
	PlatformWin();
	virtual ~PlatformWin();

	// Platform interface
	virtual const Config &	config() const;
	virtual DisplayDevice *	display() const;
	virtual AudioDevice *	audio() const;

	virtual bool			cursorEnabled() const;
	virtual PointInt		cursorPosition() const;
	virtual dword			cursorButtons() const;
	virtual bool			buttonsInverted() const;
	
	virtual bool			initialize( Config & context );			
	virtual bool			update();			
	virtual bool			uninitialize();

	virtual void			setSoftwareCursor( bool bSoftwareCursor );
	virtual bool			setHardwareCursor( Image::Link pImage, int nFrame );
	virtual bool			showHardwareCursor( bool bShow );
	virtual void			enableCursor( bool enable );			
	virtual void			invertButtons( bool bInverted );	
	virtual void			setCursorPosition( const PointInt & pos );

	//! Process incoming windows messages
	long					winProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

private:
	//! Handle the real cursor states
	bool					activateCursor();
	bool					deactivateCursor();

	// Data
	Config					m_Config;
	CharString				m_sClass;
	CharString				m_sApplication;
	HINSTANCE				m_hInstance;
	ATOM					m_Class;
	HWND					m_hWnd;

	DisplayDevice::Ref		m_Display;
	bool					m_InFullScreen;
	AudioDevice::Ref		m_Audio;

	bool					m_bCursorActive;	// true when cursor is inside our window
	bool					m_bCursorVisible;	// true when hardware cursor is visible
	bool					m_CursorEnabled;
	PointInt				m_CursorPosition;
	dword					m_CursorButtons;
	bool					m_bInvertButtons;	// true to swap left/right mouse buttons
};

//----------------------------------------------------------------------------

inline const Platform::Config & PlatformWin::config() const
{
	return m_Config;
}

inline DisplayDevice * PlatformWin::display() const
{
	return( m_Display );
}

inline AudioDevice * PlatformWin::audio() const
{
	return( m_Audio );
}

inline bool PlatformWin::cursorEnabled() const
{
	return( m_CursorEnabled );
}

inline PointInt PlatformWin::cursorPosition() const
{
	return( m_CursorPosition );
}

inline dword PlatformWin::cursorButtons() const
{
	return( m_CursorButtons );
}

inline bool PlatformWin::buttonsInverted() const
{
	return m_bInvertButtons;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

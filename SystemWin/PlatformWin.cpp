/*
	PlatformWin.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define SYSTEMWIN_DLL
#include "Debug/Assert.h"
#include "Debug/Profile.h"
#include "Audio/Sound.h"
#include "Render3d/NodeSound.h"
#include "Render3d/Material.h"
#include "Standard/Settings.h"
#include "System/Messages.h"
#include "System/CommandTarget.h"
#include "System/Keyboard.h"
#include "SystemWin/PlatformWin.h"

//---------------------------------------------------------------------------------------------------

IMPLEMENT_FACTORY( PlatformWin, Platform );

//----------------------------------------------------------------------------

#define IGNORE_AUDIO_FAILURE			// don't error if audio fails to initialize

#if defined(_DEBUG)
const char * SETTINGS_SECTION = "ClientD";
#else
const char * SETTINGS_SECTION = "Client";
#endif

const char * DEFAULT_MODE = "1024x768";
const int DEFAULT_WINDOW_WIDTH = 1024;
const int DEFAULT_WINDOW_HEIGHT = 768;
const int DEFAULT_WINDOW_DEPTH = 16;
const int DI_MOUSE_BUFFERSIZE = 16;

//----------------------------------------------------------------------------

LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( Platform::sm_pPlatform )
		return ((PlatformWin *)Platform::sm_pPlatform)->winProc( hwnd, message, wParam, lParam );

	return ::DefWindowProc(hwnd, message, wParam, lParam);
}

//! Handle the real cursor states
bool PlatformWin::activateCursor()
{
	if ( !m_bCursorActive )
	{
		TRACKMOUSEEVENT trackStruct;
		trackStruct.cbSize      = sizeof(TRACKMOUSEEVENT);
		trackStruct.dwFlags     = TME_LEAVE;
		trackStruct.hwndTrack   = m_hWnd;
		trackStruct.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&trackStruct);

		m_bCursorActive = true;
		if ( m_Config.bSoftwareCursor )
			showHardwareCursor( false );
	}

	return true;
}

bool PlatformWin::deactivateCursor()
{
	if ( m_bCursorActive )
	{
		m_bCursorActive = false;
		if ( m_Config.bSoftwareCursor )
			showHardwareCursor( true );
	}

	return true;
}

long PlatformWin::winProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	if ( message >= HM_KEYFIRST && message <= HM_KEYLAST )
	{
		if ( message == WM_SYSKEYDOWN && wParam == VK_RETURN )
		{
			// toggle fullscreen mode..
			if ( m_Display )
			{
				Settings settings( SETTINGS_SECTION );
				if ( m_Display->setMode( m_Display->activeMode(), !m_Display->windowed() ) )
					settings.put( "windowed", m_Display->windowed() );
			}
			return 0;
		}

		dword key = wParam;
		if ( message != HM_CHAR && message != HM_SYSKEYDOWN && message != HM_SYSKEYUP )
		{
			// add modifier flags to add keyboard messages except for HM_CHAR, since controls using HM_CHAR
			// want't ansi characters only
			if ( Keyboard::shiftDown() )
				key |= HMK_SHIFT_MASK;
			if ( Keyboard::ctrlDown() )
				key |= HMK_CONTROL_MASK;
			if ( Keyboard::altDown() )
				key |= HMK_ALT_MASK;

			// remap the key
			key = Keyboard::map( key );
		}
		
		// pass the message onto the medusa interface
		CommandTarget::postWindowMessage( hwnd, message, key, lParam );
		// send message onto the windows default procedure now
		return 0;
	}
	
	if ( m_bInvertButtons && message >= HM_LBUTTONDOWN && message <= HM_RBUTTONDBLCLK )
	{
		static const UINT INVERT_REMAP[] = 
		{
			WM_RBUTTONDOWN, // HM_LBUTTONDOWN	
			WM_RBUTTONUP, // HM_LBUTTONUP	
			WM_RBUTTONDBLCLK, // HM_LBUTTONDBLCLK
			WM_LBUTTONDOWN, // HM_RBUTTONDOWN	
			WM_LBUTTONUP, // HM_RBUTTONUP	
			WM_LBUTTONDBLCLK, // HM_RBUTTONDBLCLK
		};

		message = INVERT_REMAP[ message - WM_LBUTTONDOWN ];
	}

	switch( message )
	{
	case WM_MOVE:
		{
			RECT r;
			GetWindowRect( m_hWnd, &r );

			// store the new window position into the registry, so on next run we can restore the windows position..
			Settings settings( SETTINGS_SECTION );
			settings.put( "X", r.left );
			settings.put( "Y", r.top );
		}
		break;
	case WM_SIZE:
		break;
	case WM_SETCURSOR:
		return 1;
	case WM_MOUSEMOVE:
		// ignore all mouse move messages, we poll for the cursor position always..
		activateCursor();
		return 0;
	case WM_MOUSEACTIVATE:
		activateCursor();
		return MA_ACTIVATE;
	case WM_MOUSELEAVE:
		deactivateCursor();
		return 0;
	case WM_DESTROY:
		PostMessage( NULL, HM_QUIT, 0, 0 );
		return 0;
	case HM_MOUSEWHEEL:
		CommandTarget::postWindowMessage( m_hWnd, HM_MOUSEWHEEL, wParam, (dword)&m_CursorPosition );
		return 0;
	case WM_LBUTTONDOWN:
		m_CursorButtons |= LEFT;
		CommandTarget::postWindowMessage( m_hWnd, HM_LBUTTONDOWN, 0, (dword)&m_CursorPosition );
		return 0;
	case WM_LBUTTONUP:
		m_CursorButtons &= ~LEFT;
		CommandTarget::postWindowMessage( m_hWnd, HM_LBUTTONUP, 0, (dword)&m_CursorPosition );
		return 0;
	case WM_RBUTTONDOWN:
		m_CursorButtons |= RIGHT;
		CommandTarget::postWindowMessage( m_hWnd, HM_RBUTTONDOWN, 0, (dword)&m_CursorPosition );
		return 0;
	case WM_RBUTTONUP:
		m_CursorButtons &= ~RIGHT;
		CommandTarget::postWindowMessage( m_hWnd, HM_RBUTTONUP, 0, (dword)&m_CursorPosition );
		return 0;
	case WM_MBUTTONDOWN:
		m_CursorButtons |= MIDDLE;
		CommandTarget::postWindowMessage( m_hWnd, HM_MBUTTONDOWN, 0, (dword)&m_CursorPosition );
		return 0;
	case WM_MBUTTONUP:
		m_CursorButtons &= ~MIDDLE;
		CommandTarget::postWindowMessage( m_hWnd, HM_MBUTTONUP, 0, (dword)&m_CursorPosition );
		return 0;
	}

	CommandTarget::postWindowMessage( hwnd, message, wParam, lParam );
	return ::DefWindowProc(hwnd, message, wParam, lParam);
}

//----------------------------------------------------------------------------

PlatformWin::PlatformWin()
{
	m_hInstance = NULL;
	m_Class = NULL;
	m_hWnd = NULL;
	m_InFullScreen = false;

	m_bCursorActive = false;
	m_bCursorVisible = true;
	m_CursorEnabled = true;
	m_CursorPosition.set( 320, 240 );
	m_CursorButtons = 0;
	m_bInvertButtons = false;
}

PlatformWin::~PlatformWin()
{
	uninitialize();
}

//----------------------------------------------------------------------------

bool PlatformWin::initialize( Config & context )
{
	m_Config = context;
	m_sClass = context.className;
	m_sApplication = context.applicationName;
	m_hInstance = (::HINSTANCE)context.instance;
	m_InFullScreen = !context.bWindowed;
	m_bInvertButtons = context.bInvertButtons;

	int nDevice = 0, nWidth = 0, nHeight = 0, nDepth = 0;
	if ( sscanf_s( context.displayMode, "%d:%dx%dx%d", &nDevice, &nWidth, &nHeight, &nDepth ) != 4 )
	{
		TRACE( "Failed to parse display mode, falling back to default..." );

		nDevice = 0;
		nWidth = DEFAULT_WINDOW_DEPTH;
		nHeight = DEFAULT_WINDOW_HEIGHT;
		nDepth = DEFAULT_WINDOW_DEPTH;
	}

	// register the window class
	if ( m_Class == NULL )
	{
		WNDCLASS wcl;

		wcl.hInstance     = m_hInstance;
		wcl.lpszClassName = m_sClass;
		wcl.lpfnWndProc   = windowProc;
		wcl.style         = CS_HREDRAW | CS_VREDRAW;
		wcl.hIcon         = LoadIcon(m_hInstance, m_Config.icon );
		wcl.hCursor       = NULL;
		wcl.lpszMenuName  = NULL;
		wcl.cbClsExtra    = 0;
		wcl.cbWndExtra    = 0;
		
		wcl.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	
		if(! (m_Class = RegisterClass(&wcl)) )
			return false;
	}

	// create the window
	if ( m_hWnd == NULL )
	{
		Settings settings( SETTINGS_SECTION );
		int cx = nWidth + (GetSystemMetrics(SM_CXSIZEFRAME) * 2);
		int cy = nHeight + (GetSystemMetrics(SM_CYSIZEFRAME) * 2) 
			+ GetSystemMetrics(SM_CYCAPTION);
		// grab the last window position from the registry...
		int x = settings.get( "X", GetSystemMetrics(SM_CXFULLSCREEN) / 2 - (cx / 2) );
		//int cx = reg.get( "CX", nWidth + (GetSystemMetrics(SM_CXSIZEFRAME) * 2) );
		int y = settings.get( "Y", GetSystemMetrics(SM_CYFULLSCREEN) / 2 - (cy / 2) );
		//int cy = reg.get( "CY", nHeight + (GetSystemMetrics(SM_CYSIZEFRAME) * 2) 
		//	+ GetSystemMetrics(SM_CYCAPTION) );

		// create the window at the last known location...
		m_hWnd = CreateWindow(	m_sClass,
								m_sApplication,   					
								WS_OVERLAPPEDWINDOW,
								x,
								y,
								cx,
								cy,
								NULL,
								NULL,
								m_hInstance,
								NULL);

		if( m_hWnd == NULL )
		{
			MessageBox( NULL, "Failed to create window!", "Display Failure", MB_OK );
			return false;
		}

		ShowWindow( m_hWnd, SW_SHOWNORMAL );
		SetFocus(m_hWnd );
	}

	// create the display device
	if (! m_Display.valid() )
	{
		// create a D3D display device for Windows 
		m_Display = DisplayDevice::create( "DisplayDeviceD3D" );
		if (! m_Display.valid() )
		{
			MessageBox( NULL, "Failed to create display class!", "Display Failure", MB_OK );
			return false;
		}

		// find the display mode
		m_Display->lock();

		const DisplayDevice::Mode * pDefaultMode = NULL;
		bool bDisplayInitialized = false;
		for(int k=0;k<m_Display->modeCount();k++)
		{
			const DisplayDevice::Mode * pMode = m_Display->mode(k);
			if ( pDefaultMode == NULL && strstr( pMode->modeDescription, DEFAULT_MODE ) != NULL )
				pDefaultMode = pMode;

			if ( context.displayMode == pMode->modeDescription )
			{
				if ( bDisplayInitialized = m_Display->initialize( m_hWnd, pMode, context.bWindowed, true, context.eFSAA ) )
					break;
			}
		}

		// look for a default mode then..
		if (! bDisplayInitialized && pDefaultMode != NULL )
			bDisplayInitialized = m_Display->initialize( m_hWnd, pDefaultMode, context.bWindowed, true, context.eFSAA );

		m_Display->unlock();

		if ( !bDisplayInitialized )
		{
			DestroyWindow( m_hWnd );
			m_hWnd = NULL;

			MessageBox( NULL, "Failed to initialize display!", context.displayMode, MB_OK );
			return false;
		}
	}

	//MessageBox( NULL, L"Display initialized!" , L"", MB_OK );

	// create the audio device
	if (! m_Audio.valid() )
	{
		m_Audio = AudioDevice::create( "AudioDeviceDS" );
		if ( ! m_Audio.valid() )
		{
			MessageBox( NULL, "Failed to initialize audio class!", "Audio Failure", MB_OK );
			return false;
		}

		// check audio settings
		context.audioVoices = Clamp( context.audioVoices, 8, 128 );
		if ( context.audioKhz != 22050 || context.audioKhz != 44100 || context.audioKhz != 48000 )
			context.audioKhz = 44100;
		if ( context.audioChan != 1 || context.audioChan != 2 )
			context.audioChan = 2;
		if ( context.audioBits != 8 || context.audioBits != 16 )
			context.audioBits = 16;

		// initialize the audio device
		bool bAudioInitialized = m_Audio->initialize( m_hWnd, 
			context.audioVoices, 
			context.audioKhz, 
			context.audioBits, 
			context.audioChan );

#ifdef IGNORE_AUDIO_FAILURE
		if (! bAudioInitialized ) 
			m_Audio = NULL;		// release the audio object
#else
		if (! bAudioInitialized )
		{
			MessageBox( NULL, STR("Failed to initialize audio!"), STR("Audio Failure"), MB_OK );
			return false;
		}
#endif
	}

	return true;
}

bool PlatformWin::uninitialize()
{
	Sound::releaseAudioBuffers();
	//NodeSound::releaseAudioBuffers();

	if ( m_Audio.valid() )
	{
		m_Audio->release();
		m_Audio = NULL;
	}
	if ( m_Display.valid() )
	{
		m_Display->release();
		m_Display = NULL;
	}
	if ( m_hWnd != NULL )
	{
		DestroyWindow( m_hWnd );
		m_hWnd = NULL;
	}
	if ( m_Class != NULL )
	{
		UnregisterClass( (const char *)m_Class, m_hInstance );
		m_Class = NULL;
	}

	return true;
}

bool PlatformWin::update()
{
	// pump window messages
	MSG	msg;
	while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
	{
		if ( msg.message == HM_QUIT || msg.message == WM_QUIT )
			return false;
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	bool bWindowed = display()->windowed();

	// poll the cursors current position... 
	POINT pt;
	::GetCursorPos( &pt );
	::ScreenToClient( m_hWnd, &pt );

	PointInt ptCurrentCursor( pt.x, pt.y );
	if ( ptCurrentCursor != m_CursorPosition )
	{
		PointInt ptDelta( ptCurrentCursor.x - m_CursorPosition.x, ptCurrentCursor.y - m_CursorPosition.y );
		if ( m_CursorEnabled )
		{
			m_CursorPosition.m_X = ptCurrentCursor.x;
			m_CursorPosition.m_Y = ptCurrentCursor.y;
		}
		else
		{
			POINT ptRestore;
			ptRestore.x = m_CursorPosition.x;
			ptRestore.y = m_CursorPosition.y;
			::ClientToScreen( m_hWnd, &ptRestore );
			::SetCursorPos( ptRestore.x, ptRestore.y );
		}

		CommandTarget::postWindowMessage( m_hWnd, HM_MOUSEMOVE, (dword)&ptDelta, (dword)&m_CursorPosition );
	}

	return true;
}

//----------------------------------------------------------------------------

void PlatformWin::setSoftwareCursor( bool bSoftwareCursor )
{
	m_Config.bSoftwareCursor = bSoftwareCursor;
	if (! m_Config.bSoftwareCursor )
		showHardwareCursor( true );
	else
		showHardwareCursor( false );
}

bool PlatformWin::setHardwareCursor( Image::Link pImage, int nFrame )
{
	typedef std::map< u64, HCURSOR >	CursorMap;

	static CursorMap cursorMap;
	u64 nKey = pImage.key() + nFrame;

	CursorMap::iterator iCursor = cursorMap.find( nKey );
	if ( iCursor != cursorMap.end() )
	{
		::SetCursor( iCursor->second );
		return true;
	}

	if (! pImage.valid( true, false ) )
	{
		LOG_ERROR( "PlatformWin", "setHardwareCursor() called with invalid image." );
		return false;
	}

	// create a new HCURSOR from our image and the given frame...
	HDC hDC = ::GetDC( NULL );
	HDC hAndMaskDC = ::CreateCompatibleDC( hDC );
	HDC hXorMaskDC = ::CreateCompatibleDC( hDC );

	SizeInt szImage( pImage->size() );
	HBITMAP hAndMaskBitmap = ::CreateCompatibleBitmap( hDC, szImage.width, szImage.height );
	HBITMAP hXorMaskBitmap = ::CreateCompatibleBitmap( hDC, szImage.width, szImage.height );
	
	// convert the image to RGB8888 if needed...
	if ( pImage->format() != ColorFormat::RGB8888 )
		pImage->setFormat( ColorFormat::RGB8888 );

	HBITMAP hOldAndMaskBitmap	= (HBITMAP)::SelectObject(hAndMaskDC,hAndMaskBitmap);
	HBITMAP hOldXorMaskBitmap	= (HBITMAP)::SelectObject(hXorMaskDC,hXorMaskBitmap);

	for(int x=0;x<szImage.width;++x)
	{
		for(int y=0;y<szImage.height;++y)
		{
			Color c = ((Color *)pImage->frame( nFrame ))[ (y * szImage.width) + x ];
			if ( c.alpha < 128 )
			{
				// pixel is transparent..
				::SetPixel( hAndMaskDC, x, y, RGB(255,255,255) );
				::SetPixel( hXorMaskDC, x, y, RGB(0,0,0) );
			}
			else
			{
				// pixel is normal..
				::SetPixel( hAndMaskDC, x, y, RGB(0,0,0) );
				::SetPixel( hXorMaskDC, x, y, RGB(c.r,c.g,c.b) );
			}
		}
	}

	::SelectObject(hAndMaskDC,hOldAndMaskBitmap);
	::SelectObject(hXorMaskDC,hOldXorMaskBitmap);
	::DeleteDC(hXorMaskDC);
	::DeleteDC(hAndMaskDC);
	::ReleaseDC(NULL,hDC);

	//Create the cursor using the masks and the hotspot values provided
	ICONINFO iconinfo;
	memset( &iconinfo, 0, sizeof(iconinfo) );

	iconinfo.fIcon		= FALSE;
	iconinfo.xHotspot	= szImage.width / 2; //pImage->hotSpot().x;
	iconinfo.yHotspot	= szImage.height / 2; //pImage->hotSpot().y; 
	iconinfo.hbmMask	= hAndMaskBitmap;
	iconinfo.hbmColor	= hXorMaskBitmap;

	HCURSOR hCursor = ::CreateIconIndirect(&iconinfo);
	cursorMap[ nKey ] = hCursor;		// stick into map so we only need to do this once per image frame..
	::SetCursor( hCursor );

	::DeleteObject( hAndMaskBitmap );
	::DeleteObject( hXorMaskBitmap );

	return true;
}

bool PlatformWin::showHardwareCursor( bool bShow )
{
	if ( m_bCursorVisible != bShow )
	{
		m_bCursorVisible = bShow;
		// ShowCursor has an internal count, we need to use a while loop here
		// to make sure the internal count matches what we need to see..
		while( (::ShowCursor( m_bCursorVisible ) >= 0) != m_bCursorVisible );
	}

	return true;
}

void PlatformWin::enableCursor( bool enable )
{
	m_CursorEnabled = enable;
}

void PlatformWin::invertButtons( bool bInverted )
{
	m_bInvertButtons = bInverted;
}

void PlatformWin::setCursorPosition( const PointInt & pos )
{
	m_CursorPosition = pos;
}

//----------------------------------------------------------------------------
// EOF

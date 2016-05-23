/*
	Messages.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef MESSAGES_H
#define MESSAGES_H

#include "Standard/Types.h"



//----------------------------------------------------------------------------

// keyboard modifiers
const u32 HMK_SHIFT					= 0x01;
const u32 HMK_SHIFT_MASK			= HMK_SHIFT << 16;
const u32 HMK_CONTROL				= 0x02;
const u32 HMK_CONTROL_MASK			= HMK_CONTROL << 16;
const u32 HMK_ALT					= 0x04;
const u32 HMK_ALT_MASK				= HMK_ALT << 16;

//----------------------------------------------------------------------------

const u32 HK_LBUTTON				=	0x01;
const u32 HK_RBUTTON				=	0x02;
const u32 HK_CANCEL					=	0x03;
const u32 HK_MBUTTON				=	0x04;    // NOT contiguous with L & RBUTTON
									
const u32 HK_BACK					=	0x08;
const u32 HK_TAB					=	0x09;
									
const u32 HK_CLEAR					=	0x0C;
const u32 HK_RETURN					=	0x0D;
									
const u32 HK_SHIFT					=	0x10;
const u32 HK_CONTROL				=	0x11;
const u32 HK_MENU					=	0x12;
const u32 HK_PAUSE					=	0x13;
const u32 HK_CAPITAL				=	0x14;
									
const u32 HK_KANA					=	0x15;
const u32 HK_HANGEUL				=	0x15;	// old name - should be here for compatibility
const u32 HK_HANGUL					=	0x15;
const u32 HK_JUNJA					=	0x17;
const u32 HK_FINAL					=	0x18;
const u32 HK_HANJA					=	0x19;
const u32 HK_KANJI					=	0x19;
									
const u32 HK_ESCAPE					=	0x1B;
									
const u32 HK_CONVERT				=	0x1C;
const u32 HK_NONCONVERT				=	0x1D;
const u32 HK_ACCEPT					=	0x1E;
const u32 HK_MODECHANGE				=	0x1F;

const u32 HK_SPACE					=	0x20;
const u32 HK_PRIOR					=	0x21;
const u32 HK_NEXT					=	0x22;
const u32 HK_END					=	0x23;
const u32 HK_HOME					=	0x24;
const u32 HK_LEFT					=	0x25;
const u32 HK_UP						=	0x26;
const u32 HK_RIGHT					=	0x27;
const u32 HK_DOWN					=	0x28;
const u32 HK_SELECT					=	0x29;
const u32 HK_PRINT					=	0x2A;
const u32 HK_EXECUTE				=	0x2B;
const u32 HK_SNAPSHOT				=	0x2C;
const u32 HK_INSERT					=	0x2D;
const u32 HK_DELETE					=	0x2E;
const u32 HK_HELP					=	0x2F;

//
// HK_0 thru HK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39)
//

const u32 HK_0						=	0x30;
const u32 HK_1						=	0x31;
const u32 HK_2						=	0x32;
const u32 HK_3						=	0x33;
const u32 HK_4						=	0x34;
const u32 HK_5						=	0x35;
const u32 HK_6						=	0x36;
const u32 HK_7						=	0x37;
const u32 HK_8						=	0x38;
const u32 HK_9						=	0x39;

//
// HK_A thru HK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A)
//

const u32 HK_A						=	0x41;
const u32 HK_B						=	0x42;
const u32 HK_C						=	0x43;
const u32 HK_D						=	0x44;
const u32 HK_E						=	0x45;
const u32 HK_F						=	0x46;
const u32 HK_G						=	0x47;
const u32 HK_H						=	0x48;
const u32 HK_I						=	0x49;
const u32 HK_J						=	0x4A;
const u32 HK_K						=	0x4B;
const u32 HK_L						=	0x4C;
const u32 HK_M						=	0x4D;
const u32 HK_N						=	0x4E;
const u32 HK_O						=	0x4F;
const u32 HK_P						=	0x50;
const u32 HK_Q						=	0x51;
const u32 HK_R						=	0x52;
const u32 HK_S						=	0x53;
const u32 HK_T						=	0x54;
const u32 HK_U						=	0x55;
const u32 HK_V						=	0x56;
const u32 HK_W						=	0x57;
const u32 HK_X						=	0x58;
const u32 HK_Y						=	0x59;
const u32 HK_Z						=	0x5A;
									
const u32 HK_LWIN					=	0x5B;
const u32 HK_RWIN					=	0x5C;
const u32 HK_APPS					=	0x5D;
									
const u32 HK_NUMPAD0				=	0x60;
const u32 HK_NUMPAD1				=	0x61;
const u32 HK_NUMPAD2				=	0x62;
const u32 HK_NUMPAD3				=	0x63;
const u32 HK_NUMPAD4				=	0x64;
const u32 HK_NUMPAD5				=	0x65;
const u32 HK_NUMPAD6				=	0x66;
const u32 HK_NUMPAD7				=	0x67;
const u32 HK_NUMPAD8				=	0x68;
const u32 HK_NUMPAD9				=	0x69;
const u32 HK_MULTIPLY				=	0x6A;
const u32 HK_ADD					=	0x6B;
const u32 HK_SEPARATOR				=	0x6C;
const u32 HK_SUBTRACT				=	0x6D;
const u32 HK_DECIMAL				=	0x6E;
const u32 HK_DIVIDE					=	0x6F;
const u32 HK_F1						=	0x70;
const u32 HK_F2						=	0x71;
const u32 HK_F3						=	0x72;
const u32 HK_F4						=	0x73;
const u32 HK_F5						=	0x74;
const u32 HK_F6						=	0x75;
const u32 HK_F7						=	0x76;
const u32 HK_F8						=	0x77;
const u32 HK_F9						=	0x78;
const u32 HK_F10					=	0x79;
const u32 HK_F11					=	0x7A;
const u32 HK_F12					=	0x7B;
const u32 HK_F13					=	0x7C;
const u32 HK_F14					=	0x7D;
const u32 HK_F15					=	0x7E;
const u32 HK_F16					=	0x7F;
const u32 HK_F17					=	0x80;
const u32 HK_F18					=	0x81;
const u32 HK_F19					=	0x82;
const u32 HK_F20					=	0x83;
const u32 HK_F21					=	0x84;
const u32 HK_F22					=	0x85;
const u32 HK_F23					=	0x86;
const u32 HK_F24					=	0x87;
									
const u32 HK_NUMLOCK				=	0x90;
const u32 HK_SCROLL					=	0x91;

//
//HK_L* & HK_R* - left and right Alt, Ctrl and Shift virtual keys.
//Used only as parameters to GetAsyncKeyState() and GetKeyState().
//No other API or message will distinguish left and right keys in this way.
//

const u32 HK_LSHIFT					=	0xA0;
const u32 HK_RSHIFT					=	0xA1;
const u32 HK_LCONTROL				=	0xA2;
const u32 HK_RCONTROL				=	0xA3;
const u32 HK_LMENU					=	0xA4;
const u32 HK_RMENU					=	0xA5;

const u32 HK_COMMA					=	0xBC;
const u32 HK_PERIOD					=	0xBE;    // . on main keyboard 
const u32 HK_SLASH					=	0xBF;    // / on main keyboard 
									
const u32 HK_PROCESSKEY				=	0xE5;

const u32 HK_ATTN					=	0xF6;
const u32 HK_CRSEL					=	0xF7;
const u32 HK_EXSEL					=	0xF8;
const u32 HK_EREOF					=	0xF9;
const u32 HK_PLAY					=	0xFA;
const u32 HK_ZOOM					=	0xFB;
const u32 HK_NONAME					=	0xFC;
const u32 HK_PA1					=	0xFD;
const u32 HK_OEM_CLEAR				=	0xFE;

// Keyboard messages.
const u32 HM_KEYFIRST				=	0x0100;
const u32 HM_KEYDOWN				=	0x0100;
const u32 HM_KEYUP					=	0x0101;
const u32 HM_CHAR					=	0x0102;
const u32 HM_DEADCHAR				=	0x0103;
const u32 HM_SYSKEYDOWN				=	0x0104;
const u32 HM_SYSKEYUP				=	0x0105;
const u32 HM_SYSCHAR				=	0x0106;
const u32 HM_SYSDEADCHAR			=	0x0107;
const u32 HM_KEYLAST				=	0x0107;

// Mouse Messages.
const u32 HM_MOUSEFIRST				=	0x0200;
const u32 HM_MOUSEMOVE				=	0x0200;
const u32 HM_LBUTTONDOWN			=	0x0201;
const u32 HM_LBUTTONUP				=	0x0202;
const u32 HM_LBUTTONDBLCLK			=	0x0203;
const u32 HM_RBUTTONDOWN			=	0x0204;
const u32 HM_RBUTTONUP				=	0x0205;
const u32 HM_RBUTTONDBLCLK			=	0x0206;
const u32 HM_MBUTTONDOWN			=	0x0207;
const u32 HM_MBUTTONUP				=	0x0208;
const u32 HM_MBUTTONDBLCLK			=	0x0209;
const u32 HM_MOUSEWHEEL				=	0x020A;
const u32 HM_MOUSELAST				=	0x020A;

// User define messages.
const u32 HM_USER					=	0x0400;
const u32 HM_COMMAND				=	0x0401;
const u32 HM_QUIT					=	0x0402;
const u32 HM_MOUSEDELTA				=	0x0403;

//----------------------------------------------------------------------------



#endif

//-------------------------------------------------------------------------------
// EOF


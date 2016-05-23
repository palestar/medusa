/*
	Messages.h

	GUI3D Messages, this is a comprehensive list of all messages that can be generated 
	by the classes contained in the GUI3D project

	(c)2005 Palestar, Richard Lyle
*/

#ifndef GUI3D_MESSAGES_H
#define GUI3D_MESSAGES_H



//----------------------------------------------------------------------------

// InterfaceContext messages
const u32 IC_EXIT					=	0x0500;
const u32 IC_SET_SCENE				=	0x0501;
// WindowButton messages
const u32 WB_OVERBUTTON				=	0x0510;
const u32 WB_BUTTONDOWN				=	0x0511;
const u32 WB_BUTTONUP				=	0x0512;
const u32 WB_MENUDOWN				=	0x051b;
const u32 WB_MENUUP					=	0x051c;
// WindowEdit messages
const u32 WE_BEGINEDIT				=	0x0513;
const u32 WE_EDITDONE				=	0x0514;
// WindowList messages
const u32 WL_SELECT_CHANGE			=	0x0515;
const u32 WL_SELECT					=	0x0516;
// WindowSlider messages
const u32 WS_BEGIN_SLIDE			=	0x0517;
const u32 WS_END_SLIDE				=	0x0518;
// WindowMenu messages
const u32 WM_SELECT_CHANGE			=	0x0519;
const u32 WM_SELECT					=	0x051a;

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF

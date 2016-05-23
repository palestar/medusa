/*
	Cursor.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef CURSOR_H
#define CURSOR_H

//----------------------------------------------------------------------------

enum CursorState
{
	HIDDEN,

	POINTER,				// normal pointer
	MOVE,					// move window pointer
	HARROW,					// horz arrow
	VARROW,					// vert arrow
	EDIT,					// edit pointer, user by WindowEdit
	SELECT,					// select pointer, used by WindowButton and WindowList

	USER1,					// user states
	USER2,
	USER3,
	USER4,
	USER5,
};

//-------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF

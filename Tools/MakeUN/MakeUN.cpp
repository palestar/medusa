/*
	MakeUN.cpp
	Make a Unique Number and paste it to the clipboard
	(c)2005 Palestar, Richard Lyle
*/

#define STANDARD_DLL		// we're statically linking the uid.cpp file
#include "Standard/UniqueNumber.h"

#include <stdio.h>
#include <windows.h>

//----------------------------------------------------------------------------------------

int main(int argc,char **argv)
{
	printf("MakeUN (c)2005 Palestar, Richard Lyle\n");
	printf("Compiled %s, %s\n\n",__DATE__,__TIME__);

	// generate a number
	UniqueNumber number;

	char paste[256];
	sprintf_s( paste, sizeof(paste), "%I64u", (u64)number );

	// allocate memory from the global heap, and copy our string into it
	HGLOBAL paste_global = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,strlen(paste) + 1);
	memcpy(GlobalLock(paste_global),paste,strlen(paste) + 1);
	GlobalUnlock(paste_global);

	if (OpenClipboard(NULL))
	{
		SetClipboardData( CF_TEXT, paste_global );
		CloseClipboard();

		printf("Pasted '%s' to the clipboard.\n\n",paste);
	}
	else
		printf("Failed to paste to the clipboard!\n\n");

	return(0);
}

//----------------------------------------------------------------------------------------
// EOF

// MakeKeys.cpp : Defines the entry point for the console application.
//

#define STANDARD_DLL
#include "Standard/UniqueNumber.h"
#include "Standard/Platform.h"

#include <time.h>
#include <stdio.h>



//----------------------------------------------------------------------------

struct Key
{
	dword		time;
};

//----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	printf("MakeKeys (c)2005 Palestar, Richard Lyle\n");
	printf("Compiled %s, %s\n\n",__DATE__,__TIME__);

	if ( argc < 3 )
	{
		printf("Usage:\nMakeKeys <Count> <Days>\n\n" );
		return -1;
	}

	int count = atoi( argv[1] );
	int days = atoi( argv[2] );
	Key key;
	key.time = time(NULL) + (days * 86400);

	printf("Making %d keys, which will expire %s", count, ctime( (const long *)&key.time ) );

	for(int i=0;i<count;i++)
	{
		UniqueNumber number;
		
		char filename[256];
		sprintf( filename, "%I64u.key", (u64)number );

		FILE * fp = fopen( filename, "wb" );
		if ( fp == NULL )
		{
			printf("Failed to open %s", filename);
			return -1;
		}

		fwrite( &key, sizeof(Key), 1, fp );
		fclose( fp );
	}

	return 0;
}

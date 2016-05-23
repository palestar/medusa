/*
	Main.cpp
	Main for VC2M (Visual C++ to Makefile converter)
	(c)2006 Palestar Inc, Richard Lyle
*/

#include <stdio.h>
#include <stdlib.h>

#include <string>			// STL
#include <list>

//---------------------------------------------------------------------------------------------------

struct Project
{
	std::string		sGUID;			// globally unique ID
	std::string		sName;
	std::string		sProject;
	std::list< std::string >
					Dependecies;

	Project & operator=( const Project & copy )
	{
		sGUID = copy.sGUID;
		sName = copy.sName;
		sProject = copy.sProject;
		Dependecies = copy.Dependecies;
		return *this;
	}
};

//---------------------------------------------------------------------------------------------------

// parse a Visual Studio solution file, enum the projects found inside the solution...
bool ParseSLN( const char * pSLN, std::list< Project > & projects )
{
	FILE * fp = NULL;
	if ( fopen_s( &fp, pSLN, "r" ) != 0 )
		return false;

	char sLine[ 1024 ];
	while( fgets( sLine, sizeof(sLine), fp ) )
	{
		// Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Bin", "Bin\Bin.vcproj", "{F62DE62B-7D7A-4A6D-BA5B-12EB111CB75C}"
		char sGUID1[ 128 ];
		char sGUID2[ 128 ];
		char sName[ 128 ];
		char sProject[ 1024 ];

		int n = sscanf_s( sLine, "Project(\"{%s}\") = \"%s\", \"%s\", \"{%s}\"", sGUID1, sName, sProject, sGUID2 );
		if ( n == 4 )
		{
			Project prj;
			prj.sGUID = sGUID2;
			prj.sName = sName;
			prj.sProject = sProject;

			projects.push_back( prj );
		}
	}

	fclose( fp );
	return true;
}

//---------------------------------------------------------------------------------------------------

int main( int argc, char ** argv )
{
	if ( argc < 2 )
	{
		printf( "VC2M: Visual Studio C++ to Makefile converter...\n\nUsage: VC2M.exe <Solution File>\n" );
		return -1;
	}

	std::list< Project > projects;
	if (! ParseSLN( argv[ 1 ] , projects ) )
	{
		printf( "Failed to parse solution file '%s'!\n", argv[1]);
		return -1;
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------
//EOF

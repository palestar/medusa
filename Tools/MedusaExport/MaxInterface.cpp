/*
	MaxInterface.cpp

	Max Plugin Interface Implementation, see bottom for exported functions which max interfaces through
	(c)2005 Palestar, Richard Lyle
*/

#include "stdafx.h"
#include "ExportDialog.h"
#include "SceneBuilder.h"
#include "Resource.h"

#include "File/Path.h"
#include "File/FileDisk.h"

//----------------------------------------------------------------------------

class MedusaExport : public SceneExport 
{
public:
	// SceneExport Interface
	int ExtCount()
	{ 
		return(1); 
	}

	const TCHAR * Ext(int n)
	{ 
		return(_T("PRT")); 
	}

	const TCHAR * LongDesc()
	{
		return(_T("Resourcer Port")); 
	}

	const TCHAR * ShortDesc()
	{ 
		return(_T("Port")); 
	}

	const TCHAR * AuthorName()
	{ 
		return(_T("Richard Lyle")); 
	}

	const TCHAR * CopyrightMessage()
	{ 
		return(_T("(c)2005 Palestar Inc")); 
	}	

	const TCHAR * OtherMessage1()
	{ 
		return(_T("")); 
	}

	const TCHAR * OtherMessage2()
	{ 
		return(_T("")); 
	}

	unsigned int Version()
	{ 
		return(100); 
	}

	void ShowAbout(HWND hWnd)			
	{}

	int	DoExport(const TCHAR * pName, ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options )
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		CExportDialog dialog;
		if ( suppressPrompts || dialog.DoModal() == IDOK )
		{
			// remove the ports directory from the material/texture directories
			CharString sImages = dialog.m_Images.Right( dialog.m_Images.GetLength() - dialog.m_sPortsDirectory.GetLength() );
			CharString sMaterials = dialog.m_Materials.Right( dialog.m_Materials.GetLength() - dialog.m_sPortsDirectory.GetLength() );

			// enumerate the scene
			SceneBuilder scene( CharString(dialog.m_sPortsDirectory), 
				sImages, sMaterials,
				dialog.m_ExportAnimation != 0, dialog.m_ExportVertexAnimation != 0, 
				dialog.m_OverwriteTextures != 0, dialog.m_bPrefixNames != 0,
				dialog.m_Scale, ei, i );

			if (! scene.m_Scene != NULL )
				return false;		// failed to build the scene!

			// save the scene object
			try {
				OutStream output( new FileDisk( CharString(pName), FileDisk::WRITE ) );
				output << scene.m_Scene;
			}
			catch( File::FileError )
			{
				return false;
			}

			// success!
			return true;
		}

		return( false );
	}
};


//-------------------------------------------------------------------------------

// factory for HydraExport
class MedusaExportClassDesc : public ClassDesc 
{
public:

	// ClassDesc Inteface
	int 			IsPublic() 
	{ 
		return( 1 ); 
	}
	void *			Create(BOOL loading = FALSE) 
	{
		return( new MedusaExport() );
	}
	const TCHAR *	ClassName() 
	{
		return _T("HydraExport"); //GetString(IDS_CLASS_NAME);
	}
	SClass_ID		SuperClassID() 
	{
		return SCENE_EXPORT_CLASS_ID;
	}
	Class_ID		ClassID() 
	{
#ifndef _DEBUG
		return Class_ID(0x6461bb13, 0x7d859e70);
#else
		return Class_ID(0x5ec96772, 0x1a1a532f);
#endif
	}
	const TCHAR* 	Category() 
	{
		return _T("Scene Export"); //GetString(IDS_CATEGORY);
	}

	void			ResetClassParams (BOOL fileReset)
	{}
};

//-------------------------------------------------------------------------------

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR* LibDescription()
{
	return _T("Medusa Export"); //GetString(IDS_LIBDESCRIPTION);
}

// This function returns the number of plug-in classes this DLL
//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
	return 1;
}

// This function returns the number of plug-in classes this DLL
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) 
	{
		case 0: 
			{
				static MedusaExportClassDesc MedusaExportDesc;
				return &MedusaExportDesc;
			}
		default: 
			return 0;
	}
}

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

//-------------------------------------------------------------------------------
// EOF

/**
	@file ImportFBX.h
	@brief TODO

	(c)2009 Palestar Inc
	@author Richard Lyle @date 10/19/2009 1:23:48 PM
*/

#ifndef IMPORTDAE_H
#define IMPORTDAE_H

#include "Tools/ScenePort/Port.h"

#include <map>
#include <string>

namespace fbxsdk_2013_2 {
	// forward declare
	class FbxManager;
	class FbxScene;
	class FbxNode;
};

//---------------------------------------------------------------------------------------------------

class ImportFBX : public ScenePort::Importer
{
public:
	//! Construction
	ImportFBX();
	~ImportFBX();

	//! ScenePort::Importer interface
	bool				import( ScenePort * a_pScene, const char * a_pFileName );

protected:
	//! Data
	CharString			m_sImportFile;			// filename of file being imported
	CharString			m_sBaseDirectory;		// directory that contains the file we are importing

	fbxsdk_2013_2::FbxManager *	
						m_pSDKManager;
    fbxsdk_2013_2::FbxScene *			
						m_pScene;
	
	//! Mutators
	bool				importNodes( ScenePort * a_pScene, 
							BaseNodePort * a_pParent, 
							fbxsdk_2013_2::FbxNode * a_pNode );

	NodePort *			importNode( ScenePort * a_pScene,
							BaseNodePort * a_pParent,
							fbxsdk_2013_2::FbxNode * a_pNode );
	NodePort *			importMesh( ScenePort * a_pScene,
							BaseNodePort * a_pParent,
							fbxsdk_2013_2::FbxNode * a_pNode );
	CharString			importMaterial( ScenePort * a_pScene, 
							fbxsdk_2013_2::FbxNode * a_pNode, int m = 0 );
	CharString			importImage( ScenePort * a_pScene, 
							const char * a_pFile );
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF

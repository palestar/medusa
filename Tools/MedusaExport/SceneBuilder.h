/*
	SceneBuilder.h

	This object builds a matching object from max into our 3d system
	(c)2005 Palestar, Richard Lyle
*/

#ifndef SCENE_BUILDER_H 
#define SCENE_BUILDER_H

#include <math.h>
#include <wchar.h>

//#include "coreexp.h"
//#include "tab.h"
//#include "maxtypes.h"
//#include "inode.h"

#define Color		MaxColor
#define Material	MaxMaterial;

#include "Max.h"
#include "stdmat.h"				// Max standard materials include
#include "impexp.h"				// ExpInterface
#include "bmmlib.h"				// bitmap manageer library

#undef Color
#undef Material

//using namespace MaxSDK;

#include "ProgDlg.h"

#include "Math/BoxHull.h"
#include "Tools/ScenePort/Port.h"

//----------------------------------------------------------------------------

class NodeComplexMeshPort;	// forward declare
class ExpInterface;
class Interface;
//class ScenePort;
//class BoxHull;

class SceneBuilder : public ITreeEnumProc 
{
public:
	// Construction
	SceneBuilder( const char * pPortsDirectory, const char * pImagesPath, const char * pMaterialPath,
		bool captureAnimation, bool captureVertexAnimation, bool overwriteTextures, bool bPrefixNames, 
		float scaleToY, ExpInterface *ei, Interface * i );
	virtual	~SceneBuilder();

	// ITreeEnumProc Interface
	int					callback( INode *node );
	// Data
	ScenePort *			m_Scene;					// the Scene

private:
	CharString			m_sPortsDirectory;			// ports root path
	CharString			m_ImagePath;				// image path
	CharString			m_MaterialPath;
	bool				m_OverwriteTextures;
	bool				m_bPrefixNames;

	bool				m_CaptureAnimation;			// build animation information
	bool				m_CaptureVertexAnimation;	// build vertex animation information
	ExpInterface *		m_ExpInterface;				// max interface objects
	Interface *			m_Interface;

	bool				m_BuildScene;				// are we building the scene
	BoxHull 			m_SceneHull;				// box hull of the entire 3dsmax scene
	float				m_SceneScale;				// calculated from m_ScaleToY and m_SceneHull
	int					m_NodeCount;				// number of nodes
	int					m_StartFrame;				// animation information
	int					m_EndFrame;

	CProgressDlg		m_Progress;

	// Accessors
	bool				findPort( const char * fileName ) const;
	CharString			locateMap( const char * fileName ) const;

	// Mutators
	void				exportNode( BaseNodePort::Ref parent, INode * pNode, bool parity = false );

	int					numMaterials( INode * pNode );
	void				addTriangles( NodeComplexMeshPort * pMesh, INode * pNode, int f, bool parity );
	CharString			buildMaterial( INode * pNode, int materialIndex );

	void				savePort( const char * pFilePath, Port * pPort );

	bool				materialDoubleSided( INode * pNode );
	bool				materialFaceMapped( INode * pNode, int materialIndex );
};

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// EOF


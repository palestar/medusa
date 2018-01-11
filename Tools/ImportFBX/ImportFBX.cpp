/**
	@file ImportFBX.cpp

	(c)2006 Palestar Inc
	@author Richard Lyle @date 10/27/2008 11:16:02 PM
*/

#define WINVER 0xA00

#include "File/Path.h"
#include "File/FileDisk.h"
#include "Debug/Trace.h"

#undef TRACE		// avoid warning with MFC
#undef ASSERT

#include "ImportFBX.h"
#include "Tools/ResourcerDoc/ResourcerDoc.h"
#include "Tools/NodeComplexMeshPort/Port.h"
#include "Tools/ResourcerDoc/Port.h"
#include "Tools/MaterialPort/Port.h"
#include "Tools/ImagePort/Port.h"

#include <map>
#include <vector>

#define FBXSDK_NEW_API

#include "fbxsdk.h"

using namespace fbxsdk_2013_2;

const std::string LOCAL_MARKER("#");

//---------------------------------------------------------------------------------------------------

ImportFBX FBX_IMPORTER;

static bool LoadScene(FbxManager* pSdkManager, FbxScene * pScene, const char* pFilename)
{
    // Create an importer.
    FbxImporter * pImporter = FbxImporter::Create(pSdkManager,"");

    // Initialize the importer by providing a filename.
    bool bImportStatus = pImporter->Initialize( pFilename, -1, pSdkManager->GetIOSettings() );
    if( !bImportStatus )
        return false;
 
    // Import the scene.
    bool bStatus = pImporter->Import( pScene );

    // Destroy the importer.
    pImporter->Destroy();

    return bStatus;
}

static void DumpObjects( FbxObject & object )
{
	FbxClassId classId = object.GetClassId();
	const char * pClassName = classId.GetName();
	LOG_STATUS( "DumpObjects", pClassName );

	int nObjects = object.GetSrcObjectCount();
	for(int j=0;j<nObjects;++j)
	{
		FbxObject * pObject = object.GetSrcObject( j );
		if ( pObject )
			DumpObjects( *pObject );
	}
}

static CharString GetTextureName( FbxProperty & property )
{
	if( property.IsValid() )
	{
		int nLayeredTextures = property.GetSrcObjectCount( FbxLayeredTexture::ClassId );
		for(int i=0;i<nLayeredTextures;++i)
		{
			FbxLayeredTexture *pLayeredTexture = (FbxLayeredTexture*)property.GetSrcObject(FbxLayeredTexture::ClassId, i);
			if ( pLayeredTexture )
			{
				int nLayeredTextures = pLayeredTexture->GetSrcObjectCount(FbxFileTexture::ClassId);
				for(int k =0; k<nLayeredTextures; ++k)
				{
					FbxFileTexture* pTexture = FbxCast <FbxFileTexture> (pLayeredTexture->GetSrcObject(FbxFileTexture::ClassId,k));
					if ( pTexture )
						return pTexture->GetFileName();
				}
			}
		}

		int nTextures = property.GetSrcObjectCount(FbxFileTexture::ClassId);
		for(int k=0;k<nTextures;++k)
		{
			FbxFileTexture* pTexture = FbxCast <FbxFileTexture> (property.GetSrcObject(FbxFileTexture::ClassId,k));
			if ( pTexture )
				return pTexture->GetFileName();
		}
	}

	return "";
}

static Vector3 ConvertVector( const FbxVector4 & v )
{
	return Vector3( -v[0], -v[1], -v[2] );
}

static Color ConvertColor( const FbxColor & c )
{
	return Color( (u8)(255 * Clamp<double>( c[0], 0.0, 1.0)), 
		(u8)(255 * Clamp<double>( c[1], 0.0, 1.0)), 
		(u8)(255 * Clamp<double>( c[2], 0.0, 1.0)) );
}

// convert a FBX matrix into a Matrix33 for orientation and a Vector3 for transform
static void FBXToMedusa( FbxAMatrix &fbx, Matrix33 & f, Vector3 & t )
{
    double *matrix = fbx;
    f.i.x = (float)matrix[0];  f.i.y = (float)matrix[1];  f.i.z = (float)matrix[2];
    f.j.x = (float)matrix[4];  f.j.y = (float)matrix[5];  f.j.z = (float)matrix[6];
    f.k.x = (float)matrix[8];  f.k.y = (float)matrix[9];  f.k.z = (float)matrix[10];
    t.x = (float)matrix[12]; t.y = (float)matrix[13]; t.z = (float)matrix[14];
}

static bool CompareVerts( const NodeComplexMesh2::Vertex & lhs, const NodeComplexMesh2::Vertex & rhs )
{
	return( (lhs.position - rhs.position).magnitude() < EPSILON &&
		fabs( lhs.u - rhs.u) < EPSILON &&
		fabs( lhs.v - rhs.v) < EPSILON );
}

//---------------------------------------------------------------------------------------------------

ImportFBX::ImportFBX() : m_pSDKManager( NULL ), m_pScene( NULL )
{}

ImportFBX::~ImportFBX()
{}

bool ImportFBX::import( ScenePort * a_pScene, const char * a_pFileName )
{
	Path p( a_pFileName );
	if ( stricmp( p.extension(), ".fbx" ) != 0				// Filmbox
		&& stricmp( p.extension(), ".dae" ) != 0			// Collada DAE
		&& stricmp( p.extension(), ".max" ) != 0			// 3D studio Max
		&& stricmp( p.extension(), ".3ds" ) != 0			// 3D studio Max
		&& stricmp( p.extension(), ".dxf" ) != 0			// Autodesk
		&& stricmp( p.extension(), ".obj" ) != 0			// Alias OBJ
	)
	{
		// not a supported extension, early out...
		return false;
	}

	m_sImportFile = a_pFileName;
	m_sBaseDirectory = Path( a_pFileName ).directory();

	CharString sImportPath( CResourcerDoc::document()->currentDirectory() );
	//CharString sMaterialPath( sImportPath + "Materials\\" );
	//CreateDirectory( sMaterialPath, NULL );
	//CharString sTexturesPath( importPath + "Textures\\" );
	//CreateDirectory( sTexturesPath, NULL );

	m_pSDKManager = FbxManager::Create();
	if (! m_pSDKManager )
		return false;

	// Create an IOSettings object.
	FbxIOSettings * ios = FbxIOSettings::Create(m_pSDKManager, IOSROOT );
	m_pSDKManager->SetIOSettings(ios);

	m_pScene = FbxScene::Create(m_pSDKManager,"");
	if (! m_pScene )
		return false;

	bool bImported = false;
	if ( (bImported = LoadScene(m_pSDKManager, m_pScene, a_pFileName)) )
	{
		DumpObjects( *m_pScene );

		// convert coordinate system...
		FbxAxisSystem fbxSceneAxisSystem = m_pScene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem fbxOurAxisSystem( FbxAxisSystem::eDirectX ); //FbxAxisSystem::eYAxis, FbxAxisSystem::eParityEven, FbxAxisSystem::eLeftHanded );
		if(fbxSceneAxisSystem != fbxOurAxisSystem)
			fbxOurAxisSystem.ConvertScene( m_pScene );

		bImported = importNodes( a_pScene, a_pScene->m_Root, m_pScene->GetRootNode() );
	}

	m_pSDKManager->Destroy();
	m_pSDKManager = NULL;
	m_pScene = NULL;

	return bImported;
}

//---------------------------------------------------------------------------------------------------

bool ImportFBX::importNodes( ScenePort * a_pScene, 
						BaseNodePort * a_pParent, 
						FbxNode * a_pNode )
{
	if (! a_pNode || !a_pParent )
		return false;

	NodePort * pNode = NULL;

	FbxNodeAttribute::EType eType = FbxNodeAttribute::eUnknown;
	FbxNodeAttribute * pAttribute = a_pNode->GetNodeAttribute();
	if ( pAttribute )
		eType = pAttribute->GetAttributeType();

    switch ( eType )
    {
    case FbxNodeAttribute::eMesh:      
		pNode = importMesh( a_pScene, a_pParent, a_pNode );
		break;
	default:
		pNode = importNode( a_pScene, a_pParent, a_pNode );
        break;
    }   

	if ( pNode )
	{
		// keep the names the same
		pNode->setName( a_pNode->GetName() );
		// attach imported node 
		a_pParent->m_Children.push( pNode );

		// recurse into any child nodes..
		for( int i=0;i<a_pNode->GetChildCount();++i )
		{
			if (! importNodes( a_pScene, pNode, a_pNode->GetChild(i) ) )
				return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------------------------------

NodePort * ImportFBX::importNode( ScenePort * a_pScene,
						BaseNodePort * a_pParent,
						FbxNode * a_pNode )
{
	NodePort * pOut = new NodePort();

	FbxTime t( 0 );
	FBXToMedusa( a_pNode->EvaluateLocalTransform( t ), pOut->m_Frame, pOut->m_Position );

	return pOut;
}
								
NodePort * ImportFBX::importMesh( ScenePort * a_pScene,
						BaseNodePort * a_pParent,
						FbxNode * a_pNode )
{
	// convert the node to triangles...
	FbxGeometryConverter converter( m_pSDKManager);
	switch( a_pNode->GetNodeAttribute()->GetAttributeType() )
	{
        case FbxNodeAttribute::eMesh:
			if (! converter.TriangulateInPlace( a_pNode ) )
				return NULL;
			break;
	}

    FbxMesh * pMesh = a_pNode->GetMesh();
	if (! pMesh )
		return NULL;

    FbxLayerElementArrayTemplate<int> * pMaterialIndcies = 0;
    pMesh->GetMaterialIndices(&pMaterialIndcies);
    const int nMaterialIndices = pMaterialIndcies ? pMaterialIndcies->GetCount() : 0;
    
    FbxLayerElementArrayTemplate<FbxVector2> *pDiffuseUVs = 0;    
    pMesh->GetTextureUV(&pDiffuseUVs, FbxLayerElement::eTextureDiffuse); 
    
	FbxLayerElementUV::EMappingMode eDiffuseMappingMode = FbxLayerElementUV::eNone;

	const FbxLayer * pLayer = pMesh->GetLayer(0);
	if(pLayer)
	{
		const FbxLayerElementUV * pDiffuseLayer = pLayer->GetUVs(FbxLayerElement::eTextureDiffuse);
		if(pDiffuseLayer)
			eDiffuseMappingMode = pDiffuseLayer->GetMappingMode();
	}

	NodeComplexMeshPort * pOut = new NodeComplexMeshPort();

	// add all the materials first..
	for(int m=0;m<a_pNode->GetMaterialCount();++m)
		pOut->m_Materials.push( importMaterial( a_pScene, a_pNode, m ) );

	// we must have at least one material, create a default one if needed..
	if ( pOut->m_Materials.size() == 0 )
		pOut->m_Materials.push( importMaterial( a_pScene, a_pNode, 0 ) );

	pOut->m_Triangles.allocate( 1 );
	pOut->m_Triangles[0].allocate( pOut->m_Materials.size() );

	FbxVector4 * pVerts = pMesh->GetControlPoints();

	int nFrame = 0;
    int nPolygonCount = pMesh->GetPolygonCount();
    for (int p = 0; p < nPolygonCount; ++p) 
	{
		int nPolygonSize = pMesh->GetPolygonSize( p );
		ASSERT( nPolygonSize == 3 );		// everything should be in triangles

		int nMaterial = pMaterialIndcies->GetAt( p < nMaterialIndices ? p : 0 );
		NodeComplexMesh2::Triangle & triangle = pOut->m_Triangles[nFrame][nMaterial].push();

		// create triangles from the polygons..
        for (int j = 0; j < nPolygonSize; ++j) 
		{
			const int nVertexIndex = pMesh->GetPolygonVertex( p, j );

			Vertex vertex;
			vertex.position = ConvertVector( pVerts[ nVertexIndex ] );
			FbxVector4 normal;
			pMesh->GetPolygonVertexNormal(p, j, normal);
			vertex.normal = ConvertVector( normal );
		
			vertex.u = vertex.v = 0.0f;

			if(pDiffuseUVs)
			{
				FbxVector2 fbxDiffuseUV(0,0);
				if(eDiffuseMappingMode == FbxLayerElementUV::eByPolygonVertex)
					fbxDiffuseUV = pDiffuseUVs->GetAt(pMesh->GetTextureUVIndex(p, j));
				else
					fbxDiffuseUV = pDiffuseUVs->GetAt( nVertexIndex );
				vertex.u = (float)fbxDiffuseUV[0];
				vertex.v = 1.0f - fmod( (float)fbxDiffuseUV[1], 1.0f );
			}

			// search for a vertex that we already have..
			int nVertexFound = -1;
			for(int k=0;nVertexFound < 0 && k<pOut->m_Verts.size();++k)
				if ( CompareVerts( pOut->m_Verts[k], vertex ) )
					nVertexFound = k;

			if ( nVertexFound < 0 )
			{
				// no vertex found, add new vertex for mesh..
				nVertexFound = pOut->m_Verts.size();
				pOut->m_Verts.push( vertex );
			}

			triangle.verts[j] = nVertexFound;
        }
    }

	// default to optimize the mesh...
	pOut->m_Optimize = true;
	pOut->m_CalcNormals = true;
	pOut->m_Reverse = true;				// quick fix just to get FBX importing working..

	// get the frame & position, then transform our verts..
	FbxTime t( 0 );
	FBXToMedusa( a_pNode->EvaluateLocalTransform( t ), pOut->m_Frame, pOut->m_Position );
	pOut->transformVerts();

	return pOut;
}

CharString ImportFBX::importMaterial( ScenePort * a_pScene, FbxNode * a_pNode, int m /*= 0*/ )
{
	if (! a_pNode )
		return "";

	CharString sImportPath( CResourcerDoc::document()->currentDirectory() );
	const char * pMaterialName = a_pNode->GetName();

	FbxSurfaceMaterial * pMaterial = FbxCast<FbxSurfaceMaterial>( a_pNode->GetSrcObject(FbxSurfaceMaterial::ClassId, m) );
	if ( pMaterial )
		pMaterialName = pMaterial->GetName();

	CharString sMaterialPort( sImportPath + "m_" + pMaterialName + ".prt" );
	if (! FileDisk::fileExists( sMaterialPort ) )
	{
		// create material 
		MaterialPort::Ref pMaterialPort = new MaterialPort();

		if ( pMaterial != NULL )
		{
			FbxSurfaceLambert * pLambert = FbxCast<FbxSurfaceLambert>( pMaterial );
			if ( pLambert != NULL )
			{
				pMaterialPort->m_Ambient = ConvertColor( pLambert->Ambient.Get() );
				pMaterialPort->m_Diffuse = ConvertColor( pLambert->Diffuse.Get() );
				pMaterialPort->m_Emissive = ConvertColor( pLambert->Emissive.Get() );
			}

			DumpObjects( *pMaterial );

			FbxProperty diffuse = pMaterial->FindProperty( FbxSurfaceMaterial::sDiffuse );
			if ( diffuse.IsValid() )
			{
				pMaterialPort->m_Textures.push( MaterialPort::Texture( PrimitiveSurface::DIFFUSE, 
					importImage( a_pScene, GetTextureName( diffuse ) ), true ) );
			}

			FbxProperty emissive = pMaterial->FindProperty( FbxSurfaceMaterial::sEmissive );
			if ( emissive.IsValid() )
			{
				pMaterialPort->m_Textures.push( MaterialPort::Texture( PrimitiveSurface::LIGHTMAP, 
					importImage( a_pScene, GetTextureName( emissive ) ), true ) );
			}

			FbxProperty bump = pMaterial->FindProperty( FbxSurfaceMaterial::sBump );
			if ( bump.IsValid() )
			{
				pMaterialPort->m_Textures.push( MaterialPort::Texture( PrimitiveSurface::BUMPMAP, 
					importImage( a_pScene, GetTextureName( bump ) ), true ) );
			}
		}

		// save the material to disk
		bool bSaved = false;
		try {
			OutStream( new FileDisk( sMaterialPort, FileDisk::READ_WRITE ) ) << pMaterialPort;
			bSaved = true;
		}
		catch( File::FileError )
		{}
	}

	return CResourcerDoc::document()->relative( sMaterialPort );
}

static bool FindFileRecursively( const CharString & a_sPath, const CharString & a_sMask, CharString & a_sFile )
{
	CharString sPath( a_sPath );
	if (! sPath.endsWith( PATH_SEPERATOR ) )
		sPath += PATH_SEPERATOR;

	// search for our specific file in this directory..
	FindFile ff( sPath + a_sMask, false, true );
	if ( ff.fileCount() > 0 )
	{
		a_sFile = ff.file(0);
		return true;
	}

	// recurse into any subdirectories as well..
	ff.findFiles( sPath + "*.*" );
	for(int i=0;i<ff.directoryCount();++i)
	{
		if ( ff.directory(i)[0] == '.' )
			continue;
		if ( FindFileRecursively( sPath + ff.directory(i), a_sMask, a_sFile ) )
			return true;
	}

	return false;
}

CharString ImportFBX::importImage( ScenePort * a_pScene, const char * a_pFile )
{
	if (! a_pFile || a_pFile[0] == 0 )
		return "";

	CharString sImportPath( CResourcerDoc::document()->currentDirectory() );
	CharString sImagePort( sImportPath + "i_" + Path( a_pFile ).name() + ".prt" );

	if (! FileDisk::fileExists( sImagePort ) )
	{
		CharString sImportFile( a_pFile );

		ImagePort::Ref pImagePort = new ImagePort();
		if (! FileDisk::fileExists( sImportFile ) )
		{
			// get only the file part..
			sImportFile = Path( sImportFile ).file();

			// image file not found, try to find it by searching each directory recusrively
			// from the base path..
			CharString sFindPath( m_sBaseDirectory );
			sFindPath.replace( '/','\\' );

			CharString sFoundFile;
			if (! FindFileRecursively( sFindPath, sImportFile, sFoundFile ) )
				return "";		// failed to find the file..

			sImportFile = sFoundFile;

			//int nLastSlash = sFindPath.reverseFind( '\\' );
			//while( nLastSlash >= 0 )
			//{
			//	sFindPath = sFindPath.left( nLastSlash );
			//	nLastSlash = sFindPath.reverseFind( '\\' );

			//	CharString sFoundFile;
			//	if ( FindFileRecursively( sFindPath, sImportFile, sFoundFile ) )
			//	{
			//		sImportFile = sFoundFile;
			//		break;
			//	}
			//}

			//if ( nLastSlash < 0 )
			//	return "";		// failed to find the texture!
		}

		if (! pImagePort->importFile( sImportFile ) )
			a_pScene->report( CharString().format( "Failed to import image (%s)!", sImportFile ) );

		bool bSaved = false;
		try {
			OutStream( new FileDisk( sImagePort, FileDisk::READ_WRITE ) ) << pImagePort;
			bSaved = true;
		}
		catch( File::FileError )
		{}
	}

	return CResourcerDoc::document()->relative( sImagePort );
}

//---------------------------------------------------------------------------------------------------
//EOF

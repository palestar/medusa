/*
	SceneBuilder.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "stdafx.h"
#include "SceneBuilder.h" 

#include "File/Path.h"
#include "Standard/StringHash.h"
#include "File/FileDisk.h"
#include "Tools/NodeComplexMeshPort/Port.h"
#include "Tools/ImagePort/Port.h"
#include "Tools/MaterialPort/Port.h"

//----------------------------------------------------------------------------

SceneBuilder::SceneBuilder( const char * pPortsDirectory, const char * pImagesPath, const char * pMaterialPath,
						   bool captureAnimation, bool captureVertexAnimation, bool overwriteTextures, bool bPrefixNames, 
						   float scaleToY, ExpInterface *ei, Interface * i ) : m_Scene( NULL )
{
	m_Progress.Create();

	try {
		m_sPortsDirectory = pPortsDirectory;
		m_ImagePath = pImagesPath;
		m_MaterialPath = pMaterialPath;
		m_OverwriteTextures = overwriteTextures;
		m_bPrefixNames = bPrefixNames;

		m_CaptureAnimation = captureAnimation;
		m_CaptureVertexAnimation = captureVertexAnimation;
		m_ExpInterface = ei;
		m_Interface = i;

		m_NodeCount = 0;
		m_SceneHull.resetBox();
		m_SceneScale = 1.0f;

		// get the animation range
		Interval interval = m_Interface->GetAnimRange();
		m_StartFrame = interval.Start() / GetTicksPerFrame();
		m_EndFrame = interval.End() / GetTicksPerFrame();

		// create the root object
		m_Scene = new ScenePort;
		m_Scene->m_Fps = GetFrameRate();

		// first pass, count the nodes and determine the extents of the scene
		m_BuildScene = false;
		m_ExpInterface->theScene->EnumTree( this );

		// calculate the scene scale based on the desired height (m_ScaleToY) 
		// and the bounding box of the entire scene (m_SceneHull)
		float sceneHeight = fabs(m_SceneHull.min.y - m_SceneHull.max.y);
		if ( sceneHeight > 0.0f )
		{
			m_SceneScale = scaleToY / sceneHeight;
			if ( m_SceneScale > 0.0f )
			{
				NodePort * newRoot = new NodePort;
				newRoot->m_Frame = Matrix33( Vector3( m_SceneScale, 0, 0 ),
					Vector3( 0, m_SceneScale, 0 ),
					Vector3( 0, 0, m_SceneScale ) );

				m_Scene->m_Root = newRoot;
			}
		}

		//TRACE("sceneHeight = %f, scaleToY = %f, sceneScale = %f\n", sceneHeight, scaleToY, m_SceneScale );

		// setup the progress dialog
		m_Progress.SetRange(0,m_NodeCount);
		m_Progress.SetStep(1);

		// enumerate the scene through this object
		m_BuildScene = true;
		m_ExpInterface->theScene->EnumTree( this );

	}
	catch(...)
	{
		m_Scene = NULL;
	}
}

SceneBuilder::~SceneBuilder()
{}

//----------------------------------------------------------------------------

int SceneBuilder::callback( INode * pNode )
{
	if ( ! m_BuildScene )
	{
		if ( ! pNode->IsHidden() )
		{
			TimeValue t = m_StartFrame * GetTicksPerFrame();

			Object * pObject = pNode->EvalWorldState(t).obj;
			if ( pObject->CanConvertToType(triObjectClassID))
			{
				// get the triangle object
				TriObject * pTriObject = (TriObject *)pObject->ConvertToType(t, triObjectClassID);

				// update the bounding box
				pTriObject->mesh.buildBoundingBox();
				
				// get the transformed bounding box
				Box3 nodeBox = pTriObject->mesh.getBoundingBox( &pNode->GetObjectTM(t) );
				m_SceneHull.addVertex( Vector3( nodeBox.pmin.x, nodeBox.pmin.z, nodeBox.pmin.y ) );
				m_SceneHull.addVertex( Vector3( nodeBox.pmax.x, nodeBox.pmax.z, nodeBox.pmax.y ) );

				if ( pTriObject != pObject)
					pTriObject->DeleteThis();
			}
		}

		m_NodeCount++;
		return TREE_CONTINUE ;
	}

	exportNode( m_Scene->m_Root, pNode );

	return TREE_IGNORECHILDREN;
}

//----------------------------------------------------------------------------

CharString SceneBuilder::locateMap( const char * fileName ) const
{
	if (! FileDisk::fileExists( fileName ) )
	{
		Path	mapPath( fileName );

		for(int i=0;i<TheManager->GetMapDirCount();i++)
		{
			CharString newPath( CharString( TheManager->GetMapDir(i) ) + "\\" + mapPath.file() );
			if ( FileDisk::fileExists( newPath ) )
				return( newPath );		// found the map
		}

		// failed to find the map
		return( mapPath.file() );
	}

	return( fileName );
}

bool SceneBuilder::findPort( const char * fileName ) const
{
	if ( FileDisk::fileExists( m_sPortsDirectory + fileName ) )
		return( true );

	return( false );
}

void SceneBuilder::exportNode( BaseNodePort::Ref parent, INode * pNode, bool parity /*= false*/ )
{
	m_Progress.StepIt();

	// get the node position and orientation
	TimeValue t = m_StartFrame * GetTicksPerFrame();
	INode *	pParentNode = pNode->GetParentNode();
	Matrix3 localFrame = pNode->GetNodeTM(t) * Inverse(pParentNode->GetNodeTM(t));

	// check for a parity change on this node
	if ( localFrame.Parity() )
		parity = !parity;

	// determine what type of object we have here
	Object *pObject = pNode->EvalWorldState(t).obj;
	if ( pObject->CanConvertToType(triObjectClassID))
	{
		// create the node
		NodeComplexMeshPort * pPort = new NodeComplexMeshPort;
		pPort->setName( CharString( pNode->GetName() ) );

		// set the node position and rotation
		pPort->m_Position = Vector3( localFrame.GetRow(3).x,localFrame.GetRow(3).z,localFrame.GetRow(3).y );
		pPort->m_Frame = Matrix33( Vector3( localFrame.GetRow(0).x,localFrame.GetRow(0).z,localFrame.GetRow(0).y ),
			Vector3( localFrame.GetRow(2).x,localFrame.GetRow(2).z,localFrame.GetRow(2).y ),
			Vector3( localFrame.GetRow(1).x,localFrame.GetRow(1).z,localFrame.GetRow(1).y ) );

		// add all the materials to the mesh
		for(int j=0;j<numMaterials( pNode );j++)
			pPort->m_Materials.push( buildMaterial( pNode, j ) );

		if ( m_CaptureAnimation )
		{
			pPort->m_NodeFrames.allocate( (m_EndFrame - m_StartFrame) + 1 );
			for(int f=m_StartFrame;f<=m_EndFrame;f++)
			{
				NodeComplexMeshPort::Frame & frame = pPort->m_NodeFrames[ f - m_StartFrame ];

				// get the current time
				TimeValue t = f * GetTicksPerFrame();

				INode * pParentNode = pNode->GetParentNode();
				Matrix3 localFrame = pNode->GetNodeTM(t) * Inverse(pParentNode->GetNodeTM(t));

				frame.position = Vector3( localFrame.GetRow(3).x,localFrame.GetRow(3).z,localFrame.GetRow(3).y );
				frame.frame = Matrix33( Vector3( localFrame.GetRow(0).x,localFrame.GetRow(0).z,localFrame.GetRow(0).y ),
					Vector3( localFrame.GetRow(2).x,localFrame.GetRow(2).z,localFrame.GetRow(2).y ),
					Vector3( localFrame.GetRow(1).x,localFrame.GetRow(1).z,localFrame.GetRow(1).y ) );
			}
		}

		if ( m_CaptureVertexAnimation )
		{
			int frames = (m_EndFrame - m_StartFrame) + 1;
			pPort->m_Triangles.allocate( frames );
			for(int f=m_StartFrame;f<=m_EndFrame;f++)
			{
				pPort->m_Triangles[f].allocate( pPort->m_Materials.size() );
				addTriangles( pPort, pNode, f, parity );
				pPort->cullVerticies();
			}
		}
		else
		{
			pPort->m_Triangles.allocate( 1 );
			pPort->m_Triangles[0].allocate( pPort->m_Materials.size() );
			addTriangles( pPort, pNode, m_StartFrame, parity );
		}
		
		//pPort->cullVerticies();

		parent->addChild( pPort );
		parent = pPort;
	}

	// recurse into the child nodes
	if (pNode->NumberOfChildren() > 0)
	{
		for(int c=0;c<pNode->NumberOfChildren();c++)
			exportNode( parent, pNode->GetChildNode(c), parity );
	}
}

void SceneBuilder::addTriangles( NodeComplexMeshPort * pPort, INode * pNode, int f, bool parity )
{
	ASSERT( f >= m_StartFrame && f <= m_EndFrame );

	TimeValue	t = f * GetTicksPerFrame();
	int			f2 = f - m_StartFrame;

	// determine what type of object we have here
	Object *pObject = pNode->EvalWorldState(t).obj;
	if ( pObject->CanConvertToType(triObjectClassID))
	{
		// get the triangle object
		TriObject * pTriObject = (TriObject *)pObject->ConvertToType(t, triObjectClassID);
		// Get the mesh object
		Mesh * pMesh = &pTriObject->mesh;
		// make sure the normals are built
		pMesh->buildNormals();

		// get the verticies offset
		Matrix3 meshFrame(1);
		meshFrame.PreTranslate(pNode->GetObjOffsetPos());
		PreRotateMatrix(meshFrame, pNode->GetObjOffsetRot());
		ApplyScaling(meshFrame, pNode->GetObjOffsetScale());

		// add the triangles to the mesh port
		for(int i=0;i<pMesh->numFaces;i++)
		{
			Face * pFace = pMesh->faces + i;

			int m = pFace->getMatID();
			if ( m >= pPort->m_Materials.size() )
				m = 0;

			NodeComplexMesh2::Triangle & newTriangle = pPort->m_Triangles[f2][m].push();
			if ( materialFaceMapped( pNode, m ) )
			{
				static Point3 basic_tva[3] = 
				{
					Point3(0.0,1.0,0.0),Point3(0.0,0.0,0.0),Point3(1.0,0.0,0.0) 
				};
				static Point3 basic_tvb[3] = 
				{
					Point3(1.0,0.0,0.0),Point3(1.0,1.0,0.0),Point3(0.0,1.0,0.0)
				};
				static int nextpt[3] = { 1,2,0 };
				static int prevpt[3] = { 2,0,1 };

				int nhid = 2;
				if ( !(pFace->flags & EDGE_A))
					nhid = 0;
				else if ( !(pFace->flags & EDGE_B))
					nhid = 1;
				else if ( !(pFace->flags & EDGE_C))
					nhid = 2;
				int na = 2 - nhid;

				Point3 * basetv = (pFace->v[prevpt[nhid]]<pFace->v[nhid]) ? basic_tva : basic_tvb;

				for(int i=0;i<3;i++)
				{
					newTriangle.verts[ i ] = pPort->m_Verts.size();
					NodeComplexMesh2::Vertex & newVertex = pPort->m_Verts.push();

					int vi = pFace->v[i];
					Point3 position = meshFrame * pMesh->verts[ vi ];
					Point3 normal = pMesh->getNormal( vi );

					newVertex.position = Vector3( position.x, position.z, position.y );
					newVertex.normal = Vector3( normal.x, normal.z, normal.y ).normalize();
					newVertex.u = basetv[ na ].x;
					newVertex.v = basetv[ na ].y;
					na = nextpt[ na ];
				}
			}
			else
			{
				TVFace * pTVFace = pMesh->tvFace ? pMesh->tvFace + i : NULL;
				for(int v=0;v<3;v++)
				{
					newTriangle.verts[ v ] = pPort->m_Verts.size();
					NodeComplexMesh2::Vertex & newVertex = pPort->m_Verts.push();

					int vi = pFace->v[v];
					Point3 position = meshFrame * pMesh->verts[ vi ];
					Point3 normal = pMesh->getNormal( vi );

					newVertex.position = Vector3( position.x, position.z, position.y );
					newVertex.normal = Vector3( normal.x, normal.z, normal.y ).normalize();
					newVertex.u = pTVFace ? pMesh->tVerts[ pTVFace->t[v] ].x : 0.0f;
					newVertex.v = pTVFace ? 1.0f - pMesh->tVerts[ pTVFace->t[v] ].y : 0.0f;
				}
			}
			if ( !parity )
			{
				int swap = newTriangle.verts[ 0 ] ;
				newTriangle.verts[ 0 ] = newTriangle.verts[ 2 ];
				newTriangle.verts[ 2 ] = swap;
			}
		}

		// release the triangle object
		if ( pTriObject != pObject)
			pTriObject->DeleteThis();
	}

}

int SceneBuilder::numMaterials( INode * pNode )
{
	Mtl * pMaterial = pNode->GetMtl();

	if (! pMaterial )
		return 1;
	if (! pMaterial->IsMultiMtl() )
		return 1;

	return( pMaterial->NumSubMtls() );
}

CharString SceneBuilder::buildMaterial( INode * pNode, int materialIndex )
{
	// setup the material
	CharString materialFile = "";

	Mtl * pMaterial = pNode->GetMtl();
	if ( pMaterial && pMaterial->IsMultiMtl() )
		pMaterial = pMaterial->GetSubMtl( materialIndex );

	if ( pMaterial != NULL )
	{
		CharString materialName( pMaterial->GetName().data() );
		if ( materialName.length() > 0 )
			materialName.format( m_bPrefixNames ? "m_%s" : "%s", materialName );
		else
			materialName.format( m_bPrefixNames ? "m_%s%d" : "%s%d", CharString( pNode->GetName() ), materialIndex );

		materialFile = m_MaterialPath + materialName + ".prt";
		if (! findPort( materialFile ) || m_OverwriteTextures )
		{
			// setup the material properties, translate as best we can from 3dsmax
			MaterialPort::Ref newMaterial = new MaterialPort;

			// physical properties
			newMaterial->m_Friction = pMaterial->GetDynamicsProperty(0, 0, DYN_STATIC_FRICTION );
			// visible properties
			newMaterial->m_Diffuse = (DWORD)pMaterial->GetDiffuse();
			newMaterial->m_Diffuse.m_A = 255 - (pMaterial->GetXParency() * 255);
			newMaterial->m_Ambient = (DWORD)pMaterial->GetAmbient();
			newMaterial->m_Emissive = newMaterial->m_Diffuse * pMaterial->GetSelfIllum();
			newMaterial->m_Specular = (DWORD)pMaterial->GetSpecular();
			newMaterial->m_SpecularPower = pMaterial->GetShinStr();

			//----------------------------------------------------------------------------

			Texmap *pTexture = pMaterial->GetSubTexmap( ID_DI );		// Diffuse
			if (pTexture && pTexture->ClassID() == Class_ID(BMTEX_CLASS_ID,0) )
			{
				BitmapTex *pBMtexture = (BitmapTex *)pTexture;

				CharString imageName;
				imageName.format( m_bPrefixNames ? "d_%s" : "%s", Path( pBMtexture->GetMapName() ).name() );

				CharString imagePort( m_ImagePath + imageName + ".PRT" );
				if ( !findPort( imagePort ) || m_OverwriteTextures )
				{
					// create a new image
					ImagePort::Ref newImage = new ImagePort;

					CharString mapName( locateMap( CharString( pBMtexture->GetMapName() ) ) );
					if (! newImage->importFile( mapName ) )
						MessageBox(NULL,"Failed to import image", CharString( mapName ), MB_OK );

					savePort( imagePort, newImage );
				}

				// set the diffuse image
				newMaterial->m_DiffusePort = imagePort;
			}

			//-------------------------------------------------------------------------------

			pTexture = pMaterial->GetSubTexmap( ID_OP );		// Opacity map
			if (pTexture && pTexture->ClassID() == Class_ID(BMTEX_CLASS_ID,0) )
			{
				BitmapTex *pBMtexture = (BitmapTex *)pTexture;

				CharString imageName;
				imageName.format( m_bPrefixNames ? "a_%s" : "%s", Path( pBMtexture->GetMapName() ).name() );

				CharString imagePort( m_ImagePath + imageName + ".PRT" );
				if ( !findPort( imagePort ) || m_OverwriteTextures ) 
				{
					// create a new image
					ImagePort::Ref newImage = new ImagePort;

					CharString mapName( locateMap( CharString( pBMtexture->GetMapName() ) ) );
					if (! newImage->importFile( mapName ) )
						MessageBox(NULL,"Failed to import image", CharString(mapName), MB_OK );

					savePort( imagePort, newImage );
				}

				// set the diffuse image
				newMaterial->m_Blending = PrimitiveMaterial::ALPHA;
				newMaterial->m_AlphaPort = imagePort;
			}

			// save the material to disk
			savePort( materialFile, newMaterial );
		}
	}
	else
	{
		CharString materialName( pNode->GetName() );
		if ( materialName.length() > 0 )
			materialName.format( m_bPrefixNames ? "m_%s" : "%s", materialName );
		else
			materialName.format( m_bPrefixNames ? "m_%s%d" : "%s%d", pNode->GetName(), materialIndex );

		materialFile = m_MaterialPath + materialName + ".PRT";
		if (! findPort( materialFile ) || m_OverwriteTextures )
		{
			// create material 
			MaterialPort::Ref newMaterial = new MaterialPort;
			//nodeMaterial->setName( pNode->GetName() );
			newMaterial->m_Ambient = newMaterial->m_Diffuse = pNode->GetWireColor();
			// save the material to disk
			savePort( materialFile, newMaterial );
		}
	}

	return materialFile;
}

void SceneBuilder::savePort( const char * pFilePath, Port * pPort )
{
	CharString sLocalFile = CharString(m_sPortsDirectory) + pFilePath;

	try {
		OutStream output( new FileDisk( sLocalFile, FileDisk::WRITE ) );
		output << pPort;
	}
	catch( File::FileError )
	{
		MessageBox(NULL,"Failed to export file!", CharString( sLocalFile ), MB_OK );
	}
}

bool SceneBuilder::materialDoubleSided( INode * pNode )
{
	return( (pNode->GetMtl()->Requirements( -1 ) & MTLREQ_2SIDE) != 0 );
}

bool SceneBuilder::materialFaceMapped( INode * pNode, int materialIndex )
{
	Mtl * pMaterial = pNode->GetMtl();
	if ( pMaterial && pMaterial->IsMultiMtl() )
		pMaterial = pMaterial->GetSubMtl( materialIndex );

	return( pMaterial != NULL ? (pMaterial->Requirements( -1 ) & MTLREQ_FACEMAP) != 0 : false );
}


//----------------------------------------------------------------------------
// EOF


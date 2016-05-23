/*
	Port.cpp
	(c)2005 Palestar, Richard Lyle
*/

#define NODECOMPLEXMESHPORT_DLL
#include "stdafx.h"
#include "Port.h"

#include "File/Path.h"
#include "File/FileDisk.h"

#include "Tools/ResourcerDoc/ResourcerDoc.h"
#include "Tools/ResourcerDoc/Port.h"
#include "Tools/MaterialPort/Port.h"
#include "Tools/ImagePort/Port.h"

//----------------------------------------------------------------------------

#pragma pack(push, PORT_IMPORT_CPP )
#pragma pack(1)

struct LWO_FORM
{
	dword		form;
	dword		size;
	dword		tag;
};

struct LWO_CHUNK
{
	dword		tag;
	dword		size;
};

struct LWO_SUBCHUNK
{
	dword		tag;
	word		size;
};

struct LWO_POINT
{
	float		x;
	float		y;
	float		z;
};

#pragma pack( pop, PORT_IMPORT_CPP )

//----------------------------------------------------------------------------

template<class T>
T swapEndian( T swap )
{
	T swapped;
	byte * pSrc = (byte *)&swap;
	byte * pDst = (byte *)&swapped;

	for(int i=0;i<sizeof(T);i++)
		pDst[sizeof(T) - (i + 1)] = pSrc[ i ];

	return( swapped );
}

template<class T>
T wordAlign( T value )
{
	if ( value & 0x1 )
		return value + 1;
	return value;
}

//----------------------------------------------------------------------------

// import LightWave object
//

bool NodeComplexMeshPort::importLWO( const char * fileName )
{
	byte * pLWOFile = NULL;
	LWO_FORM * pForm = NULL;
	LWO_FORM * pLastForm = NULL;

	try {
		FileDisk file( fileName );

		pLWOFile = new byte[ file.size() ];
		
		pForm = (LWO_FORM *)pLWOFile;
		pLastForm = (LWO_FORM *)(pLWOFile + file.size());

		file.read( pLWOFile, file.size() );
	}
	catch( File::FileError )
	{
		delete [] pLWOFile;
		return false;
	}

	// parse the file forms
	while( pForm < pLastForm )
	{
		if ( swapEndian( pForm->form ) != 'FORM' )
			return false;
		dword formSize = wordAlign( swapEndian( pForm->size ) );
		
		switch( swapEndian( pForm->tag ) )
		{
		case 'LWOB':
			decodeLWOB( fileName, (byte *)(pForm + 1), ((byte *)pForm) + formSize );
			break;
		case 'LWO2':
			decodeLWO2( fileName, (byte *)(pForm + 1), ((byte *)pForm) + formSize );
			break;
		default:
			MessageBox( NULL, CharString().format("Unknown LWO form 0x%x", pForm->tag), fileName, MB_OK );
			break;
		} // end switch( swapEndian( pForm->tag ) )

		pForm = (LWO_FORM *)(((byte *)pForm) + formSize + 8);
	} // end while( pForm < pLastForm )

	
	/*
	// Calculate the vertex normals
	//
	Array< Array< Vector3 > > normals;
	normals.allocate( m_Triangles.size() );

	for(int i=0;i<m_Triangles.size();i++)
	{
		Array< Triangle > & triangles = m_Triangles[ i ];

		normals[i].allocate( triangles.size() );
		for(int j=0;j<triangles.size();j++)
		{
			Triangle & triangle = triangles[j];

			// calculate the normal for this triangle
			Vector3 normal( ((m_Verts[ triangle.verts[0] ].position - m_Verts[ triangle.verts[1] ].position) & 
								(m_Verts[ triangle.verts[0] ].position - m_Verts[ triangle.verts[2] ].position)) );
			normal.normalize();

			// store the normal
			normals[i][j] = normal;
		}
	}

	for(i=0;i<m_Verts.size();i++)
	{
		Vector3 normal( 0.0f );

		// count the number of triangles using this vertex
		int	count = 0;
		for(int j=0;j<m_Triangles.size();j++)
			for(int k=0;k<m_Triangles[j].size();k++)
			{
				Triangle & triangle = m_Triangles[j][k];
				if ( triangle.verts[0] == i ||
					triangle.verts[1] == i ||
					triangle.verts[2] == i )
				{
					normal += normals[j][k];
					count++;
				}
			}

		if ( count > 0 )
			m_Verts[i].normal = normal / float(count);
	}
	*/

	delete [] pLWOFile;
	return true;
}

//----------------------------------------------------------------------------

enum TextureAxis
{
	NOAXIS = -1,

	XAXIS,
	YAXIS,
	ZAXIS,
};

struct LightWaveSurface
{
	CharString		name;
	float				smoothingAngle;
	Vector3		center;
	Vector3		scale;
	TextureAxis			axis;
	bool				doubleSided;
	bool				clampU, clampV;

	LightWaveSurface & operator=(const LightWaveSurface & copy )
	{
		name = copy.name;
		smoothingAngle = copy.smoothingAngle;
		center = copy.center;
		scale = copy.scale;
		axis = copy.axis;
		doubleSided = copy.doubleSided;
		clampU = copy.clampU;
		clampV = copy.clampV;

		return *this;
	}
};


//----------------------------------------------------------------------------

void NodeComplexMeshPort::decodeLWOB( const char * fileName, byte * begin, byte * end )
{
	CharString importPath( CResourcerDoc::document()->currentDirectory() );

	m_Triangles.allocate( 1 );

	// create the image and material directories
	CharString materialPath( importPath + "Materials\\" );
	CreateDirectory( materialPath, NULL );
	CharString imagePath( importPath + "Images\\" );
	CreateDirectory( imagePath, NULL );

	// surface information from LightWave
	Array< LightWaveSurface > lwSurfaces;
	
	LWO_CHUNK * pChunk = (LWO_CHUNK *)begin;
	LWO_CHUNK * pLastChunk = (LWO_CHUNK *)end;
	
	// parse the chunks
	while( pChunk < pLastChunk )
	{
		dword chunkSize = wordAlign( swapEndian( pChunk->size ) );

		switch( swapEndian( pChunk->tag ) )
		{
		case 'PNTS':	// points
			{
				LWO_POINT * pPoint = (LWO_POINT *)(pChunk + 1);
				int pointCount = chunkSize / 12;

				// allocate the verticies
				m_Verts.allocate( pointCount );
				for(int i=0;i<pointCount;i++)
				{
					Vertex & newVertex = m_Verts[i];
					newVertex.position.x = swapEndian( pPoint->x );
					newVertex.position.y = swapEndian( pPoint->y );
					newVertex.position.z = swapEndian( pPoint->z );
					//newVertex.diffuse = Color(255,255,255,255);
					newVertex.normal = Vector3(0,1,0);		// calculate the vertex normal later
					//newVertex.u = newVertex.v = 0.0f;

					pPoint++;
				}

				TRACE( CharString().format("NodeComplexMeshPort::importLWO(), %d points", pointCount) );
			}
			break;
		case 'SRFS':	// surfaces
			{
				char * pName = (char *)( pChunk + 1 );
				char * pLastName = pName + chunkSize;
		
				while( pName < pLastName )
				{
					CharString name("");
					while( *pName != 0 && pName < pLastName )
					{
						name += CharString().format("%c",*pName);
						pName++;
					}

					// make the name pretty
					name.pretty();

					// create the surface
					LightWaveSurface & surface = lwSurfaces.push();
					surface.name = name;
					surface.smoothingAngle = 0.0f;
					surface.center = Vector3( 0.0f );
					surface.scale = Vector3( 0.0f );
					surface.axis = NOAXIS;
					surface.doubleSided = false;
					surface.clampU = surface.clampV = false;

					// skip any additional nulls
					while( *pName == 0 && pName < pLastName )
						pName++;
				}

				// allocate the triangles and materials
				int surfaceCount = lwSurfaces.size();
				m_Materials.allocate( surfaceCount );
				m_Triangles[0].allocate( surfaceCount );
			}
			break;
		case 'POLS':	// polygons
			{
				byte * pPolygon = (byte *)( pChunk + 1);
				byte * pLastPolygon = pPolygon + chunkSize;

				int triangleCount = 0;
				while( pPolygon < pLastPolygon )
				{
					word vertexCount = swapEndian( *((word *)pPolygon) );
					pPolygon += sizeof(word);

					bool validPolygon = true;
					if ( vertexCount < 3 )
						validPolygon = false;

					Array< int > polygonVerts( vertexCount );
					for(int i=0;i<vertexCount;i++)
					{
						int index = swapEndian( *((word *)pPolygon) );
						pPolygon += sizeof(word);

						ASSERT( index >= 0 && index < m_Verts.size() );
						polygonVerts[ i ] = index;
					}

					// get the surface index for this polygon
					short materialIndex = swapEndian( *((short *)pPolygon) );
					pPolygon += sizeof(short);

					if ( materialIndex < 0 )
					{
						MessageBox(NULL,"Cannot import object with detail polygons!", fileName, MB_OK );
						return;
					}

					// make the material index zero based
					materialIndex -= 1;
					ASSERT( materialIndex < m_Materials.size() );

					if ( validPolygon )
					{
						// convert this polygon into triangles
						Array< Triangle > & triangles = m_Triangles[0][ materialIndex ];
						for(int i=2;i<vertexCount;i++)
						{
							triangleCount++;

							Triangle & newTriangle = triangles.push();
							newTriangle.verts[0] = polygonVerts[ 0 ];
							newTriangle.verts[1] = polygonVerts[ i - 1];
							newTriangle.verts[2] = polygonVerts[ i ];
						}
					}
				}
				TRACE( CharString().format("NodeComplexMeshPort::importLWO(), %d triangles", triangleCount) );
			}
			break;
		case 'SURF':	// surface
			{
				// get the surface name
				char * pName = (char *)(pChunk + 1);

				CharString surfaceName( pName );
				surfaceName.pretty();

				int materialIndex = -1;
				for(int i = 0;i<lwSurfaces.size();i++)
					if ( surfaceName == lwSurfaces[i].name )
					{
						materialIndex = i;
						break;
					}

				// did we find the material
				if ( materialIndex < 0 )
					break;

				pName += wordAlign( surfaceName.length() + 1 );

				// create a material port
				MaterialPort * pMaterial = new MaterialPort;
				
				LWO_SUBCHUNK * pSubChunk = (LWO_SUBCHUNK *)( pName );
				LWO_SUBCHUNK * pLastSubChunk = (LWO_SUBCHUNK *)(((byte *)( pChunk + 1)) + chunkSize);

				while( pSubChunk < pLastSubChunk )
				{
					int subChunkSize = wordAlign( swapEndian( pSubChunk->size ) );

					switch( swapEndian( pSubChunk->tag ) )
					{
					case 'COLR':	// base color
						{
							byte * pColors = (byte *)( pSubChunk + 1 );

							Color baseColor( *pColors, 
								*(pColors + 1), 
								*(pColors + 2), 
								255 );
									
							//pMaterial->m_Diffuse = baseColor;
							//pMaterial->m_Ambient = baseColor;
							//pMaterial->m_Specular = baseColor;
						}
						break;
					case 'FLAG':	// Flags
						{
							word flags = swapEndian( *((word *)(pSubChunk + 1)) );

							if ( flags & 0x100 )
								lwSurfaces[ materialIndex ].doubleSided = true;
							if ( flags & 0x200 )	// additive
								pMaterial->m_Blending = PrimitiveMaterial::ADDITIVE;
						}
						break;
					case 'LUMI':	// Luminosity
						{
							word luminosity = swapEndian( *((word *)(pSubChunk + 1)) );

							float fLum = float(luminosity) / 256.0f;

							Color baseColor = Color( 128,128,128 ) * fLum;
							pMaterial->m_Diffuse = baseColor;
							pMaterial->m_Ambient = baseColor;
							pMaterial->m_Specular = baseColor;

							if ( fLum > 1.0f )
								pMaterial->m_Emissive = Color(255,255,255) * (fLum - 1.0f);
						}
						break;
					case 'DIFF':	// Diffuse
						{
							word diffuse = swapEndian( *((word *)(pSubChunk + 1)) );
						}
						break;
					case 'SPEC':	// Specular
						{
							word specular = swapEndian( *((word *)(pSubChunk + 1)) );
						}
						break;
					case 'REFL':	// Reflectivity
					case 'TRAN':	// Transparency
					case 'VLUM':	// Luminosity
					case 'VDIF':	// Diffuse
					case 'VSPC':	// Specular
					case 'VRFL':	// Reflectivity
					case 'VTRN':	// Transparency
					case 'GLOS':	// Glossiness
					case 'RFLT':	// Reflection Mode
					case 'RIMG':	// Reflection Image
					case 'RSAN':	// Reflection Map Image Seam Angle
					case 'RIND':	// Refractive Index
					case 'EDGE':	// Edge Transparency Threshold
						break;
					case 'SMAN':	// Maximum Smoothing Angle
						{
							float smoothingAngle = swapEndian( *((float *)(pSubChunk + 1)) );
							lwSurfaces[ materialIndex ].smoothingAngle = smoothingAngle;
						}
						break;
					case 'CTEX':	// Color Texture
					case 'DTEX':	// Diffuse Texture
					case 'STEX':	// Specular Texture
					case 'RTEX':	// Reflection Texture
					case 'TTEX':	// Transparency Texture
					case 'LTEX':	// Luminosity Texture
					case 'BTEX':	// Bump Texture
						break;
					case 'TFLG':	// Texture Flags
						{
							word flags = swapEndian( *((word *)(pSubChunk + 1)) );
							if ( flags & 0x1 )
								lwSurfaces[ materialIndex ].axis = XAXIS;
							else if ( flags & 0x2 )
								lwSurfaces[ materialIndex ].axis = YAXIS;
							else if ( flags & 0x4 )
								lwSurfaces[ materialIndex ].axis = ZAXIS;
						}
						break;
					case 'TSIZ':	// texture size
						{
							float * pSize = (float *)( pSubChunk + 1 );
							lwSurfaces[ materialIndex ].scale = 
								Vector3( swapEndian( pSize[0] ), 
								swapEndian( pSize[1] ), 
								swapEndian( pSize[2] ) );
						}
						break;
					case 'TCTR':	// texture center
						{
							float * pSize = (float *)( pSubChunk + 1 );
							lwSurfaces[ materialIndex ].center = 
								Vector3( swapEndian( pSize[0] ), 
								swapEndian( pSize[1] ), 
								swapEndian( pSize[2] ) );
						}
						break;
					case 'TREF':	// Texture Reference
					case 'TFAL':	// Texture Falloff
					case 'TVEL':	// Texture Velocity
					case 'TCLR':	// Texture Color
					case 'TVAL':	// Texture Value
					case 'TAMP':	// Bump Amplitude
					case 'TFP0':	// Algorithmic Texture Parameter
					case 'TFP1':	// Algorithmic Texture Parameter
					case 'TFP2':	// Algorithmic Texture Parameter
					case 'TFP3':	// Algorithmic Texture Parameter
					case 'TIP0':	// Algorithmic Texture Parameter	
					case 'TSP0':	// Algorithmic Texture Parameter
					case 'TFRQ':	// Texture Frequency
						break;
					case 'TIMG':	// Texture Image
						{
							CharString fileName( (char *)( pSubChunk + 1) );
							CharString diffusePort( imagePath + Path( fileName ).name() + ".prt" );

							pMaterial->m_DiffusePort = diffusePort;

							if ( FileDisk::fileExists( diffusePort ) )
								break;

							// create a image port of this texture
							ImagePort * pImagePort = new ImagePort;
							if ( pImagePort->importFile( fileName ) )
							{
								// save the image port
								try {
									OutStream( new FileDisk( diffusePort, FileDisk::READ_WRITE ) ) 
										<< pImagePort;
								}
								catch( File::FileError )
								{
									MessageBox(NULL,"Failed to save image port!", diffusePort, MB_OK );
								}
							}
							else
							{
								MessageBox(NULL,"Failed to import image!", fileName, MB_OK );
								delete pImagePort;
							}
						}
						break;
					case 'TALP':	// Texture Alpha Image
						{
							CharString fileName( (char *)( pSubChunk + 1) );
							CharString alphaPort( imagePath + Path( fileName ).name() + ".prt" );

							pMaterial->m_AlphaPort = alphaPort;

							if ( FileDisk::fileExists( alphaPort ) )
								break;

							// create a image port of this texture
							ImagePort * pImagePort = new ImagePort;
							if ( pImagePort->importFile( fileName ) )
							{
								// save the image port
								try {
									OutStream( new FileDisk( alphaPort, FileDisk::READ_WRITE ) ) 
										<< pImagePort;
								}
								catch( File::FileError )
								{
									MessageBox(NULL,"Failed to save image port!", alphaPort, MB_OK );
								}
							}
							else
							{
								MessageBox(NULL,"Failed to import image!", fileName, MB_OK );
								delete pImagePort;
							}
						}
						break;
					case 'TWRP':	// Texture Wrap Mode
						{
							word * pWord = (word *)(pSubChunk + 1);
							word widthWrap = swapEndian( pWord[ 0 ] );
							word heightWrap = swapEndian( pWord[ 1 ] );

							lwSurfaces[ materialIndex ].clampU = widthWrap != 2 ? true : false;
							lwSurfaces[ materialIndex ].clampV = heightWrap != 2 ? true : false;
						}
						break;
					case 'TAAS':	// Texture Antialiasing Strength
					case 'TOPC':	// Relative Texture Opacity
					case 'SHDR':	// Shader File
					case 'SDAT':	// Shader Data
					case 'IMSQ':	// Image Sequence
					case 'FLYR':	// Flyer Timecode
					case 'IMCC':	// Color Cycle
						break;
					default:
						MessageBox(NULL, CharString().format("Unknown LWO surface chunk 0x%x", pSubChunk->tag), 
							fileName, MB_OK );
						break;
					}

					pSubChunk = (LWO_SUBCHUNK *)(((byte *)(pSubChunk + 1)) + subChunkSize);
				}

				CharString materialPort( materialPath + surfaceName + ".prt" );

				try {
					OutStream( new FileDisk( materialPort, FileDisk::READ_WRITE ) ) 
						<< pMaterial;
				}
				catch( File::FileError )
				{
					MessageBox(NULL,"Failed to save material", materialPort, MB_OK );
				}

				// set the material port
				m_Materials[ materialIndex ] = materialPort;
			}
			break;
		default:
			MessageBox(NULL, CharString().format("Unknown LWO chunk 0x%x", pChunk->tag), fileName, MB_OK );
			break;

		} // end switch( swapEndian( pChunk->tag ) )

		pChunk = (LWO_CHUNK *)(((byte *)(pChunk + 1)) + chunkSize);
	} // end while( pChunk < pLastChunk )

	// check for double-sided surfaces
	for(int i=0;i<lwSurfaces.size();i++)
		if (lwSurfaces[i].doubleSided)
		{
			Array< Triangle > & triangles = m_Triangles[0][ i ];

			int triangleCount = triangles.size();
			for(int k=0;k<triangleCount;k++)
			{
				Triangle newTriangle = triangles[k];

				int swap = newTriangle.verts[0];
				newTriangle.verts[0] = newTriangle.verts[2];
				newTriangle.verts[2] = swap;

				triangles.push( newTriangle );
			}
		}

	// calculate the UV's for all the polygons
	BoxHull objectHull;
	for(int i=0;i<m_Verts.size();i++)
		objectHull.addVertex( m_Verts[i].position );

	// get the center point of this object
	Vector3 objectCenter( (objectHull.max + objectHull.min) * 0.5f );

	Vector3 radiusVector( objectHull.max );
	radiusVector -= objectCenter;

	// calculate the scale
	float scale = sqrtf( radiusVector | radiusVector );
	float invScale = 1.0f / scale;

	for(int i=0;i<m_Materials.size();i++)
	{
		Vector3 textureOrigin( (lwSurfaces[i].center - objectCenter) * invScale );
		Vector3 textureScale( lwSurfaces[i].scale * invScale );

		Vector3 textureScaleInv( 1.0f / textureScale.x,
			1.0f / textureScale.y,
			1.0f / textureScale.z );

		bool clampU = lwSurfaces[i].clampU;
		bool clampV = lwSurfaces[i].clampV;

		Array< Triangle > & triangles = m_Triangles[0][ i ];
		switch( lwSurfaces[i].axis )
		{
		case NOAXIS:
			break;
		case XAXIS:
			{
				for(int j=0;j<triangles.size();j++)
				{
					Triangle & triangle = triangles[ j ];
					for(int k=0;k<3;k++)
					{
						Vertex & vertex = m_Verts[ triangle.verts[k] ];
						Vector3 point(( vertex.position - objectCenter) * invScale );
						vertex.u = ((point.z - textureOrigin.z) * textureScaleInv.z) + 0.5f;
						if ( clampU )
							vertex.u = Clamp( vertex.u, 0.0f, 1.0f );
						vertex.v = ((textureOrigin.y - point.y) * textureScaleInv.y) + 0.5f;
						if ( clampV )
							vertex.v = Clamp( vertex.v, 0.0f, 1.0f );
					}
				}
			}
			break;
		case YAXIS:
			{
				for(int j=0;j<triangles.size();j++)
				{
					Triangle & triangle = triangles[ j ];
					for(int k=0;k<3;k++)
					{
						Vertex & vertex = m_Verts[ triangle.verts[k] ];

						Vector3 point( (vertex.position - objectCenter) * invScale );
						vertex.u = ((point.x - textureOrigin.x) * textureScaleInv.x) + 0.5f;
						if ( clampU )
							vertex.u = Clamp( vertex.u, 0.0f, 1.0f );
						vertex.v = ((textureOrigin.z - point.z ) * textureScaleInv.z) + 0.5f;
						if ( clampV )
							vertex.v = Clamp( vertex.v, 0.0f, 1.0f );
					}
				}
			}
			break;
		case ZAXIS:
			{
				for(int j=0;j<triangles.size();j++)
				{
					Triangle & triangle = triangles[ j ];
					for(int k=0;k<3;k++)
					{
						Vertex & vertex = m_Verts[ triangle.verts[k] ];

						Vector3 point( (vertex.position - objectCenter) * invScale );
						vertex.u = ((point.x - textureOrigin.x) * textureScaleInv.x) + 0.5f;
						if ( clampU )
							vertex.u = Clamp( vertex.u, 0.0f, 1.0f );
						vertex.v = ((textureOrigin.y - point.y) * textureScaleInv.y) + 0.5f;
						if ( clampV )
							vertex.v = Clamp( vertex.v, 0.0f, 1.0f );
					}
				}
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------

struct LAYR
{
	word			number;
	word			flags;
	Vector3	pivot;		
	CharString	name;
	word			parent;

	LAYR & operator=(const LAYR & copy )
	{
		number = copy.number;
		flags = copy.flags;
		pivot = copy.pivot;
		name = copy.name;
		parent = copy.parent;

		return *this;
	}
};

struct UV
{
	float u, v;
};

struct VMAP
{
	CharString			name;
	Hash< int,UV >		uv;

	VMAP & operator=( const VMAP & copy )
	{
		name = copy.name;
		uv = copy.uv;

		return *this;
	}
};

struct CLIP
{
	dword			id;
	CharString	image;

	CLIP & operator=( const CLIP & copy )
	{
		id = copy.id;
		image = copy.image;

		return *this;
	}
};

struct SURF
{
	CharString		name;
	CharString		source;
	CharString		map;		// name of uvmap to use
	dword				clip;		// clip aka image to use for surface

	SURF & operator=(const SURF & copy )
	{
		name = copy.name;
		source = copy.source;
		//color = copy.color;
		//sided = copy.sided;
		map = copy.map;
		clip = copy.clip;
		return *this;
	}
};

//----------------------------------------------------------------------------

inline CharString parseString( byte * &pointer )
{
	CharString string;

	// add to the string until the null terminator or the end of the block
	while( *pointer != 0 )
	{
		string += *pointer;
		pointer++;
	}
	pointer++;

	// if the string length is odd, then skip one more byte
	if ( dword( pointer ) & 0x1 )
		pointer++;

	return string;
}

// parse variable length index!
inline dword parseVX( byte * &pointer )
{
	if ( *pointer == 0xff )
	{
		dword value = swapEndian( *((dword *)pointer) ) & 0x00ffffff;
		pointer += sizeof(dword);

		return value;
	}

	dword value = swapEndian( *((word *)pointer) );
	pointer += sizeof(word);
	
	return value;
}

inline dword parseDWORD( byte * &pointer )
{
	dword value = swapEndian( *((dword *)pointer) );
	pointer += sizeof(dword);

	return value;
}

inline word parseWORD( byte * &pointer )
{
	word value = swapEndian( *((word *)pointer) );
	pointer += sizeof(word);

	return value;
}

inline float parseFloat( byte * &pointer )
{
	float value = swapEndian( *((float *)pointer) );
	pointer += sizeof(float);

	return value;
}

inline LWO_CHUNK parseChunk( byte * &pointer )
{
	LWO_CHUNK chunk;
	chunk.tag = parseDWORD( pointer );
	chunk.size = parseDWORD( pointer );

	return chunk;
}

inline LWO_SUBCHUNK parseSubChunk( byte * &pointer )
{
	LWO_SUBCHUNK chunk;
	chunk.tag = parseDWORD( pointer );
	chunk.size = parseWORD( pointer );

	return chunk;
}

inline Color parseColor( byte * &pointer )
{
	Color color( parseFloat( pointer ) * 255, parseFloat( pointer ) * 255, parseFloat( pointer ) * 255, 255 );
	return color;
}

inline CharString stringTag( dword tag )
{
	CharString string;
	string.allocate( ' ', 4 );
	string[ 0 ] = tag >> 24;
	string[ 1 ] = (tag >> 16) & 0xff;
	string[ 2 ] = (tag >> 8) & 0xff;
	string[ 3 ] = tag & 0xff;

	return string;
}

void NodeComplexMeshPort::decodeLWO2( const char * fileName, byte * pBeginForm, byte * pEndForm )
{
	CharString importPath( CResourcerDoc::document()->currentDirectory() );

	m_Triangles.allocate( 1 );

	// create the image and material directories
	CharString materialPath( importPath + "Materials\\" );
	CreateDirectory( materialPath, NULL );
	CharString imagePath( importPath + "Images\\" );
	CreateDirectory( imagePath, NULL );

	Array< CharString >			tags;
	Array< LAYR >					layers;							// layers
	Array< SURF >					surfaces;						// surfaces
	Array< Array< int > >		polygons;						// polygons
	Hash< int, Array< int > >	surfacePolygons;				// surface -> polygon index
	Array< VMAP >					maps;							// uvmaps
	Array< CLIP >					clips;							// image clips

	// parse the chunks
	while( pBeginForm < pEndForm )
	{
		LWO_CHUNK chunk = parseChunk( pBeginForm );
		int chunkSize = wordAlign( chunk.size );

		byte * pBeginChunk = pBeginForm;
		byte * pEndChunk = pBeginChunk + chunkSize;
		byte * pNextChunk = pBeginForm + chunkSize;
		
		switch( chunk.tag )
		{
		case 'SURF':
			{
				SURF & surface = surfaces.push();
				surface.name = parseString( pBeginChunk );
				surface.source = parseString( pBeginChunk );
				//surface.color = Color(255,255,255,255);
				//surface.sided = false;
				surface.clip = 0xffffffff;

				// look for source surface, copy attributes to this surface
				for(int i=0;i<surfaces.size() - 1;i++)
					if ( surfaces[i].name == surface.source )
						surface = surfaces[i];

				while( pBeginChunk < pEndChunk )
				{
					LWO_SUBCHUNK subChunk = parseSubChunk( pBeginChunk );
					byte * pEndSubChunk = pBeginChunk + wordAlign( subChunk.size );

					switch( subChunk.tag )
					{
					case 'COLR':
						//surface.color = parseColor( pBeginChunk );
						break;
					case 'SIDE':
						if ( parseWORD( pBeginChunk ) == 3 )
							MessageBox( NULL, "Double-sided surfaces are not allowed!", fileName, MB_OK );
						//surface.sided = parseWORD( pBeginChunk ) == 3 ? true : false;
						break;
					case 'BLOK':
						{
							dword type = parseDWORD( pBeginChunk );
							dword id = parseDWORD( pBeginChunk );

							if ( type == 'IMAP' )
							{
								while( pBeginChunk < pEndSubChunk )
								{
									LWO_SUBCHUNK subChunk = parseSubChunk( pBeginChunk );
									byte * pEndSubChunk = pBeginChunk + wordAlign( subChunk.size );

									switch( subChunk.tag )
									{
									case 'PROJ':
										if ( parseWORD( pBeginChunk ) != 5 )
											MessageBox( NULL, "All surfaces must be UV mapped!", fileName, MB_OK );
										break;
									case 'VMAP':
										surface.map = parseString( pBeginChunk );
										break;
									case 'IMAG':
										surface.clip = parseVX( pBeginChunk );
										break;
									case 'CHAN':	// texture channel
									case 'OPAC':	// opacity
									case 'ENAB':	// enabled
									case 'NEGA':	// ?
									case 'AXIS':	// axis of texture
									case 'TMAP':	// texture map information
									case 'WRAP':	// image wrap options
									case 'WRPW':	// image wrap amounts
									case 'WRPH':	// image wrap amounts
									case 'AAST':	// antialiasing strength
									case 'PIXB':	// pixel blending
									case 'STCK':	// ?
									case 'TAMP':	// ?
										break;
									default:
										MessageBox(NULL, CharString().format("Unknown BLOK subchunk '%s'", stringTag( subChunk.tag )), fileName, MB_OK );
										break;
									}

									pBeginChunk = pEndSubChunk;
								}
							}
						}
						break;
					case 'DIFF':	// surface diffuse intensity
					case 'LUMI':	// surface luminosity
					case 'SPEC':	// specular intensity
					case 'REFL':	// reflection intensity
					case 'TRAN':	// transparency
					case 'TRNL':	// tranlucency
					case 'SMAN':	// smoothing angle
					case 'RFOP':	// reflection options
					case 'TROP':	// transparency options
						break;
					default:
						MessageBox(NULL, CharString().format("Unknown SURF subchunk '%s'", stringTag( subChunk.tag ) ), fileName, MB_OK );
						break;
					}

					pBeginChunk = pEndSubChunk;
				}
			}
			break;
		case 'TAGS':	// tags, replaced the SRFS chunk
			{
				while( pBeginChunk < pEndChunk )
				{
					tags.push( parseString( pBeginChunk ) );
				}
			}
			break;
		case 'ENVL':	// envelope
			break;
		case 'CLIP':	// image or sequence definition
			{
				CLIP & clip = clips.push();
				clip.id = parseDWORD( pBeginChunk );

				while( pBeginChunk < pEndChunk )
				{
					LWO_SUBCHUNK subChunk = parseSubChunk( pBeginChunk );
					byte * pNextChunk = pBeginChunk + wordAlign( subChunk.size );

					switch( subChunk.tag )
					{
					case 'STIL':
						clip.image = parseString( pBeginChunk );
						break;
					default:
						MessageBox(NULL, CharString().format("Unknown CLIP subchunk '%s'", stringTag( subChunk.tag ) ), fileName, MB_OK );
						break;
					}

					pBeginChunk = pNextChunk;
				}
			}
			break;
		case 'LAYR':	// layer
			{
				LAYR & layer = layers.push();
				layer.number = parseWORD( pBeginChunk );
				layer.flags = parseWORD( pBeginChunk );
				layer.pivot.x = parseFloat( pBeginChunk );
				layer.pivot.y = parseFloat( pBeginChunk );
				layer.pivot.z = parseFloat( pBeginChunk );
				layer.name = parseString( pBeginChunk );
				layer.parent = parseWORD( pBeginChunk );
			}
			break;
		case 'VMAP':	// vertex mapping
			{
				if ( parseDWORD( pBeginChunk ) == 'TXUV' )
				{
					if ( parseWORD( pBeginChunk ) == 2 )
					{
						VMAP & map = maps.push();
						map.name = parseString( pBeginChunk );

						while( pBeginChunk < pEndChunk )
						{
							UV & uv = map.uv[ parseVX( pBeginChunk ) ];
							uv.u = parseFloat( pBeginChunk );
							uv.v = 1.0f - parseFloat( pBeginChunk );
						}
					}
					else
						MessageBox( NULL, "UV map should only have 2 dimensions!", fileName, MB_OK );
				}
			}
			break;
		case 'PNTS':	// points
			{
				// calculate the number of points from the chunk size
				int pointCount = chunkSize / 12;

				// allocate the verticies
				m_Verts.allocate( pointCount );
				for(int i=0;i<pointCount;i++)
				{
					Vertex & newVertex = m_Verts[i];
					newVertex.position.x = parseFloat( pBeginChunk );
					newVertex.position.y = parseFloat( pBeginChunk );
					newVertex.position.z = parseFloat( pBeginChunk );
					//newVertex.diffuse = Color(255,255,255,255);
					newVertex.normal = Vector3(0,1,0);		// calculate the vertex normal later
				}

				ASSERT( pBeginChunk <= pEndChunk );

				TRACE( CharString().format("NodeComplexMeshPort::importLWO(), %d points", pointCount) );
			}
			break;
		case 'POLS':	// polygons
			{
				// check the polygon type, this can be FACE (supported), CURV, or PACH
				if ( parseDWORD( pBeginChunk ) == 'FACE' )
				{
					while( pBeginChunk < pEndChunk )
					{
						word vertexCount = parseWORD( pBeginChunk );

						Array< int > & polygon = polygons.push();
						polygon.allocate( vertexCount );

						for(int i=0;i<vertexCount;i++)
						{
							polygon[ i ] = parseVX( pBeginChunk );
							ASSERT( polygon[ i ] < m_Verts.size() );
						}
					}
				}
			}
			break;
		case 'PTAG':	// polygon tag mapping, maps the polygons to the surfaces
			{
				// check the polygon tag type
				if ( parseDWORD( pBeginChunk ) == 'SURF' )
				{
					while( pBeginChunk < pEndChunk )
					{
						dword index = parseVX( pBeginChunk );
						word tag = parseWORD( pBeginChunk );

						surfacePolygons[ tag ].push( index );
					}
				}
			}
			break;
		case 'DESC':	// description line (optional)
		case 'BBOX':	// bounding box (optional)
		case 'TEXT':	// comment (optional)
		case 'ICON':	// thumbnail icon image (optional)
			break;
		default:
			MessageBox(NULL, CharString().format("Unknown LWO2 chunk '%s'", stringTag( chunk.tag ) ), fileName, MB_OK );
			break;

		} // end switch( swapEndian( pChunk->tag ) )

		pBeginForm = pNextChunk;
	} // end while( pChunk < pLastChunk )


	// allocate the triangles and materials
	int surfaceCount = surfaces.size();
	m_Materials.allocate( surfaceCount );
	m_Triangles[0].allocate( surfaceCount );

	// build the surfaces for the mesh
	for(int i=0;i<surfaceCount;i++)
	{
		SURF & surface = surfaces[i];
		//ASSERT( surface.name == tags[i] );

		CharString materialPort( materialPath + surface.name + ".prt" );
		m_Materials[ i ] = materialPort;

		if ( FileDisk::fileExists( materialPort ) )
		{
			if ( MessageBox( NULL, "MaterialPort already exists, overwrite?", materialPort, MB_YESNO ) != IDYES )
				continue;
		}

		// create the material port
		MaterialPort * pMaterial = new MaterialPort;

		// build the image port 
		for(int j=0;j<clips.size();j++)
			if ( clips[j].id == surface.clip )
			{
				CLIP & clip = clips[j];

				CharString fileName( clip.image );
				CharString diffusePort( imagePath + Path( fileName ).name() + ".prt" );

				pMaterial->m_DiffusePort = diffusePort;

				if ( FileDisk::fileExists( diffusePort ) )
				{
					// ask if user wants to overwrite the imageport
					if ( MessageBox( NULL, "ImagePort already exists, overwrite?", diffusePort, MB_YESNO ) != IDYES )
						break;
				}

				// create a image port of this texture
				ImagePort * pImagePort = new ImagePort;
				if ( pImagePort->importFile( fileName ) )
				{
					// save the image port
					try {
						OutStream( new FileDisk( diffusePort, FileDisk::READ_WRITE ) ) 
							<< pImagePort;
					}
					catch( File::FileError )
					{
						MessageBox(NULL,"Failed to save image port!", diffusePort, MB_OK );
					}
				}
				else
					MessageBox(NULL,"Failed to import image!", fileName, MB_OK );

				delete pImagePort;
				break;
			}

		// save the material to disk
		try {
			OutStream( new FileDisk( materialPort, FileDisk::READ_WRITE ) ) 
				<< pMaterial;
		}
		catch( File::FileError )
		{
			MessageBox(NULL,"Failed to save material", materialPort, MB_OK );
		}

		delete pMaterial;

		// find the polygon group
		int polygonGroup = -1;
		for(int t=0;t<tags.size();t++)
			if ( surface.name.compareNoCase( tags[t] ) == 0 )
			{
				polygonGroup = t;
				break;
			}

		if ( polygonGroup < 0 )
		{
			MessageBox( NULL, "Failed to find polygons for surface!", surface.name, MB_OK );
			continue;
		}

		if ( surface.map.length() > 0 )
		{
			// find the UV map for this surface
			for(int k=0;k<maps.size();k++)
				if ( maps[k].name == surface.map )
				{
					VMAP & map = maps[k];

					// generate the triangles for this surface
					Array< int > & spolygons = surfacePolygons[ polygonGroup ];
					for(int j=0;j<spolygons.size();j++)
					{
						int index = spolygons[ j ];

						ASSERT( index < polygons.size() );
						Array< int > & verts = polygons[ index ];

						// convert this polygon into triangles
						Array< Triangle > & triangles = m_Triangles[0][ i ];
						for(int i=2;i<verts.size();i++)
						{
							Triangle & newTriangle = triangles.push();
							
							newTriangle.verts[0] = verts[ 0 ];
							//newTriangle.uv[ 0 ] = map.uv[ verts[ 0 ] ];
							newTriangle.verts[1] = verts[ i - 1];
							//newTriangle.uv[ 1 ] = map.uv[ verts[ i - 1 ] ];
							newTriangle.verts[2] = verts[ i ];
							//newTriangle.uv[ 2 ] = map.uv[ verts[ i ] ];
						}
					}

					break;
				}
		}
		else
		{
			// no UV map

			// generate the triangles for this surface
			Array< int > & spolygons = surfacePolygons[ polygonGroup ];
			for(int j=0;j<spolygons.size();j++)
			{
				int index = spolygons[ j ];

				ASSERT( index < polygons.size() );
				Array< int > & verts = polygons[ index ];

				// convert this polygon into triangles
				Array< Triangle > & triangles = m_Triangles[0][ i ];
				for(int i=2;i<verts.size();i++)
				{
					Triangle & newTriangle = triangles.push();
					newTriangle.verts[0] = verts[ 0 ];
					newTriangle.verts[1] = verts[ i - 1];
					newTriangle.verts[2] = verts[ i ];
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------
// EOF


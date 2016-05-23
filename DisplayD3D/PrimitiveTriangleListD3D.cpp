/*
	PrimitiveTriangleListD3D.cpp
	(c)2005 Palestar, Richard Lyle
*/
 
#define USE_DRAW_INDEXED_PRIMITIVE		// undefine to use the slower DrawPrimitiveVB

#include "DisplayD3D/PrimitiveTriangleListD3D.h"
#include "DisplayD3D/PrimitiveFactory.h"

//------------------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleListD3D );

PrimitiveTriangleListD3D::PrimitiveTriangleListD3D() : m_VB( NULL )
{}

bool PrimitiveTriangleListD3D::execute()
{
	int triangleCount = m_VBSize / 3;
	if ( triangleCount > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(Vertex) ) != D3D_OK )
			return false;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLELIST, 0, triangleCount ) != D3D_OK )
			return false;
		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}

	return true;
}

void PrimitiveTriangleListD3D::clear()
{}

void PrimitiveTriangleListD3D::release()
{
	RELEASEDD( m_VB );
}

bool PrimitiveTriangleListD3D::initialize( int triangleCount, const Triangle * pTriangles )
{
	RELEASEDD( m_VB );

	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	m_VBSize = triangleCount * 3;

	dword vbSize = m_VBSize * sizeof(Vertex);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, 
		D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;
	
	byte * pBuffer;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;
	memcpy( pBuffer, pTriangles, vbSize );
	m_VB->Unlock();

	//TRACE( CharString().format("PrimitiveTriangleListD3D::initialize, m_VBSize = %d", m_VBSize ) );
	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleListDD3D );

PrimitiveTriangleListDD3D::PrimitiveTriangleListDD3D() : m_pTriangles( NULL ), m_TriangleCount( 0 )
{}

bool PrimitiveTriangleListDD3D::execute( )
{
	if ( m_TriangleCount > 0 )
	{
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB(
			DisplayDeviceD3D::VB_VERTEX, m_TriangleCount * 3, m_pTriangles );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLELIST, vbOffset, m_TriangleCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nTrianglesRendered += m_TriangleCount;
	}

	return true;
}

void PrimitiveTriangleListDD3D::clear()
{}

void PrimitiveTriangleListDD3D::release()
{
	delete [] m_pTriangles;
	m_pTriangles = NULL;

	m_TriangleCount = 0;
}

bool PrimitiveTriangleListDD3D::initialize( int triangleCount )
{
	release();

	m_pTriangles = new Triangle[ triangleCount ];
	m_TriangleCount = triangleCount;
	return true;
}

bool PrimitiveTriangleListDD3D::initialize( int triangleCount, const Triangle * pTriangles )
{
	initialize( triangleCount );
	memcpy( m_pTriangles, pTriangles, sizeof(Triangle) * triangleCount );
	return true;
}

Triangle * PrimitiveTriangleListDD3D::lock()
{
	return m_pTriangles;
}

void PrimitiveTriangleListDD3D::unlock()
{}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleListID3D );

PrimitiveTriangleListID3D::PrimitiveTriangleListID3D() : m_VB( NULL ), m_VBI( NULL ), m_SystemVB( NULL )
{}

bool PrimitiveTriangleListID3D::execute()
{
	if ( m_Triangles > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(Vertex) ) != D3D_OK )
			return false;
		if ( d3dd->SetIndices( m_VBI ) != D3D_OK )
			return false;
		if ( d3dd->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_Verts, 0, m_Triangles ) != D3D_OK )
			return false;
		DisplayDevice::sm_nTrianglesRendered += m_Triangles;
	}
	return true;
}

void PrimitiveTriangleListID3D::clear()
{}

void PrimitiveTriangleListID3D::release()
{
	RELEASEDD( m_VB );
	RELEASEDD( m_SystemVB );
	RELEASEDD( m_VBI );
}

bool PrimitiveTriangleListID3D::initialize( int triangleCount, const Triangle * pTriangles )
{
	release();

	//TRACE( CharString().format("PrimitiveTriangleListID3D::initialize, building index table from %d triangles...", triangleCount ) );
	m_Triangles = triangleCount;

	// create the vertex array
	Array< Vertex > verts;
	// allocate the index array
	Array< word > vbi( m_Triangles * 3 );
	ASSERT( vbi.size() < 65536 );

	int index = 0;
	for(int i=0;i<triangleCount;i++)
		for(int j=0;j<3;j++)
		{
			const Vertex & vert = pTriangles[i].v[j];
	
			vbi[ index ] = 0xffff;

			// check the verts array for a vertex that matches
			for(int k=0;k<verts.size();k++)
				if ( (verts[k].position - vert.position).magnitude2() < 0.001f &&
					(verts[k].normal - vert.normal).magnitude2() < 0.001f &&
					fabs(verts[k].u - vert.u) < 0.001f &&
					fabs(verts[k].v - vert.v) < 0.001f )
				{
					// found matching vert
					vbi[ index ] = k;
					break;
				}

			if ( vbi[ index ] == 0xffff )
			{
				// add new unique vertex
				verts.push( vert );
				vbi[ index ] = verts.size() - 1;
			}

			// next vertex
			index++;
		}
	ASSERT( index == vbi.size() );

	m_Verts = verts.size();

	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	dword vbSize = m_Verts * sizeof(Vertex);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, 
		D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;
	
	byte * pBuffer = NULL;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;

	for(int i=0;i<m_Verts;i++)
		((Vertex *)pBuffer)[i] = verts[i];

	m_VB->Unlock();

	dword vbiSize = vbi.size() * sizeof(word);
	if ( d3dd->CreateIndexBuffer( vbiSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_VBI, NULL ) != D3D_OK )
		return false;

	if ( m_VBI->Lock( 0, vbiSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;

	for(int i=0;i<vbi.size();i++)
		((word *)pBuffer)[i] = vbi[i];

	m_VBI->Unlock();

	//TRACE( CharString().format("PrimitiveTriangleListID3D::initialize, m_Triangles = %d, m_Verts = %d", m_Triangles, m_Verts ) );
	return true;
}

bool PrimitiveTriangleListID3D::initialize( int triangleCount, const word * pTriangles,
								int vertexCount, const Vertex * pVerticies )
{
	release();

	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	m_Triangles = triangleCount;
	m_Verts = vertexCount;

	dword vbSize = m_Verts * sizeof(Vertex);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, 
		D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;
	
	byte * pBuffer;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;
	memcpy( pBuffer, pVerticies, vbSize );
	m_VB->Unlock();

	dword vbi = m_Triangles * 3;
	dword vbiSize = vbi * sizeof(word);
	if ( d3dd->CreateIndexBuffer( vbiSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_VBI, NULL ) != D3D_OK )
		return false;

	if ( m_VBI->Lock( 0, vbiSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;
	memcpy( pBuffer, pTriangles, vbiSize );
	m_VBI->Unlock();

	//TRACE( CharString().format("PrimitiveTriangleListID3D::initialize, m_Triangles = %d, m_Verts = %d", m_Triangles, m_Verts ) );
	return true;
}

bool PrimitiveTriangleListID3D::initialize( int triangleCount, const word * pTriangles,
								PrimitiveTriangleListI * pVerts )
{
	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	release();

	m_Triangles = triangleCount;
	m_Verts = ((PrimitiveTriangleListID3D *)pVerts)->m_Verts;	

	// share the vertex buffer with the other primitive
	m_VB = ((PrimitiveTriangleListID3D *)pVerts)->m_VB;
	if ( !m_VB )
		return false;
	m_VB->AddRef();

	dword vbi = m_Triangles * 3;
	dword vbiSize = vbi * sizeof(word);
	if ( d3dd->CreateIndexBuffer( vbiSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_VBI, NULL ) != D3D_OK )
		return false;

	byte * pBuffer;
	if ( m_VBI->Lock( 0, vbiSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;
	memcpy( pBuffer, pTriangles, vbiSize );
	m_VBI->Unlock();

	//TRACE( CharString().format("PrimitiveTriangleListID3D::initialize, m_Triangles = %d, m_Verts = %d (%p)", m_Triangles, m_Verts, pVerts ) );
	return true;
}

bool PrimitiveTriangleListID3D::createOptimizedVB( LPDIRECT3DDEVICE9 d3dd, int triangleCount, const word * pTriangles )
{
	ASSERT( m_SystemVB );

	m_Triangles = triangleCount;

	// get the system vertex buffer info
	D3DVERTEXBUFFER_DESC svbd;
	m_SystemVB->GetDesc( &svbd );
	// calculate the number of verticies in this buffer
	int vertexCount = svbd.Size / sizeof(Vertex );
	
	// lock the system vertex buffer
	Vertex * pVerticies = NULL;
	if ( m_SystemVB->Lock( 0, svbd.Size, (void **)&pVerticies, 0 ) != D3D_OK )
		return false;

	// initialize the remap array
	Array< int > remap( vertexCount );
	for(int i=0;i<remap.size();i++)
		remap[i] = -1;

	// initialize the culled verts array
	Array< Vertex > culled;
	//culled.setGrowSize( vertexCount );

	// create the index buffer, cull and sort the veritices while initializing the index buffer
	dword vbi = m_Triangles * 3;
	dword vbiSize = vbi * sizeof(word);
	if ( d3dd->CreateIndexBuffer( vbiSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_VBI, NULL ) != D3D_OK )
		return false;

	byte * pBuffer = NULL;
	if ( m_VBI->Lock( 0, vbiSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;

	for(dword j=0;j<vbi;j++)
	{
		word v = pTriangles[j];
		if ( remap[v] < 0 )
		{
			remap[v] = culled.size();
			culled.push( pVerticies[v] );
		}

		((word *)pBuffer)[j] = remap[v];
	}
	m_VBI->Unlock();

	// create the optmized vertex buffer
	m_Verts = culled.size();
	dword vbSize = m_Verts * sizeof(Vertex);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, 
		D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;
	
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;

	for(int i=0;i<culled.size();i++)
		((Vertex *)pBuffer)[i] = culled[i];

	m_VB->Unlock();
	m_SystemVB->Unlock();

	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleListLD3D );

PrimitiveTriangleListLD3D::PrimitiveTriangleListLD3D() : m_VB( NULL ), m_VBSize( 0 )
{}

bool PrimitiveTriangleListLD3D::execute()
{
	int triangleCount = m_VBSize / 3;
	if ( triangleCount > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(VertexL) ) != D3D_OK )
			return false;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLELIST, 0, triangleCount ) != D3D_OK )
			return false;
		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}
	return true;
}

void PrimitiveTriangleListLD3D::clear()
{}

void PrimitiveTriangleListLD3D::release()
{
	RELEASEDD( m_VB );
	m_VBSize = 0;
}

bool PrimitiveTriangleListLD3D::initialize( int triangleCount, const TriangleL * pTriangles )
{
	ASSERT( triangleCount > 0 );

	release();

	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	m_VBSize = triangleCount * 3;

	dword vbSize = m_VBSize * sizeof(VertexL);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1, 
		D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;
	
	byte * pBuffer;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;
	memcpy( pBuffer, pTriangles, vbSize );
	m_VB->Unlock();

	//TRACE( CharString().format("PrimitiveTriangleListLD3D::initialize, m_VBSize = %d", m_VBSize ) );
	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleListDLD3D );

PrimitiveTriangleListDLD3D::PrimitiveTriangleListDLD3D() : m_pTriangles( NULL ), m_TriangleCount( 0 )
{}

bool PrimitiveTriangleListDLD3D::execute()
{
	if ( m_TriangleCount > 0 )
	{
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEXL, m_TriangleCount * 3, m_pTriangles );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLELIST, vbOffset, m_TriangleCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nTrianglesRendered += m_TriangleCount;
	}
	return true;
}

void PrimitiveTriangleListDLD3D::clear()
{}

void PrimitiveTriangleListDLD3D::release()
{
	delete [] m_pTriangles;
	m_pTriangles = NULL;

	m_TriangleCount = 0;
}

bool PrimitiveTriangleListDLD3D::initialize( int triangleCount )
{
	release();

	m_pTriangles = new TriangleL[ triangleCount ];
	m_TriangleCount = triangleCount;
	return true;
}

bool PrimitiveTriangleListDLD3D::initialize( int triangleCount, const TriangleL * pTriangles )
{
	initialize( triangleCount );
	memcpy( m_pTriangles, pTriangles, sizeof(TriangleL) * triangleCount );
	return true;
}

TriangleL * PrimitiveTriangleListDLD3D::lock()
{
	return m_pTriangles;
}

void PrimitiveTriangleListDLD3D::unlock()
{}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleListTLD3D );

PrimitiveTriangleListTLD3D::PrimitiveTriangleListTLD3D() : m_VB( NULL )
{}

bool PrimitiveTriangleListTLD3D::execute()
{
	int triangleCount = m_VBSize / 3;
	if ( triangleCount > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(VertexTL) ) != D3D_OK )
			return false;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLELIST, 0, triangleCount ) != D3D_OK )
			return false;
		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}
	return true;
}

void PrimitiveTriangleListTLD3D::clear()
{}

void PrimitiveTriangleListTLD3D::release()
{
	RELEASEDD( m_VB );
}

bool PrimitiveTriangleListTLD3D::initialize( int triangleCount, const TriangleTL * pTriangles )
{
	release();

	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	m_VBSize = triangleCount * 3;

	dword vbSize = m_VBSize * sizeof(VertexTL);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, 
		D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;
	
	byte * pBuffer;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;
	memcpy( pBuffer, pTriangles, vbSize );
	m_VB->Unlock();

	//TRACE( CharString().format("PrimitiveTriangleListTLD3D::initializeVB, m_VBSize = %d", m_VBSize ) );
	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleListDTLD3D );

PrimitiveTriangleListDTLD3D::PrimitiveTriangleListDTLD3D() : m_pTriangles( NULL ), m_TriangleCount( 0 )
{}

bool PrimitiveTriangleListDTLD3D::execute()
{
	if ( m_TriangleCount > 0 )
	{
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEXTL, m_TriangleCount * 3, m_pTriangles );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLELIST, vbOffset, m_TriangleCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nTrianglesRendered += m_TriangleCount;
	}
	return true;
}

void PrimitiveTriangleListDTLD3D::clear()
{}

void PrimitiveTriangleListDTLD3D::release()
{
	delete [] m_pTriangles;
	m_pTriangles = NULL;

	m_TriangleCount = 0;
}

bool PrimitiveTriangleListDTLD3D::initialize( int triangleCount )
{
	release();

	m_pTriangles = new TriangleTL[ triangleCount ];
	m_TriangleCount = triangleCount;
	return true;
}

bool PrimitiveTriangleListDTLD3D::initialize( int triangleCount, const TriangleTL * pTriangles )
{
	initialize( triangleCount );
	memcpy( m_pTriangles, pTriangles, sizeof(TriangleTL) * triangleCount );
	return true;
}

TriangleTL * PrimitiveTriangleListDTLD3D::lock()
{
	return m_pTriangles;
}

void PrimitiveTriangleListDTLD3D::unlock()
{}

//------------------------------------------------------------------------------------
// EOF


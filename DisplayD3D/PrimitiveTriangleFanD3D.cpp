/*
	PrimitiveTriangleFanD3D.cpp
	(c)2005 Palestar, Richard Lyle
*/
 
#include "DisplayD3D/PrimitiveTriangleFanD3D.h"
#include "DisplayD3D/PrimitiveFactory.h"

//------------------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleFanD3D );

PrimitiveTriangleFanD3D::PrimitiveTriangleFanD3D() : m_VB( NULL )
{}

bool PrimitiveTriangleFanD3D::execute()
{
	int triangleCount = m_Verts - 2;
	if ( triangleCount > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(Vertex) ) != D3D_OK )
			return false;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, triangleCount ) != D3D_OK )
			return false;
		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}
	return true;
}

void PrimitiveTriangleFanD3D::clear()
{}

void PrimitiveTriangleFanD3D::release()
{
	RELEASEDD( m_VB );
}

bool PrimitiveTriangleFanD3D::initialize( int vertexCount, const Vertex * pVerticies )
{
	RELEASEDD( m_VB );

	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	m_Verts = vertexCount;

	dword vbSize = m_Verts * sizeof(Vertex);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;
	
	byte * pBuffer;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;
	memcpy( pBuffer, pVerticies, vbSize );
	m_VB->Unlock();

	TRACE( CharString().format("PrimitiveTriangleFanD3D::initialize, m_Verts = %d", m_Verts ) );
	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleFanDD3D );

PrimitiveTriangleFanDD3D::PrimitiveTriangleFanDD3D() : m_pVerts( NULL ), m_VertexCount( 0 )
{}

bool PrimitiveTriangleFanDD3D::execute()
{
	int triangleCount = m_VertexCount - 2;
	if ( triangleCount > 0 )
	{
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEX, m_VertexCount, m_pVerts );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLEFAN, vbOffset, triangleCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}
	return true;
}

void PrimitiveTriangleFanDD3D::clear()
{}

void PrimitiveTriangleFanDD3D::release()
{
	delete [] m_pVerts;
	m_pVerts = NULL;
	m_VertexCount = 0;
}

bool PrimitiveTriangleFanDD3D::initialize( int vertexCount )
{
	release();

	m_VertexCount = vertexCount;
	m_pVerts = new Vertex[ vertexCount ];
	return true;
}

bool PrimitiveTriangleFanDD3D::initialize( int vertexCount, const Vertex * pVerticies )
{
	initialize( vertexCount );
	memcpy( m_pVerts, pVerticies, sizeof(Vertex) * vertexCount );
	return true;
}

Vertex * PrimitiveTriangleFanDD3D::lock()
{
	return m_pVerts;
}

void PrimitiveTriangleFanDD3D::unlock()
{}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleFanLD3D );

PrimitiveTriangleFanLD3D::PrimitiveTriangleFanLD3D() : m_VB( NULL )
{}

bool PrimitiveTriangleFanLD3D::execute()
{
	int triangleCount = m_Verts - 2;
	if ( triangleCount > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(VertexL) ) != D3D_OK )
			return false;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, triangleCount ) != D3D_OK )
			return false;
		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}
	return true;
}

void PrimitiveTriangleFanLD3D::clear()
{}

void PrimitiveTriangleFanLD3D::release()
{
	RELEASEDD( m_VB );
}

bool PrimitiveTriangleFanLD3D::initialize( int vertexCount, const VertexL * pVerticies )
{
	RELEASEDD( m_VB );

	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	m_Verts = vertexCount;
	dword vbSize = m_Verts * sizeof(VertexL);

	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1, 
		D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;
	
	byte * pBuffer;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;
	memcpy( pBuffer, pVerticies, vbSize );
	m_VB->Unlock();

	TRACE( CharString().format("PrimitiveTriangleFanLD3D::initialize, m_Verts = %d", m_Verts ) );
	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleFanDLD3D );

PrimitiveTriangleFanDLD3D::PrimitiveTriangleFanDLD3D() : m_VertexCount( 0 ), m_pVerts( NULL )
{}

bool PrimitiveTriangleFanDLD3D::execute()
{
	int triangleCount = m_VertexCount - 2;
	if ( triangleCount > 0 )
	{
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEXL, m_VertexCount, m_pVerts );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLEFAN, vbOffset, triangleCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}
	return true;
}

void PrimitiveTriangleFanDLD3D::clear()
{}

void PrimitiveTriangleFanDLD3D::release()
{
	delete [] m_pVerts;
	m_pVerts = NULL;
	m_VertexCount = 0;
}

bool PrimitiveTriangleFanDLD3D::initialize( int vertexCount )
{
	release();

	m_pVerts = new VertexL[ vertexCount ];
	m_VertexCount = vertexCount;
	return true;
}

bool PrimitiveTriangleFanDLD3D::initialize( int vertexCount, const VertexL * pVerticies )
{
	initialize( vertexCount );
	memcpy( m_pVerts, pVerticies, sizeof(VertexL) * vertexCount );
	return true;
}

VertexL * PrimitiveTriangleFanDLD3D::lock()
{
	return m_pVerts;
}

void PrimitiveTriangleFanDLD3D::unlock()
{}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleFanDTLD3D );

PrimitiveTriangleFanDTLD3D::PrimitiveTriangleFanDTLD3D() : m_VertexCount( 0 ), m_pVerts( NULL )
{}

bool PrimitiveTriangleFanDTLD3D::execute()
{
	int triangleCount = m_VertexCount - 2;
	if ( triangleCount > 0 )
	{
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEXTL, m_VertexCount, m_pVerts );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLEFAN, vbOffset, triangleCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}

	return true;
}

void PrimitiveTriangleFanDTLD3D::clear()
{}

void PrimitiveTriangleFanDTLD3D::release()
{
	delete [] m_pVerts;
	m_pVerts = NULL;
	m_VertexCount = 0;
}

bool PrimitiveTriangleFanDTLD3D::initialize( int vertexCount )
{
	release();

	m_pVerts = new VertexTL[ vertexCount ];
	m_VertexCount = vertexCount;
	return true;
}

bool PrimitiveTriangleFanDTLD3D::initialize( int vertexCount, const VertexTL * pVerticies )
{
	initialize( vertexCount );
	memcpy( m_pVerts, pVerticies, sizeof(VertexTL) * vertexCount );
	return true;
}

VertexTL * PrimitiveTriangleFanDTLD3D::lock()
{
	return m_pVerts;
}

void PrimitiveTriangleFanDTLD3D::unlock()
{}

//------------------------------------------------------------------------------------
// EOF


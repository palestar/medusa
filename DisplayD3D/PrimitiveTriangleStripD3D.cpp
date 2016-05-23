/*
	PrimitiveTriangleStripD3D.cpp
	(c)2005 Palestar, Richard Lyle
*/
 

// Fix for VS2005
#ifndef POINTER_64
#define POINTER_64		__ptr64
#endif

#include "Debug/Assert.h"
#include "DisplayD3D/PrimitiveTriangleStripD3D.h"
#include "DisplayD3D/PrimitiveFactory.h"

//------------------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleStripD3D );

PrimitiveTriangleStripD3D::PrimitiveTriangleStripD3D() : m_VB( NULL )
{}

bool PrimitiveTriangleStripD3D::execute()
{
	int triangleCount = m_Verts - 2;
	if ( triangleCount > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(Vertex) ) != D3D_OK )
			return false;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, triangleCount ) != D3D_OK )
			return false;
		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}
	return true;
}

void PrimitiveTriangleStripD3D::clear()
{}

void PrimitiveTriangleStripD3D::release()
{
	RELEASEDD( m_VB );
}

bool PrimitiveTriangleStripD3D::initialize( int vertexCount, const Vertex * pVerticies )
{
	RELEASEDD( m_VB );

	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	m_Verts = vertexCount;

	dword vbSize = m_Verts * sizeof(Vertex);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, D3DPOOL_MANAGED, 
		&m_VB, NULL ) != D3D_OK )
		return false;
	
	byte * pBuffer;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;
	memcpy( pBuffer, pVerticies, vbSize );
	m_VB->Unlock();

	TRACE( CharString().format("PrimitiveTriangleStripD3D::initialize, m_Verts = %d", m_Verts ) );
	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleStripLD3D );

PrimitiveTriangleStripLD3D::PrimitiveTriangleStripLD3D() : m_VB( NULL )
{}

bool PrimitiveTriangleStripLD3D::execute()
{
	int triangleCount = m_Verts - 2;
	if ( triangleCount > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(VertexL) ) != D3D_OK )
			return false;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, triangleCount ) != D3D_OK )
			return false;
		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}
	return true;
}

void PrimitiveTriangleStripLD3D::clear()
{}

void PrimitiveTriangleStripLD3D::release()
{
	RELEASEDD( m_VB );
}

bool PrimitiveTriangleStripLD3D::initialize( int vertexCount, const VertexL * pVerticies )
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

	TRACE( CharString().format("PrimitiveTriangleStripLD3D::initialize, m_Verts = %d", m_Verts ) );
	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleStripDLD3D );

PrimitiveTriangleStripDLD3D::PrimitiveTriangleStripDLD3D() : m_pVerts( NULL ), m_VertexCount( 0 )
{}

bool PrimitiveTriangleStripDLD3D::execute()
{
	int triangleCount = m_VertexCount - 2;
	if ( triangleCount > 0 )
	{
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEXL, m_VertexCount, m_pVerts );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLESTRIP, vbOffset, triangleCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}
	return true;
}

void PrimitiveTriangleStripDLD3D::clear()
{}

void PrimitiveTriangleStripDLD3D::release()
{
	delete [] m_pVerts;
	m_pVerts = NULL;

	m_VertexCount = 0;
}

bool PrimitiveTriangleStripDLD3D::initialize( int vertexCount )
{
	release();

	m_pVerts = new VertexL[ vertexCount ];
	m_VertexCount = vertexCount;
	return true;
}

bool PrimitiveTriangleStripDLD3D::initialize( int vertexCount, const VertexL * pVerticies )
{
	initialize( vertexCount );
	memcpy( m_pVerts, pVerticies, sizeof(VertexL) * vertexCount );
	return true;
}

VertexL * PrimitiveTriangleStripDLD3D::lock()
{
	return m_pVerts;
}

void PrimitiveTriangleStripDLD3D::unlock()
{}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveTriangleStripDTLD3D );

PrimitiveTriangleStripDTLD3D::PrimitiveTriangleStripDTLD3D() : m_pVerts( NULL ), m_VertexCount( 0 )
{}

bool PrimitiveTriangleStripDTLD3D::execute()
{
	int triangleCount = m_VertexCount - 2;
	if ( triangleCount > 0 )
	{
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEXTL, m_VertexCount, m_pVerts );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_TRIANGLESTRIP, vbOffset, triangleCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nTrianglesRendered += triangleCount;
	}
	return true;
}

void PrimitiveTriangleStripDTLD3D::clear()
{}

void PrimitiveTriangleStripDTLD3D::release()
{
	delete [] m_pVerts;
	m_pVerts = NULL;

	m_VertexCount = 0;
}

bool PrimitiveTriangleStripDTLD3D::initialize( int vertexCount )
{
	release();

	m_pVerts = new VertexTL[ vertexCount ];
	m_VertexCount = vertexCount;
	return true;
}

bool PrimitiveTriangleStripDTLD3D::initialize( int vertexCount, const VertexTL * pVerticies )
{
	initialize( vertexCount );
	memcpy( m_pVerts, pVerticies, sizeof(VertexTL) * vertexCount );
	return true;
}

VertexTL * PrimitiveTriangleStripDTLD3D::lock()
{
	return m_pVerts;
}

void PrimitiveTriangleStripDTLD3D::unlock()
{}

//------------------------------------------------------------------------------------
// EOF


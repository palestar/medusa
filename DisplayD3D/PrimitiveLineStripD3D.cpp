/*
	PrimitiveLineStripD3D.cpp
	(c)2005 Palestar, Richard Lyle
*/
 
#include "Debug/Assert.h"
#include "DisplayD3D/PrimitiveLineStripD3D.h"
#include "DisplayD3D/PrimitiveFactory.h"

//------------------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveLineStripD3D );

PrimitiveLineStripD3D::PrimitiveLineStripD3D() :  m_VB( NULL )
{}

bool PrimitiveLineStripD3D::execute()
{
	int lineCount = m_Verts - 1;
	if ( lineCount > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(Vertex) ) != D3D_OK )
			return false;
		if ( d3dd->DrawPrimitive( D3DPT_LINESTRIP, 0, lineCount ) != D3D_OK )
			return false;
		DisplayDevice::sm_nLinesRendered += lineCount;
	}

	return true;
}

void PrimitiveLineStripD3D::clear()
{}

void PrimitiveLineStripD3D::release()
{
	RELEASEDD( m_VB );
}

bool PrimitiveLineStripD3D::initialize( int vertexCount, const Vertex * pVerticies )
{
	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	m_Verts = vertexCount;
	// release any previous vertex buffer
	RELEASEDD( m_VB );

	dword vbSize = m_Verts * sizeof(Vertex);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;

	byte * pBuffer = NULL;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;

	memcpy( pBuffer, pVerticies, sizeof(Vertex) * vertexCount );		
	m_VB->Unlock();
	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveLineStripLD3D );

PrimitiveLineStripLD3D::PrimitiveLineStripLD3D() : m_VB( NULL )
{}

bool PrimitiveLineStripLD3D::execute()
{
	int lineCount = m_Verts - 1;
	if ( lineCount > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(VertexL) ) != D3D_OK )
			return false;
		if ( d3dd->DrawPrimitive( D3DPT_LINESTRIP, 0, lineCount ) != D3D_OK )
			return false;
		DisplayDevice::sm_nLinesRendered += lineCount;
	}

	return true;
}

void PrimitiveLineStripLD3D::clear()
{}

void PrimitiveLineStripLD3D::release()
{
	RELEASEDD( m_VB );
}

bool PrimitiveLineStripLD3D::initialize( int vertexCount, const VertexL * pVerticies )
{
	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	m_Verts = vertexCount;
	// release any previous vertex buffer
	RELEASEDD( m_VB );

	dword vbSize = m_Verts * sizeof(VertexL);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | 
		D3DFVF_DIFFUSE | D3DFVF_TEX1, D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;
	
	byte * pBuffer = NULL;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;

	// copy in the new veritices
	memcpy( pBuffer, pVerticies, sizeof(VertexL) * vertexCount );	
	m_VB->Unlock();
	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveLineStripDLD3D );

PrimitiveLineStripDLD3D::PrimitiveLineStripDLD3D() : m_pVerts( NULL ), m_VertexCount( 0 )
{}

bool PrimitiveLineStripDLD3D::execute()
{
	int lineCount = m_VertexCount - 1;
	if ( lineCount > 0 )
	{
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEXL, m_VertexCount, m_pVerts );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_LINESTRIP, vbOffset, lineCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nLinesRendered += lineCount;
	}

	return true;
}

void PrimitiveLineStripDLD3D::clear()
{}

void PrimitiveLineStripDLD3D::release()
{
	delete [] m_pVerts;
	m_pVerts = NULL;

	m_VertexCount = 0;
}

bool PrimitiveLineStripDLD3D::initialize( int vertexCount )
{
	release();

	m_VertexCount = vertexCount;
	m_pVerts = new VertexL[ vertexCount ];
	return true;
}

bool PrimitiveLineStripDLD3D::initialize( int vertexCount, const VertexL * pVerticies )
{
	// allocate the buffer
	initialize( vertexCount );
	// copy in the new veritices
	memcpy( m_pVerts, pVerticies, sizeof(VertexL) * vertexCount );	
	return true;
}

VertexL * PrimitiveLineStripDLD3D::lock()
{
	return m_pVerts;
}

void PrimitiveLineStripDLD3D::unlock()
{}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveLineStripTLD3D );

PrimitiveLineStripTLD3D::PrimitiveLineStripTLD3D() : m_VB( NULL )
{}

bool PrimitiveLineStripTLD3D::execute()
{
	int lineCount = m_Verts - 1;
	if ( lineCount > 0 )
	{
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 ) != D3D_OK )
			return false;
		if ( d3dd->SetStreamSource( 0, m_VB, 0, sizeof(VertexTL) ) != D3D_OK )
			return false;
		if ( d3dd->DrawPrimitive( D3DPT_LINESTRIP, 0, lineCount ) != D3D_OK )
			return false;
		DisplayDevice::sm_nLinesRendered += lineCount;
	}

	return true;
}

void PrimitiveLineStripTLD3D::clear()
{}

void PrimitiveLineStripTLD3D::release()
{
	RELEASEDD( m_VB );
}

bool PrimitiveLineStripTLD3D::initialize( int vertexCount, const VertexTL * pVerticies )
{
	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;

	m_Verts = vertexCount;
	// release any previous vertex buffer
	RELEASEDD( m_VB );

	dword vbSize = m_Verts * sizeof(VertexTL);
	if ( d3dd->CreateVertexBuffer( vbSize, D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, 
		D3DPOOL_MANAGED, &m_VB, NULL ) != D3D_OK )
		return false;

	byte * pBuffer = NULL;
	if ( m_VB->Lock( 0, vbSize, (void **)&pBuffer, 0 ) != D3D_OK )
		return false;

	// copy in the new veritices
	memcpy( pBuffer, pVerticies, sizeof(VertexTL) * vertexCount );	
	m_VB->Unlock();
	return true;
}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveLineStripDTLD3D );

PrimitiveLineStripDTLD3D::PrimitiveLineStripDTLD3D() : m_pVerts( NULL ), m_VertexCount( 0 )
{}

bool PrimitiveLineStripDTLD3D::execute()
{
	int lineCount = m_VertexCount - 1;
	if ( lineCount > 0 )
	{
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEXTL, m_VertexCount, m_pVerts );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_LINESTRIP, vbOffset, lineCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nLinesRendered += lineCount;
	}

	return true;
}

void PrimitiveLineStripDTLD3D::clear()
{}

void PrimitiveLineStripDTLD3D::release()
{
	delete [] m_pVerts;
	m_pVerts = NULL;

	m_VertexCount = 0;
}

bool PrimitiveLineStripDTLD3D::initialize( int vertexCount )
{
	release();

	m_VertexCount = vertexCount;
	m_pVerts = new VertexTL[ vertexCount ];
	return true;
}

bool PrimitiveLineStripDTLD3D::initialize( int vertexCount, const VertexTL * pVerticies )
{
	initialize( vertexCount );
	memcpy( m_pVerts, pVerticies, sizeof(VertexTL) * vertexCount );	
	return true;
}

VertexTL * PrimitiveLineStripDTLD3D::lock()
{
	return m_pVerts;
}

void PrimitiveLineStripDTLD3D::unlock()
{}

//------------------------------------------------------------------------------------
// EOF


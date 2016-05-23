/*
	PrimitiveWindowD3D.cpp
	(c)2005 Palestar, Richard Lyle
*/
 
#include "DisplayD3D/PrimitiveWindowD3D.h"
#include "DisplayD3D/PrimitiveFactory.h"

//------------------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveWindowD3D );

PrimitiveWindowD3D::PrimitiveWindowD3D()
{}

//----------------------------------------------------------------------------

bool PrimitiveWindowD3D::execute()
{
	int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
		DisplayDeviceD3D::VB_VERTEXTL, 4, m_Quad );
	if ( vbOffset < 0 )
		return false;

	LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
	if ( d3dd->DrawPrimitive( D3DPT_TRIANGLEFAN, vbOffset, 2 ) != D3D_OK )
		return false;

	DisplayDevice::sm_nTrianglesRendered += 2;
	return true;
}

void PrimitiveWindowD3D::clear()
{}

void PrimitiveWindowD3D::release()
{}

//----------------------------------------------------------------------------

#undef RGB

bool PrimitiveWindowD3D::initialize( const RectInt & window, const RectFloat & uv, Color diffuse )
{
	// initialize the D3DTLVERTEX array
	m_Quad[ 0 ].position.x = window.left;		m_Quad[ 0 ].position.y = window.top;
	m_Quad[ 1 ].position.x = window.right;		m_Quad[ 1 ].position.y = window.top;
	m_Quad[ 2 ].position.x = window.right;		m_Quad[ 2 ].position.y = window.bottom;
	m_Quad[ 3 ].position.x = window.left;		m_Quad[ 3 ].position.y = window.bottom;
	m_Quad[ 0 ].position.z = m_Quad[ 1 ].position.z = m_Quad[ 2 ].position.z = m_Quad[ 3 ].position.z = 0.0f;

	m_Quad[ 0 ].u = uv.left;			m_Quad[ 0 ].v = uv.top;
	m_Quad[ 1 ].u = uv.right;			m_Quad[ 1 ].v = uv.top;
	m_Quad[ 2 ].u = uv.right;			m_Quad[ 2 ].v = uv.bottom;
	m_Quad[ 3 ].u = uv.left;			m_Quad[ 3 ].v = uv.bottom;

	m_Quad[ 0 ].w = m_Quad[ 1 ].w = m_Quad[ 2 ].w = m_Quad[ 3 ].w = 1.0f;
	m_Quad[ 0 ].diffuse = m_Quad[ 1 ].diffuse = m_Quad[ 2 ].diffuse = m_Quad[ 3 ].diffuse = diffuse;
	return true;
}

//------------------------------------------------------------------------------------
// EOF


/*
	PrimitiveLineListD3D.cpp
	(c)2005 Palestar, Richard Lyle
*/
 
#include "DisplayD3D/PrimitiveLineListD3D.h"
#include "DisplayD3D/PrimitiveFactory.h"

//------------------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveLineListD3D );

PrimitiveLineListD3D::PrimitiveLineListD3D() : m_pLines( NULL ), m_LineCount( 0 )
{}

bool PrimitiveLineListD3D::execute()
{
	if ( m_LineCount > 0 )
	{
		// initialize the dynamic vertex buffer
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEX, m_LineCount * 2, m_pLines );
		if ( vbOffset < 0 )
			return false;
		// draw the line strip
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_LINELIST, vbOffset, m_LineCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nLinesRendered += m_LineCount;
	}
	return true;
}

void PrimitiveLineListD3D::clear()
{}

void PrimitiveLineListD3D::release()
{
	delete [] m_pLines;
	m_pLines = NULL;
	m_LineCount = 0;
}

bool PrimitiveLineListD3D::initialize( int lineCount )
{
	release();

	m_LineCount = lineCount;
	m_pLines = new Line[ m_LineCount ];
	return true;
}

bool PrimitiveLineListD3D::initialize( int lineCount, const Line * pLines )
{
	initialize( lineCount );
	memcpy( m_pLines, pLines, sizeof(Line) * lineCount );
	return true;
}

Line * PrimitiveLineListD3D::lock()
{
	return m_pLines;
}

void PrimitiveLineListD3D::unlock()
{}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveLineListLD3D );

PrimitiveLineListLD3D::PrimitiveLineListLD3D() : m_pLines( NULL ), m_LineCount( 0 )
{}

bool PrimitiveLineListLD3D::execute()
{
	if ( m_LineCount > 0 )
	{
		// allocate some space from the dynamic vertex buffer for this line strip
		int vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB( 
			DisplayDeviceD3D::VB_VERTEXL, m_LineCount * 2, m_pLines );
		if ( vbOffset < 0 )
			return false;
		// draw the line strip
		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_LINELIST, vbOffset, m_LineCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nLinesRendered += m_LineCount;
	}

	return true;
}

void PrimitiveLineListLD3D::clear()
{}

void PrimitiveLineListLD3D::release()
{
	delete [] m_pLines;
	m_pLines = NULL;
	m_LineCount = 0;
}

bool PrimitiveLineListLD3D::initialize( int lineCount )
{
	release();

	m_LineCount = lineCount;
	m_pLines = new LineL[ m_LineCount ];
	return true;
}

bool PrimitiveLineListLD3D::initialize( int lineCount, const LineL * pLines )
{
	initialize( lineCount );
	memcpy( m_pLines, pLines, sizeof(LineL) * lineCount );
	return true;
}

LineL * PrimitiveLineListLD3D::lock()
{
	return m_pLines;
}

void PrimitiveLineListLD3D::unlock()
{}

//----------------------------------------------------------------------------

IMPLEMENT_PRIMITIVE_FACTORY( PrimitiveLineListDTLD3D );

PrimitiveLineListDTLD3D::PrimitiveLineListDTLD3D() : m_pLines( NULL ), m_LineCount( 0 )
{}

bool PrimitiveLineListDTLD3D::execute()
{
	if ( m_LineCount > 0 )
	{
		int	vbOffset = ((DisplayDeviceD3D *)m_pDevice)->setupDVB(  
			DisplayDeviceD3D::VB_VERTEXTL, m_LineCount * 2, m_pLines );
		if ( vbOffset < 0 )
			return false;

		LPDIRECT3DDEVICE9 d3dd = ((DisplayDeviceD3D *)m_pDevice)->m_pD3DD;
		if ( d3dd->DrawPrimitive( D3DPT_LINELIST, vbOffset, m_LineCount ) != D3D_OK )
			return false;

		DisplayDevice::sm_nLinesRendered += m_LineCount;
	}

	return true;
}

void PrimitiveLineListDTLD3D::clear()
{}

void PrimitiveLineListDTLD3D::release()
{
	delete [] m_pLines;
	m_pLines = NULL;
	m_LineCount = 0;
}


bool PrimitiveLineListDTLD3D::initialize( int lineCount )
{
	release();

	m_LineCount = lineCount;
	m_pLines = new LineTL[ m_LineCount ];
	return true;
}

bool PrimitiveLineListDTLD3D::initialize( int lineCount, const LineTL * pLines )
{
	// allocate the buffer
	initialize( lineCount );
	// copy the vertex information into our buffer
	memcpy( m_pLines, pLines, sizeof(LineTL) * lineCount );	
	return true;
}

LineTL * PrimitiveLineListDTLD3D::lock()
{
	return m_pLines;
}

void PrimitiveLineListDTLD3D::unlock()
{}

//------------------------------------------------------------------------------------
// EOF


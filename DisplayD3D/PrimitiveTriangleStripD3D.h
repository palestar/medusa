/*
	PrimitiveTriangleStripD3D.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_TRIANGLE_FAN_D3D_H 
#define PRIMITIVE_TRIANGLE_FAN_D3D_H

#include "Display/PrimitiveTriangleStrip.h"
#include "DisplayD3D/DisplayDeviceD3D.h"

//----------------------------------------------------------------------------

class PrimitiveTriangleStripD3D : public PrimitiveTriangleStrip
{
public:
	// Construction
	PrimitiveTriangleStripD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleStrip interface
	bool			initialize( int vertexCount, const Vertex * pVerticies );

private:
	LPDIRECT3DVERTEXBUFFER9
					m_VB;
	int				m_Verts;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleStripLD3D : public PrimitiveTriangleStripL
{
public:
	// Construction
	PrimitiveTriangleStripLD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleStrip interface
	bool			initialize( int vertexCount, const VertexL * pVerticies );

private:
	LPDIRECT3DVERTEXBUFFER9
					m_VB;
	int				m_Verts;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleStripDLD3D : public PrimitiveTriangleStripDL
{
public:
	// Construction
	PrimitiveTriangleStripDLD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleStrip interface
	bool			initialize( int vertexCount );
	bool			initialize( int vertexCount, const VertexL * pVerticies );

	VertexL *		lock();
	void			unlock();

private:
	VertexL *		m_pVerts;
	int				m_VertexCount;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleStripDTLD3D : public PrimitiveTriangleStripDTL
{
public:
	// Construction
	PrimitiveTriangleStripDTLD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleStrip interface
	bool			initialize( int vertexCount );
	bool			initialize( int vertexCount, const VertexTL * pVerticies );

	VertexTL *		lock();
	void			unlock();

private:
	VertexTL *		m_pVerts;
	int				m_VertexCount;
};

//----------------------------------------------------------------------------

#endif

//------------------------------------------------------------------------------------
// EOF

/*
	PrimitiveLineStripD3D.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_LINE_STRIP_D3D_H 
#define PRIMITIVE_LINE_STRIP_D3D_H

#include "Display/PrimitiveLineStrip.h"
#include "DisplayD3D/DisplayDeviceD3D.h"

//----------------------------------------------------------------------------

class PrimitiveLineStripD3D : public PrimitiveLineStrip
{
public:
	// Construction
	PrimitiveLineStripD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveLineStrip interface
	bool			initialize( int vertexCount, const Vertex * pVerticies );

private:
	// Data
	LPDIRECT3DVERTEXBUFFER9
					m_VB;
	int				m_Verts;
};

//----------------------------------------------------------------------------

class PrimitiveLineStripLD3D : public PrimitiveLineStripL
{
public:
	// Construction
	PrimitiveLineStripLD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveLineStrip interface
	bool			initialize( int vertexCount, const VertexL * pVerticies );

private:
	// Data
	LPDIRECT3DVERTEXBUFFER9
					m_VB;
	int				m_Verts;
};

//----------------------------------------------------------------------------

class PrimitiveLineStripDLD3D : public PrimitiveLineStripDL
{
public:
	// Construction
	PrimitiveLineStripDLD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveLineStrip interface
	bool			initialize( int vertexCount );
	bool			initialize( int vertexCount, const VertexL * pVerticies );

	VertexL *		lock();
	void			unlock();

private:
	// Data
	int				m_VertexCount;
	VertexL *		m_pVerts;
};

//----------------------------------------------------------------------------

class PrimitiveLineStripTLD3D : public PrimitiveLineStripTL
{
public:
	// Construction
	PrimitiveLineStripTLD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveLineStrip interface
	bool			initialize( int vertexCount, const VertexTL * pVerticies );

private:
	// Data
	LPDIRECT3DVERTEXBUFFER9
					m_VB;
	int				m_Verts;
};

//----------------------------------------------------------------------------

class PrimitiveLineStripDTLD3D : public PrimitiveLineStripDTL
{
public:
	// Construction
	PrimitiveLineStripDTLD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveLineStrip interface
	bool			initialize( int vertexCount );
	bool			initialize( int vertexCount, const VertexTL * pVerticies );

	VertexTL *		lock();
	void			unlock();

private:
	// Data
	int				m_VertexCount;
	VertexTL *		m_pVerts;
};


//----------------------------------------------------------------------------



#endif

//------------------------------------------------------------------------------------
// EOF

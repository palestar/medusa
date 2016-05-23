/*
	PrimitiveTriangleFanD3D.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_TRIANGLE_FAN_D3D_H 
#define PRIMITIVE_TRIANGLE_FAN_D3D_H

#include "Display/PrimitiveTriangleFan.h"
#include "DisplayD3D/DisplayDeviceD3D.h"



//------------------------------------------------------------------------------------

class PrimitiveTriangleFanD3D : public PrimitiveTriangleFan
{
public:
	// Construction
	PrimitiveTriangleFanD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleFan interface
	bool			initialize( int vertexCount, const Vertex * pVerticies );
private:
	// Data
	LPDIRECT3DVERTEXBUFFER9
					m_VB;
	int				m_Verts;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleFanDD3D : public PrimitiveTriangleFanD
{
public:
	// Construction
	PrimitiveTriangleFanDD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleFan interface
	bool			initialize( int vertexCount );
	bool			initialize( int vertexCount, const Vertex * pVerticies );
	Vertex *		lock();
	void			unlock();

private:
	// Data
	int				m_VertexCount;
	Vertex *		m_pVerts;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleFanLD3D : public PrimitiveTriangleFanL
{
public:
	// Construction
	PrimitiveTriangleFanLD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleFan interface
	bool			initialize( int vertexCount, const VertexL * pVerticies );
private:
	// Data
	LPDIRECT3DVERTEXBUFFER9
					m_VB;
	int				m_Verts;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleFanDLD3D : public PrimitiveTriangleFanDL
{
public:
	// Construction
	PrimitiveTriangleFanDLD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleFan interface
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

class PrimitiveTriangleFanDTLD3D : public PrimitiveTriangleFanDTL
{
public:
	// Construction
	PrimitiveTriangleFanDTLD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleFan interface
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

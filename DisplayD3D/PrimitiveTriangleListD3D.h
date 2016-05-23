/*
	PrimitiveTriangleListD3D.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_TRIANGLE_LIST_D3D_H 
#define PRIMITIVE_TRIANGLE_LIST_D3D_H

#include "Display/PrimitiveTriangleList.h"
#include "DisplayD3D/DisplayDeviceD3D.h"

//------------------------------------------------------------------------------------

class PrimitiveTriangleListD3D : public PrimitiveTriangleList
{
public:
	// Construction
	PrimitiveTriangleListD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleList interface
	bool			initialize( int triangleCount, const Triangle * pTriangles );

private:
	// Data
	int				m_VBSize;
	LPDIRECT3DVERTEXBUFFER9
					m_VB;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleListDD3D : public PrimitiveTriangleListD
{
public:
	// Construction
	PrimitiveTriangleListDD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleList interface
	bool			initialize( int triangleCount );
	bool			initialize( int triangleCount, const Triangle * pTriangles );
	Triangle *		lock();
	void			unlock();

private:
	// Data
	Triangle *		m_pTriangles;
	int				m_TriangleCount;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleListID3D : public PrimitiveTriangleListI
{
public:
	// Construction
	PrimitiveTriangleListID3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleList interface
	bool			initialize( int triangleCount, const Triangle * pTriangles );
	bool			initialize( int triangleCount, const word * pTriangles,
									int vertexCount, const Vertex * pVerticies );
	bool			initialize( int triangleCount, const word * pTriangles,
									PrimitiveTriangleListI * pVerts );	

private:
	// Data
	int				m_Triangles;
	int				m_Verts;

	LPDIRECT3DVERTEXBUFFER9
					m_VB;
	LPDIRECT3DVERTEXBUFFER9
					m_SystemVB;
	LPDIRECT3DINDEXBUFFER9
					m_VBI;

	// static
	bool			createOptimizedVB( LPDIRECT3DDEVICE9 d3dd, int triangleCount, const word * pTriangles );
};

//------------------------------------------------------------------------------------

class PrimitiveTriangleListLD3D : public PrimitiveTriangleListL
{
public:
	// Construction
	PrimitiveTriangleListLD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleList interface
	bool			initialize( int triangleCount, const TriangleL * pTriangles );

private:
	// Data
	int				m_VBSize;
	LPDIRECT3DVERTEXBUFFER9
					m_VB;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleListDLD3D : public PrimitiveTriangleListDL
{
public:
	// Construction
	PrimitiveTriangleListDLD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleList interface
	bool			initialize( int triangleCount );
	bool			initialize( int triangleCount, const TriangleL * pTriangles );
	TriangleL *		lock();
	void			unlock();

private:
	// Data
	TriangleL *		m_pTriangles;
	int				m_TriangleCount;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleListTLD3D : public PrimitiveTriangleListTL
{
public:
	// Construction
	PrimitiveTriangleListTLD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleList interface
	bool			initialize( int triangleCount, const TriangleTL * pTriangles );

private:
	// Data
	int				m_VBSize;
	LPDIRECT3DVERTEXBUFFER9
					m_VB;
};

//----------------------------------------------------------------------------

class PrimitiveTriangleListDTLD3D : public PrimitiveTriangleListDTL
{
public:
	// Construction
	PrimitiveTriangleListDTLD3D();

	// DevicePrimitive Interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveTriangleList interface
	bool			initialize( int triangleCount );
	bool			initialize( int triangleCount, const TriangleTL * pTriangles );
	TriangleTL *	lock();
	void			unlock();

private:
	// Data
	TriangleTL *	m_pTriangles;
	int				m_TriangleCount;
};

//----------------------------------------------------------------------------



#endif

//------------------------------------------------------------------------------------
// EOF

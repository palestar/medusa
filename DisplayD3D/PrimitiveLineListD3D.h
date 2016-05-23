/*
	PrimitiveLineListD3D.h
	(c)2005 Palestar, Richard Lyle
*/

#ifndef PRIMITIVE_LINE_LIST_D3D_H 
#define PRIMITIVE_LINE_LIST_D3D_H

#include "Display/PrimitiveLineList.h"
#include "DisplayD3D/DisplayDeviceD3D.h"

//------------------------------------------------------------------------------------

class PrimitiveLineListD3D : public PrimitiveLineList
{
public:
	// Construction
	PrimitiveLineListD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveLineList interface
	bool			initialize( int lineCount );
	bool			initialize( int lineCount, const Line * pLines );

	Line *			lock();
	void			unlock();

private:
	// Data
	int				m_LineCount;
	Line *			m_pLines;
};

//----------------------------------------------------------------------------

class PrimitiveLineListLD3D : public PrimitiveLineListL
{
public:
	// Construction
	PrimitiveLineListLD3D();

	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveLineList interface
	bool			initialize( int lineCount );
	bool			initialize( int lineCount, const LineL * pLines );

	LineL *			lock();
	void			unlock();

private:
	// Data
	int				m_LineCount;
	LineL *			m_pLines;
};

//----------------------------------------------------------------------------

class PrimitiveLineListDTLD3D : public PrimitiveLineListDTL
{
public:
	// Construction
	PrimitiveLineListDTLD3D();
	
	// DevicePrimitive interface
	bool			execute();
	void			clear();
	void			release();

	// PrimitiveLineList interface
	bool			initialize( int lineCount );
	bool			initialize( int lineCount, const LineTL * pLines );

	LineTL *		lock();
	void			unlock();

private:
	// Data
	int				m_LineCount;
	LineTL *		m_pLines;
};


//----------------------------------------------------------------------------



#endif

//------------------------------------------------------------------------------------
// EOF

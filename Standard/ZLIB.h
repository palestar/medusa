/*
	ZLIB.h

	C++ wrapper for the zlib
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef ZLIB_H
#define ZLIB_H

#include "Standard/Types.h"
#include "Standard/Codec.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL ZLIB : public Codec
{
public:
	// Types
	enum Levels {
		DEFAULT					= -1,
		BEST_COMPRESSION		= 9,
		BEST_SPEED				= 1,
		NO_COMPRESSION			= 0,
	};

	// Construction
	ZLIB();
	~ZLIB();

	// Codec interface
	virtual Codec *	clone();
	virtual int		encode( const void * pInput, int nInputBytes, 
						void * pOutput, int nOutputBytes, EncodeLevel nLevel );
	virtual int		decode( const void * pInput, int nInputBytes, 
						void * pOutput, int nOutputBytes );
	virtual void	reset();

private:
	// Data
	void *			m_pCompressor;
	void *			m_pExpander;
};

//----------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
//EOF

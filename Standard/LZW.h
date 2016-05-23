/*
	LZW.h

	This class is used to compress / uncompress data, LZW implementation
	(c)2005 Palestar, Richard Lyle
*/

#ifndef LZW_H
#define LZW_H

#include "Standard/Types.h"
#include "Standard/Codec.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL LZW : public Codec
{
public:
	// Codec interface
	virtual Codec *	clone();
	virtual int		encode( const void * pInput, int nInputBytes, 
						void * pOutput, int nOutputBytes, EncodeLevel nLevel );
	virtual int		decode( const void * pInput, int nInputBytes, 
						void * pOutput, int nOutputBytes );
	virtual void	reset();

private:
	// Constants
	enum 
	{
		BITS			= 15,
		MAX_CODE		= (1 << BITS) - 1,
		TABLE_SIZE		= 35023,
		TABLE_BANKS		= (TABLE_SIZE >> 8) + 1,
		
		END_OF_STREAM	= 256,
		BUMP_CODE		= 257,
		FLUSH_CODE		= 258,
		FIRST_CODE		= 259,
		UNUSED			= -1,
	};

	// Types
	struct Dictionary
	{
		int		codeValue;
		int		parentCode;
		char	character;
	};

	// Data
	Dictionary		m_Dict[ TABLE_BANKS ][ 256 ];
	char			m_DecodeStack[ TABLE_SIZE ];

	unsigned int	m_NextCode;
	int				m_CurrentCodeBits;
	unsigned int	m_NextBumpCode;

	// Mutators
	Dictionary &	hash( int i );

	void			initializeDictionary();
	unsigned int	findChildNode( int parentCode, int childCharacter );
	unsigned int	decodeString( unsigned int count, unsigned int code );
};

//----------------------------------------------------------------------------

inline LZW::Dictionary & LZW::hash( int i )
{
	return( m_Dict[ i >> 8 ][ i & 0xff ] );
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
// EOF



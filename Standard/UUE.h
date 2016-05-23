/*
	UUE.h

	UUE(ncode)/UUD(ecode) classes. 
	These class are used to convert binary data to ascii text or convert uuencoded ascii text to binary data

	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef UUE_H
#define UUE_H

#include "Standard/Types.h"
#include "Standard/String.h"
#include "MedusaDll.h"

//----------------------------------------------------------------------------

class DLL UUE		// encode binary to ascii
{
public:
	// Construction
	UUE();
	UUE( const UUE & copy );
	UUE( const byte * pEncode, int bytes );
	virtual ~UUE();

	// Accessors
	const char *	encoded() const;
	// Mutators
	void			encode( const byte * pEncode, int bytes );
	UUE &			operator=( const UUE & copy );

private:
	// Data
	char *			m_pEncoded;
};	

//----------------------------------------------------------------------------

class DLL UUD		// decode ascii to binary
{
public:
	// Exceptions
	class BadEncoding {};

	// Construction
	UUD();
	UUD( const UUD & copy );
	UUD( const char * pEncoded );	// throws BadEncoding()
	virtual	~UUD();

	// Accessors
	const byte *	decoded() const;
	int				decodedBytes() const;
	// Mutators
	void			release();
	bool			decode( const char * pEncoded );
	UUD &			operator=( const UUD & copy );

private:
	// Data
	byte *			m_pDecoded;
	int				m_DecodedBytes;
};

//----------------------------------------------------------------------------

inline const char * UUE::encoded() const
{
	return m_pEncoded;
}

inline const byte * UUD::decoded() const
{
	return m_pDecoded;
}

inline int UUD::decodedBytes() const
{
	return m_DecodedBytes;
}

//----------------------------------------------------------------------------



#endif

//----------------------------------------------------------------------------
//EOF

/*
	Binary.h

	This class is used to contain a block of data
	(c)2000 Palestar Inc, Richard Lyle
*/

#ifndef BINARY_H
#define BINARY_H

#include "File/Stream.h"
#include "GCQDll.h"

//----------------------------------------------------------------------------

class DLL Binary
{
public:
	// Construction
	Binary();
	Binary( byte * pData, int iDataSize );
	virtual		~Binary();

	// Accessors
	byte *		data() const;
	int			size() const;

	// Mutators
	void		initialize( const byte * pData, int iDataSize );
	void		release();

	Binary &	operator=( const Binary & copy );

private:
	byte *		m_pData;
	int			m_DataSize;
};

//----------------------------------------------------------------------------

inline Binary::Binary() : m_pData( NULL ), m_DataSize( 0 )
{}

inline Binary::Binary( byte * pData, int iDataSize ) : m_pData( NULL ), m_DataSize( 0 )
{
	initialize( pData, iDataSize );
}

inline Binary::~Binary()
{
	release();
}

//----------------------------------------------------------------------------

inline byte * Binary::data() const
{
	return m_pData;
}

inline int Binary::size() const
{
	return m_DataSize;
}

//----------------------------------------------------------------------------

inline void Binary::initialize( const byte * pData, int iDataSize )
{
	release();

	if ( iDataSize > 0 )
	{
		m_DataSize = iDataSize;
		m_pData = new byte[ m_DataSize ];

		if ( pData != NULL )
			memcpy( m_pData, pData, m_DataSize );
	}
}

inline void Binary::release()
{
	if ( m_pData != NULL )
	{
		delete [] m_pData;
		m_pData = NULL;
	}
	m_DataSize = 0;
}

//----------------------------------------------------------------------------

inline Binary & Binary::operator=( const Binary & copy )
{
	initialize( copy.m_pData, copy.m_DataSize );
	return *this;
}

inline const InStream & operator>>( const InStream & input, Binary & bin )
{
	int size;
	input >> size;

	if ( size > 0 )
	{
		bin.initialize( NULL, size );
		input.read( bin.data(), bin.size() );
	}

	return input;
}

inline const OutStream & operator<<( const OutStream & output, const Binary & bin )
{
	output << bin.size();
	if ( bin.size() > 0 )
		output.write( bin.data(), bin.size() );
	return output;
}



#endif

//----------------------------------------------------------------------------
//EOF

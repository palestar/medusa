/*
	FileFIFO.cpp
	(c)2005 Palestar Inc, Richard Lyle
*/

#include "File/FileFIFO.h"

//----------------------------------------------------------------------------

FileFIFO::FileFIFO()
{}

FileFIFO::FileFIFO( const FileFIFO & copy ) : m_Buffer( copy.m_Buffer )
{}

FileFIFO::FileFIFO( const FIFOBuffer & fifo ) : m_Buffer( fifo )
{}

FileFIFO::~FileFIFO()
{}

//----------------------------------------------------------------------------

File * FileFIFO::clone() const
{
	return new FileFIFO( *this );
}

bool FileFIFO::isReadable() const
{
	return true;
}

bool FileFIFO::isWritable() const
{
	return true;
}

File::Position FileFIFO::position() const
{
	return 0;
}

File::Size FileFIFO::size() const
{
	return m_Buffer.size();
}

File::Size FileFIFO::read( void * pBuffer, Size nBytes )
{
	return m_Buffer.pop( pBuffer, nBytes );
}

File::Size FileFIFO::write( const void * pBuffer, Size nBytes )
{
	return m_Buffer.push( pBuffer, nBytes );
}

void FileFIFO::flush()
{}

File::Position FileFIFO::setPosition( Position nPos )
{
	return 0;
}

File::Size FileFIFO::setSize( Size nSize )
{
	return 0;
}

File::Size FileFIFO::setEOF()
{
	return 0;
}

void FileFIFO::lock()
{}

void FileFIFO::unlock()
{}

//----------------------------------------------------------------------------
//EOF

/*
	FileArchive.cpp
	(c)2005 Palestar, Richard Lyle
*/

#include "Debug/Assert.h"
#include "File/FileArchive.h"

#pragma warning(disable:4244 ) // d:\projects\hydra\file\filearchive.cpp(95) : warning C4244: 'argument' : conversion from 'unsigned __int64' to 'unsigned long', possible loss of data

//-------------------------------------------------------------------------------

FileArchive::FileArchive() 
	: m_pArchive( NULL ), m_pHandle( NULL )
{}

FileArchive::FileArchive( const FileArchive & copy ) 
	: File(), m_pArchive( NULL ), m_pHandle( NULL )
{
	if ( copy.m_pArchive && copy.m_pHandle )
	{
		m_pArchive = copy.m_pArchive;
		m_pHandle = m_pArchive->duplicate( copy.m_pHandle );
	}
}

FileArchive::FileArchive( Archive * pArchive, const WidgetKey & file ) 
	: m_pArchive( NULL ), m_pHandle( NULL )
{
	if ( !open( pArchive, file ) )
		throw FileError();
}

FileArchive::~FileArchive()
{
	close();
}

//----------------------------------------------------------------------------

File * FileArchive::clone() const
{
	return( new FileArchive( *this ) );
}

bool FileArchive::isReadable() const
{
	return( true );
}

bool FileArchive::isWritable() const
{
	ASSERT( m_pArchive );
	return( m_pArchive->access() == Archive::READ_WRITE );
}

File::Position FileArchive::position() const
{
	return( m_pHandle->position );
}

File::Size FileArchive::size() const
{
	return( m_pHandle->size );
}

File::Size FileArchive::read( void * pBuffer, Size bytes )
{
	if ( m_pArchive != NULL )
		return m_pArchive->read( m_pHandle, pBuffer, bytes );
	return 0;
}

File::Size FileArchive::write( const void * pBuffer, Size bytes )
{
	if ( m_pArchive != NULL )
		return m_pArchive->write( m_pHandle, pBuffer, bytes );
	return 0;
}

void FileArchive::flush()
{}

File::Position FileArchive::setPosition( Position position )
{
	if ( m_pArchive != NULL )
		return m_pArchive->setPosition( m_pHandle, position );
	return 0;
}

File::Size FileArchive::setSize( Size newSize )
{
	if ( m_pArchive != NULL )
	{
		m_pArchive->setPosition( m_pHandle, newSize );
		return m_pArchive->setEOF( m_pHandle );
	}
	return 0;
}

File::Size FileArchive::setEOF()
{
	if ( m_pArchive != NULL )
		return m_pArchive->setEOF( m_pHandle );
	return 0;
}

void FileArchive::lock()
{}

void FileArchive::unlock()
{}

//----------------------------------------------------------------------------

bool FileArchive::open( Archive * pArchive, const WidgetKey & file )
{
	ASSERT( pArchive );

	close();

	m_pArchive = pArchive;
	m_pHandle = m_pArchive->open( file );
	if (! m_pHandle )
		return false;

	return true;
}

void FileArchive::close()
{
	if ( m_pArchive && m_pHandle )
	{
		m_pArchive->close( m_pHandle );
		m_pHandle = NULL;
		m_pArchive = NULL;
	}
}

void FileArchive::setVersion( dword version )
{
	ASSERT( m_pArchive && m_pHandle );
	m_pArchive->setVersion( m_pHandle, version );
}

void FileArchive::setFlags( dword flags )
{
	ASSERT( m_pArchive && m_pHandle );
	m_pArchive->setFlags( m_pHandle, flags );
}

//--------------------------------------------------------------------------
// EOF


/*
	FileMap.cpp

	Memory-Mapped file object
	(c)2005 Palestar, Richard Lyle
*/

#include "File/FileMap.h"

#if defined(_WIN32)
#include <windows.h>
#endif

//-------------------------------------------------------------------------------

FileMap::FileMap()
{
	m_hMapping = 0;
	m_pView = 0;
	m_ViewSize = 0;
	m_ViewPos = 0;
}

FileMap::FileMap( const FileMap & copy ) : File()
{
	if ( ! open( copy.m_sFileName ) )
		throw FileError();
}

FileMap::FileMap( const char * pFileName )
{
	if ( ! open( pFileName ) )
		throw FileError();
}

FileMap::~FileMap()
{
	close();
}

//-------------------------------------------------------------------------------

File * FileMap::clone() const
{
	return new FileMap( *this );
}

bool FileMap::isReadable() const
{
	return true;
}

bool FileMap::isWritable() const
{
	return false;
}

FileMap::Position FileMap::position() const
{
	return m_ViewPos;
}

FileMap::Size FileMap::size() const
{
	return m_ViewSize;
}

//-------------------------------------------------------------------------------

File::Size FileMap::read( void * pBuffer, Size nBytes )
{
	if ( (m_ViewPos + nBytes) > m_ViewSize )
		nBytes = m_ViewSize - m_ViewPos;
	
	if ( nBytes > 0 )
	{
		memcpy( pBuffer, ((byte *)m_pView) + m_ViewPos, (dword)nBytes );
		m_ViewPos += (dword)nBytes;
	}

	return nBytes;
}

File::Size FileMap::write( const void* p_Src, Size num_bytes )
{
	return 0;		// not supported
}

void FileMap::flush()
{}

File::Position FileMap::setPosition( Position pos )
{
	if ( pos >= (Position)m_ViewSize )
		throw FileError();

	m_ViewPos = (dword)pos;
	return m_ViewPos;
}

File::Size FileMap::setSize( Size newSize)
{
	return 0;		// not supported
}

File::Size FileMap::setEOF()
{
	return 0;		// not supported
}

void FileMap::lock()
{}

void FileMap::unlock()
{}

//----------------------------------------------------------------------------

void * FileMap::view() const
{
	return( m_pView );
}

//----------------------------------------------------------------------------

bool FileMap::open( const char * pFileName )
{
	m_sFileName = pFileName;
	if (! (m_hMapping = FileMap::fileMapOpen( m_sFileName )) )
		return false;

	m_pView = FileMap::fileMapPointer( m_hMapping );
	m_ViewSize = FileMap::fileMapSize( m_hMapping );
	m_ViewPos = 0;

	return true;
}

void FileMap::close()
{
	if ( m_hMapping != 0 )
	{
		FileMap::fileMapClose( m_hMapping );

		m_hMapping = 0;
		m_sFileName = "";
		m_pView = 0;
		m_ViewSize = 0;
		m_ViewPos = 0;
	}
}

//---------------------------------------------------------------------------------------------------

struct CORE_FILEMAP
{
	void *	pFile;
	void *	pMap;
	void *	pView;
};

void * FileMap::fileMapOpen( const char * pFileName )
{
#if defined(_WIN32)
	// open the file
	void * pFile = CreateFileA( pFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( pFile == NULL )
		return NULL;		// failed to open file

	void * pMap = CreateFileMapping( pFile, NULL, PAGE_READONLY, 0, 0, NULL );
	void * pView = MapViewOfFile( pMap, FILE_MAP_READ, 0, 0, 0);
	if ( pMap == NULL || pView == NULL )
	{
		CloseHandle( (HANDLE)pFile );
		return NULL;
	}

	CORE_FILEMAP * pFM = new CORE_FILEMAP;
	pFM->pFile = pFile;
	pFM->pMap = pMap;
	pFM->pView = pView;
	return pFM;
#else
	return NULL;
#endif
}

dword FileMap::fileMapSize( void * pMapping )
{
#if defined(_WIN32)
	return GetFileSize( ((CORE_FILEMAP *)pMapping)->pFile, NULL );
#else
	return 0;
#endif
}

void * FileMap::fileMapPointer( void * pMapping )
{
#if defined(_WIN32)
	return ((CORE_FILEMAP *)pMapping)->pView;
#else
	return NULL;
#endif
}

void FileMap::fileMapClose( void * pMapping )
{
#if defined(_WIN32)
	if ( pMapping != NULL )
	{
		UnmapViewOfFile( ((CORE_FILEMAP *)pMapping)->pView );
		CloseHandle( ((CORE_FILEMAP *)pMapping)->pMap );
		CloseHandle( ((CORE_FILEMAP *)pMapping)->pFile );

		delete pMapping;
	}
#endif
}

//-------------------------------------------------------------------------------
// EOF

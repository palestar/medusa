/*
    File.cpp
    Abstract file object 
    (c)2006 Palestar, Richard Lyle
*/

#include "File/File.h"
#include "Debug/Assert.h"

#include <limits.h>

//-----------------------------------------------------------------------------------

File::File()
{}

File::~File()
{}

//-----------------------------------------------------------------------------------

void File::append( const void* p_Src, Size nBytes )
{
    ASSERT(p_Src);

    setPosition(size());
    write( p_Src, nBytes );
}

void File::insert( const void * pSrc, Size nBytes )
{
	if ( pSrc == NULL || nBytes <= 0 )
		return;

    // move all bytes above position up by 'nBytes'
    Position nInsertPos = position();
    if (nInsertPos < size() && 0 < nBytes )
    {
        Size nMoveSize = size() - nInsertPos;
        byte * pMoveBuffer = new byte[ nMoveSize ];

        read(pMoveBuffer, nMoveSize);
        setPosition(nInsertPos + nBytes);

        write(pMoveBuffer, nMoveSize);
        setPosition(nInsertPos);

		delete [] pMoveBuffer;
    }

    write( pSrc, nBytes);
}

void File::remove(Size nBytes)
{
	if ( nBytes <= 0 )
		return;

    Position nRemovePos = position();
    if (nRemovePos < size())
    {
        if (nRemovePos < size() - nBytes)
        {
            // shift trailing bytes above the removed down
            Size nMoveSize = size() - nRemovePos - nBytes;
            byte * pMoveBuffer = new byte[ nMoveSize ];

            setPosition(nRemovePos + nBytes);
            read(pMoveBuffer, nMoveSize);

            setPosition(nRemovePos);
            write(pMoveBuffer, nMoveSize);

            setEOF();
            setPosition(nRemovePos);

			delete [] pMoveBuffer;
        } 
        else
            setEOF();
    }
}

//-----------------------------------------------------------------------------------
// EOF

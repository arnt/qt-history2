#include "filebuffer.h"
#include <stdlib.h>
#include <string.h>

FileBuffer::FileBuffer( const char* data, unsigned int length )
{
    buffSize = length;
    buffer = 0;

    if( data )
	if( buffer = (char*)malloc( length ) )
	    memcpy( buffer, data, length );
}

FileBuffer::~FileBuffer()
{
    if( buffer )
	free( buffer );
}

char* FileBuffer::data()
{
    return buffer;
}

unsigned int FileBuffer::size()
{
    return buffSize;
}
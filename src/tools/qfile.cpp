/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfile.cpp#1 $
**
** Implementation of QFile class
**
** Author  : Haavard Nord
** Created : 930812
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qfile.h"
#if defined(UNIX)
#include <unistd.h>
#endif
#include <limits.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qfile.cpp#1 $";
#endif


QFile::QFile()
{
    fh = 0;
    length = 0;
}

QFile::QFile( const char *fileName )
{
    fn = fileName;
    fh = 0;
    length = 0;
}

QFile::~QFile()
{
    close();					// close file
}


bool QFile::exists( const char *fileName )	// test if file exists
{
    FILE *f = fopen( fileName, "r" );
    if ( f )
	fclose( f );
    return f != 0;
}

bool QFile::file( const char *fileName )	// set file name
{
    fn = fileName;
    return TRUE;
}

bool QFile::remove( const char *fileName )	// remove file
{
    if ( fileName == 0 )
	fileName = fn;
    if ( !fn.isNull() && fn == fileName )
	close();				// close open file
#if defined(CHECK_NULL)
    if ( fileName == 0 )
	warning( "QFile::remove: No file name specified" );
#endif
#if defined(UNIX)
    return unlink( fileName ) == 0;		// unlink more common in UNIX
#else
    return ::remove( fileName ) == 0;		// use standard ANSI remove
#endif
}


#if defined(_OS_MAC_) || defined(_OS_MSDOS_) || defined(_OS_OS2_) || defined(_OS_WINNT_)
#define USE_BINARY_FILEMODE			// non-translated file mode
#endif

bool QFile::open( int mode )			// open file
{
#if defined(USE_BINARY_FILEMODE)
    static char *perms[] = { 0, "rb", "wb", "r+b" };
#else
    static char *perms[] = { 0, "r", "w", "r+" };
#endif

    if ( fh ) {					// file already open
#if defined(CHECK_STATE)
	warning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    if ( fn.isNull() )				// no file name defined
	return FALSE;
    if ( !setMode( mode ) )
	return FALSE;
    fh = 0;
    ptr = length = 0;
    mode = (smode >> 4) & 3;
    if ( mode > 0 && mode <= 3 ) {		// access flags ok
	fh = fopen( (const char *)fn, perms[mode] );
	if ( !fh && (mode == 3) )		// create if new r/w file
#if defined(USE_BINARY_FILEMODE)
	    fh = fopen( (const char *)fn, "w+b" );
#else
	    fh = fopen( (const char *)fn, "w+" );
#endif
	if ( fh ) {				// file was opened/created
	    fseek( fh, 0, SEEK_END );		// set to end of file
	    length = ftell( fh );		// get file size
	    fseek( fh, 0, SEEK_SET );		// set back to start of file
	}
    }
    return fh != 0;
}

bool QFile::open( int m, FILE *f )		// open file, using file handle
{
    if ( !setMode( m ) )
	return FALSE;
    fh = f;
    ptr = 0;
    length = LONG_MAX;				// file might be stdin etc.
    return TRUE;
}

bool QFile::close()				// close file
{
    bool result = TRUE;
    if ( fh )
	result = (fclose( fh ) == 0);
    smode = Stream_Null;			// reset internal state
    ptr = length = 0;
    fh = 0;
    return result;
}

bool QFile::flush()				// flush file
{
    if ( fh )
	return !fflush( fh );
    return TRUE;
}


long QFile::size()				// get file size
{
    return length;
}

long QFile::at()				// get file pointer
{
    return ptr;
}

bool QFile::at( long n )			// set data pointer
{
    if ( fseek( fh, n, SEEK_SET ) == 0 )	// set new file position
	ptr = n;
    else {
#if defined(CHECK_RANGE)
	warning( "QFile::at: Cannot set file position %ld", n );
#endif
	return FALSE;
    }
    return TRUE;
}


QStream& QFile::_read( char *p, uint len )	// read data from file
{
#if defined(CHECK_STATE)
    if ( !fh ) {				// file not open
	warning( "QFile::_read: File not open" );
	return *this;
    }
    if ( !(mode() & Stream_ReadOnly) ) {	// reading not permitted
	warning( "QFile::_read: Read operation illegal in this mode" );
	return *this;
    }
#endif
    if ( fread( p, 1, len, fh ) != len ) {	// read from file
#if defined(CHECK_RANGE)
	warning( "QFile::_read: File read error" );
#endif
	return *this;
    }
    ptr += len;
    return *this;
}

QStream& QFile::_write( const char *p, uint len )// write data to file
{
#if defined(CHECK_STATE)
    if ( !fh ) {				// file not open
	warning( "QFile::_write: File not open" );
	return *this;
    }
    if ( !(mode() & Stream_WriteOnly) ) {	// writing not permitted
	warning( "QFile::_write: Write operation illegal in this mode" );
	return *this;
    }
#endif
    if ( fwrite( p, 1, len, fh ) != len ) {	// write to file
#if defined(CHECK_RANGE)
	warning( "QFile::_write: File write error" );
#endif
	return *this;
    }
    ptr += len;
    if ( ptr > length )				// update file length
	length = ptr;
    return *this;
}


int QFile::getch()				// get next char
{
#if defined(CHECK_STATE)
    if ( !fh ) {				// file not open
	warning( "QFile::getch: File not open" );
	return EOF;
    }
    if ( !(mode() & Stream_ReadOnly) ) {	// reading not permitted
	warning( "QFile::getch: Read operation illegal in this mode" );
	return EOF;
    }
#endif
    ptr++;
    return getc( fh );
}

int QFile::putch( int ch )			// put char
{
#if defined(CHECK_STATE)
    if ( !fh ) {				// file not open
	warning( "QFile::getch: File not open" );
	return EOF;
    }
    if ( !(mode() & Stream_WriteOnly) ) {	// writing not permitted
	warning( "QFile::putch: Write operation illegal in this mode" );
	return EOF;
    }
#endif
    ptr++;
    if ( ptr > length )				// update file length
	length = ptr;
    return putc( ch, fh );
}

int QFile::ungetch( int ch )			// put back char
{
#if defined(CHECK_STATE)
    if ( !fh ) {				// file not open
	warning( "QFile::ungetch: File not open" );
	return EOF;
    }
#endif
    if ( ch != EOF )
	ptr--;
    return ungetc( ch, fh );
}

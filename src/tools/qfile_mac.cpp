/****************************************************************************
** $Id$
**
** Implementation of QFile class
**
** Created : 950628
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qplatformdefs.h"
#include "qfile.h"
#include "qfiledefs_p.h"
#include "qdir.h"
#include "qt_mac.h"

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

const unsigned char * p_str(const char * c, int len=-1); //qglobal.cpp

bool qt_file_access( const QString& fn, int t )
{
	static FSSpec ret;
	const unsigned char *p = p_str(QFile::encodeName(QDir::convertSeparators(fn)));
	if(FSMakeFSSpec(0, 0, p, &ret) != noErr)
        return FALSE;
#ifdef Q_OS_MACX
    if ( fn.isEmpty() )
	    return FALSE;
    return ACCESS( QFile::encodeName(fn), t ) == 0;
#else
    return TRUE;
#endif
}


bool QFile::remove( const QString &fileName )
{
    if ( fileName.isEmpty() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QFile::remove: Empty or null file name" );
#endif
	return FALSE;
    }
    return unlink( QFile::encodeName(QDir::convertSeparators(fileName)) ) == 0;	
}

bool QFile::open( int m )
{
    if ( isOpen() ) {				// file already open
#if defined(QT_CHECK_STATE)
	qWarning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    if ( fn.isNull() ) {			// no file name defined
#if defined(QT_CHECK_NULL)
	qWarning( "QFile::open: No file name specified" );
#endif
	return FALSE;
    }
    init();					// reset params
    setMode( m );
    if ( !(isReadable() || isWritable()) ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QFile::open: File access not specified" );
#endif
	return FALSE;
    }
    bool ok = TRUE;
#if defined(QT_LARGE_FILE_SUPPORT)
    struct stat64 st;
#else
    struct stat st;
#endif
    if ( isRaw() ) {
	int oflags = O_RDONLY;
	if ( isReadable() && isWritable() )
	    oflags = O_RDWR;
	else if ( isWritable() )
	    oflags = O_WRONLY;
	if ( flags() & IO_Append ) {		// append to end of file?
	    if ( flags() & IO_Truncate )
		oflags |= (O_CREAT | O_TRUNC);
	    else
		oflags |= (O_APPEND | O_CREAT);
	    setFlags( flags() | IO_WriteOnly ); // append implies write
	} else if ( isWritable() ) {		// create/trunc if writable
	    if ( flags() & IO_Truncate )
		oflags |= (O_CREAT | O_TRUNC);
	    else
		oflags |= O_CREAT;
	}
#ifdef Q_OS_MAC9
	if ( isTranslated() )
	    oflags |= O_TEXT;
	else
	    oflags |= O_BINARY;
#endif
#if defined(QT_LARGE_FILE_SUPPORT)
	fd = ::open64( QFile::encodeName(QDir::convertSeparators(fn)), oflags, 0666 );
#else
	fd = ::open( QFile::encodeName(QDir::convertSeparators(fn)), oflags, 0666 );
#endif

	if ( fd != -1 ) {			// open successful
#if defined(QT_LARGE_FILE_SUPPORT)
	    ::fstat64( fd, &st ); // get the stat for later usage
#else
	    ::fstat( fd, &st ); // get the stat for later usage
#endif
	} else {
	    ok = FALSE;
	}
    } else {					// buffered file I/O
	QCString perm;
	char perm2[4];
	bool try_create = FALSE;
	if ( flags() & IO_Append ) {		// append to end of file?
	    setFlags( flags() | IO_WriteOnly ); // append implies write
	    perm = isReadable() ? "a+" : "a";
	} else {
	    if ( isReadWrite() ) {
		if ( flags() & IO_Truncate ) {
		    perm = "w+";
		} else {
		    perm = "r+";
		    try_create = TRUE;		// try to create if not exists
		}
	    } else if ( isReadable() ) {
		perm = "r";
	    } else if ( isWritable() ) {
		perm = "w";
	    }
	}
	qstrcpy( perm2, perm );
#ifdef Q_OS_MAC9
	if ( isTranslated() )
	    strcat( perm2, "t" );
	else
	    strcat( perm2, "b" );
#endif
	for (;;) { // At most twice

#if defined(QT_LARGE_FILE_SUPPORT)
	    fh = fopen64( QFile::encodeName(QDir::convertSeparators(fn)), perm2 );
#else
	    fh = fopen( QFile::encodeName(QDir::convertSeparators(fn)), perm2 );
#endif

	    if ( !fh && try_create ) {
		perm2[0] = 'w';			// try "w+" instead of "r+"
		try_create = FALSE;
	    } else {
		break;
	    }
	}
	if ( fh ) {
#if defined(QT_LARGE_FILE_SUPPORT)
	    ::fstat64( fileno(fh), &st ); // get the stat for later usage
#else
	    ::fstat( fileno(fh), &st ); // get the stat for later usage
#endif
	} else {
	    ok = FALSE;
	}
    }
    if ( ok ) {
	setState( IO_Open );
	// on successful open the file stat was got; now test what type
	// of file we have
	if ( (st.st_mode & S_IFMT) != S_IFREG ) {
	    // non-seekable
	    setType( IO_Sequential );
	    length = INT_MAX;
	    ioIndex = (flags() & IO_Append) == 0 ? 0 : length;
	} else {
	    length = (Offset)st.st_size;
	    ioIndex = (flags() & IO_Append) == 0 ? 0 : length;
	    if ( !(flags()&IO_Truncate) && length == 0 && isReadable() ) {
		// try if you can read from it (if you can, it's a sequential
		// device; e.g. a file in the /proc filesystem)
		int c = getch();
		if ( c != -1 ) {
		    ungetch(c);
		    setType( IO_Sequential );
		    length = INT_MAX;
		}
	    }
	}
    } else {
	init();
	if ( errno == EMFILE )			// no more file handles/descrs
	    setStatus( IO_ResourceError );
	else
	    setStatus( IO_OpenError );
    }
    return ok;
}

bool QFile::open( int m, FILE *f )
{
    if ( isOpen() ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    init();
    setMode( m &~IO_Raw );
    setState( IO_Open );
    fh = f;
    ext_f = TRUE;
#if defined(QT_LARGE_FILE_SUPPORT)
    struct stat64 st;
    ::fstat64( fileno(fh), &st );
    ioIndex = (Offset)ftello64( fh );
#else
    struct stat st;
    ::fstat( fileno(fh), &st );
    ioIndex = (Offset)ftell( fh );
#endif
    if ( (st.st_mode & S_IFMT) != S_IFREG || f == stdin ) { //stdin is non seekable
	// non-seekable
	setType( IO_Sequential );
	length = INT_MAX;
    } else {
	length = (Offset)st.st_size;
	if ( !(flags()&IO_Truncate) && length == 0 && isReadable() ) {
	    // try if you can read from it (if you can, it's a sequential
	    // device; e.g. a file in the /proc filesystem)
	    int c = getch();
	    if ( c != -1 ) {
		ungetch(c);
		setType( IO_Sequential );
		length = INT_MAX;
	    }
	}
    }
    return TRUE;
}


bool QFile::open( int m, int f )
{
    if ( isOpen() ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    init();
    setMode( m |IO_Raw );
    setState( IO_Open );
    fd = f;
    ext_f = TRUE;
#if defined(QT_LARGE_FILE_SUPPORT)
    struct stat64 st;
    ::fstat64( fd, &st );
    ioIndex = (Offset)::lseek64(fd, 0, SEEK_CUR);
#else
    struct stat st;
    ::fstat( fd, &st );
    ioIndex = (Offset)::lseek(fd, 0, SEEK_CUR);
#endif
    if ( (st.st_mode & S_IFMT) != S_IFREG || f == 0 ) { // stdin is not seekable...
	// non-seekable
	setType( IO_Sequential );
	length = INT_MAX;
    } else {
	length = (Offset)st.st_size;
	if ( length == 0 && isReadable() ) {
	    // try if you can read from it (if you can, it's a sequential
	    // device; e.g. a file in the /proc filesystem)
	    int c = getch();
	    if ( c != -1 ) {
		ungetch(c);
		setType( IO_Sequential );
		length = INT_MAX;
	    }
	    resetStatus();
	}
    }
    return TRUE;
}


QIODevice::Offset QFile::size() const
{
#if defined(QT_LARGE_FILE_SUPPORT)
    struct stat64 st;
#else
    struct stat st;
#endif
    if ( isOpen() ) {
#if defined(QT_LARGE_FILE_SUPPORT)
	::fstat64( fh ? fileno(fh) : fd, &st );
#else
	::fstat( fh ? fileno(fh) : fd, &st );
#endif
    } else {
#if defined(QT_LARGE_FILE_SUPPORT)
	::stat64( QFile::encodeName(fn), &st );
#else
	::stat( QFile::encodeName(fn), &st );
#endif
    }
    return st.st_size;
}


bool QFile::at( Offset pos )
{
    if ( !isOpen() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QFile::at: File is not open" );
#endif
	return FALSE;
    }
    bool ok;
    if ( isRaw() ) {
#if defined(QT_LARGE_FILE_SUPPORT)
	pos = (Offset)::lseek64( fd, pos, SEEK_SET );
#else
	pos = (Offset)::lseek( fd, pos, SEEK_SET );
#endif
	ok = (long int) pos != -1;		// ### fix this bad hack!
    } else {					// buffered file
#if defined(QT_LARGE_FILE_SUPPORT)
	ok = ::fseeko64(fh, pos, SEEK_SET) == 0;
#else
	ok = ::fseek(fh, pos, SEEK_SET) == 0;
#endif
    }
    if ( ok )
	ioIndex = pos;
#if defined(QT_CHECK_RANGE)
    else
	qWarning( "QFile::at: Cannot set file position %ld", pos );
#endif
    return ok;
}


Q_LONG QFile::readBlock( char *p, Q_ULONG len )
{
#if defined(QT_CHECK_NULL)
    if ( !p )
	qWarning( "QFile::readBlock: Null pointer error" );
#endif
#if defined(QT_CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	qWarning( "QFile::readBlock: File not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	qWarning( "QFile::readBlock: Read operation not permitted" );
	return -1;
    }
#endif
    int nread = 0;					// number of bytes read
    if ( !ungetchBuffer.isEmpty() ) {
	// need to add these to the returned string.
	int l = ungetchBuffer.length();
	while( nread < l ) {
	    *p = ungetchBuffer[ l - nread - 1 ];
	    p++;
	    nread++;
	}
	ungetchBuffer.truncate( l - nread );
    }

    if ( nread < (int)len ) {
	if ( isRaw() ) {				// raw file
	    nread += ::read( fd, p, len-nread );
	    if ( len && nread <= 0 ) {
		nread = 0;
		setStatus(IO_ReadError);
	    }
	} else {					// buffered file
	    nread += fread( p, 1, len-nread, fh );
	    if ( (uint)nread != len ) {
		if ( ferror( fh ) || nread==0 )
		    setStatus(IO_ReadError);
	    }
	}
    }
    ioIndex += nread;
    return nread;
}


Q_LONG QFile::writeBlock( const char *p, Q_ULONG len )
{
#if defined(QT_CHECK_NULL)
    if ( p == 0 && len != 0 )
	qWarning( "QFile::writeBlock: Null pointer error" );
#endif
#if defined(QT_CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	qWarning( "QFile::writeBlock: File not open" );
	return -1;
    }
    if ( !isWritable() ) {			// writing not permitted
	qWarning( "QFile::writeBlock: Write operation not permitted" );
	return -1;
    }
#endif
    int nwritten;				// number of bytes written
    if ( isRaw() )				// raw file
	nwritten = ::write( fd, (void *)p, len );
    else					// buffered file
	nwritten = fwrite( p, 1, len, fh );
    if ( nwritten != (int)len ) {		// write error
	if ( errno == ENOSPC )			// disk is full
	    setStatus( IO_ResourceError );
	else
	    setStatus( IO_WriteError );
	if ( isRaw() )				// recalc file position
#if defined(QT_LARGE_FILE_SUPPORT)
	    ioIndex = (Offset)::lseek64( fd, 0, SEEK_CUR );
#else
	    ioIndex = (Offset)::lseek( fd, 0, SEEK_CUR );
#endif
	else
#if defined(QT_LARGE_FILE_SUPPORT)
	    ioIndex = ::fseeko64( fh, 0, SEEK_CUR );
#else
	    ioIndex = ::fseek( fh, 0, SEEK_CUR );
#endif
    } else {
	ioIndex += nwritten;
    }
    if ( ioIndex > length )			// update file length
	length = ioIndex;
    return nwritten;
}

int QFile::handle() const
{
    if ( !isOpen() )
	return -1;
    else if ( fh )
	return fileno( fh );
    else
	return fd;
}

void QFile::close()
{
    bool ok = FALSE;
    if ( isOpen() ) {				// file is not open
	if ( fh ) {				// buffered file
	    if ( ext_f )
		ok = fflush( fh ) != -1;	// flush instead of closing
	    else
		ok = fclose( fh ) != -1;
	} else {				// raw file
	    if ( ext_f )
		ok = TRUE;			// cannot close
	    else
		ok = ::close( fd ) != -1;
	}
	init();					// restore internal state
    }
    if (!ok)
	setStatus( IO_UnspecifiedError );

    return;
}

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.cpp#61 $
**
** Implementation of QFileInfo class
**
** Created : 950628
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qglobal.h"

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

#include "qfile.h"
#include "qfiledefs.h"

#include <windows.h>
#include <direct.h>
#include <tchar.h>

QCString qt_win95Name(const QString s)
{
    if ( s[0] == '/' && s[1] == '/' ) {
	// Win95 cannot handle slash-slash needs slosh-slosh.
	QString ss(s);
	ss[0] = '\\';
	ss[1] = '\\';
	int n = ss.find('/');
	if ( n >= 0 )
	    ss[n] = '\\';
	return qt_winQString2MB(ss);
    } else {
	return qt_winQString2MB(s);
    }
}

bool qt_file_access( const QString& fn, int t )
{
    if ( fn.isEmpty() )
	return FALSE;
    if ( qt_winunicode )
	return _taccess((TCHAR*)qt_winTchar(fn,TRUE), t) == 0;
    else
	return ACCESS(qt_win95Name(fn), t) == 0;
}

/*!
  Removes the file \a fileName.
  Returns TRUE if successful, otherwise FALSE.
*/

bool QFile::remove( const QString &fileName )
{
    if ( fileName.isEmpty() ) {
#if defined(CHECK_NULL)
	qWarning( "QFile::remove: Empty or null file name" );
#endif
	return FALSE;
    }
    // use standard ANSI remove
    if ( qt_winunicode )
	return _tremove((const TCHAR*)qt_winTchar(fileName,TRUE)) == 0;
    else
	return ::remove(qt_win95Name(fileName)) == 0;
}

#define HAS_TEXT_FILEMODE

/*!
  Opens the file specified by the file name currently set, using the mode \e m.
  Returns TRUE if successful, otherwise FALSE.

  The mode parameter \e m must be a combination of the following flags:
  <ul>
  <li>\c IO_Raw specified raw (non-buffered) file access.
  <li>\c IO_ReadOnly opens the file in read-only mode.
  <li>\c IO_WriteOnly opens the file in write-only mode (and truncates).
  <li>\c IO_ReadWrite opens the file in read/write mode, equivalent to
  \c (IO_ReadOnly|IO_WriteOnly).
  <li>\c IO_Append opens the file in append mode. This mode is very useful
  when you want to write something to a log file. The file index is set to
  the end of the file. Note that the result is undefined if you position the
  file index manually using at() in append mode.
  <li>\c IO_Truncate truncates the file.
  <li>\c IO_Translate enables carriage returns and linefeed translation
  for text files under MS-DOS, Windows and OS/2.
  </ul>

  The raw access mode is best when I/O is block-operated using 4kB block size
  or greater. Buffered access works better when reading small portions of
  data at a time.

  <strong>Important:</strong> When working with buffered files, data may
  not be written to the file at once. Call \link flush() flush\endlink
  to make sure the data is really written.

  \warning We have experienced problems with some C libraries when a buffered
  file is opened for both reading and writing. If a read operation takes place
  immediately after a write operation, the read buffer contains garbage data.
  Worse, the same garbage is written to the file. Calling flush() before
  readBlock() solved this problem.

  If the file does not exist and \c IO_WriteOnly or \c IO_ReadWrite is
  specified, it is created.

  Example:
  \code
    QFile f1( "/tmp/data.bin" );
    QFile f2( "readme.txt" );
    f1.open( IO_Raw | IO_ReadWrite | IO_Append );
    f2.open( IO_ReadOnly | IO_Translate );
  \endcode

  \sa name(), close(), isOpen(), flush()
*/

bool QFile::open( int m )
{
    if ( isOpen() ) {				// file already open
#if defined(CHECK_STATE)
	qWarning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    if ( fn.isNull() ) {			// no file name defined
#if defined(CHECK_NULL)
	qWarning( "QFile::open: No file name specified" );
#endif
	return FALSE;
    }
    init();					// reset params
    setMode( m );
    if ( !(isReadable() || isWritable()) ) {
#if defined(CHECK_RANGE)
	qWarning( "QFile::open: File access not specified" );
#endif
	return FALSE;
    }
    bool ok = TRUE;
    if ( isRaw() ) {				// raw file I/O
	int oflags = OPEN_RDONLY;
	if ( isReadable() && isWritable() )
	    oflags = OPEN_RDWR;
	else if ( isWritable() )
	    oflags = OPEN_WRONLY;
	if ( flags() & IO_Append ) {		// append to end of file?
	    if ( flags() & IO_Truncate )
		oflags |= (OPEN_CREAT | OPEN_TRUNC);
	    else
		oflags |= (OPEN_APPEND | OPEN_CREAT);
	    setFlags( flags() | IO_WriteOnly ); // append implies write
	} else if ( isWritable() ) {		// create/trunc if writable
	    if ( flags() & IO_Truncate )
		oflags |= (OPEN_CREAT | OPEN_TRUNC);
	    else
		oflags |= OPEN_CREAT;
	}
#if defined(HAS_TEXT_FILEMODE)
	if ( isTranslated() )
	    oflags |= OPEN_TEXT;
	else
	    oflags |= OPEN_BINARY;
#endif
#if defined(HAS_ASYNC_FILEMODE)
	if ( isAsynchronous() )
	    oflags |= OPEN_ASYNC;
#endif
	if ( qt_winunicode ) {
	    fd = ::_topen((const TCHAR*)qt_winTchar(fn,TRUE), oflags, 0666 );
	} else {
	    fd = OPEN(qt_win95Name(fn), oflags, 0666 );
	}

	if ( fd != -1 ) {			// open successful
	    STATBUF st;
	    FSTAT( fd, &st );
	    if ( (st.st_mode&STAT_MASK) == STAT_DIR ) {
		ok = FALSE;
	    } else {
		length = (int)st.st_size;
		ioIndex  = (flags() & IO_Append) == 0 ? 0 : length;
	    }
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
	strcpy( perm2, perm );
#if defined(HAS_TEXT_FILEMODE)
	if ( isTranslated() )
	    strcat( perm2, "t" );
	else
	    strcat( perm2, "b" );
#endif
	while (1) { // At most twice
	    if ( qt_winunicode ) {
		TCHAR tperm2[4];
		tperm2[0] = perm2[0];
		tperm2[1] = perm2[1];
		tperm2[2] = perm2[2];
		tperm2[3] = perm2[3];
		fh = _tfopen((const TCHAR*)qt_winTchar(fn,TRUE), tperm2 );
	    } else {
		fh = fopen(qt_win95Name(fn),
			    perm2 );
	    }
	    if ( !fh && try_create ) {
		perm2[0] = 'w';			// try "w+" instead of "r+"
		try_create = FALSE;
	    } else {
		break;
	    }
	}
	if ( fh ) {
	    STATBUF st;
	    FSTAT( FILENO(fh), &st );
	    if ( (st.st_mode&STAT_MASK) == STAT_DIR ) {
		ok = FALSE;
	    } else {
		length = (int)st.st_size;
		ioIndex  = (flags() & IO_Append) == 0 ? 0 : length;
	    }
	} else {
	    ok = FALSE;
	}
    }
    if ( ok ) {
	setState( IO_Open );
    } else {
	init();
	if ( errno == EMFILE )			// no more file handles/descrs
	    setStatus( IO_ResourceError );
	else
	    setStatus( IO_OpenError );
    }
    return ok;
}



/*!
  Opens a file in the mode \e m using an existing file handle \e f.
  Returns TRUE if successful, otherwise FALSE.

  Example:
  \code
    #include <stdio.h>

    void printError( const char* msg )
    {
	QFile f;
	f.open( IO_WriteOnly, stderr );
	f.writeBlock( msg, strlen(msg) );	// write to stderr
	f.close();
    }
  \endcode

  When a QFile is opened using this function, close() does not actually
  close the file, only flushes it.

  \warning If \e f is \c stdin, \c stdout, \c stderr, you may not
  be able to seek.  See QIODevice::isSequentialAccess() for more
  information.

  \sa close()
*/

bool QFile::open( int m, FILE *f )
{
    if ( isOpen() ) {
#if defined(CHECK_RANGE)
	qWarning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    init();
    setMode( m &~IO_Raw );
    setState( IO_Open );
    fh = f;
    ext_f = TRUE;
    STATBUF st;
    FSTAT( FILENO(fh), &st );
    ioIndex = (int)ftell( fh );
    if ( (st.st_mode & STAT_MASK) != STAT_REG ) {
	// non-seekable
	setType( IO_Sequential );
	length = INT_MAX;
    } else {
	length = (int)st.st_size;
    }
    return TRUE;
}


/*!
  Opens a file in the mode \e m using an existing file descriptor \e f.
  Returns TRUE if successful, otherwise FALSE.

  When a QFile is opened using this function, close() does not actually
  close the file.

  \warning If \e f is one of 0 (stdin), 1 (stdout) or 2 (stderr), you may not
  be able to seek. size() is set to \c INT_MAX (in limits.h).

  \sa close()
*/

bool QFile::open( int m, int f )
{
    if ( isOpen() ) {
#if defined(CHECK_RANGE)
	qWarning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    init();
    setMode( m |IO_Raw );
    setState( IO_Open );
    fd = f;
    ext_f = TRUE;
    STATBUF st;
    FSTAT( fd, &st );
    ioIndex  = (int)LSEEK(fd, 0, SEEK_CUR);
    if ( (st.st_mode & STAT_MASK) != STAT_REG ) {
	// non-seekable
	setType( IO_Sequential );
	length = INT_MAX;
    } else {
	length = (int)st.st_size;
    }
    return TRUE;
}

/*!
  Returns the file size.
  \sa at()
*/

uint QFile::size() const
{
    STATBUF st;
    if ( isOpen() ) {
	FSTAT( fh ? FILENO(fh) : fd, &st );
    } else {
#if defined (UNIX)
	STAT( QFile::encodeName(fn), &st );
#elif defined(_OS_WIN32_)
	if ( qt_winunicode ) {
	    _tstat((const TCHAR*)qt_winTchar(fn,TRUE), &st);
	} else {
	    _stat(qt_win95Name(fn), &st);
	}
#endif
    }
    return st.st_size;
}

/*!
  \fn int QFile::at() const
  Returns the file index.
  \sa size()
*/

/*!
  Sets the file index to \e pos. Returns TRUE if successful, otherwise FALSE.

  Example:
  \code
    QFile f( "data.bin" );
    f.open( IO_ReadOnly );			// index set to 0
    f.at( 100 );				// set index to 100
    f.at( f.at()+50 );				// set index to 150
    f.at( f.size()-80 );			// set index to 80 before EOF
    f.close();
  \endcode

  \warning The result is undefined if the file was \link open() opened\endlink
  using the \c IO_Append specifier.

  \sa size(), open()
*/

bool QFile::at( int pos )
{
    if ( !isOpen() ) {
#if defined(CHECK_STATE)
	qWarning( "QFile::at: File is not open" );
#endif
	return FALSE;
    }
    bool ok;
    if ( isRaw() ) {				// raw file
	pos = (int)LSEEK(fd, pos, SEEK_SET);
	ok = pos != -1;
    } else {					// buffered file
	ok = fseek(fh, pos, SEEK_SET) == 0;
    }
    if ( ok )
	ioIndex = pos;
#if defined(CHECK_RANGE)
    else
	qWarning( "QFile::at: Cannot set file position %d", pos );
#endif
    return ok;
}

/*!
  Reads at most \e len bytes from the file into \e p and returns the
  number of bytes actually read.

  Returns -1 if a serious error occurred.

  \warning We have experienced problems with some C libraries when a buffered
  file is opened for both reading and writing. If a read operation takes place
  immediately after a write operation, the read buffer contains garbage data.
  Worse, the same garbage is written to the file. Calling flush() before
  readBlock() solved this problem.

  \sa writeBlock()
*/

int QFile::readBlock( char *p, uint len )
{
#if defined(CHECK_NULL)
    if ( !p )
	qWarning( "QFile::readBlock: Null pointer error" );
#endif
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	qWarning( "QFile::readBlock: File not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	qWarning( "QFile::readBlock: Read operation not permitted" );
	return -1;
    }
#endif
    int nread;					// number of bytes read
    if ( isRaw() ) {				// raw file
	nread = READ( fd, p, len );
	if ( len && nread <= 0 ) {
	    nread = 0;
	    setStatus(IO_ReadError);
	}
    } else {					// buffered file
	nread = fread( p, 1, len, fh );
	if ( (uint)nread != len ) {
	    if ( ferror( fh ) || nread==0 )
		setStatus(IO_ReadError);
	}
    }
    ioIndex += nread;
    return nread;
}

/*! \overload int writeBlock( const QByteArray& data )
*/

/*! \reimp

  Writes \e len bytes from \e p to the file and returns the number of
  bytes actually written.

  Returns -1 if a serious error occurred.

  \warning When working with buffered files, data may not be written
  to the file at once. Call flush() to make sure the data is really
  written.

  \sa readBlock()
*/

int QFile::writeBlock( const char *p, uint len )
{
#if defined(CHECK_NULL)
    if ( p == 0 && len != 0 )
	qWarning( "QFile::writeBlock: Null pointer error" );
#endif
#if defined(CHECK_STATE)
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
	nwritten = WRITE( fd, p, len );
    else					// buffered file
	nwritten = fwrite( p, 1, len, fh );
    if ( nwritten != (int)len ) {		// write error
	if ( errno == ENOSPC )			// disk is full
	    setStatus( IO_ResourceError );
	else
	    setStatus( IO_WriteError );
	if ( isRaw() )				// recalc file position
	    ioIndex = (int)LSEEK( fd, 0, SEEK_CUR );
	else
	    ioIndex = fseek( fh, 0, SEEK_CUR );
    } else {
	ioIndex += nwritten;
    }
    if ( ioIndex > length )			// update file length
	length = ioIndex;
    return nwritten;
}

/*!
  Returns the file handle of the file.

  This is a small positive integer, suitable for use with C library
  functions such as fdopen() and fcntl(), as well as with QSocketNotifier.

  If the file is not open or there is an error, handle() returns -1.

  \sa QSocketNotifier
*/

int QFile::handle() const
{
    if ( !isOpen() )
	return -1;
    else if ( fh )
	return FILENO( fh );
    else
	return fd;
}

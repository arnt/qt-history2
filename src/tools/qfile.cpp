/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfile.cpp#82 $
**
** Implementation of QFile class
**
** Created : 930812
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

#include "qfile.h"

#if defined(_OS_WIN32_)
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif
#endif

#include "qfiledefs.h"

#if defined(_OS_WIN32_)
#include <tchar.h>
#endif

/*!
  \class QFile qfile.h
  \brief The QFile class is an I/O device that operates on files.

  \ingroup io

  QFile is an I/O device for reading and writing binary and text files.	A
  QFile may be used by itself (readBlock and writeBlock) or by more
  conveniently using QDataStream or QTextStream.

  Here is a code fragment that uses QTextStream to read a text
  file line by line. It prints each line with a line number.
  \code
    QFile f("file.txt");
    if ( f.open(IO_ReadOnly) ) {    // file opened successfully
	QTextStream t( &f );	    // use a text stream
	QString s;
	int n = 1;
	while ( !t.eof() ) {	    // until end of file...
	    s = t.readLine();	    // line of text excluding '\n'
	    printf( "%3d: %s\n", n++, (const char *)s );
	}
	f.close();
    }
  \endcode

  The QFileInfo class holds detailed information about a file, such as
  access permissions, file dates and file types.

  The QDir class manages directories and lists of file names.

  \sa QDataStream, QTextStream
*/


/*!
  Constructs a QFile with no name.
*/

QFile::QFile()
{
    init();
}

/*!
  Constructs a QFile with a file name \e name.
  \sa setName()
*/

QFile::QFile( const QString &name )
    : fn(name)
{
    init();
}


/*!
  Destroys a QFile.  Calls close().
*/

QFile::~QFile()
{
    close();
}


/*!
  \internal
  Initialize internal data.
*/

void QFile::init()
{
    setFlags( IO_Direct );
    setStatus( IO_Ok );
    fh	   = 0;
    fd	   = 0;
    length = 0;
    ioIndex  = 0;
    ext_f  = FALSE;				// not an external file handle
}


/*!
  \fn QString QFile::name() const
  Returns the name set by setName().
  \sa setName(), QFileInfo::fileName()
*/

/*!
  Sets the name of the file. The name can include an absolute directory
  path or it can be a name or a path relative to the current directory.

  Do not call this function if the file has already been opened.

  Note that if the name is relative QFile does not associate it with the
  current directory.  If you change directory before calling open(), open
  uses the new current directory.

  Example:
  \code
     QFile f;
     QDir::setCurrent( "/tmp" );
     f.setName( "readme.txt" );
     QDir::setCurrent( "/home" );
     f.open( IO_ReadOnly );	   // opens "/home/readme.txt" under UNIX
  \endcode

  Also note that the directory separator '/' works for all operating
  systems supported by Qt.

  \sa name(), QFileInfo, QDir
*/

void QFile::setName( const QString &name )
{
    if ( isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QFile::setName: File is open" );
#endif
	close();
    }
    fn = name;
}

bool qt_file_access( const QString& fn, int t )
{
    if ( fn.isEmpty() )
	return FALSE;
#if defined (UNIX) || defined(__CYGWIN32__)
    return ACCESS( QFile::encodeName(fn), t ) == 0;
#else
    if ( qt_winunicode ) {
	return _taccess((const TCHAR*)qt_winTchar(fn,TRUE), t) == 0;
    } else {
	return _access(qt_winQString2MB(fn), t) == 0;
    }
#endif
}

/*!
  Returns TRUE if this file exists, otherwise FALSE.
  \sa name()
*/

bool QFile::exists() const
{
    return qt_file_access( fn, F_OK );
}

/*!
  Returns TRUE if the file given by \e fileName exists, otherwise FALSE.
*/

bool QFile::exists( const QString &fileName )
{
    return qt_file_access( fileName, F_OK );
}


/*!
  Removes the file specified by the file name currently set.
  Returns TRUE if successful, otherwise FALSE.

  The file is closed before it is removed.
*/

bool QFile::remove()
{
    close();
    return remove( fn );
}

/*!
  Removes the file \a fileName.
  Returns TRUE if successful, otherwise FALSE.
*/

bool QFile::remove( const QString &fileName )
{
    if ( fileName.isEmpty() ) {
#if defined(CHECK_NULL)
        warning( "QFile::remove: Empty or NULL file name" );
#endif
        return FALSE;
    }
#if defined(UNIX)
    return unlink( QFile::encodeName(fileName) ) == 0;	// unlink more common in UNIX
#else
    // use standard ANSI remove
    if ( qt_winunicode ) {
	return _tremove((const TCHAR*)qt_winTchar(fileName,TRUE)) == 0;
    } else {
	return ::remove(qt_winQString2MB(fileName)) == 0;
    }
#endif
}


#if defined(_OS_MAC_) || defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
# define HAS_TEXT_FILEMODE			// has translate/text filemode
#endif
#if defined(O_NONBLOCK)
# define HAS_ASYNC_FILEMODE
# define OPEN_ASYNC O_NONBLOCK
#elif defined(O_NDELAY)
# define HAS_ASYNC_FILEMODE
# define OPEN_ASYNC O_NDELAY
#endif

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
	warning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    if ( fn.isNull() ) {			// no file name defined
#if defined(CHECK_NULL)
	warning( "QFile::open: No file name specified" );
#endif
	return FALSE;
    }
    init();					// reset params
    setMode( m );
    if ( !(isReadable() || isWritable()) ) {
#if defined(CHECK_RANGE)
	warning( "QFile::open: File access not specified" );
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
#if defined(HAS_TEXT_FILEMODE) && !defined(_OS_MAC_)
	if ( isTranslated() )
	    oflags |= OPEN_TEXT;
	else
	    oflags |= OPEN_BINARY;
#endif
#if defined(HAS_ASYNC_FILEMODE)
	if ( isAsynchronous() )
	    oflags |= OPEN_ASYNC;
#endif
#if defined(UNIX)
	fd = OPEN( QFile::encodeName(fn), oflags, 0666 );
#else
	if ( qt_winunicode ) {
	    fd = _topen((const TCHAR*)qt_winTchar(fn,TRUE), oflags, 0666 );
	} else {
	    fd = _open(qt_winQString2MB(fn), oflags, 0666 );
	}
#endif

	if ( fd != -1 ) {			// open successful
	    STATBUF st;
	    FSTAT( fd, &st );
	    length = (int)st.st_size;
	    ioIndex  = (flags() & IO_Append) == 0 ? 0 : length;
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
#if defined(UNIX)
	    fh = fopen( QFile::encodeName(fn), perm2 );
#else
	    if ( qt_winunicode ) {
		TCHAR tperm2[4];
		tperm2[0] = perm2[0];
		tperm2[1] = perm2[1];
		tperm2[2] = perm2[2];
		tperm2[3] = perm2[3];
		fh = _tfopen((const TCHAR*)qt_winTchar(fn,TRUE), tperm2 );
	    } else {
		fh = fopen(qt_winQString2MB(fn), perm2 );
	    }
#endif
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
	    length = (int)st.st_size;
	    ioIndex  = (flags() & IO_Append) == 0 ? 0 : length;
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
	warning( "QFile::open: File already open" );
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
	warning( "QFile::open: File already open" );
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
  Closes an open file.

  The file is closed even if it was opened with an existing file
  handle or a file descriptor, \e except that stdin, stdout and stderr
  are never closed.

  Some "write-behind" filesystems may report an unspecified error on
  closing the file. These errors only indiciate that something may
  have gone wrong since the previous open(). In such a case status()
  reports IO_UnspecifiedError after close(), otherwise IO_Ok.

  \sa open(), flush()
*/

void QFile::close()
{
    bool ok = FALSE;
    if ( isOpen() ) {				// file is not open
	if ( fh ) {					// buffered file
	    if ( ext_f )
		fflush( fh );			// cannot close
	    else
		ok = fclose( fh ) != -1;
	} else {					// raw file
	    if ( ext_f )
		;					// cannot close
	    else
		ok = CLOSE( fd ) != -1;
	}
	init();					// restore internal state
    }
    if (!ok)
	setStatus (IO_UnspecifiedError);

    return;
}


/*!
  Flushes the file buffer to the disk.

  close() also flushes the file buffer.
*/

void QFile::flush()
{
    if ( isOpen() && fh )			// can only flush open/buffered
	fflush( fh );				//   file
}


/*!
  Returns the file size.
  \sa at()
*/

uint QFile::size() const
{
    STATBUF st;
    if ( isOpen() )
	FSTAT( fh ? FILENO(fh) : fd, &st );
    else {
#if defined (UNIX)
	STAT( QFile::encodeName(fn), &st );
#else
	if ( qt_winunicode ) {
	    _tstat((const TCHAR*)qt_winTchar(fn,TRUE), &st);
	} else {
	    _stat(qt_winQString2MB(fn), &st);
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
	warning( "QFile::at: File is not open" );
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
	warning( "QFile::at: Cannot set file position %d", pos );
#endif
    return ok;
}


/*!
  Returns TRUE if the end of file has been reached, otherwise FALSE.
  \sa size()
*/

bool QFile::atEnd() const
{
    if ( !isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QFile::atEnd: File is not open" );
#endif
	return FALSE;
    }
    if ( isDirectAccess() ) {
	if ( at() < length )
	    return FALSE;
    }
    return QIODevice::atEnd();
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
	warning( "QFile::readBlock: Null pointer error" );
#endif
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	warning( "QFile::readBlock: File not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QFile::readBlock: Read operation not permitted" );
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


/*!
  Writes \e len bytes from \e p to the file and returns the number of
  bytes actually written.

  Returns -1 if a serious error occurred.

  <strong>Important:</strong> When working with buffered files, data may
  not be written to the file at once. Call \link flush() flush\endlink
  to make sure the data is really written.

  \sa readBlock()
*/

int QFile::writeBlock( const char *p, uint len )
{
#if defined(CHECK_NULL)
    if ( p == 0 && len != 0 )
	warning( "QFile::writeBlock: Null pointer error" );
#endif
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	warning( "QFile::writeBlock: File not open" );
	return -1;
    }
    if ( !isWritable() ) {			// writing not permitted
	warning( "QFile::writeBlock: Write operation not permitted" );
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
  Reads a line of text.

  Reads bytes from the file until end-of-line is reached, or up to \a
  maxlen bytes, and returns the number of bytes read, or -1 in case of
  error.  The terminating newline is not stripped.

  This function is efficient only for buffered files.  Avoid
  readLine() for files that have been opened with the \c IO_Raw
  flag.

  \sa readBlock(), QTextStream::readLine()
*/

int QFile::readLine( char *p, uint maxlen )
{
    if ( maxlen == 0 )				// application bug?
	return 0;
#if defined(CHECK_STATE)
    CHECK_PTR( p );
    if ( !isOpen() ) {				// file not open
	warning( "QFile::readLine: File not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QFile::readLine: Read operation not permitted" );
	return -1;
    }
#endif
    int nread;					// number of bytes read
    if ( isRaw() ) {				// raw file
	nread = QIODevice::readLine( p, maxlen );
    } else {					// buffered file
	p = fgets( p, maxlen, fh );
	if ( p ) {
	    nread = strlen( p );
	    ioIndex += nread;
	} else {
	    nread = -1;
	    setStatus(IO_ReadError);
	}
    }
    return nread;
}


/*!
  Reads a line of text.

  Reads bytes from the file until end-of-line is reached, or up to \a
  maxlen bytes, and returns the number of bytes read, or -1 in case of
  error.  The terminating newline is not stripped.

  This function is efficient only for buffered files.  Avoid
  readLine() for files that have been opened with the \c IO_Raw
  flag.

  Note that the string is read as plain Latin1 bytes, not Unicode.

  \sa readBlock(), QTextStream::readLine()
*/

int QFile::readLine( QString& s, uint maxlen )
{
    QByteArray ba(maxlen);
    int l = readLine(ba.data(),maxlen);
    if ( l >= 0 ) {
	ba.truncate(l);
	s = QCString(ba);
    }
    return l;
}


/*!
  Reads a single byte/character from the file.

  Returns the byte/character read, or -1 if the end of the file has been
  reached.

  \sa putch(), ungetch()
*/

int QFile::getch()
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	warning( "QFile::getch: File not open" );
	return EOF;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QFile::getch: Read operation not permitted" );
	return EOF;
    }
#endif
    
    int ch;

    if ( !ungetchBuffer.isEmpty() ) {
	int len = ungetchBuffer.length();
	ch = ungetchBuffer[ len-1 ];
	ungetchBuffer.truncate( len - 1 );
	return ch;
    }
    
    if ( isRaw() ) {				// raw file (inefficient)
	char buf[1];
	ch = readBlock( buf, 1 ) == 1 ? buf[0] : EOF;
    } else {					// buffered file
	if ( (ch = getc( fh )) != EOF )
	    ioIndex++;
	else
	    setStatus(IO_ReadError);
    }
    return ch;
}

/*!
  Writes the character \e ch to the file.

  Returns \e ch, or -1 if some error occurred.

  \sa getch(), ungetch()
*/

int QFile::putch( int ch )
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	warning( "QFile::putch: File not open" );
	return EOF;
    }
    if ( !isWritable() ) {			// writing not permitted
	warning( "QFile::putch: Write operation not permitted" );
	return EOF;
    }
#endif
    if ( isRaw() ) {				// raw file (inefficient)
	char buf[1];
	buf[0] = ch;
	ch = writeBlock( buf, 1 ) == 1 ? ch : EOF;
    } else {					// buffered file
	if ( (ch = putc( ch, fh )) != EOF ) {
	    ioIndex++;
	    if ( ioIndex > length )		// update file length
		length = ioIndex;
	} else {
	    setStatus(IO_WriteError);
	}
    }
    return ch;
}

/*!
  Puts the character \e ch back into the file and decrements the index if it
  is not zero.

  This function is normally called to "undo" a getch() operation.

  Returns \e ch, or -1 if some error occurred.

  \sa getch(), putch()
*/

int QFile::ungetch( int ch )
{
#if defined(CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	warning( "QFile::ungetch: File not open" );
	return EOF;
    }
    if ( !isReadable() ) {			// reading not permitted
	warning( "QFile::ungetch: Read operation not permitted" );
	return EOF;
    }
#endif
    if ( ch == EOF )				// cannot unget EOF
	return ch;
    
    if ( isSequentialAccess() && !fh) {
	// pipe or similar => we cannot ungetch, so do it manually
	ungetchBuffer +=ch;
	return ch;
    }
    
    if ( isRaw() ) {				// raw file (very inefficient)
	char buf[1];
	at( ioIndex-1 );
	buf[0] = ch;
	if ( writeBlock(buf, 1) == 1 )
	    at ( ioIndex-1 );
	else
	    ch = EOF;
    } else {					// buffered file
	if ( (ch = ungetc(ch, fh)) != EOF )
	    ioIndex--;
	else
	    setStatus( IO_ReadError );
    }
    return ch;
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
	return fileno( fh );
    else
	return fd;
}

static
QCString locale_encoder( const QString &fileName )
{
    return fileName.local8Bit();
}

static QFile::EncoderFn encoder = locale_encoder;

/*!
  When you use QFile, QFileInfo, and QDir to access the filesystem
  with Qt, you can use Unicode filenames.  On Unix, these filenames
  are converted to an 8-bit encoding.  If you want to do your own
  file I/O on Unix, you should convert the filename using this
  function.  On Windows NT, Unicode filenames are supported directly
  in the filesystem and this function should be avoided. On Windows 95,
  non-Latin1 locales are not supported at this time.

  By default, this function converts to the local 8-bit encoding
  determined by the user's locale.  This is sufficient for
  filenames that the user chooses.  Filenames hard-coded into the
  application should only use 7-bit ASCII filename characters.

  The conversion scheme can be changed using setEncodingFunction().
  This might be useful if you wish to give the user an option to
  store in filenames in UTF-8, etc., but beware that such filenames
  would probably then be unrecognizable when seen by other programs.

  \sa decodedName().
*/
QCString QFile::encodeName( const QString &fileName )
{
    return (*encoder)(fileName);
}

/*!
  Sets the function for encoding Unicode filenames.
  The default encodes in the locale-specific 8-bit encoding.

  \sa encodeName()
*/
void QFile::setEncodingFunction( EncoderFn f )
{
    encoder = f;
}

static
QString locale_decoder( const QCString &localFileName )
{
    return QString::fromLocal8Bit(localFileName);
}

static QFile::DecoderFn decoder = locale_decoder;

/*!
  This does the reverse of QFile::encodeName().

  \sa setDecodingFunction()
*/
QString QFile::decodeName( const QCString &localFileName )
{
    return (*decoder)(localFileName);
}

/*!
  Sets the function for decoding 8-bit filenames.
  The default uses the locale-specific 8-bit encoding.

  \sa encodeName(), decodedName()
*/
void QFile::setDecodingFunction( DecoderFn f )
{
    decoder = f;
}

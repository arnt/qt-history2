/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfile.cpp#22 $
**
** Implementation of QFile class
**
** Author  : Haavard Nord
** Created : 930812
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfile.h"
#include "qfiledef.h"
#include <limits.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qfile.cpp#22 $";
#endif


/*----------------------------------------------------------------------------
  \class QFile qfile.h
  \brief The QFile class provides system-independent file access.

  \ingroup tools
  \ingroup files

  A file is an \link QIODevice I/O device\endlink that can read and write
  binary and text files.

  The QFileInfo class holds detailed information about a file, such as
  access permissions, file dates and file types.

  The QDir class manages directories and lists of file names.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Constructs a QFile with no name.
 ----------------------------------------------------------------------------*/

QFile::QFile()
{
    init();
}

/*----------------------------------------------------------------------------
  Constructs a QFile with a file name \e name.
  \sa setName()
 ----------------------------------------------------------------------------*/

QFile::QFile( const char *name )
{
    init();
    fn = name;
}

/*----------------------------------------------------------------------------
  Destroys a QFile.  Calls close().
 ----------------------------------------------------------------------------*/

QFile::~QFile()
{
    close();					// close file
}

void QFile::init()				// initialize internal data
{
    setFlags( IO_Direct );
    setStatus( IO_Ok );
    fh     = 0;
    fd     = 0;
    length = 0;
    index  = 0;
}


/*----------------------------------------------------------------------------
  \fn const char *QFile::name() const
  Returns the name set by setName().

  This is NOT a function that always returns a pure file name (i.e. without
  path information).

  \sa setName(), QFileInfo::fileName().  
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the name of the file, the name can include an absolute directory
  path or it can be a name or a path relative to the current directory.

  Note that if the name is relative it will NOT be associated with the current
  directory, thus changing the current directory before doing
  open() will change the location of the QFile.

  Do not call this function if the file has already been opened.

  Example:
  \code
     QFile f;
     QDir::setCurrent( "/tmp" );
     f.setName( "readme.txt" );
     QDir::setCurrent( "/home" );
     f.open( IO_ReadOnly );	   // will open "/home/readme.txt" under UNIX
  \endcode

  \sa name(), QFileInfo, QDir
 ----------------------------------------------------------------------------*/

void QFile::setName( const char *name )
{
    if ( isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QFile::setName: File is open" );
#endif
        close();
    }
    fn = name;
}


/*----------------------------------------------------------------------------
  Returns TRUE if the file exists, otherwise FALSE.

  \sa name()
 ----------------------------------------------------------------------------*/

bool QFile::exists() const
{
    if ( fn.isEmpty() )
	return FALSE;
    return ACCESS( fn.data(), Q_FILE_OK ) == 0;
}

/*----------------------------------------------------------------------------
  Returns TRUE if the file given by \e fileName exists, otherwise FALSE.
 ----------------------------------------------------------------------------*/

bool QFile::exists( const char *fileName )
{
#if defined(CHECK_NULL)
    ASSERT( fileName != 0 );
#endif
    return ACCESS( fileName, Q_FILE_OK ) == 0;
}


#if defined(_OS_MAC_) || defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
#define HAS_TEXT_FILEMODE			// has translate/text filemode
#endif
#if defined(O_NONBLOCK)
#define HAS_ASYNC_FILEMODE
#define OPEN_ASYNC O_NONBLOCK
#elif defined(O_NDELAY)
#define HAS_ASYNC_FILEMODE
#define OPEN_ASYNC O_NDELAY
#endif

/*----------------------------------------------------------------------------
  Opens the file specified by the file name currently set, using the mode \e m.
  Returns TRUE if successful, otherwise FALSE.

  The mode parameter \e m must be a combination of the following flags.
  <ul>
  <li>\c IO_Raw specified raw (unbuffered) file access.
  <li>\c IO_ReadOnly opens a file in read-only mode.
  <li>\c IO_WriteOnly opens a file in write-only mode.
  <li>\c IO_ReadWrite opens a file in read/write mode.
  <li>\c IO_Append sets the file index to the end of the file.
  <li>\c IO_Truncate truncates the file.
  <li>\c IO_Translate enables carriage returns and linefeed translation
  for text files under MS-DOS, Window, OS/2 and Macintosh.  Cannot be
  combined with \c IO_Raw.
  </ul>

  If the file does not exist and \c IO_WriteOnly or \c IO_ReadWrite is
  specified, it will be created.

  Example:
  \code
    QFile f1( "/tmp/data.bin" );
    QFile f2( "readme.txt" );
    f1.open( IO_Raw | IO_ReadWrite | IO_Append );
    f2.open( IO_ReadOnly | IO_Translate );
  \endcode

  \sa name(), close()
 ----------------------------------------------------------------------------*/

bool QFile::open( int m )			// open file
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
    setStatus( 0 );
    if ( !(isReadable() || isWritable()) ) {
#if defined(CHECK_RANGE)
	warning( "QFile::open: File access not specified" );
#endif
	return FALSE;
    }
    bool ok = TRUE;
    if ( isRaw() ) {				// raw file I/O
	int oflags = 0;
	if ( isReadable() )
	    oflags |= OPEN_RDONLY;
	if ( flags() & IO_Append ) {		// append to end of file?
	    if ( flags() & IO_Truncate )
		oflags |= (OPEN_CREAT | OPEN_TRUNC);
	    else
		oflags |= (OPEN_APPEND | OPEN_CREAT);
	    setFlags( flags() | IO_WriteOnly ); // append implies write
	}
	else if ( isWritable() )		// create/trunc if writable
	    oflags |= (OPEN_CREAT | OPEN_TRUNC);//   but not append
	if ( isWritable() )
	    oflags |= isReadable() ? OPEN_WRONLY : OPEN_RDWR;
#if defined(HAS_TEXT_FILEMODE) && !defined(_OS_MAC_)
	if ( isTranslated() )
	    oflags |= OPEN_TEXT;
#endif
#if defined(HAS_ASYNC_FILEMODE)
	if ( isAsynchronous() )
	    oflags |= OPEN_ASYNC;
#endif
	fd = OPEN( (const char *)fn, oflags, 0666 );
	if ( fd != -1 ) {			// open successful
	    length = LSEEK( fd, 0, SEEK_END );	// get size of file
	    if ( !(flags() & IO_Append) )	// reset file position
		LSEEK( fd, 0, SEEK_SET );
	}
	else
	    ok = FALSE;
    }
    else {					// buffered file I/O
	const char *perm = 0;
	char perm2[4];
	bool try_create = FALSE;
	if ( flags() & IO_Append ) {		// append to end of file?
	    setFlags( flags() | IO_WriteOnly ); // append implies write
	    perm = isReadable() ? "a+" : "a";
	}
	else {
	    if ( isReadWrite() ) {
		if ( flags() & IO_Truncate )
		    perm = "w+";
		else {
		    perm = "r+";
		    try_create = TRUE;		// try to create if not exists
		}
	    }
	    else if ( isReadable() )
		perm = "r";
	    else if ( isWritable() )
		perm = "w";
	}
	strcpy( perm2, perm );
#if defined(HAS_TEXT_FILEMODE)
	if ( isTranslated() )
	    strcat( perm2, "t" );
	else
	    strcat( perm2, "b" );
#endif
	fh = fopen( (const char *)fn, perm2 );
	if ( !fh && try_create ) {
	    perm2[0] = 'w';			// try "w+" instead of "r+"
	    fh = fopen( (const char *)fn, perm2 );
	}
	if ( fh ) {
	    if ( flags() & IO_Append ) {	// index is at end of file
		length = ftell( fh );
		index = length;
	    }
	    else {				// calc size of file
		fseek( fh, 0, SEEK_END );
		length = ftell( fh );
		fseek( fh, 0, SEEK_SET );
		index = 0;
	    }
	}
	else
	    ok = FALSE;
    }
    if ( !ok ) {
	if ( errno == EMFILE )			// no more file handles/descrs
	    setStatus( IO_ResourceError );
	else
	    setStatus( IO_OpenError );
    }
    setState( IO_Open );
    return ok;
}

/*----------------------------------------------------------------------------
  Opens a file in the mode \e m using an existing file handle \e f.
  Returns TRUE if successful, otherwise FALSE.

  Example:
  \code
    #include <stdio.h>

    void printError( const char *msg )
    {
        QFile f;
	f.open( IO_WriteOnly, stderr );
	f.writeBlock( msg, strlen(msg) );	// write to stderr
	f.close();
    }
  \endcode

  \sa close()
 ----------------------------------------------------------------------------*/

bool QFile::open( int m, FILE *f )		// open file, using file handle
{
    if ( isOpen() ) {
#if defined(CHECK_RANGE)
	warning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    init();
    setMode( m );
    setState( IO_Open );
    fh = f;
    length = LONG_MAX;				// file might be stdin etc.
    return TRUE;
}

/*----------------------------------------------------------------------------
  Opens a file in the mode \e m using an existing file descriptor \e f.
  Returns TRUE if successful, otherwise FALSE.

  \sa close()
 ----------------------------------------------------------------------------*/

bool QFile::open( int m, int f )		// open file, using file descr
{
    if ( isOpen() ) {
#if defined(CHECK_RANGE)
	warning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    init();
    setMode( m );
    setState( IO_Open );
    fd = f;
    length = LONG_MAX;				// file might be stdin etc.
    return TRUE;
}

/*----------------------------------------------------------------------------
  Closes an open file.  The file will be closed even if it was opened with
  an existing file handle or file descriptor.
 ----------------------------------------------------------------------------*/

void QFile::close()				// close file
{
    if ( !isOpen() )				// file is not open
	return;
    if ( fh )					// buffered file
	fclose( fh );
    else					// raw file
	CLOSE( fd );
    init();					// restore internal state
}

/*----------------------------------------------------------------------------
  Flushes the file buffer to the disk.

  Calling close() will also flush.
 ----------------------------------------------------------------------------*/

void QFile::flush()				// flush file
{
    if ( isOpen() && fh )			// can only flush open/buffered
	fflush( fh );				//   file
}


/*----------------------------------------------------------------------------
  Returns the file size.
  \sa at()
 ----------------------------------------------------------------------------*/

long QFile::size() const
{
    if ( isOpen() ) {
        return length;
    } else {
	QFile f( fn );
	long  s = 0;
	if ( f.open(IO_ReadOnly) ) {
	    s = f.size();
	    f.close();
	}
	return s;
    }
}

/*----------------------------------------------------------------------------
  Returns the file index.
  \sa size()
 ----------------------------------------------------------------------------*/

long QFile::at() const
{
    return index;
}

/*----------------------------------------------------------------------------
  Sets the file index to \e n.  Returns TRUE if successful, otherwise FALSE.

  Example:
  \code
    QFile f( "data.bin" );
    f.open( IO_ReadOnly );			// index set to 0
    f.at( 100 );				// set index to 100
    f.at( f.at()+50 );				// set index to 150
    f.at( f.size()-80 );			// set index to 80 before EOF
    f.close();
  \endcode

  \sa size()
 ----------------------------------------------------------------------------*/

bool QFile::at( long n )
{
    if ( !isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QFile::at: File is not open" );
#endif
	return FALSE;
    }
    bool ok = TRUE;
    if ( isRaw() ) {				// raw file
	if ( LSEEK( fd, n, SEEK_SET ) == -1 )
	    ok = FALSE;
    }
    else {					// buffered file
	if ( fseek( fh, n, SEEK_SET ) != 0 )
	    ok = FALSE;
    }
    if ( ok )
	index = n;
#if defined(CHECK_RANGE)
    else
	warning( "QFile::at: Cannot set file position %ld", n );
#endif
    return ok;
}


/*----------------------------------------------------------------------------
  Reads at most \e len bytes from the file into \e p and
  returns the number of bytes actually read.

  Returns -1 if a serious error occurred.

  \sa writeBlock()
 ----------------------------------------------------------------------------*/

int QFile::readBlock( char *p, uint len )
{
#if defined(CHECK_STATE)
    CHECK_PTR( p );
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
    if ( isRaw() )				// raw file
	nread = READ( fd, p, len );
    else					// buffered file
	nread = fread( p, 1, len, fh );
    index += nread;
    return nread;
}

/*----------------------------------------------------------------------------
  Writes \e len bytes from \e p to the file and returns the number of
  bytes actually written.

  Returns -1 if a serious error occurred.

  \sa readBlock()
 ----------------------------------------------------------------------------*/

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
	    index = LSEEK( fd, 0, SEEK_CUR );
	else
	    index = fseek( fh, 0, SEEK_CUR );
    }
    else
	index += nwritten;
    if ( index > length )			// update file length
	length = index;
    return nwritten;
}


/*----------------------------------------------------------------------------
  Reads a line of text.

  Reads bytes from the file until end-of-line is reached, or up to
  \e maxlen bytes.
  This function is efficient only for buffered files.  Avoid
  readLine() for files that have been opened with the \c IO_Raw
  flag.

  \sa readBlock()
 ----------------------------------------------------------------------------*/

int QFile::readLine( char *p, uint maxlen )
{
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
    if ( isRaw() )				// raw file
	nread = QIODevice::readLine( p, maxlen );
    else {					// buffered file
	p = fgets( p, maxlen, fh );
	nread = p ? strlen( p ) : 0;
    }
    index += nread;
    return nread;
}


/*----------------------------------------------------------------------------
  Reads a single byte/character from the file.

  Returns -1 if the end of file has been reached.

  \sa putch(), ungetch()
 ----------------------------------------------------------------------------*/

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
    if ( isRaw() ) {				// raw file (inefficient)
	char buf[1];
	ch = readBlock( buf, 1 ) == 1 ? buf[0] : EOF;
    }
    else {					// buffered file
	if ( (ch = getc( fh )) != EOF )
	    index++;
    }
    return ch;
}

/*----------------------------------------------------------------------------
  Writes the character \e ch to the file.

  Returns \e ch, or -1 if some error occurred.

  \sa getch(), ungetch()
 ----------------------------------------------------------------------------*/

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
    }
    else {					// buffered file
	if ( (ch = putc( ch, fh )) != EOF ) {
	    index++;
	    if ( index > length )		// update file length
		length = index;
	}
    }
    return ch;
}

/*----------------------------------------------------------------------------
  Puts the character \e ch back into the file.

  This function is normally called to "undo" a getch() operator.

  Returns \e ch, or -1 if some error occurred.

  \sa getch(), putch()
 ----------------------------------------------------------------------------*/

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
    if ( isRaw() ) {				// raw file (very inefficient)
	char buf[1];
	at( index-1 );
	buf[0] = ch;
	if ( writeBlock( buf, 1 ) == 1 )
	    at ( index-1 );
	else
	    ch = EOF;
    }
    else {					// buffered file
	if ( (ch = ungetc( ch, fh )) != EOF )
	    index--;
    }
    return ch;
}

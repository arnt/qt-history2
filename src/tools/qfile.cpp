/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfile.cpp#14 $
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
#include "qfileinf.h"
#include "qdir.h"
#if !defined(_OS_MAC_)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <errno.h>
#if defined(UNIX)
#include <unistd.h>
#endif
#if defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
#include <io.h>
#endif
#include <limits.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qfile.cpp#14 $";
#endif

/*! \class QFile qfile.h

  \brief The QFile class provides system-independent file access and
  related functions.

  \ingroup tools

  This class is not yet documented.  Our <a
  href=http://www.troll.no/>home page</a> contains a pointer to the
  current version of Qt. */


QFile::QFile()
{
    init();
}

QFile::QFile( const char *fullPathFileName )
{
    init();
    fn = fullPathFileName;
}

QFile::QFile( const QDir &d, const char *fileName )
{
    init();
    fn = d.fullPathName( fileName );
}

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

void QFile::setFileName( const char *fileName ) // set file name
{
    if ( isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QFile::setFileName: File is open" );
#endif
        close();
    }
    fn = fileName;
}

void QFile::setFileName(  const QDir &d, const char *fileName )// set file name
{
    setFileName(  d.fullPathName( fileName ) );
}

#undef STATBUF
#undef STAT
#undef STAT_REG
#undef STAT_DIR
#undef STAT_LNK
#undef OPEN
#undef CLOSE
#undef LSEEK
#undef READ
#undef WRITE
#undef OPEN_RDONLY
#undef OPEN_WRONLY
#undef OPEN_CREAT
#undef OPEN_TRUNC
#undef OPEN_APPEND
#undef OPEN_TEXT

#if defined(_CC_MSC_) || defined(_CC_SYM_)
#define STATBUF	 _stat				// non-ANSI defs
#define STAT	 _stat
#define STAT_REG _S_IFREG
#define STAT_DIR _S_IFDIR
#if defined(_S_IFLNK)
#define STAT_LNK _S_IFLNK
#endif
#define OPEN	::_open
#define CLOSE	::_close
#define LSEEK	_lseek
#define READ	_read
#define WRITE	_write
#define OPEN_RDONLY	_O_RDONLY
#define OPEN_WRONLY	_O_WRONLY
#define OPEN_RDWR	_O_RDWR
#define OPEN_CREAT	_O_CREAT
#define OPEN_TRUNC	_O_TRUNC
#define OPEN_APPEND	_O_APPEND
#define OPEN_TEXT	_O_TEXT

#else						// all other systems

#define STATBUF	 stat
#define STAT	 ::stat
#define STAT_REG S_IFREG
#define STAT_DIR S_IFDIR
#if defined(S_IFLNK)
#define STAT_LNK S_IFLNK
#endif
#define OPEN	::open
#define CLOSE	::close
#define LSEEK	::lseek
#define READ	::read
#define WRITE	::write
#define OPEN_RDONLY	O_RDONLY
#define OPEN_WRONLY	O_WRONLY
#define OPEN_RDWR	O_RDWR
#define OPEN_CREAT	O_CREAT
#define OPEN_TRUNC	O_TRUNC
#define OPEN_APPEND	O_APPEND
#if defined(O_TEXT)
#define OPEN_TEXT	O_TEXT
#endif
#endif

long QFile::get_stat( bool lnk ) const		// get file stat, 0 if error
{
#if defined(_OS_MAC_)
    return 0;
#else
    if ( fn.isNull() )				// no filename specified
	return 0;
    struct STATBUF st;
#if defined(_OS_LINUX_)
    if ( lnk )					// detect if symlink
	return lstat(fn.data(), &st)==0 ? st.st_mode : 0;
#else
    lnk = lnk;
#endif
    return STAT(fn.data(), &st)==0 ? st.st_mode : 0;
#endif
}

bool QFile::exists() const			// test if current file exists
{
    return get_stat() != 0;
}

bool QFile::isFile() const			// is it a regular file?
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    return (get_stat() & STAT_REG) == STAT_REG;
#endif
}

bool QFile::isDir() const			// is it a directory?
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    return (get_stat() & STAT_DIR) == STAT_DIR;
#endif
}

bool QFile::isSymLink() const			// is it a symbolic link?
{
#if defined(_OS_MAC_)
    return FALSE;
#else
#if defined(UNIX) || defined(STAT_LNK)
    return (get_stat(TRUE) & STAT_LNK) == STAT_LNK;
#else
    return FALSE;
#endif
#endif
}


#if defined(UNIX)
#define FILENAME_CASE 1				// case-sensisive filenames
#else
#define FILENAME_CASE 0				// case-insensitive filenames
#endif

bool QFile::remove()	// remove file
{
    close();					// close file if open
    return remove( fn.data()  );
}

bool QFile::remove( const QDir &d, const char *fileName )  // remove file
{
    return remove( d.fullPathName( fileName ) );
}

bool QFile::remove( const char *fileName )	// remove file
{
    if ( fileName == 0 || fileName[0] == '\0' ) {
#if defined(CHECK_NULL)
	warning( "QFile::remove: Empty or NULL file name." );
#endif
	return FALSE;
    }
#if defined(UNIX)
    return unlink( fileName ) == 0;		// unlink more common in UNIX
#else
    return ::remove( fileName ) == 0;		// use standard ANSI remove
#endif
}

bool QFile::exists( const char *fullPathFileName )
{
    QFile f( fullPathFileName );
    return f.exists();
}

bool QFile::exists( const QDir &d, const char *fileName )
{
    return exists( d.fullPathName( fileName ) );
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
	fd = OPEN( (const char *)fn, oflags );
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

void QFile::flush()				// flush file
{
    if ( isOpen() && fh )			// can only flush open/buffered
	fflush( fh );				//   file
}


long QFile::size() const			// get file size
{
    if ( isOpen() ) {
        return length;
    } else {
        QFileInfo fi( *this );
        return fi,size();
    }
}

long QFile::at() const				// get file position
{
    return index;
}

bool QFile::at( long n )			// set file position
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


int QFile::readBlock( char *p, uint len )	// read data from file
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

int QFile::writeBlock( const char *p, uint len ) // write data to file
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

int QFile::readLine( char *p, uint maxlen )	// read data from file
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


int QFile::getch()				// get next char
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

int QFile::putch( int ch )			// put char
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

int QFile::ungetch( int ch )			// put back char
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

/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.cpp#61 $
**
** Implementation of QFileInfo class
**
** Created : 950628
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
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
    } else if ( s.length() > 3 && s[2] == '/' && s[3] == '/' ) {
	QString ss(s);
	ss[2] = '\\';
	ss.remove( 3, 1 );
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
	return ::_taccess((TCHAR*)qt_winTchar(fn,TRUE), t) == 0;
    else
	return QT_ACCESS(qt_win95Name(fn), t) == 0;
}

bool QFile::remove( const QString &fileName )
{
    if ( fileName.isEmpty() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QFile::remove: Empty or null file name" );
#endif
	return FALSE;
    }
    // use standard ANSI remove
    if ( qt_winunicode )
	return ::_tremove((const TCHAR*)qt_winTchar(fileName,TRUE)) == 0;
    else
	return ::remove(qt_win95Name(fileName)) == 0;
}

#define HAS_TEXT_FILEMODE

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
    if ( isRaw() ) {				// raw file I/O
	int oflags = QT_OPEN_RDONLY;
	if ( isReadable() && isWritable() )
	    oflags = QT_OPEN_RDWR;
	else if ( isWritable() )
	    oflags = QT_OPEN_WRONLY;
	if ( flags() & IO_Append ) {		// append to end of file?
	    if ( flags() & IO_Truncate )
		oflags |= (QT_OPEN_CREAT | QT_OPEN_TRUNC);
	    else
		oflags |= (QT_OPEN_APPEND | QT_OPEN_CREAT);
	    setFlags( flags() | IO_WriteOnly ); // append implies write
	} else if ( isWritable() ) {		// create/trunc if writable
	    if ( flags() & IO_Truncate )
		oflags |= (QT_OPEN_CREAT | QT_OPEN_TRUNC);
	    else
		oflags |= QT_OPEN_CREAT;
	}
#if defined(HAS_TEXT_FILEMODE)
	if ( isTranslated() )
	    oflags |= QT_OPEN_TEXT;
	else
	    oflags |= QT_OPEN_BINARY;
#endif
#if defined(HAS_ASYNC_FILEMODE)
	if ( isAsynchronous() )
	    oflags |= QT_OPEN_ASYNC;
#endif
	if ( qt_winunicode ) {
	    fd = ::_topen((const TCHAR*)qt_winTchar(fn,TRUE), oflags, 0666 );
	} else {
	    fd = QT_OPEN(qt_win95Name(fn), oflags, 0666 );
	}

	if ( fd != -1 ) {			// open successful
	    QT_STATBUF st;
	    QT_FSTAT( fd, &st );
	    if ( (st.st_mode& QT_STAT_MASK) == QT_STAT_DIR ) {
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
	qstrcpy( perm2, perm );
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
		fh = ::_tfopen((const TCHAR*)qt_winTchar(fn,TRUE), tperm2 );
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
	    QT_STATBUF st;
	    QT_FSTAT( QT_FILENO(fh), &st );
	    if ( (st.st_mode& QT_STAT_MASK) == QT_STAT_DIR ) {
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
    QT_STATBUF st;
    QT_FSTAT( QT_FILENO(fh), &st );
    ioIndex = (int)ftell( fh );
    if ( (st.st_mode & QT_STAT_MASK) != QT_STAT_REG ) {
	// non-seekable
	setType( IO_Sequential );
	length = INT_MAX;
    } else {
	length = (int)st.st_size;
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
    QT_STATBUF st;
    QT_FSTAT( fd, &st );
    ioIndex  = (int)QT_LSEEK(fd, 0, SEEK_CUR);
    if ( (st.st_mode & QT_STAT_MASK) != QT_STAT_REG ) {
	// non-seekable
	setType( IO_Sequential );
	length = INT_MAX;
    } else {
	length = (int)st.st_size;
    }
    return TRUE;
}

Q_ULONG QFile::size() const
{
    QT_STATBUF st;
    if ( isOpen() ) {
	QT_FSTAT( fh ? QT_FILENO(fh) : fd, &st );
    } else {
	if ( qt_winunicode ) {
	    ::_tstat((const TCHAR*)qt_winTchar(fn,TRUE), (QT_STATBUF4TSTAT*)&st);
	} else {
	    QT_STAT(qt_win95Name(fn), &st);
	}
    }
    return st.st_size;
}

bool QFile::at( Q_ULONG pos )
{
    if ( !isOpen() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QFile::at: File is not open" );
#endif
	return FALSE;
    }
    bool okay;
    if ( isRaw() ) {				// raw file
	pos = (int)QT_LSEEK(fd, pos, SEEK_SET);
	okay = pos != -1;
    } else {					// buffered file
	okay = fseek(fh, pos, SEEK_SET) == 0;
    }
    if ( okay )
	ioIndex = pos;
#if defined(QT_CHECK_RANGE)
    else
	qWarning( "QFile::at: Cannot set file position %d", pos );
#endif
    return okay;
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
    Q_ULONG nread = 0;					// number of bytes read
    if ( !ungetchBuffer.isEmpty() ) {
	// need to add these to the returned string.
	Q_ULONG l = ungetchBuffer.length();
	while( nread < l ) {
	    *p = ungetchBuffer[ int(l - nread - 1) ];
	    p++;
	    nread++;
	}
	ungetchBuffer.truncate( l - nread );
    }

    if( nread < (int)len ) {
	if ( isRaw() ) {				// raw file
	    nread += QT_READ( fd, p, len - nread );
	    if ( len && nread <= 0 ) {
		nread = 0;
		setStatus(IO_ReadError);
	    }
	} else {					// buffered file
	    nread += fread( p, 1, len - nread, fh );
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
    Q_ULONG nwritten;				// number of bytes written
    if ( isRaw() )				// raw file
	nwritten = QT_WRITE( fd, p, len );
    else					// buffered file
	nwritten = fwrite( p, 1, len, fh );
    if ( nwritten != (int)len ) {		// write error
	if ( errno == ENOSPC )			// disk is full
	    setStatus( IO_ResourceError );
	else
	    setStatus( IO_WriteError );
	if ( isRaw() )				// recalc file position
	    ioIndex = (int)QT_LSEEK( fd, 0, SEEK_CUR );
	else
	    ioIndex = fseek( fh, 0, SEEK_CUR );
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
	return QT_FILENO( fh );
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
		ok = QT_CLOSE( fd ) != -1;
	}
	init();					// restore internal state
    }
    if (!ok)
	setStatus (IO_UnspecifiedError);

    return;
}

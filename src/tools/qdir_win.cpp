/****************************************************************************
** $Id$
**
** Implementation of QDir class
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

#include "qdir.h"
#include "qdir_p.h"
#include "qnamespace.h"
#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qregexp.h"
#include "qstringlist.h"

#include <windows.h>

#if defined(Q_OS_OS2EMX)
extern "C" Q_UINT32 DosQueryCurrentDisk(Q_UINT32*,Q_UINT32*);
#define NO_ERROR 0
#endif

#if defined(Q_FS_FAT) || defined(Q_OS_OS2EMX)

void QDir::slashify( QString& n )
{
    if ( n.isNull() )
	return;
    for ( int i=0; i<(int)n.length(); i++ ) {
	if ( n[i] ==  '\\' )
	    n[i] = '/';
    }
}

extern QCString qt_win95Name(const QString s);

#else

static void QDir::slashify( QString& n)
{
    return;
}

#endif

QString QDir::homeDirPath()
{
    QString d;
    d = QString::fromLatin1( getenv("HOME") );
    if ( d.isEmpty() || !QFile::exists( d ) ) {
	d = QString::fromLatin1( getenv("USERPROFILE") );
	if ( d.isEmpty() || !QFile::exists( d ) ) {
	    d = QString::fromLatin1( getenv("HOMEDRIVE") ) + QString::fromLatin1( getenv("HOMEPATH") );
	    if ( d.isEmpty() || !QFile::exists( d ) )
		d = rootDirPath();
	}
    }

    slashify( d );
    return d;
}

/*!
  Returns the canonical path, i.e. a path without symbolic links or
  redundant "." or ".." elements.

  On systems that do not have symbolic links this function will always
  return the same string that absPath returns.  If the canonical path
  does not exist (normally due to dangling symbolic links)
  canonicalPath() returns a null string.

  \sa path(), absPath(), exists(), cleanDirPath(), dirName(),
      absFilePath(), QString::isNull()
*/

QString QDir::canonicalPath() const
{
    QString r;

    char cur[PATH_MAX];
    QT_GETCWD( cur, PATH_MAX );
    if ( qt_winunicode ) {
	if ( ::_tchdir((TCHAR*)qt_winTchar(dPath,TRUE)) >= 0 ) {
	    TCHAR tmp[PATH_MAX];
	    if ( ::_tgetcwd( tmp, PATH_MAX ) )
		r = qt_winQString(tmp);
	}
    } else {
	if ( QT_CHDIR(qt_win95Name(dPath)) >= 0 ) {
	    char tmp[PATH_MAX];
	    if ( QT_GETCWD( tmp, PATH_MAX ) )
		r = tmp;
	}
    }
    QT_CHDIR( cur );
    slashify( r );
    return r;
}

/*!
  Creates a directory.

  If \a acceptAbsPath is TRUE a path starting with a separator ('/')
  will create the absolute directory, if \a acceptAbsPath is FALSE
  any number of separators at the beginning of \a dirName will be removed.

  Returns TRUE if successful, otherwise FALSE.

  \sa rmdir()
*/

bool QDir::mkdir( const QString &dirName, bool acceptAbsPath ) const
{
    if ( qt_winunicode ) {
	return ::_tmkdir((const TCHAR*)qt_winTchar(filePath(dirName,acceptAbsPath),TRUE)) == 0;
    } else {
	return _mkdir(qt_win95Name(filePath(dirName,acceptAbsPath))) == 0;
    }
}

/*!
  Removes a directory.

  If \a acceptAbsPath is TRUE a path starting with a separator ('/')
  will remove the absolute directory, if \a acceptAbsPath is FALSE
  any number of separators at the beginning of \a dirName will be removed.

  The directory must be empty for rmdir() to succeed.

  Returns TRUE if successful, otherwise FALSE.

  \sa mkdir()
*/

bool QDir::rmdir( const QString &dirName, bool acceptAbsPath ) const
{
    if ( qt_winunicode ) {
	return ::_trmdir((const TCHAR*)qt_winTchar(filePath(dirName,acceptAbsPath),TRUE)) == 0;
    } else {
	return _rmdir(qt_win95Name(filePath(dirName,acceptAbsPath))) == 0;
    }
}


/*!
  Returns TRUE if the directory is readable AND we can open files by
  name. This function will return FALSE if only one of these is present.
  \warning A FALSE value from this function is not a guarantee that files
  in the directory are not accessible.

  \sa QFileInfo::isReadable()
*/

bool QDir::isReadable() const
{
    if ( qt_winunicode ) {
	return ::_taccess((const TCHAR*)qt_winTchar(dPath,TRUE), R_OK) == 0;
    } else {
	return QT_ACCESS(qt_win95Name(dPath), R_OK) == 0;
    }
}

/*!
  Returns TRUE if the directory is the root directory, otherwise FALSE.

  Note: If the directory is a symbolic link to the root directory this
  function returns FALSE. If you want to test for this you can use
  canonicalPath():

  Example:
  \code
    QDir d( "/tmp/root_link" );
    d = d.canonicalPath();
    if ( d.isRoot() )
	qWarning( "It IS a root link!" );
  \endcode

  \sa root(), rootDirPath()
*/

bool QDir::isRoot() const
{
    return dPath == "/" || dPath == "//" ||
	(dPath[0].isLetter() && dPath.mid(1,dPath.length()) == ":/");
}

/*!
  Renames a file or directory.

  If \a acceptAbsPaths is TRUE a path starting with a separator ('/')
  will rename the file with the absolute path, if \a acceptAbsPaths is FALSE
  any number of separators at the beginning of the names will be removed.

  Returns TRUE if successful; otherwise returns FALSE.

  On most file systems, rename() fails only if \a oldName does not exist
  or if \a newName and \a oldName are not on the same partition.
  On Windows, rename() will fail if \a newName already exists.  However,
  there are also other reasons why rename() can fail.  For example, on
  at least one file system rename() fails if newName points to an open
  file.
*/

bool QDir::rename( const QString &oldName, const QString &newName,
		   bool acceptAbsPaths	)
{
    if ( oldName.isEmpty() || newName.isEmpty() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QDir::rename: Empty or null file name" );
#endif
	return FALSE;
    }
    QString fn1 = filePath( oldName, acceptAbsPaths );
    QString fn2 = filePath( newName, acceptAbsPaths );
    if ( qt_winunicode ) {
	TCHAR* t2 = (TCHAR*)qt_winTchar_new(fn2);
	bool r = ::_trename((const TCHAR*)qt_winTchar(fn1,TRUE), t2) == 0;
	delete [] t2;
	return r;
    } else {
	return ::rename(qt_win95Name(fn1), qt_win95Name(fn2)) == 0;
    }
}
/*!
  Sets the application's current working directory to \a path.
  Returns TRUE if the directory was successfully changed; otherwise
  returns FALSE.
*/


bool QDir::setCurrent( const QString &path )
{
    int r;

    if ( qt_winunicode ) {
	r = ::_tchdir((const TCHAR*)qt_winTchar(path,TRUE));
    } else {
	r = QT_CHDIR(qt_win95Name(path));
    }

    return r >= 0;
}

/*!
  Returns the absolute path of the application's current directory.
  \sa current()
*/

QString QDir::currentDirPath()
{
    QString result;

    if ( qt_winunicode ) {
	TCHAR currentName[PATH_MAX];
	if ( ::_tgetcwd(currentName,PATH_MAX) != 0 ) {
	    result = qt_winQString(currentName);
	}
    } else {
	char currentName[PATH_MAX];
	if ( QT_GETCWD(currentName,PATH_MAX) != 0 ) {
	    result = QString::fromLatin1(currentName);
	}
    }
    slashify( result );

    return result;
}

/*!
  Returns the absolute path for the root directory.

  For UNIX operating systems this returns "/". For Windows file
  systems this returns "c:/".

  \sa root() drives()
*/


QString QDir::rootDirPath()
{
#if defined(Q_FS_FAT)
    QString d = QString::fromLatin1( getenv("SystemDrive") );
    if ( d.isEmpty() )
	d = "c:";
    d = d + "/";
#elif defined(Q_OS_OS2EMX)
    char dir[4];
    _abspath( dir, "/", _MAX_PATH );
    QString d( dir );
#endif
    return d;
}

/*!
  Returns TRUE if \a path is relative; returns FALSE if it is absolute.
  \sa isRelative()
*/

bool QDir::isRelativePath( const QString &path )
{
    int len = path.length();
    if ( len == 0 )
	return TRUE;

    int i = 0;
    if ( path[0].isLetter() && path[1] == ':' )		// drive, e.g. a:
	i = 2;
    return path[i] != '/' && path[i] != '\\';
}

/*!
  \internal
  Reads directory entries.
*/

bool QDir::readDirEntries( const QString &nameFilter,
			   int filterSpec, int sortSpec )
{
    int i;

    QStringList filters = qt_makeFilterList( nameFilter );

    bool doDirs	    = (filterSpec & Dirs)	!= 0;
    bool doFiles    = (filterSpec & Files)	!= 0;
    bool noSymLinks = (filterSpec & NoSymLinks) != 0;
    bool doReadable = (filterSpec & Readable)	!= 0;
    bool doWritable = (filterSpec & Writable)	!= 0;
    bool doExecable = (filterSpec & Executable) != 0;
    bool doHidden   = (filterSpec & Hidden)	!= 0;

    // show hidden files if the user asks explicitly for e.g. .*
    if ( !doHidden && !nameFilter.isEmpty() && nameFilter[0] == '.' )
	doHidden = TRUE;
    bool doModified = (filterSpec & Modified)	!= 0;
    bool doSystem   = (filterSpec & System)	!= 0;

    //QRegExp   wc( nameFilter, FALSE, TRUE );	// wild card, case insensitive
    bool      first = TRUE;
    QString   p = dPath.copy();
    int	      plen = p.length();
    HANDLE    ff;
    WIN32_FIND_DATA finfo;
    QFileInfo fi;

#undef	IS_SUBDIR
#undef	IS_RDONLY
#undef	IS_ARCH
#undef	IS_HIDDEN
#undef	IS_SYSTEM
#undef	FF_ERROR

#define IS_SUBDIR   FILE_ATTRIBUTE_DIRECTORY
#define IS_RDONLY   FILE_ATTRIBUTE_READONLY
#define IS_ARCH	    FILE_ATTRIBUTE_ARCHIVE
#define IS_HIDDEN   FILE_ATTRIBUTE_HIDDEN
#define IS_SYSTEM   FILE_ATTRIBUTE_SYSTEM
#define FF_ERROR    INVALID_HANDLE_VALUE

    if ( plen == 0 ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QDir::readDirEntries: No directory name specified" );
#endif
	return FALSE;
    }
    if ( p.at(plen-1) != '/' && p.at(plen-1) != '\\' )
	p += '/';
    p += QString::fromLatin1("*.*");

#ifndef Q_OS_TEMP
    if ( qt_winunicode ) {
#endif
	ff = FindFirstFile((TCHAR*)qt_winTchar(p,TRUE),&finfo);
#ifndef Q_OS_TEMP
    } else {
	// Cast is safe, since char is at end of WIN32_FIND_DATA
	ff = FindFirstFileA(qt_win95Name(p),(WIN32_FIND_DATAA*)&finfo);
    }
#endif
    if ( ff == FF_ERROR ) {
	// if it is a floppy disk drive, it might just not have a file on it
	if ( plen > 1 && p[1] == ':' &&
		( p[0]=='A' || p[0]=='a' || p[0]=='B' || p[0]=='b' ) ) {
	    if ( !fiList ) {
		fiList = new QFileInfoList;
		Q_CHECK_PTR( fiList );
		fiList->setAutoDelete( TRUE );
	    } else {
		fiList->clear();
	    }
	    return TRUE;
	}
#ifndef QT_NO_CODECS
#if defined(QT_CHECK_RANGE)
	qWarning( "QDir::readDirEntries: Cannot read the directory: %s (UTF8)",
		  dPath.utf8().data() );
#endif
#endif
	return FALSE;
    }

    if ( !fiList ) {
	fiList = new QFileInfoList;
	Q_CHECK_PTR( fiList );
	fiList->setAutoDelete( TRUE );
    } else {
	fiList->clear();
    }

    for ( ;; ) {
	if ( first )
	    first = FALSE;
	else {
#ifndef Q_OS_TEMP
	    if ( qt_winunicode ) {
#endif
		if ( !FindNextFile(ff,&finfo) )
		    break;
#ifndef Q_OS_TEMP
	    } else {
		if ( !FindNextFileA(ff,(WIN32_FIND_DATAA*)&finfo) )
		    break;
	    }
#endif
	}
	int  attrib = finfo.dwFileAttributes;
	bool isDir	= (attrib & IS_SUBDIR) != 0;
	bool isFile	= !isDir;
	bool isSymLink	= FALSE;
	bool isReadable = TRUE;
	bool isWritable = (attrib & IS_RDONLY) == 0;
	bool isExecable = FALSE;
	bool isModified = (attrib & IS_ARCH)   != 0;
	bool isHidden	= (attrib & IS_HIDDEN) != 0;
	bool isSystem	= (attrib & IS_SYSTEM) != 0;

	QString fname;
	if ( qt_winunicode ) {
	    fname = qt_winQString(finfo.cFileName);
	} else {
	    fname = qt_winMB2QString((const char*)finfo.cFileName);
	}
	if ( !match( filters, fname ) && !(allDirs && isDir) )
	    continue;

	if  ( (doDirs && isDir) || (doFiles && isFile) ) {
	    QString name = fname;
	    slashify(name);
	    if ( doExecable ) {
		QString ext = name.right(4).lower();
		if ( ext == ".exe" || ext == ".com" || ext == ".bat" ||
		     ext == ".pif" || ext == ".cmd" )
		    isExecable = TRUE;
	    }

	    if ( noSymLinks && isSymLink )
		continue;
	    if ( (filterSpec & RWEMask) != 0 )
		if ( (doReadable && !isReadable) ||
		     (doWritable && !isWritable) ||
		     (doExecable && !isExecable) )
		    continue;
	    if ( doModified && !isModified )
		continue;
	    if ( !doHidden && isHidden )
		continue;
	    if ( !doSystem && isSystem )
		continue;
	    fi.setFile( *this, name );
	    fiList->append( new QFileInfo( fi ) );
	}
    }
    FindClose( ff );

#undef	IS_SUBDIR
#undef	IS_RDONLY
#undef	IS_ARCH
#undef	IS_HIDDEN
#undef	IS_SYSTEM
#undef	FF_ERROR

    if ( !fList ) {
	fList  = new QStringList;
	Q_CHECK_PTR( fList );
    } else {
	fList->clear();
    }

    // Sort...
    QDirSortItem* si= new QDirSortItem[fiList->count()];
    QFileInfo* itm;
    i=0;
    for (itm = fiList->first(); itm; itm = fiList->next())
	si[i++].item = itm;
    qt_cmp_si_sortSpec = sortSpec;
    qsort( si, i, sizeof(si[0]), qt_cmp_si );
    // put them back in the list
    fiList->setAutoDelete( FALSE );
    fiList->clear();
    int j;
    for ( j=0; j<i; j++ ) {
	fiList->append( si[j].item );
	fList->append( si[j].item->fileName() );
    }
    delete [] si;
    fiList->setAutoDelete( TRUE );

    if ( filterSpec == (FilterSpec)filtS && sortSpec == (SortSpec)sortS &&
	 nameFilter == nameFilt )
	dirty = FALSE;
    else
	dirty = TRUE;
    return TRUE;
}


/*!
  Returns a list of the root directories on this system.  On
  win32, this returns a number of QFileInfo objects containing "C:/",
  "D:/" etc.  On other operating systems, it returns a list containing
  just one root directory (e.g. "/").

  The returned pointer is owned by Qt. Callers should \e not delete or
  modify it.
*/

const QFileInfoList * QDir::drives()
{
    // at most one instance of QFileInfoList is leaked, and this variable
    // points to that list
    static QFileInfoList * knownMemoryLeak = 0;

    if ( !knownMemoryLeak ) {
	knownMemoryLeak = new QFileInfoList;

#if defined(Q_OS_WIN32)
	Q_UINT32 driveBits = (Q_UINT32) GetLogicalDrives() & 0x3ffffff;
#elif defined(Q_OS_OS2EMX)
	Q_UINT32 driveBits, cur;
	if (DosQueryCurrentDisk(&cur,&driveBits) != NO_ERROR)
	    exit(1);
	driveBits &= 0x3ffffff;
#endif

	char driveName[4];

#ifndef Q_OS_TEMP
	qstrcpy( driveName, "a:/" );
#else
	qstrcpy( driveName, "/" );
#endif

	while( driveBits ) {
	    if ( driveBits & 1 )
		knownMemoryLeak->append(
		      new QFileInfo( QString::fromLatin1(driveName) ) );
	    driveName[0]++;
	    driveBits = driveBits >> 1;
	}
    }

    return knownMemoryLeak;
}

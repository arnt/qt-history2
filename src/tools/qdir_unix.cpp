/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.cpp#61 $
**
** Implementation of QDirclass
**
** Created : 950628
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qglobal.h"

#include "qdir.h"
#ifndef QT_NO_DIR

#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qregexp.h"
#include "qstringlist.h"
#include <stdlib.h>
#include <ctype.h>

extern QStringList qt_makeFilterList( const QString &filter );

extern int qt_cmp_si_sortSpec;

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

extern int qt_cmp_si( const void *, const void * );

#if defined(Q_C_CALLBACKS)
}
#endif


void QDir::slashify( QString& s )
{
#if defined(_OS_VMS_)
    if ( s[0] != QChar('/') )
	s.prepend( QChar('/') );
    int p0 = s.find( QChar('[') );
    int p1 = s.findRev( QChar(']') );
    if ( p0 >= 0 && p1 >= 0 && p0 < p1 ) {
	for( int i = p0; i <= p1; i++ ) {
	    QChar c = s[i];
	    if ( c == QChar('[') || c == QChar('.') || c == QChar(']') )
		s.replace( i, 1, QChar('/') );
	}
    }

    int j = s.find( QString::fromLatin1( "/.." ) );
    if ( j >= 0 ) {
	int k = s.findRev( QChar('/'), -s.length() + j - 1 );
	if ( k >= 0 )
	    s.truncate( k+1 );
    }
#else
    Q_UNUSED( s );
    return;
#endif
}

QString QDir::homeDirPath()
{
    QString d;
    d = QFile::decodeName(getenv("HOME"));
    slashify( d );
    if ( d.isNull() )
	d = rootDirPath();
    return d;
}

QString QDir::canonicalPath() const
{
    QString r;

    char cur[PATH_MAX];
    char tmp[PATH_MAX];
    GETCWD( cur, PATH_MAX );
    if ( CHDIR(QFile::encodeName(dPath)) >= 0 ) {
	GETCWD( tmp, PATH_MAX );
	r = QFile::decodeName(tmp);
    }
    CHDIR( cur );

    slashify( r );
    return r;
}

bool QDir::mkdir( const QString &dirName, bool acceptAbsPath ) const
{
    QString d( QFile::encodeName( filePath( dirName, acceptAbsPath ) ) );
#if defined(_OS_VMS_)
    d += QChar( ']' );
    for( int i = 0; i < (int)d.length(); i++ ) {
	if( d[i] == QChar( ']' ) ) {
	    d[i] = QChar( '.' );
	    break;
	}
    }
#endif
    return MKDIR( d, 0777 ) == 0;
}

bool QDir::rmdir( const QString &dirName, bool acceptAbsPath ) const
{
    return RMDIR( QFile::encodeName(filePath(dirName,acceptAbsPath)) ) == 0;
}

bool QDir::isReadable() const
{
    QString s( QFile::encodeName(dPath) );

#if defined(_OS_VMS_)
    if ( s.contains( QString::fromLatin1("[000000]") ) ) {
	s += QString::fromLatin1( ".dir;1" );
    }
    else if ( !s.contains( QString::fromLatin1( ".DIR;1" ) ) ) {
	s += QString::fromLatin1( ".dir;1" );
    }
#endif

    return ACCESS( s.latin1(), R_OK | X_OK ) == 0;
}

bool QDir::isRoot() const
{
    return dPath == QString::fromLatin1("/");
}

bool QDir::rename( const QString &name, const QString &newName,
		   bool acceptAbsPaths	)
{
    if ( name.isEmpty() || newName.isEmpty() ) {
#if defined(CHECK_NULL)
	qWarning( "QDir::rename: Empty or null file name(s)" );
#endif
	return FALSE;
    }
    QCString fn1 = QFile::encodeName( filePath( name, acceptAbsPaths ) );
    QCString fn2 = QFile::encodeName( filePath( newName, acceptAbsPaths ) );
#if defined(_OS_VMS_)
    fn1 += ".dir";
    fn2 += ".dir";
#endif
    return ::rename( fn1, fn2 ) == 0;
}

bool QDir::setCurrent( const QString &path )
{
    int r;
    r = CHDIR( QFile::encodeName(path) );
    return r >= 0;
}

QString QDir::currentDirPath()
{
    QString result;

    STATBUF st;
    if ( STAT( ".", &st ) == 0 ) {
	char currentName[PATH_MAX];
	if ( GETCWD( currentName, PATH_MAX ) != 0 )
	    result = QFile::decodeName(currentName);
#if defined(DEBUG)
	if ( result.isNull() )
	    qWarning( "QDir::currentDirPath: getcwd() failed" );
#endif
    } else {
#if defined(DEBUG)
	qWarning( "QDir::currentDirPath: stat(\".\") failed" );
#endif
    }
    slashify( result );
    return result;
}

QString QDir::rootDirPath()
{
    QString d = QString::fromLatin1( "/" );
    return d;
}

bool QDir::isRelativePath( const QString &path )
{
    int len = path.length();
    if ( len == 0 )
	return TRUE;
    return path[0] != '/';
}

bool QDir::readDirEntries( const QString &nameFilter,
			   int filterSpec, int sortSpec )
{
    int i;
    if ( !fList ) {
	fList  = new QStringList;
	CHECK_PTR( fList );
	fiList = new QFileInfoList;
	CHECK_PTR( fiList );
	fiList->setAutoDelete( TRUE );
    } else {
	fList->clear();
	fiList->clear();
    }

    QStringList filters = qt_makeFilterList( nameFilter );

    bool doDirs	    = (filterSpec & Dirs)	!= 0;
    bool doFiles    = (filterSpec & Files)	!= 0;
    bool noSymLinks = (filterSpec & NoSymLinks) != 0;
    bool doReadable = (filterSpec & Readable)	!= 0;
    bool doWritable = (filterSpec & Writable)	!= 0;
    bool doExecable = (filterSpec & Executable) != 0;
    bool doHidden   = (filterSpec & Hidden)	!= 0;

#if defined(_OS_OS2EMX_)
    //QRegExp   wc( nameFilter, FALSE, TRUE );	// wild card, case insensitive
#else
    //QRegExp   wc( nameFilter, TRUE, TRUE );	// wild card, case sensitive
#endif
    QFileInfo fi;
    DIR	     *dir;
    dirent   *file;

    dir = opendir( QFile::encodeName(dPath) );
    if ( !dir ) {
#if defined(CHECK_NULL)
	qWarning( "QDir::readDirEntries: Cannot read the directory: %s",
		  QFile::encodeName(dPath).data() );
#endif
	return FALSE;
    }

    while ( (file = readdir(dir)) ) {
	QString fn = QFile::decodeName(file->d_name);
	fi.setFile( *this, fn );
	if ( !match( filters, fn ) && !(allDirs && fi.isDir()) )
	     continue;
	if  ( (doDirs && fi.isDir()) || (doFiles && fi.isFile()) ) {
	    if ( noSymLinks && fi.isSymLink() )
	        continue;
	    if ( (filterSpec & RWEMask) != 0 )
	        if ( (doReadable && !fi.isReadable()) ||
	             (doWritable && !fi.isWritable()) ||
	             (doExecable && !fi.isExecutable()) )
	            continue;
	    if ( !doHidden && fn[0] == '.' &&
	         fn != QString::fromLatin1(".")
	         && fn != QString::fromLatin1("..") )
	        continue;
	    fiList->append( new QFileInfo( fi ) );
	}
    }
    if ( closedir(dir) != 0 ) {
#if defined(CHECK_NULL) && !defined(_OS_VMS_)
	qWarning( "QDir::readDirEntries: Cannot close the directory: %s (UTF8)",
		  dPath.utf8().data() );
#endif
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

const QFileInfoList * QDir::drives()
{
    // at most one instance of QFileInfoList is leaked, and this variable
    // points to that list
    static QFileInfoList * knownMemoryLeak = 0;

    if ( !knownMemoryLeak ) {
	knownMemoryLeak = new QFileInfoList;
	// non-win32 versions both use just one root directory
	knownMemoryLeak->append( new QFileInfo( rootDirPath() ) );
    }

    return knownMemoryLeak;
}
#endif //QT_NO_DIR

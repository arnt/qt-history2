/****************************************************************************
**
** Implementation of QFileInfo class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qdatetime.h"
#include "qdir.h"

#include <limits.h>

/*!
    Returns true if this object points to a file. Returns FALSE if the
    object points to something which isn't a file, e.g. a directory or
    a symlink.

    \sa isDir(), isSymLink()
*/
bool QFileInfo::isFile() const
{
    if (!d)
	return false;

    if ( !d->cache )
	d->doStat();
    return d->could_stat ? (d->st.st_mode & S_IFMT) == S_IFREG : false;
}

/*!
    Returns true if this object points to a directory or to a symbolic
    link to a directory; otherwise returns false.

    \sa isFile(), isSymLink()
*/
bool QFileInfo::isDir() const
{
    if (!d)
	return false;

    if ( !d->cache )
	d->doStat();
    return d->could_stat ? (d->st.st_mode & S_IFMT) == S_IFDIR : false;
}

/*!
    Returns true if this object points to a symbolic link (or to a
    shortcut on Windows); otherwise returns false.

    \sa isFile(), isDir(), readLink()
*/

bool QFileInfo::isSymLink() const
{
    if (!d)
	return false;

    if ( !d->cache )
	d->doStat();
    return d->symLink;
}

/*!
    Returns the name a symlink (or shortcut on Windows) points to, or
    a QString::null if the object isn't a symbolic link.

    This name may not represent an existing file; it is only a string.
    QFileInfo::exists() returns true if the symlink points to an
    existing file.

    \sa exists(), isSymLink(), isDir(), isFile()
*/

QString QFileInfo::readLink() const
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_OS2EMX)
    if ( !d || !d->symLink )
	return QString();

    QString r;
    char s[PATH_MAX+1];
    int len = readlink( QFile::encodeName(d->fileName()), s, PATH_MAX );
    if ( len >= 0 ) {
	s[len] = '\0';
	r = QFile::decodeName(QByteArray(s));
    }
    return r;
#else
    return QString();
#endif
}

static const uint nobodyID = (uint) -2;

/*!
    Returns the owner of the file. On systems where files
    do not have owners, or if an error occurs, QString::null is
    returned.

    This function can be time consuming under Unix (in the order of
    milliseconds).

    \sa ownerId(), group(), groupId()
*/

QString QFileInfo::owner() const
{
    passwd *pw = getpwuid( ownerId() );
    if ( pw )
	return QFile::decodeName( QByteArray(pw->pw_name) );
    return QString::null;
}

/*!
    Returns the id of the owner of the file.

    On Windows and on systems where files do not have owners this
    function returns ((uint) -2).

    \sa owner(), group(), groupId()
*/

uint QFileInfo::ownerId() const
{
    if ( !d->cache )
	d->doStat();
    if ( d->could_stat )
	return d->st.st_uid;
    return nobodyID;
}

/*!
    Returns the group of the file. On Windows, on systems where files
    do not have groups, or if an error occurs, QString::null is
    returned.

    This function can be time consuming under Unix (in the order of
    milliseconds).

    \sa groupId(), owner(), ownerId()
*/

QString QFileInfo::group() const
{
    struct group *gr = getgrgid( groupId() );
    if ( gr )
	return QFile::decodeName( QByteArray(gr->gr_name) );
    return QString::null;
}

/*!
    Returns the id of the group the file belongs to.

    On Windows and on systems where files do not have groups this
    function always returns (uint) -2.

    \sa group(), owner(), ownerId()
*/

uint QFileInfo::groupId() const
{
    if ( !d->cache )
	d->doStat();
    if ( d->could_stat )
	return d->st.st_gid;
    return nobodyID;
}


/*!
    Tests for file permissions. The \a permissionSpec argument can be
    several flags of type \c PermissionSpec OR-ed together to check
    for permission combinations.

    On systems where files do not have permissions this function
    always returns true.

    Example:
    \code
	QFileInfo fi( "/tmp/archive.tar.gz" );
	if ( fi.permission( QFileInfo::WriteUser | QFileInfo::ReadGroup ) )
	    qWarning( "I can change the file; my group can read the file" );
	if ( fi.permission( QFileInfo::WriteGroup | QFileInfo::WriteOther ) )
	    qWarning( "The group or others can change the file" );
    \endcode

    \sa isReadable(), isWritable(), isExecutable()
*/

bool QFileInfo::permission( int permissionSpec ) const
{
    if ( !d->cache )
	d->doStat();
    if ( d->could_stat ) {
	uint mask = 0;
	if ( permissionSpec & ReadOwner )
	    mask |= S_IRUSR;
	if ( permissionSpec & WriteOwner )
	    mask |= S_IWUSR;
	if ( permissionSpec & ExeOwner )
	    mask |= S_IXUSR;
	if ( permissionSpec & ReadUser )
	    mask |= S_IRUSR;
	if ( permissionSpec & WriteUser )
	    mask |= S_IWUSR;
	if ( permissionSpec & ExeUser )
	    mask |= S_IXUSR;
	if ( permissionSpec & ReadGroup )
	    mask |= S_IRGRP;
	if ( permissionSpec & WriteGroup )
	    mask |= S_IWGRP;
	if ( permissionSpec & ExeGroup )
	    mask |= S_IXGRP;
	if ( permissionSpec & ReadOther )
	    mask |= S_IROTH;
	if ( permissionSpec & WriteOther )
	    mask |= S_IWOTH;
	if ( permissionSpec & ExeOther )
	    mask |= S_IXOTH;
	if ( mask ) {
	   return (d->st.st_mode & mask) == mask;
	} else {
	   qWarning( "QFileInfo::permission: permissionSpec is 0" );
	   return true;
	}
    } else {
	return false;
    }
}

void QFileInfoPrivate::doStat() const
{
    if (cache)
	return;

    symLink = false;
    cache = true;
    could_stat = true;

    symLink = false;
#if defined(Q_OS_UNIX) && defined(S_IFLNK)
    if ( ::lstat( QFile::encodeName(fn), &st ) == 0 ) {
	if ( S_ISLNK( st.st_mode ) )
	    symLink = true;
	else
	    return;
    }
#endif

    if ( ::stat(QFile::encodeName(fn), &st) != 0 && !symLink )
	could_stat = false;
}

/*!
    Returns the file's path.

    If \a absPath is true an absolute path is returned.

    \sa dir(), filePath(), fileName(), isRelative()
*/
#ifndef QT_NO_DIR
QString QFileInfo::dirPath( bool absPath ) const
{
    QString s;
    if ( absPath )
	s = absFilePath();
    else
	s = d->fileName();
    int pos = s.lastIndexOf( '/' );
    if ( pos == -1 ) {
	return QString::fromLatin1( "." );
    } else {
	if ( pos == 0 )
	    return QString::fromLatin1( "/" );
	return s.left( pos );
    }
}
#endif

/*!
    Returns the name of the file, excluding the path.

    Example:
    \code
	QFileInfo fi( "/tmp/archive.tar.gz" );
	QString name = fi.fileName();		// name = "archive.tar.gz"
    \endcode

    \sa isRelative(), filePath(), baseName(), extension()
*/

QString QFileInfo::fileName() const
{
    if (!d)
	return QString();

    int p = d->fileName().lastIndexOf( '/' );
    if ( p == -1 ) {
	return d->fileName();
    } else {
	return d->fileName().mid( p + 1 );
    }
}

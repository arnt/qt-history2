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
#include "qdatetime.h"
#include "qdir.h"
#include "qfiledefs_p.h"


/*!
    \class QFileInfo
    \reentrant
    \brief The QFileInfo class provides system-independent file information.

    \ingroup io

    QFileInfo provides information about a file's name and position
    (path) in the file system, its access rights and whether it is a
    directory or symbolic link, etc. The file's size and last
    modified/read times are also available.

    A QFileInfo can point to a file with either a relative or an
    absolute file path. Absolute file paths begin with the directory
    separator "/" (or with a drive specification on Windows). Relative
    file names begin with a directory name or a file name and specify
    a path relative to the current working directory. An example of an
    absolute path is the string "/tmp/quartz". A relative path might
    look like "src/fatlib". You can use the function isRelative() to
    check whether a QFileInfo is using a relative or an absolute file
    path. You can call the function convertToAbs() to convert a
    relative QFileInfo's path to an absolute path.

    The file that the QFileInfo works on is set in the constructor or
    later with setFile(). Use exists() to see if the file exists and
    size() to get its size.

    To speed up performance, QFileInfo caches information about the
    file. Because files can be changed by other users or programs, or
    even by other parts of the same program, there is a function that
    refreshes the file information: refresh(). If you want to switch
    off a QFileInfo's caching and force it to access the file system
    every time you request information from it call setCaching(FALSE).

    The file's type is obtained with isFile(), isDir() and
    isSymLink(). The readLink() function provides the name of the file
    the symlink points to.

    Elements of the file's name can be extracted with dirPath() and
    fileName(). The fileName()'s parts can be extracted with
    baseName() and extension().

    The file's dates are returned by created(), lastModified() and
    lastRead(). Information about the file's access permissions is
    obtained with isReadable(), isWritable() and isExecutable(). The
    file's ownership is available from owner(), ownerId(), group() and
    groupId(). You can examine a file's permissions and ownership in a
    single statement using the permission() function.

    If you need to read and traverse directories, see the QDir class.
*/

/*!
    \enum QFileInfo::PermissionSpec

    This enum is used by the permission() function to report the
    permissions and ownership of a file. The values may be OR-ed
    together to test multiple permissions and ownership values.

    \value ReadOwner The file is readable by the owner of the file.
    \value WriteOwner The file is writable by the owner of the file.
    \value ExeOwner The file is executable by the owner of the file.
    \value ReadUser The file is readable by the user.
    \value WriteUser The file is writable by the user.
    \value ExeUser The file is executable by the user.
    \value ReadGroup The file is readable by the group.
    \value WriteGroup The file is writable by the group.
    \value ExeGroup The file is executable by the group.
    \value ReadOther The file is readable by anyone.
    \value WriteOther The file is writable by anyone.
    \value ExeOther The file is executable by anyone.

    \warning The semantics of \c ReadUser, \c WriteUser and \c ExeUser are
    unfortunately not platform independent: on Unix, the rights of the owner of
    the file are returned and on Windows the rights of the current user are
    returned. This behavior might change in a future Qt version. If you want to
    find the rights of the owner of the file, you should use the flags \c
    ReadOwner, \c WriteOwner and \c ExeOwner. If you want to find out the
    rights of the current user, you should use isReadable(), isWritable() and
    isExecutable().
*/


/*!
    Constructs a new empty QFileInfo.
*/

QFileInfo::QFileInfo()
{
    d = 0;
}

/*!
    Constructs a new QFileInfo that gives information about the given
    file. The \a file can also include an absolute or relative path.

    \sa setFile(), isRelative(), QDir::setCurrent(), QDir::isRelativePath()
*/

QFileInfo::QFileInfo( const QString &file )
{
    d = new QFileInfoPrivate(file);
}

/*!
    Constructs a new QFileInfo that gives information about file \a
    file.

    If the \a file has a relative path, the QFileInfo will also have a
    relative path.

    \sa isRelative()
*/

QFileInfo::QFileInfo( const QFile &file )
{
    d = new QFileInfoPrivate(file.name());
}

/*!
    Constructs a new QFileInfo that gives information about the file
    called \a fileName in the directory \a d.

    If \a d has a relative path, the QFileInfo will also have a
    relative path.

    \sa isRelative()
*/
#ifndef QT_NO_DIR
QFileInfo::QFileInfo( const QDir &dir, const QString &fileName )
{
    d = new QFileInfoPrivate(dir.filePath( fileName ));
}
#endif
/*!
    Constructs a new QFileInfo that is a copy of \a fi.
*/

QFileInfo::QFileInfo( const QFileInfo &fi )
    : d(fi.d)
{
    if (d)
	++d->ref;
}

/*!
    Destroys the QFileInfo and frees its resources.
*/

QFileInfo::~QFileInfo()
{
    if (d && !--d->ref)
	delete d;
}


/*!
    Makes a copy of \a fi and assigns it to this QFileInfo.
*/

QFileInfo &QFileInfo::operator=( const QFileInfo &fi )
{
    QFileInfoPrivate *x = fi.d;
    if (x)
	++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (x && !--x->ref)
	delete x;

    return *this;
}


/*!
    Sets the file that the QFileInfo provides information about to \a
    file.

    The \a file can also include an absolute or relative file path.
    Absolute paths begin with the directory separator (e.g. "/" under
    Unix) or a drive specification (under Windows). Relative file
    names begin with a directory name or a file name and specify a
    path relative to the current directory.

    Example:
    \code
    QString absolute = "/local/bin";
    QString relative = "local/bin";
    QFileInfo absFile( absolute );
    QFileInfo relFile( relative );

    QDir::setCurrent( QDir::rootDirPath() );
    // absFile and relFile now point to the same file

    QDir::setCurrent( "/tmp" );
    // absFile now points to "/local/bin",
    // while relFile points to "/tmp/local/bin"
    \endcode

    \sa isRelative(), QDir::setCurrent(), QDir::isRelativePath()
*/

void QFileInfo::setFile( const QString &file )
{
    detach();
    d->setFileName(file);
}

/*!
    \overload

    Sets the file that the QFileInfo provides information about to \a
    file.

    If \a file includes a relative path, the QFileInfo will also have
    a relative path.

    \sa isRelative()
*/

void QFileInfo::setFile( const QFile &file )
{
    detach();
    d->setFileName(file.name());
}

/*!
    \overload

    Sets the file that the QFileInfo provides information about to \a
    fileName in directory \a dir.

    If \a fileName includes a relative path, the QFileInfo will also
    have a relative path.

    \sa isRelative()
*/
#ifndef QT_NO_DIR
void QFileInfo::setFile( const QDir &dir, const QString &fileName )
{
    detach();
    d->setFileName(dir.filePath( fileName ));
}
#endif

/*!
    Returns TRUE if the file exists; otherwise returns FALSE.
*/

bool QFileInfo::exists() const
{
    return QFileInfoPrivate::access( d->fileName(), F_OK );
}

/*!
    Refreshes the information about the file, i.e. reads in information
    from the file system the next time a cached property is fetched.
*/

void QFileInfo::refresh() const
{
    d->cache = false;
}

/*!
    \fn bool QFileInfo::caching() const

  \obsolete
*/

/*!
  \fn void QFileInfo::setCaching( bool enable )

  \obsolete
*/


/*!
    Returns the file name, including the path (which may be absolute
    or relative).

    \sa isRelative(), absFilePath()
*/

QString QFileInfo::filePath() const
{
    return d->fileName();
}

/*!
    Returns the base name of the file.

    If \a complete is FALSE (the default) the base name consists of
    all characters in the file name up to (but not including) the \e
    first '.' character.

    If \a complete is TRUE the base name consists of all characters in
    the file up to (but not including) the \e last '.' character.

    The path is not included in either case.

    Example:
    \code
	QFileInfo fi( "/tmp/archive.tar.gz" );
	QString base = fi.baseName();  // base = "archive"
	base = fi.baseName( TRUE );    // base = "archive.tar"
    \endcode

    \sa fileName(), extension()
*/

QString QFileInfo::baseName( bool complete ) const
{
    QString tmp = fileName();
    int pos = complete ? tmp.lastIndexOf( '.' ) : tmp.indexOf( '.' );
    if ( pos == -1 )
	return tmp;
    else
	return tmp.left( pos );
}

/*!
    Returns the file's extension name.

    If \a complete is TRUE (the default), extension() returns the
    string of all characters in the file name after (but not
    including) the first '.'  character.

    If \a complete is FALSE, extension() returns the string of all
    characters in the file name after (but not including) the last '.'
    character.

    Example:
    \code
	QFileInfo fi( "/tmp/archive.tar.gz" );
	QString ext = fi.extension();  // ext = "tar.gz"
	ext = fi.extension( FALSE );   // ext = "gz"
    \endcode

    \sa fileName(), baseName()
*/

QString QFileInfo::extension( bool complete ) const
{
    QString s = fileName();
    int pos = complete ? s.indexOf( '.' ) : s.lastIndexOf( '.' );
    if ( pos < 0 )
	return QString::fromLatin1( "" );
    else
	return s.right( s.length() - pos - 1 );
}

/*!
    Returns the file's path as a QDir object.

    If the QFileInfo is relative and \a absPath is FALSE, the QDir
    will be relative; otherwise it will be absolute.

    \sa dirPath(), filePath(), fileName(), isRelative()
*/
#ifndef QT_NO_DIR
QDir QFileInfo::dir( bool absPath ) const
{
    return QDir( dirPath(absPath) );
}
#endif


/*!
    Returns TRUE if the user can write to the file; otherwise returns FALSE.

    \sa isWritable(), isExecutable(), permission()
*/

bool QFileInfo::isReadable() const
{
#ifdef Q_WS_WIN
    return QFileInfoPrivate::access( d->fileName(), R_OK ) && permission( ReadUser );
#else
    return QFileInfoPrivate::access( d->fileName(), R_OK );
#endif
}

/*!
    Returns TRUE if the file is writable; otherwise returns FALSE.

    \sa isReadable(), isExecutable(), permission()
*/

bool QFileInfo::isWritable() const
{
#ifdef Q_WS_WIN
    return QFileInfoPrivate::access( d->fileName(), W_OK ) && permission( WriteUser );
#else
    return QFileInfoPrivate::access( d->fileName(), W_OK );
#endif
}

/*!
    Returns TRUE if the file is executable; otherwise returns FALSE.

    \sa isReadable(), isWritable(), permission()
*/

bool QFileInfo::isExecutable() const
{
#ifdef Q_WS_WIN
    return QFileInfoPrivate::access( d->fileName(), X_OK ) && permission( ExeUser );
#else
    return QFileInfoPrivate::access( d->fileName(), X_OK );
#endif
}

#ifndef Q_WS_WIN
bool QFileInfo::isHidden() const
{
    return d->fileName()[ 0 ] == QChar( '.' );
}
#endif

#ifndef QT_NO_DIR
/*!
    Returns TRUE if the file path name is relative. Returns FALSE if
    the path is absolute (e.g. under Unix a path is absolute if it
    begins with a "/").
*/
bool QFileInfo::isRelative() const
{
    return QDir::isRelativePath( d->fileName() );
}

/*!
    Converts the file's path to an absolute path.

    If it is already absolute, nothing is done.

    \sa filePath(), isRelative()
*/

bool QFileInfo::convertToAbs()
{
    if ( isRelative() )
	d->setFileName(absFilePath());
    return QDir::isRelativePath(d->fileName());
}
#endif

/*!
    Returns the file size in bytes, or 0 if the file does not exist or
    if the size is 0 or if the size cannot be fetched.
*/
QIODevice::Offset QFileInfo::size() const
{
    if ( !d->cache )
	d->doStat();
    if ( d->could_stat )
	return (QIODevice::Offset)d->st.st_size;
    else
	return 0;
}

/*!
    Returns the date and time when the file was created.

    On platforms where this information is not available, returns the
    same as lastModified().

    \sa created() lastModified() lastRead()
*/

QDateTime QFileInfo::created() const
{
    if ( !d->cache )
	d->doStat();
    if ( d->could_stat && d->st.st_ctime != 0 ) {
	QDateTime dt;
	dt.setTime_t( d->st.st_ctime );
	return dt;
    } else {
	return lastModified();
    }
}

/*!
    Returns the date and time when the file was last modified.

    \sa created() lastModified() lastRead()
*/

QDateTime QFileInfo::lastModified() const
{
    QDateTime dt;
    if ( !d->cache )
	d->doStat();
    if ( d->could_stat )
	dt.setTime_t( d->st.st_mtime );
    return dt;
}

/*!
    Returns the date and time when the file was last read (accessed).

    On platforms where this information is not available, returns the
    same as lastModified().

    \sa created() lastModified() lastRead()
*/

QDateTime QFileInfo::lastRead() const
{
    if ( !d->cache )
	d->doStat();
    if ( d->could_stat && d->st.st_atime != 0 ) {
	QDateTime dt;
	dt.setTime_t( d->st.st_atime );
	return dt;
    } else {
	return lastModified();
    }
}

#ifndef QT_NO_DIR

/*!
    Returns the absolute path including the file name.

    The absolute path name consists of the full path and the file
    name. On Unix this will always begin with the root, '/',
    directory. On Windows this will always begin 'D:/' where D is a
    drive letter, except for network shares that are not mapped to a
    drive letter, in which case the path will begin '//sharename/'.

    This function returns the same as filePath(), unless isRelative()
    is TRUE.

    If the QFileInfo is empty it returns QDir::currentDirPath().

    This function can be time consuming under Unix (in the order of
    milliseconds).

    \sa isRelative(), filePath()
*/
QString QFileInfo::absFilePath() const
{
    QString tmp;
    if ( QDir::isRelativePath(d->fileName())
#if defined(Q_OS_WIN32)
	 && d->fileName.at(1) != ':'
#endif
	 ) {
	tmp = QDir::currentDirPath();
	tmp += '/';
    }
    tmp += d->fileName();
    d->makeAbs( tmp );
    return QDir::cleanDirPath( tmp );
}

/*! \internal
    Detaches all internal data.
*/
void QFileInfo::detach()
{
    if ( d ) {
	QFileInfoPrivate *x = new QFileInfoPrivate(d->fileName());
	x->st = d->st;
	x->cache = d->cache;
	x->could_stat = d->could_stat;
#if defined(Q_OS_UNIX)
	x->symLink = d->symLink;
#endif

	++x->ref;
	x = qAtomicSetPtr(&d, x);
	if (!--x->ref)
	    delete x;
    } else {
	d = new QFileInfoPrivate();
	++d->ref;
    }
}

#endif

/****************************************************************************
**
** Implementation of QFileInfo class
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfileinfo.h"
#include <qplatformdefs.h>
#include <qglobal.h>
#include <qatomic.h>
#include <qfileinfoengine.h>
#include "qdir.h"

#define d d_func()
#define q q_func()

//************* QFileInfoEngineHandler
static QList<QFileInfoEngineHandler*> *fileInfoHandlers = 0;
QFileInfoEngineHandler::QFileInfoEngineHandler()
{
    if(!fileInfoHandlers)
        fileInfoHandlers = new QList<QFileInfoEngineHandler*>;
    fileInfoHandlers->append(this);
}

QFileInfoEngineHandler::~QFileInfoEngineHandler()
{
    fileInfoHandlers->removeAll(this);
    if(fileInfoHandlers->isEmpty()) {
        delete fileInfoHandlers;
        fileInfoHandlers = 0;
    }
}
QFileInfoEngine *qt_createFileInfoEngine(const QString &path)
{
    if(fileInfoHandlers) {
        for(int i = 0; i < fileInfoHandlers->size(); i++) {
            if(QFileInfoEngine *ret = fileInfoHandlers->at(i)->createFileInfoEngine(path))
                return ret;
        }
    }
    return new QFSFileInfoEngine(path);
}

//************* QFileInfoPrivate
class QFileInfoPrivate
{
    QFileInfo *q_ptr;
    Q_DECLARE_PUBLIC(QFileInfo)

protected:
    QFileInfoPrivate(QFileInfo *, const QFileInfo *copy=0);
    ~QFileInfoPrivate();

    void initFileInfoEngine(const QString &);

    uint getFileInfo(QFileInfoEngine::FileInfo) const;
    QDateTime &getFileTime(QFileInfoEngine::FileTime) const;
private:
    enum { CachedPerms=0x01, CachedTypes=0x02, CachedFlags=0x04,
           CachedMTime=0x10, CachedCTime=0x20, CachedATime=0x40,
           CachedSize =0x08 };
    struct Data {
        inline Data() : fileInfoEngine(0) { ref = 1; clear(); }
        inline ~Data() { delete fileInfoEngine; }
        inline void clear() {
            fileInfo = 0;
            cached = 0;
        }
        mutable QAtomic ref;

        QFileInfoEngine *fileInfoEngine;
        mutable QString fileName;

        mutable uint fileSize;
        mutable QDateTime fileTimes[3];
        mutable uint fileInfo;
        mutable uchar cached;
    } *data;
    inline void reset() {
        detach();
        data->clear();
    }
    void detach();
};

QFileInfoPrivate::QFileInfoPrivate(QFileInfo *qq, const QFileInfo *copy) : q_ptr(qq)
{ 
    if(copy) {
        ++copy->d->data->ref;
        data = copy->d->data;
    } else {
        data = new QFileInfoPrivate::Data;
        data->clear();
    }
}

QFileInfoPrivate::~QFileInfoPrivate()
{
    if (!--data->ref)
        delete data;
    data = 0;
    q_ptr = 0; 
}

void QFileInfoPrivate::initFileInfoEngine(const QString &file)
{
    detach();
    delete data->fileInfoEngine;
    data->fileInfoEngine = 0;
    data->clear();
    data->fileInfoEngine = qt_createFileInfoEngine(file);
    data->fileName = file;
}

void QFileInfoPrivate::detach()
{
    if (data->ref != 1) {
        QFileInfoPrivate::Data *x = data;
        data = new QFileInfoPrivate::Data;
        initFileInfoEngine(x->fileName);
        --x->ref;
    }
}

uint QFileInfoPrivate::getFileInfo(QFileInfoEngine::FileInfo request) const
{
    uint masks = 0;
    if((request & QFileInfoEngine::TypeMask) && !(data->cached & CachedTypes)) {
        data->cached |= CachedTypes;
        masks |= QFileInfoEngine::TypeMask;
    }
    if((request & QFileInfoEngine::PermsMask) && !(data->cached & CachedPerms)) {
        data->cached |= CachedPerms;
        masks |= QFileInfoEngine::PermsMask;
    }
    if((request & QFileInfoEngine::FlagsMask) && !(data->cached & CachedFlags)) {
        data->cached |= CachedFlags;
        masks |= QFileInfoEngine::FlagsMask;
    }
    if(masks) 
        data->fileInfo |= (data->fileInfoEngine->fileFlags(masks) & masks); 
    return data->fileInfo & request;
}

QDateTime &QFileInfoPrivate::getFileTime(QFileInfoEngine::FileTime request) const
{
    if(request == QFileInfoEngine::CreationTime) {
        if(data->cached & CachedCTime)
            return data->fileTimes[request];
        return (data->fileTimes[request] = data->fileInfoEngine->fileTime(request));
    }
    if(request == QFileInfoEngine::ModificationTime) {
        if(data->cached & CachedMTime)
            return data->fileTimes[request];
        return (data->fileTimes[request] = data->fileInfoEngine->fileTime(request));
    }
    if(request == QFileInfoEngine::AccessTime) {
        if(data->cached & CachedATime)
            return data->fileTimes[request];
        return (data->fileTimes[request] = data->fileInfoEngine->fileTime(request));
    }
    return data->fileTimes[0]; //cannot really happen
}

//************* QFileInfo

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
    every time you request information from it call setCaching(false).

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

QFileInfo::QFileInfo() : d_ptr(new QFileInfoPrivate(this))
{
}

/*!
    Constructs a new QFileInfo that gives information about the given
    file. The \a file can also include an absolute or relative path.

    \sa setFile(), isRelative(), QDir::setCurrent(), QDir::isRelativePath()
*/

QFileInfo::QFileInfo(const QString &file) : d_ptr(new QFileInfoPrivate(this))
{
    d->initFileInfoEngine(file);
}

/*!
    Constructs a new QFileInfo that gives information about file \a
    file.

    If the \a file has a relative path, the QFileInfo will also have a
    relative path.

    \sa isRelative()
*/

QFileInfo::QFileInfo(const QFile &file) : d_ptr(new QFileInfoPrivate(this))
{
    d->initFileInfoEngine(file.name());
}

/*!
    Constructs a new QFileInfo that gives information about the file
    called \a fileName in the directory \a dir.

    If \a dir has a relative path, the QFileInfo will also have a
    relative path.

    \sa isRelative()
*/

#ifndef QT_NO_DIR
QFileInfo::QFileInfo(const QDir &dir, const QString &file) : d_ptr(new QFileInfoPrivate(this))
{
    d->initFileInfoEngine(dir.filePath(file));
}
#endif

/*!
    Constructs a new QFileInfo that is a copy of \a fi.
*/

QFileInfo::QFileInfo(const QFileInfo &fileinfo) : d_ptr(new QFileInfoPrivate(this, &fileinfo))
{

}

/*!
    Destroys the QFileInfo and frees its resources.
*/


QFileInfo::~QFileInfo()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
    Makes a copy of \a fi and assigns it to this QFileInfo.
*/

QFileInfo &QFileInfo::operator=(const QFileInfo &fileinfo)
{
    QFileInfoPrivate::Data *x = fileinfo.d->data;
    ++x->ref;
    x = qAtomicSetPtr(&d->data, x);
    if (!--x->ref)
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
    QFileInfo absFile(absolute);
    QFileInfo relFile(relative);

    QDir::setCurrent(QDir::rootDirPath());
    // absFile and relFile now point to the same file

    QDir::setCurrent("/tmp");
    // absFile now points to "/local/bin",
    // while relFile points to "/tmp/local/bin"
    \endcode

    \sa isRelative(), QDir::setCurrent(), QDir::isRelativePath()
*/

void
QFileInfo::setFile(const QString &file)
{
    d->initFileInfoEngine(file);
}

/*!
    \overload

    Sets the file that the QFileInfo provides information about to \a
    file.

    If \a file includes a relative path, the QFileInfo will also have
    a relative path.

    \sa isRelative()
*/

void
QFileInfo::setFile(const QFile &file)
{
    d->initFileInfoEngine(file.name());
}

#ifndef QT_NO_DIR
/*!
    \overload

    Sets the file that the QFileInfo provides information about to \a
    fileName in directory \a dir.

    If \a fileName includes a relative path, the QFileInfo will also
    have a relative path.

    \sa isRelative()
*/

void
QFileInfo::setFile(const QDir &dir, const QString &file)
{
    d->initFileInfoEngine(dir.filePath(file));
}

/*!
    Returns the absolute path including the file name.

    The absolute path name consists of the full path and the file
    name. On Unix this will always begin with the root, '/',
    directory. On Windows this will always begin 'D:/' where D is a
    drive letter, except for network shares that are not mapped to a
    drive letter, in which case the path will begin '//sharename/'.

    This function returns the same as filePath(), unless isRelative()
    is true.

    If the QFileInfo is empty it returns QDir::currentDirPath().

    This function can be time consuming under Unix (in the order of
    milliseconds).

    \sa isRelative(), filePath()
*/

QString
QFileInfo::absFilePath() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->fileName(QFileInfoEngine::AbsoluteName);
}

/*!
    Returns the file's path.

    If \a absPath is true an absolute path is returned.

    \sa dir(), filePath(), fileName(), isRelative()
*/

QString
QFileInfo::dirPath(bool absPath) const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->fileName(absPath ? QFileInfoEngine::AbsoluteDirPath : QFileInfoEngine::DirPath);
}

QString
QFileInfo::canonicalPath() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->fileName(QFileInfoEngine::Canonical);
}

/*!
    Returns true if the file path name is relative. Returns false if
    the path is absolute (e.g. under Unix a path is absolute if it
    begins with a "/").
*/

bool
QFileInfo::isRelative() const
{
    if(!d->data->fileInfoEngine)
        return true;
    return d->data->fileInfoEngine->isRelativePath();
}


/*!
    Converts the file's path to an absolute path.

    If it is already absolute, nothing is done.

    \sa filePath(), isRelative()
*/

bool
QFileInfo::convertToAbs()
{
    if(!d->data->fileInfoEngine)
        return false;
    QString absFileName = d->data->fileInfoEngine->fileName(QFileInfoEngine::AbsoluteName);
    if(QDir::isRelativePath(d->data->fileName))
        return false;
    d->detach();
    d->data->fileName = absFileName;
    d->data->fileInfoEngine->setFileName(absFileName);
    return true;
}
#endif

/*!
    Returns true if the file exists; otherwise returns false.
*/

bool
QFileInfo::exists() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::Exists);
}

/*!
    Refreshes the information about the file, i.e. reads in information
    from the file system the next time a cached property is fetched.
*/

void
QFileInfo::refresh()
{
    d->reset();
}

/*!
    Returns the file name, including the path (which may be absolute
    or relative).

    \sa isRelative(), absFilePath()
*/

QString
QFileInfo::filePath() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileName;
}

/*!
    Returns the name of the file, excluding the path.

    Example:
    \code
        QFileInfo fi("/tmp/archive.tar.gz");
        QString name = fi.fileName();                // name = "archive.tar.gz"
    \endcode

    \sa isRelative(), filePath(), baseName(), extension()
*/

QString
QFileInfo::fileName() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->fileName(QFileInfoEngine::BaseName);
}

/*!
    Returns the base name of the file.

    If \a complete is false (the default) the base name consists of
    all characters in the file name up to (but not including) the \e
    first '.' character.

    If \a complete is true the base name consists of all characters in
    the file up to (but not including) the \e last '.' character.

    The path is not included in either case.

    Example:
    \code
        QFileInfo fi("/tmp/archive.tar.gz");
        QString base = fi.baseName();  // base = "archive"
        base = fi.baseName(true);    // base = "archive.tar"
    \endcode

    \sa fileName(), extension()
*/

QString
QFileInfo::baseName(bool complete) const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    QString ret = d->data->fileInfoEngine->fileName(QFileInfoEngine::BaseName);
    int pos = complete ? ret.lastIndexOf('.') : ret.indexOf('.');
    if(pos == -1)
        return ret;
    return ret.left(pos);
}

/*!
    Returns the file's extension name.

    If \a complete is true (the default), extension() returns the
    string of all characters in the file name after (but not
    including) the first '.'  character.

    If \a complete is false, extension() returns the string of all
    characters in the file name after (but not including) the last '.'
    character.

    Example:
    \code
        QFileInfo fi("/tmp/archive.tar.gz");
        QString ext = fi.extension();  // ext = "tar.gz"
        ext = fi.extension(false);   // ext = "gz"
    \endcode

    \sa fileName(), baseName()
*/

QString
QFileInfo::extension(bool complete) const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    QString ret = d->data->fileInfoEngine->fileName(QFileInfoEngine::BaseName);
    int pos = complete ? ret.indexOf('.') : ret.lastIndexOf('.');
    if(pos == -1)
        return QString("");
    return ret.mid(pos + 1);
}

#ifndef QT_NO_DIR
/*!
    Returns the file's path as a QDir object.

    If the QFileInfo is relative and \a absPath is false, the QDir
    will be relative; otherwise it will be absolute.

    \sa dirPath(), filePath(), fileName(), isRelative()
*/

QDir QFileInfo::dir(bool absPath) const
{
    return QDir(dirPath(absPath));
}
#endif

/*!
    Returns true if the user can write to the file; otherwise returns false.

    \sa isWritable(), isExecutable(), permission()
*/

bool
QFileInfo::isReadable() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::ReadUser);
}

/*!
    Returns true if the file is writable; otherwise returns false.

    \sa isReadable(), isExecutable(), permission()
*/


bool
QFileInfo::isWritable() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::WriteUser);
}

/*!
    Returns true if the file is executable; otherwise returns false.

    \sa isReadable(), isWritable(), permission()
*/

bool
QFileInfo::isExecutable() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::ExeUser);
}

bool
QFileInfo::isHidden() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::Hidden);
}

/*!
    Returns true if this object points to a file. Returns false if the
    object points to something which isn't a file, e.g. a directory or
    a symlink.

    \sa isDir(), isSymLink()
*/

bool
QFileInfo::isFile() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::File);
}

/*!
    Returns true if this object points to a directory or to a symbolic
    link to a directory; otherwise returns false.

    \sa isFile(), isSymLink()
*/

bool
QFileInfo::isDir() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::Directory);
}

/*!
    Returns true if this object points to a symbolic link (or to a
    shortcut on Windows); otherwise returns false.

    \sa isFile(), isDir(), readLink()
*/

bool
QFileInfo::isSymLink() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::Link);
}

/*!
    Returns the name a symlink (or shortcut on Windows) points to, or
    a QString::null if the object isn't a symbolic link.

    This name may not represent an existing file; it is only a string.
    QFileInfo::exists() returns true if the symlink points to an
    existing file.

    \sa exists(), isSymLink(), isDir(), isFile()
*/

QString
QFileInfo::readLink() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->fileName(QFileInfoEngine::LinkName);
}

/*!
    Returns the owner of the file. On systems where files
    do not have owners, or if an error occurs, QString::null is
    returned.

    This function can be time consuming under Unix (in the order of
    milliseconds).

    \sa ownerId(), group(), groupId()
*/

QString
QFileInfo::owner() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->owner(QFileInfoEngine::User);
}

/*!
    Returns the id of the owner of the file.

    On Windows and on systems where files do not have owners this
    function returns ((uint) -2).

    \sa owner(), group(), groupId()
*/

uint
QFileInfo::ownerId() const
{
    if(!d->data->fileInfoEngine)
        return 0;
    return d->data->fileInfoEngine->ownerId(QFileInfoEngine::User);
}

/*!
    Returns the group of the file. On Windows, on systems where files
    do not have groups, or if an error occurs, QString::null is
    returned.

    This function can be time consuming under Unix (in the order of
    milliseconds).

    \sa groupId(), owner(), ownerId()
*/

QString
QFileInfo::group() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->owner(QFileInfoEngine::Group);
}

/*!
    Returns the id of the group the file belongs to.

    On Windows and on systems where files do not have groups this
    function always returns (uint) -2.

    \sa group(), owner(), ownerId()
*/

uint
QFileInfo::groupId() const
{
    if(!d->data->fileInfoEngine)
        return 0;
    return d->data->fileInfoEngine->ownerId(QFileInfoEngine::Group);
}

/*!
    Tests for file permissions. The \a permissionSpec argument can be
    several flags of type \c PermissionSpec OR-ed together to check
    for permission combinations.

    On systems where files do not have permissions this function
    always returns true.

    Example:
    \code
        QFileInfo fi("/tmp/archive.tar.gz");
        if (fi.permission(QFileInfo::WriteUser | QFileInfo::ReadGroup))
            qWarning("I can change the file; my group can read the file");
        if (fi.permission(QFileInfo::WriteGroup | QFileInfo::WriteOther))
            qWarning("The group or others can change the file");
    \endcode

    \sa isReadable(), isWritable(), isExecutable()
*/

bool
QFileInfo::permission(uint permissionSpec) const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo((QFileInfoEngine::FileInfo)permissionSpec) == permissionSpec;
}

/*!
    Returns the file size in bytes, or 0 if the file does not exist or
    if the size is 0 or if the size cannot be fetched.
*/

QIODevice::Offset
QFileInfo::size() const
{
    if(!d->data->fileInfoEngine)
        return 0;
    if(!(d->data->cached & QFileInfoPrivate::CachedSize))
        d->data->fileSize = d->data->fileInfoEngine->size();
    return d->data->fileSize;
}

/*!
    Returns the date and time when the file was created.

    On platforms where this information is not available, returns the
    same as lastModified().

    \sa created() lastModified() lastRead()
*/

QDateTime
QFileInfo::created() const
{
    if(!d->data->fileInfoEngine)
        return QDateTime();
    return d->getFileTime(QFileInfoEngine::CreationTime);
}

/*!
    Returns the date and time when the file was last modified.

    \sa created() lastModified() lastRead()
*/

QDateTime
QFileInfo::lastModified() const
{
    if(!d->data->fileInfoEngine)
        return QDateTime();
    return d->getFileTime(QFileInfoEngine::ModificationTime);
}

/*!
    Returns the date and time when the file was last read (accessed).

    On platforms where this information is not available, returns the
    same as lastModified().

    \sa created() lastModified() lastRead()
*/

QDateTime
QFileInfo::lastRead() const
{
    if(!d->data->fileInfoEngine)
        return QDateTime();
    return d->getFileTime(QFileInfoEngine::AccessTime);
}

/*! \internal
    Detaches all internal data.
*/

void 
QFileInfo::detach()
{
    d->detach();
}

/****************************************************************************
**
** Definition of QFileEngine and QFSFileEngine classes.
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

#include "qfileengine.h"
#include "qfileengine_p.h"
#include <qdatetime.h>
#include <qplatformdefs.h>

#include <errno.h>

#if defined(O_NONBLOCK)
# define HAS_ASYNC_FILEMODE
# define QT_OPEN_ASYNC O_NONBLOCK
#elif defined(O_NDELAY)
# define HAS_ASYNC_FILEMODE
# define QT_OPEN_ASYNC O_NDELAY
#endif
#if defined(Q_OS_MSDOS) || defined(Q_OS_WIN32) || defined(Q_OS_OS2)
# define HAS_TEXT_FILEMODE                        // has translate/text filemode
#endif

// POSIX Large File Support redefines open -> open64
#if defined(open)
# undef open
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

#define d d_func()
#define q q_func()

//************* QFileEngineHandler
/*!
    \class QFileEngineHandler qfileengine.h
    \reentrant

    \brief The QFileEngineHandler class allows new filesystems to be plugged into Qt

    \ingroup io
    \mainclass

    Using the QFileEngineHandler allows new QFileEngines to be created
    based on a filepath. Subclassing QFileEngineHandler and
    reimplementing createFileEngine() will cause your new QFileEngine
    to be used for a given path. Once instaniated it will
    automatically be added to the list of known file handlers.
*/

static QList<QFileEngineHandler*> *fileHandlers;

/*!
   Constructs a QFileEngineHandler. Once created this handler will be
   consulted via the QFileEngineHandler::createFileEngine for any paths used.

   \sa createFileEngine
 */
QFileEngineHandler::QFileEngineHandler()
{
    if(!fileHandlers)
        fileHandlers = new QList<QFileEngineHandler*>;
    fileHandlers->append(this);
}

/*!
   Destroys a QFileEngineHandler. This will automatically remove the
   handler from the list of known handlers.
 */
QFileEngineHandler::~QFileEngineHandler()
{
    fileHandlers->removeAll(this);
    if(fileHandlers->isEmpty()) {
        delete fileHandlers;
        fileHandlers = 0;
    }
}

/* 
    \fn QFileEngine *QFileEngineHandler::createFileEngine(const QString &path)
  
    This function will be called when a path is used, if \a path is a
    path for your QFileEngine return a new instance to it, the caller
    is responsible for destruction. If you do not handle paths that
    look like \a path then just return 0 and a default QFileEngine to
    handle the filesystem will be used.

    This virtual function must be reimplemented by all subclasses.
 */


//**************** QFileEngine
/*!
    \class QFileEngine qfileengine.h
    \reentrant

    \brief The QFileEngine class provides an abstraction for filesystem information.

    \ingroup io
    \mainclass

    QFileEngine is an abstraction for basic file system operations. A
    QFileEngine is consulted internally in QFile, QFileInfo, and
    QDir. Plugging in a new QFileEngine is normally a matter of
    subclassing QFileEngine and creating a subclass of
    QFileEngineHandler for your type of files. Finally instantiate
    your QFileEngineHandler (often via a global static) and your new
    subclass will be used internally.

    A QFileEngine can reference a file or a directory, several of the
    functions will have only have meaning when pointing to either
    however.

    QFileEngine can be created to do syncronous network IO based file
    system operations, local file system information, or even a
    resource system to access file based resources.

   \sa QFileEngineHandler, setFileName
*/

/*!
   Constructs a QFileEngine.
 */
QFileEngine::QFileEngine() : QIOEngine(*new QFileEnginePrivate)
{

}

/*!
   Constructs a QFileEngine.
 */
QFileEngine::QFileEngine(QFileEnginePrivate &dd)  : QIOEngine(dd)
{
}

/*!
    Destroys the QFileEngine.
 */
QFileEngine::~QFileEngine()
{
}

/*!
   Creates a new QFileEngine instance for \a file. This function will
   always return a valid QFileEngine, the default file engine created
   will consult the local filesystem.
 */
QFileEngine *QFileEngine::createFileEngine(const QString &file)
{
    if(fileHandlers) {
        for(int i = 0; i < fileHandlers->size(); i++) {
            if(QFileEngine *ret = fileHandlers->at(i)->createFileEngine(file))
                return ret;
        }
    }
    return new QFSFileEngine(file);
}


/* 
    \fn void QFileEngine::setFileName(const QString &file)
  
    Sets the filename in your QFileEngine to \a file. This filename
    will be the file that the rest of the virtual functions will
    operate on.

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn bool QFileEngine::remove()
  
    Requests that the file be removed. If the operation fails return
    is false, otherwise true.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName
 */

/* 
    \fn bool QFileEngine::rename(const QString &newName)
  
    Requests that the file be renamed to \a newName. If the
    operation fails return is false, otherwise true.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName
 */

/* 
    \fn bool QFileEngine::mkdir(const QString &dirName, QDir::Recursion recurse) const
  
    Requests that the directory \a dirName be created. When \a recurse is
    \b QDir::Recursive then all subpaths in \a dirname must be
    created, otherwise it is expected all the subpaths already
    exist. If the operation fails return is false, otherwise true.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName
 */

/* 
    \fn bool QFileEngine::rmdir(const QString &dirName, QDir::Recursion recurse) const
  
    Requests that the directory \a dirName be removed. When \a recurse
    is \b QDir::Recursive then all subpaths in \a dirname will be
    removed if empty, otherwise only \a dirname is removed. If the
    operation fails return is false, otherwise true.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName
 */

/* 
    \fn QStringList QFileEngine::entryList(int filterSpec, const QStringList &filters) const
  
    Requests that a list of all files matching \a filters based on \a
    filterSpec in the directory be returned. If the file pointed to is
    not a directory then an empty list should be returned.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName
 */

/* 
    \fn bool QFileEngine::caseSensitive() const
  
    Determines if the file system referenced by the QFileEngine is
    case sensitive; if your file system is case sensitive return true,
    otherwise false.

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn bool QFileEngine::isRoot() const
  
    Determines if the file pointed to is the root of your file system,
    this is used to determine if the file has a parent directory.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName
 */

/* 
    \fn bool QFileEngine::isRelativePath() const
  
    Determines if the file current pointed to is considered relative
    by your file system; if the file is relative return true,
    otherwise false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName
 */

/* 
    \fn uint QFileEngine::fileFlags(uint type) const
  
    This function is used to gather information about the file based
    on the \a type. The type will be set to several or'd together
    members of \b QFileEngine::FileInfo being requested this function
    will returned another set of or'd together QFileEngine::FileInfo
    members that are currently on in the file pointed to.

    The \a type could be considered an optimization so it can be
    ignored and the return value to the same set for all calls, it is
    provided as a hint to which data will be actually used (and thus
    lookup can be avoided).

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName, FileInfo
 */


/* 
    \fn QString QFileEngine::fileName(FileName file) const 
  
    Requests that different format of the file name pointed to be
    returned based on \a file. For any QFileEngine::FileName not
    handled return the file name set in QFileEngine::setFileName.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName, FileName
 */

/* 
    \fn uint QFileEngine::ownerId(FileOwner id) const
  
    Request that the owning user or group id be returned based on \a
    id. If no owner can be determined -2 should be returned.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName, FileOwner
 */

/* 
    \fn QString QFileEngine::owner(FileOwner id) const
  
    Request that the owning user or group name be returned based on \a
    id. If no owner can be determined a null QString should be returned.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName, QString::isNull, FileOwner
 */

/* 
    \fn QDateTime QFileEngine::fileTime(FileTime time) const
  
    Request that date information be returned about the file. If a
    file time cannot be determined a invalid QDateTime should be
    returned.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName, QDateTime, QDateTime::isValid, FileTime
 */

/*!
    \enum QFileEngine::FileName

    These values will be used when requesting different format for a filename.

    \value DefaultName The same filename as passed into the QFileEngine
    \value BaseName The name of the file excluding the path
    \value PathName The path to the file excluding the BaseName
    \value AbsoluteName The absolute path to the file (including BaseName)
    \value AbsolutePathName The absolute path to the file (excluding BaseName) 
    \value LinkName Where the file is linked to if a link is referenced in the path
    \value CanonicalName Often very similar to LinkName will return the true path to the file

    \sa fileName, setFileName
*/

/*!
    \enum QFileEngine::FileInfo

    These flags will be passed and returned by the fileFlags function. They can be or'd together to
    reflect a single set of information about a file.

    \value ReadOwnerPerm If the owner of the file has permission to read from it
    \value WriteOwnerPerm If the owner of the file has permission to write to it
    \value ExeOwnerPerm If the owner of the file has permission to execute it
    \value ReadUserPerm If the current user has permission to read from the file
    \value WriteUserPerm If the current user has permission to write to the file
    \value ExeUserPerm If the current user has permission to execute the file
    \value ReadGroupPerm If the current user's group has permission to read from the file
    \value WriteGroupPerm If the current user's group has permission to write to the file
    \value ExeGroupPerm If the current user's group has permission to execute the file
    \value ReadOtherPerm If the all users have permission to read from the file
    \value WriteOtherPerm If the all users have permission to write to the  file
    \value ExeOtherPerm If the all users have permission to execute the file

    \value LinkType If the file is a considered a link to the file system
    \value FileType If the file is a considered a regular file to the file system
    \value DirectoryType If the file is a considered a directory to the file system

    \value HiddenFlag If the file is determined to not be visible
    \value ExistsFlag If the file actually exists

    \omitvalue PermsMask
    \omitvalue TypesMask
    \omitvalue FlagsMask
    \omitvalue FileInfoAll

    \sa fileFlags, setFileName
*/

/*!
    \enum QFileEngine::FileTime

    These values will be passed to fileTime to determine date
    information about the file.

    \value CreationTime Creation of the file
    \value ModificationTime Last time the file was modified
    \value AccessTime Last time the file was access

    \sa fileTime, setFileName
*/

/*!
    \enum QFileEngine::FileOwner

    These values will be passed to determine ownership of a file.

    \value OwnerUser The user who owns a file
    \value OwnerGroup The group who owns a file

    \sa owner, ownerId, setFileName
*/



//**************** QFSFileEnginePrivate
QFSFileEnginePrivate::QFSFileEnginePrivate() : QFileEnginePrivate()
{
    sequential = 0;
    tried_stat = false;
    fd = -1;
    init();
}

//**************** QFSFileEngine
QFSFileEngine::QFSFileEngine(const QString &file) : QFileEngine(*new QFSFileEnginePrivate)
{
    d->file = QFSFileEnginePrivate::fixToQtSlashes(file);    
    d->resetErrors();
}

QFSFileEngine::QFSFileEngine() : QFileEngine(*new QFSFileEnginePrivate)
{
}

void
QFSFileEngine::setFileName(const QString &file)
{
    d->file = QFSFileEnginePrivate::fixToQtSlashes(file);
    d->tried_stat = false;
}

bool
QFSFileEngine::open(int flags)
{
    d->resetErrors();
    if (d->file.isEmpty()) {
        qWarning("QFSFileEngine::open: No file name specified");
        d->setError(QIODevice::OpenError, "No file name specified");
        return false;
    }
    int oflags = QT_OPEN_RDONLY;
    if ((flags & QFile::ReadWrite) == QFile::ReadWrite)
        oflags = QT_OPEN_RDWR;
    else if (flags & QFile::WriteOnly)
        oflags = QT_OPEN_WRONLY;
    if (flags & QFile::Append) {                // append to end of file?
        if (!(flags & QFile::ReadOnly) || (flags & QFile::Truncate))
            oflags |= (QT_OPEN_CREAT | QT_OPEN_TRUNC);
        else
            oflags |= (QT_OPEN_APPEND | QT_OPEN_CREAT);
    } else if (flags & QFile::WriteOnly) {                // create/trunc if writable
        if (!(flags & QFile::ReadOnly) || (flags & QFile::Truncate))
            oflags |= (QT_OPEN_CREAT | QT_OPEN_TRUNC);
        else
            oflags |= QT_OPEN_CREAT;
    }
#if defined(HAS_TEXT_FILEMODE)
    if (flags & QFile::Translate)
        oflags |= QT_OPEN_TEXT;
    else
        oflags |= QT_OPEN_BINARY;
#endif
#if defined(HAS_ASYNC_FILEMODE)
    if (flags & QFile::Async)
        oflags |= QT_OPEN_ASYNC;
#endif
    d->external_file = 0;
    d->fd = d->sysOpen(d->file, oflags);
    if(d->fd != -1) {
        d->sequential = 0;
        struct stat st;
        ::fstat(d->fd, &st);
        if ((st.st_mode & S_IFMT) != S_IFREG) {
            d->sequential = 1;
        } else {
            struct stat st;
            ::fstat(d->fd, &st);
            char char_read;
            if(!st.st_size && readBlock(&char_read, 1) == 1) {
                d->ungetchBuffer += char_read;
                d->sequential = 1;
            }
        }
        return true;
    }
    d->setError(errno == EMFILE ? QIODevice::ResourceError : QIODevice::OpenError, errno);
    return false;
}

bool
QFSFileEngine::open(int, int fd)
{
    d->external_file = 1;
    d->fd = fd;
    if(d->fd != -1) {
        d->sequential = 0;
        struct stat st;
        ::fstat(d->fd, &st);
	if ((st.st_mode & QT_STAT_MASK) != QT_STAT_REG || !fd) //stdin is non seekable
            d->sequential = 1;
#ifdef Q_OS_UNIX
	else { 
            char char_read;
            if(!st.st_size && readBlock(&char_read, 1) == 1) {
                d->ungetchBuffer += char_read;
                d->sequential = 1;
            }
        }
#endif
        return true;
    }
    return false;
}

bool
QFSFileEngine::close()
{
    if (d->fd == -1)
        return false;
    d->resetErrors();
    flush();
    int ret = d->external_file ? 0 : QT_CLOSE(d->fd);
    d->tried_stat = 0;
    d->fd = -1;
    if(ret == -1) {
        d->setError(QIODevice::UnspecifiedError, errno);
        return false;
    }
    return true;
}

void
QFSFileEngine::flush()
{
    d->ungetchBuffer.clear();
}

Q_LONG
QFSFileEngine::readBlock(char *data, Q_LONG len)
{
    Q_LONG ret = 0;
    if (!d->ungetchBuffer.isEmpty()) {
        Q_LONG l = d->ungetchBuffer.size();
        while(ret < l) {
            *data = d->ungetchBuffer.at(l - ret - 1);
            data++;
            ret++;
        }
        d->ungetchBuffer.resize(l - ret);
        len -= ret;
    }
    d->resetErrors();
    if(len && ret != len) {
        int read = QT_READ(d->fd, data, len);
        if(read <= 0)
            d->setError(QIODevice::ReadError, errno);
        else 
            ret += read;
    }
    return ret;
}

int
QFSFileEngine::ungetch(int ch)
{
    d->ungetchBuffer += ch;
    return ch;
}

Q_LONG
QFSFileEngine::writeBlock(const char *data, Q_LONG len)
{
    d->resetErrors();
    Q_LONG ret = QT_WRITE(d->fd, data, len);
    if(ret != len)
        d->setError(errno == ENOSPC ? QIODevice::ResourceError : QIODevice::WriteError, errno);
    return ret;
}

QFile::Offset
QFSFileEngine::at() const
{
    return QT_LSEEK(d->fd, 0, SEEK_CUR);
}

bool
QFSFileEngine::atEnd() const
{
    if(!d->ungetchBuffer.isEmpty())
        return false;
    return (at() == size());
}

int
QFSFileEngine::handle() const
{
    return d->fd;
}

bool
QFSFileEngine::seek(QFile::Offset pos)
{
    if(QT_LSEEK(d->fd, pos, SEEK_SET) == -1) {
        qWarning("QFile::at: Cannot set file position %lld", pos);
        d->setError(QIODevice::PositionError, errno);
        return false;
    }
    d->ungetchBuffer.clear();
    return true;
}

bool
QFSFileEngine::isSequential() const
{
    return d->sequential;
}

QDateTime
QFSFileEngine::fileTime(FileTime time) const
{
    QDateTime ret;
    if(d->doStat()) {
        if(time == CreationTime)
            ret.setTime_t(d->st.st_ctime ? d->st.st_ctime : d->st.st_mtime);
        else if(time == ModificationTime)
            ret.setTime_t(d->st.st_mtime);
        else if(time == AccessTime)
            ret.setTime_t(d->st.st_atime);
    }
    return ret;
}

QIODevice::Status
QFSFileEngine::errorStatus() const
{
    return d->errorStatus;
}

QString 
QFSFileEngine::errorString() const
{
    return d->errorString;
}

QIOEngine::Type 
QFSFileEngine::type() const
{
    return QIOEngine::File;
}

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfileengine.h"
#include <qplatformdefs.h>
#include "qfileengine_p.h"
#include <qdatetime.h>

#include <private/qfsfileengine_p.h>

#include <errno.h>

#if defined(O_NONBLOCK)
# define HAS_ASYNC_FILEMODE
# define QT_OPEN_ASYNC O_NONBLOCK
#elif defined(O_NDELAY)
# define HAS_ASYNC_FILEMODE
# define QT_OPEN_ASYNC O_NDELAY
#endif

//************* QFileEngineHandler
/*!
    \class QFileEngineHandler
    \reentrant
    \internal

    \brief The QFileEngineHandler class allows custom QFileEngines to be
    plugged into Qt.

    \ingroup io

    You can create your own QFileEngine subclass to handle the files
    in particular paths. To make Qt aware of your QFileEngine
    subclass, create a QFileEngineHandler subclass and reimplement
    createFileEngine() to return an instance of your QFileEngine for
    the paths you are interested in. This will add your QFileEngine to
    Qt's list of file handlers and will ensure that your QFileEngine
    is used for files with paths that your createFileEngine()
    recognises.

    One way to instantiate your QFileEngineHandler is to make it a
    global static.
*/

static QList<QFileEngineHandler*> *fileHandlers;

/*!
   Constructs a QFileEngineHandler. Once created this handler's
   createFileEngine() function will be called (along with all the
   other handlers) for any paths used. The first handler (most
   recently created) that handles the given path (i.e. that returns a
   QFileEngine) is used for the new path.

   \sa createFileEngine()
 */
QFileEngineHandler::QFileEngineHandler()
{
    if(!fileHandlers)
        fileHandlers = new QList<QFileEngineHandler*>;
    fileHandlers->prepend(this);
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

/*!
    \fn QFileEngine *QFileEngineHandler::createFileEngine(const QString &path)

    This function is called when a new \a path is used. If \a path is
    a path for your QFileEngine subclass your reimplementation of this
    function must return a new instance of your subclass; the caller
    will take ownership and be responsible for the instance's
    destruction. If \a path is not a path that is handled by your
    subclass simply return 0; in this case a default QFileEngine will
    be used to handle the \a path.

    This virtual function must be reimplemented by all
    QFileEngineHandler subclasses.
 */


//**************** QFileEngine
/*!
    \class QFileEngine
    \reentrant
    \internal

    \brief The QFileEngine class provides an abstraction for accessing
    the filesystem.

    \ingroup io
    \mainclass

    The QDir, QFile, and QFileInfo classes all make use of a
    QFileEngine internally. If you create your own QFileEngine
    subclass (and register it with Qt by creating a QFileEngineHandler
    subclass), your file engine will be used when the path is one that
    your file engine handles.

    A QFileEngine refers to one file or one directory. If the referent
    is a file the setFileName(), rename(), and remove()
    functions are applicable. If the referent is a directory the
    mkdir(), rmdir(), and entryList() functions are
    applicable. In all cases the caseSensitive(), isRelativePath(),
    fileFlags(), ownerId(), owner(), and fileTime() functions are
    applicable.

    A QFileEngine subclass can be created to do syncronous network I/O
    based file system operations, local file system operations, or
    to operate as a resource system to access file based resources.

   \sa QFileEngineHandler, setFileName()
*/

/*!
   Constructs a new QFileEngine that does not refer to any file or directory.

   \sa setFileName()
 */
QFileEngine::QFileEngine() : d_ptr(new QFileEnginePrivate)
{
    d_ptr->q_ptr = this;
}

/*!
   \internal

   Constructs a QFileEngine.
 */
QFileEngine::QFileEngine(QFileEnginePrivate &dd) : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys the QFileEngine.
 */
QFileEngine::~QFileEngine()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
   Creates a new QFileEngine instance for the given \a file. This
   function will always return a valid QFileEngine. The default file
   engine that is created if no custom file engine handles files of
   \a{file}'s path is one that operates on the local file system.
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


/*!
    \fn void QFileEngine::setFileName(const QString &file)

    Sets the file engine's file name to \a file. This file name is the
    file that the rest of the virtual functions will operate on.

    This virtual function must be reimplemented by all subclasses.

    \sa rename()
 */

/*! \fn bool QFileEngine::open(int mode)

    Opens the file in the specified \a mode. Returns true if the file
    was successfully opened; otherwise returns false.

    The \a mode is an OR combination of QIODevice::OpenMode and
    QIODevice::HandlingMode values.
*/

/*! \fn bool QFileEngine::close()

    Closes the file.
*/

/*! \fn void QFileEngine::flush()

    Flushes the open file.
*/

/*! \fn qint64 QFileEngine::size() const

    Returns the size of the file.
*/

/*! \fn qint64 QFileEngine::at() const

    Returns the current file position.

    This is the position of the data read/write head of the file.
*/

/*! \fn bool QFileEngine::seek(qint64 offset)

    Sets the file position to the given \a offset. Returns true if
    the position was successfully set; otherwise returns false.

    The offset is from the beginning of the file, unless the
    file is sequential.

    \sa isSequential()
*/

/*! \fn bool QFileEngine::isSequential() const

    Returns true if the file is a sequential access device; returns
    false if the file is a direct access device.

    Operations involving size() and seek(int) are not valid on
    sequential devices.
*/

/*! \fn qint64 QFileEngine::read(char *data, qint64 maxSize)

    Reads a number of characters from the file into \a data. At most
    \a maxSize characters will be read.

    Returns -1 if a fatal error occurs, or 0 if there are no bytes to
    read.
*/

/*! \fn qint64 QFileEngine::write(const char *data, qint64 size)

    Writes \a size bytes from \a data to the file. Returns the number
    of characters written on success; otherwise returns -1.
*/

/*! \fn bool QFileEngine::link(const QString &newName)

    Creates a link from the file currently specified by fileName() to
    \a newName. What a link is depends on the underlying filesystem
    (be it a shortcut on Windows or a symbolic link on Unix). Returns
    true if successful; otherwise returns false.
*/

/*! \enum QFileEngine::Type

    \value File  The file is an actual file on the file system.
    \value Resource  The file is a Qt resource.

    \value User  The first value to be used for custom engine types.
    \value MaxUser  The last value to be used for custom engine types.

    \sa type()
*/

/*! \fn Type QFileEngine::type() const

    Returns the type of engine.

    If you reimplement this function, the value you return should be
    between \c User and \c MaxUser.
*/

/*!
  Returns the QFile::FileError that resulted from the last failed
  operation. If QFile::UnspecifiedError is returned, QFile will
  use its own idea of the error status.

  \sa QFile::FileError, errorString
 */
QFile::FileError QFileEngine::error() const
{
    return QFile::UnspecifiedError;
}

/*!
  Returns the human-readable message appropriate to the current error
  reported by error(). If no suitable string is available, an
  empty string is returned.

  \sa error()
 */
QString QFileEngine::errorString() const
{
    return QString();
}

/*!
  Maps the file contents from \a offset for the given number of bytes
  \a size, returning a pointer to the contents. If this fails, 0 is
  returned.

  The default implementation falls back to block reading/writing if
  this function returns 0.

  \sa unmap()
*/

uchar *QFileEngine::map(qint64 /* offset */, qint64 /* size */)
{
    return 0;
}

/*!
   Unmap previously mapped file \a data from memory.

   \sa QFileEngine::map()
*/
void QFileEngine::unmap(uchar * /* data */)
{
}

/*!
    \fn bool QFileEngine::remove()

    Requests that the file is deleted from the file system. If the
    operation succeeds return true; otherwise return false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName() rmdir()
 */

/*!
    \fn bool QFileEngine::rename(const QString &newName)

    Requests that the file be renamed to \a newName in the file
    system. If the operation succeeds return true; otherwise return
    false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
 */

/*!
    \fn bool QFileEngine::mkdir(const QString &dirName, bool createParentDirectories) const

    Requests that the directory \a dirName be created. If 
    \a createParentDirectories is true, then any sub-directories in \a dirName
    that don't exist must be created. If \a createParentDirectories is false then
    any sub-directories in \a dirName must already exist for the function to
    succeed. If the operation succeeds return true; otherwise return
    false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName() rmdir() isRelativePath()
 */

/*!
    \fn bool QFileEngine::rmdir(const QString &dirName, bool recurseParentDirectories) const

    Requests that the directory \a dirName is deleted from the file
    system. When \a recurseParentDirectories is true, then any empty
    parent-directories in \a dirName must also be deleted. If 
    \a recurseParentDirectories is false, only the \a dirName leaf-node
    should be deleted. In most file systems a directory cannot be deleted
    using this function if it is non-empty. If the operation succeeds
    return true; otherwise return false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName() remove() mkdir() isRelativePath()
 */

/*!
    \fn bool QFileEngine::setSize(qint64 size)

    Requests that the file be set to size \a size. If \a size is larger
    than the current file then it is filled with 0's, if smaller it is
    simply truncated. If the operations succceeds return true; otherwise
    return false;

    This virtual function must be reimplemented by all subclasses.

    \sa size()
*/

/*!
    \fn bool QFileEngine::chmod(uint perms)

    Requests that the file's permissions be set to \a perms. The argument
    perms will be set to the OR-ed together combination of
    QFileEngine::FileInfo, with only the QFileEngine::PermsMask being
    honored. If the operations succceeds return true; otherwise return
    false;

    This virtual function must be reimplemented by all subclasses.

    \sa size()
*/

/*!
    \fn QStringList QFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const

    Requests that a list of all the files matching the \a filters
    list based on the \a filterNames in the file engine's directory
    are returned.

    Should return an empty list if the file engine refers to a file
    rather than a directory, or if the directory is unreadable or does
    not exist or if nothing matches the specifications.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
 */

/*!
    \fn bool QFileEngine::caseSensitive() const

    Should return true if the underlying file system is case-sensitive;
    otherwise return false.

    This virtual function must be reimplemented by all subclasses.
 */

/*!
    \fn bool QFileEngine::isRelativePath() const

    Return true if the file referred to by this file engine has a
    relative path; otherwise return false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
 */

/*!
    \fn uint QFileEngine::fileFlags(FileFlags type) const

    This function should return the set of OR'd flags that are true
    for the file engine's file, and that are in the \a type's OR'd
    members.

    In your reimplementation you can use the \a type argument as an
    optimization hint and only return the OR'd set of members that are
    true and that match those in \a type; in other words you can
    ignore any members not mentioned in \a type, thus avoiding some
    potentially expensive lookups or system calls.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
*/

/*!
    \fn QString QFileEngine::fileName(FileName file) const

    Return  the file engine's current file name in the format
    specified by \a file.

    If you don't handle some \c FileName possibilities, return the
    file name set in setFileName() when an unhandled format is
    requested.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName(), FileName
 */

/*!
    \fn uint QFileEngine::ownerId(FileOwner owner) const

    If \a owner is \c OwnerUser return the ID of the user who owns
    the file. If \a owner is \c OwnerGroup return the ID of the group
    that own the file. If you can't determine the owner return -2.

    This virtual function must be reimplemented by all subclasses.

    \sa owner() setFileName(), FileOwner
 */

/*!
    \fn QString QFileEngine::owner(FileOwner owner) const

    If \a owner is \c OwnerUser return the name of the user who owns
    the file. If \a owner is \c OwnerGroup return the name of the group
    that own the file. If you can't determine the owner return
    QString().

    This virtual function must be reimplemented by all subclasses.

    \sa ownerId() setFileName(), FileOwner
 */

/*!
    \fn QDateTime QFileEngine::fileTime(FileTime time) const

    If \a time is \c CreationTime, return when the file was created.
    If \a time is \c ModificationTime, return when the file was most
    recently modified. If \a time is \c AccessTime, return when the
    file was most recently accessed (e.g. read or written).
    If the time cannot be determined return QDateTime() (an invalid
    date time).

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName(), QDateTime, QDateTime::isValid(), FileTime
 */

/*!
    \enum QFileEngine::FileName

    These values are used to request a file name in a particular
    format.

    \value DefaultName The same filename that was passed to the
    QFileEngine.
    \value BaseName The name of the file excluding the path.
    \value PathName The path to the file excluding the base name.
    \value AbsoluteName The absolute path to the file (including
    the base name).
    \value AbsolutePathName The absolute path to the file (excluding
    the base name).
    \value LinkName The full file name of the file that this file is a
link to. (This will be empty if this file is not a link.)
    \value CanonicalName Often very similar to LinkName. Will return the true path to the file.
    \value CanonicalPathName Same as CanonicalName, excluding the base name.

    \sa fileName(), setFileName()
*/

/*!
    \enum QFileEngine::FileFlag

    The permissions and types of a file, suitable for OR'ing together.

    \value ReadOwnerPerm The owner of the file has permission to read
    it.
    \value WriteOwnerPerm The owner of the file has permission to
    write to it.
    \value ExeOwnerPerm The owner of the file has permission to
    execute it.
    \value ReadUserPerm The current user has permission to read the
    file.
    \value WriteUserPerm The current user has permission to write to
    the file.
    \value ExeUserPerm The current user has permission to execute the
    file.
    \value ReadGroupPerm Members of the current user's group have
    permission to read the file.
    \value WriteGroupPerm Members of the current user's group have
    permission to write to the file.
    \value ExeGroupPerm Members of the current user's group have
    permission to execute the file.
    \value ReadOtherPerm All users have permission to read the file.
    \value WriteOtherPerm All users have permission to write to the
    file.
    \value ExeOtherPerm All users have permission to execute the file.

    \value LinkType The file is a link to another file (or link) in
    the file system (i.e. not a file or directory).
    \value FileType The file is a regular file to the file system
    (i.e. not a link or directory)
    \value DirectoryType The file is a directory in the file system
    (i.e. not a link or file).

    \value HiddenFlag The file is hidden.
    \value ExistsFlag The file actually exists in the file system.
    \value RootFlag  The file or the file pointed to is the root of the filesystem.
    \value LocalDiskFlag The file resides on the local disk and can be passed to standard file functions.

    \omitvalue PermsMask
    \omitvalue TypesMask
    \omitvalue FlagsMask
    \omitvalue FileInfoAll

    \sa fileFlags(), setFileName()
*/

/*!
    \enum QFileEngine::FileTime

    These are used by the fileTime() function.

    \value CreationTime When the file was created.
    \value ModificationTime When the file was most recently modified.
    \value AccessTime When the file was most recently accessed (e.g.
    read or written to).

    \sa setFileName()
*/

/*!
    \enum QFileEngine::FileOwner

    \value OwnerUser The user who owns the file.
    \value OwnerGroup The group who owns the file.

    \sa owner(), ownerId(), setFileName()
*/

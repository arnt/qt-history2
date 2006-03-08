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

#include "qplatformdefs.h"
#include "qfile.h"
#include "qfsfileengine.h"
#include "qtemporaryfile.h"
#include "qlist.h"
#include "qfileinfo.h"
#include "private/qiodevice_p.h"
#include "private/qfile_p.h"
#include "private/qunicodetables_p.h"
#if defined(QT_BUILD_CORE_LIB)
# include "qcoreapplication.h"
#endif

#include <errno.h>

static QByteArray locale_encode(const QString &f)
{
#ifndef Q_OS_DARWIN
    return f.toLocal8Bit();
#else
    // Mac always expects UTF-8
    return f.toUtf8();
#endif
}

static QString locale_decode(const QByteArray &f)
{
#ifndef Q_OS_DARWIN
    return QString::fromLocal8Bit(f);
#else
    // Mac always expects UTF-8
    return QUnicodeTables::normalize(QString::fromUtf8(f), QString::NormalizationForm_C);
#endif
}

//************* QFilePrivate
QFile::EncoderFn QFilePrivate::encoder = locale_encode;
QFile::DecoderFn QFilePrivate::decoder = locale_decode;

QFilePrivate::QFilePrivate()
    : fileEngine(0), error(QFile::NoError)
{
}

QFilePrivate::~QFilePrivate()
{
    delete fileEngine;
    fileEngine = 0;
}

bool
QFilePrivate::openExternalFile(int flags, int fd)
{
    delete fileEngine;
    QFSFileEngine *fe = new QFSFileEngine;
    fe->setFileName(fileName);
    fileEngine = fe;
    return fe->open(QIODevice::OpenMode(flags), fd);
}

bool
QFilePrivate::openExternalFile(int flags, FILE *fh)
{
    delete fileEngine;
    QFSFileEngine *fe = new QFSFileEngine;
    fe->setFileName(fileName);
    fileEngine = fe;
    return fe->open(QIODevice::OpenMode(flags), fh);
}

void
QFilePrivate::setError(QFile::FileError err)
{
    error = err;
    errorString.clear();
}

void
QFilePrivate::setError(QFile::FileError err, const QString &errStr)
{
    error = err;
    errorString = errStr;
}

void
QFilePrivate::setError(QFile::FileError err, int errNum)
{
    error = err;
    errorString = qt_error_string(errNum);
}

//************* QFile

/*!
    \class QFile
    \brief The QFile class provides an interface for reading from and writing to files.

    \ingroup io
    \mainclass
    \reentrant

    QFile is an I/O device for reading and writing text and binary
    files and \l{The Qt Resource System}{resources}. A QFile may be
    used by itself or, more conveniently, with a QTextStream or
    QDataStream.

    The file name is usually passed in the constructor, but it can be
    set at any time using setFileName(). You can check for a file's
    existence using exists(), and remove a file using remove(). (More
    advanced file system related operations are provided by QFileInfo
    and QDir.)

    The file is opened with open(), closed with close(), and flushed
    with flush(). Data is usually read and written using QDataStream
    or QTextStream, but you can also call the QIODevice-inherited
    functions read(), readLine(), readAll(), write(). QFile also
    inherits getChar(), putChar(), and ungetChar(), which work one
    character at a time.

    The size of the file is returned by size(). You can get the
    current file position using pos(), or move to a new file position
    using seek(). If you've reached the end of the file, atEnd()
    returns true.

    The following example reads a text file line by line:

    \quotefromfile snippets/file/file.cpp
    \skipto noStream_snippet
    \skipto QFile
    \printto /^\}/

    The QIODevice::Text flag passed to open() tells Qt to convert
    Windows-style line terminators ("\\r\\n") into C++-style
    terminators ("\\n"). By default, QFile assumes binary, i.e. it
    doesn't perform any conversion on the bytes stored in the file.

    The next example uses QTextStream to read a text file
    line by line:

    \skipto readTextStream_snippet
    \skipto QFile
    \printto /^\}/

    QTextStream takes care of converting the 8-bit data stored on
    disk into a 16-bit Unicode QString. By default, it assumes that
    the user system's local 8-bit encoding is used (e.g., ISO 8859-1
    for most of Europe; see QTextCodec::codecForLocale() for
    details). This can be changed using setCodec().

    To write text, we can use operator<<(), which is overloaded to
    take a QTextStream on the left and various data types (including
    QString) on the right:

    \skipto writeTextStream_snippet
    \skipto QFile
    \printto /^\}/

    QDataStream is similar, in that you can use operator<<() to write
    data and operator>>() to read it back. See the class
    documentation for details.

    When you use QFile, QFileInfo, and QDir to access the file system
    with Qt, you can use Unicode file names. On Unix, these file
    names are converted to an 8-bit encoding. If you want to use
    standard C++ APIs (\c <cstdio> or \c <iostream>) or
    platform-specific APIs to access files instead of QFile, you can
    use the encodeName() and decodeName() functions to convert
    between Unicode file names and 8-bit file names.

    On Unix, there are some special system files (e.g. in \c /proc) for which
    size() will always return 0, yet you may still be able to read more data
    from such a file; the data is generated in direct response to you calling
    read(). In this case, however, you cannot use atEnd() to determine if
    there is more data to read (since atEnd() will return true for a file that
    claims to have size 0). Instead, you should either call readAll(), or call
    read() or readLine() repeatedly until no more data can be read. The next
    example uses QTextStream to read \c /proc/modules line by line:
    
    \skipto readRegularEmptyFile_snippet
    \skipto QFile
    \printto /^\}/

    \sa QTextStream, QDataStream, QFileInfo, QDir, {The Qt Resource System}
*/

/*!
    \enum QFile::FileError

    This enum describes the errors that may be returned by the error()
    function.

    \value NoError          No error occurred.
    \value ReadError        An error occurred when reading from the file.
    \value WriteError       An error occurred when writing to the file.
    \value FatalError       A fatal error occurred.
    \value ResourceError
    \value OpenError        The file could not be opened.
    \value AbortError       The operation was aborted.
    \value TimeOutError     A timeout occurred.
    \value UnspecifiedError An unspecified error occurred.
    \value RemoveError      The file could not be removed.
    \value RenameError      The file could not be renamed.
    \value PositionError    The position in the file could not be changed.
    \value ResizeError      The file could not be resized.
    \value PermissionsError The file could not be accessed.
    \value CopyError        The file could not be copied.

    \omitvalue ConnectError
*/

/*!
    \enum QFile::Permission

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

    \warning Because of differences in the platforms supported by Qt,
    the semantics of ReadUser, WriteUser and ExeUser are
    platform-dependent: On Unix, the rights of the owner of the file
    are returned and on Windows the rights of the current user are
    returned. This behavior might change in a future Qt version.
*/

#ifdef QT3_SUPPORT
/*!
    \typedef QFile::PermissionSpec

    Use QFile::Permission instead.
*/
#endif

#ifdef QT_NO_QOBJECT
QFile::QFile()
    : QIODevice(*new QFilePrivate)
{
    unsetError();
}
QFile::QFile(const QString &name)
    : QIODevice(*new QFilePrivate)
{
    d_func()->fileName = name;
    unsetError();
}
QFile::QFile(QFilePrivate &dd)
    : QIODevice(dd)
{
    unsetError();
}
#else
/*!
    \internal
*/
QFile::QFile()
    : QIODevice(*new QFilePrivate, 0)
{
    unsetError();
}
/*!
    Constructs a new file object with the given \a parent.
*/
QFile::QFile(QObject *parent)
    : QIODevice(*new QFilePrivate, parent)
{
    unsetError();
}
/*!
    Constructs a new file object to represent the file with the given \a name.
*/
QFile::QFile(const QString &name)
    : QIODevice(*new QFilePrivate, 0)
{
    Q_D(QFile);
    d->fileName = name;
    unsetError();
}
/*!
    Constructs a new file object with the given \a parent to represent the
    file with the specified \a name.
*/
QFile::QFile(const QString &name, QObject *parent)
    : QIODevice(*new QFilePrivate, parent)
{
    Q_D(QFile);
    d->fileName = name;
    unsetError();
}
/*!
    \internal
*/
QFile::QFile(QFilePrivate &dd, QObject *parent)
    : QIODevice(dd, parent)
{
    unsetError();
}
#endif

/*!
    Destroys the file object, closing it if necessary.
*/
QFile::~QFile()
{
    close();
#ifdef QT_NO_QOBJECT
    delete d_ptr;
#endif
}

/*!
    Returns the name set by setFileName().

    \sa setFileName(), QFileInfo::fileName()
*/
QString QFile::fileName() const
{
    return fileEngine()->fileName(QAbstractFileEngine::DefaultName);
}

/*!
    Sets the \a name of the file. The name can have no path, a
    relative path, or an absolute path.

    Do not call this function if the file has already been opened.

    If the file name has no path or a relative path, the path used
    will be the application's current directory path
    \e{at the time of the open()} call.

    Example:
    \code
        QFile file;
        QDir::setCurrent("/tmp");
        file.setFileName("readme.txt");
        QDir::setCurrent("/home");
        file.open(QIODevice::ReadOnly);      // opens "/home/readme.txt" under Unix
    \endcode

    Note that the directory separator "/" works for all operating
    systems supported by Qt.

    \sa fileName(), QFileInfo, QDir
*/
void
QFile::setFileName(const QString &name)
{
    Q_D(QFile);
    if (isOpen()) {
        qWarning("QFile::setFileName: File is already opened");
        close();
    }
    if(d->fileEngine) { //get a new file engine later
        delete d->fileEngine;
        d->fileEngine = 0;
    }
    d->fileName = name;
}

/*!
    \fn QString QFile::decodeName(const char *localFileName)

    \overload

    Returns the Unicode version of the given \a localFileName. See
    encodeName() for details.
*/

/*!
    By default, this function converts \a fileName to the local 8-bit
    encoding determined by the user's locale. This is sufficient for
    file names that the user chooses. File names hard-coded into the
    application should only use 7-bit ASCII filename characters.

    \sa decodeName() setEncodingFunction()
*/

QByteArray
QFile::encodeName(const QString &fileName)
{
    return (*QFilePrivate::encoder)(fileName);
}

/*!
    \typedef QFile::EncoderFn

    This is a typedef for a pointer to a function with the following
    signature:

    \code
        QByteArray myEncoderFunc(const QString &fileName);
    \endcode

    \sa setEncodingFunction(), encodeName()
*/

/*!
    This does the reverse of QFile::encodeName() using \a localFileName.

    \sa setDecodingFunction(), encodeName()
*/

QString
QFile::decodeName(const QByteArray &localFileName)
{
    return (*QFilePrivate::decoder)(localFileName);
}

/*!
    \fn void QFile::setEncodingFunction(EncoderFn function)

    \nonreentrant

    Sets the \a function for encoding Unicode file names. The
    default encodes in the locale-specific 8-bit encoding.

    \sa encodeName(), setDecodingFunction()
*/

void
QFile::setEncodingFunction(EncoderFn f)
{
    QFilePrivate::encoder = f;
}

/*!
    \typedef QFile::DecoderFn

    This is a typedef for a pointer to a function with the following
    signature:

    \code
        QString myDecoderFunc(const QByteArray &localFileName);
    \endcode

    \sa setDecodingFunction()
*/

/*!
    \fn void QFile::setDecodingFunction(DecoderFn function)

    \nonreentrant

    Sets the \a function for decoding 8-bit file names. The
    default uses the locale-specific 8-bit encoding.

    \sa setEncodingFunction(), decodeName()
*/

void
QFile::setDecodingFunction(DecoderFn f)
{
    QFilePrivate::decoder = f;
}

/*!
    \overload

    Returns true if the file specified by fileName() exists; otherwise
    returns false.

    \sa fileName(), setFileName()
*/

bool
QFile::exists() const
{
    // 0x1000000 = QAbstractFileEngine::Refresh, forcing an update
    return (fileEngine()->fileFlags(QAbstractFileEngine::FlagsMask
                                    | QAbstractFileEngine::FileFlag(0x1000000)) & QAbstractFileEngine::ExistsFlag);
}

/*!
    Returns true if the file specified by \a fileName exists; otherwise
    returns false.
*/

bool
QFile::exists(const QString &fileName)
{
    return QFileInfo(fileName).exists();
}

/*!
    \overload

    Returns the absolute path of the file or directory a symlink (or shortcut
    on Windows) points to, or a an empty string if the object isn't a symbolic
    link.

    This name may not represent an existing file; it is only a string.
    QFile::exists() returns true if the symlink points to an existing file.

    \sa fileName() setFileName()
*/

QString
QFile::readLink() const
{
    return fileEngine()->fileName(QAbstractFileEngine::LinkName);
}

/*!
    Returns the absolute path of the file or directory referred to by the
    symlink (or shortcut on Windows) specified by \a fileName, or returns an
    empty string if the \a fileName does not correspond to a symbolic link.

    This name may not represent an existing file; it is only a string.
    QFile::exists() returns true if the symlink points to an existing file.
*/

QString
QFile::readLink(const QString &fileName)
{
    return QFileInfo(fileName).readLink();
}

/*!
    Removes the file specified by fileName(). Returns true if successful;
    otherwise returns false.

    The file is closed before it is removed.

    \sa setFileName()
*/

bool
QFile::remove()
{
    Q_D(QFile);
    if (d->fileName.isEmpty()) {
        qWarning("QFile::remove: Empty or null file name");
        return false;
    }
    close();
    if(error() == QFile::NoError) {
        if(fileEngine()->remove()) {
            unsetError();
            return true;
        }
        d->setError(QFile::RemoveError, errno);
    }
    return false;
}

/*!
    \overload

    Removes the file specified by the \a fileName given.

    Returns true if successful; otherwise returns false.

    \sa remove()
*/

bool
QFile::remove(const QString &fileName)
{
    return QFile(fileName).remove();
}

/*!
    Renames the file currently specified by fileName() to \a newName.
    Returns true if successful; otherwise returns false.

    If a file with the name \a newName already exists, rename() returns false
    (i.e., QFile will not overwrite it).
    
    The file is closed before it is renamed.

    \sa setFileName()
*/

bool
QFile::rename(const QString &newName)
{
    Q_D(QFile);
    if (d->fileName.isEmpty()) {
        qWarning("QFile::rename: Empty or null file name");
        return false;
    }
    if (QFile(newName).exists()) {
        // ### Race condition. If a file is moved in after this, it /will/ be
        // overwritten. On Unix, the proper solution is to use hardlinks:
        // return ::link(old, new) && ::remove(old);
        return false;
    }
    close();
    if(error() == QFile::NoError) {
        if(fileEngine()->rename(newName)) {
            unsetError();
            return true;
        } else {
            QFile in(fileName());
            QFile out(newName);
            if (in.open(QIODevice::ReadOnly)) {
                if(out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    bool error = false;
                    char block[1024];
                    while(!in.atEnd()) {
                        long read = in.read(block, 1024);
                        if(read == -1)
                            break;
                        if(read != out.write(block, read)) {
                            d->setError(QFile::CopyError, QLatin1String("Failure to write block"));
                            error = true;
                            break;
                        }
                    }
                    if(!error)
                        in.remove();
                    return !error;
                 }
            }
        }
        d->setError(QFile::RenameError, errno);
    }
    return false;
}

/*!
    \overload

    Renames the file \a oldName to \a newName. Returns true if
    successful; otherwise returns false.

    If a file with the name \a newName already exists, rename() returns false
    (i.e., QFile will not overwrite it).

    \sa rename()
*/

bool
QFile::rename(const QString &oldName, const QString &newName)
{
    return QFile(oldName).rename(newName);
}

/*!
    Creates a link named \a linkName that points to the file currently specified by fileName(). 
    What a link is depends on the underlying filesystem
    (be it a shortcut on Windows or a symbolic link on Unix). Returns
    true if successful; otherwise returns false.

    \sa setFileName()
*/

bool
QFile::link(const QString &linkName)
{
    Q_D(QFile);
    if (d->fileName.isEmpty()) {
        qWarning("QFile::link: Empty or null file name");
        return false;
    }
    QFileInfo fi(linkName);
    if(fileEngine()->link(fi.absoluteFilePath())) {
        unsetError();
        return true;
    }
    d->setError(QFile::RenameError, errno);
    return false;
}

/*!
    \overload

    Creates a link named \a linkName that points to the file \a fileName. What a link is
    depends on the underlying filesystem (be it a shortcut on Windows
    or a symbolic link on Unix). Returns true if successful; otherwise
    returns false.

    \sa link()
*/

bool
QFile::link(const QString &fileName, const QString &linkName)
{
    return QFile(fileName).link(linkName);
}

/*!
    Copies the file currently specified by fileName() to \a newName.
    Returns true if successful; otherwise returns false.

    If a file with the name \a newName already exists, copy() returns false
    (i.e., QFile will not overwrite it).

    The file is closed before it is copied.

    \sa setFileName()
*/

bool
QFile::copy(const QString &newName)
{
    Q_D(QFile);
    if (d->fileName.isEmpty()) {
        qWarning("QFile::copy: Empty or null file name");
        return false;
    }
    close();
    if(error() == QFile::NoError) {
        if(fileEngine()->copy(newName)) {
            unsetError();
            return true;
        } else {
            bool error = false;
            if(!open(QFile::ReadOnly)) {
                error = true;
                QString errorMessage = QLatin1String("Cannot open %1 for input");
                d->setError(QFile::CopyError, errorMessage.arg(d->fileName));
            } else {
                QTemporaryFile out;
                if(!out.open()) {
                    close();
                    error = true;
                    d->setError(QFile::CopyError, QLatin1String("Cannot open for output"));
                } else {
                    char block[1024];
                    while(!atEnd()) {
                        qint64 in = read(block, 1024);
                        if(in == -1)
                            break;
                        if(in != out.write(block, in)) {
                            d->setError(QFile::CopyError, QLatin1String("Failure to write block"));
                            error = true;
                            break;
                        }
                    }
                    if(!error && !out.rename(newName)) {
                        error = true;
                        QString errorMessage = QLatin1String("Cannot create %1 for output");
                        d->setError(QFile::CopyError, errorMessage.arg(newName));
                    }
                }
            }
            if(!error) {
                QFile::setPermissions(newName, permissions());
                unsetError();
                return true;
            }
        }
    }
    return false;
}

/*!
    \overload

    Copies the file \a fileName to \a newName. Returns true if successful;
    otherwise returns false.

    If a file with the name \a newName already exists, copy() returns false
    (i.e., QFile will not overwrite it).

    \sa rename()
*/

bool
QFile::copy(const QString &fileName, const QString &newName)
{
    return QFile(fileName).copy(newName);
}

/*!
    Returns true if the file can only be manipulated sequentially;
    otherwise returns false.

    Most files support random-access, but some special files may not.

    \sa QIODevice::isSequential()
*/
bool QFile::isSequential() const
{
    Q_D(const QFile);
    return d->fileEngine && d->fileEngine->isSequential();
}

/*!
    Opens the file using OpenMode \a mode.

    The \a mode must be QIODevice::ReadOnly, QIODevice::WriteOnly, or
    QIODevice::ReadWrite. It may also have additional flags, such as
    QIODevice::Text and QIODevice::Unbuffered.

    \sa QIODevice::OpenMode
*/
bool QFile::open(OpenMode mode)
{
    Q_D(QFile);
    if (isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    if (mode & Append)
        mode |= WriteOnly;

    unsetError();
    if ((mode & (ReadOnly | WriteOnly)) == 0) {
        qWarning("QIODevice::open: File access not specified");
        return false;
    }
    if (fileEngine()->open(mode)) {
        setOpenMode(mode);
        if (mode & Append)
            seek(size());
        return true;
    }
    QFile::FileError err = fileEngine()->error();
    if(err == QFile::UnspecifiedError)
        err = QFile::OpenError;
    d->setError(err, fileEngine()->errorString());
    return false;
}

/*! \fn QFile::open(OpenMode, FILE*)

    Use open(FILE *, OpenMode) instead.
*/

/*!
    \overload

    Opens the existing file handle \a fh in the given \a mode.
    Returns true if successful; otherwise returns false.

    Example:
    \code
        #include <stdio.h>

        void printError(const char* msg)
        {
            QFile file;
            file.open(stderr, QIODevice::WriteOnly);
            file.write(msg, qstrlen(msg));        // write to stderr
            file.close();
        }
    \endcode

    When a QFile is opened using this function, close() does not actually
    close the file, but only flushes it.

    \warning If \a fh is \c stdin, \c stdout, or \c stderr, you may not be
    able to seek(). See QIODevice::isSequentialAccess() for more
    information.

    \sa close()
*/
bool QFile::open(FILE *fh, OpenMode mode)
{
    Q_D(QFile);
    if (isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    if (mode & Append)
        mode |= WriteOnly;
    unsetError();
    if ((mode & (ReadOnly | WriteOnly)) == 0) {
        qWarning("QFile::open: File access not specified");
        return false;
    }
    if(d->openExternalFile(mode, fh)) {
        setOpenMode(mode);
        if (mode & Append) {
            seek(size());
        } else {
            long pos = ftell(fh);
            if (pos != -1)
                seek(pos);
        }
        return true;
    }
    return false;
}

/*! \fn QFile::open(OpenMode, int)

    Use open(int, OpenMode) instead.
*/

/*!
    \overload

    Opens the existing file descripter \a fd in the given \a mode.
    Returns true if successful; otherwise returns false.

    When a QFile is opened using this function, close() does not
    actually close the file.

    The QFile that is opened using this function is automatically set
    to be in raw mode; this means that the file input/output functions
    are slow. If you run into performance issues, you should try to
    use one of the other open functions.

    \warning If \a fd is 0 (\c stdin), 1 (\c stdout), or 2 (\c
    stderr), you may not be able to seek(). size() is set to \c
    LLONG_MAX (in \c <climits>).

    \sa close()
*/
bool QFile::open(int fd, OpenMode mode)
{
    Q_D(QFile);
    if (isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    if (mode & Append)
        mode |= WriteOnly;
    unsetError();
    if ((mode & (ReadOnly | WriteOnly)) == 0) {
        qWarning("QFile::open: File access not specified");
        return false;
    }
    if(d->openExternalFile(mode, fd)) {
        setOpenMode(mode);
        if (mode & Append)
            seek(size());
        return true;
    }
    return false;
}

/*!
  Returns the file handle of the file.

  This is a small positive integer, suitable for use with C library
  functions such as fdopen() and fcntl(). On systems that use file
  descriptors for sockets (i.e. Unix systems, but not Windows) the handle
  can be used with QSocketNotifier as well.

  If the file is not open, or there is an error, handle() returns -1.

  \sa QSocketNotifier
*/

int
QFile::handle() const
{
    if (!isOpen())
        return -1;
    
    if (QAbstractFileEngine *engine = fileEngine())
        return engine->handle();
    return -1;
}

/*!
    \fn QString QFile::name() const

    Use fileName() instead.
*/

/*!
    \fn void QFile::setName(const QString &name)

    Use setFileName() instead.
*/

/*!
    Sets the file size (in bytes) \a sz. Returns true if the file if the
    resize succeeds; false otherwise. If \a sz is larger than the file
    currently is the new bytes will be set to 0, if \a sz is smaller the
    file is simply truncated.

    \sa size(), setFileName()
*/

bool
QFile::resize(qint64 sz)
{
    Q_D(QFile);
    if (isOpen() && fileEngine()->pos() > sz)
        seek(sz);
    if(fileEngine()->setSize(sz)) {
        unsetError();
        return true;
    }
    d->setError(QFile::ResizeError, errno);
    return false;
}

/*!
    \overload

    Sets \a fileName to size (in bytes) \a sz. Returns true if the file if
    the resize succeeds; false otherwise. If \a sz is larger than \a
    fileName currently is the new bytes will be set to 0, if \a sz is
    smaller the file is simply truncated.

    \sa resize()
*/

bool
QFile::resize(const QString &fileName, qint64 sz)
{
    return QFile(fileName).resize(sz);
}

/*!
    Returns the complete OR-ed together combination of
    QFile::Permission for the file.

    \sa setPermissions(), setFileName()
*/

QFile::Permissions
QFile::permissions() const
{
    QAbstractFileEngine::FileFlags perms = fileEngine()->fileFlags(QAbstractFileEngine::PermsMask) & QAbstractFileEngine::PermsMask;
    return QFile::Permissions((int)perms); //ewww
}

/*!
    \overload

    Returns the complete OR-ed together combination of
    QFile::Permission for \a fileName.
*/

QFile::Permissions
QFile::permissions(const QString &fileName)
{
    return QFile(fileName).permissions();
}

/*!
    Sets the permissions for the file to \a permissions.

    \sa permissions(), setFileName()
*/

bool
QFile::setPermissions(Permissions permissions)
{
    Q_D(QFile);
    if(fileEngine()->setPermissions(permissions)) {
        unsetError();
        return true;
    }
    d->setError(QFile::PermissionsError, errno);
    return false;
}

/*!
    \overload

    Sets the permissions for \a fileName file to \a permissions.
*/

bool
QFile::setPermissions(const QString &fileName, Permissions permissions)
{
    return QFile(fileName).setPermissions(permissions);
}

/*!
    Flushes any buffered data to the file.
*/

bool
QFile::flush()
{
    Q_D(QFile);
    if (!fileEngine()->flush()) {
        QFile::FileError err = fileEngine()->error();
        if(err == QFile::UnspecifiedError)
            err = QFile::WriteError;
        d->setError(err, fileEngine()->errorString());
        return false;
    }
    return true;
}

/*!
  \reimp
*/

void
QFile::close()
{
    Q_D(QFile);
    if(!isOpen())
        return;
    QIODevice::close();

    unsetError();
    if(!fileEngine()->close())
        d->setError(fileEngine()->error(), fileEngine()->errorString());
}

/*!
  Returns the size of the file.

  For regular empty files on Unix (e.g. those in \c /proc), this function
  returns 0; the contents of such a file are generated on demand in response
  to you calling read().
*/

qint64 QFile::size() const
{
    return fileEngine()->size();
}

/*!
  \reimp
*/

qint64 QFile::pos() const
{
    return QIODevice::pos();
}

/*!
  Returns true if the end of the file has been reached; otherwise returns
  false.
  
  For regular empty files on Unix (e.g. those in \c /proc), this function
  returns true, since the file system reports that the size of such a file is
  0. Therefore, you should not depend on atEnd() when reading data from such a
  file, but rather call read() until no more data can be read.
*/

bool QFile::atEnd() const
{
    if (!isOpen())
        return true;
    return QIODevice::atEnd() || (isSequential() && bytesAvailable() == 0);
}

/*!
  \reimp
*/

bool QFile::seek(qint64 off)
{
    Q_D(QFile);
    if (!isOpen()) {
        qWarning("QFile::seek: IODevice is not open");
        return false;
    }

    QIODevice::seek(off);
    if (!fileEngine()->seek(off)) {
        QFile::FileError err = fileEngine()->error();
        if(err == QFile::UnspecifiedError)
            err = QFile::PositionError;
        d->setError(err, fileEngine()->errorString());
        return false;
    }
    unsetError();
    return true;
}

/*!
  \reimp
*/
qint64 QFile::readLineData(char *data, qint64 maxlen)
{
    return fileEngine()->readLine(data, maxlen);
}

/*!
  \reimp
*/

qint64 QFile::readData(char *data, qint64 len)
{
    Q_D(QFile);
    unsetError();

    qint64 ret = 0;
    qint64 read = fileEngine()->read(data+ret, len-ret);
    if (read != -1)
        ret += read;

    if(ret < 0) {
        QFile::FileError err = fileEngine()->error();
        if(err == QFile::UnspecifiedError)
            err = QFile::ReadError;
        d->setError(err, fileEngine()->errorString());
    }
    return ret;
}

/*!
  \reimp
*/

qint64
QFile::writeData(const char *data, qint64 len)
{
    Q_D(QFile);
    unsetError();

    qint64 ret = fileEngine()->write(data, len);
    if(ret < 0) {
        QFile::FileError err = fileEngine()->error();
        if(err == QFile::UnspecifiedError)
            err = QFile::WriteError;
        d->setError(err, fileEngine()->errorString());
    }
    return ret;
}

/*!
    \internal
    Returns the QIOEngine for this QFile object.
*/
QAbstractFileEngine *QFile::fileEngine() const
{
    Q_D(const QFile);
    if(!d->fileEngine)
        d->fileEngine = QAbstractFileEngine::create(d->fileName);
    return d->fileEngine;
}

/*!
    Returns the file error status.

    The I/O device status returns an error code. For example, if open()
    returns false, or a read/write operation returns -1, this function can
    be called to find out the reason why the operation failed.

    \sa unsetError()
*/

QFile::FileError
QFile::error() const
{
    Q_D(const QFile);
    return d->error;
}

/*!
    Sets the file's error to QFile::NoError.

    \sa error()
*/
void
QFile::unsetError()
{
    Q_D(QFile);
    d->setError(QFile::NoError);
}

/*
    Returns a human-readable description of an error that occurred on
    the device. The error described by the string corresponds to
    changes of QFile::error(). If the status is reset, the error
    string is also reset.

    \code
        QFile file("address.dat");
        if (!file.open(QIODevice::ReadOnly) {
            QMessageBox::critical(this, tr("Error"),
                    tr("Could not open file for reading: %1")
                    .arg(file.errorString()));
            return;
        }
    \endcode

    \sa unsetError()
*/

/* ### NEEDS TO BE FIXED
QString
QFile::errorString() const
{
    if (d->errorString.isEmpty()) {
        const char *str = 0;
        switch (d->error) {
        case NoError:
        case UnspecifiedError:
            str = QT_TRANSLATE_NOOP("QFile", "Unknown error");
            break;
        case ReadError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not read from the file");
            break;
        case WriteError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not write to the file");
            break;
        case FatalError:
            str = QT_TRANSLATE_NOOP("QFile", "Fatal error");
            break;
        case ResourceError:
            str = QT_TRANSLATE_NOOP("QFile", "Resource error");
            break;
        case OpenError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not open the file");
            break;
#ifdef QT3_SUPPORT
        case ConnectError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not connect to host");
            break;
#endif
        case AbortError:
            str = QT_TRANSLATE_NOOP("QFile", "Aborted");
            break;
        case TimeOutError:
            str = QT_TRANSLATE_NOOP("QFile", "Timeout");
            break;
        case RemoveError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not remove file");
            break;
        case RenameError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not rename file");
            break;
        case PositionError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not position in file");
            break;
        case PermissionsError:
            str = QT_TRANSLATE_NOOP("QFile", "Failure to set Permissions");
            break;
        case CopyError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not copy file");
            break;
        case ResizeError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not resize file");
            break;
        }
#if defined(QT_BUILD_CORE_LIB)
        QString ret = QCoreApplication::translate("QFile", str);
#ifdef QT3_SUPPORT
        if(ret == str)
            ret = QCoreApplication::translate("QIODevice", str);
#endif
        return ret;
#else
        return QString::fromLatin1(str);
#endif
    }
    return d->errorString;
}
*/

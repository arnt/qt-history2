/****************************************************************************
**
** Implementation of QFile class.
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

#include "qfile.h"
#include <qplatformdefs.h>
#include <qglobal.h>
#include <private/qiodevice_p.h>
#include <qfileengine.h>
#include <qfileinfo.h>
#include <qlist.h>

#include <errno.h>

#define d d_func()
#define q q_func()

//************* QFileEngineHandler
static QList<QFileEngineHandler*> *fileHandlers;
QFileEngineHandler::QFileEngineHandler()
{
    if(!fileHandlers)
        fileHandlers = new QList<QFileEngineHandler*>;
    fileHandlers->append(this);
}

QFileEngineHandler::~QFileEngineHandler()
{
    fileHandlers->removeAll(this);
    if(fileHandlers->isEmpty()) {
        delete fileHandlers;
        fileHandlers = 0;
    }
}
QFileEngine *qt_createFileEngine(const QString &path)
{
    if(fileHandlers) {
        for(int i = 0; i < fileHandlers->size(); i++) {
            if(QFileEngine *ret = fileHandlers->at(i)->createFileEngine(path))
                return ret;
        }
    }
    return new QFSFileEngine;
}

//************* QFilePrivate
class QFilePrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QFile)

protected:
    QFilePrivate();
    ~QFilePrivate();

    void initFileEngine(const QString &);

    bool openExternalFile(int fd);
    void reset();

private:
    inline static QByteArray locale_encode(const QString &f) 
           { return f.toLocal8Bit(); }
    static QFile::EncoderFn encoder;
    inline static QString locale_decode(const QByteArray &f) 
           { return QString::fromLocal8Bit(f); }
    static QFile::DecoderFn decoder;
    
    QFileEngine *fileEngine;

    QString fileName;
    uint external_file : 1;
    QByteArray ungetchBuffer;
};

QFile::EncoderFn QFilePrivate::encoder = QFilePrivate::locale_encode;
QFile::DecoderFn QFilePrivate::decoder = QFilePrivate::locale_decode;

QFilePrivate::QFilePrivate() : fileEngine(0), external_file(0)
{ 
    reset();
}

QFilePrivate::~QFilePrivate()
{
    delete fileEngine;
    fileEngine = 0;
}

void 
QFilePrivate::initFileEngine(const QString &file)
{
    Q_ASSERT(!fileEngine || !fileEngine->isOpen());
    delete fileEngine;
    fileName = file;
    fileEngine = qt_createFileEngine(file);
    reset();
}

bool
QFilePrivate::openExternalFile(int fd)
{
    Q_ASSERT(!fileEngine || !fileEngine->isOpen());
    delete fileEngine;
    external_file = true;
    QFSFileEngine *fe = new QFSFileEngine;
    fileEngine = fe;
    reset();
    return fe->open(fd);
}

void 
QFilePrivate::reset()
{
    ungetchBuffer.clear();
}

//************* QFile

/*!
    \class QFile
    \reentrant
    \brief The QFile class is an I/O device that operates on files.

    \ingroup io
    \mainclass

    QFile is an I/O device for reading and writing binary and text
    files. A QFile may be used by itself or more conveniently with a
    QDataStream or QTextStream.

    The file name is usually passed in the constructor but can be
    changed with setName(). You can check for a file's existence with
    exists() and remove a file with remove().

    The file is opened with open(), closed with close() and flushed
    with flush(). Data is usually read and written using QDataStream
    or QTextStream, but you can read with readBlock() and readLine()
    and write with writeBlock(). QFile also supports getch(),
    ungetch() and putch().

    The size of the file is returned by size(). You can get the
    current file position or move to a new file position using the
    at() functions. If you've reached the end of the file, atEnd()
    returns true. The file handle is returned by handle().

    Here is a code fragment that uses QTextStream to read a text file
    line by line. It prints each line with a line number.
    \code
    QStringList lines;
    QFile file("file.txt");
    if (file.open(IO_ReadOnly)) {
        QTextStream stream(&file);
        QString line;
        int i = 1;
        while (!stream.atEnd()) {
            line = stream.readLine(); // line of text excluding '\n'
            printf("%3d: %s\n", i++, line.latin1());
            lines += line;
        }
        file.close();
    }
    \endcode

    Writing text is just as easy. The following example shows how to
    write the data we read into the string list from the previous
    example:
    \code
    QFile file("file.txt");
    if (file.open(IO_WriteOnly)) {
        QTextStream stream(&file);
        QStringList::ConstIterator i = lines.constBegin();
        for (; i != lines.constEnd(); ++i)
            stream << *i << "\n";
        file.close();
    }
    \endcode

    The QFileInfo class holds detailed information about a file, such
    as access permissions, file dates and file types.

    The QDir class manages directories and lists of file names.

    Qt uses Unicode file names. If you want to do your own I/O on Unix
    systems you may want to use encodeName() (and decodeName()) to
    convert the file name into the local encoding.

    \important readAll() at()

    \sa QDataStream, QTextStream
*/

/*!
    \fn Q_LONG QFile::writeBlock(const QByteArray& data)

    \overload
*/


/*!
    Constructs a QFile with no name.
*/
QFile::QFile() : QIODevice(*new QFilePrivate)
{
    setFlags(IO_Direct);
    resetStatus();
}

/*!
    Constructs a QFile with a file name \a name.

    \sa setName()
*/
QFile::QFile(const QString &name) : QIODevice(*new QFilePrivate)
{
    setFlags(IO_Direct);
    resetStatus();
    d->initFileEngine(name);
}

/*!
    Destroys a QFile. Calls close().
*/
QFile::~QFile()
{
    close();
}

/*!
    Returns the name set by setName().

    \sa setName(), QFileInfo::fileName()
*/
QString
QFile::name() const
{
    return d->fileName;
}

/*!
    Sets the name of the file to \a name. The name can have no path, a
    relative path or an absolute absolute path.

    Do not call this function if the file has already been opened.

    If the file name has no path or a relative path, the path used
    will be whatever the application's current directory path is
    \e{at the time of the open()} call.

    Example:
    \code
        QFile file;
        QDir::setCurrent("/tmp");
        file.setName("readme.txt");
        QDir::setCurrent("/home");
        file.open(IO_ReadOnly);      // opens "/home/readme.txt" under Unix
    \endcode

    Note that the directory separator "/" works for all operating
    systems supported by Qt.

    \sa name(), QFileInfo, QDir
*/
void
QFile::setName(const QString &name)
{
    d->fileName = name;
}

/*!
    When you use QFile, QFileInfo, and QDir to access the file system
    with Qt, you can use Unicode file names. On Unix, these file names
    are converted to an 8-bit encoding. If you want to do your own
    file I/O on Unix, you should convert the file name using this
    function. On Windows NT/2000, Unicode file names are supported
    directly in the file system and this function should be avoided.
    On Windows 95, non-Latin1 locales are not supported.

    By default, this function converts \a fileName to the local 8-bit
    encoding determined by the user's locale. This is sufficient for
    file names that the user chooses. File names hard-coded into the
    application should only use 7-bit ASCII filename characters.

    The conversion scheme can be changed using setEncodingFunction().
    This might be useful if you wish to give the user an option to
    store file names in UTF-8, etc., but be aware that such file names
    would probably then be unrecognizable when seen by other programs.

    \sa decodeName()
*/

QByteArray
QFile::encodeName(const QString &fileName)
{
    return (*QFilePrivate::encoder)(fileName);
}

/*!
    \enum QFile::EncoderFn

    This is used by QFile::setEncodingFunction().
*/


/*!
    This does the reverse of QFile::encodeName() using \a localFileName.

    \sa setDecodingFunction()
*/

QString
QFile::decodeName(const QByteArray &localFileName)
{
    return (*QFilePrivate::decoder)(localFileName);
}

/*!
    \nonreentrant

    Sets the function for encoding Unicode file names to \a f. The
    default encodes in the locale-specific 8-bit encoding.

    \sa encodeName()
*/

void
QFile::setEncodingFunction(EncoderFn f)
{
    QFilePrivate::encoder = f;
}

/*!
    \enum QFile::DecoderFn

    This is used by QFile::setDecodingFunction().
*/

/*!
    \nonreentrant

    Sets the function for decoding 8-bit file names to \a f. The
    default uses the locale-specific 8-bit encoding.

    \sa encodeName(), decodeName()
*/

void
QFile::setDecodingFunction(DecoderFn f)
{
    QFilePrivate::decoder = f;
}

/*!
    \overload

    Returns true if this file exists; otherwise returns false.

    \sa name()
*/

bool
QFile::exists() const
{
    return QFileInfo(*this).exists();    
}

/*!
    Returns true if the file given by \a fileName exists; otherwise
    returns false.
*/

bool
QFile::exists(const QString &fileName)
{
    return QFileInfo(fileName).exists();    
}

/*!
    Removes the file specified by the file name currently set. Returns
    true if successful; otherwise returns false.

    The file is closed before it is removed.
*/

bool
QFile::remove()
{
    if (d->fileName.isEmpty()) {
        qWarning("QFile::remove: Empty or null file name");
        return false;
    }
    if(!d->fileEngine)
        return false;
    return d->fileEngine->remove(d->fileName);
}

/*!
    \overload
    Removes the file \a fileName.
  Returns true if successful, otherwise false.
*/

bool
QFile::remove(const QString &fileName)
{
    return QFile(fileName).remove();
}

/*!
    Opens the file specified by the file name currently set, using the
    mode \a m. Returns true if successful, otherwise false.

    \keyword IO_Raw
    \keyword IO_ReadOnly
    \keyword IO_WriteOnly
    \keyword IO_ReadWrite
    \keyword IO_Append
    \keyword IO_Truncate
    \keyword IO_Translate

    The mode parameter \a m must be a combination of the following flags:
    \table
    \header \i Flag \i Meaning
    \row \i IO_Raw
         \i Raw (non-buffered) file access.
    \row \i IO_ReadOnly
         \i Opens the file in read-only mode.
    \row \i IO_WriteOnly
         \i Opens the file in write-only mode. If this flag is used
            with another flag, e.g. \c IO_ReadOnly or \c IO_Raw or \c
            IO_Append, the file is \e not truncated; but if used on
            its own (or with \c IO_Truncate), the file is truncated.
    \row \i IO_ReadWrite
         \i Opens the file in read/write mode, equivalent to \c
            (IO_ReadOnly | IO_WriteOnly).
    \row \i IO_Append
         \i Opens the file in append mode. (You must actually use \c
            (IO_WriteOnly | IO_Append) to make the file writable and
            to go into append mode.) This mode is very useful when you
            want to write something to a log file. The file index is
            set to the end of the file. Note that the result is
            undefined if you position the file index manually using
            at() in append mode.
    \row \i IO_Truncate
         \i Truncates the file.
    \row \i IO_Translate
         \i Enables carriage returns and linefeed translation for text
            files under Windows.
    \endtable

    The raw access mode is best when I/O is block-operated using a 4KB
    block size or greater. Buffered access works better when reading
    small portions of data at a time.

    \warning When working with buffered files, data may not be written
    to the file at once. Call flush() to make sure that the data is
    really written.

    \warning If you have a buffered file opened for both reading and
    writing you must not perform an input operation immediately after
    an output operation or vice versa. You should always call flush()
    or a file positioning operation, e.g. at(), between input and
    output operations, otherwise the buffer may contain garbage.

    If the file does not exist and \c IO_WriteOnly or \c IO_ReadWrite
    is specified, it is created.

    Example:
    \code
        QFile f1("/tmp/data.bin");
        f1.open(IO_Raw | IO_ReadWrite);

        QFile f2("readme.txt");
        f2.open(IO_ReadOnly | IO_Translate);

        QFile f3("audit.log");
        f3.open(IO_WriteOnly | IO_Append);
    \endcode

    \sa name(), close(), isOpen(), flush()
*/

bool
QFile::open(int mode)
{
    if (d->fileEngine && d->fileEngine->isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    if (d->fileName.isEmpty()) {
        qWarning("QFile::open: No file name specified");
        return false;
    }
    if(mode & (Append|WriteOnly)) //append implies write
        mode |= WriteOnly;
    setFlags(IO_Direct);
    resetStatus();
    setMode(mode);
    if (!(isReadable() || isWritable())) {
        qWarning("QFile::open: File access not specified");
        return false;
    }
    d->external_file = false;
    d->reset();
    if(!d->fileEngine)
        d->initFileEngine(d->fileName);
    if(d->fileEngine->open(mode, d->fileName)) {
        setState(IO_Open);
        if(d->fileEngine->isSequential())
            setType(Sequential);
        return true;
    }
    setStatus(errno == EMFILE ? IO_ResourceError : IO_OpenError, errno);
    return false;
}

/*!
    \overload
  Opens a file in the mode \a m using an existing file handle \a f.
  Returns true if successful, otherwise false.

  Example:
  \code
    #include <stdio.h>

    void printError(const char* msg)
    {
        QFile f;
        f.open(IO_WriteOnly, stderr);
        f.writeBlock(msg, qstrlen(msg));        // write to stderr
        f.close();
    }
  \endcode

  When a QFile is opened using this function, close() does not actually
  close the file, only flushes it.

  \warning If \a f is \c stdin, \c stdout, \c stderr, you may not
  be able to seek.  See QIODevice::isSequentialAccess() for more
  information.

  \sa close()
*/

bool
QFile::open(int mode, FILE *fh)
{
    return open(mode, QT_FILENO(fh));
}

/*!
    \overload
  Opens a file in the mode \a m using an existing file descriptor \a f.
  Returns true if successful, otherwise false.

  When a QFile is opened using this function, close() does not actually
  close the file.

  The QFile that is opened using this function, is automatically set to be in
  raw mode; this means that the file input/output functions are slow. If you
  run into performance issues, you should try to use one of the other open
  functions.

  \warning If \a f is one of 0 (stdin), 1 (stdout) or 2 (stderr), you may not
  be able to seek. size() is set to \c LLONG_MAX (in limits.h).

  \sa close()
*/

bool
QFile::open(int mode, int fd)
{
    if (d->fileEngine && d->fileEngine->isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    if(mode & (Append|WriteOnly)) //append implies write
        mode |= WriteOnly;
    setFlags(IO_Direct);
    resetStatus();
    setMode(mode);
    if (!(isReadable() || isWritable())) {
        qWarning("QFile::open: File access not specified");
        return false;
    }
    if(d->openExternalFile(fd)) {
        setState(IO_Open);
        setMode(mode | IO_Raw);
        if(d->fileEngine->isSequential())
            setType(IO_Sequential);
        return true;
    }
    return false;
}

/*!
  Closes an open file.

  The file is not closed if it was opened with an existing file handle.
  If the existing file handle is a \c FILE*, the file is flushed.
  If the existing file handle is an \c int file descriptor, nothing
  is done to the file.

  Some "write-behind" filesystems may report an unspecified error on
  closing the file. These errors only indicate that something may
  have gone wrong since the previous open(). In such a case status()
  reports IO_UnspecifiedError after close(), otherwise IO_Ok.

  \sa open(), flush()
*/

void
QFile::close()
{
    bool closed = true;
    if(d->external_file) {
        flush();
    } else {
        if(d->fileEngine && !d->fileEngine->close()) {
            closed = false;
            setStatus(IO_UnspecifiedError, errno);
        }
    }
    if(closed) {
        setFlags(IO_Direct);
        resetStatus();
    }
}

void
QFile::flush()
{
    if(d->fileEngine && d->fileEngine->isOpen()) {
        d->ungetchBuffer.clear();
        d->fileEngine->flush();
    }
}

/*!
  Returns the file size.
  \sa at()
*/

QFile::Offset
QFile::size() const
{
    if(!d->fileEngine)
        return 0;
    return d->fileEngine->size();
}

/*! \reimp
*/
QFile::Offset
QFile::at() const
{
    if (!isOpen()) 
        return 0;
    return d->fileEngine->at();
}

/*!
    \overload

    Sets the file index to \a pos. Returns true if successful;
    otherwise returns false.

    Example:
    \code
    QFile f("data.bin");
    f.open(IO_ReadOnly);  // index set to 0
    f.at(100);            // set index to 100
    f.at(f.at()+50);            // set index to 150
    f.at(f.size()-80);    // set index to 80 before EOF
    f.close();
    \endcode

    Use \c at() without arguments to retrieve the file offset.

    \warning The result is undefined if the file was open()'ed using
    the \c IO_Append specifier.

    \sa size(), open()
*/

bool
QFile::at(Offset offset)
{
    if (!isOpen()) {
        qWarning("QFile::at: File is not open");
        return false;
    }
    if (isSequentialAccess()) 
        return false;
    if(d->fileEngine->seek(offset)) {
        d->ungetchBuffer.clear();
        return true;
    }
    return false;
}


/*!
    Returns true if the end of file has been reached; otherwise returns false.
    If QFile has not been open()'d, then the behavior is undefined.

    \sa size()
*/
bool
QFile::atEnd() const
{
    if (!isOpen()) {
        qWarning("QFile::atEnd: File is not open");
        return false;
    }
    return d->fileEngine->atEnd();
}

/*!
  \reimp

  \warning We have experienced problems with some C libraries when a buffered
  file is opened for both reading and writing. If a read operation takes place
  immediately after a write operation, the read buffer contains garbage data.
  Worse, the same garbage is written to the file. Calling flush() before
  readBlock() solved this problem.
*/

Q_LONG
QFile::readBlock(char *data, Q_ULONG len)
{
    if (len <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {
        qWarning("QFile::readBlock: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QFile::readBlock: Read operation not permitted");
        return -1;
    }

    Q_ULONG ret = 0;
    if (!d->ungetchBuffer.isEmpty()) {
        uint l = d->ungetchBuffer.size();
        while(ret < l) {
            *data = d->ungetchBuffer.at(l - ret - 1);
            data++;
            ret++;
        }
        d->ungetchBuffer.resize(l - ret);
    }
    if(ret != len) {
        ret += d->fileEngine->readBlock(data, len-ret);
        if (len && ret <= 0) {
            ret = 0;
            setStatus(IO_ReadError, errno);
        }
    }
    return ret;
}

/*! \reimp

  Writes \a len bytes from \a p to the file and returns the number of
  bytes actually written.

  Returns -1 if a serious error occurred.

  \warning When working with buffered files, data may not be written
  to the file at once. Call flush() to make sure the data is really
  written.

  \sa readBlock()
*/

Q_LONG
QFile::writeBlock(const char *data, Q_ULONG len)
{
    if (!len) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {                                // file not open
        qWarning("QFile::writeBlock: File not open");
        return -1;
    }
    if (!isWritable()) {                        // writing not permitted
        qWarning("QFile::writeBlock: Write operation not permitted");
        return -1;
    }

    Q_ULONG ret = d->fileEngine->writeBlock(data, len);
    if (ret != len) // write error
        setStatus(errno == ENOSPC ? IO_ResourceError : IO_WriteError, errno);
    return ret;
}

/*!
    Reads a line of text.

    Reads bytes from the file into the char* \a p, until end-of-line
    or \a maxlen bytes have been read, whichever occurs first. Returns
    the number of bytes read, or -1 if there was an error. Any
    terminating newline is not stripped.

    This function is only efficient for buffered files. Avoid
    readLine() for files that have been opened with the \c IO_Raw
    flag.

    \sa readBlock(), QTextStream::readLine()
*/

Q_LONG
QFile::readLine(char *data, Q_ULONG maxlen)
{
    if (maxlen == 0) // application bug?
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {
        qWarning("QFile::readLine: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QFile::readLine: Read operation not permitted");
        return -1;
    }

    return QIODevice::readLine(data, maxlen);
}

/*!
    \overload

    Reads a line of text.

    Reads bytes from the file into string \a s, until end-of-line or
    \a maxlen bytes have been read, whichever occurs first. Returns
    the number of bytes read, or -1 if there was an error, e.g. end of
    file. Any terminating newline is not stripped.

    This function is only efficient for buffered files. Avoid using
    readLine() for files that have been opened with the \c IO_Raw
    flag.

    Note that the string is read as plain Latin1 bytes, not Unicode.

    \sa readBlock(), QTextStream::readLine()
*/

Q_LONG
QFile::readLine(QString &s, Q_ULONG maxlen)
{
    QByteArray ba;
    ba.resize(maxlen);
    Q_LONG l = readLine(ba.data(),maxlen);
    if (l >= 0)
        s = QString::fromLatin1(ba);
    return l;
}

/*!
    Reads a single byte/character from the file.

    Returns the byte/character read, or -1 if the end of the file has
    been reached.

    \sa putch(), ungetch()
*/

int
QFile::getch()
{
    if (!isOpen()) {                                // file not open
        qWarning("QFile::getch: File not open");
        return EOF;
    }
    if (!isReadable()) {                        // reading not permitted
        qWarning("QFile::getch: Read operation not permitted");
        return EOF;
    }
    char ret;
    if (!d->ungetchBuffer.isEmpty()) {
        int len = d->ungetchBuffer.size();
        ret = d->ungetchBuffer[len - 1];
        d->ungetchBuffer.truncate(len - 1);
    } else if(readBlock(&ret, 1) != 1) {
        ret = EOF;
    }
    return (int)ret;
}

/*!
    Writes the character \a ch to the file.

    Returns \a ch, or -1 if some error occurred.

    \sa getch(), ungetch()
*/

int
QFile::putch(int ch)
{
    if (!isOpen()) {                                // file not open
        qWarning("QFile::putch: File not open");
        return EOF;
    }
    if (!isWritable()) {                        // writing not permitted
        qWarning("QFile::putch: Write operation not permitted");
        return EOF;
    }

    char ret = ch;
    if(writeBlock((char*)&ret, 1) != 1)
        ret = EOF;
    return (int)ret;
}

/*!
    Puts the character \a ch back into the file and decrements the
    index if it is not zero.

    This function is normally called to "undo" a getch() operation.

    Returns \a ch, or -1 if an error occurred.

    \sa getch(), putch()
*/

int
QFile::ungetch(int ch)
{
    if (!isOpen()) {                                // file not open
        qWarning("QFile::ungetch: File not open");
        return EOF;
    }
    if (!isReadable()) {                        // reading not permitted
        qWarning("QFile::ungetch: Read operation not permitted");
        return EOF;
    }
    if (ch == EOF)                                // cannot unget EOF
        return ch;
    d->ungetchBuffer += ch;
    return ch;
}

/*!
  Returns the file handle of the file.

  This is a small positive integer, suitable for use with C library
  functions such as fdopen() and fcntl(). On systems that use file
  descriptors for sockets (ie. Unix systems, but not Windows) the handle
  can be used with QSocketNotifier as well.

  If the file is not open or there is an error, handle() returns -1.

  \sa QSocketNotifier
*/

int
QFile::handle() const
{
    if (!isOpen())
        return -1;
    return d->fileEngine->handle();
}

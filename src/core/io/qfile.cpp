/****************************************************************************
**
** Implementation of QFile class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"

// POSIX Large File Support redefines open -> open64
#if defined(open)
# undef open
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

#include "qfile.h"
#include "qfiledefs_p.h"
#include "qdatastream.h"
#include <limits.h>

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
        for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
            stream << *it << "\n";
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

QFile::QFile()
: d(0)
{
    init();
}

/*!
    Constructs a QFile with a file name \a name.

    \sa setName()
*/

QFile::QFile(const QString &name)
    : d(0)
{
    init();
    d->fn = name;
}


/*!
    Destroys a QFile. Calls close().
*/

QFile::~QFile()
{
    close();
    delete d;
}


/*!
  \internal
  Initialize internal data.
*/

void QFile::init()
{
    delete d;
    d = new QFilePrivate;
    setFlags(IO_Direct);
    setStatus(IO_Ok);
    d->fh = 0;
    d->fd = 0;
    d->length = 0;
    ioIndex = 0;
    d->ext_f = false; // not an external file handle
}


/*!
    Returns the name set by setName().

    \sa setName(), QFileInfo::fileName()
*/
QString QFile::name() const
{
    return d->fn;
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

void QFile::setName(const QString &name)
{
    if (isOpen()) {
        qWarning("QFile::setName: File is open");
        close();
    }
    d->fn = name;
}

/*!
    \overload

    Returns true if this file exists; otherwise returns false.

    \sa name()
*/

bool QFile::exists() const
{
    return QFileInfoPrivate::access(d->fn, F_OK);
}

/*!
    Returns true if the file given by \a fileName exists; otherwise
    returns false.
*/

bool QFile::exists(const QString &fileName)
{
    return QFileInfoPrivate::access(fileName, F_OK);
}


/*!
    Removes the file specified by the file name currently set. Returns
    true if successful; otherwise returns false.

    The file is closed before it is removed.
*/

bool QFile::remove()
{
    close();
    return remove(d->fn);
}

#if defined(Q_OS_MAC) || defined(Q_OS_MSDOS) || defined(Q_OS_WIN32) || defined(Q_OS_OS2)
# define HAS_TEXT_FILEMODE                        // has translate/text filemode
#endif
#if defined(O_NONBLOCK)
# define HAS_ASYNC_FILEMODE
# define OPEN_ASYNC O_NONBLOCK
#elif defined(O_NDELAY)
# define HAS_ASYNC_FILEMODE
# define OPEN_ASYNC O_NDELAY
#endif

/*!
    Flushes the file buffer to the disk.

    close() also flushes the file buffer.
*/

void QFile::flush()
{
    if (isOpen() && d->fh)  // can only flush open/buffered files
        fflush(d->fh);
}

/*! \reimp
    \fn QIODevice::Offset QFile::at() const
*/

/*!
    Returns true if the end of file has been reached; otherwise returns false.
    If QFile has not been open()'d, then the behavior is undefined.

    \sa size()
*/

bool QFile::atEnd() const
{
    if (!isOpen()) {
        qWarning("QFile::atEnd: File is not open");
        return false;
    }
    if (isDirectAccess() && !isTranslated()) {
        if (at() < d->length)
            return false;
    }
    return QIODevice::atEnd();
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

Q_LONG QFile::readLine(char *p, Q_ULONG maxlen)
{
    if (maxlen == 0)                                // application bug?
        return 0;
    Q_CHECK_PTR(p);
    if (!isOpen()) {                                // file not open
        qWarning("QFile::readLine: File not open");
        return -1;
    }
    if (!isReadable()) {                        // reading not permitted
        qWarning("QFile::readLine: Read operation not permitted");
        return -1;
    }
    Q_LONG nread;                                // number of bytes read
    if (isRaw()) {                                // raw file
        nread = QIODevice::readLine(p, maxlen);
    } else {                                        // buffered file
        p = fgets(p, maxlen, d->fh);
        if (p) {
            nread = qstrlen(p);
            if (!isSequentialAccess())
                ioIndex += nread;
        } else {
            nread = -1;
            setStatus(IO_ReadError, QFILEERR_READ);
        }
    }
    return nread;
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

Q_LONG QFile::readLine(QString& s, Q_ULONG maxlen)
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

int QFile::getch()
{
    if (!isOpen()) {                                // file not open
        qWarning("QFile::getch: File not open");
        return EOF;
    }
    if (!isReadable()) {                        // reading not permitted
        qWarning("QFile::getch: Read operation not permitted");
        return EOF;
    }

    int ch;

    if (!d->ungetchBuffer.isEmpty()) {
        int len = d->ungetchBuffer.size();
        ch = d->ungetchBuffer[len - 1];
        d->ungetchBuffer.truncate(len - 1);
        return ch;
    }

    if (isRaw()) {                                // raw file (inefficient)
        char buf[1];
        ch = readBlock(buf, 1) == 1 ? buf[0] : EOF;
    } else {                                        // buffered file
        if ((ch = getc(d->fh)) != EOF) {
            if (!isSequentialAccess())
                ioIndex++;
        } else {
            setStatus(IO_ReadError, QFILEERR_READ);
        }
    }
    return ch;
}

/*!
    Writes the character \a ch to the file.

    Returns \a ch, or -1 if some error occurred.

    \sa getch(), ungetch()
*/

int QFile::putch(int ch)
{
    if (!isOpen()) {                                // file not open
        qWarning("QFile::putch: File not open");
        return EOF;
    }
    if (!isWritable()) {                        // writing not permitted
        qWarning("QFile::putch: Write operation not permitted");
        return EOF;
    }
    if (isRaw()) {                                // raw file (inefficient)
        char buf[1];
        buf[0] = ch;
        ch = writeBlock(buf, 1) == 1 ? ch : EOF;
    } else {                                        // buffered file
        if ((ch = putc(ch, d->fh)) != EOF) {
            if (!isSequentialAccess())
                ioIndex++;
            if (ioIndex > d->length)                // update file length
                d->length = ioIndex;
        } else {
            setStatus(IO_WriteError, QFILEERR_WRITE);
        }
    }
    return ch;
}

/*!
    Puts the character \a ch back into the file and decrements the
    index if it is not zero.

    This function is normally called to "undo" a getch() operation.

    Returns \a ch, or -1 if an error occurred.

    \sa getch(), putch()
*/

int QFile::ungetch(int ch)
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

    if (isSequentialAccess() && !d->fh) {
        // pipe or similar: we cannot ungetch, so do it manually
        d->ungetchBuffer += ch;
        return ch;
    }

    if (isRaw()) {                                // raw file (very inefficient)
        char buf[1];
        at(ioIndex-1);
        buf[0] = ch;
        if (writeBlock(buf, 1) == 1)
            at(ioIndex - 1);
        else
            ch = EOF;
    } else {                                        // buffered file
        if ((ch = ungetc(ch, d->fh)) != EOF) {
            if (!isSequentialAccess())
                ioIndex--;
        } else {
            setStatus(IO_ReadError, QFILEERR_READ);
        }
    }
    return ch;
}


static QByteArray locale_encoder(const QString &fileName)
{
    return fileName.toLocal8Bit();
}


static QFile::EncoderFn encoder = locale_encoder;

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

QByteArray QFile::encodeName(const QString &fileName)
{
    return (*encoder)(fileName);
}

/*!
    \enum QFile::EncoderFn

    This is used by QFile::setEncodingFunction().
*/

/*!
    \nonreentrant

    Sets the function for encoding Unicode file names to \a f. The
    default encodes in the locale-specific 8-bit encoding.

    \sa encodeName()
*/
void QFile::setEncodingFunction(EncoderFn f)
{
    encoder = f;
}

static
QString locale_decoder(const QByteArray &localFileName)
{
    return QString::fromLocal8Bit(localFileName);
}

static QFile::DecoderFn decoder = locale_decoder;

/*!
    This does the reverse of QFile::encodeName() using \a localFileName.

    \sa setDecodingFunction()
*/
QString QFile::decodeName(const QByteArray &localFileName)
{
    return (*decoder)(localFileName);
}

/*!
    \overload
*/
QString QFile::decodeName(const char *localFileName)
{
    return decodeName(QByteArray(localFileName));
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

void QFile::setDecodingFunction(DecoderFn f)
{
    decoder = f;
}

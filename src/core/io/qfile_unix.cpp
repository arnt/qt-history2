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
static inline int qt_open(const char *pathname, int flags, mode_t mode)
{ return ::open(pathname, flags, mode); }
#if defined(open)
# undef open
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

#include "qfile.h"
#include "qfiledefs_p.h"
#include <errno.h>
#include <limits.h>

#ifndef LLONG_MAX
#define LLONG_MAX Q_INT64_C(9223372036854775807)
#endif

#define d d_func()
#define q q_func()

extern const char *qt_fileerr_read;

bool QFileInfoPrivate::access(const QString &fn, int t)
{
    if (fn.isEmpty())
        return false;
    return ::access(QFile::encodeName(fn), t) == 0;
}

/*!
    \overload
    Removes the file \a fileName.
  Returns true if successful, otherwise false.
*/

bool QFile::remove(const QString &fileName)
{
    if (fileName.isEmpty()) {
        qWarning("QFile::remove: Empty or null file name");
        return false;
    }
    return unlink(QFile::encodeName(fileName)) == 0;
}

#if defined(O_NONBLOCK)
# define HAS_ASYNC_FILEMODE
# define OPEN_ASYNC O_NONBLOCK
#elif defined(O_NDELAY)
# define HAS_ASYNC_FILEMODE
# define OPEN_ASYNC O_NDELAY
#endif

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

bool QFile::open(int m)
{
    if (isOpen()) {                                // file already open
        qWarning("QFile::open: File already open");
        return false;
    }
    if (d->fn.isEmpty()) {                        // no file name defined
        qWarning("QFile::open: No file name specified");
        return false;
    }
    d->init();                                    // reset params
    setMode(m);
    if (!(isReadable() || isWritable())) {
        qWarning("QFile::open: File access not specified");
        return false;
    }
    bool ok = true;
    struct stat st;
    if (isRaw()) {
        int oflags = O_RDONLY;
        if (isReadable() && isWritable())
            oflags = O_RDWR;
        else if (isWritable())
            oflags = O_WRONLY;
        if (flags() & IO_Append) {                // append to end of file?
            if (flags() & IO_Truncate)
                oflags |= (O_CREAT | O_TRUNC);
            else
                oflags |= (O_APPEND | O_CREAT);
            setFlags(flags() | IO_WriteOnly); // append implies write
        } else if (isWritable()) {                // create/trunc if writable
            if (flags() & IO_Truncate)
                oflags |= (O_CREAT | O_TRUNC);
            else
                oflags |= O_CREAT;
        }
#if defined(HAS_TEXT_FILEMODE)
        if (isTranslated())
            oflags |= OPEN_TEXT;
        else
            oflags |= OPEN_BINARY;
#endif
#if defined(HAS_ASYNC_FILEMODE)
        if (isAsynchronous())
            oflags |= OPEN_ASYNC;
#endif
        d->fd = qt_open(QFile::encodeName(d->fn), oflags, 0666);

        if (d->fd != -1) {                        // open successful
            ::fstat(d->fd, &st);                  // get the stat for later usage
        } else {
            ok = false;
        }
    } else {                                        // buffered file I/O
        const char *perm = 0;
        char perm2[4];
        bool try_create = false;
        if (flags() & IO_Append) {                // append to end of file?
            setFlags(flags() | IO_WriteOnly); // append implies write
            perm = isReadable() ? "a+" : "a";
        } else {
            if (isReadWrite()) {
                if (flags() & IO_Truncate) {
                    perm = "w+";
                } else {
                    perm = "r+";
                    try_create = true;                // try to create if not exists
                }
            } else if (isReadable()) {
                perm = "r";
            } else if (isWritable()) {
                perm = "w";
            }
        }
        qstrcpy(perm2, perm);
#if defined(HAS_TEXT_FILEMODE)
        if (isTranslated())
            strcat(perm2, "t");
        else
            strcat(perm2, "b");
#endif
        for (;;) { // At most twice
            d->fh = fopen(QFile::encodeName(d->fn), perm2);

            if (!d->fh && try_create) {
                perm2[0] = 'w';                        // try "w+" instead of "r+"
                try_create = false;
            } else {
                break;
            }
        }
        if (d->fh) {
            ::fstat(fileno(d->fh), &st);                // get the stat for later usage
        } else {
            ok = false;
        }
    }
    if (ok) {
        setState(IO_Open);
        // on successful open the file stat was got; now test what type
        // of file we have
        if ((st.st_mode & S_IFMT) != S_IFREG) {
            // non-seekable
            setType(IO_Sequential);
            d->length = LLONG_MAX;
            ioIndex = 0;
        } else {
            d->length = st.st_size;
            ioIndex = (flags() & IO_Append) == 0 ? Offset(0) : d->length;
            if (!(flags()&IO_Truncate) && d->length == 0 && isReadable()) {
                // try if you can read from it (if you can, it's a sequential
                // device; e.g. a file in the /proc filesystem)
                int c = getch();
                if (c != -1) {
                    ungetch(c);
                    setType(IO_Sequential);
                    d->length = LLONG_MAX;
                    ioIndex = 0;
                }
                resetStatus();
            }
        }
    } else {
        d->init();
        setStatus(errno == EMFILE ? IO_ResourceError : IO_OpenError, errno);
    }
    return ok;
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

bool QFile::open(int m, FILE *f)
{
    if (isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    d->init();
    setMode(m &~IO_Raw);
    setState(IO_Open);
    d->fh = f;
    d->ext_f = true;
    struct stat st;
    ::fstat(fileno(d->fh), &st);
#if defined(QT_LARGEFILE_SUPPORT)
    ioIndex = ftello(d->fh);
#else
    ioIndex = ftell(d->fh);
#endif
    if ((st.st_mode & S_IFMT) != S_IFREG || f == stdin) { //stdin is non seekable
        // non-seekable
        setType(IO_Sequential);
        d->length = LLONG_MAX;
        ioIndex = 0;
    } else {
        d->length = st.st_size;
        if (!(flags()&IO_Truncate) && d->length == 0 && isReadable()) {
            // try if you can read from it (if you can, it's a sequential
            // device; e.g. a file in the /proc filesystem)
            int c = getch();
            if (c != -1) {
                ungetch(c);
                setType(IO_Sequential);
                d->length = LLONG_MAX;
                ioIndex = 0;
            }
            resetStatus();
        }
    }
    return true;
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


bool QFile::open(int m, int f)
{
    if (isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    d->init();
    setMode(m |IO_Raw);
    setState(IO_Open);
    d->fd = f;
    d->ext_f = true;
    struct stat st;
    ::fstat(d->fd, &st);
    ioIndex = ::lseek(d->fd, 0, SEEK_CUR);
    if ((st.st_mode & S_IFMT) != S_IFREG || f == 0) { // stdin is not seekable...
        // non-seekable
        setType(IO_Sequential);
        d->length = LLONG_MAX;
        ioIndex = 0;
    } else {
        d->length = st.st_size;
        if (d->length == 0 && isReadable()) {
            // try if you can read from it (if you can, it's a sequential
            // device; e.g. a file in the /proc filesystem)
            int c = getch();
            if (c != -1) {
                ungetch(c);
                setType(IO_Sequential);
                d->length = LLONG_MAX;
                ioIndex = 0;
            }
            resetStatus();
        }
    }
    return true;
}

/*!
  Returns the file size.
  \sa at()
*/

QIODevice::Offset QFile::size() const
{
    struct stat st;
    int ret = 0;
    if (isOpen()) {
        ret = ::fstat(d->fh ? fileno(d->fh) : d->fd, &st);
    } else {
        ret = ::stat(QFile::encodeName(d->fn), &st);
    }
    if (ret == -1)
        return 0;
    return st.st_size;
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

bool QFile::at(Offset pos)
{
    if (!isOpen()) {
        qWarning("QFile::at: File is not open");
        return false;
    }
    if (isSequentialAccess())
        return false;
    bool ok;
    if (isRaw()) {
        off_t l = ::lseek(d->fd, pos, SEEK_SET);
        ok = (l != -1);
        pos = l;
    } else {                                        // buffered file
#if defined(QT_LARGEFILE_SUPPORT)
        ok = (::fseeko(d->fh, pos, SEEK_SET) == 0);
#else
        ok = (::fseek(d->fh, pos, SEEK_SET) == 0);
#endif
    }
    if (ok)
        ioIndex = (Offset)pos;
    else
        qWarning("QFile::at: Cannot set file position %lld", pos);
    return ok;
}

/*!
  \reimp

  \warning We have experienced problems with some C libraries when a buffered
  file is opened for both reading and writing. If a read operation takes place
  immediately after a write operation, the read buffer contains garbage data.
  Worse, the same garbage is written to the file. Calling flush() before
  readBlock() solved this problem.
*/

Q_LONG QFile::readBlock(char *p, Q_ULONG len)
{
    if (!len) // nothing to do
        return 0;

    if (!p)
        qWarning("QFile::readBlock: Null pointer error");
    if (!isOpen()) {
        qWarning("QFile::readBlock: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QFile::readBlock: Read operation not permitted");
        return -1;
    }
    Q_ULONG nread = 0;                                        // number of bytes read
    if (!d->ungetchBuffer.isEmpty()) {
        // need to add these to the returned string.
        uint l = d->ungetchBuffer.size();
        while(nread < l) {
            *p = d->ungetchBuffer.at(l - nread - 1);
            p++;
            nread++;
        }
        d->ungetchBuffer.resize(l - nread);
    }

    if (nread < len) {
        if (isRaw()) {                                  // raw file
            nread += ::read(d->fd, p, len-nread);
            if (len && nread <= 0) {
                nread = 0;
                setStatus(IO_ReadError, errno);
            }
        } else {                                        // buffered file
            nread += fread(p, 1, len - nread, d->fh);
            if ((uint)nread != len) {
                if (ferror(d->fh) || nread == 0)
                    setStatus(IO_ReadError, QFILEERR_READ);
            }
        }
    }
    if (!isSequentialAccess())
        ioIndex += nread;
    return nread;
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

Q_LONG QFile::writeBlock(const char *p, Q_ULONG len)
{
    if (!len) // nothing to do
        return 0;

    if (p == 0 && len != 0)
        qWarning("QFile::writeBlock: Null pointer error");
    if (!isOpen()) {                                // file not open
        qWarning("QFile::writeBlock: File not open");
        return -1;
    }
    if (!isWritable()) {                        // writing not permitted
        qWarning("QFile::writeBlock: Write operation not permitted");
        return -1;
    }
    Q_ULONG nwritten;                                // number of bytes written
    if (isRaw())                                // raw file
        nwritten = ::write(d->fd, (void *)p, len);
    else                                        // buffered file
        nwritten = fwrite(p, 1, len, d->fh);
    if (nwritten != len) {                // write error
        setStatus(errno == ENOSPC ? IO_ResourceError : IO_WriteError, errno);
        if (!isSequentialAccess()) {
            if (isRaw())                        // recalc file position
                ioIndex = ::lseek(d->fd, 0, SEEK_CUR);
            else {
#if defined(QT_LARGEFILE_SUPPORT)
                ioIndex = ::fseeko(d->fh, 0, SEEK_CUR);
#else
                ioIndex = ::fseek(d->fh, 0, SEEK_CUR);
#endif
            }
        }
    } else {
        if (!isSequentialAccess())
            ioIndex += nwritten;
    }
    if (ioIndex > d->length)                        // update file length
        d->length = ioIndex;
    return nwritten;
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

int QFile::handle() const
{
    if (!isOpen())
        return -1;
    else if (d->fh)
        return fileno(d->fh);
    else
        return d->fd;
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

void QFile::close()
{
    bool ok = false;
    if (isOpen()) {                                // file is not open
        if (d->fh) {                               // buffered file
            if (d->ext_f)
                ok = fflush(d->fh) != -1;          // flush instead of closing
            else
                ok = fclose(d->fh) != -1;
        } else {                                   // raw file
            if (d->ext_f)
                ok = true;                         // cannot close
            else
                ok = ::close(d->fd) != -1;
        }
        d->init();                                 // restore internal state
    }
    if (!ok)
        setStatus(IO_UnspecifiedError, errno);
}

/****************************************************************************
**
** Implementation of QFileInfo class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"

#include "qfile.h"
#include "qfiledefs_p.h"
#include <limits.h>

#ifndef LLONG_MAX
#define LLONG_MAX Q_INT64_C(9223372036854775807)
#endif

extern const char *qt_fileerr_read;

#define d d_func()
#define q q_func()

QByteArray qt_win95Name(const QString s)
{
    QString ss(s);
    if (s[0] == '/' && s[1] == '/') {
        // Win95 cannot handle slash-slash needs slosh-slosh.
        ss[0] = '\\';
        ss[1] = '\\';
        int n = ss.indexOf('/');
        if (n >= 0)
            ss[n] = '\\';
        return ss.toLocal8Bit();
    } else if (s.length() > 3 && s[2] == '/' && s[3] == '/') {
        ss[2] = '\\';
        ss.remove(3, 1);
        int n = ss.indexOf('/');
        if (n >= 0)
            ss[n] = '\\';
    }
    return ss.toLocal8Bit();
}

bool QFileInfoPrivate::access(const QString &fn, int t)
{
    if (fn.isEmpty())
        return false;
    QT_WA({
        return ::_waccess((TCHAR*)fn.utf16(), t) == 0;
    } , {
        return QT_ACCESS(qt_win95Name(fn), t) == 0;
    });
}

bool isValidFile(const QString& fileName)
{
    // Only : needs to be checked for, other invalid characters
    // are currently checked by fopen()
    int findColon = fileName.lastIndexOf(':');
    if (findColon == -1)
        return true;
    else if (findColon != 1)
        return false;
    else
        return fileName[0].isLetter();
}

bool QFile::remove(const QString &fileName)
{
    if (fileName.isEmpty()) {
        qWarning("QFile::remove: Empty or null file name");
        return false;
    }
    QT_WA({
        return ::_wremove((TCHAR*)fileName.utf16()) == 0;
    } , {
        return ::remove(qt_win95Name(fileName)) == 0;
    });
}

#define HAS_TEXT_FILEMODE

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
    if (!isValidFile(d->fn)) {
        qWarning("QFile::open: Invalid filename specified");
        return false;
    }

    bool ok = true;
    if (isRaw()) {                                // raw file I/O
        int oflags = QT_OPEN_RDONLY;
        if (isReadable() && isWritable())
            oflags = QT_OPEN_RDWR;
        else if (isWritable())
            oflags = QT_OPEN_WRONLY;
        if (flags() & IO_Append) {                // append to end of file?
            if (flags() & IO_Truncate)
                oflags |= (QT_OPEN_CREAT | QT_OPEN_TRUNC);
            else
                oflags |= (QT_OPEN_APPEND | QT_OPEN_CREAT);
            setFlags(flags() | IO_WriteOnly); // append implies write
        } else if (isWritable()) {                // create/trunc if writable
            if (flags() & IO_Truncate)
                oflags |= (QT_OPEN_CREAT | QT_OPEN_TRUNC);
            else
                oflags |= QT_OPEN_CREAT;
        }
#if defined(HAS_TEXT_FILEMODE)
        if (isTranslated())
            oflags |= QT_OPEN_TEXT;
        else
            oflags |= QT_OPEN_BINARY;
#endif
#if defined(HAS_ASYNC_FILEMODE)
        if (isAsynchronous())
            oflags |= QT_OPEN_ASYNC;
#endif
        QT_WA({
            d->fd = ::_wopen((TCHAR*)d->fn.utf16(), oflags, 0666);
        } , {
            d->fd = QT_OPEN(qt_win95Name(d->fn), oflags, 0666);
        });

        if (d->fd != -1) {                        // open successful
            QT_STATBUF st;
            QT_FSTAT(d->fd, &st);
            if ((st.st_mode& QT_STAT_MASK) == QT_STAT_DIR) {
                ok = false;
            } else {
                d->length = (int)st.st_size;
                ioIndex = (flags() & IO_Append) == 0 ? 0 : d->length;
            }
        } else {
            ok = false;
        }
    } else {                                        // buffered file I/O
        QByteArray perm;
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
            QT_WA({
                TCHAR tperm2[4];
                tperm2[0] = perm2[0];
                tperm2[1] = perm2[1];
                tperm2[2] = perm2[2];
                tperm2[3] = perm2[3];
                d->fh = ::_wfopen((TCHAR*)d->fn.utf16(), tperm2);
            } , {
                d->fh = fopen(qt_win95Name(d->fn), perm2);
            });
            if (errno == EACCES)
                break;
            if (!d->fh && try_create) {
                perm2[0] = 'w';                        // try "w+" instead of "r+"
                try_create = false;
            } else {
                break;
            }
        }
        if (d->fh) {
            QT_STATBUF st;
            QT_FSTAT(QT_FILENO(d->fh), &st);
            if ((st.st_mode& QT_STAT_MASK) == QT_STAT_DIR) {
                ok = false;
            } else {
                d->length = (int)st.st_size;
                ioIndex  = (flags() & IO_Append) == 0 ? 0 : d->length;
            }
        } else {
            ok = false;
        }
    }
    if (ok) {
        setState(IO_Open);
    } else {
        d->init();
        setStatus(errno == EMFILE ? ResourceError : OpenError, errno);
    }
    return ok;
}



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
    QT_STATBUF st;
    QT_FSTAT(QT_FILENO(d->fh), &st);
    ioIndex = (int)ftell(d->fh);
    if ((st.st_mode & QT_STAT_MASK) != QT_STAT_REG) {
        // non-seekable
        setType(IO_Sequential);
        d->length = LLONG_MAX;
    } else {
        d->length = (int)st.st_size;
    }
    return true;
}


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
    QT_STATBUF st;
    QT_FSTAT(d->fd, &st);
    ioIndex  = (int)QT_LSEEK(d->fd, 0, SEEK_CUR);
    if ((st.st_mode & QT_STAT_MASK) != QT_STAT_REG) {
        // non-seekable
        setType(IO_Sequential);
        d->length = LLONG_MAX;
    } else {
        d->length = (int)st.st_size;
    }
    return true;
}

QIODevice::Offset QFile::size() const
{
    QT_STATBUF st;
    int ret = 0;
    if (isOpen()) {
        ret = QT_FSTAT(d->fh ? QT_FILENO(d->fh) : d->fd, &st);
    } else {
        QT_WA({
            ret = QT_TSTAT((TCHAR*)d->fn.utf16(), (QT_STATBUF4TSTAT*)&st);
        } , {
            ret = QT_STAT(qt_win95Name(d->fn), &st);
        });
    }
    if (ret == -1)
        return 0;
    return st.st_size;
}

bool QFile::at(Offset pos)
{
    if (!isOpen()) {
        qWarning("QFile::at: File is not open");
        return false;
    }
    bool okay;
    if (isRaw()) {                                // raw file
        pos = (int)QT_LSEEK(d->fd, pos, SEEK_SET);
        okay = pos != (Q_ULONG)-1;
    } else {                                        // buffered file
        okay = fseek(d->fh, pos, SEEK_SET) == 0;
    }
    if (okay)
        ioIndex = pos;
    else
        qWarning("QFile::at: Cannot set file position %d", pos);
    return okay;
}

Q_LONG QFile::readBlock(char *p, Q_ULONG len)
{
    if (!len) // nothing to do
        return 0;

    if (!p)
        qWarning("QFile::readBlock: Null pointer error");
    if (!isOpen()) {                                // file not open
        qWarning("QFile::readBlock: File not open");
        return -1;
    }
    if (!isReadable()) {                        // reading not permitted
        qWarning("QFile::readBlock: Read operation not permitted");
        return -1;
    }
    Q_ULONG nread = 0;                                        // number of bytes read
    if (!d->ungetchBuffer.isEmpty()) {
        // need to add these to the returned string.
        Q_ULONG l = d->ungetchBuffer.size();
        while (nread < l) {
            *p = d->ungetchBuffer[int(l - nread - 1)];
            p++;
            nread++;
        }
        d->ungetchBuffer.truncate(l - nread);
    }

    if(nread < len) {
        if (isRaw()) {                                // raw file
            nread += QT_READ(d->fd, p, len - nread);
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
    ioIndex += nread;
    return nread;
}

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
        nwritten = QT_WRITE(d->fd, p, len);
    else                                        // buffered file
        nwritten = fwrite(p, 1, len, d->fh);
    if (nwritten != len) {                // write error
        setStatus(errno == ENOSPC ? IO_ResourceError : IO_WriteError, errno);
        if (isRaw())                                // recalc file position
            ioIndex = (int)QT_LSEEK(d->fd, 0, SEEK_CUR);
        else
            ioIndex = fseek(d->fh, 0, SEEK_CUR);
    } else {
        ioIndex += nwritten;
    }
    if (ioIndex > d->length)                        // update file length
        d->length = ioIndex;
    return nwritten;
}

int QFile::handle() const
{
    if (!isOpen())
        return -1;
    else if (d->fh)
        return QT_FILENO(d->fh);
    else
        return d->fd;
}

void QFile::close()
{
    bool ok = false;
    if (isOpen()) {
        if (d->fh) {                            // buffered file
            if (d->ext_f)
                ok = fflush(d->fh) != -1;       // flush instead of closing
            else
                ok = fclose(d->fh) != -1;
        } else {                                // raw file
            if (d->ext_f)
                ok = true;                      // cannot close
            else
                ok = QT_CLOSE(d->fd) != -1;
        }
        d->init();                              // restore internal state
    }
    if (!ok)
        setStatus(IO_UnspecifiedError, errno);
}

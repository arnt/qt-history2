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

extern const char* qt_fileerr_read;

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

bool QFileInfoPrivate::access(const QString& fn, int t)
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
    if (fn.isEmpty()) {                        // no file name defined
        qWarning("QFile::open: No file name specified");
        return false;
    }
    init();                                        // reset params
    setMode(m);
    if (!(isReadable() || isWritable())) {
        qWarning("QFile::open: File access not specified");
        return false;
    }
    if (!isValidFile(fn)) {
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
            fd = ::_wopen((TCHAR*)fn.utf16(), oflags, 0666);
        } , {
            fd = QT_OPEN(qt_win95Name(fn), oflags, 0666);
        });

        if (fd != -1) {                        // open successful
            QT_STATBUF st;
            QT_FSTAT(fd, &st);
            if ((st.st_mode& QT_STAT_MASK) == QT_STAT_DIR) {
                ok = false;
            } else {
                length = (int)st.st_size;
                ioIndex  = (flags() & IO_Append) == 0 ? 0 : length;
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
                fh = ::_wfopen((TCHAR*)fn.utf16(), tperm2);
            } , {
                fh = fopen(qt_win95Name(fn),
                            perm2);
            });
            if (errno == EACCES)
                break;
            if (!fh && try_create) {
                perm2[0] = 'w';                        // try "w+" instead of "r+"
                try_create = false;
            } else {
                break;
            }
        }
        if (fh) {
            QT_STATBUF st;
            QT_FSTAT(QT_FILENO(fh), &st);
            if ((st.st_mode& QT_STAT_MASK) == QT_STAT_DIR) {
                ok = false;
            } else {
                length = (int)st.st_size;
                ioIndex  = (flags() & IO_Append) == 0 ? 0 : length;
            }
        } else {
            ok = false;
        }
    }
    if (ok) {
        setState(IO_Open);
    } else {
        init();
        if (errno == EMFILE)                        // no more file handles/descrs
            setStatus(IO_ResourceError);
        else
            setStatus(IO_OpenError);
        setErrorStringErrno(errno);
    }
    return ok;
}



bool QFile::open(int m, FILE *f)
{
    if (isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    init();
    setMode(m &~IO_Raw);
    setState(IO_Open);
    fh = f;
    ext_f = true;
    QT_STATBUF st;
    QT_FSTAT(QT_FILENO(fh), &st);
    ioIndex = (int)ftell(fh);
    if ((st.st_mode & QT_STAT_MASK) != QT_STAT_REG) {
        // non-seekable
        setType(IO_Sequential);
        length = LLONG_MAX;
    } else {
        length = (int)st.st_size;
    }
    return true;
}


bool QFile::open(int m, int f)
{
    if (isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    init();
    setMode(m |IO_Raw);
    setState(IO_Open);
    fd = f;
    ext_f = true;
    QT_STATBUF st;
    QT_FSTAT(fd, &st);
    ioIndex  = (int)QT_LSEEK(fd, 0, SEEK_CUR);
    if ((st.st_mode & QT_STAT_MASK) != QT_STAT_REG) {
        // non-seekable
        setType(IO_Sequential);
        length = LLONG_MAX;
    } else {
        length = (int)st.st_size;
    }
    return true;
}

QIODevice::Offset QFile::size() const
{
    QT_STATBUF st;
    int ret = 0;
    if (isOpen()) {
        ret = QT_FSTAT(fh ? QT_FILENO(fh) : fd, &st);
    } else {
        QT_WA({
            ret = QT_TSTAT((TCHAR*)fn.utf16(), (QT_STATBUF4TSTAT*)&st);
        } , {
            ret = QT_STAT(qt_win95Name(fn), &st);
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
        pos = (int)QT_LSEEK(fd, pos, SEEK_SET);
        okay = pos != (Q_ULONG)-1;
    } else {                                        // buffered file
        okay = fseek(fh, pos, SEEK_SET) == 0;
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
    if (!ungetchBuffer.isEmpty()) {
        // need to add these to the returned string.
        Q_ULONG l = ungetchBuffer.length();
        while(nread < l) {
            *p = ungetchBuffer[int(l - nread - 1)];
            p++;
            nread++;
        }
        ungetchBuffer.truncate(l - nread);
    }

    if(nread < len) {
        if (isRaw()) {                                // raw file
            nread += QT_READ(fd, p, len - nread);
            if (len && nread <= 0) {
                nread = 0;
                setStatus(IO_ReadError);
                setErrorStringErrno(errno);
            }
        } else {                                        // buffered file
            nread += fread(p, 1, len - nread, fh);
            if ((uint)nread != len) {
                if (ferror(fh) || nread==0) {
                    setStatus(IO_ReadError);
                    setErrorString(qt_fileerr_read);
                }
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
        nwritten = QT_WRITE(fd, p, len);
    else                                        // buffered file
        nwritten = fwrite(p, 1, len, fh);
    if (nwritten != len) {                // write error
        if (errno == ENOSPC)                        // disk is full
            setStatus(IO_ResourceError);
        else
            setStatus(IO_WriteError);
        setErrorStringErrno(errno);
        if (isRaw())                                // recalc file position
            ioIndex = (int)QT_LSEEK(fd, 0, SEEK_CUR);
        else
            ioIndex = fseek(fh, 0, SEEK_CUR);
    } else {
        ioIndex += nwritten;
    }
    if (ioIndex > length)                        // update file length
        length = ioIndex;
    return nwritten;
}

int QFile::handle() const
{
    if (!isOpen())
        return -1;
    else if (fh)
        return QT_FILENO(fh);
    else
        return fd;
}

void QFile::close()
{
    bool ok = false;
    if (isOpen()) {                                // file is not open
        if (fh) {                                // buffered file
            if (ext_f)
                ok = fflush(fh) != -1;        // flush instead of closing
            else
                ok = fclose(fh) != -1;
        } else {                                // raw file
            if (ext_f)
                ok = true;                        // cannot close
            else
                ok = QT_CLOSE(fd) != -1;
        }
        init();                                        // restore internal state
    }
    if (!ok) {
        setStatus (IO_UnspecifiedError);
        setErrorStringErrno(errno);
    }

    return;
}

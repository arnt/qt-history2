/****************************************************************************
**
** Implementation of QFSFileEngine class
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
#include <qplatformdefs.h>

extern QString qt_fixToQtSlashes(const QString &path);

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

QFileEngine::QFileEngine(QFileEnginePrivate &dd)  : d_ptr(&dd)
{
    d->q_ptr = this;
}

QFileEngine::~QFileEngine()
{
    delete d_ptr;
    d_ptr = 0;
}

//**************** QFSFileEnginePrivate
QFSFileEnginePrivate::QFSFileEnginePrivate() : QFileEnginePrivate()
{
    hasCachedChar = 0;
    fd = -1;
    init();
}

//**************** QFSFileEngine
QFSFileEngine::QFSFileEngine(const QString &file) : QFileEngine(*new QFSFileEnginePrivate)
{
    d->file = qt_fixToQtSlashes(file);    
}

QFSFileEngine::QFSFileEngine() : QFileEngine(*new QFSFileEnginePrivate)
{
}

void
QFSFileEngine::setFileName(const QString &file)
{
    d->file = qt_fixToQtSlashes(file);
}

bool
QFSFileEngine::isOpen() const
{
    return (d->fd != -1);
}

bool
QFSFileEngine::open(int flags)
{
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
    d->hasCachedChar = 0;
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
            if(!st.st_size && readBlock(&d->cachedCharRead, 1) == 1) {
                d->cachedCharRead = 1;
                d->sequential = 1;
            }
        }
        return true;
    }
    return false;
}

bool
QFSFileEngine::open(int, int fd)
{
    d->hasCachedChar = 0;
    d->fd = fd;
    if(d->fd != -1) {
        d->sequential = 0;
        struct stat st;
        ::fstat(d->fd, &st);
	if ((st.st_mode & QT_STAT_MASK) != QT_STAT_REG || !fd) //stdin is non seekable
            d->sequential = 1;
#ifdef Q_OS_UNIX
	else if(!st.st_size && readBlock(&d->cachedCharRead, 1) == 1) {
            d->hasCachedChar = 1;
	    d->sequential = 1;
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
    int ret = QT_CLOSE(d->fd);
    d->fd = -1;
    return ret != -1;
}

void
QFSFileEngine::flush()
{
    //##not needed since I've not implemented buffered mode.. --Sam
}

Q_LONG
QFSFileEngine::readBlock(uchar *data, Q_LONG len)
{
    int ret = 0;
    if(d->hasCachedChar) {
        *(data++) = d->cachedCharRead;
        d->hasCachedChar = 0;
        len--;
        ret++;
    }
    if(len > 0) 
        ret += QT_READ(d->fd, data, len);
    return ret;
}

Q_LONG
QFSFileEngine::writeBlock(const uchar *data, Q_LONG len)
{
    return QT_WRITE(d->fd, data, len);
}

QFile::Offset
QFSFileEngine::at() const
{
    return QT_LSEEK(d->fd, 0, SEEK_CUR);
}

bool
QFSFileEngine::atEnd() const
{
    if(d->hasCachedChar)
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
        return false;
    }
    d->hasCachedChar = 0;
    return true;
}

bool
QFSFileEngine::isSequential() const
{
    return d->sequential;
}


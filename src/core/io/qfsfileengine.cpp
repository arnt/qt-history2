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

#if defined(O_NONBLOCK)
# define HAS_ASYNC_FILEMODE
# define QT_OPEN_ASYNC O_NONBLOCK
#elif defined(O_NDELAY)
# define HAS_ASYNC_FILEMODE
# define QT_OPEN_ASYNC O_NDELAY
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
    cachedCharRead = -1;
    fd = -1;
    init();
}

//**************** QFSFileEngine
QFSFileEngine::QFSFileEngine() : QFileEngine(*new QFSFileEnginePrivate)
{

}

bool
QFSFileEngine::isOpen() const
{
    return (d->fd != -1);
}

bool
QFSFileEngine::open(int mode, const QString &file)
{
    int oflags = QT_OPEN_RDONLY;
    if ((mode & QFile::ReadWrite) == QFile::ReadWrite)
        oflags = QT_OPEN_RDWR;
    else if ((mode & QFile::WriteOnly) == QFile::WriteOnly)
        oflags = QT_OPEN_WRONLY;
    if (mode & QFile::Append) {                // append to end of file?
        if (mode & QFile::Truncate)
            oflags |= (QT_OPEN_CREAT | QT_OPEN_TRUNC);
        else
            oflags |= (QT_OPEN_APPEND | QT_OPEN_CREAT);
    } else if (mode & QFile::WriteOnly) {                // create/trunc if writable
        if (mode & QFile::Truncate)
            oflags |= (QT_OPEN_CREAT | QT_OPEN_TRUNC);
        else
            oflags |= QT_OPEN_CREAT;
    }
#if defined(HAS_TEXT_FILEMODE)
    if (mode & QFile::Translate)
        oflags |= QT_OPEN_TEXT;
    else
        oflags |= QT_OPEN_BINARY;
#endif
#if defined(HAS_ASYNC_FILEMODE)
    if (mode & QFile::Async)
        oflags |= QT_OPEN_ASYNC;
#endif
    d->cachedCharRead = -1;
    d->fd = d->sysOpen(QFile::encodeName(file), oflags);
    if(d->fd != -1) {
        d->sequential = 0;
        struct stat st;
        ::fstat(d->fd, &st);
        if ((st.st_mode & S_IFMT) != S_IFREG) {
            d->sequential = 1;
        } else {
            struct stat st;
            ::fstat(d->fd, &st);
            if(!st.st_size && readBlock(&d->cachedCharRead, 1) == 1)
                d->sequential = 1;
        }
        return true;
    }
    return false;
}

bool
QFSFileEngine::open(int fd)
{
    d->cachedCharRead = -1;
    d->fd = fd;
    if(d->fd != -1) {
        d->sequential = 0;
        struct stat st;
        ::fstat(d->fd, &st);
	if ((st.st_mode & QT_STAT_MASK) != QT_STAT_REG || !fd) //stdin is non seekable
            d->sequential = 1;
#ifdef Q_OS_UNIX
	else if(!st.st_size && readBlock(&d->cachedCharRead, 1) == 1)
	    d->sequential = 1;
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
QFSFileEngine::readBlock(char *data, Q_ULONG len)
{
    int ret = 0;
    if(d->cachedCharRead != -1) {
        *(data++) = d->cachedCharRead;
        d->cachedCharRead = -1;
        len--;
        ret++;
    }
    if(len > 0) 
        ret += QT_READ(d->fd, data, len);
    return ret;
}

Q_LONG
QFSFileEngine::writeBlock(const char *data, Q_ULONG len)
{
    return QT_WRITE(d->fd, (void *)data, len);
}
    
QFile::Offset
QFSFileEngine::at() const
{
    return QT_LSEEK(d->fd, 0, SEEK_CUR);
}

bool
QFSFileEngine::atEnd() const
{
    if(d->cachedCharRead != -1)
        return false;
    return (at() == size());
}


QFile::Offset
QFSFileEngine::size() const
{
    QT_STATBUF st;
    QT_FSTAT(d->fd, &st);
    return st.st_size;
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
    d->cachedCharRead = -1;
    return true;
}

bool 
QFSFileEngine::isSequential() const
{
    return d->sequential;
}


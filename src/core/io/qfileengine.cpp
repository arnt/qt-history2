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

//**************** QFileEngine
QFileEngine::QFileEngine(QFileEnginePrivate &dd)  : QIOEngine(dd)
{
}

QFileEngine::~QFileEngine()
{
}

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
QFSFileEngine::errorMessage() const
{
    return d->errorMessage;
}

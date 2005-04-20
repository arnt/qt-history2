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

#include "qfsfileengine_p.h"

#include <qdatetime.h>

#include <errno.h>

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
    Q_D(QFSFileEngine);
    d->file = QFSFileEnginePrivate::fixToQtSlashes(file);
    d->resetErrors();
}

QFSFileEngine::QFSFileEngine() : QFileEngine(*new QFSFileEnginePrivate)
{
}

QFSFileEngine::QFSFileEngine(QFSFileEnginePrivate &dd)
    : QFileEngine(dd)
{
}


QFSFileEngine::~QFSFileEngine()
{
}

void
QFSFileEngine::setFileName(const QString &file)
{
    Q_D(QFSFileEngine);
    d->file = QFSFileEnginePrivate::fixToQtSlashes(file);
    d->tried_stat = false;
}

bool
QFSFileEngine::open(int flags)
{
    Q_D(QFSFileEngine);
    d->resetErrors();
    if (d->file.isEmpty()) {
        qWarning("QFSFileEngine::open: No file name specified");
        d->setError(QFile::OpenError, QLatin1String("No file name specified"));
        return false;
    }

    if (flags & QFile::Append)
        flags |= QFile::WriteOnly;

    int oflags = QT_OPEN_RDONLY;
    if ((flags & QFile::ReadWrite) == QFile::ReadWrite) {
        oflags = QT_OPEN_RDWR | QT_OPEN_CREAT;
    } else if (flags & QFile::WriteOnly) {
        oflags = QT_OPEN_WRONLY | QT_OPEN_CREAT;
    }

    if (flags & QFile::Append) {
        oflags |= QT_OPEN_APPEND;
    } else if (flags & QFile::WriteOnly) {
        if ((flags & QFile::Truncate) || !(flags & QFile::ReadOnly))
            oflags |= QT_OPEN_TRUNC;
    }

#if defined(Q_OS_MSDOS) || defined(Q_OS_WIN32) || defined(Q_OS_OS2)
    oflags |= QT_OPEN_BINARY; // we handle all text translations our self.
#endif

    d->external_file = 0;
    d->fd = d->sysOpen(d->file, oflags);
    if(d->fd != -1) {
        // Before appending, seek to the end of the file to allow
        // at() to return the correct position before ::write()
        //  has been called.
        if (flags & QFile::Append)
            QT_LSEEK(d->fd, 0, SEEK_END);

        d->sequential = 0;
        struct stat st;
        ::fstat(d->fd, &st);
        if ((st.st_mode & S_IFMT) != S_IFREG) {
            d->sequential = 1;
        } else {
            char char_read;
            if(!st.st_size && read(&char_read, 1) == 1) {
                d->ungetchBuffer += char_read;
                d->sequential = 1;
            }
        }
        return true;
    }
    d->setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError, errno);
    return false;
}

bool
QFSFileEngine::open(int, int fd)
{
    Q_D(QFSFileEngine);
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
            if(!st.st_size && read(&char_read, 1) == 1) {
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
    Q_D(QFSFileEngine);
    if (d->fd == -1)
        return false;
    d->resetErrors();
    flush();
    int ret = d->external_file ? 0 : QT_CLOSE(d->fd);
    d->tried_stat = 0;
    d->fd = -1;
    if(ret == -1) {
        d->setError(QFile::UnspecifiedError, errno);
        return false;
    }
    return true;
}

void
QFSFileEngine::flush()
{
    Q_D(QFSFileEngine);
    d->ungetchBuffer.clear();
}

qint64
QFSFileEngine::read(char *data, qint64 len)
{
    Q_D(QFSFileEngine);
    qint64 ret = 0;
    if (!d->ungetchBuffer.isEmpty()) {
        qint64 l = d->ungetchBuffer.size();
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
        if(read <= 0) {
            if(!ret)
                ret = -1;
            d->setError(QFile::ReadError, errno);
        } else {
            ret += read;
        }
    }
    return ret;
}

qint64
QFSFileEngine::write(const char *data, qint64 len)
{
    Q_D(QFSFileEngine);
    d->resetErrors();
    qint64 ret = QT_WRITE(d->fd, data, len);
    if(ret != len)
        d->setError(errno == ENOSPC ? QFile::ResourceError : QFile::WriteError, errno);
    return ret;
}

qint64
QFSFileEngine::at() const
{
    Q_D(const QFSFileEngine);
    return QT_LSEEK(d->fd, 0, SEEK_CUR);
}

int
QFSFileEngine::handle() const
{
    Q_D(const QFSFileEngine);
    return d->fd;
}

bool
QFSFileEngine::seek(qint64 pos)
{
    Q_D(QFSFileEngine);
    if(QT_LSEEK(d->fd, pos, SEEK_SET) == -1) {
        qWarning("QFile::at: Cannot set file position %lld", pos);
        d->setError(QFile::PositionError, errno);
        return false;
    }
    d->ungetchBuffer.clear();
    return true;
}

bool
QFSFileEngine::isSequential() const
{
    Q_D(const QFSFileEngine);
    return d->sequential;
}

QDateTime
QFSFileEngine::fileTime(FileTime time) const
{
    Q_D(const QFSFileEngine);
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

QFile::FileError
QFSFileEngine::error() const
{
    Q_D(const QFSFileEngine);
    return d->error;
}

QString
QFSFileEngine::errorString() const
{
    Q_D(const QFSFileEngine);
    return d->errorString;
}

QFileEngine::Type
QFSFileEngine::type() const
{
    return QFileEngine::File;
}

uchar
*QFSFileEngine::map(qint64 /*off*/, qint64 /*len*/)
{
    return 0;
}

void
QFSFileEngine::unmap(uchar * /*data*/)
{
}

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

#include <errno.h>
#include <stdio.h>

#include <qdatetime.h>

// Required to build with msvc.net 2002
#ifndef S_ISREG
#define S_ISREG(x)   (((x) & S_IFMT) == S_IFREG)
#endif

#ifdef Q_OS_WIN
#  ifndef S_ISCHR
#    define S_ISCHR(x)   (((x) & S_IFMT) == S_IFCHR)
#  endif
#  ifndef S_ISFIFO
#    define S_ISFIFO(x) false
#  endif
#  ifndef S_ISSOCK
#    define S_ISSOCK(x) false
#  endif
#endif

//**************** QFSFileEnginePrivate
QFSFileEnginePrivate::QFSFileEnginePrivate() : QAbstractFileEnginePrivate()
{
    sequential = 0;
    tried_stat = 0;
    fd = -1;
    fh = 0;
    lastIOCommand = IOFlushCommand;
    closeFileHandle = false;
    init();
}

//**************** QFSFileEngine
QFSFileEngine::QFSFileEngine(const QString &file) : QAbstractFileEngine(*new QFSFileEnginePrivate)
{
    Q_D(QFSFileEngine);
    d->file = QFSFileEnginePrivate::fixToQtSlashes(file);
}

QFSFileEngine::QFSFileEngine() : QAbstractFileEngine(*new QFSFileEnginePrivate)
{
}

QFSFileEngine::QFSFileEngine(QFSFileEnginePrivate &dd)
    : QAbstractFileEngine(dd)
{
}


QFSFileEngine::~QFSFileEngine()
{
    Q_D(QFSFileEngine);
    if (d->closeFileHandle) {
        if (d->fh) {
            fclose(d->fh);
        } else if (d->fd != -1) {
            ::close(d->fd);
        }
    }
}

void
QFSFileEngine::setFileName(const QString &file)
{
    Q_D(QFSFileEngine);
    d->file = QFSFileEnginePrivate::fixToQtSlashes(file);
    d->tried_stat = 0;
}

static QByteArray openModeToFopenMode(QIODevice::OpenMode flags, const QString &fileName = QString())
{
    QByteArray mode;
    if ((flags & QIODevice::ReadOnly) && !(flags & QIODevice::Truncate)) {
        mode = "rb";
        if (flags & QIODevice::WriteOnly) {
            if (!fileName.isEmpty() &&QFile::exists(fileName))
                mode = "rb+";
            else
                mode = "wb+";
        }
    } else if (flags & QIODevice::WriteOnly) {
        mode = "wb";
        if (flags & QIODevice::ReadOnly)
            mode += "+";
    }
    if (flags & QIODevice::Append) {
        mode = "ab";
        if (flags & QIODevice::ReadOnly)
            mode += "+";
    }
    return mode;
}

bool
QFSFileEngine::open(QIODevice::OpenMode flags)
{
    Q_D(QFSFileEngine);
    if (d->file.isEmpty()) {
        qWarning("QFSFileEngine::open: No file name specified");
        setError(QFile::OpenError, QLatin1String("No file name specified"));
        return false;
    }

    if (flags & QFile::Append)
        flags |= QFile::WriteOnly;
#ifdef Q_OS_UNIX
    d->fh = QT_FOPEN(QFile::encodeName(d->file).constData(),
                     openModeToFopenMode(flags, d->file).constData());
    if (!d->fh) {
        QString errString = QT_TRANSLATE_NOOP(QFSFileEngine, "Unknown error");
        setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
                 qt_error_string(int(errno)));
        return false;
    }

    if (flags & QIODevice::Unbuffered)
        setvbuf(d->fh, 0, _IONBF, 0);

    if (flags & QIODevice::Append)
        QT_FSEEK(d->fh, 0, SEEK_END);

    d->closeFileHandle = true;
    d->fd = QT_FILENO(d->fh);

    QT_STATBUF st;
    if (QT_FSTAT(QT_FILENO(d->fh), &st) != 0)
        return false;
    d->sequential = S_ISCHR(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode);
    return true;
#endif

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
        if ((st.st_mode & S_IFMT) != S_IFREG)
            d->sequential = 1;
        return true;
    }
    setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError, qt_error_string(errno));
    return false;
}

bool
QFSFileEngine::open(QIODevice::OpenMode flags, int fd)
{
    Q_D(QFSFileEngine);
    d->closeFileHandle = false;
#ifdef Q_OS_UNIX
    d->fh = fdopen(d->fd, openModeToFopenMode(flags).constData());
    if (!d->fh) {
        QString errString = QT_TRANSLATE_NOOP(QFSFileEngine, "Unknown error");
        setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
                 qt_error_string(int(errno)));
        return false;
    }

    if (flags & QIODevice::Unbuffered)
        setvbuf(d->fh, 0, _IONBF, 0);

    if (flags & QIODevice::Append)
        QT_FSEEK(d->fh, 0, SEEK_END);

    d->fd = QT_FILENO(d->fh);

    QT_STATBUF st;
    if (QT_FSTAT(QT_FILENO(d->fh), &st) != 0)
        return false;
    d->sequential = S_ISCHR(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode);
    return true;
#else
    Q_UNUSED(flags);
#endif
    d->fd = fd;
    if(d->fd != -1) {
        d->sequential = 0;
        struct stat st;
        ::fstat(d->fd, &st);
	if ((st.st_mode & QT_STAT_MASK) != QT_STAT_REG || !fd) //stdin is non seekable
            d->sequential = 1;
        return true;
    }
    return false;
}

bool QFSFileEngine::open(QIODevice::OpenMode /* flags */, FILE *fh)
{
    Q_D(QFSFileEngine);
    d->fh = fh;
    d->fd = QT_FILENO(fh);
    QT_STATBUF st;
    if (QT_FSTAT(QT_FILENO(fh), &st) != 0)
	return false;
    d->sequential = S_ISCHR(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode);
    d->closeFileHandle = false;
    return true;
}

bool
QFSFileEngine::close()
{
    Q_D(QFSFileEngine);

    flush();
    d->tried_stat = 0;
    if (d->fh) {
	if (d->closeFileHandle)
	    fclose(d->fh);
	d->fh = 0;
	d->fd = -1;
	return true;
    }

    if (d->fd == -1)
        return false;

    int ret = d->closeFileHandle ? QT_CLOSE(d->fd) : 0;
    d->fd = -1;
    if(ret == -1) {
	setError(QFile::UnspecifiedError, qt_error_string(errno));
        return false;
    }
    return true;
}

bool
QFSFileEngine::flush()
{
    Q_D(QFSFileEngine);
    d->ungetchBuffer.clear();

    if (!d->fh)
        return false;
#ifdef Q_OS_WIN
    QT_FPOS_T pos;
    int gotPos = QT_FGETPOS(d->fh, &pos);
#endif
    fflush(d->fh);
#ifdef Q_OS_WIN
    if (gotPos == 0)
        QT_FSETPOS(d->fh, &pos);
#endif
    d->lastIOCommand = QFSFileEnginePrivate::IOFlushCommand;
    return true;
}

qint64
QFSFileEngine::read(char *data, qint64 len)
{
    Q_D(QFSFileEngine);

    if (d->fh) {
        if (d->lastIOCommand != QFSFileEnginePrivate::IOReadCommand) {
            flush();
            d->lastIOCommand = QFSFileEnginePrivate::IOReadCommand;
        }

        if (feof(d->fh))
            return 0;

        size_t readBytes = 0;
#ifdef Q_OS_UNIX
        if (d->sequential) {
            int oldFlags = fcntl(fileno(d->fh), F_GETFL);

            for (int i = 0; i < 2; ++i) {
                // Make the underlying file descriptor non-blocking
                int v = 1;
                if ((oldFlags & O_NONBLOCK) == 0)
                    fcntl(fileno(d->fh), F_SETFL, oldFlags | O_NONBLOCK, &v, sizeof(v));

                size_t read = fread(data + readBytes, 1, size_t(len - readBytes), d->fh);
                if (read > 0) {
                    readBytes += read;
                    break;
                } else {
                    if (readBytes)
                        break;
                    readBytes = read;
                }

                // Restore the blocking state of the underlying socket
                if ((oldFlags & O_NONBLOCK) == 0) {
                    int v = 1;
                    fcntl(fileno(d->fh), F_SETFL, oldFlags, &v, sizeof(v));
                    if (readBytes == 0) {
                        int readByte = fgetc(d->fh);
                        if (readByte != -1) {
                            *data = uchar(readByte);
                            readBytes += 1;
                        }
                    }
                }
            }
            if ((oldFlags & O_NONBLOCK) == 0) {
                int v = 1;
                fcntl(fileno(d->fh), F_SETFL, oldFlags, &v, sizeof(v));
            }
        } else
#endif
        {
            readBytes = fread(data, 1, size_t(len), d->fh);
        }
        if (readBytes == 0)
            setError(QFile::ReadError, qt_error_string(int(errno)));
        return readBytes;
    }

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
    if(len && ret != len) {
        int read = QT_READ(d->fd, data, len);
        if(read <= 0) {
            if(!ret)
                ret = -1;
            setError(QFile::ReadError, qt_error_string(errno));
        } else {
            ret += read;
        }
    }
    return ret;
}

qint64 QFSFileEngine::readLine(char *data, qint64 maxlen)
{
    Q_D(QFSFileEngine);
    if (!d->fh)
        return QAbstractFileEngine::readLine(data, maxlen);

    if (d->lastIOCommand != QFSFileEnginePrivate::IOReadCommand) {
        flush();
        d->lastIOCommand = QFSFileEnginePrivate::IOReadCommand;
    }
    if (feof(d->fh))
        return 0;

    // QIODevice::readLine() passes maxlen - 1 to QFile::readLineData()
    // because it has made space for the '\0' at the end of data.  But fgets
    // does the same, so we'd get two '\0' at the end - passing maxlen + 1
    // solves this.
    if (!fgets(data, int(maxlen + 1), d->fh)) {
        setError(QFile::ReadError, qt_error_string(int(errno)));
	return 0;
    }
    return qstrlen(data);
}

qint64
QFSFileEngine::write(const char *data, qint64 len)
{
    Q_D(QFSFileEngine);

    if (d->fh) {
        if (d->lastIOCommand != QFSFileEnginePrivate::IOWriteCommand) {
            flush();
            d->lastIOCommand = QFSFileEnginePrivate::IOWriteCommand;
        }
        qint64 result = qint64(fwrite(data, 1, size_t(len), d->fh));
        if (result > 0)
            return result;
        setError(QFile::ReadError, qt_error_string(int(errno)));
        return result;
    }

    qint64 ret = QT_WRITE(d->fd, data, len);
    if(ret != len)
        setError(errno == ENOSPC ? QFile::ResourceError : QFile::WriteError, qt_error_string(errno));
    return ret;
}

qint64
QFSFileEngine::pos() const
{
    Q_D(const QFSFileEngine);
    if (d->fh)
        return qint64(QT_FTELL(d->fh));
    return QT_LSEEK(d->fd, 0, SEEK_CUR);
}

int
QFSFileEngine::handle() const
{
    Q_D(const QFSFileEngine);
    return d->fd;
}

QAbstractFileEngine::Iterator *QFSFileEngine::beginEntryList(QDir::Filters filters, const QStringList &filterNames)
{
    Q_UNUSED(filters);
    Q_UNUSED(filterNames);
    return 0;
}

QAbstractFileEngine::Iterator *QFSFileEngine::endEntryList()
{
    return 0;
}

bool
QFSFileEngine::seek(qint64 pos)
{
    Q_D(QFSFileEngine);
    if (d->fh) {
        if (QT_FSEEK(d->fh, QT_OFF_T(pos), SEEK_SET) == -1) {
            setError(QFile::ReadError, qt_error_string(int(errno)));
            return false;
        }
        return true;
    }

    if(QT_LSEEK(d->fd, pos, SEEK_SET) == -1) {
        qWarning("QFile::at: Cannot set file position %lld", pos);
        setError(QFile::PositionError, qt_error_string(errno));
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

bool QFSFileEngine::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
    Q_UNUSED(extension);
    Q_UNUSED(option);
    Q_UNUSED(output);
    return false;
}

bool QFSFileEngine::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

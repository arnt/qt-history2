#include <qdatetime.h>
#include <stdio.h>
#include <qdebug.h>
#include <errno.h>

#include "qfileengine.h"
#include "qbufferedfsfileengine_p.h"

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
#  ifndef fseeko
#    define fseeko fseek
#  endif
#  ifndef ftello
#    define ftello ftell
#  endif
#  ifdef off_t
#    undef off_t
#  endif
#  define off_t long
#endif

QBufferedFSFileEngine::QBufferedFSFileEngine(const QString &fileName)
    : QFSFileEngine(*new QBufferedFSFileEnginePrivate)
{
    Q_D(QBufferedFSFileEngine);
    d->file = QFSFileEnginePrivate::fixToQtSlashes(fileName);
}

QBufferedFSFileEngine::~QBufferedFSFileEngine()
{
    Q_D(QBufferedFSFileEngine);
    if (d->fh && d->closeFileHandle)
        fclose(d->fh);
}

QFileEngine::Type QBufferedFSFileEngine::type() const
{
    return BufferedFile;
}

bool QBufferedFSFileEngine::open(int flags)
{
    Q_D(QBufferedFSFileEngine);
    if (d->file.isEmpty()) {
        qWarning("QBufferedFSFileEngine::open: No file name specified");
        d->setError(QFile::OpenError, QT_TRANSLATE_NOOP(QBufferedFSFileEngine, "No file name specified"));
        return false;
    }

    QByteArray mode;
    if ((flags & QIODevice::ReadOnly) && !(flags & QIODevice::Truncate)) {
        mode = "rb";
        if (flags & QIODevice::WriteOnly) {
            if (QFile::exists(d->file))
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
    d->fh = fopen(QFile::encodeName(d->file).constData(), mode.constData());
    if (!d->fh) {
        QString errString = QT_TRANSLATE_NOOP(QBufferedFSFileEngine, "Unknown error");
        d->setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
                    qt_error_string(int(errno)));
        return false;
    }
    if (flags & QIODevice::Append)
        fseeko(d->fh, 0, SEEK_END);

    d->closeFileHandle = true;
    d->fd = fileno(d->fh);
    struct stat st;
    if (::fstat(fileno(d->fh), &st) != 0)
	return false;
    d->sequential = S_ISCHR(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode);

    return true;
}

bool QBufferedFSFileEngine::open(int /* flags */, FILE *fh)
{
    Q_D(QBufferedFSFileEngine);
    d->fh = fh;
    d->fd = fileno(fh);
    struct stat st;
    if (::fstat(fileno(fh), &st) != 0)
	return false;
    d->sequential = S_ISCHR(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode);
    d->closeFileHandle = false;
    return true;
}

bool QBufferedFSFileEngine::close()
{
    Q_D(QBufferedFSFileEngine);
    flush();
    if (d->fh && d->closeFileHandle)
        fclose(d->fh);
    d->fh = 0;
    d->fd = -1;
    return true;
}

void QBufferedFSFileEngine::flush()
{
    Q_D(QBufferedFSFileEngine);
#ifdef Q_OS_WIN
    fpos_t pos;
    int gotPos = fgetpos(d->fh, &pos);
#endif
    fflush(d->fh);
#ifdef Q_OS_WIN
    if (gotPos == 0)
        fsetpos(d->fh, &pos);
#endif
    d->lastIOCommand = QBufferedFSFileEnginePrivate::IOFlushCommand;
}

qint64 QBufferedFSFileEngine::at() const
{
    Q_D(const QBufferedFSFileEngine);
    return qint64(ftell(d->fh));
}

bool QBufferedFSFileEngine::seek(qint64 offset)
{
    Q_D(QBufferedFSFileEngine);
    if (fseeko(d->fh, off_t(offset), SEEK_SET) == -1) {
        d->setError(QFile::ReadError, qt_error_string(int(errno)));
        return false;
    }
    return true;
}

qint64 QBufferedFSFileEngine::read(char *data, qint64 maxlen)
{
    Q_D(QBufferedFSFileEngine);
    if (d->lastIOCommand != QBufferedFSFileEnginePrivate::IOReadCommand) {
        flush();
        d->lastIOCommand = QBufferedFSFileEnginePrivate::IOReadCommand;
    }

    if (feof(d->fh))
        return 0;

#ifdef Q_OS_UNIX
    size_t readBytes = 0;
    int oldFlags = fcntl(fileno(d->fh), F_GETFL);
    
    for (int i = 0; i < 2; ++i) {
	// Make the underlying file descriptor non-blocking
	if (d->sequential) {
	    int v = 1;
	    if ((oldFlags & O_NONBLOCK) == 0)
		fcntl(fileno(d->fh), F_SETFL, oldFlags | O_NONBLOCK, &v, sizeof(v));
	}

	size_t read = fread(data + readBytes, 1, size_t(maxlen - readBytes), d->fh);
	if (read > 0) {
	    readBytes += read;
	    break;
	} else {
	    if (readBytes)
		break;
	    readBytes = read;
	}

	// Restore the blocking state of the underlying socket
	if (d->sequential) {
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
    }
    if ((oldFlags & O_NONBLOCK) == 0) {
	int v = 1;
	fcntl(fileno(d->fh), F_SETFL, oldFlags, &v, sizeof(v));
    }
#else
    size_t readBytes = fread(data, 1, size_t(maxlen), d->fh);
#endif
    if (readBytes == 0)
        d->setError(QFile::ReadError, qt_error_string(int(errno)));
    return readBytes;
}

qint64 QBufferedFSFileEngine::readLine(char *data, qint64 maxlen)
{
    Q_D(QBufferedFSFileEngine);
    if (d->lastIOCommand != QBufferedFSFileEnginePrivate::IOReadCommand) {
        flush();
        d->lastIOCommand = QBufferedFSFileEnginePrivate::IOReadCommand;
    }
    if (feof(d->fh))
        return 0;
    if (!fgets(data, int(maxlen), d->fh)) {
        d->setError(QFile::ReadError, qt_error_string(int(errno)));
	return 0;
    }
    return qstrlen(data);
}

qint64 QBufferedFSFileEngine::write(const char *data, qint64 len)
{
    Q_D(QBufferedFSFileEngine);
    if (d->lastIOCommand != QBufferedFSFileEnginePrivate::IOWriteCommand) {
        flush();
        d->lastIOCommand = QBufferedFSFileEnginePrivate::IOWriteCommand;
    }
    qint64 result = qint64(fwrite(data, 1, size_t(len), d->fh));
    if (result > 0)
        return result;
    d->setError(QFile::ReadError, qt_error_string(int(errno)));
    return result;
}


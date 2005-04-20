#include <qdatetime.h>
#include <stdio.h>

#include "qfileengine.h"
#include "qbufferedfsfileengine_p.h"

// Required to build with msvc.net 2002
#ifndef S_ISREG
#define S_ISREG(x)   (((x) & S_IFMT) == S_IFREG)
#endif

QBufferedFSFileEngine::QBufferedFSFileEngine()
    : QFSFileEngine(*new QBufferedFSFileEnginePrivate)
{
}

QFileEngine::Type QBufferedFSFileEngine::type() const
{
    return Type(BufferedFSFileEngine);
}

bool QBufferedFSFileEngine::open(int /* flags */)
{
    return false;
}

bool QBufferedFSFileEngine::open(int /* flags */, FILE *fh)
{
    Q_D(QBufferedFSFileEngine);
    d->fh = fh;
    d->fd = fileno(fh);
    struct stat st;
    if (::fstat(fileno(fh), &st) != 0)
	return false;
    d->sequential = !S_ISREG(st.st_mode);
    return true;
}

bool QBufferedFSFileEngine::close()
{
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
    return fseek(d->fh, long(offset), SEEK_SET) != -1;
}

qint64 QBufferedFSFileEngine::read(char *data, qint64 maxlen)
{
    Q_D(QBufferedFSFileEngine);
    if (d->lastIOCommand != QBufferedFSFileEnginePrivate::IOReadCommand) {
        flush();
        d->lastIOCommand = QBufferedFSFileEnginePrivate::IOReadCommand;
    }
    if (feof(d->fh))
	    return -1;
    return fread(data, 1, size_t(maxlen), d->fh);
}

qint64 QBufferedFSFileEngine::write(const char *data, qint64 len)
{
    Q_D(QBufferedFSFileEngine);
    if (d->lastIOCommand != QBufferedFSFileEnginePrivate::IOWriteCommand) {
        flush();
        d->lastIOCommand = QBufferedFSFileEnginePrivate::IOWriteCommand;
    }
    return fwrite(data, 1, size_t(len), d->fh);
}


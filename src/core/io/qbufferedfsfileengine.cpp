#include <qdatetime.h>
#include <stdio.h>

#include "qfileengine.h"
#include "qbufferedfsfileengine_p.h"

QBufferedFSFileEngine::QBufferedFSFileEngine()
    : QFSFileEngine(*new QBufferedFSFileEnginePrivate)
{
}

bool QBufferedFSFileEngine::open(int /* flags */, FILE *fh)
{
    Q_D(QBufferedFSFileEngine);
    d->fh = fh;
    d->fd = fileno(fh);
    return true;
}

bool QBufferedFSFileEngine::close()
{
    return true;
}

void QBufferedFSFileEngine::flush()
{
    Q_D(QBufferedFSFileEngine);
    fflush(d->fh);
}

Q_LONGLONG QBufferedFSFileEngine::at() const
{
    Q_D(const QBufferedFSFileEngine);
    return Q_LONGLONG(ftell(d->fh));
}

bool QBufferedFSFileEngine::seek(Q_LONGLONG offset)
{
    Q_D(QBufferedFSFileEngine);
    return fseek(d->fh, long(offset), SEEK_SET) != -1;
}

Q_LONGLONG QBufferedFSFileEngine::read(char *data, Q_LONGLONG maxlen)
{
    Q_D(QBufferedFSFileEngine);
    return fread(data, 1, size_t(maxlen), d->fh);
}

Q_LONGLONG QBufferedFSFileEngine::write(const char *data, Q_LONGLONG len)
{
    Q_D(QBufferedFSFileEngine);
    return fwrite(data, 1, size_t(len), d->fh);
}


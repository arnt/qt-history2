/****************************************************************************
**
** Implementation of QFile class.
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

#include "qfile.h"
#include <qplatformdefs.h>
#include <qglobal.h>
#include <private/qiodevice_p.h>
#include <qfileengine.h>
#include <qfileinfo.h>


#include <errno.h>

#define d d_func()
#define q q_func()

//************* QFilePrivate
class QFilePrivate : public QIODevicePrivate
{
    QFile *q_ptr;
    Q_DECLARE_PUBLIC(QFile)

protected:
    QFilePrivate(QFile*);
    ~QFilePrivate();

    void initFileEngine(const QString &);

    bool openExternalFile(int fd);
    void reset();

private:
    inline static QByteArray locale_encode(const QString &f) 
           { return f.toLocal8Bit(); }
    static QFile::EncoderFn encoder;
    inline static QString locale_decode(const QByteArray &f) 
           { return QString::fromLocal8Bit(f); }
    static QFile::DecoderFn decoder;
    
    QFileEngine *fileEngine;

    QString fileName;
    uint external_file : 1;
    QByteArray ungetchBuffer;
};

QFile::EncoderFn QFilePrivate::encoder = QFilePrivate::locale_encode;
QFile::DecoderFn QFilePrivate::decoder = QFilePrivate::locale_decode;

QFilePrivate::QFilePrivate(QFile *qq) : q_ptr(qq), fileEngine(0), external_file(0)
{ 
    reset();
}

QFilePrivate::~QFilePrivate()
{
    delete fileEngine;
    fileEngine = 0;
}

void 
QFilePrivate::initFileEngine(const QString &file)
{
    Q_ASSERT(!fileEngine || !fileEngine->isOpen());
    delete fileEngine;
    fileName = file;
    fileEngine = new QFSFileEngine;
    reset();
}

bool
QFilePrivate::openExternalFile(int fd)
{
    Q_ASSERT(!fileEngine || !fileEngine->isOpen());
    delete fileEngine;
    external_file = true;
    QFSFileEngine *fe = new QFSFileEngine;
    fileEngine = fe;
    reset();
    return fe->open(fd);
}

void 
QFilePrivate::reset()
{
    ungetchBuffer.clear();
}

//************* QFile
QFile::QFile() : d_ptr(new QFilePrivate(this))
{
    
}

QFile::QFile(const QString &name): d_ptr(new QFilePrivate(this))
{
    d->initFileEngine(name);
}

QFile::~QFile()
{
    delete d_ptr;
    d_ptr = 0;
}

QString
QFile::name() const
{
    return d->fileName;
}

void
QFile::setName(const QString &name)
{
    d->fileName = name;
}

QByteArray
QFile::encodeName(const QString &fileName)
{
    return (*QFilePrivate::encoder)(fileName);
}

QString
QFile::decodeName(const QByteArray &localFileName)
{
    return (*QFilePrivate::decoder)(localFileName);
}

void
QFile::setEncodingFunction(EncoderFn f)
{
    QFilePrivate::encoder = f;
}

void
QFile::setDecodingFunction(DecoderFn f)
{
    QFilePrivate::decoder = f;
}

bool
QFile::exists() const
{
    return QFileInfo(*this).exists();    
}

bool
QFile::exists(const QString &fileName)
{
    return QFileInfo(fileName).exists();    
}

bool
QFile::remove()
{
    if (d->fileName.isEmpty()) {
        qWarning("QFile::remove: Empty or null file name");
        return false;
    }
    if(!d->fileEngine)
        return false;
    return d->fileEngine->remove(d->fileName);
}

bool
QFile::remove(const QString &fileName)
{
    return QFile(fileName).remove();
}

bool
QFile::open(int mode)
{
    if (d->fileEngine && d->fileEngine->isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    if (d->fileName.isEmpty()) {
        qWarning("QFile::open: No file name specified");
        return false;
    }
    if(mode & (Append|WriteOnly)) //append implies write
        mode |= WriteOnly;
    setFlags(IO_Direct);
    resetStatus();
    setMode(mode);
    if (!(isReadable() || isWritable())) {
        qWarning("QFile::open: File access not specified");
        return false;
    }
    d->external_file = false;
    d->reset();
    if(!d->fileEngine)
        d->initFileEngine(d->fileName);
    if(d->fileEngine->open(mode, d->fileName)) {
        setState(IO_Open);
        if(d->fileEngine->isSequential())
            setType(Sequential);
        return true;
    }
    return false;
}

bool
QFile::open(int mode, FILE *fh)
{
    return open(mode, QT_FILENO(fh));
}

bool
QFile::open(int mode, int fd)
{
    if (d->fileEngine && d->fileEngine->isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    if(mode & (Append|WriteOnly)) //append implies write
        mode |= WriteOnly;
    setFlags(IO_Direct);
    resetStatus();
    setMode(mode);
    if (!(isReadable() || isWritable())) {
        qWarning("QFile::open: File access not specified");
        return false;
    }
    if(d->openExternalFile(fd)) {
        setState(IO_Open);
        setMode(mode | IO_Raw);
        if(d->fileEngine->isSequential())
            setType(IO_Sequential);
        return true;
    }
    return false;
}

void
QFile::close()
{
    bool closed = true;
    if(d->external_file) {
        flush();
    } else {
        if(!d->fileEngine->close()) {
            closed = false;
            setStatus(IO_UnspecifiedError, errno);
        }
    }
    if(closed) {
        setFlags(IO_Direct);
        resetStatus();
    }
}

void
QFile::flush()
{
    if(d->fileEngine && d->fileEngine->isOpen()) {
        d->ungetchBuffer.clear();
        d->fileEngine->flush();
    }
}

QFile::Offset
QFile::size() const
{
    if(!d->fileEngine)
        return 0;
    return d->fileEngine->size();
}

QFile::Offset
QFile::at() const
{
    if (!isOpen()) 
        return 0;
    return d->fileEngine->at();
}

bool
QFile::at(Offset offset)
{
    if (!isOpen()) {
        qWarning("QFile::at: File is not open");
        return false;
    }
    if (isSequentialAccess()) 
        return false;
    if(d->fileEngine->seek(offset)) {
        d->ungetchBuffer.clear();
        return true;
    }
    return false;
}

bool
QFile::atEnd() const
{
    if (!isOpen()) {
        qWarning("QFile::atEnd: File is not open");
        return false;
    }
    return d->fileEngine->atEnd();
}

Q_LONG
QFile::readBlock(char *data, Q_ULONG len)
{
    if (len <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {
        qWarning("QFile::readBlock: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QFile::readBlock: Read operation not permitted");
        return -1;
    }

    Q_ULONG ret = 0;
    if (!d->ungetchBuffer.isEmpty()) {
        uint l = d->ungetchBuffer.size();
        while(ret < l) {
            *data = d->ungetchBuffer.at(l - ret - 1);
            data++;
            ret++;
        }
        d->ungetchBuffer.resize(l - ret);
    }
    if(ret != len) {
        ret += d->fileEngine->readBlock(data, len-ret);
        if (len && ret <= 0) {
            ret = 0;
            setStatus(IO_ReadError, errno);
        }
    }
    return ret;
}

Q_LONG
QFile::writeBlock(const char *data, Q_ULONG len)
{
    if (!len) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {                                // file not open
        qWarning("QFile::writeBlock: File not open");
        return -1;
    }
    if (!isWritable()) {                        // writing not permitted
        qWarning("QFile::writeBlock: Write operation not permitted");
        return -1;
    }

    Q_ULONG ret = d->fileEngine->writeBlock(data, len);
    if (ret != len) // write error
        setStatus(errno == ENOSPC ? IO_ResourceError : IO_WriteError, errno);
    return ret;
}

Q_LONG
QFile::readLine(char *data, Q_ULONG maxlen)
{
    if (maxlen == 0) // application bug?
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {
        qWarning("QFile::readLine: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QFile::readLine: Read operation not permitted");
        return -1;
    }

    return QIODevice::readLine(data, maxlen);
}

Q_LONG
QFile::readLine(QString &s, Q_ULONG maxlen)
{
    QByteArray ba;
    ba.resize(maxlen);
    Q_LONG l = readLine(ba.data(),maxlen);
    if (l >= 0)
        s = QString::fromLatin1(ba);
    return l;
}

int
QFile::getch()
{
    if (!isOpen()) {                                // file not open
        qWarning("QFile::getch: File not open");
        return EOF;
    }
    if (!isReadable()) {                        // reading not permitted
        qWarning("QFile::getch: Read operation not permitted");
        return EOF;
    }
    char ret;
    if (!d->ungetchBuffer.isEmpty()) {
        int len = d->ungetchBuffer.size();
        ret = d->ungetchBuffer[len - 1];
        d->ungetchBuffer.truncate(len - 1);
    } else if(readBlock(&ret, 1) != 1) {
        ret = EOF;
    }
    return (int)ret;
}

int
QFile::putch(int ch)
{
    if (!isOpen()) {                                // file not open
        qWarning("QFile::putch: File not open");
        return EOF;
    }
    if (!isWritable()) {                        // writing not permitted
        qWarning("QFile::putch: Write operation not permitted");
        return EOF;
    }

    char ret = ch;
    if(writeBlock((char*)&ret, 1) != 1)
        ret = EOF;
    return (int)ret;
}

int
QFile::ungetch(int ch)
{
    if (!isOpen()) {                                // file not open
        qWarning("QFile::ungetch: File not open");
        return EOF;
    }
    if (!isReadable()) {                        // reading not permitted
        qWarning("QFile::ungetch: Read operation not permitted");
        return EOF;
    }
    if (ch == EOF)                                // cannot unget EOF
        return ch;
    d->ungetchBuffer += ch;
    return ch;
}

int
QFile::handle() const
{
    if (!isOpen())
        return -1;
    return d->fileEngine->handle();
}

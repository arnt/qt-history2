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

#ifndef QIODEVICE_H
#define QIODEVICE_H

#ifndef QT_NO_QOBJECT
#include <QtCore/qobject.h>
#endif
#include <QtCore/qstring.h>

#include "QtCore/qobjectdefs.h"

#ifdef open
#error qiodevice.h must be included before any system header that defines open
#endif

class QByteArray;
class QIODevicePrivate;

class Q_CORE_EXPORT QIODevice
#ifndef QT_NO_QOBJECT
    : public QObject
#endif
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#endif
public:
    enum OpenModeFlag {
        NotOpen = 0x0000,
        ReadOnly = 0x0001,
        WriteOnly = 0x0002,
        ReadWrite = ReadOnly | WriteOnly,
        Append = 0x0004,
        Truncate = 0x0008,
        Text = 0x0010,
        Unbuffered = 0x0020
    };
    Q_DECLARE_FLAGS(OpenMode, OpenModeFlag)

    QIODevice();
#ifndef QT_NO_QOBJECT
    explicit QIODevice(QObject *parent);
#endif
    virtual ~QIODevice();

    OpenMode openMode() const;

    bool isOpen() const;
    bool isReadable() const;
    bool isWritable() const;
    virtual bool isSequential() const;

    virtual bool open(OpenMode mode);
    virtual void close();
    virtual bool flush();

    virtual Q_LONGLONG pos() const;
    virtual Q_LONGLONG size() const;
    virtual bool seek(Q_LONGLONG pos);
    virtual bool atEnd() const;
    virtual bool reset();

    virtual Q_LONGLONG bytesAvailable() const;
    virtual Q_LONGLONG bytesToWrite() const;

    Q_LONGLONG read(char *data, Q_LONGLONG maxlen);
    QByteArray read(Q_LONGLONG maxlen);
    QByteArray readAll();
    Q_LONGLONG readLine(char *data, Q_LONGLONG maxlen);
    QByteArray readLine(Q_LONGLONG maxlen = 0);
    virtual bool canReadLine() const;

    Q_LONGLONG write(const char *data, Q_LONGLONG len);
    Q_LONGLONG write(const QByteArray &data);

    virtual bool waitForReadyRead(int msecs);
    virtual bool waitForBytesWritten(int msecs);

    bool getChar(char *c);
    bool putChar(char c);
    void ungetChar(char c);

    QString errorString() const;

#ifndef QT_NO_QOBJECT
signals:
    void readyRead();
    void bytesWritten(Q_LONGLONG bytes);
#endif

protected:
#ifdef QT_NO_QOBJECT
    QIODevice(QIODevicePrivate &d);
#else
    QIODevice(QIODevicePrivate &d, QObject *parent);
#endif

    virtual Q_LONGLONG readData(char *data, Q_LONGLONG maxlen) = 0;
    virtual Q_LONGLONG writeData(const char *data, Q_LONGLONG len) = 0;

    void setOpenMode(OpenMode openMode);

    void setErrorString(const QString &errorString);

#ifdef QT_NO_QOBJECT
    QIODevicePrivate *d_ptr;
#endif

private:
    Q_DECLARE_PRIVATE(QIODevice)
    Q_DISABLE_COPY(QIODevice)

#ifdef QT_COMPAT
public:
    typedef Q_LONGLONG Offset;

    inline QT_COMPAT int flags() const { return (int) openMode(); }
    inline QT_COMPAT int mode() const { return (int) openMode(); }
    inline QT_COMPAT int state() const;

    inline QT_COMPAT bool isDirectAccess() const { return !isSequential(); }
    inline QT_COMPAT bool isSequentialAccess() const { return isSequential(); }
    inline QT_COMPAT bool isCombinedAccess() const { return false; }
    inline QT_COMPAT bool isBuffered() const { return true; }
    inline QT_COMPAT bool isRaw() const { return false; }
    inline QT_COMPAT bool isSynchronous() const { return true; }
    inline QT_COMPAT bool isAsynchronous() const { return false; }
    inline QT_COMPAT bool isTranslated() const { return (openMode() & Text) != 0; }
    inline QT_COMPAT bool isInactive() const { return !isOpen(); }

    typedef int Status;
    QT_COMPAT Status status() const;
    QT_COMPAT void resetStatus();

    inline QT_COMPAT Offset at() const { return pos(); }
    inline QT_COMPAT bool at(Offset offset) { return seek(offset); }

    inline QT_COMPAT Q_LONG readBlock(char *data, Q_ULONG maxlen) { return read(data, maxlen); }
    inline QT_COMPAT Q_LONG writeBlock(const char *data, Q_ULONG len) { return write(data, len); }
    inline QT_COMPAT Q_LONG writeBlock(const QByteArray &data) { return write(data); }

    inline QT_COMPAT int getch() { char c; return getChar(&c) ? int(c) : -1; }
    inline QT_COMPAT int putch(int c) { return putChar(c) ? int(c) : -1; }
    inline QT_COMPAT int ungetch(int c) { ungetChar(c); return c; }
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QIODevice::OpenMode)

#ifdef QT_COMPAT
static QT_COMPAT_VARIABLE const uint IO_Direct = 0x0100;
static QT_COMPAT_VARIABLE const uint IO_Sequential = 0x0200;
static QT_COMPAT_VARIABLE const uint IO_Combined = 0x0300;
static QT_COMPAT_VARIABLE const uint IO_TypeMask = 0x0300;

static QT_COMPAT_VARIABLE const uint IO_Raw = 0x0000;
static QT_COMPAT_VARIABLE const uint IO_Async = 0x0000;

#define IO_ReadOnly QIODevice::ReadOnly
#define IO_WriteOnly QIODevice::WriteOnly
#define IO_ReadWrite QIODevice::ReadWrite
#define IO_Append QIODevice::Append
#define IO_Truncate QIODevice::Truncate
#define IO_Translate QIODevice::Text
#define IO_ModeMask 0x00ff

static QT_COMPAT_VARIABLE const uint IO_Open = 0x1000;
static QT_COMPAT_VARIABLE const uint IO_StateMask = 0xf000;

static QT_COMPAT_VARIABLE const uint IO_Ok = 0;
static QT_COMPAT_VARIABLE const uint IO_ReadError = 1;
static QT_COMPAT_VARIABLE const uint IO_WriteError = 2;
static QT_COMPAT_VARIABLE const uint IO_FatalError = 3;
static QT_COMPAT_VARIABLE const uint IO_ResourceError = 4;
static QT_COMPAT_VARIABLE const uint IO_OpenError = 5;
static QT_COMPAT_VARIABLE const uint IO_ConnectError = 5;
static QT_COMPAT_VARIABLE const uint IO_AbortError = 6;
static QT_COMPAT_VARIABLE const uint IO_TimeOutError = 7;
static QT_COMPAT_VARIABLE const uint IO_UnspecifiedError	= 8;

inline QT_COMPAT int QIODevice::state() const
{
    return isOpen() ? 0x1000 : 0;
}
#endif

#endif // QIODEVICE_H

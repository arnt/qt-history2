/****************************************************************************
**
** Definition of QIODevice class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QIODEVICE_H
#define QIODEVICE_H

#ifndef QT_H
#include "qglobal.h"
#include "qstring.h"
#include "qobjectdefs.h"
#endif // QT_H

class QByteArray;
class QIODevicePrivate;

class Q_CORE_EXPORT QIODevice
{
    Q_DECLARE_PRIVATE(QIODevice)

public:
    typedef Q_LLONG Offset;

    enum AccessTypes {
        Direct = 0x0100,
        Sequential = 0x0200,
        Combined = Direct | Sequential,
        TypeMask = 0x0f00
    };

    enum HandlingModes {
        Raw = 0x0040,
        Async = 0x0080
    };

    enum OpenModes {
        ReadOnly = 0x0001,
        WriteOnly = 0x0002,
        ReadWrite = 0x0003,
        Append = 0x0004,
        Truncate = 0x0008,
        Translate = 0x0010,
        ModeMask = 0x00ff
    };

    enum State {
        Open = 0x1000,
        StateMask = 0xf000
    };

    enum Status {
        Ok = 0,
        ReadError = 1,
        WriteError = 2,
        FatalError = 3,
        ResourceError = 4,
        OpenError = 5,
        ConnectError = 6,
        AbortError = 7,
        TimeOutError = 8,
        UnspecifiedError = 9
    };

    QIODevice();
    virtual ~QIODevice();

    int flags() const;
    inline int mode() const { return flags() & ModeMask; }
    inline int state() const { return flags() & StateMask; }

    inline bool isDirectAccess() const { return (flags() & Direct) == Direct; }
    inline bool isSequentialAccess() const { return (flags() & Sequential) == Sequential; }
    inline bool isCombinedAccess() const { return (flags() & Combined) == Combined; }
    inline bool isBuffered() const { return (flags() & Raw) != Raw; }
    inline bool isRaw() const { return (flags() & Raw) == Raw; }
    inline bool isSynchronous() const { return (flags() & Async) != Async; }
    inline bool isAsynchronous() const { return (flags() & Async) == Async; }
    inline bool isTranslated() const { return (flags() & Translate) == Translate; }
    inline bool isReadable() const { return (flags() & ReadOnly) == ReadOnly; }
    inline bool isWritable() const { return (flags() & WriteOnly) == WriteOnly; }
    inline bool isReadWrite() const { return (flags() & ReadWrite) == ReadWrite; }
    inline bool isInactive() const { return state() == 0; }
    inline bool isOpen() const { return state() == Open; }

    int status() const;
    void resetStatus();
    QString errorString() const;

    virtual bool open(int mode) = 0;
    virtual void close() = 0;
    virtual void flush() = 0;

    virtual Offset size() const = 0;
    virtual Offset at() const;
    virtual bool at(Offset);
    virtual bool atEnd() const;
    inline bool reset() { return at(0); }

    virtual Q_LONG readBlock(char *data, Q_ULONG maxlen) = 0;
    virtual Q_LONG writeBlock(const char *data, Q_ULONG len) = 0;
    virtual Q_LONG readLine(char *data, Q_ULONG maxlen);
    Q_LONG writeBlock(const QByteArray &data);
    virtual QByteArray readAll();

    virtual int getch() = 0;
    virtual int putch(int) = 0;
    virtual int ungetch(int) = 0;

protected:
    QIODevice(QIODevicePrivate &d);

    void setFlags(int f);
    void setType(int);
    void setMode(int);
    void setState(int);
    void setStatus(int);
    void setStatus(int, const QString &errorString);
    void setStatus(int, int);

    Offset ioIndex;
    QIODevicePrivate *d_ptr;

private:
#if defined(Q_DISABLE_COPY)
    QIODevice(const QIODevice &);
    QIODevice &operator=(const QIODevice &);
#endif
};

// Compatibility defines

#define IO_Direct QIODevice::Direct
#define IO_Sequential QIODevice::Sequential
#define IO_Combined QIODevice::Combined
#define IO_TypeMask QIODevice::TypeMask

#define IO_Raw QIODevice::Raw
#define IO_Async QIODevice::Async

#define IO_ReadOnly QIODevice::ReadOnly
#define IO_WriteOnly QIODevice::WriteOnly
#define IO_ReadWrite QIODevice::ReadWrite
#define IO_Append QIODevice::Append
#define IO_Truncate QIODevice::Truncate
#define IO_Translate QIODevice::Translate
#define IO_ModeMask QIODevice::ModeMask

#define IO_Open QIODevice::Open
#define IO_StateMask QIODevice::StateMask

#define IO_Ok QIODevice::Ok
#define IO_ReadError QIODevice::ReadError
#define IO_WriteError QIODevice::WriteError
#define IO_FatalError QIODevice::FatalError
#define IO_ResourceError QIODevice::ResourceError
#define IO_OpenError QIODevice::OpenError
#define IO_ConnectError QIODevice::ConnectError
#define IO_AbortError QIODevice::AbortError
#define IO_TimeOutError QIODevice::TimeOutError
#define IO_UnspecifiedError QIODevice::UnspecifiedError

#endif // QIODEVICE_H

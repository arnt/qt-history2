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

#include "qglobal.h"
#include "qstring.h"
#include "qobjectdefs.h"

class QByteArray;
class QIODevicePrivate;

class Q_CORE_EXPORT QIODevice
{
    Q_DECLARE_PRIVATE(QIODevice)

public:
    typedef Q_LONGLONG Offset;

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
        UnspecifiedError = 9,
        RemoveError = 10,
        RenameError = 11,
        PositionError = 12,
        ResizeError = 13,
        PermissionsError = 14,
        CopyError = 15
    };

    QIODevice();
    virtual ~QIODevice();

    int flags() const;
    inline int mode() const { return flags() & ModeMask; }

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
    inline bool isInactive() const { return !isOpen(); }
    virtual bool isOpen() const = 0;

    int status() const;
    void resetStatus();
    QString errorString() const;

    virtual inline bool open(int) { return false; }
    virtual void close() = 0;
    virtual inline void flush() { }

    virtual Q_LONGLONG size() const = 0;
    virtual Q_LONGLONG at() const = 0;
    virtual bool seek(Q_LONGLONG off) = 0;
    virtual inline bool atEnd() const { return at() == size(); }
    inline bool reset() { return seek(0); }

    virtual Q_LONGLONG read(char *data, Q_LONGLONG maxlen) = 0;
    inline QByteArray read(Q_LONGLONG maxlen)
    { QByteArray ret; ret.resize(maxlen); Q_LONGLONG r = read(ret.data(), maxlen); ret.resize(r > 0 ? r : 0); return ret; }
    virtual Q_LONGLONG write(const char *data, Q_LONGLONG len) = 0;
    inline Q_LONGLONG write(const QByteArray &data) { return write(data.constData(), data.size()); }
    virtual Q_LONGLONG readLine(char *data, Q_LONGLONG maxlen);
    virtual QByteArray readLine();
    virtual QByteArray readAll();

#ifdef QT_COMPAT
    enum State {
        Open = 0x1000,
        StateMask = 0xf000
    };
    inline QT_COMPAT int state() const { return isOpen() ? Open : 0; }
    inline QT_COMPAT bool at(Q_LONGLONG off) { return seek(off); }
    inline QT_COMPAT Q_LONG readBlock(char *data, Q_LONG maxlen)
    { return read(data, maxlen); }
    inline QT_COMPAT Q_LONG writeBlock(const char *data, Q_LONG maxlen)
    { return write(data, maxlen); }
    inline QT_COMPAT Q_LONG writeBlock(const QByteArray &data)
    { return write(data); }
#endif

    virtual int getch();
    virtual int putch(int character);
    virtual int ungetch(int character) = 0;

protected:
    QIODevice(QIODevicePrivate &d);

    void setFlags(int);
    void setType(int);
    void setMode(int);
    void setBufferSize(int);

    void setStatus(int);
    void setStatus(int, const QString &errorString);
    void setStatus(int, int);

    QIODevicePrivate *d_ptr;

private:
    Q_DISABLE_COPY(QIODevice)
};

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
#if defined QT_COMPAT
#define IO_StateMask QIODevice::StateMask
#endif

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
#define IO_RemoveError QIODevice::RemoveError
#define IO_RenameError QIODevice::RenameError
#define IO_PositionError QIODevice::PositionError

#endif // QIODEVICE_H

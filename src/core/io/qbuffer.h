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

#ifndef QBUFFER_H
#define QBUFFER_H

#include "qiodevice.h"
#include "qbytearray.h"

class QBufferPrivate;

class Q_CORE_EXPORT QBuffer : public QIODevice
{
    Q_DECLARE_PRIVATE(QBuffer)
public:
    QBuffer();
    QBuffer(QByteArray *buf);
    ~QBuffer();

    virtual QIODevice::DeviceType deviceType() const { return castDeviceType(); }
    static QIODevice::DeviceType castDeviceType() { return QIODevice::IOBuffer; }

    QByteArray &buffer();
    const QByteArray &buffer() const;
    void setBuffer(QByteArray *a);
    void setData(const QByteArray &data);
    inline void setData(const char *data, int len) { setData(QByteArray(data, len)); }

#ifdef QT_COMPAT
#if !defined(Q_NO_USING_KEYWORD)
    using QIODevice::at;
#else
    inline QT_COMPAT bool at(Q_LONGLONG off) { return QIODevice::at(off); }
#endif
#endif

    bool isOpen() const;
    virtual bool open(int mode);
    virtual void close();
    virtual Q_LONGLONG size() const;
    virtual Q_LONGLONG at() const;
    virtual bool seek(Q_LONGLONG off);
    virtual Q_LONGLONG read(char *data, Q_LONGLONG maxlen);
    virtual Q_LONGLONG write(const char *data, Q_LONGLONG len);
    virtual int getch();
    virtual int putch(int character);
    virtual int ungetch(int character);

private:
    Q_DISABLE_COPY(QBuffer)
};

#endif // QBUFFER_H

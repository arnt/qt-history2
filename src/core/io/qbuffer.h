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

class QObject;
class QBufferPrivate;

class Q_CORE_EXPORT QBuffer : public QIODevice
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#endif

public:
    QBuffer();
    QBuffer(QByteArray *buf);
#ifndef QT_NO_QOBJECT
    QBuffer(QObject *parent);
    QBuffer(QByteArray *buf, QObject *parent);
#endif
    ~QBuffer();

    QByteArray &buffer();
    const QByteArray &buffer() const;
    void setBuffer(QByteArray *a);
    void setData(const QByteArray &data);
    inline void setData(const char *data, int len) { setData(QByteArray(data, len)); }

    bool open(OpenMode openMode);

    void close();
    Q_LONGLONG size() const;
    Q_LONGLONG pos() const;
    bool seek(Q_LONGLONG off);
    bool atEnd() const;

protected:
#ifdef QT_NO_QOBJECT
    QBuffer(QIODevicePrivate &dd);
#else
    QBuffer(QIODevicePrivate &dd, QObject *parent);
#endif

    Q_LONGLONG readData(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG writeData(const char *data, Q_LONGLONG len);

private:
    Q_DECLARE_PRIVATE(QBuffer)
    Q_DISABLE_COPY(QBuffer)

    Q_PRIVATE_SLOT(d, void emitSignals())
};

#endif // QBUFFER_H

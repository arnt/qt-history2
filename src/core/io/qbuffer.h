/****************************************************************************
**
** Definition of QBuffer class.
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

#ifndef QBUFFER_H
#define QBUFFER_H

#ifndef QT_H
#include "qiodevice.h"
#include "qbytearray.h"
#endif // QT_H

class QBufferPrivate;

class Q_CORE_EXPORT QBuffer : public QIODevice
{
    Q_DECLARE_PRIVATE(QBuffer)
public:
    QBuffer();
    QBuffer(QByteArray *buf);
    ~QBuffer();

    QByteArray &buffer();
    const QByteArray &buffer() const;
    void setBuffer(QByteArray *a);
    void setData(const QByteArray &data);
    inline void setData(const char *data, int len) { setData(QByteArray(data, len)); }

    virtual QIOEngine *ioEngine() const;

private:
#if defined(Q_DISABLE_COPY)
    QBuffer(const QBuffer &);
    QBuffer &operator=(const QBuffer &);
#endif
};

#endif // QBUFFER_H

/****************************************************************************
**
** Definition of QIOEngine class.
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

#ifndef QIOENGINE_H
#define QIOENGINE_H

#include "qiodevice.h"

class QIOEnginePrivate;
class Q_CORE_EXPORT QIOEngine
{
protected:
    QIOEnginePrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(QIOEngine)
public:
    virtual ~QIOEngine();

    enum Type {
        File,
        Resource,
        Socket,
        String,

        User = 50,                                // first user type id
        MaxUser = 100                                // last user type id
    };
    virtual Type type() const = 0;

    virtual bool open(int flags) = 0;
    virtual bool close() = 0;
    virtual void flush() = 0;

    virtual QIODevice::Offset size() const = 0;
    virtual QIODevice::Offset at() const = 0;
    virtual bool seek(QIODevice::Offset off) = 0;
    virtual bool atEnd() const;

    virtual bool isSequential() const = 0;

    virtual uchar *map(QIODevice::Offset off, Q_LONG len);
    virtual void unmap(uchar *data);

    virtual Q_LONG readBlock(char *data, Q_LONG maxlen) = 0;
    virtual Q_LONG writeBlock(const char *data, Q_LONG len) = 0;
    virtual QByteArray readAll();
    virtual Q_LONG readLine(char *data, Q_LONG maxlen);

    virtual int getch();
    virtual int putch(int);
    virtual int ungetch(int) = 0;

    virtual QIODevice::Status errorStatus() const;
    virtual QString errorString() const;

protected:
    QIOEngine();
    QIOEngine(QIOEnginePrivate &);
};

extern QString qt_errorstr(int errorCode);

#endif // QIOENGINE_H

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

#ifndef QSIGNAL_H
#define QSIGNAL_H

#ifndef QT_NO_QOBJECT

#include "QtCore/qobject.h"
#include "QtCore/qmetaobject.h"


// internal helper class for QSignal
class Q_CORE_EXPORT QSignalEmitter : public QObject
{
public:
    explicit QSignalEmitter(const char *type = 0);
    ~QSignalEmitter();
    const QMetaObject *metaObject() const;
    void *qt_metacast(const char *);
    int qt_metacall(QMetaObject::Call, int, void **);
    void activate(const void * = 0);
    bool connect(const QObject *receiver, const char *member, Qt::ConnectionType = Qt::AutoConnection);
    bool disconnect(const QObject *receiver, const char *member=0);
private:
    QMetaObject staticMetaObject;
    QByteArray stringdata;
};


template <typename T>
class QSignal
{
    QSignalEmitter *d;
public:
    inline QSignal() : d(0) {}
    inline ~QSignal() { delete d; }
    inline void activate(const T& t)
    { if(d) d->activate(static_cast<const void*>(&t)); }
    bool connect(const QObject *receiver, const char *member,
                  Qt::ConnectionType type = Qt::AutoConnection) {
        if (!d) d = new QSignalEmitter(QTypeInfo<T>::name());
        return d->connect(receiver, member, type);
    }
    inline bool disconnect(const QObject *receiver, const char *member=0)
        { return d ? d->disconnect(receiver, member) : false; }

private:
    Q_DISABLE_COPY(QSignal)
};

template <>
class QSignal<void>
{
    QSignalEmitter *d;
public:
    inline QSignal() : d(0) {}
    inline ~QSignal() { delete d; }
    inline void activate() { if(d)d->activate(); }
    bool connect(const QObject *receiver, const char *member,
                  Qt::ConnectionType type = Qt::AutoConnection) {
        if (!d) d = new QSignalEmitter;
        return d->connect(receiver, member, type);
    }
    inline bool disconnect(const QObject *receiver, const char *member=0)
        { return d ? d->disconnect(receiver, member) : false; }

private:
    Q_DISABLE_COPY(QSignal)
};

#endif

#endif // QSIGNAL_H

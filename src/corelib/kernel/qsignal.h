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
class Q_CORE_EXPORT QSignalEmitter : public QObject{
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

class Q_CORE_EXPORT QGenericArgument
{
public:
    inline QGenericArgument(const char *aName = 0, const void *aData = 0):
        _data(aData), _name(aName) {}
    inline void *data() const { return const_cast<void *>(_data); }
    inline const char *name() const { return _name; }

private:
    const void *_data;
    const char *_name;
};

class Q_CORE_EXPORT QGenericReturnArgument: public QGenericArgument
{
public:
    inline QGenericReturnArgument(const char *aName = 0, void *aData = 0):
        QGenericArgument(aName, aData)
    {}
};

template <class T>
class QArgument: public QGenericArgument
{
public:
    inline QArgument(const char *aName, const T &aData) :
                QGenericArgument(aName, static_cast<const void *>(&aData))
    {}
};


template<class T>
class QReturnArgument: public QGenericReturnArgument
{
public:
    inline QReturnArgument(const char *aName, T &aData) :
                QGenericReturnArgument(aName, static_cast<void *>(&aData))
    {}
};

#define Q_ARG(type, data) QArgument<type>(#type, data)
#define Q_RETURN_ARG(type, data) QReturnArgument<type>(#type, data)

/*! internal
*/
Q_CORE_EXPORT bool qInvokeMetaMember(QObject *obj, const char *member,
                 Qt::ConnectionType,
                 QGenericReturnArgument ret,
                 QGenericArgument val0 = QGenericArgument(0),
                 QGenericArgument val1 = QGenericArgument(),
                 QGenericArgument val2 = QGenericArgument(),
                 QGenericArgument val3 = QGenericArgument(),
                 QGenericArgument val4 = QGenericArgument(),
                 QGenericArgument val5 = QGenericArgument(),
                 QGenericArgument val6 = QGenericArgument(),
                 QGenericArgument val7 = QGenericArgument(),
                 QGenericArgument val8 = QGenericArgument(),
                 QGenericArgument val9 = QGenericArgument());

/*! internal
 */
inline Q_CORE_EXPORT bool qInvokeMetaMember(QObject *obj, const char *member,
                 QGenericReturnArgument ret,
                 QGenericArgument val0 = QGenericArgument(0),
                 QGenericArgument val1 = QGenericArgument(),
                 QGenericArgument val2 = QGenericArgument(),
                 QGenericArgument val3 = QGenericArgument(),
                 QGenericArgument val4 = QGenericArgument(),
                 QGenericArgument val5 = QGenericArgument(),
                 QGenericArgument val6 = QGenericArgument(),
                 QGenericArgument val7 = QGenericArgument(),
                 QGenericArgument val8 = QGenericArgument(),
                 QGenericArgument val9 = QGenericArgument())
{
    return qInvokeMetaMember(obj, member, Qt::AutoConnection, ret, val0, val1, val2, val3,
                             val4, val5, val6, val7, val8, val9);
}

/*! internal
 */
inline Q_CORE_EXPORT bool qInvokeMetaMember(QObject *obj, const char *member,
                 Qt::ConnectionType type,
                 QGenericArgument val0 = QGenericArgument(0),
                 QGenericArgument val1 = QGenericArgument(),
                 QGenericArgument val2 = QGenericArgument(),
                 QGenericArgument val3 = QGenericArgument(),
                 QGenericArgument val4 = QGenericArgument(),
                 QGenericArgument val5 = QGenericArgument(),
                 QGenericArgument val6 = QGenericArgument(),
                 QGenericArgument val7 = QGenericArgument(),
                 QGenericArgument val8 = QGenericArgument(),
                 QGenericArgument val9 = QGenericArgument())
{
    return qInvokeMetaMember(obj, member, type, QGenericReturnArgument(), val0, val1, val2, val3,
                             val4, val5, val6, val7, val8, val9);
}


/*! internal
 */
inline Q_CORE_EXPORT bool qInvokeMetaMember(QObject *obj, const char *member,
                 QGenericArgument val0 = QGenericArgument(0),
                 QGenericArgument val1 = QGenericArgument(),
                 QGenericArgument val2 = QGenericArgument(),
                 QGenericArgument val3 = QGenericArgument(),
                 QGenericArgument val4 = QGenericArgument(),
                 QGenericArgument val5 = QGenericArgument(),
                 QGenericArgument val6 = QGenericArgument(),
                 QGenericArgument val7 = QGenericArgument(),
                 QGenericArgument val8 = QGenericArgument(),
                 QGenericArgument val9 = QGenericArgument())
{
    return qInvokeMetaMember(obj, member, Qt::AutoConnection, QGenericReturnArgument(), val0,
                             val1, val2, val3, val4, val5, val6, val7, val8, val9);
}

#endif

#endif // QSIGNAL_H

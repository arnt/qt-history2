/****************************************************************************
**
** Definition of QSignal class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSIGNAL_H
#define QSIGNAL_H

#ifndef QT_H
#include "qobject.h"
#include "qmetaobject.h"
#endif // QT_H


// internal helper class for QSignal
class Q_CORE_EXPORT QSignalEmitter : public QObject{
public:
    QSignalEmitter(const char *type = 0);
    ~QSignalEmitter();
    const QMetaObject *metaObject() const { return &staticMetaObject; }
    void *qt_metacast(const char *) const;
    void activate(void * = 0);
    bool connect( const QObject *receiver, const char *member, ConnectionType = AutoConnection );
    bool disconnect( const QObject *receiver, const char *member=0 );
private:
    QMetaObject staticMetaObject;
    QByteArray stringdata;
};


template <typename T>
class QSignal
{
    QSignalEmitter *d;
public:
    inline QSignal():d(0){}
    inline ~QSignal(){ delete d; }
    inline void activate(const T& t) { if(d)d->activate((void*)&t); }
    bool connect( const QObject *receiver, const char *member,
		  Qt::ConnectionType type = Qt::AutoConnection ) {
	if (!d) d = new QSignalEmitter(QTypeInfo<T>::name());
	return d->connect(receiver, member, type);
    }
    inline bool disconnect( const QObject *receiver, const char *member=0 )
	{ return d ? d->disconnect(receiver, member) : false; }
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSignal( const QSignal & );
    QSignal &operator=( const QSignal & );
#endif
};

template <>
class QSignal<void>
{
    QSignalEmitter *d;
public:
    inline QSignal():d(0){}
    inline ~QSignal(){ delete d; }
    inline void activate() { if(d)d->activate(); }
    bool connect( const QObject *receiver, const char *member,
		  Qt::ConnectionType type = Qt::AutoConnection ) {
	if (!d) d = new QSignalEmitter;
	return d->connect(receiver, member, type);
    }
    inline bool disconnect( const QObject *receiver, const char *member=0 )
	{ return d ? d->disconnect(receiver, member) : false; }
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSignal( const QSignal & );
    QSignal &operator=( const QSignal & );
#endif
};

class Q_CORE_EXPORT QGenericArgument
{
public:
    inline QGenericArgument(const char *aName = 0, void *aData = 0):
        _data(aData), _name(aName)
    {}
    inline void *data() const
    { return _data; }
    inline const char *name() const
    { return _name; }

private:
    void *_data;
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
                QGenericArgument(aName, (void*)&aData)
    {}
};


template<class T>
class QReturnArgument: public QGenericReturnArgument
{
public:
    inline QReturnArgument(const char *aName, T &aData) :
                QGenericReturnArgument(aName, (void*)&aData)
    {}
};

#define Q_ARG(type, data) QArgument<type>(#type, data)
#define Q_RETURN_ARG(type, data) QReturnArgument<type>(#type, data)

/*! internal
*/
Q_CORE_EXPORT bool qInvokeSlot(QObject *obj, const char *slotName,
                 QGenericReturnArgument ret,
                 QGenericArgument val0 = QGenericArgument(),
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
Q_CORE_EXPORT bool qInvokeSlot(QObject *obj, const char *slotName,
                 QGenericArgument val0 = QGenericArgument(),
                 QGenericArgument val1 = QGenericArgument(),
                 QGenericArgument val2 = QGenericArgument(),
                 QGenericArgument val3 = QGenericArgument(),
                 QGenericArgument val4 = QGenericArgument(),
                 QGenericArgument val5 = QGenericArgument(),
                 QGenericArgument val6 = QGenericArgument(),
                 QGenericArgument val7 = QGenericArgument(),
                 QGenericArgument val8 = QGenericArgument(),
                 QGenericArgument val9 = QGenericArgument());

#endif // QSIGNAL_H

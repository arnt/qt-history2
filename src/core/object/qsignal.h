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


#endif // QSIGNAL_H

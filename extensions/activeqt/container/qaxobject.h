/****************************************************************************
**
** Copyright (C) 2001-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef UNICODE
#define UNICODE
#endif

#ifndef QAXOBJECT_H
#define QAXOBJECT_H

#include "qaxbase.h"

class QAxObject : public QObject, public QAxBase
{
    friend class QAxEventSink;
public:
    const QMetaObject *metaObject() const;
    void* qt_metacast( const char* ) const;
    int qt_metacall(QMetaObject::Call, int, void **);
    QObject* qObject() const { return (QObject*)this; }
    const char *className() const { return "QAxObject"; }

    QAxObject( QObject *parent = 0, const char *name = 0 );
    QAxObject( const QString &c, QObject *parent = 0, const char *name = 0 );
    QAxObject( IUnknown *iface, QObject *parent = 0, const char *name = 0 );
    ~QAxObject();

private:
    const QMetaObject *parentMetaObject() const;
};

#endif //QAXOBJECT_H

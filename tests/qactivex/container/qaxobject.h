/****************************************************************************
** $Id: $
**
** Declaration of the QComBase and QComObject classes
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef UNICODE
#define UNICODE
#endif

#ifndef QAXOBJECT_H
#define QAXOBJECT_H

#include "qaxbase.h"
#include <qobject.h>

class QAX_EXPORT QAxObject : public QObject, public QAxBase
{
    friend class QAxEventSink;
public:
    QMetaObject *metaObject() const;
    const char *className() const;
    void* qt_cast( const char* );
    bool qt_invoke( int, QUObject* );
    bool qt_emit( int, QUObject* );
    bool qt_property( int, int, QVariant* );
    QObject* qObject() { return (QObject*)this; }

    QAxObject( QObject *parent = 0, const char *name = 0 );
    QAxObject( const QString &c, QObject *parent = 0, const char *name = 0 );
    QAxObject( IUnknown *iface, QObject *parent = 0, const char *name = 0 );
    ~QAxObject();

private:
    bool initialize( IUnknown** );
    QMetaObject *parentMetaObject() const;
};

#endif //QAXOBJECT_H

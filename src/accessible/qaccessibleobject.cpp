/****************************************************************************
**
** Implementation of the QAccessibleObject class.
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

#include "qaccessibleobject.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qobject.h"

class QAccessibleObjectPrivate
{
public:
    QPointer<QObject> object;
};

/*!
    \class QAccessibleObject qaccessible.h
    \brief The QAccessibleObject class implements parts of the
    QAccessibleInterface for QObjects.

    \ingroup misc

    This class is mainly provided for convenience. All subclasses of
    the QAccessibleInterface should use this class as the base class.
*/

extern void qInsertAccessibleObject(QObject *object, QAccessibleInterface *iface);
extern void qRemoveAccessibleObject(QObject *object);

/*!
    Creates a QAccessibleObject for \a object.
*/
QAccessibleObject::QAccessibleObject( QObject *object )
{
    d = new QAccessibleObjectPrivate;
    d->object = object;

    qInsertAccessibleObject(object, this);
}

/*!
    Destroys the QAccessibleObject.

    This only happens when a call to release() decrements the internal
    reference counter to zero.
*/
QAccessibleObject::~QAccessibleObject()
{
    qRemoveAccessibleObject(d->object);

    delete d;
}

/*!
    \reimp
*/
QRESULT QAccessibleObject::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QAccessible )
	*iface = (QAccessibleInterface*)this;
    else if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

/*!
    Returns the QObject for which this QAccessibleInterface
    implementation provides information. Use isValid() to make sure
    the object pointer is safe to use.
*/
QObject *QAccessibleObject::object() const
{
#ifndef QT_NO_DEBUG
    if ( !isValid() )
	qWarning( "QAccessibleInterface is invalid. Crash pending..." );
#endif
    return d->object;
}

/*!
    \reimp
*/
bool QAccessibleObject::isValid() const
{
    return !d->object.isNull();
}

#endif //QT_ACCESSIBILITY_SUPPORT

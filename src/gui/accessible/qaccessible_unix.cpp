/****************************************************************************
**
** Implementation of QAccessible class for Unix.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qlibrary.h"

#include <stdlib.h>

typedef void(*QSetRootObject)(QAccessibleInterface *);
static QSetRootObject qSetRootObject = 0;
typedef void(*QNotifyAccessibilityUpdate)(QAccessible::Event, QAccessibleInterface*, int);
static QNotifyAccessibilityUpdate qNotifyAccessibilityUpdate = 0;

void QAccessible::initialize()
{
    if (qstrcmp(getenv("QT_ACCESSIBILITY"), "1") != 0)
        return;

    if (!qSetRootObject)
        qSetRootObject = (QSetRootObject)QLibrary::resolve("qaccclient", "qSetRootObject");
    if (!qNotifyAccessibilityUpdate)
        qNotifyAccessibilityUpdate = (QNotifyAccessibilityUpdate)QLibrary::resolve("qaccclient", "qNotifyAccessibilityUpdate");
}

void QAccessible::cleanup()
{
}

void QAccessible::updateAccessibility(QObject *o, int who, Event reason)
{
    Q_ASSERT(o);

    if (updateHandler) {
        updateHandler(o, who, reason);
        return;
    }

    initialize();
    if (!qNotifyAccessibilityUpdate)
        return;

    QAccessibleInterface *iface = 0;
    QAccessible::queryAccessibleInterface(o, &iface);
    if (iface)
        qNotifyAccessibilityUpdate(reason, iface, who);
}

void QAccessible::setRootObject(QObject *o)
{
    if (rootObjectHandler) {
        rootObjectHandler(o);
        return;
    }

    initialize();
    if (!qSetRootObject)
        return;

    QAccessibleInterface *iface = 0;
    if (o)
        QAccessible::queryAccessibleInterface(o, &iface);
    qSetRootObject(iface);
    iface->release();
}

#endif

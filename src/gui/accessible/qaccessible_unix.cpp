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

#include "qaccessible.h"
#include "qaccessiblebridge.h"

#ifndef QT_NO_ACCESSIBILITY

#include "qcoreapplication.h"
#include "qmutex.h"
#include "qvector.h"
#include "private/qfactoryloader_p.h"

#include <stdlib.h>

#ifndef QT_NO_COMPONENT
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QAccessibleBridgeFactoryInterface_iid, QCoreApplication::libraryPaths(), "/accessiblebridge"))
#endif
Q_GLOBAL_STATIC(QVector<QAccessibleBridge *>, bridges)
static bool isInit = false;

void QAccessible::initialize()
{
    if (isInit)
        return;
    isInit = true;

    if (qstrcmp(qgetenv("QT_ACCESSIBILITY"), "1") != 0)
        return;
#ifndef QT_NO_COMPONENT
    const QStringList l = loader()->keys();
    for (int i = 0; i < l.count(); ++i) {
        if (QAccessibleBridgeFactoryInterface *factory =
                qt_cast<QAccessibleBridgeFactoryInterface*>(loader()->instance(l.at(i)))) {
            QAccessibleBridge * bridge = factory->create(l.at(i));
            if (bridge)
                bridges()->append(bridge);
        }
    }
#endif
}

void QAccessible::cleanup()
{
    qDeleteAll(*bridges());
}

void QAccessible::updateAccessibility(QObject *o, int who, Event reason)
{
    Q_ASSERT(o);

    if (updateHandler) {
        updateHandler(o, who, reason);
        return;
    }

    initialize();
    if (bridges()->isEmpty())
        return;

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(o);
    if (!iface)
        return;

    for (int i = 0; i < bridges()->count(); ++i)
        bridges()->at(i)->notifyAccessibilityUpdate(reason, iface, who);
    delete iface;
}

void QAccessible::setRootObject(QObject *o)
{
    if (rootObjectHandler) {
        rootObjectHandler(o);
        return;
    }

    initialize();
    if (bridges()->isEmpty())
        return;

    if (!o)
        return;

    for (int i = 0; i < bridges()->count(); ++i) {
        QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(o);
        bridges()->at(i)->setRootObject(iface);
    }
}

#endif

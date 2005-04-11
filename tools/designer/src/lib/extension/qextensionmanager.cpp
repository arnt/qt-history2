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

#include "qextensionmanager.h"

QExtensionManager::QExtensionManager(QObject *parent)
    : QObject(parent)
{
}

void QExtensionManager::registerExtensions(QAbstractExtensionFactory *factory, const QString &iid)
{
    if (iid.isEmpty()) {
        m_globalExtension.append(factory);
        return;
    }

    if (!m_extensions.contains(iid))
        m_extensions.insert(iid, QList<QAbstractExtensionFactory*>());

    m_extensions[iid].prepend(factory);
}

void QExtensionManager::unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid)
{
    if (iid.isEmpty()) {
        m_globalExtension.removeAll(factory);
    } else if (m_extensions.contains(iid)) {
        QList<QAbstractExtensionFactory*> &factories = m_extensions[iid];
        factories.removeAll(factory);

        if (factories.isEmpty())
            m_extensions.remove(iid);
    }
}

QObject *QExtensionManager::extension(QObject *object, const QString &iid) const
{
    QList<QAbstractExtensionFactory*> l = m_extensions.value(iid);
    l += m_globalExtension;

    foreach (QAbstractExtensionFactory *factory, l) {
        if (QObject *ext = factory->extension(object, iid))
            return ext;
    }

    return 0;
}

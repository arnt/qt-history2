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

void QExtensionManager::registerExtensions(ExtensionFactory *factory, const QString &iid)
{
    m_extensions.insert(iid, factory);
}

void QExtensionManager::unregisterExtensions(ExtensionFactory *factory, const QString &iid)
{
    Q_UNUSED(factory);
    m_extensions.remove(iid);
}

QObject *QExtensionManager::extension(QObject *object, const QString &iid) const
{
    QList<ExtensionFactory*> l = m_extensions.values(iid);
    foreach (ExtensionFactory *factory, l) {
        if (QObject *ext = factory->extension(object, iid))
            return ext;
    }

    return 0;
}

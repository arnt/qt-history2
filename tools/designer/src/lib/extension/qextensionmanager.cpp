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

/*!
    \class QExtensionManager
    \brief The QExtensionManager class provides extension management facilities for \QD.
    \inmodule QtDesigner
*/

/*!
    Constructs an extenstion manager with the given \a parent.*/
QExtensionManager::QExtensionManager(QObject *parent)
    : QObject(parent)
{
}

/*!
    Register \a factory as an extension factory with an identifier specified by \a iid.*/
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

/*!
    Unregister the \a factory with the identifier specified by \a iid.*/
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

/*!
    Returns the extension for the given \a object with the identifier specified by \a iid.*/
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

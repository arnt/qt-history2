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

#include <QtDesigner/default_extensionfactory.h>
#include "qextensionmanager.h"
#include <qpointer.h>
#include <QtCore/qdebug.h>

/*!
    \class QExtensionFactory
    \brief The QExtensionFactory class provides a standard extension factory for Qt Designer.
    \inmodule QtDesigner



    \sa QExtensionManager, QAbstractExtensionFactory
*/

/*!
    Constructs an extension factory with the given \a parent.
*/
QExtensionFactory::QExtensionFactory(QExtensionManager *parent)
    : QObject(parent)
{
}

/*!
    Returns the extension specified by \a iid for the given \a object.

    \sa createExtension()
*/

QObject *QExtensionFactory::extension(QObject *object, const QString &iid) const
{
    if (!object)
        return 0;

    QPair<QString, QObject*> key = qMakePair(iid, object);
    if (!m_extensions.contains(key)) {
        if (QObject *ext = createExtension(object, iid, const_cast<QExtensionFactory*>(this))) {
            connect(ext, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
            m_extensions.insert(key, ext);
        }
    }

    if (!m_extended.contains(object)) {
        connect(object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
        m_extended.insert(object, true);
    }

    return m_extensions.value(key);
}

void QExtensionFactory::objectDestroyed(QObject *object)
{
    QMutableMapIterator< QPair<QString,QObject*>, QObject*> it(m_extensions);
    while (it.hasNext()) {
        it.next();

        QObject *o = it.key().second;
        if (o == object || object == it.value()) {
            it.remove();
        }
    }

    m_extended.remove(object);
}

/*!
    Returns an extension specified by \a iid for the given \a object.
    The extension object is created as a child of the specified \a parent.

    \sa extension()
*/
QObject *QExtensionFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    Q_UNUSED(object);
    Q_UNUSED(iid);
    Q_UNUSED(parent);

    return 0;
}

/*!
    Returns the extension manager for the extension factory.
*/
QExtensionManager *QExtensionFactory::extensionManager() const
{
    return static_cast<QExtensionManager *>(parent());
}

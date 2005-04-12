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

#ifndef QEXTENSIONMANAGER_H
#define QEXTENSIONMANAGER_H

#include <QtDesigner/extension_global.h>
#include <QtDesigner/extension.h>
#include <QtCore/QHash>

class QT_EXTENSION_EXPORT QExtensionManager: public QObject, public QAbstractExtensionManager
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionManager)
public:
    QExtensionManager(QObject *parent = 0);

    virtual void registerExtensions(QAbstractExtensionFactory *factory, const QString &iid = QString());
    virtual void unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid = QString());

    virtual QObject *extension(QObject *object, const QString &iid) const;

private:
    QHash<QString, QList<QAbstractExtensionFactory*> > m_extensions;
    QList<QAbstractExtensionFactory*> m_globalExtension;
};

#endif // QEXTENSIONMANAGER_H

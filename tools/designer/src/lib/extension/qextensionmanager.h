/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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

QT_BEGIN_HEADER

class QDESIGNER_EXTENSION_EXPORT QExtensionManager: public QObject, public QAbstractExtensionManager
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionManager)
public:
    QExtensionManager(QObject *parent = 0);
    ~QExtensionManager();

    virtual void registerExtensions(QAbstractExtensionFactory *factory, const QString &iid = QString());
    virtual void unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid = QString());

    virtual QObject *extension(QObject *object, const QString &iid) const;

private:
    typedef QList<QAbstractExtensionFactory*> FactoryList;
    typedef QHash<QString, FactoryList> FactoryMap;
    FactoryMap m_extensions;
    FactoryList m_globalExtension;
};

QT_END_HEADER

#endif // QEXTENSIONMANAGER_H

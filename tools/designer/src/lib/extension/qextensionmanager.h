#ifndef QEXTENSIONMANAGER_H
#define QEXTENSIONMANAGER_H

#include "extension_global.h"
#include "extension.h"
#include <QHash>

class QT_EXTENSION_EXPORT QExtensionManager: public QObject, public ExtensionManager
{
    Q_OBJECT
    Q_INTERFACES(ExtensionManager)
public:
    QExtensionManager(QObject *parent = 0);

    virtual void registerExtensions(ExtensionFactory *factory, const QString &iid);
    virtual void unregisterExtensions(ExtensionFactory *factory, const QString &iid);

    virtual QObject *extension(QObject *object, const QString &iid) const;

private:
    QMultiHash<QString, ExtensionFactory*> m_extensions;
};

#endif // QEXTENSIONMANAGER_H


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



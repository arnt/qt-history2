#ifndef DEFAULT_EXTENSIONFACTORY_H
#define DEFAULT_EXTENSIONFACTORY_H

#include "extension.h"

#include <QMap>
#include <QHash>
#include <qpair.h>

class QExtensionManager;

class DefaultExtensionFactory : public QObject, public ExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(ExtensionFactory)
public:
    DefaultExtensionFactory(QExtensionManager *parent = 0);

    virtual QObject *extension(QObject *object, const QString &iid) const;
    QExtensionManager *extensionManager() const;

private slots:
    void objectDestroyed(QObject *object);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;

private:
    mutable QMap< QPair<QString,QObject*>, QObject*> m_extensions;
    mutable QHash<QObject*, bool> m_extended;
};

#endif // DEFAULT_EXTENSIONFACTORY_H

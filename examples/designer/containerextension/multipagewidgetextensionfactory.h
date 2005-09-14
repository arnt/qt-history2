#ifndef MULTIPAGEWIDGETEXTENSIONFACTORY_H
#define MULTIPAGEWIDGETEXTENSIONFACTORY_H

#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QExtensionFactory>

class QExtensionManager;

class MultiPageWidgetExtensionFactory: public QExtensionFactory
{
    Q_OBJECT

public:
    MultiPageWidgetExtensionFactory(QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif

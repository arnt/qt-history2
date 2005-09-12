#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/ui4.h>

#include "multipagewidgetextensionfactory.h"
#include "multipagewidgetcontainerextension.h"
#include "multipagewidgettaskmenuextension.h"
#include "multipagewidget.h"

MultiPageWidgetExtensionFactory::MultiPageWidgetExtensionFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{}

QObject *MultiPageWidgetExtensionFactory::createExtension(QObject *object,
                                                          const QString &iid,
                                                          QObject *parent) const
{
    MultiPageWidget *widget = qobject_cast<MultiPageWidget*>(object);

    if (widget && (iid == Q_TYPEID(QDesignerTaskMenuExtension))) {
        return new MultiPageWidgetTaskMenuExtension(widget, parent);
    } else if (widget && (iid == Q_TYPEID(QDesignerContainerExtension))) {
        return new MultiPageWidgetContainerExtension(widget, parent);
    } else {
        return 0;
    }
}

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>

#include <QIcon>

#include "multipagewidget.h"
#include "multipagewidgetplugin.h"
#include "multipagewidgetextensionfactory.h"
#include "qplugin.h"

MultiPageWidgetPlugin::MultiPageWidgetPlugin(QObject *parent)
    :QObject(parent)
{
    initialized = false;
}

QString MultiPageWidgetPlugin::name() const
{
    return QString("MultiPageWidget");
}

QString MultiPageWidgetPlugin::group() const
{
    return QString("Display Widgets [Examples]");
}

QString MultiPageWidgetPlugin::toolTip() const
{
    return "";
}

QString MultiPageWidgetPlugin::whatsThis() const
{
    return "";
}

QString MultiPageWidgetPlugin::includeFile() const
{
    return QString("multipagewidget.h");
}

QIcon MultiPageWidgetPlugin::icon() const
{
    return QIcon();
}

bool MultiPageWidgetPlugin::isContainer() const
{
    return true;
}

QWidget *MultiPageWidgetPlugin::createWidget(QWidget *parent)
{
    MultiPageWidget *widget = new MultiPageWidget(parent);
    return widget;
}

bool MultiPageWidgetPlugin::isInitialized() const
{
    return initialized;
}

void MultiPageWidgetPlugin::initialize(QDesignerFormEditorInterface *formEditor)
{
    if (initialized)
        return;

    QExtensionManager *manager = formEditor->extensionManager();
    Q_ASSERT(manager != 0);

    QExtensionFactory *factory = new MultiPageWidgetExtensionFactory(manager);

    manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));
    manager->registerExtensions(factory, Q_TYPEID(QDesignerTaskMenuExtension));

    initialized = true;
}

QString MultiPageWidgetPlugin::domXml() const
{
    return QString("\
    <widget class=\"MultiPageWidget\" name=\"multipagewidget\">\
        <widget class=\"QWidget\" />\
    </widget>\
    ");
}

Q_EXPORT_PLUGIN(MultiPageWidgetPlugin)

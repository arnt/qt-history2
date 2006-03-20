/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerContainerExtension>

#include <QIcon>
#include <QtPlugin>

#include "multipagewidget.h"
#include "multipagewidgetplugin.h"
#include "multipagewidgetextensionfactory.h"

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
    QExtensionFactory *factory = new MultiPageWidgetExtensionFactory(manager);

    Q_ASSERT(manager != 0);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));

    initialized = true;
}

QString MultiPageWidgetPlugin::domXml() const
{
    return QString("\
    <widget class=\"MultiPageWidget\" name=\"multipagewidget\">\
        <widget class=\"QWidget\" name=\"page\" />\
    </widget>\
    ");
}

Q_EXPORT_PLUGIN2(containerextension, MultiPageWidgetPlugin)

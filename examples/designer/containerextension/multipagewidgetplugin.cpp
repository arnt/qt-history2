/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerPropertySheetExtension>

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
    return QLatin1String("MultiPageWidget");
}

QString MultiPageWidgetPlugin::group() const
{
    return QLatin1String("Display Widgets [Examples]");
}

QString MultiPageWidgetPlugin::toolTip() const
{
    return QString();
}

QString MultiPageWidgetPlugin::whatsThis() const
{
    return QString();
}

QString MultiPageWidgetPlugin::includeFile() const
{
    return QLatin1String("multipagewidget.h");
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
    connect(widget, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChanged(int)));
    connect(widget, SIGNAL(pageTitleChanged(const QString &)),
            this, SLOT(pageTitleChanged(const QString &)));
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
    return QLatin1String("\
    <widget class=\"MultiPageWidget\" name=\"multipagewidget\">\
        <widget class=\"QWidget\" name=\"page\" />\
    </widget>\
    ");
}

void MultiPageWidgetPlugin::currentIndexChanged(int index)
{
    Q_UNUSED(index);
    MultiPageWidget *widget = qobject_cast<MultiPageWidget*>(sender());
    if (widget) {
        QDesignerFormWindowInterface *form = QDesignerFormWindowInterface::findFormWindow(widget);
        if (form)
            form->emitSelectionChanged();
    }
}

void MultiPageWidgetPlugin::pageTitleChanged(const QString &title)
{
    Q_UNUSED(title);
    MultiPageWidget *widget = qobject_cast<MultiPageWidget*>(sender());
    if (widget) {
        QWidget *page = widget->widget(widget->currentIndex());
        QDesignerFormWindowInterface *form;
        form = QDesignerFormWindowInterface::findFormWindow(widget);
        if (form) {
            QDesignerFormEditorInterface *editor = form->core();
            QExtensionManager *manager = editor->extensionManager();
            QDesignerPropertySheetExtension *sheet;
            sheet = qt_extension<QDesignerPropertySheetExtension*>(manager, page);
            const int propertyIndex = sheet->indexOf(QLatin1String("windowTitle"));
            sheet->setChanged(propertyIndex, true);
        }
    }
}

QString MultiPageWidgetPlugin::codeTemplate() const
{
    return QLatin1String("\
    var i;\n\
    for (i = 0; i < childWidgets.length ; i++)\n\
        widget.addPage(childWidgets[i]);\n");
}

Q_EXPORT_PLUGIN2(containerextension, MultiPageWidgetPlugin)

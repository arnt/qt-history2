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

#include "qdesigner_formbuilder.h"
#include "qdesigner_widget.h"

// sdk
#include <QtDesigner/extrainfo.h>
#include <QtDesigner/container.h>
#include <QtDesigner/customwidget.h>
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstracticoncache.h>
#include <QtDesigner/ui4.h>

// shared
#include <resourcefile.h>
#include <pluginmanager.h>

#include <QtGui/QWidget>

#include <QtCore/QBuffer>
#include <QtCore/qdebug.h>

QDesignerFormBuilder::QDesignerFormBuilder(QDesignerFormEditorInterface *core)
    : m_core(core)
{
    Q_ASSERT(m_core);

    PluginManager *pluginManager = m_core->pluginManager();

    m_customFactory.clear();
    QStringList plugins = pluginManager->registeredPlugins();

    foreach (QString plugin, plugins) {
        QObject *o = pluginManager->instance(plugin);

        if (QDesignerCustomWidgetInterface *c = qobject_cast<QDesignerCustomWidgetInterface*>(o)) {
            if (!c->isInitialized())
                c->initialize(m_core);

            m_customFactory.insert(c->name(), c);
        }
    }
}

QWidget *QDesignerFormBuilder::createWidgetFromContents(const QString &contents, QWidget *parentWidget)
{
    QByteArray data = contents.toUtf8();
    QBuffer buffer(&data);
    return load(&buffer, parentWidget);
}


QWidget *QDesignerFormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    if (QDesignerCustomWidgetInterface *c = m_customFactory.value(widgetName)) {
        QWidget *widget = c->createWidget(parentWidget);
        widget->setObjectName(name);
        return widget;
    }

    if (widgetName == QLatin1String("QLabel")) {
        QLabel *label = new QDesignerLabel(parentWidget);
        label->setObjectName(name);
        return label;
    } else if (widgetName == QLatin1String("Line")) {
        Line *line = new Line(parentWidget);
        line->setObjectName(name);
        return line;
    }

    QWidget *widget = QFormBuilder::createWidget(widgetName, parentWidget, name);
    if (widget == 0) {
        // ### qWarning("failed to create a widget for type %s", widgetName.toUtf8().constData());
        widget = new QWidget(parentWidget);
        widget->setObjectName(name);
    }

    return widget;
}

bool QDesignerFormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    if (QFormBuilder::addItem(ui_widget, widget, parentWidget))
        return true;

    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(m_core->extensionManager(), parentWidget)) {
        container->addWidget(widget);
        return true;
    }

    return false;
}

bool QDesignerFormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    return QFormBuilder::addItem(ui_item, item, layout);
}

QIcon QDesignerFormBuilder::nameToIcon(const QString &filePath, const QString &qrcPath)
{
    return QIcon(core()->iconCache()->resolveQrcPath(filePath, qrcPath, workingDirectory()));
}

QPixmap QDesignerFormBuilder::nameToPixmap(const QString &filePath, const QString &qrcPath)
{
    return QPixmap(core()->iconCache()->resolveQrcPath(filePath, qrcPath, workingDirectory()));
}

void QDesignerFormBuilder::applyProperties(QObject *o, const QList<DomProperty*> &properties)
{
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), o);
    Q_ASSERT(sheet != 0);

    foreach (DomProperty *p, properties) {
        QVariant v = toVariant(o->metaObject(), p);
        if (!v.isNull())
            sheet->setProperty(sheet->indexOf(p->attributeName()), v);
    }
}

DomWidget *QDesignerFormBuilder::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
    DomWidget *ui_widget = QFormBuilder::createDom(widget, ui_parentWidget, recursive);

    if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension*>(m_core->extensionManager(), widget)) {
        extra->saveWidgetExtraInfo(ui_widget);
    }

    return ui_widget;
}

QWidget *QDesignerFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    QWidget *widget = QFormBuilder::create(ui_widget, parentWidget);

    if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension*>(m_core->extensionManager(), widget)) {
        extra->loadWidgetExtraInfo(ui_widget);
    }

    return widget;
}


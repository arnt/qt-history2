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

#include "qdesigner_formbuilder_p.h"
#include "qdesigner_widget_p.h"
#include "dynamicpropertysheet.h"
#include "qsimpleresource_p.h"

// sdk
#include <QtDesigner/extrainfo.h>
#include <QtDesigner/container.h>
#include <QtDesigner/customwidget.h>
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/abstracticoncache.h>
#include "ui4_p.h"

// shared
#include <resourcefile_p.h>
#include <widgetfactory_p.h>
#include <pluginmanager_p.h>

#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QMenuBar>
#include <QtGui/QMainWindow>
#include <QtGui/QStyleFactory>
#include <QtGui/QStyle>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include <QtCore/QBuffer>
#include <QtCore/qdebug.h>

namespace qdesigner_internal {

QDesignerFormBuilder::QDesignerFormBuilder(QDesignerFormEditorInterface *core)
    : m_core(core)
{
    Q_ASSERT(m_core);
}

QWidget *QDesignerFormBuilder::createWidgetFromContents(const QString &contents, QWidget *parentWidget)
{
    QByteArray data = contents.toUtf8();
    QBuffer buffer(&data);
    return load(&buffer, parentWidget);
}


QWidget *QDesignerFormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    QWidget *widget = 0;

    if (widgetName == QLatin1String("QToolBar")) {
        widget = new QToolBar(parentWidget);
    } else if (widgetName == QLatin1String("QMenu")) {
        widget = new QMenu(parentWidget);
    } else if (widgetName == QLatin1String("QMenuBar")) {
        widget = new QMenuBar(parentWidget);
    } else {
        widget = core()->widgetFactory()->createWidget(widgetName, parentWidget);
    }

    if (widget)
        widget->setObjectName(name);

    return widget;
}

bool QDesignerFormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    if (! QFormBuilder::addItem(ui_widget, widget, parentWidget) || qobject_cast<QMainWindow*> (parentWidget)) {
        if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(m_core->extensionManager(), parentWidget))
            container->addWidget(widget);
    }

    return true;
}

bool QDesignerFormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    return QFormBuilder::addItem(ui_item, item, layout);
}

QIcon QDesignerFormBuilder::nameToIcon(const QString &filePath, const QString &qrcPath)
{
    return QIcon(core()->iconCache()->resolveQrcPath(filePath, qrcPath, workingDirectory().absolutePath()));
}

QPixmap QDesignerFormBuilder::nameToPixmap(const QString &filePath, const QString &qrcPath)
{
    return QPixmap(core()->iconCache()->resolveQrcPath(filePath, qrcPath, workingDirectory().absolutePath()));
}

void QDesignerFormBuilder::applyProperties(QObject *o, const QList<DomProperty*> &properties)
{
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), o);
    QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension*>(core()->extensionManager(), o);
    Q_ASSERT(sheet != 0);

    const QMetaObject *meta = o->metaObject();

    foreach (DomProperty *p, properties) {
        QVariant v = toVariant(meta, p);

        if (v.isNull())
            continue;

        QByteArray pname = p->attributeName().toUtf8();
        int index = o->metaObject()->indexOfProperty(pname);

        if (index != -1) {
            // a real property
            o->setProperty(pname, v);
        } else if (strcmp(meta->className(), "QAxWidget") != 0) {
            // a fake property (but we have have to ignore QAxWidget)
            index = sheet->indexOf(p->attributeName());
            sheet->setProperty(index, v);
        } else if (dynamicSheet && dynamicSheet->dynamicPropertiesAllowed()) {
            o->setProperty(pname, v);
        }
    }
}

DomWidget *QDesignerFormBuilder::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
    DomWidget *ui_widget = QFormBuilder::createDom(widget, ui_parentWidget, recursive);
    QSimpleResource::addExtensionDataToDOM(m_core, ui_widget, widget);
    return ui_widget;
}

QWidget *QDesignerFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    QWidget *widget = QFormBuilder::create(ui_widget, parentWidget);
    QSimpleResource::applyExtensionDataFromDOM(m_core, ui_widget, widget);
    return widget;
}

QLayout *QDesignerFormBuilder::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
    return QFormBuilder::create(ui_layout, layout, parentWidget);
}

void QDesignerFormBuilder::loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    QFormBuilder::loadExtraInfo(ui_widget, widget, parentWidget);
}

QWidget *QDesignerFormBuilder::createPreview(const QDesignerFormWindowInterface *fw,
                                             const QString &styleName,
                                             QString *errorMessage)
{
    // style
    QStyle *style = 0;
    if (!styleName.isEmpty()) {
        style = QStyleFactory::create(styleName);
        if (!style) {
            *errorMessage = QObject::tr("The style %1 could not be loaded.").arg(styleName);
            return 0;
        }
    }
          
    // load
    QDesignerFormBuilder builder(fw->core());
    builder.setWorkingDirectory(fw->absoluteDir());

    QByteArray bytes = fw->contents().toUtf8();
    QBuffer buffer(&bytes);


    QWidget *widget = builder.load(&buffer, 0);
    if (!widget) { // Shouldn't happen
        *errorMessage = QObject::tr("The preview failed to build.");
        return  0;
    }

    // Apply style stored in action if any
    if (style) {
        style->setParent(widget);
        widget->setStyle(style);
        if (style->metaObject()->className() != QApplication::style()->metaObject()->className())
            widget->setPalette(style->standardPalette());
        
        const QList<QWidget*> lst = qFindChildren<QWidget*>(widget);
        foreach (QWidget *w, lst)
            w->setStyle(style);
    }
    widget->setWindowTitle(QObject::tr("%1 - [Preview]").arg(widget->windowTitle()));
    return widget;
}
    
QWidget *QDesignerFormBuilder::createPreview(const QDesignerFormWindowInterface *fw, const QString &styleName) 
{
    QString errorMessage;
    QWidget *widget = createPreview(fw, styleName, &errorMessage);
    if (!widget) {
        QMessageBox::critical(fw->core()->topLevel(), QObject::tr("Designer"), errorMessage);
        return 0;
    }    
    return widget;
}
} // namespace qdesigner_internal

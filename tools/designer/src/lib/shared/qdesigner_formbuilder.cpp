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
#include "dynamicpropertysheet.h"
#include "qsimpleresource_p.h"

#include <ui4_p.h>
#include <formbuilderextra_p.h>
// sdk
#include <QtDesigner/container.h>
#include <QtDesigner/customwidget.h>
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerWidgetFactoryInterface>
#include <QtDesigner/QDesignerCustomWidgetInterface>
#include <QtDesigner/abstracticoncache.h>
#include <abstractdialoggui_p.h>

// shared
#include <resourcefile_p.h>
#include <scripterrordialog_p.h>

#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QMenuBar>
#include <QtGui/QMainWindow>
#include <QtGui/QStyleFactory>
#include <QtGui/QStyle>
#include <QtGui/QApplication>
#include <QtGui/QAbstractScrollArea>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>

#include <QtCore/QBuffer>
#include <QtCore/qdebug.h>

#include <qdesigner_utils_p.h>

QT_BEGIN_NAMESPACE

static QString summarizeScriptErrors(const QFormScriptRunner::Errors &errors) 
{
    QString rc =  QObject::tr("Script errors occurred:");
    foreach (QFormScriptRunner::Error error, errors) {
        rc += QLatin1Char('\n');
        rc += error.errorMessage;
    }
    return rc;
}
 
namespace qdesigner_internal {

QDesignerFormBuilder::QDesignerFormBuilder(QDesignerFormEditorInterface *core, Mode mode) : 
    m_core(core),
    m_mode(mode)
{
    Q_ASSERT(m_core);
    // Disable scripting in the editors.
    QFormScriptRunner::Options options = formScriptRunner()->options();
    switch (m_mode) {
    case DisableScripts:
        options |= QFormScriptRunner::DisableScripts;
        break;
    case UseScriptAndContainerExtension:
    case UseScriptForContainerExtension:
        options |= QFormScriptRunner::DisableWarnings;
        options &= ~QFormScriptRunner::DisableScripts;
        break;
    }
    formScriptRunner()-> setOptions(options);
}

QWidget *QDesignerFormBuilder::createWidgetFromContents(const QString &contents, QWidget *parentWidget)
{
    QByteArray data = contents.toUtf8();
    QBuffer buffer(&data);
    return load(&buffer, parentWidget);
}

QWidget *QDesignerFormBuilder::create(DomUI *ui, QWidget *parentWidget)
{
    m_customWidgetsWithScript.clear();
    return QFormBuilder::create(ui, parentWidget);
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

    if (widget) {
        widget->setObjectName(name);
        if (QSimpleResource::hasCustomWidgetScript(m_core, widget))
            m_customWidgetsWithScript.insert(widget);
    }

    return widget;
}

bool QDesignerFormBuilder::addItemContainerExtension(QWidget *widget, QWidget *parentWidget)
{
    QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(m_core->extensionManager(), parentWidget);
    if (!container)
        return false;

    container->addWidget(widget);
    return true;
}

bool QDesignerFormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    // Use container extension or rely on scripts unless main window.
    if (QFormBuilder::addItem(ui_widget, widget, parentWidget))
        return true;

    // Use for mainwindow at any event
    if (qobject_cast<const QMainWindow*>(parentWidget))
        return addItemContainerExtension(widget, parentWidget);

    // Assume the script populates the container
    if (m_mode == UseScriptForContainerExtension && m_customWidgetsWithScript.contains(parentWidget))
         return true;

    return addItemContainerExtension(widget, parentWidget);;
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

/* If the property is a enum or flag value, retrieve
 * the existing enum/flag type via property sheet and use it to convert */

static bool readDomEnumerationValue(const DomProperty *p,
                                    const QDesignerPropertySheetExtension* sheet,
                                    QVariant &v)
{
    switch (p->kind()) {
    case DomProperty::Set: {
        const int index = sheet->indexOf(p->attributeName());
        if (index == -1)
            return false;
        const QVariant sheetValue = sheet->property(index);
        if (qVariantCanConvert<PropertySheetFlagValue>(sheetValue)) {
            const PropertySheetFlagValue f = qvariant_cast<PropertySheetFlagValue>(sheetValue);
            bool ok = false;
            v = f.metaFlags.parseFlags(p->elementSet(), &ok);
            if (!ok)
                designerWarning(f.metaFlags.messageParseFailed(p->elementSet()));
            return true;
        }
    }
        break;
    case DomProperty::Enum: {
        const int index = sheet->indexOf(p->attributeName());
        if (index == -1)
            return false;
        const QVariant sheetValue = sheet->property(index);
        if (qVariantCanConvert<PropertySheetEnumValue>(sheetValue)) {
            const PropertySheetEnumValue e = qvariant_cast<PropertySheetEnumValue>(sheetValue);
            bool ok = false;
            v = e.metaEnum.parseEnum(p->elementEnum(), &ok);
            if (!ok)
                designerWarning(e.metaEnum.messageParseFailed(p->elementEnum()));
            return true;
        }
    }
        break;
    default:
        break;
    }
    return false;
}

void QDesignerFormBuilder::applyProperties(QObject *o, const QList<DomProperty*> &properties)
{
    typedef QList<DomProperty*> DomPropertyList;

    if (properties.empty())
        return;

    QFormBuilderExtra *formBuilderExtra = QFormBuilderExtra::instance(this);
    const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), o);
    const QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension*>(core()->extensionManager(), o);
    const QMetaObject *meta = o->metaObject();
    const bool dynamicPropertiesAllowed = dynamicSheet && dynamicSheet->dynamicPropertiesAllowed() && strcmp(meta->className(), "QAxWidget") != 0;

    const DomPropertyList::const_iterator cend = properties.constEnd();
    for (DomPropertyList::const_iterator it = properties.constBegin(); it != cend; ++it) {
        DomProperty *p = *it;
        QVariant v;
        if (!readDomEnumerationValue(p, sheet, v))
            v = toVariant(meta, p);

        if (v.isNull())
            continue;

        const QString attributeName = p->attributeName();
        if (formBuilderExtra->applyPropertyInternally(o, attributeName, v))
            continue;

        const QByteArray pname = attributeName.toUtf8();
        const int index = meta->indexOfProperty(pname);

        QObject *obj = o;
        QAbstractScrollArea *scroll = qobject_cast<QAbstractScrollArea *>(o);
        if (scroll && QLatin1String(pname) == QLatin1String("cursor") && scroll->viewport())
            obj = scroll->viewport();

        if (index != -1 || dynamicPropertiesAllowed) {
            // a real property
            obj->setProperty(pname, v);
        }
    }
}

DomWidget *QDesignerFormBuilder::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
    DomWidget *ui_widget = QFormBuilder::createDom(widget, ui_parentWidget, recursive);
    QSimpleResource::addExtensionDataToDOM(this, m_core, ui_widget, widget);
    return ui_widget;
}

QWidget *QDesignerFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    QWidget *widget = QFormBuilder::create(ui_widget, parentWidget);
    // Do not apply state if scripts are to be run in preview mode
    QSimpleResource::applyExtensionDataFromDOM(this, m_core, ui_widget, widget, m_mode == DisableScripts);
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
                                             const QString &appStyleSheet,
                                      ScriptErrors *scriptErrors,
                                             QString *errorMessage)
{
    scriptErrors->clear();
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
    QDesignerFormBuilder builder(fw->core(), UseScriptAndContainerExtension);
    builder.setWorkingDirectory(fw->absoluteDir());

    const bool warningsEnabled = QSimpleResource::setWarningsEnabled(false);
    QByteArray bytes = fw->contents().toUtf8();
    QSimpleResource::setWarningsEnabled(warningsEnabled);

    QBuffer buffer(&bytes);

    QWidget *widget = builder.load(&buffer, 0);
    if (!widget) { // Shouldn't happen
        *errorMessage = QObject::tr("The preview failed to build.");
        return  0;
    }
    // Check for script errors
    *scriptErrors = builder.formScriptRunner()->errors();
    if (!scriptErrors->empty()) {
        *errorMessage = summarizeScriptErrors(*scriptErrors);
        delete widget;
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
    // Fake application style sheet by prepending. (If this doesn't work, fake by nesting
    // into parent widget).
    if (!appStyleSheet.isEmpty()) {
        QString styleSheet = appStyleSheet;
        styleSheet += QLatin1Char('\n');
        styleSheet +=  widget->styleSheet();
        widget->setStyleSheet(styleSheet);
    }

    widget->setWindowTitle(QObject::tr("%1 - [Preview]").arg(widget->windowTitle()));
    return widget;
}

QWidget *QDesignerFormBuilder::createPreview(const QDesignerFormWindowInterface *fw, const QString &styleName)
{
    return createPreview(fw, styleName, QString());
}

QWidget *QDesignerFormBuilder::createPreview(const QDesignerFormWindowInterface *fw, const QString &styleName, const QString &appStyleSheet, QString *errorMessage)
{
    ScriptErrors scriptErrors;
    return  createPreview(fw, styleName, appStyleSheet, &scriptErrors, errorMessage);
}

QWidget *QDesignerFormBuilder::createPreview(const QDesignerFormWindowInterface *fw, const QString &styleName, const QString &appStyleSheet)
{
    ScriptErrors scriptErrors;
    QString errorMessage;
    QWidget *widget = createPreview(fw, styleName, appStyleSheet, &scriptErrors, &errorMessage);
    if (!widget) {
        // Display Script errors or message box
        QWidget *dialogParent = fw->core()->topLevel();
        if (scriptErrors.empty()) {
            fw->core()->dialogGui()->message(dialogParent, QDesignerDialogGuiInterface::PreviewFailureMessage,
                                             QMessageBox::Warning, QObject::tr("Designer"), errorMessage, QMessageBox::Ok);
        } else {
            ScriptErrorDialog scriptErrorDialog(scriptErrors, dialogParent);
            scriptErrorDialog.exec();
        }
        return 0;
    }
    return widget;
}

QPixmap QDesignerFormBuilder::createPreviewPixmap(const QDesignerFormWindowInterface *fw, const QString &styleName, const QString &appStyleSheet)
{
    QWidget *widget = createPreview(fw, styleName, appStyleSheet);
    if (!widget)
        return QPixmap();

    const QPixmap rc = QPixmap::grabWidget (widget);
    widget->deleteLater();
    return rc;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE

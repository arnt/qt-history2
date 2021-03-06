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

#include <QtDesigner/QDesignerComponents>

#include <actioneditor_p.h>
#include <widgetdatabase_p.h>
#include <widgetfactory_p.h>

#include <formeditor/formeditor.h>
#include <widgetbox/widgetbox.h>
#include <propertyeditor/propertyeditor.h>
#include <objectinspector/objectinspector.h>
#include <taskmenu/taskmenu_component.h>
#include <resourceeditor_p.h>
#include <qdesigner_integration_p.h>
#include <signalsloteditor/signalsloteditorwindow.h>

#include <buddyeditor/buddyeditor_plugin.h>
#include <signalsloteditor/signalsloteditor_plugin.h>
#include <tabordereditor/tabordereditor_plugin.h>

#include <QtDesigner/QDesignerLanguageExtension>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerResourceBrowserInterface>

#include <QtCore/qplugin.h>
#include <QtCore/QDir>

// ### keep it in sync with Q_IMPORT_PLUGIN in qplugin.h
#define DECLARE_PLUGIN_INSTANCE(PLUGIN) \
    extern QT_ADD_NAMESPACE(QObject) *qt_plugin_instance_##PLUGIN(); \
    class Static##PLUGIN##PluginInstance { public: \
        Static##PLUGIN##PluginInstance() {                      \
            QT_ADD_NAMESPACE(qRegisterStaticPluginInstanceFunction) \
                (&qt_plugin_instance_##PLUGIN); \
        } \
    };

#define INIT_PLUGIN_INSTANCE(PLUGIN) \
    do { \
        Static##PLUGIN##PluginInstance instance; \
        Q_UNUSED(instance); \
    } while (0)

DECLARE_PLUGIN_INSTANCE(SignalSlotEditorPlugin)
DECLARE_PLUGIN_INSTANCE(BuddyEditorPlugin)
DECLARE_PLUGIN_INSTANCE(TabOrderEditorPlugin)

static void initResources()
{
    // Q_INIT_RESOURCE only usable in functions in global namespace 
    Q_INIT_RESOURCE(formeditor);
    Q_INIT_RESOURCE(widgetbox);
}


static void initInstances()
{
    static bool plugins_initialized = false;

    if (!plugins_initialized) {
        INIT_PLUGIN_INSTANCE(SignalSlotEditorPlugin);
        INIT_PLUGIN_INSTANCE(BuddyEditorPlugin);
        INIT_PLUGIN_INSTANCE(TabOrderEditorPlugin);
        plugins_initialized = true;
    }
}

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerComponents
    \brief The QDesignerComponents class provides a central resource for the various components
    used in the \QD user interface.
    \inmodule QtDesigner
    \internal

    The QDesignerComponents class is a factory for each of the standard components present
    in the \QD user interface. It is mostly useful for developers who want to implement
    a standalone form editing environment using \QD's components, or who need to integrate
    \QD's components into an existing integrated development environment (IDE).

    \sa QDesignerFormEditorInterface, QDesignerObjectInspectorInterface,
        QDesignerPropertyEditorInterface, QDesignerWidgetBoxInterface
*/

/*!
    Initializes the resources used by the components.*/
void QDesignerComponents::initializeResources()
{
    initResources();
}

/*!
    Initializes the plugins used by the components.*/
void QDesignerComponents::initializePlugins(QDesignerFormEditorInterface *core)
{
    qdesigner_internal::QDesignerIntegration::initializePlugins(core);
}

/*!
    Constructs a form editor interface with the given \a parent.*/
QDesignerFormEditorInterface *QDesignerComponents::createFormEditor(QObject *parent)
{
    initInstances();
    return new qdesigner_internal::FormEditor(parent);
}

/*!
    Returns a new task menu with the given \a parent for the \a core interface.*/
QObject *QDesignerComponents::createTaskMenu(QDesignerFormEditorInterface *core, QObject *parent)
{
    return new qdesigner_internal::TaskMenuComponent(core, parent);
}

/*!
    Returns a new widget box interface with the given \a parent for the \a core interface.*/
QDesignerWidgetBoxInterface *QDesignerComponents::createWidgetBox(QDesignerFormEditorInterface *core, QWidget *parent)
{
    qdesigner_internal::WidgetBox *widgetBox = new qdesigner_internal::WidgetBox(core, parent);

    const QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core);

    do {
        if (lang) {
            const QString languageWidgetBox = lang->widgetBoxContents();
            if (!languageWidgetBox.isEmpty()) {
                widgetBox->loadContents(lang->widgetBoxContents());
                break;
            }
        }

        widgetBox->setFileName(QLatin1String(":/trolltech/widgetbox/widgetbox.xml"));
        widgetBox->load();
    } while (false);

    QString rc = QDir::homePath();
    rc += QLatin1String("/.designer");
    rc += QLatin1String("/widgetbox");
    if (lang) {
        rc += QLatin1Char('.');
        rc += lang->uiExtension();
    }
    rc += QLatin1String(".xml");

    widgetBox->setFileName(rc);
    widgetBox->load();

    return widgetBox;
}

/*!
    Returns a new property editor interface with the given \a parent for the \a core interface.*/
QDesignerPropertyEditorInterface *QDesignerComponents::createPropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::PropertyEditor(core, parent);
}

/*!
    Returns a new object inspector interface with the given \a parent for the \a core interface.*/
QDesignerObjectInspectorInterface *QDesignerComponents::createObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::ObjectInspector(core, parent);
}

/*!
    Returns a new action editor interface with the given \a parent for the \a core interface.*/
QDesignerActionEditorInterface *QDesignerComponents::createActionEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::ActionEditor(core, parent);
}

/*!
    Returns a new resource editor with the given \a parent for the \a core interface.*/
QWidget *QDesignerComponents::createResourceEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    if (QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core)) {
        QWidget *w = lang->createResourceBrowser(parent);
        if (w)
            return w;
    }
    return new qdesigner_internal::ResourceEditor(core, true, parent);
}

/*!
    Returns a new signal-slot editor with the given \a parent for the \a core interface.*/
QWidget *QDesignerComponents::createSignalSlotEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::SignalSlotEditorWindow(core, parent);
}

QT_END_NAMESPACE


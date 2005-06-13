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

#include "abstractformeditor.h"

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/abstractimagecollection.h>
#include <QtDesigner/abstractmetadatabase.h>
#include <QtDesigner/abstractwidgetbox.h>
#include <QtDesigner/abstractwidgetfactory.h>
#include <QtDesigner/abstractpropertyeditor.h>
#include <QtDesigner/abstractwidgetdatabase.h>
#include <QtDesigner/abstractobjectinspector.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstracticoncache.h>

class PluginManager;

/*!
    \class QDesignerFormEditorInterface
    \brief The QDesignerFormEditorInterface class provides an interface that is used to
    control \QD's form editor component.
    \inmodule QtDesigner
*/

/*!
    Constructs a form editor interface with the given \a parent.*/
QDesignerFormEditorInterface::QDesignerFormEditorInterface(QObject *parent)
    : QObject(parent),
      m_topLevel(0),
      m_widgetBox(0),
      m_propertyEditor(0),
      m_formWindowManager(0),
      m_extensionManager(0),
      m_metaDataBase(0),
      m_widgetDataBase(0),
      m_widgetFactory(0),
      m_objectInspector(0)
{
}

/*!
    Destroys the interface to the form editor.*/
QDesignerFormEditorInterface::~QDesignerFormEditorInterface()
{
}

/*!
    Returns an interface to \QD's widget box.*/
QDesignerWidgetBoxInterface *QDesignerFormEditorInterface::widgetBox() const
{ return m_widgetBox; }

/*!
    Sets the widget box used by the form editor to the specified \a widgetBox.*/
void QDesignerFormEditorInterface::setWidgetBox(QDesignerWidgetBoxInterface *widgetBox)
{ m_widgetBox = widgetBox; }

/*!
    Returns an interface to the property editor used by the form editor.*/
QDesignerPropertyEditorInterface *QDesignerFormEditorInterface::propertyEditor() const
{ return m_propertyEditor; }

/*!
Sets the property editor used by the form editor to the specified \a propertyEditor.*/
void QDesignerFormEditorInterface::setPropertyEditor(QDesignerPropertyEditorInterface *propertyEditor)
{ m_propertyEditor = propertyEditor; }

/*!
    Returns the top-level widget used by the form editor.*/
QWidget *QDesignerFormEditorInterface::topLevel() const
{ return m_topLevel; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setTopLevel(QWidget *topLevel)
{ m_topLevel = topLevel; }

/*!
    Returns the interface used to control the form window manager.*/
QDesignerFormWindowManagerInterface *QDesignerFormEditorInterface::formWindowManager() const
{ return m_formWindowManager; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setFormManager(QDesignerFormWindowManagerInterface *formWindowManager)
{ m_formWindowManager = formWindowManager; }

/*!
    Returns the extension manager used by the form editor.*/
QExtensionManager *QDesignerFormEditorInterface::extensionManager() const
{ return m_extensionManager; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setExtensionManager(QExtensionManager *extensionManager)
{ m_extensionManager = extensionManager; }

/*!
    Returns an interface to the meta database used by the form editor.*/
QDesignerMetaDataBaseInterface *QDesignerFormEditorInterface::metaDataBase() const
{ return m_metaDataBase; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setMetaDataBase(QDesignerMetaDataBaseInterface *metaDataBase)
{ m_metaDataBase = metaDataBase; }

/*!
    Returns an interface to the widget database used by the form editor.*/
QDesignerWidgetDataBaseInterface *QDesignerFormEditorInterface::widgetDataBase() const
{ return m_widgetDataBase; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setWidgetDataBase(QDesignerWidgetDataBaseInterface *widgetDataBase)
{ m_widgetDataBase = widgetDataBase; }

/*!
    Returns an interface to the widget factory used by the form editor to create widgets
    for the form.*/
QDesignerWidgetFactoryInterface *QDesignerFormEditorInterface::widgetFactory() const
{ return m_widgetFactory; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setWidgetFactory(QDesignerWidgetFactoryInterface *widgetFactory)
{ m_widgetFactory = widgetFactory; }

/*!
    Returns an interface to the object inspector used by the form editor.*/
QDesignerObjectInspectorInterface *QDesignerFormEditorInterface::objectInspector() const
{ return m_objectInspector; }

/*!
    Sets the object inspector used by the form editor to the specified \a objectInspector.*/
void QDesignerFormEditorInterface::setObjectInspector(QDesignerObjectInspectorInterface *objectInspector)
{ m_objectInspector = objectInspector; }

/*!
    Returns an interface to the icon cache used by the form editor to manage icons.*/
QDesignerIconCacheInterface *QDesignerFormEditorInterface::iconCache() const
{ return m_iconCache; }

/*!
    \internal
*/
void QDesignerFormEditorInterface::setIconCache(QDesignerIconCacheInterface *cache)
{ m_iconCache = cache; }

/*!
    Returns the plugin manager used by the form editor.*/
PluginManager *QDesignerFormEditorInterface::pluginManager() const
{ return m_pluginManager; }

/*!
Sets the plugin manager used by the form editor to the specified \a pluginManager.*/
void QDesignerFormEditorInterface::setPluginManager(PluginManager *pluginManager)
{ m_pluginManager = pluginManager; }

/*!
    Returns the path to the resources used by the form editor.*/
QString QDesignerFormEditorInterface::resourceLocation() const
{
#ifdef Q_WS_MAC
    return QLatin1String(":/trolltech/formeditor/images/mac");
#else
    return QLatin1String(":/trolltech/formeditor/images/win");
#endif
}

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

QDesignerFormEditorInterface::~QDesignerFormEditorInterface()
{
}

QDesignerWidgetBoxInterface *QDesignerFormEditorInterface::widgetBox() const
{ return m_widgetBox; }

void QDesignerFormEditorInterface::setWidgetBox(QDesignerWidgetBoxInterface *widgetBox)
{ m_widgetBox = widgetBox; }

QDesignerPropertyEditorInterface *QDesignerFormEditorInterface::propertyEditor() const
{ return m_propertyEditor; }

void QDesignerFormEditorInterface::setPropertyEditor(QDesignerPropertyEditorInterface *propertyEditor)
{ m_propertyEditor = propertyEditor; }

QWidget *QDesignerFormEditorInterface::topLevel() const
{ return m_topLevel; }

void QDesignerFormEditorInterface::setTopLevel(QWidget *topLevel)
{ m_topLevel = topLevel; }

QDesignerFormWindowManagerInterface *QDesignerFormEditorInterface::formWindowManager() const
{ return m_formWindowManager; }

void QDesignerFormEditorInterface::setFormManager(QDesignerFormWindowManagerInterface *formWindowManager)
{ m_formWindowManager = formWindowManager; }

QExtensionManager *QDesignerFormEditorInterface::extensionManager() const
{ return m_extensionManager; }

void QDesignerFormEditorInterface::setExtensionManager(QExtensionManager *extensionManager)
{ m_extensionManager = extensionManager; }

QDesignerMetaDataBaseInterface *QDesignerFormEditorInterface::metaDataBase() const
{ return m_metaDataBase; }

void QDesignerFormEditorInterface::setMetaDataBase(QDesignerMetaDataBaseInterface *metaDataBase)
{ m_metaDataBase = metaDataBase; }

QDesignerWidgetDataBaseInterface *QDesignerFormEditorInterface::widgetDataBase() const
{ return m_widgetDataBase; }

void QDesignerFormEditorInterface::setWidgetDataBase(QDesignerWidgetDataBaseInterface *widgetDataBase)
{ m_widgetDataBase = widgetDataBase; }

QDesignerWidgetFactoryInterface *QDesignerFormEditorInterface::widgetFactory() const
{ return m_widgetFactory; }

void QDesignerFormEditorInterface::setWidgetFactory(QDesignerWidgetFactoryInterface *widgetFactory)
{ m_widgetFactory = widgetFactory; }

QDesignerObjectInspectorInterface *QDesignerFormEditorInterface::objectInspector() const
{ return m_objectInspector; }

void QDesignerFormEditorInterface::setObjectInspector(QDesignerObjectInspectorInterface *objectInspector)
{ m_objectInspector = objectInspector; }

QDesignerIconCacheInterface *QDesignerFormEditorInterface::iconCache() const
{ return m_iconCache; }

void QDesignerFormEditorInterface::setIconCache(QDesignerIconCacheInterface *cache)
{ m_iconCache = cache; }

PluginManager *QDesignerFormEditorInterface::pluginManager() const
{ return m_pluginManager; }

void QDesignerFormEditorInterface::setPluginManager(PluginManager *pluginManager)
{ m_pluginManager = pluginManager; }

QString QDesignerFormEditorInterface::resourceLocation() const
{
#ifdef Q_WS_MAC
    return QLatin1String(":/trolltech/formeditor/images/mac");
#else
    return QLatin1String(":/trolltech/formeditor/images/win");
#endif
}

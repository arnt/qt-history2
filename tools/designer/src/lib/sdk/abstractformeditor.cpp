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

#include <abstractformwindow.h>
#include <abstractformwindowmanager.h>
#include <abstractimagecollection.h>
#include <abstractmetadatabase.h>
#include <abstractwidgetbox.h>
#include <abstractwidgetfactory.h>
#include <abstractpropertyeditor.h>
#include <abstractwidgetdatabase.h>
#include <abstractobjectinspector.h>
#include <qextensionmanager.h>
#include <abstractpixmapcache.h>

class PluginManager;

AbstractFormEditor::AbstractFormEditor(QObject *parent)
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

AbstractFormEditor::~AbstractFormEditor()
{
}

AbstractWidgetBox *AbstractFormEditor::widgetBox() const
{ return m_widgetBox; }

void AbstractFormEditor::setWidgetBox(AbstractWidgetBox *widgetBox)
{ m_widgetBox = widgetBox; }

AbstractPropertyEditor *AbstractFormEditor::propertyEditor() const
{ return m_propertyEditor; }

void AbstractFormEditor::setPropertyEditor(AbstractPropertyEditor *propertyEditor)
{ m_propertyEditor = propertyEditor; }

QWidget *AbstractFormEditor::topLevel() const
{ return m_topLevel; }

void AbstractFormEditor::setTopLevel(QWidget *topLevel)
{ m_topLevel = topLevel; }

AbstractFormWindowManager *AbstractFormEditor::formWindowManager() const
{ return m_formWindowManager; }

void AbstractFormEditor::setFormManager(AbstractFormWindowManager *formWindowManager)
{ m_formWindowManager = formWindowManager; }

QExtensionManager *AbstractFormEditor::extensionManager() const
{ return m_extensionManager; }

void AbstractFormEditor::setExtensionManager(QExtensionManager *extensionManager)
{ m_extensionManager = extensionManager; }

AbstractMetaDataBase *AbstractFormEditor::metaDataBase() const
{ return m_metaDataBase; }

void AbstractFormEditor::setMetaDataBase(AbstractMetaDataBase *metaDataBase)
{ m_metaDataBase = metaDataBase; }

AbstractWidgetDataBase *AbstractFormEditor::widgetDataBase() const
{ return m_widgetDataBase; }

void AbstractFormEditor::setWidgetDataBase(AbstractWidgetDataBase *widgetDataBase)
{ m_widgetDataBase = widgetDataBase; }

AbstractWidgetFactory *AbstractFormEditor::widgetFactory() const
{ return m_widgetFactory; }

void AbstractFormEditor::setWidgetFactory(AbstractWidgetFactory *widgetFactory)
{ m_widgetFactory = widgetFactory; }

AbstractObjectInspector *AbstractFormEditor::objectInspector() const
{ return m_objectInspector; }

void AbstractFormEditor::setObjectInspector(AbstractObjectInspector *objectInspector)
{ m_objectInspector = objectInspector; }

AbstractPixmapCache *AbstractFormEditor::pixmapCache() const
{ return m_pixmapCache; }

void AbstractFormEditor::setPixmapCache(AbstractPixmapCache *cache)
{ m_pixmapCache = cache; }

PluginManager *AbstractFormEditor::pluginManager() const
{ return m_pluginManager; }

void AbstractFormEditor::setPluginManager(PluginManager *pluginManager)
{ m_pluginManager = pluginManager; }


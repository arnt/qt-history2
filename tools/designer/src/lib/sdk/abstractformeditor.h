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

#ifndef ABSTRACTFORMEDITOR_H
#define ABSTRACTFORMEDITOR_H

#include "sdk_global.h"

#include <QObject>
#include <QPointer>

class AbstractWidgetBox;
class AbstractPropertyEditor;
class AbstractFormWindowManager;
class AbstractWidgetDataBase;
class AbstractMetaDataBase;
class AbstractWidgetFactory;
class AbstractDnDManager;
class AbstractObjectInspector;
class AbstractIconCache;
class PluginManager;

class QWidget;

class QExtensionManager;

class QT_SDK_EXPORT AbstractFormEditor: public QObject
{
    Q_OBJECT
public:
    AbstractFormEditor(QObject *parent = 0);
    virtual ~AbstractFormEditor();

    QExtensionManager *extensionManager() const;

    QWidget *topLevel() const;
    AbstractWidgetBox *widgetBox() const;
    AbstractPropertyEditor *propertyEditor() const;
    AbstractObjectInspector *objectInspector() const;
    AbstractFormWindowManager *formWindowManager() const;
    AbstractWidgetDataBase *widgetDataBase() const;
    AbstractMetaDataBase *metaDataBase() const;
    AbstractWidgetFactory *widgetFactory() const;
    AbstractIconCache *iconCache() const;
    PluginManager *pluginManager() const;
    QString resourceLocation() const; // ### Remove me!!!

    void setTopLevel(QWidget *topLevel);
    void setWidgetBox(AbstractWidgetBox *widgetBox);
    void setPropertyEditor(AbstractPropertyEditor *propertyEditor);
    void setObjectInspector(AbstractObjectInspector *objectInspector);
    void setPluginManager(PluginManager *pluginManager);

protected:
    void setFormManager(AbstractFormWindowManager *formWindowManager);
    void setMetaDataBase(AbstractMetaDataBase *metaDataBase);
    void setWidgetDataBase(AbstractWidgetDataBase *widgetDataBase);
    void setWidgetFactory(AbstractWidgetFactory *widgetFactory);
    void setExtensionManager(QExtensionManager *extensionManager);
    void setIconCache(AbstractIconCache *cache);

private:
    QPointer<QWidget> m_topLevel;
    QPointer<AbstractWidgetBox> m_widgetBox;
    QPointer<AbstractPropertyEditor> m_propertyEditor;
    QPointer<AbstractFormWindowManager> m_formWindowManager;
    QPointer<QExtensionManager> m_extensionManager;
    QPointer<AbstractMetaDataBase> m_metaDataBase;
    QPointer<AbstractWidgetDataBase> m_widgetDataBase;
    QPointer<AbstractWidgetFactory> m_widgetFactory;
    QPointer<AbstractObjectInspector> m_objectInspector;
    QPointer<AbstractIconCache> m_iconCache;
    PluginManager *m_pluginManager; // ### QPointer!?

private:
    AbstractFormEditor(const AbstractFormEditor &other);
    void operator = (const AbstractFormEditor &other);
};

#endif // ABSTRACTFORMEDITOR_H

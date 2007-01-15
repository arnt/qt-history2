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

#ifndef ABSTRACTFORMEDITOR_H
#define ABSTRACTFORMEDITOR_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/QObject>
#include <QtCore/QPointer>

QT_BEGIN_HEADER

class QDesignerWidgetBoxInterface;
class QDesignerPropertyEditorInterface;
class QDesignerFormWindowManagerInterface;
class QDesignerWidgetDataBaseInterface;
class QDesignerMetaDataBaseInterface;
class QDesignerWidgetFactoryInterface;
class QDesignerObjectInspectorInterface;
class QDesignerBrushManagerInterface;
class QDesignerIconCacheInterface;
class QDesignerActionEditorInterface;
class QDesignerIntegrationInterface;
class QDesignerPluginManager;

class QWidget;

class QExtensionManager;

class QDESIGNER_SDK_EXPORT QDesignerFormEditorInterface: public QObject
{
    Q_OBJECT
public:
    QDesignerFormEditorInterface(QObject *parent = 0);
    virtual ~QDesignerFormEditorInterface();

    QExtensionManager *extensionManager() const;

    QWidget *topLevel() const;
    QDesignerWidgetBoxInterface *widgetBox() const;
    QDesignerPropertyEditorInterface *propertyEditor() const;
    QDesignerObjectInspectorInterface *objectInspector() const;
    QDesignerFormWindowManagerInterface *formWindowManager() const;
    QDesignerWidgetDataBaseInterface *widgetDataBase() const;
    QDesignerMetaDataBaseInterface *metaDataBase() const;
    QDesignerWidgetFactoryInterface *widgetFactory() const;
    QDesignerBrushManagerInterface *brushManager() const;
    QDesignerIconCacheInterface *iconCache() const;
    QDesignerActionEditorInterface *actionEditor() const;
    QDesignerIntegrationInterface *integration() const;
    QDesignerPluginManager *pluginManager() const;
    QString resourceLocation() const;

    void setTopLevel(QWidget *topLevel);
    void setWidgetBox(QDesignerWidgetBoxInterface *widgetBox);
    void setPropertyEditor(QDesignerPropertyEditorInterface *propertyEditor);
    void setObjectInspector(QDesignerObjectInspectorInterface *objectInspector);
    void setPluginManager(QDesignerPluginManager *pluginManager);
    void setActionEditor(QDesignerActionEditorInterface *actionEditor);
    void setIntegration(QDesignerIntegrationInterface *integration);

protected:
    void setFormManager(QDesignerFormWindowManagerInterface *formWindowManager);
    void setMetaDataBase(QDesignerMetaDataBaseInterface *metaDataBase);
    void setWidgetDataBase(QDesignerWidgetDataBaseInterface *widgetDataBase);
    void setWidgetFactory(QDesignerWidgetFactoryInterface *widgetFactory);
    void setExtensionManager(QExtensionManager *extensionManager);
    void setBrushManager(QDesignerBrushManagerInterface *brushManager);
    void setIconCache(QDesignerIconCacheInterface *cache);

private:
    QPointer<QWidget> m_pad[12];
    struct QDesignerFormEditorInterfacePrivate;
    QDesignerFormEditorInterfacePrivate *d;

private:
    QDesignerFormEditorInterface(const QDesignerFormEditorInterface &other);
    void operator = (const QDesignerFormEditorInterface &other);
};

QT_END_HEADER

#endif // ABSTRACTFORMEDITOR_H

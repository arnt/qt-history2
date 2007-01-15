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

#include "abstractformeditor.h"
#include "private/qobject_p.h"

#include <QtDesigner/QtDesigner>

class QDesignerFormEditorInterfacePrivate : public  QObjectPrivate {
public:
    QDesignerFormEditorInterfacePrivate() : m_pluginManager(0) {}

    QPointer<QWidget> m_topLevel;
    QPointer<QDesignerWidgetBoxInterface> m_widgetBox;
    QPointer<QDesignerPropertyEditorInterface> m_propertyEditor;
    QPointer<QDesignerFormWindowManagerInterface> m_formWindowManager;
    QPointer<QExtensionManager> m_extensionManager;
    QPointer<QDesignerMetaDataBaseInterface> m_metaDataBase;
    QPointer<QDesignerWidgetDataBaseInterface> m_widgetDataBase;
    QPointer<QDesignerWidgetFactoryInterface> m_widgetFactory;
    QPointer<QDesignerObjectInspectorInterface> m_objectInspector;
    QPointer<QDesignerBrushManagerInterface> m_brushManager;
    QPointer<QDesignerIntegrationInterface> m_integration;
    QPointer<QDesignerIconCacheInterface> m_iconCache;
    QPointer<QDesignerActionEditorInterface> m_actionEditor;
    QDesignerPluginManager *m_pluginManager;
};

/*!
    \class QDesignerFormEditorInterface

    \brief The QDesignerFormEditorInterface class allows you to access
    Qt Designer's various components.

    \inmodule QtDesigner

    \QD's current QDesignerFormEditorInterface object holds
    information about all \QD's components: The action editor, the
    object inspector, the property editor, the widget box, and the
    extension and form window managers. QDesignerFormEditorInterface
    contains a collection of functions that provides interfaces to all
    these components. They are typically used to query (and
    manipulate) the respective component. For example:

    \code
        QDesignerObjectInspectorInterface *objectInspector = 0;
        objectInspector = formEditor->objectInspector();

        QDesignerFormWindowManagerInterface *manager = 0;
        manager = formEditor->formWindowManager();

        objectInspector->setFormWindow(manager->formWindow(0));
    \endcode

    QDesignerFormEditorInterface is not intended to be instantiated
    directly. A pointer to \QD's current QDesignerFormEditorInterface
    object (\c formEditor in the example above) is provided by the
    QDesignerCustomWidgetInterface::initialize() function's
    parameter. When implementing a custom widget plugin, you must
    subclass the QDesignerCustomWidgetInterface to expose your plugin
    to \QD.

    QDesignerFormEditorInterface also provides functions that can set
    the action editor, property editor, object inspector and widget
    box. These are only useful if you want to provide your own custom
    components.

    Finally, QDesignerFormEditorInterface provides the topLevel()
    function that returns \QD's top-level widget.

    \sa QDesignerCustomWidgetInterface
*/

/*!
    Constructs a QDesignerFormEditorInterface object with the given \a
    parent.
*/

QDesignerFormEditorInterface::QDesignerFormEditorInterface(QObject *parent)
    : QObject(*(new QDesignerFormEditorInterfacePrivate), parent),
      m_pad13(0)
{
}

/*!
    Destroys the QDesignerFormEditorInterface object.
*/
QDesignerFormEditorInterface::~QDesignerFormEditorInterface()
{
}

/*!
    Returns an interface to \QD's widget box.

    \sa setWidgetBox()
*/
QDesignerWidgetBoxInterface *QDesignerFormEditorInterface::widgetBox() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_widgetBox; 
}

/*!
    Sets \QD's widget box to be the specified \a widgetBox.

    \sa widgetBox()
*/
void QDesignerFormEditorInterface::setWidgetBox(QDesignerWidgetBoxInterface *widgetBox)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_widgetBox = widgetBox; 
}

/*!
    Returns an interface to \QD's property editor.

    \sa setPropertyEditor()
*/
QDesignerPropertyEditorInterface *QDesignerFormEditorInterface::propertyEditor() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_propertyEditor; 
}

/*!
    Sets \QD's property editor to be the specified \a propertyEditor.

    \sa propertyEditor()
*/
void QDesignerFormEditorInterface::setPropertyEditor(QDesignerPropertyEditorInterface *propertyEditor)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_propertyEditor = propertyEditor; 
}

/*!
    Returns an interface to \QD's action editor.

    \sa setActionEditor()
*/
QDesignerActionEditorInterface *QDesignerFormEditorInterface::actionEditor() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_actionEditor; 
}

/*!
    Sets \QD's action editor to be the specified \a actionEditor.

    \sa actionEditor()
*/
void QDesignerFormEditorInterface::setActionEditor(QDesignerActionEditorInterface *actionEditor)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_actionEditor = actionEditor; 
}

/*!
    Returns \QD's top-level widget.
*/
QWidget *QDesignerFormEditorInterface::topLevel() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_topLevel; 
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setTopLevel(QWidget *topLevel)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_topLevel = topLevel; 
}

/*!
    Returns an interface to \QD's form window manager.
*/
QDesignerFormWindowManagerInterface *QDesignerFormEditorInterface::formWindowManager() const
{
    Q_D(const QDesignerFormEditorInterface);
    return d->m_formWindowManager; 
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setFormManager(QDesignerFormWindowManagerInterface *formWindowManager)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_formWindowManager = formWindowManager; 
}

/*!
    Returns an interface to \QD's extension manager.
*/
QExtensionManager *QDesignerFormEditorInterface::extensionManager() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_extensionManager; 
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setExtensionManager(QExtensionManager *extensionManager)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_extensionManager = extensionManager; 
}

/*!
    \internal

    Returns an interface to the meta database used by the form editor.
*/
QDesignerMetaDataBaseInterface *QDesignerFormEditorInterface::metaDataBase() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_metaDataBase; 
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setMetaDataBase(QDesignerMetaDataBaseInterface *metaDataBase)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_metaDataBase = metaDataBase; 
}

/*!
    \internal

    Returns an interface to the widget database used by the form editor.
*/
QDesignerWidgetDataBaseInterface *QDesignerFormEditorInterface::widgetDataBase() const
{
    Q_D(const QDesignerFormEditorInterface);
    return d->m_widgetDataBase;
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setWidgetDataBase(QDesignerWidgetDataBaseInterface *widgetDataBase)
{
    Q_D(QDesignerFormEditorInterface);
    d->m_widgetDataBase = widgetDataBase; 
}

/*!
    \internal

    Returns an interface to the widget factory used by the form editor
    to create widgets for the form.
*/
QDesignerWidgetFactoryInterface *QDesignerFormEditorInterface::widgetFactory() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_widgetFactory;
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setWidgetFactory(QDesignerWidgetFactoryInterface *widgetFactory)
{
    Q_D(QDesignerFormEditorInterface);
    d->m_widgetFactory = widgetFactory; 
}

/*!
    Returns an interface to \QD's object inspector.
*/
QDesignerObjectInspectorInterface *QDesignerFormEditorInterface::objectInspector() const
{
    Q_D(const QDesignerFormEditorInterface);
    return d->m_objectInspector; 
}

/*!
    Sets \QD's object inspector to be the specified \a
    objectInspector.

    \sa objectInspector()
*/
void QDesignerFormEditorInterface::setObjectInspector(QDesignerObjectInspectorInterface *objectInspector)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_objectInspector = objectInspector;
}

/*!
    \internal

    Returns an interface to the brush manager used by the palette editor.
*/
QDesignerBrushManagerInterface *QDesignerFormEditorInterface::brushManager() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_brushManager; 
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setBrushManager(QDesignerBrushManagerInterface *brushManager)
{
    Q_D(QDesignerFormEditorInterface);
    d->m_brushManager = brushManager; 
}

/*!
    \internal

    Returns an interface to the integration.
*/
QDesignerIntegrationInterface *QDesignerFormEditorInterface::integration() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_integration; 
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setIntegration(QDesignerIntegrationInterface *integration)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_integration = integration; 
}

/*!
    \internal

    Returns an interface to the icon cache used by the form editor to
    manage icons.
*/
QDesignerIconCacheInterface *QDesignerFormEditorInterface::iconCache() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_iconCache; 
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setIconCache(QDesignerIconCacheInterface *cache)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_iconCache = cache; 
}

/*!
    \internal

    Returns the plugin manager used by the form editor.
*/
QDesignerPluginManager *QDesignerFormEditorInterface::pluginManager() const
{ 
    Q_D(const QDesignerFormEditorInterface);
    return d->m_pluginManager; 
}

/*!
    \internal

    Sets the plugin manager used by the form editor to the specified
    \a pluginManager.
*/
void QDesignerFormEditorInterface::setPluginManager(QDesignerPluginManager *pluginManager)
{ 
    Q_D(QDesignerFormEditorInterface);
    d->m_pluginManager = pluginManager; 
}

/*!
    \internal

    Returns the path to the resources used by the form editor.
*/
QString QDesignerFormEditorInterface::resourceLocation() const
{
#ifdef Q_WS_MAC
    return QLatin1String(":/trolltech/formeditor/images/mac");
#else
    return QLatin1String(":/trolltech/formeditor/images/win");
#endif
}

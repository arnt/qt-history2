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

#include "qextensionmanager.h"

/*!
    \class QExtensionManager

    \brief The QExtensionManager class provides extension management
    facilities for Qt Designer.

    \inmodule QtDesigner

    In \QD the extensions are not created until they are required. For
    that reason you must, when implementing an extension, also create
    a QExtensionFactory, i.e a class that is able to make an instance
    of your extension, and register it using a QExtensionManager.

    The registration of an extension factory is typically made in the
    QDesignerCustomWidgetInterface::initialize() function:

    \code
        void MyPlugin::initialize(QDesignerFormEditorInterface *formEditor)
        {
            if (initialized)
                return;

            QExtensionManager *manager = formEditor->extensionManager();
            Q_ASSERT(manager != 0);

            manager->registerExtensions(new MyExtensionFactory(manager),
                                        Q_TYPEID(QDesignerTaskMenuExtension));

            initialized = true;
        }
    \endcode

    The QDesignerCustomWidgetInterface class is used to expose the
    custom widget to \QD. The \l
    {QDesignerCustomWidgetInterface::initialize()}{initialize()}
    function is called after the plugin is loaded into Qt
    Designer. The function's QDesignerFormWindowInterface parameter
    provides the plugin with a gateway to all of \QD's API's; we can
    use this parameter and retrieve \QD's extension manager to
    register our extension factory.

    Then, when an extension is required, \QD will run through all its
    registered factories calling QExtensionFactory::createExtension()
    for each until the first one that is able to create the requested
    extension for the selected widget, is found. This factory will
    then create the extension.

    There are four available types of extensions in \QD:
    QDesignerContainerExtension , QDesignerMemberSheetExtension,
    QDesignerPropertySheetExtension and
    QDesignerTaskMenuExtension. \QD's behavior is the same whether the
    requested extension is associated with a container, a member
    sheet, a property sheet or a task menu.

    For a complete example using the QExtensionManager class, see the
    \l {designer/taskmenuextension}{Task Menu Extension example}. The
    example shows how to create a custom widget plugin for Qt
    Designer, and how to to use the QDesignerTaskMenuExtension class
    to add custom items to \QD's task menu.

    \sa QExtensionFactory, QAbstractExtensionManager
*/

/*!
    Constructs an extension manager with the given \a parent.
*/
QExtensionManager::QExtensionManager(QObject *parent)
    : QObject(parent)
{
}

/*!
    Register the given extension \a factory specified by \a iid.
*/
void QExtensionManager::registerExtensions(QAbstractExtensionFactory *factory, const QString &iid)
{
    if (iid.isEmpty()) {
        m_globalExtension.append(factory);
        return;
    }

    if (!m_extensions.contains(iid))
        m_extensions.insert(iid, QList<QAbstractExtensionFactory*>());

    m_extensions[iid].prepend(factory);
}

/*!
    Unregister the given \a factory specified by \a iid.
*/
void QExtensionManager::unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid)
{
    if (iid.isEmpty()) {
        m_globalExtension.removeAll(factory);
    } else if (m_extensions.contains(iid)) {
        QList<QAbstractExtensionFactory*> &factories = m_extensions[iid];
        factories.removeAll(factory);

        if (factories.isEmpty())
            m_extensions.remove(iid);
    }
}

/*!
    Returns the extension, specified by \a iid, for the given \a
    object.
*/
QObject *QExtensionManager::extension(QObject *object, const QString &iid) const
{
    QList<QAbstractExtensionFactory*> l = m_extensions.value(iid);
    l += m_globalExtension;

    foreach (QAbstractExtensionFactory *factory, l) {
        if (QObject *ext = factory->extension(object, iid))
            return ext;
    }

    return 0;
}

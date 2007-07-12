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

#include "qextensionmanager.h"

/*!
    \class QExtensionManager

    \brief The QExtensionManager class provides extension management
    facilities for Qt Designer.

    \inmodule QtDesigner

    In \QD the extensions are not created until they are required. For
    that reason, when implementing an extension, you must also create
    a QExtensionFactory, i.e a class that is able to make an instance
    of your extension, and register it using \QD's extension manager.

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

    The QExtensionManager is not intended to be instantiated
    directly. You can retrieve an interface to \QD's extension manager
    using the QDesignerFormEditorInterface::extensionManager()
    function. A pointer to \QD's current QDesignerFormEditorInterface
    object (\c formEditor in the example above) is provided by the
    QDesignerCustomWidgetInterface::initialize() function's
    parameter. When implementing a custom widget plugin, you must
    subclass the QDesignerCustomWidgetInterface to expose your plugin
    to \QD.

    Then, when an extension is required, \QD's extension manager will
    run through all its registered factories calling
    QExtensionFactory::createExtension() for each until the first one
    that is able to create the requested extension for the selected
    object, is found. This factory will then make an instance of the
    extension.

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
  Destroys the extension manager
*/
QExtensionManager::~QExtensionManager()
{
}

/*!
    Register the extension specified by the given \a factory and
    extension identifier \a iid.
*/
void QExtensionManager::registerExtensions(QAbstractExtensionFactory *factory, const QString &iid)
{
    if (iid.isEmpty()) {
        m_globalExtension.prepend(factory);
        return;
    }

    FactoryMap::iterator it = m_extensions.find(iid);
    if (it == m_extensions.end())
        it = m_extensions.insert(iid, FactoryList());

    it.value().prepend(factory);
}

/*!
    Unregister the extension specified by the given \a factory and
    extension identifier \a iid.
*/
void QExtensionManager::unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid)
{
    if (iid.isEmpty()) {
        m_globalExtension.removeAll(factory);
        return;
    }

    const FactoryMap::iterator it = m_extensions.find(iid);
    if (it == m_extensions.end())
        return;

    FactoryList &factories = it.value();
    factories.removeAll(factory);

    if (factories.isEmpty())
        m_extensions.erase(it);
}

/*!
    Returns the extension specified by \a iid, for the given \a
    object.
*/
QObject *QExtensionManager::extension(QObject *object, const QString &iid) const
{
    const FactoryMap::const_iterator it = m_extensions.constFind(iid);
    if (it != m_extensions.constEnd()) {
        const FactoryList::const_iterator fcend = it.value().constEnd();
        for (FactoryList::const_iterator fit = it.value().constBegin(); fit != fcend; ++fit)
            if (QObject *ext = (*fit)->extension(object, iid))
                return ext;
    }
    const FactoryList::const_iterator gfcend =  m_globalExtension.constEnd();
    for (FactoryList::const_iterator git = m_globalExtension.constBegin(); git != gfcend; ++git)
        if (QObject *ext = (*git)->extension(object, iid))
            return ext;

    return 0;
}

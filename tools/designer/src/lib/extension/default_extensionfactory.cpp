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

#include <QtDesigner/default_extensionfactory.h>
#include "qextensionmanager.h"
#include <qpointer.h>
#include <QtCore/qdebug.h>

/*!
    \class QExtensionFactory

    \brief The QExtensionFactory class allows you to create a factory
    that is able to make instances of plugin extensions in Qt
    Designer.

    \inmodule QtDesigner

    In Qt Designer the extensions are not created until they are
    required. For that reason, when implementing a custom extension,
    you must also create a QExtensionFactory, i.e a class that is able
    to make an instance of your extension, and register it using a
    QExtensionManager.

    The QExtensionManager class provides extension management
    facilities for Qt Designer. When an extension is required, Qt
    Designer will run through all its registered factories calling
    QExtensionFactory::createExtension() for each until the first one
    that is able to create a requested extension, is found. This
    factory will then make an instance of the extension for the
    plugin.

    There are four available types of extensions in Qt Designer:
    QDesignerContainerExtension , QDesignerMemberSheetExtension,
    QDesignerPropertySheetExtension and QDesignerTaskMenuExtension. Qt
    Designer's behavior is the same whether the requested extension is
    associated with a multi page container, a member sheet, a property
    sheet or a task menu.

    You can either create a new QExtensionFactory and reimplement the
    QExtensionFactory::createExtension() function. For example:

    \code
        QObject *ANewExtensionFactory::createExtension(QObject *object,
                const QString &iid, QObject *parent) const
        {
            if (iid != Q_TYPEID(QDesignerContainerExtension))
                return 0;

            if (MyCustomWidget *widget = qobject_cast<MyCustomWidget*>
                   (object))
                return new MyContainerExtension(widget, parent);

            return 0;
        }
    \endcode

    Or you can use an existing factory, expanding the
    QExtensionFactory::createExtension() function to make the factory
    able to create your extension as well. For example:

    \code
        QObject *AGeneralExtensionFactory::createExtension(QObject *object,
                const QString &iid, QObject *parent) const
        {
            if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
                if (MyCustomWidget *widget = qobject_cast<MyCustomWidget*>
                        (object))
                    return new MyTaskMenuExtension(widget, parent);

            } else if (iid == Q_TYPEID(QDesignerContainerExtension)) {
                if (MyCustomWidget *widget = qobject_cast<MyCustomWidget*>
                        (object))
                    return new MyContainerExtension(widget, parent);

            } else {
                return 0;
            }
        }
    \endcode

    For a complete example using the QExtensionFactory class, see the
    \l {designer/taskmenuextension}{Task Menu Extension example}. The
    example shows how to create a custom widget plugin for Qt
    Designer, and how to to use the QDesignerTaskMenuExtension class
    to add custom items to Qt Designer's task menu.

    \sa QExtensionManager, QAbstractExtensionFactory, qt_extension()
*/

/*!
    Constructs an extension factory with the given \a parent.
*/
QExtensionFactory::QExtensionFactory(QExtensionManager *parent)
    : QObject(parent)
{
}

/*!
    Returns the extension specified by \a iid for the given \a object.

    \sa createExtension()
*/

QObject *QExtensionFactory::extension(QObject *object, const QString &iid) const
{
    if (!object)
        return 0;

    QPair<QString, QObject*> key = qMakePair(iid, object);
    if (!m_extensions.contains(key)) {
        if (QObject *ext = createExtension(object, iid, const_cast<QExtensionFactory*>(this))) {
            connect(ext, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
            m_extensions.insert(key, ext);
        }
    }

    if (!m_extended.contains(object)) {
        connect(object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
        m_extended.insert(object, true);
    }

    return m_extensions.value(key);
}

void QExtensionFactory::objectDestroyed(QObject *object)
{
    QMutableMapIterator< QPair<QString,QObject*>, QObject*> it(m_extensions);
    while (it.hasNext()) {
        it.next();

        QObject *o = it.key().second;
        if (o == object || object == it.value()) {
            it.remove();
        }
    }

    m_extended.remove(object);
}

/*!
    Returns an extension specified by \a iid for the given \a object.
    The extension object is created as a child of the specified \a parent.

    \sa extension()
*/
QObject *QExtensionFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    Q_UNUSED(object);
    Q_UNUSED(iid);
    Q_UNUSED(parent);

    return 0;
}

/*!
    Returns the extension manager for the extension factory.
*/
QExtensionManager *QExtensionFactory::extensionManager() const
{
    return static_cast<QExtensionManager *>(parent());
}

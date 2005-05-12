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

#include "qaccessiblebridge.h"

/*!
    \class QAccessibleBridge

    \ingroup accessibility

*/

/*!
    \fn QAccessibleBridge::~QAccessibleBridge()

*/

/*!
    \fn void QAccessibleBridge::setRootObject(QAccessibleInterface *)
*/

/*!
    \fn void QAccessibleBridge::notifyAccessibilityUpdate(int, QAccessibleInterface *, int)
*/

/*!
    \class QAccessibleBridgePlugin
    \brief The QAccessibleBridgePlugin class provides an abstract
    base for accessibility bridge plugins.

    \ingroup plugins
    \ingroup accessibility

    Writing an accessibility bridge plugin is achieved by subclassing
    this base class, reimplementing the pure virtual functions keys()
    and create(), and exporting the class with the \c
    Q_EXPORT_PLUGIN() macro.

    \sa QAccessibleBridge, QAccessiblePlugin, {How to Write Qt Plugins}
*/

/*!
    Constructs an accessibility bridge plugin with the given \a
    parent. This is invoked automatically by the \c Q_EXPORT_PLUGIN()
    macro.
*/
QAccessibleBridgePlugin::QAccessibleBridgePlugin(QObject *parent): QObject(parent)
{

}

/*!
    Destroys the accessibility bridge plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QAccessibleBridgePlugin::~QAccessibleBridgePlugin()
{

}

/*!
    \fn QStringList QAccessibleBridgePlugin::keys() const

    Returns the list of keys this plugins supports.

    These keys must be the names of the bridges that this
    plugin provides.

    \sa create()
*/

/*!
    \fn QAccessibleBridge *QAccessibleBridgePlugin::create(const QString &key)

    Creates and returns the QAccessibleBridge object corresponding to
    the given \a key.

    \sa keys()
*/

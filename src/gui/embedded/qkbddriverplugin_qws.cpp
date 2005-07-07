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

#include "qkbddriverplugin_qws.h"

#ifndef QT_NO_LIBRARY

#include "qkbd_qws.h"

/*!
    \class QKbdDriverPlugin
    \brief The QKbdDriverPlugin class provides an abstract base for
    Qtopia Core keyboard driver plugins.

    \ingroup plugins

    The keyboard driver plugin is a simple plugin interface that makes
    it easy to create custom keyboard drivers.

    Writing a keyboard driver plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys() and
    create(), and exporting the class with the Q_EXPORT_PLUGIN()
    macro. See \l{How to Create Qt Plugins} for details.

    This class is only available in Qtopia Core.
*/

/*!
    \fn QStringList QKbdDriverPlugin::keys() const

    Returns the list of keyboard drivers this plugin supports.

    \sa create()
*/

/*!
    Constructs a keyboard driver plugin with the given \a parent. This
    is invoked automatically by the Q_EXPORT_PLUGIN() macro.
*/
QKbdDriverPlugin::QKbdDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the keyboard driver plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QKbdDriverPlugin::~QKbdDriverPlugin()
{
}

/*!
    \fn QScreen *QKbdDriverPlugin::create(const QString &driver, const QString &device)

    Creates a driver matching the type specified by \a driver and \a device.

    \sa keys()
*/

#endif // QT_NO_LIBRARY

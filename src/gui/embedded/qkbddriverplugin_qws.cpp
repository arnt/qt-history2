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

#include "qkbddriverplugin_qws.h"

#ifndef QT_NO_LIBRARY

#include "qkbd_qws.h"

/*!
    \class QKbdDriverPlugin
    \ingroup plugins
    \ingroup qws

    \brief The QKbdDriverPlugin class is an abstract base class for
    keyboard driver plugins.

    Note that this class is only available in \l {Qtopia Core}.

    QKbdDriverPlugin is a simple plugin interface that makes it easy
    to create custom keyboard drivers that can be loaded dynamically
    into applications using the QKbdDriverFactory class.

    Writing a custom keyboard driver plugin is achieved by subclassing
    QKbdDriverPlugin, reimplementing the pure virtual keys() and
    create() functions, and exporting the class using the
    Q_EXPORT_PLUGIN2() macro (See \l{How to Create Qt Plugins} for
    details).

    \sa QKbdDriverFactory, QWSKeyboardHandler, {Qtopia Core Character Input}
*/

/*!
    \fn QStringList QKbdDriverPlugin::keys() const

    Returns the list of valid keys, i.e. the keyboard drivers
    supported by this plugin.

    \l {Qtopia Core} currently supports the following drivers by
    default: \c SL5000, \c Yopy, \c VR41xx, \c TTY and \c USB.

    \sa create()
*/

/*!
    Constructs a keyboard driver plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QKbdDriverPlugin::QKbdDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the keyboard driver plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QKbdDriverPlugin::~QKbdDriverPlugin()
{
}

/*!
    \fn QScreen *QKbdDriverPlugin::create(const QString &key, const QString &device)

    Creates a driver matching the type specified by the given \a
    key and \a device parameters. Keys are case-insensitive.

    \sa keys()
*/

#endif // QT_NO_LIBRARY

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

#include "qmousedriverplugin_qws.h"

#ifndef QT_NO_LIBRARY

#include "qmouse_qws.h"

/*!
    \class QMouseDriverPlugin
    \ingroup plugins
    \ingroup qws

    \brief The QMouseDriverPlugin class is an abstract base class for
    mouse driver plugins.

    Note that this class is only available in \l {Qtopia Core}.

    \l {Qtopia Core} provides ready-made drivers for several mouse
    protocols, see the \l {Qtopia Core Pointer Handling}{pointer
    handling} documentation for details. Custom mouse drivers can be
    implemented by subclassing the QWSMouseHandler class and creating
    a mouse driver plugin.

    A mouse driver plugin can be created by subclassing
    QMouseDriverPlugin and reimplementing the pure virtual keys() and
    create() functions. By exporting the derived class using the
    Q_EXPORT_PLUGIN2() macro, \l {Qtopia Core}'s implementation of the
    QMouseDriverFactory class will automatically detect the plugin and
    load the driver into the server application at runtime. See \l
    {How to Create Qt Plugins} for details.

    \sa QWSMouseHandler, QMouseDriverFactory
*/

/*!
    \fn QStringList QMouseDriverPlugin::keys() const

    Returns the list of valid keys, i.e. the mouse drivers supported
    by this plugin.

    \l {Qtopia Core} provides ready-made drivers for several mouse
    protocols, see the \l {Qtopia Core Pointer Handling}{pointer
    handling} documentation for details.

    \sa create()
*/

/*!
    Constructs a mouse driver plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QMouseDriverPlugin::QMouseDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the mouse driver plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QMouseDriverPlugin::~QMouseDriverPlugin()
{
}

/*!
    \fn QScreen* QMouseDriverPlugin::create(const QString &key, const QString& device)

    Creates a driver matching the type specified by the given \a key
    and \a device parameters. Note that keys are case-insensitive.

    \sa keys()
*/

#endif // QT_NO_LIBRARY

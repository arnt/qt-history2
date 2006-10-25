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

    QMouseDriverPlugin is a simple plugin interface that makes it easy
    to create custom mouse drivers that can be loaded dynamically
    into applications using the QMouseDriverFactory class.

    Writing a custom mouse driver plugin is achieved by subclassing
    QMouseDriverPlugin, reimplementing the pure virtual keys() and
    create() functions, and exporting the class with the
    Q_EXPORT_PLUGIN2() macro. See \l{How to Create Qt Plugins} for
    details.

    \sa QMouseDriverFactory, QWSMouseHandler, {Qtopia Core Pointer
    Handling}
*/

/*!
    \fn QStringList QMouseDriverPlugin::keys() const

    Returns the list of valid keys, i.e. mouse drivers supported by this plugin.

    \l {Qtopia Core} currently supports the following drivers by
    default: \c MouseMan, \c IntelliMouse, \c Microsoft, \c VR41xx, \c
    LinuxTP, \c Yopy and \c Tslib.

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
    and \a device parameters. Keys are case-insensitive.

    \sa keys()
*/

#endif // QT_NO_LIBRARY

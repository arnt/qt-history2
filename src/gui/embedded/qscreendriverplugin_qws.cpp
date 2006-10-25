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

#include "qscreendriverplugin_qws.h"

#ifndef QT_NO_LIBRARY

/*!
    \class QScreenDriverPlugin
    \ingroup plugins
    \ingroup qws

    \brief The QScreenDriverPlugin class is an abstract base class for
    graphics driver plugins.

    Note that this class is only available in \l {Qtopia Core}.

    QScreenDriverPlugin is a simple plugin interface that makes it easy
    to create custom graphics drivers that can be loaded dynamically into
    applications using the QScreenDriverFactory class.

    Writing a custom graphics driver plugin is achieved by subclassing
    QScreenDriverPlugin, reimplementing the pure virtual keys() and
    create() functions, and exporting the class using the
    Q_EXPORT_PLUGIN2() macro. See \l{How to Create Qt Plugins} for
    details.

    \sa QScreen, QScreenDriverFactory, {Running Qtopia Core Applications}
*/

/*!
    \fn QStringList QScreenDriverPlugin::keys() const

    Returns the list of valid keys, i.e. the screen drivers supported
    by this plugin.

    \l {Qtopia Core} currently supports the following drivers by
    default: \c QVfb (\l {Qtopia Core's virtual framebuffer}), \c
    LinuxFb (The Linux framebuffer), \c Transformed (for rotated
    displays) and \c VNC (a \l {The VNC Protocol and Qtopia Core}{VNC}
    server).

    \sa create()
*/

/*!
    Constructs a graphics driver plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QScreenDriverPlugin::QScreenDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys this graphics driver plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QScreenDriverPlugin::~QScreenDriverPlugin()
{
}


/*!
    \fn QScreen* QScreenDriverPlugin::create(const QString &key, int displayId)

    Creates a plugin matching the driver specified by the given \a key,
    using the display specified by \a displayId. Keys are case-insensitive.

    \sa keys()
*/

#endif // QT_NO_LIBRARY

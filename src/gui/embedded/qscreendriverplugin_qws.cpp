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
    screen driver plugins in Qtopia Core.

    Note that this class is only available in \l {Qtopia Core}.

    \l {Qtopia Core} provides ready-made drivers for several screen
    protocols, see the \l {Qtopia Core Display Management}{display
    management} documentation for details. Custom screen drivers can be
    implemented by subclassing the QScreen class and creating a screen
    driver plugin.

    A screen driver plugin can be created by subclassing
    QScreenDriverPlugin and reimplementing the pure virtual keys() and
    create() functions. By exporting the derived class using the
    Q_EXPORT_PLUGIN2() macro, \l {Qtopia Core}'s implementation of the
    QScreenDriverFactory class will automatically detect the plugin
    and load the driver into the server application at runtime.  See
    \l{How to Create Qt Plugins} for details.

    \sa QScreen, QScreenDriverFactory
*/

/*!
    \fn QStringList QScreenDriverPlugin::keys() const

    Implement this function to return the list of valid keys, i.e. the
    screen drivers supported by this plugin.

    \l {Qtopia Core} provides ready-made drivers for several screen
    protocols, see the \l {Qtopia Core Display Management}{display
    management} documentation for details.

    \sa create()
*/

/*!
    Constructs a screen driver plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QScreenDriverPlugin::QScreenDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys this screen driver plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QScreenDriverPlugin::~QScreenDriverPlugin()
{
}


/*!
    \fn QScreen* QScreenDriverPlugin::create(const QString &key, int displayId)

    Implement this function to create a driver matching the type
    specified by the given \a key and \a displayId parameters. Note
    that keys are case-insensitive.

    \sa keys()
*/

#endif // QT_NO_LIBRARY

/****************************************************************************
**
** Implementation of QMouseDriverPlugin.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmousedriverplugin_qws.h"

#ifndef QT_NO_COMPONENT

#include "qmouse_qws.h"

/*!
    \class QMouseDriverPlugin qmousedriverplugin_qws.h
    \brief The QMouseDriverPlugin class provides an abstract base for
    Qt/Embedded mouse driver plugins.

    \ingroup plugins

    The mouse driver plugin is a simple plugin interface that makes it
    easy to create custom mouse drivers.

    Writing a mouse driver plugin is achieved by subclassing this base
    class, reimplementing the pure virtual functions keys() and
    create(), and exporting the class with the \c Q_EXPORT_PLUGIN
    macro. See the \link plugins-howto.html Plugins
    documentation\endlink for details.

    This class is only available in Qt/Embedded.
*/

/*!
    \fn QStringList QMouseDriverPlugin::keys() const

    Returns the list of mouse drivers this plugin supports.

    \sa create()
*/


/*!
    Constructs a mouse driver plugin with the given \a parent. This is
    invoked automatically by the \c Q_EXPORT_PLUGIN macro.
*/
QMouseDriverPlugin::QMouseDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the mouse driver plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QMouseDriverPlugin::~QMouseDriverPlugin()
{
}


/*!
    \fn QScreen* QMouseDriverPlugin::create(const QString &driver, const QString& device)

    Creates a driver matching the type specified by \a driver and
    which uses device \a device.

    \sa keys()
*/

#endif // QT_NO_COMPONENT

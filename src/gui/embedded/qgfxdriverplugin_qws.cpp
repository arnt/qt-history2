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

#include "qgfxdriverplugin_qws.h"

#ifndef QT_NO_COMPONENT

/*!
    \class QGfxDriverPlugin
    \brief The QGfxDriverPlugin class provides an abstract base for
    Qt/Embedded graphics driver plugins.

    \ingroup plugins
    \ingroup qws

    The graphics driver plugin is a simple plugin interface that makes
    it easy to create custom graphics drivers.

    Writing a graphics driver plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys() and
    create(), and exporting the class with the \c Q_EXPORT_PLUGIN()
    macro. See \l{How to Create Qt Plugins} for details.

    This class is only available in Qt/Embedded.
*/

/*!
    \fn QStringList QGfxDriverPlugin::keys() const

    Returns the list of graphics drivers this plugin supports.

    \sa create()
*/

/*!
    Constructs a graphics driver plugin with the given \a parent. This
    is invoked automatically by the \c Q_EXPORT_PLUGIN() macro.
*/
QGfxDriverPlugin::QGfxDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the graphics driver plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QGfxDriverPlugin::~QGfxDriverPlugin()
{
}


/*!
    \fn QScreen* QGfxDriverPlugin::create(const QString &driver, int displayId)

    Creates a driver matching the type specified by \a driver, that
    will use display \a displayId.

    \sa keys()
*/

#endif // QT_NO_COMPONENT

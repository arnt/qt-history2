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

#include "qiconengineplugin.h"
#include "qiconengine.h"

/*!
    \class QIconEnginePlugin
    \brief The QIconEnginePlugin class provides an abstract base for custom QIconEngine plugins.

    \ingroup plugins

    \bold {Use QIconEnginePluginV2 instead.}

    The icon engine plugin is a simple plugin interface that makes it easy to
    create custom icon engines that can be loaded dynamically into applications
    through QIcon. QIcon uses the file or resource name's suffix to determine
    what icon engine to use.

    Writing a icon engine plugin is achieved by subclassing this base class,
    reimplementing the pure virtual functions keys() and create(), and
    exporting the class with the Q_EXPORT_PLUGIN2() macro.

    \sa {How to Create Qt Plugins}
*/

/*!
    \fn QStringList QIconEnginePlugin::keys() const

    Returns a list of icon engine keys that this plugin supports. The keys correspond
    to the suffix of the file or resource name used when the plugin was created.
    Keys are case insensitive.

    \sa create()
*/

/*!
    \fn QIconEngine* QIconEnginePlugin::create(const QString& filename)

    Creates and returns a QIconEngine object for the icon with the given
    \a filename.

    \sa keys()
*/

/*!
    Constructs a icon engine plugin with the given \a parent. This is invoked
    automatically by the Q_EXPORT_PLUGIN2() macro.
*/
QIconEnginePlugin::QIconEnginePlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the icon engine plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QIconEnginePlugin::~QIconEnginePlugin()
{
}

// version 2

/*!
    \class QIconEnginePluginV2
    \brief The QIconEnginePluginV2 class provides an abstract base for custom QIconEngineV2 plugins.

    \ingroup plugins
    \since 4.3

    The icon engine plugin is a simple plugin interface that makes it easy to
    create custom icon engines that can be loaded dynamically into applications
    through QIcon. QIcon uses the file or resource name's suffix to determine
    what icon engine to use.

    Writing a icon engine plugin is achieved by subclassing this base class,
    reimplementing the pure virtual functions keys() and create(), and
    exporting the class with the Q_EXPORT_PLUGIN2() macro.

    \sa {How to Create Qt Plugins}
*/

/*!
    \fn QStringList QIconEnginePluginV2::keys() const

    Returns a list of icon engine keys that this plugin supports. The keys correspond
    to the suffix of the file or resource name used when the plugin was created.
    Keys are case insensitive.

    \sa create()
*/

/*!
    \fn QIconEngineV2* QIconEnginePluginV2::create(const QString& filename = QString())

    Creates and returns a QIconEngine object for the icon with the given
    \a filename.

    \sa keys()
*/

/*!
    Constructs a icon engine plugin with the given \a parent. This is invoked
    automatically by the Q_EXPORT_PLUGIN2() macro.
*/
QIconEnginePluginV2::QIconEnginePluginV2(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the icon engine plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QIconEnginePluginV2::~QIconEnginePluginV2()
{
}

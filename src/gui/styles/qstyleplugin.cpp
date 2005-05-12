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

#include "qstyleplugin.h"
#include "qstyle.h"

/*!
    \class QStylePlugin
    \brief The QStylePlugin class provides an abstract base for custom QStyle plugins.

    \ingroup plugins

    The style plugin is a simple plugin interface that makes it easy
    to create custom styles that can be loaded dynamically into
    applications with a QStyleFactory.

    Writing a style plugin is achieved by subclassing this base class,
    reimplementing the pure virtual functions keys() and create(), and
    exporting the class with the \c Q_EXPORT_PLUGIN() macro.

    \sa {How to Create Qt Plugins}
*/

/*!
    \fn QStringList QStylePlugin::keys() const

    Returns the list of style keys this plugin supports.

    These keys are usually the class names of the custom styles that
    are implemented in the plugin.

    \sa create()
*/

/*!
    \fn QStyle* QStylePlugin::create(const QString& key)

    Creates and returns a QStyle object for the style key \a key. The
    style key is usually the class name of the required style.

    \sa keys()
*/

/*!
    Constructs a style plugin with parent \a parent. This is invoked automatically by the
    \c Q_EXPORT_PLUGIN() macro.
*/
QStylePlugin::QStylePlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the style plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QStylePlugin::~QStylePlugin()
{
}

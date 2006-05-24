/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdecorationplugin_qws.h"
#include "qdecoration_qws.h"

/*!
    \class QDecorationPlugin
    \ingroup qws
    \ingroup plugins

    \brief The QDecorationPlugin class is an abstract base class for
    decoration plugins.

    Note that this class is only available in \l {Qtopia Core}.

    QDecorationPlugin is a simple plugin interface that makes it easy
    to create custom decorations that can be loaded dynamically into
    applications using the QDecorationFactory class.

    Writing a decoration plugin is achieved by subclassing
    QDecorationPlugin, reimplementing the pure virtual keys() and
    create() functions, and exporting the class using the
    Q_EXPORT_PLUGIN2() macro. See \l{How to Create Qt Plugins} for
    details.

    \sa QDecorationFactory, QDecoration
*/

/*!
    \fn QStringList QDecorationPlugin::keys() const

    Returns the list of valid keys, i.e. the decoration keys supported
    by this plugin.

    A key is usually the class name of a custom decoration.  \l
    {Qtopia Core} currently supports the following decorations by
    default: \c Default, \c Styled and \c Windows.

    \sa create()
*/

/*!
    \fn QDecoration *QDecorationPlugin::create(const QString &key)

    Creates a QDecoration object for the given decoration \a key. Keys
    are case-insensitive.

    \sa keys()
*/

/*!
    Constructs a decoration plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QDecorationPlugin::QDecorationPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the decoration plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QDecorationPlugin::~QDecorationPlugin()
{
}

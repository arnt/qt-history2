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

#include "qiconengineplugin.h"
#include "qiconengine.h"

/*!
    \class QIconEnginePlugin qiconengineplugin.h
    \brief The QIconEnginePlugin class provides an abstract base for custom QIconEngine plugins.

    \ingroup plugins

    The icon engine plugin is a simple plugin interface that makes it
    easy to create custom icon engines that can be loaded dynamically
    into applications through QIcon. QIcon uses the file or resource
    name's suffix to determine what icon engine to use.

    Writing a icon engine plugin is achieved by subclassing this base class,
    reimplementing the pure virtual functions keyss() and create(), and
    exporting the class with the \c Q_EXPORT_PLUGIN macro. See the
    \link plugins-howto.html plugins documentation\endlink for an
    example.
*/

/*!
    \fn QStringList QIconEnginePlugin::keys() const

    Returns the list of icon engine keys this plugin supports.  The
    keys are the file or resource name's suffix.

    \sa create()
*/

/*!
    \fn QIconEngine* QIconEnginePlugin::create(const QString& filename)

    Creates and returns a QIconEngine object for the icon request with
    \a filename.

    \sa keys()
*/

/*!
    Constructs a icon engine plugin with parent \a parent. This is invoked automatically by the
    \c Q_EXPORT_PLUGIN macro.
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


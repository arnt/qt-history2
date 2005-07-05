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

#include "qpictureformatplugin.h"
#if !defined(QT_NO_LIBRARY) && !defined(QT_NO_PICTURE)
#include "qpicture.h"

/*!
    \class QPictureFormatPlugin
    \brief The QPictureFormatPlugin class provides an abstract base for custom picture format plugins.

    \ingroup plugins

    The picture format plugin is a simple plugin interface that makes
    it easy to create custom picture formats that can be used
    transparently by applications.

    Writing an picture format plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys(),
    loadPicture(), savePicture(), and installIOHandler(), and
    exporting the class with the Q_EXPORT_PLUGIN() macro.

    \sa {How to Create Qt Plugins}
*/

/*!
    \fn QStringList QPictureFormatPlugin::keys() const

    Returns the list of picture formats this plugin supports.

    \sa installIOHandler()
*/

/*!
    \fn  bool QPictureFormatPlugin::installIOHandler(const QString &format)

    Installs a QPictureIO picture I/O handler for the picture format \a
    format.

    \sa keys()
*/


/*!
    Constructs an picture format plugin with the given \a parent.
    This is invoked automatically by the Q_EXPORT_PLUGIN() macro.
*/
QPictureFormatPlugin::QPictureFormatPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the picture format plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QPictureFormatPlugin::~QPictureFormatPlugin()
{
}


/*!\internal
 */
bool QPictureFormatPlugin::loadPicture(const QString &format, const QString &filename, QPicture *pic)
{
    Q_UNUSED(format)
    Q_UNUSED(filename)
    Q_UNUSED(pic)
    return false;
}

/*! \internal
 */
bool QPictureFormatPlugin::savePicture(const QString &format, const QString &filename, const QPicture &pic)
{
    Q_UNUSED(format)
    Q_UNUSED(filename)
    Q_UNUSED(pic)
    return false;
}

#endif // QT_NO_LIBRARY || QT_NO_PICTURE

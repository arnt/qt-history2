/****************************************************************************
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

#include "qimageformatplugin.h"
#ifndef QT_NO_IMAGEFORMATPLUGIN
#include "qimage.h"

/*!
    \class QImageFormatPlugin qimageformatplugin.h
    \brief The QImageFormatPlugin class provides an abstract base for custom image format plugins.

    \ingroup plugins

    The image format plugin is a simple plugin interface that makes
    it easy to create custom image formats that can be used
    transparently by applications.

    Writing an image format plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys() and
    installIOHandler(), and exporting the class with the
    Q_EXPORT_PLUGIN macro.  See the \link plugins-howto.html Plugins
    documentation\endlink for details.
*/

/*!
    \fn QStringList QImageFormatPlugin::keys() const

    Returns the list of image formats this plugin supports.

    \sa installIOHandler()
*/


/*!
    \fn  bool QImageFormatPlugin::installIOHandler(const QString &format)

    Installs a QImageIO image I/O handler for the image format \a
    format.

    \sa keys()
*/


/*!
    Constructs an image format plugin with the given \a parent. This
    is invoked automatically by the Q_EXPORT_PLUGIN macro.
*/
QImageFormatPlugin::QImageFormatPlugin(QObject* parent)
    :QObject(parent)
{
}

/*!
    Destroys the image format plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QImageFormatPlugin::~QImageFormatPlugin()
{
}


/*!\internal
 */
bool QImageFormatPlugin::loadImage(const QString &format, const QString &filename, QImage *image)
{
    Q_UNUSED(format)
    Q_UNUSED(filename)
    Q_UNUSED(image)
    return false;
}

/*! \internal
 */
bool QImageFormatPlugin::saveImage(const QString &format, const QString &filename, const QImage &image)
{
    Q_UNUSED(format)
    Q_UNUSED(filename)
    Q_UNUSED(image)
    return false;
}

#endif // QT_NO_IMAGEFORMATPLUGIN

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

#include "qpictureformatplugin.h"
#ifndef QT_NO_PICTUREFORMATPLUGIN
#include "qpictureformatinterface_p.h"
#include "qpicture.h"

/*!
    \class QPictureFormatPlugin qpictureformatplugin.h
    \brief The QPictureFormatPlugin class provides an abstract base for custom picture format plugins.

    \ingroup plugins

    The picture format plugin is a simple plugin interface that makes
    it easy to create custom picture formats that can be used
    transparently by applications.

    Writing an picture format plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys() and
    installIOHandler(), and exporting the class with the
    Q_EXPORT_PLUGIN macro.  See the \link plugins-howto.html Plugins
    documentation\endlink for details.
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

class QPictureFormatPluginPrivate : public QPictureFormatInterface
{
public:
    QPictureFormatPluginPrivate(QPictureFormatPlugin *p)
        : plugin(p)
    {
    }
    virtual ~QPictureFormatPluginPrivate();

    QRESULT queryInterface(const QUuid &iid, QUnknownInterface **iface);
    Q_REFCOUNT;

    QStringList featureList() const;

    QRESULT loadPicture(const QString &format, const QString &filename, QPicture *);
    QRESULT savePicture(const QString &format, const QString &filename, const QPicture &);

    QRESULT installIOHandler(const QString &);

private:
    QPictureFormatPlugin *plugin;
};

QPictureFormatPluginPrivate::~QPictureFormatPluginPrivate()
{
    delete plugin;
}

QRESULT QPictureFormatPluginPrivate::queryInterface(const QUuid &iid, QUnknownInterface **iface)
{
    *iface = 0;

    if (iid == IID_QUnknown)
        *iface = this;
    else if (iid == IID_QFeatureList)
        *iface = this;
    else if (iid == IID_QPictureFormat)
        *iface = this;
    else
        return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList QPictureFormatPluginPrivate::featureList() const
{
    return plugin->keys();
}

QRESULT QPictureFormatPluginPrivate::loadPicture(const QString &format, const QString &filename, QPicture *pic)
{
    return plugin->loadPicture(format, filename, pic) ? QS_FALSE : QS_OK;
}

QRESULT QPictureFormatPluginPrivate::savePicture(const QString &format, const QString &filename, const QPicture &pic)
{
    return plugin->savePicture(format, filename, pic) ? QS_FALSE : QS_OK;
}

QRESULT QPictureFormatPluginPrivate::installIOHandler(const QString &format)
{
    return plugin->installIOHandler(format) ? QS_FALSE : QS_OK;
}

/*!
    Constructs an picture format plugin. This is invoked automatically
    by the Q_EXPORT_PLUGIN macro.
*/
QPictureFormatPlugin::QPictureFormatPlugin()
    : QGPlugin(d = new QPictureFormatPluginPrivate(this))
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

#endif // QT_NO_PICTUREFORMATPLUGIN

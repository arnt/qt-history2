/****************************************************************************
**
** Implementation of QKbdDriverPlugin.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qkbddriverplugin_qws.h"

#ifndef QT_NO_COMPONENT

#include "qkbddriverinterface_p.h"
#include "qkbd_qws.h"

/*!
    \class QKbdDriverPlugin qkbddriverplugin_qws.h
    \brief The QKbdDriverPlugin class provides an abstract base for
    Qt/Embedded keyboard driver plugins.

    \ingroup plugins

    The keyboard driver plugin is a simple plugin interface that makes
    it easy to create custom keyboard drivers.

    Writing a keyboard driver plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys() and
    create(), and exporting the class with the \c Q_EXPORT_PLUGIN
    macro. See the \link plugins-howto.html Plugins
    documentation\endlink for details.

    This class is only available in Qt/Embedded.
*/

/*!
    \fn QStringList QKbdDriverPlugin::keys() const

    Returns the list of keyboard drivers this plugin supports.

    \sa create()
*/


class QKbdDriverPluginPrivate : public QKbdDriverInterface
{
public:
    QKbdDriverPluginPrivate( QKbdDriverPlugin *p )
	: plugin( p )
    {
    }
    virtual ~QKbdDriverPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

#ifndef QT_NO_STRINGLIST
    QStringList featureList() const;
#endif

    QWSKeyboardHandler* create( const QString& driver, const QString &device );

private:
    QKbdDriverPlugin *plugin;
};

QKbdDriverPluginPrivate::~QKbdDriverPluginPrivate()
{
    delete plugin;
}

QRESULT QKbdDriverPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QKbdDriver )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

#ifndef QT_NO_STRINGLIST
QStringList QKbdDriverPluginPrivate::featureList() const
{
    return plugin->keys();
}
#endif

QWSKeyboardHandler* QKbdDriverPluginPrivate::create( const QString& driver, const QString &device )
{
    return plugin->create( driver, device );
}

/*!
    Constructs a keyboard driver plugin. This is invoked automatically
    by the \c Q_EXPORT_PLUGIN macro.
*/
QKbdDriverPlugin::QKbdDriverPlugin()
    : QGPlugin( d = new QKbdDriverPluginPrivate( this ) )
{
}

/*!
    Destroys the keyboard driver plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QKbdDriverPlugin::~QKbdDriverPlugin()
{
}


/*!
    \fn QScreen* QKbdDriverPlugin::create( const QString &driver, const QString &device )

    Creates a driver matching the type specified by \a driver and \a device.

    \sa keys()
*/

QWSKeyboardHandler* QKbdDriverPlugin::create( const QString& driver, const QString &device )
{
    Q_UNUSED( driver )
    Q_UNUSED( device )
    return 0;
}

#endif // QT_NO_COMPONENT

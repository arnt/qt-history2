/****************************************************************************
**
** Implementation of QSqlDriverPlugin class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qstyleplugin.h"

#ifndef QT_NO_STYLE
#ifndef QT_NO_COMPONENT

#include "qstyleinterface_p.h"
#include "qobjectcleanuphandler.h"
#include "qstyle.h"

/*!
    \class QStylePlugin qstyleplugin.h
    \brief The QStylePlugin class provides an abstract base for custom QStyle plugins.

    \ingroup plugins

    The style plugin is a simple plugin interface that makes it easy
    to create custom styles that can be loaded dynamically into
    applications with a QStyleFactory.

    Writing a style plugin is achieved by subclassing this base class,
    reimplementing the pure virtual functions keys() and create(), and
    exporting the class with the \c Q_EXPORT_PLUGIN macro. See the
    \link plugins-howto.html plugins documentation\endlink for an
    example.
*/

/*!
    \fn QStringList QStylePlugin::keys() const

    Returns the list of style keys this plugin supports.

    These keys are usually the class names of the custom styles that
    are implemented in the plugin.

    \sa create()
*/

/*!
    \fn QStyle* QStylePlugin::create( const QString& key )

    Creates and returns a QStyle object for the style key \a key. The
    style key is usually the class name of the required style.

    \sa keys()
*/

class QStylePluginPrivate : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    QStylePluginPrivate( QStylePlugin *p )
	: plugin( p )
    {
    }

    virtual ~QStylePluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QStyle *create( const QString &key );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QStylePlugin *plugin;
    QObjectCleanupHandler styles;
};

QRESULT QStylePluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = (QStyleFactoryInterface*)this;
    else if ( iid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( iid == IID_QStyleFactory )
	*iface = (QStyleFactoryInterface*)this;
    else if ( iid == IID_QLibrary )
	*iface = (QLibraryInterface*) this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStylePluginPrivate::~QStylePluginPrivate()
{
    delete plugin;
}

QStringList QStylePluginPrivate::featureList() const
{
    return plugin->keys();
}

QStyle *QStylePluginPrivate::create( const QString &key )
{
    QStyle *st = plugin->create( key );
    styles.add( st );
    return st;
}

bool QStylePluginPrivate::init()
{
    return TRUE;
}

void QStylePluginPrivate::cleanup()
{
    styles.clear();
}

bool QStylePluginPrivate::canUnload() const
{
    return styles.isEmpty();
}


/*!
    Constructs a style plugin. This is invoked automatically by the
    \c Q_EXPORT_PLUGIN macro.
*/
QStylePlugin::QStylePlugin()
    : QGPlugin( (QStyleFactoryInterface*)(d = new QStylePluginPrivate( this )) )
{
}

/*!
    Destroys the style plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QStylePlugin::~QStylePlugin()
{
    // don't delete d, as this is deleted by d
}

#endif // QT_NO_COMPONENT
#endif // QT_NO_STYLE

/****************************************************************************
**
** Implementation of QTextCodecPlugin class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextcodecplugin.h"
#ifndef QT_NO_TEXTCODECPLUGIN
#include "qtextcodecinterface_p.h"

/*!
    \class QTextCodecPlugin qtextcodecplugin.h
    \brief The QTextCodecPlugin class provides an abstract base for custom QTextCodec plugins.
    \reentrant
    \ingroup plugins

    The text codec plugin is a simple plugin interface that makes it
    easy to create custom text codecs that can be loaded dynamically
    into applications.

    Writing a text codec plugin is achieved by subclassing this base
    class, reimplementing the pure virtual functions names(),
    createForName(), mibEnums() and createForMib(), and exporting the
    class with the \c Q_EXPORT_PLUGIN macro. See the \link
    plugins-howto.html Qt Plugins documentation \endlink for details.

    See the \link http://www.iana.org/assignments/character-sets IANA
    character-sets encoding file\endlink for more information on mime
    names and mib enums.
*/

/*!
    \fn QStringList QTextCodecPlugin::names() const

    Returns the list of mime names supported by this plugin.

    \sa createForName()
*/

/*!
    \fn QTextCodec *QTextCodecPlugin::createForName( const QString &name );

    Creates a QTextCodec object for the codec called \a name.

    \sa names()
*/


/*!
    \fn QList<int> QTextCodecPlugin::mibEnums() const

    Returns the list of mib enums supported by this plugin.

    \sa createForMib()
*/

/*!
    \fn QTextCodec *QTextCodecPlugin::createForMib( int mib );

    Creates a QTextCodec object for the mib enum \a mib.

    (See \link
    ftp://ftp.isi.edu/in-notes/iana/assignments/character-sets the
    IANA character-sets encoding file\endlink for more information)

    \sa mibEnums()
*/



class QTextCodecPluginPrivate : public QTextCodecFactoryInterface
{
public:
    QTextCodecPluginPrivate( QTextCodecPlugin *p )
	: plugin( p )
    {
    }
    virtual ~QTextCodecPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QTextCodec *createForMib( int mib );
    QTextCodec *createForName( const QString &name );

private:
    QTextCodecPlugin *plugin;
};

QTextCodecPluginPrivate::~QTextCodecPluginPrivate()
{
    delete plugin;
}

QRESULT QTextCodecPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QTextCodecFactory )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList QTextCodecPluginPrivate::featureList() const
{
    QStringList keys = plugin->names();
    QList<int> mibs = plugin->mibEnums();
    for ( QList<int>::Iterator it = mibs.begin(); it != mibs.end(); ++it )
	keys += QString("MIB-%1").arg( *it );
    return keys;
}

QTextCodec *QTextCodecPluginPrivate::createForMib( int mib )
{
    return plugin->createForMib( mib );
}

QTextCodec *QTextCodecPluginPrivate::createForName( const QString &name )
{
    return plugin->createForName( name );
}


/*!
    Constructs a text codec plugin. This is invoked automatically by
    the \c Q_EXPORT_PLUGIN macro.
*/
QTextCodecPlugin::QTextCodecPlugin()
    : QGPlugin( d = new QTextCodecPluginPrivate( this ) )
{
}

/*!
    Destroys the text codec plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QTextCodecPlugin::~QTextCodecPlugin()
{
}

#endif // QT_NO_TEXTCODECPLUGIN

#include "qtextcodecplugin.h"
#include "qtextcodecinterface.h"

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
    QTextCodec *createForName( const QString &key );

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
    return plugin->keys();
}

QTextCodec *QTextCodecPluginPrivate::createForMib( int mib )
{
    return plugin->createForMib( mib );
}

QTextCodec *QTextCodecPluginPrivate::createForName( const QString &name )
{
    return plugin->createForName( name );
}


QTextCodecPlugin::QTextCodecPlugin()
{
    d = new QTextCodecPluginPrivate( this );
}

QTextCodecPlugin::~QTextCodecPlugin()
{
}

QStringList QTextCodecPlugin::keys() const
{
    return QStringList();
}

QTextCodec *QTextCodecPlugin::createForMib( int mib )
{
    return 0;
}

QTextCodec *QTextCodecPlugin::createForName( const QString &name )
{
    return 0;
}

QUnknownInterface *QTextCodecPlugin::iface()
{
    QUnknownInterface *i;
    d->queryInterface( IID_QUnknown, &i );
    return i;
}

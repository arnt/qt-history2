#include "qimageformatplugin.h"
#include "qimageformatinterface.h"

class QImageFormatPluginPrivate : public QImageFormatInterface
{
public:
    QImageFormatPluginPrivate( QImageFormatPlugin *p )
	: plugin( p )
    {
    }
    virtual ~QImageFormatPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;

    QRESULT loadImage( const QString &format, const QString &filename, QImage * );
    QRESULT saveImage( const QString &format, const QString &filename, const QImage & );

    QRESULT installIOHandler( const QString & );

private:
    QImageFormatPlugin *plugin;
};

QImageFormatPluginPrivate::~QImageFormatPluginPrivate()
{
    delete plugin;
}

QRESULT QImageFormatPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QImageFormat )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList QImageFormatPluginPrivate::featureList() const
{
    return plugin->featureList();
}

QRESULT QImageFormatPluginPrivate::loadImage( const QString &format, const QString &filename, QImage *image )
{
    return plugin->loadImage( format, filename, image );
}

QRESULT QImageFormatPluginPrivate::saveImage( const QString &format, const QString &filename, const QImage &image )
{
    return plugin->saveImage( format, filename, image );
}

QRESULT QImageFormatPluginPrivate::installIOHandler( const QString &format )
{
    return plugin->installIOHandler( format );
}


QImageFormatPlugin::QImageFormatPlugin()
{
    d = new QImageFormatPluginPrivate( this );
}

QImageFormatPlugin::~QImageFormatPlugin()
{
}

QStringList QImageFormatPlugin::featureList() const
{
    return QStringList();
}

bool QImageFormatPlugin::loadImage( const QString &format, const QString &filename, QImage *image )
{
    return FALSE;
}

bool QImageFormatPlugin::saveImage( const QString &format, const QString &filename, const QImage &image )
{
    return FALSE;
}

bool QImageFormatPlugin::installIOHandler( const QString &format )
{
    return FALSE;
}

QUnknownInterface *QImageFormatPlugin::iface()
{
    QUnknownInterface *i;
    d->queryInterface( IID_QUnknown, &i );
    return i;
}

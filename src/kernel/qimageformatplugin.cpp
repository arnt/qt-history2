#include "qimageformatplugin.h"
#include "qimageformatinterface_p.h"

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
    return plugin->keys();
}

QRESULT QImageFormatPluginPrivate::loadImage( const QString &format, const QString &filename, QImage *image )
{
    return plugin->loadImage( format, filename, image ) ? QS_FALSE : QS_OK;
}

QRESULT QImageFormatPluginPrivate::saveImage( const QString &format, const QString &filename, const QImage &image )
{
    return plugin->saveImage( format, filename, image ) ? QS_FALSE : QS_OK;
}

QRESULT QImageFormatPluginPrivate::installIOHandler( const QString &format )
{
    return plugin->installIOHandler( format ) ? QS_FALSE : QS_OK;
}


QImageFormatPlugin::QImageFormatPlugin()
{
    d = new QImageFormatPluginPrivate( this );
    _iface = d;
}

QImageFormatPlugin::~QImageFormatPlugin()
{
}

bool QImageFormatPlugin::loadImage( const QString &format, const QString &filename, QImage *image )
{
    Q_UNUSED( format )
    Q_UNUSED( filename )
    Q_UNUSED( image )
    return FALSE;
}

bool QImageFormatPlugin::saveImage( const QString &format, const QString &filename, const QImage &image )
{
    Q_UNUSED( format )
    Q_UNUSED( filename )
    Q_UNUSED( image )
    return FALSE;
}


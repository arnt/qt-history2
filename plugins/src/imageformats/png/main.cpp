#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif
#include <qimageformatinterface.h>

#ifdef QT_NO_IMAGEIO_PNG
#undef QT_NO_IMAGEIO_PNG
#endif
#include "../../../../src/kernel/qpngio.cpp"

class PNGFormat : public QImageFormatInterface
{
public:
    PNGFormat();
    Q_REFCOUNT;
    QRESULT queryInterface( const QUuid &, QUnknownInterface ** );

    QStringList featureList() const;

    QRESULT loadImage( const QString &format, const QString &filename, QImage * );
    QRESULT saveImage( const QString &format, const QString &filename, const QImage& );

    QRESULT installIOHandler( const QString & );

private:
    ulong ref;
};

PNGFormat::PNGFormat()
: ref( 0 )
{
}

QRESULT PNGFormat::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_QImageFormat )
	*iface = (QImageFormatInterface*)this;
    else 
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList PNGFormat::featureList() const
{
    QStringList list;
    list << "PNG";

    return list;
}

QRESULT PNGFormat::loadImage( const QString &format, const QString &filename, QImage *image )
{
    if ( format != "PNG" )
	return QE_INVALIDARG;

    QImageIO io;
    io.setFileName( filename );
    io.setImage( *image );

    read_png_image( &io );

    return QS_OK;
}

QRESULT PNGFormat::saveImage( const QString &format, const QString &filename, const QImage &image )
{
    if ( format != "PNG" )
	return QE_INVALIDARG;

    QImageIO io;
    io.setFileName( filename );
    io.setImage( image );

    write_png_image( &io );

    return QS_OK;
}

QRESULT PNGFormat::installIOHandler( const QString &name )
{
    if ( name != "PNG" ) 
	return QE_INVALIDARG;

    qInitPngIO();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( PNGFormat )
}

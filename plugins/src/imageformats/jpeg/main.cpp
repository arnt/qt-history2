#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif
#include <qimageformatinterface.h>

#ifdef QT_NO_IMAGEIO_JPEG
#undef QT_NO_IMAGEIO_JPEG
#endif
#include "../../../../src/kernel/qjpegio.cpp"

class JPEGFormat : public QImageFormatInterface
{
public:
    JPEGFormat();
    Q_REFCOUNT;
    QRESULT queryInterface( const QUuid &, QUnknownInterface ** );

    QStringList featureList() const;

    QRESULT loadImage( const QString &format, const QString &filename, QImage * );
    QRESULT saveImage( const QString &format, const QString &filename, const QImage & );

    QRESULT installIOHandler( const QString & );
};

JPEGFormat::JPEGFormat()
{
}

QRESULT JPEGFormat::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

QStringList JPEGFormat::featureList() const
{
    QStringList list;
    list << "JPEG";

    return list;
}

QRESULT JPEGFormat::loadImage( const QString &format, const QString &filename, QImage *image )
{
    if ( format != "JPEG" )
	return QE_INVALIDARG;

    QImageIO io;
    io.setFileName( filename );
    io.setImage( *image );

    read_jpeg_image( &io );

    return QS_OK;
}

QRESULT JPEGFormat::saveImage( const QString &format, const QString &filename, const QImage &image )
{
    if ( format != "JPEG" )
	return QE_INVALIDARG;

    QImageIO io;
    io.setFileName( filename );
    io.setImage( image );

    write_jpeg_image( &io );

    return QS_OK;
}

QRESULT JPEGFormat::installIOHandler( const QString &name )
{
    if ( name.upper() != "JPEG" )
	return QE_INVALIDARG;

    qInitJpegIO();
    return QS_OK;
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( JPEGFormat )
}

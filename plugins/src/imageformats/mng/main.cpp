#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include <qimageformatinterface.h>

#ifdef QT_NO_IMAGEIO_MNG
#undef QT_NO_IMAGEIO_MNG
#endif
#include "../../../../src/kernel/qmngio.cpp"

class MNGFormat : public QImageFormatInterface
{
public:
    MNGFormat();
    Q_REFCOUNT;
    QRESULT queryInterface( const QUuid &, QUnknownInterface ** );

    QStringList featureList() const;


    QRESULT loadImage( const QString &format, const QString &filename, QImage *image );
    QRESULT saveImage( const QString &format, const QString &filename, const QImage &image );

    QRESULT installIOHandler( const QString & );
};

MNGFormat::MNGFormat()
{
}

QRESULT MNGFormat::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

QStringList MNGFormat::featureList() const
{
    QStringList list;
    list << "MNG";

    return list;
}

QRESULT MNGFormat::loadImage( const QString &, const QString &, QImage * )
{
    return QE_NOTIMPL;
}

QRESULT MNGFormat::saveImage( const QString &, const QString &, const QImage& )
{
    return QE_NOTIMPL;
}

QRESULT MNGFormat::installIOHandler( const QString &name )
{
    if ( name != "MNG" )
	return QE_INVALIDARG;

    qInitMngIO();
    return QS_OK;
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( MNGFormat )
}

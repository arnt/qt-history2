#ifndef QIMAGEFORMATINTERFACE_H
#define QIMAGEFORMATINTERFACE_H

#ifndef QT_H
#include <private/qcom_p.h>
#endif // QT_H

#ifndef QT_NO_COMPONENT

// {04903F05-54B1-4726-A849-FB5CB097CA87} 
#ifndef IID_QImageFormat
#define IID_QImageFormat QUuid( 0x04903f05, 0x54b1, 0x4726, 0xa8, 0x49, 0xfb, 0x5c, 0xb0, 0x97, 0xca, 0x87 )
#endif

class QImage;

struct Q_EXPORT QImageFormatInterface : public QFeatureListInterface
{
    virtual QRESULT loadImage( const QString &format, const QString &filename, QImage * ) = 0;
    virtual QRESULT saveImage( const QString &format, const QString &filename, const QImage & ) = 0;

    virtual QRESULT installIOHandler( const QString & ) = 0;
};

#endif // QT_NO_COMPONENT

#endif // QIMAGEFORMATINTERFACE_H

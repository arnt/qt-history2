#include "qimage.h"
#include "qmime.h"
#include "qdragobject.h"

#if defined(Q_WS_X11)
#include <X11/Xlib.h>				// needed for systemBitOrder
#include <X11/Xutil.h>
#include <X11/Xos.h>
#endif

#ifndef QT_NO_MIME
/*! \fn QImage QImage::fromMimeSource( const QString &abs_name )

    Convenience function. Gets the data associated with the absolute
    name \a abs_name from the default mime source factory and decodes
    it to an image.

    \sa QMimeSourceFactory, QImage::fromMimeSource(), QImageDrag::decode()
*/
QImage qFromMimeSource_helper( const QString &abs_name )
{
    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( abs_name );
    if ( !m ) {
	qWarning("QImage::fromMimeSource: Cannot find image \"%s\" in the mime source factory", abs_name.latin1() );
	return QImage();
    }
    QImage img;
    QImageDrag::decode( m, img );
    return img;
}
#endif

/*
    Sets the image bits to the \a pixmap contents and returns a
    reference to the image.

    If the image shares data with other images, it will first
    dereference the shared data.

    Makes a call to QPixmap::convertToImage().
*/

/*! \fn QImage::Endian QImage::systemBitOrder()

    Determines the bit order of the display hardware. Returns
    QImage::LittleEndian (LSB first) or QImage::BigEndian (MSB first).

    \sa systemByteOrder()
*/

#if defined(Q_WS_X11)
QImage::Endian qX11BitmapBitOrder()
{ return BitmapBitOrder(qt_xdisplay()) == MSBFirst ? QImage::BigEndian : QImage::LittleEndian; }
#endif

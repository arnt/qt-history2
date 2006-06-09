/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#ifndef QT_NO_QWS_TRANSFORMED

#include "qscreentransformed_qws.h"
#include <qvector.h>
#include <private/qpainter_p.h>
#include <qdebug.h>
#include <qmatrix.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include <qwindowsystem_qws.h>
#include <qwsdisplay_qws.h>

#define QT_ROTATION_CACHEDREAD 1
#define QT_ROTATION_CACHEDWRITE 2
#define QT_ROTATION_PACKING 3
#define QT_ROTATION_TILED 4

#ifndef QT_ROTATION_ALGORITHM
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
#define QT_ROTATION_ALGORITHM QT_ROTATION_TILED
#else
#define QT_ROTATION_ALGORITHM QT_ROTATION_CACHEDREAD
#endif
#endif

#if QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
static const int tileSize = 32;
#endif

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
#if QT_ROTATION_ALGORITHM == QT_ROTATION_PACKED || QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
#error Big endian version not implemented for the transformed driver!
#endif
#endif

//#define QT_REGION_DEBUG

// Global function ---------------------------------------------------------------------------------
static QTransformedScreen *qt_trans_screen = 0;

void qws_setScreenTransformation(int t)
{
    if (qt_trans_screen) {
        qt_trans_screen->setTransformation((QTransformedScreen::Transformation)t);
        qwsServer->refresh();
    }
}

// -------------------------------------------------------------------------------------------------
// Transformed Screen
// -------------------------------------------------------------------------------------------------

/*!
    \class QTransformedScreen
    \ingroup qws

    \brief The QTransformedScreen class implements a screen driver for
    a transformed screen.

    Note that this class is only available in \l {Qtopia Core}.
    Custom screen drivers can be added by subclassing the
    QScreenDriverPlugin class, using the QScreenDriverFactory class to
    dynamically load the driver into the application, but there should
    only be one screen object per application.

    The default implementation of QTransformedScreen inherits the
    QVFbScreen class if the virtual framebuffer is enabled, or
    QLinuxFbScreen if it is not. But note that any QScreen subclass,
    or QScreen itself, can serve as its base class. This is easily
    achieved by manipulating the \c QT_TRANS_SCREEN_BASE definition in
    the header file.

    Use the QScreen::isTransformed() function to determine if a screen
    is transformed. The QTransformedScreen class itself provides means
    of rotating the screen with its setTransformation() function; the
    transformation() function returns the currently set rotation in
    terms of the \l Transformation enum (which describes the various
    available rotation settings). Alternatively, QTransformedScreen
    provides an implementation of the QScreen::transformOrientation()
    function, returning the current rotation as an integer value.

    \sa QScreen, QScreenDriverPlugin, {Running Applications}
*/

/*!
    \enum QTransformedScreen::Transformation

    This enum describes the various rotations a transformed screen can
    have.

    \value None No rotation
    \value Rot90 90 degrees rotation
    \value Rot180 180 degrees rotation
    \value Rot270 270 degrees rotation
*/

/*!
    \fn void QTransformedScreen::blit(const QImage & img, const QPoint & topLeft, const QRegion & region)
    \reimp
*/

/*!
    \fn bool QTransformedScreen::connect(const QString & displaySpec)
    \reimp
*/

/*!
    \fn bool QTransformedScreen::isTransformed() const
    \reimp
*/

/*!
    \fn QSize QTransformedScreen::mapFromDevice(const QSize & s) const
    \reimp
*/

/*!
    \fn QPoint QTransformedScreen::mapFromDevice(const QPoint &, const QSize &) const
    \reimp
*/

/*!
    \fn QRect QTransformedScreen::mapFromDevice(const QRect &, const QSize &) const
    \reimp
*/

/*!
    \fn QRegion QTransformedScreen::mapFromDevice(const QRegion &, const QSize &) const
    \reimp
*/

/*!
    \fn QSize QTransformedScreen::mapToDevice(const QSize & s) const
    \reimp
*/

/*!
    \fn QPoint QTransformedScreen::mapToDevice(const QPoint &, const QSize &) const
    \reimp
*/

/*!
    \fn QRect QTransformedScreen::mapToDevice(const QRect &, const QSize &) const
    \reimp
*/

/*!
    \fn QRegion QTransformedScreen::mapToDevice(const QRegion &, const QSize &) const
    \reimp
*/

/*!
    \fn void QTransformedScreen::solidFill(const QColor & color, const QRegion & region)
    \reimp
*/

/*!
    \fn int QTransformedScreen::transformOrientation() const
    \reimp
*/

/*!
    \fn QTransformedScreen::QTransformedScreen(int displayId)

    Constructs a QTransformedScreen object. The \a displayId argument
    identifies the Qtopia Core server to connect to.
*/

/*!
    \fn void QTransformedScreen::setTransformation ( Transformation transformation )

    Rotates this screen object according to the specified \a transformation.

    \sa transformation()
*/

/*!
    \fn Transformation QTransformedScreen::transformation() const

    Returns the currently set rotation.

    \sa setTransformation(), QScreen::transformOrientation()
*/
QTransformedScreen::QTransformedScreen(int display_id)
    : QT_TRANS_SCREEN_BASE(display_id)
{
    trans = None;
    qt_trans_screen = this;

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::QTransformedScreen";
#endif
}

bool QTransformedScreen::connect(const QString &displaySpec)
{
    bool result = QT_TRANS_SCREEN_BASE::connect(displaySpec);
    int indexOfRot = displaySpec.indexOf(":Rot");
    if (result && indexOfRot != -1) {
       int lastIndexOfRot = displaySpec.lastIndexOf(":");
       int t = qMin(displaySpec.mid(indexOfRot + 4, lastIndexOfRot - (indexOfRot + 4)).toUInt()/90, uint(Rot270));

        setTransformation(static_cast<Transformation>(t));
    }
    return result;
}


int QTransformedScreen::transformOrientation() const
{
    return (int)trans;
}

void QTransformedScreen::setTransformation(Transformation t)
{
    trans = t;
    QSize s = mapFromDevice(QSize(dw, dh));
    w = s.width();
    h = s.height();

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::setTransformation" << t << "size" << w << h << "dev size" << dw << dh;
#endif

}

static inline QRect correctNormalized(const QRect &r) {
    const int x1 = qMin(r.left(), r.right());
    const int x2 = qMax(r.left(), r.right());
    const int y1 = qMin(r.top(), r.bottom());
    const int y2 = qMax(r.top(), r.bottom());

    return QRect( QPoint(x1,y1), QPoint(x2,y2) );
}

template <class SRC, class DST>
static inline DST colorConvert(SRC color)
{
    return color;
}

#ifdef QT_QWS_DEPTH_16
template <>
static inline quint16 colorConvert(quint32 color)
{
    return qt_convRgbTo16(color);
}
#endif

#ifdef QT_QWS_DEPTH_8
template <>
static inline quint8 colorConvert(quint32 color)
{
    uchar r = (qRed(color) + 0x19) / 0x33;
    uchar g = (qGreen(color) + 0x19) / 0x33;
    uchar b = (qBlue(color) + 0x19) / 0x33;

    return r*6*6 + g*6 + b;
}

template <>
static inline quint8 colorConvert(quint16 color)
{
    return colorConvert<quint32, quint8>(qt_conv16ToRgb(color));
}
#endif // QT_QWS_DEPTH_8

typedef void (*BlitFunc)(const QImage &, const QRect &, const QPoint &);

#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD

template <class SRC, class DST>
static inline void blit90_cachedRead(const QImage &image, const QRect &rect,
                                     const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = qt_screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(qt_screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    for (int y = 0; y < h; ++y) {
        for (int x = w - 1; x >= 0; --x) {
            dest[(w - x - 1) * dstride + y] = colorConvert<SRC,DST>(src[x]);
        }
        src += sstride;
    }
}

template <class SRC, class DST>
static void blit270_cachedRead(const QImage &image, const QRect &rect,
                               const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = qt_screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits()) +
                      rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(qt_screen->base())
                 + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    src += (h - 1) * sstride;
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            dest[x * dstride + h - y - 1] = colorConvert<SRC,DST>(src[x]);
        }
        src -= sstride;
    }
}

#endif // QT_ROTATION_CACHEDREAD

#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE

template <class SRC, class DST>
static inline void blit90_cachedWrite(const QImage &image, const QRect &rect,
                                      const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = qt_screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(qt_screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    for (int x = w - 1; x >= 0; --x) {
        DST *d = dest + (w - x - 1) * dstride;
        for (int y = 0; y < h; ++y) {
            *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
        }
    }

}

template <class SRC, class DST>
static void blit270_cachedWrite(const QImage &image, const QRect &rect,
                                const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = qt_screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits()) +
                      rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(qt_screen->base())
                 + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    for (int x = 0; x < w; ++x) {
        DST *d = dest + x * dstride;
        for (int y = h - 1; y >= 0; --y) {
            *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
        }
    }
}

#endif // QT_ROTATION_CACHEDWRITE

#if QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING

template <class SRC, class DST>
static inline void blit90_packing(const QImage &image, const QRect &rect,
                                  const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = qt_screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(qt_screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned = (quint32(dest) & (sizeof(quint32)-1)) / sizeof(DST);

    for (int x = w - 1; x >= 0; --x) {
        int y = 0;

        for (int i = 0; i < unaligned; ++i) {
            dest[(w - x - 1) * dstride + y]
                = colorConvert<SRC,DST>(src[y * sstride + x]);
            ++y;
        }

        quint32 *d = reinterpret_cast<quint32*>(dest + (w - x - 1) * dstride
                                                + unaligned);
        const int rest = (h - unaligned) % pack;
        while (y < h - rest) {
            quint32 c = colorConvert<SRC,DST>(src[y * sstride + x]);
            for (int i = 1; i < pack; ++i) {
                c |= colorConvert<SRC,DST>(src[(y + i) * sstride + x])
                     << (sizeof(int) * 8 / pack * i);
            }
            *d++ = c;
            y += pack;
        }

        while (y < h) {
            dest[(w - x - 1) * dstride + y]
                 = colorConvert<SRC,DST>(src[y * sstride + x]);
            ++y;
        }
    }
}

template <class SRC, class DST>
static inline void blit270_packing(const QImage &image, const QRect &rect,
                                   const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = qt_screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits()) +
                      rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(qt_screen->base())
                 + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned = (quint32(dest) & (sizeof(quint32)-1)) / sizeof(DST);

    for (int x = 0; x < w; ++x) {
        int y = h - 1;

        for (int i = 0; i < unaligned; ++i) {
            dest[x * dstride + h - y - 1]
                = colorConvert<SRC,DST>(src[y * sstride + x]);
            --y;
        }

        quint32 *d = reinterpret_cast<quint32*>(dest + x * dstride
                                                + unaligned);
        const int rest = (h - unaligned) % pack;
        while (y > rest) {
            quint32 c = colorConvert<SRC,DST>(src[y * sstride + x]);
            for (int i = 1; i < pack; ++i) {
                c |= colorConvert<SRC,DST>(src[(y - i) * sstride + x])
                     << (sizeof(int) * 8 / pack * i);
            }
            *d++ = c;
            y -= pack;
        }
        while (y >= 0) {
            dest[x * dstride + h - y - 1]
                = colorConvert<SRC,DST>(src[y * sstride + x]);
            --y;
        }
    }
}

#endif // QT_ROTATION_PACKING

#if QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
template <class SRC, class DST>
static inline void blit90_tiled(const QImage &image, const QRect &rect,
                                const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = qt_screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(qt_screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned = qMin((quint32(dest) & (sizeof(quint32)-1)) / sizeof(DST), uint(h));

    const int numTilesX = w / tileSize;
    const int numTilesY = (h - unaligned) / tileSize;
    const int restX = w % tileSize;
    const int restY = (h - unaligned) % tileSize;

    // XXX: restY/restX should be treated as smaller tiles

    for (int tx = numTilesX - 1; tx >= 0; --tx) {
        const int startx = (tx + 1) * tileSize - 1;
        const int stopx = startx - tileSize;

        if (unaligned) {
            for (int x = startx; x > stopx; --x) {
                DST *d = dest + (w - x - 1) * dstride;
                for (int y = 0; y < unaligned; ++y) {
                    *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
                }
            }
        }

        for (int ty = 0; ty < numTilesY; ++ty) {
            const int starty = ty * tileSize + unaligned;
            const int stopy = starty + tileSize;

            for (int x = startx; x > stopx; --x) {
                quint32 *d = reinterpret_cast<quint32*>(dest + (w - x - 1) * dstride + starty);
                for (int y = starty; y < stopy; y += pack) {
                    quint32 c = colorConvert<SRC,DST>(src[y * sstride + x]);
                    for (int i = 1; i < pack; ++i) {
                        const int shift = (sizeof(int) * 8 / pack * i);
                        const DST color = colorConvert<SRC,DST>(src[(y + i) * sstride + x]);
                        c |= color << shift;
                    }
                    *d++ = c;
                }
            }
        }

        if (restY) {
            const int starty = h - restY;
            for (int x = startx; x > stopx; --x) {
                DST *d = dest + (w - x - 1) * dstride + starty;
                for (int y = starty; y < h; ++y) {
                    *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
                }
            }
        }
    }

    if (restX) {
        const int startx = w - restX;
        for (int x = startx; x < w; ++x) {
            DST *d = dest + (w - x - 1) * dstride;
            for (int y = 0; y < h; ++y) {
                *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
            }
        }
    }
}

template <class SRC, class DST>
static inline void blit270_tiled(const QImage &image, const QRect &rect,
                                 const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = qt_screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                     + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(qt_screen->base())
                + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    const int pack = sizeof(quint32) / sizeof(DST);
    const int unaligned = qMin((quint32(dest) & (sizeof(quint32)-1)) / sizeof(DST), uint(h));

    const int numTilesX = w / tileSize;
    const int numTilesY = (h - unaligned) / tileSize;
    const int restX = w % tileSize;
    const int restY = (h - unaligned) % tileSize;


    // XXX: restY/restX should be treated as smaller tiles

    for (int tx = 0; tx < numTilesX; ++tx) {
        const int startx = tx * tileSize;
        const int stopx = startx + tileSize;

        if (unaligned) {
            for (int x = startx; x < stopx; ++x) {
                DST *d = dest + x * dstride;
                for (int y = h - 1; y >= h - unaligned; --y) {
                    *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
                }
            }
        }

        for (int ty = 0; ty < numTilesY; ++ty) {
            const int starty = h - 1 - unaligned - ty * tileSize;
            const int stopy = starty - tileSize;

            for (int x = startx; x < stopx; ++x) {
                quint32 *d = reinterpret_cast<quint32*>(dest + x * dstride
                                                        + h - 1 - starty);
                for (int y = starty; y > stopy; y -= pack) {
                    quint32 c = colorConvert<SRC,DST>(src[y * sstride + x]);
                    for (int i = 1; i < pack; ++i) {
                        const int shift = (sizeof(int) * 8 / pack * i);
                        const DST color = colorConvert<SRC,DST>(src[(y - i) * sstride + x]);
                        c |= color << shift;
                    }
                    *d++ = c;
                }
            }
        }
        if (restY) {
            const int starty = restY - 1;
            for (int x = startx; x < stopx; ++x) {
                DST *d = dest + x * dstride + h - 1 - starty;
                for (int y = starty; y >= 0; --y) {
                    *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
                }
            }
        }
    }

    if (restX) {
        const int startx = w - restX;
        for (int x = startx; x < w; ++x) {
            DST *d = dest + x * dstride;
            for (int y = h - 1; y >= 0; --y) {
                *d++ = colorConvert<SRC,DST>(src[y * sstride + x]);
            }
        }
    }
}

#endif // QT_ROTATION_ALFORITHM

template <class SRC, class DST>
static void blit90(const QImage &image, const QRect &rect, const QPoint &topLeft)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    blit90_cachedRead<SRC,DST>(image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    blit90_cachedWrite<SRC,DST>(image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING
    blit90_packing<SRC,DST>(image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
    blit90_tiled<SRC,DST>(image, rect, topLeft);
#endif
}

template <class SRC, class DST>
static void blit180(const QImage &image, const QRect &rect, const QPoint &topLeft)
{
    const int sstride = image.bytesPerLine() / sizeof(SRC);
    const int dstride = qt_screen->linestep() / sizeof(DST);
    const SRC *src = reinterpret_cast<const SRC *>(image.bits())
                      + rect.top() * sstride + rect.left();
    DST *dest = reinterpret_cast<DST*>(qt_screen->base())
                 + topLeft.y() * dstride + topLeft.x();
    const int h = rect.height();
    const int w = rect.width();

    src += (h - 1) * sstride;
    for (int y = h - 1; y >= 0; --y) {
        for (int x = w - 1; x >= 0; --x) {
            dest[(h - y - 1) * dstride + w - x - 1] = colorConvert<SRC,DST>(src[x]);
        }
        src -= sstride;
    }
}

template <class SRC, class DST>
static void blit270(const QImage &image, const QRect &rect, const QPoint &topLeft)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    blit270_cachedRead<SRC,DST>(image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    blit270_cachedWrite<SRC,DST>(image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_PACKING
    blit270_packing<SRC,DST>(image, rect, topLeft);
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_TILED
    blit270_tiled<SRC,DST>(image, rect, topLeft);
#endif
}

#define SET_BLIT_FUNC(src, dst, rotation, func) \
do {                                            \
    switch (rotation) {                         \
    case Rot90:                                 \
        func = blit90<src, dst>;                \
        break;                                  \
    case Rot180:                                \
        func = blit180<src, dst>;               \
        break;                                  \
    case Rot270:                                \
        func = blit270<src, dst>;               \
        break;                                  \
    default:                                    \
        break;                                  \
    }                                           \
} while (0)


void QTransformedScreen::blit(const QImage &image, const QPoint &topLeft,
                              const QRegion &region)
{
    if (trans == None) {
        QT_TRANS_SCREEN_BASE::blit(image, topLeft, region);
        return;
    }

    const QVector<QRect> rects = region.rects();
    const QRect bound = QRect(0, 0, QScreen::w, QScreen::h)
                        & QRect(topLeft, image.size());

    BlitFunc func = 0;
    switch (qt_screen->depth()) {
#ifdef QT_QWS_DEPTH_32
    case 32:
        SET_BLIT_FUNC(quint32, quint32, trans, func);
        break;
#endif
#ifdef QT_QWS_DEPTH_16
    case 16:
        if (image.depth() == 16)
            SET_BLIT_FUNC(quint16, quint16, trans, func);
        else
            SET_BLIT_FUNC(quint32, quint16, trans, func);
        break;
#endif
#ifdef QT_QWS_DEPTH_8
    case 8:
        if (image.depth() == 16)
            SET_BLIT_FUNC(quint16, quint8, trans, func);
        else
            SET_BLIT_FUNC(quint32, quint8, trans, func);
        break;
#endif
    default:
        return;
    }
    if (!func)
        return;

    QWSDisplay::grab();
    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i) & bound;

        QPoint dst;
        switch (trans) {
        case Rot90:
            dst = mapToDevice(r.topRight(), QSize(w, h));
            break;
        case Rot180:
            dst = mapToDevice(r.bottomRight(), QSize(w, h));
            break;
        case Rot270:
            dst = mapToDevice(r.bottomLeft(), QSize(w, h));
            break;
        default:
            break;
        }
        func(image, r.translated(-topLeft), dst);
    }
    QWSDisplay::ungrab();

}

void QTransformedScreen::solidFill(const QColor &color, const QRegion &region)
{
    QRegion tr = mapToDevice(region, QSize(w,h));

    Q_ASSERT(tr.boundingRect() == mapToDevice(region.boundingRect(), QSize(w,h)));

#ifdef QT_REGION_DEBUG
    qDebug() << "QTransformedScreen::solidFill region" << region << "transformed" << tr;
#endif
    QT_TRANS_SCREEN_BASE::solidFill(color, tr);
}


// Mapping functions
QSize QTransformedScreen::mapToDevice(const QSize &s) const
{
    switch (trans) {
    case None:
    case Rot180:
        break;
    case Rot90:
    case Rot270:
        return QSize(s.height(), s.width());
        break;
    }
    return s;
}

QSize QTransformedScreen::mapFromDevice(const QSize &s) const
{
    switch (trans) {
    case None:
    case Rot180:
        break;
    case Rot90:
    case Rot270:
        return QSize(s.height(), s.width());
        break;
    }
        return s;
}

QPoint QTransformedScreen::mapToDevice(const QPoint &p, const QSize &s) const
{
    QPoint rp(p);

    switch (trans) {
    case None:
        break;
    case Rot90:
        rp.setX(p.y());
        rp.setY(s.width() - p.x() - 1);
        break;
    case Rot180:
        rp.setX(s.width() - p.x() - 1);
        rp.setY(s.height() - p.y() - 1);
        break;
    case Rot270:
        rp.setX(s.height() - p.y() - 1);
        rp.setY(p.x());
        break;
    }

    return rp;
}

QPoint QTransformedScreen::mapFromDevice(const QPoint &p, const QSize &s) const
{
    QPoint rp(p);

    switch (trans) {
    case None:
        break;
    case Rot90:
        rp.setX(s.height() - p.y() - 1);
        rp.setY(p.x());
        break;
    case Rot180:
        rp.setX(s.width() - p.x() - 1);
        rp.setY(s.height() - p.y() - 1);
        break;
    case Rot270:
        rp.setX(p.y());
        rp.setY(s.width() - p.x() - 1);
        break;
    }

    return rp;
}





QRect QTransformedScreen::mapToDevice(const QRect &r, const QSize &s) const
{
    if (r.isNull())
        return QRect();

    QRect tr;
    switch (trans) {
    case None:
        tr = r;
        break;
    case Rot90:
        tr.setCoords(r.y(), s.width() - r.x() - 1,
                     r.bottom(), s.width() - r.right() - 1);
        break;
    case Rot180:
        tr.setCoords(s.width() - r.x() - 1, s.height() - r.y() - 1,
                     s.width() - r.right() - 1, s.height() - r.bottom() - 1);
        break;
    case Rot270:
        tr.setCoords(s.height() - r.y() - 1, r.x(),
                     s.height() - r.bottom() - 1, r.right());
        break;
    }

    return correctNormalized(tr);
}

QRect QTransformedScreen::mapFromDevice(const QRect &r, const QSize &s) const
{
    if (r.isNull())
        return QRect();

    QRect tr;
    switch (trans) {
    case None:
        tr = r;
        break;
    case Rot90:
        tr.setCoords(s.height() - r.y() - 1, r.x(),
                     s.height() - r.bottom() - 1, r.right());
        break;
    case Rot180:
        tr.setCoords(s.width() - r.x() - 1, s.height() - r.y() - 1,
                     s.width() - r.right() - 1, s.height() - r.bottom() - 1);
        break;
    case Rot270:
        tr.setCoords(r.y(), s.width() - r.x() - 1,
                     r.bottom(), s.width() - r.right() - 1);
        break;
    }

    return correctNormalized(tr);
}

#if 1
QRegion QTransformedScreen::mapToDevice(const QRegion &rgn, const QSize &s) const
{
    if (trans == None)
        return rgn;

#ifdef QT_REGION_DEBUG
    qDebug() << "mapToDevice size" << s << "rgn:  " << rgn;
#endif
    QRect tr;
    QRegion trgn;
    QVector<QRect> a = rgn.rects();
    const QRect *r = a.data();

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch (trans) {
    case None:
        break;
    case Rot90:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(r->y(), w - r->x() - 1,
                         r->bottom(), w - r->right() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot180:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(w - r->x() - 1, h - r->y() - 1,
                         w - r->right() - 1, h - r->bottom() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot270:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(h - r->y() - 1, r->x(),
                         h - r->bottom() - 1, r->right());
            trgn |= correctNormalized(tr);
        }
        break;
    }
#ifdef QT_REGION_DEBUG
    qDebug() << "mapToDevice trgn:  " << trgn;
#endif
    return trgn;
}

QRegion QTransformedScreen::mapFromDevice(const QRegion &rgn, const QSize &s) const
{
    if (trans == None)
        return rgn;
#ifdef QT_REGION_DEBUG
    qDebug() << "fromDevice: realRegion count:  " << rgn.rects().size() << " isEmpty? " << rgn.isEmpty() << "  bounds:" << rgn.boundingRect();
#endif
    QRect tr;
    QRegion trgn;
    QVector<QRect> a = rgn.rects();
    const QRect *r = a.data();

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch (trans) {
    case None:
        break;
    case Rot90:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(h - r->y() - 1, r->x(),
                         h - r->bottom() - 1, r->right());
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot180:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(w - r->x() - 1, h - r->y() - 1,
                         w - r->right() - 1, h - r->bottom() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    case Rot270:
        for (int i = 0; i < size; i++, r++) {
            tr.setCoords(r->y(), w - r->x() - 1,
                         r->bottom(), w - r->right() - 1);
            trgn |= correctNormalized(tr);
        }
        break;
    }
#ifdef QT_REGION_DEBUG
    qDebug() << "fromDevice: transRegion count: " << trgn.rects().size() << " isEmpty? " << trgn.isEmpty() << "  bounds:" << trgn.boundingRect();
#endif
    return trgn;
}
#endif

#endif // QT_NO_QWS_TRANSFORMED

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpixmap_p.h"

#include "qwidget.h"
#include "qbitmap.h"
#include "qcolormap.h"
#include "qimage.h"
#include "qmatrix.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qwsdisplay_qws.h"
#include "qscreen_qws.h"
#include "qhash.h"
#include <stdlib.h>
#include <limits.h>

#include <private/qpaintengine_raster_p.h>
#include <private/qwidget_p.h>
#include <private/qbackingstore_p.h>

#include <qdebug.h>

QPixmap QPixmap::grabWindow(WId window, int x, int y, int w, int h)
{
    QWidget *widget = QWidget::find(window);
    if (!widget)
        return QPixmap();

    QRect grabRect = widget->frameGeometry();
    if (w < 0)
        w = grabRect.width() - x;
    if (h < 0)
        h = grabRect.height() - y;
    grabRect &= QRect(x, y, w, h);

    QPixmap pixmap;

    QWSDisplay::grab(false);

    if (qt_screen->pixelFormat() != QImage::Format_Invalid) {
        QImage img(qt_screen->base(),
                   qt_screen->width(), qt_screen->height(),
                   qt_screen->linestep(), qt_screen->pixelFormat());
        pixmap = fromImage(img.copy(grabRect));
    } else if (qt_screen->base()) {

        // temporary workaraounds for screen depths supported by the linuxfb
        // driver. Should be replaced by some QScreen::toImage() function
        // or something...
        switch (qt_screen->depth()) {
#ifdef QT_QWS_DEPTH_24
        case 24: {
            QImage img(grabRect.width(), grabRect.height(), QImage::Format_RGB32);
            const uchar *src = qt_screen->base()
                               + qt_screen->linestep() * grabRect.y()
                               + sizeof(quint24) * grabRect.x();
            qt_rectconvert<quint32, quint24>(reinterpret_cast<quint32*>(img.bits()),
                                             reinterpret_cast<const quint24*>(src),
                                             0, 0, grabRect.width(), grabRect.height(),
                                             img.bytesPerLine(),
                                             qt_screen->linestep());
            pixmap = fromImage(img);
            break;
        }
#endif // QT_QWS_DEPTH_24
#ifdef QT_QWS_DEPTH_18
        case 18: {
            QImage img(grabRect.width(), grabRect.height(), QImage::Format_RGB32);
            const uchar *src = qt_screen->base()
                               + qt_screen->linestep() * grabRect.y()
                               + sizeof(quint18) * grabRect.x();
            qt_rectconvert<quint32, quint18>(reinterpret_cast<quint32*>(img.bits()),
                                             reinterpret_cast<const quint18*>(src),
                                             0, 0, grabRect.width(), grabRect.height(),
                                             img.bytesPerLine(),
                                             qt_screen->linestep());
            pixmap = fromImage(img);
            break;
        }
#endif // QT_QWS_DEPTH_18
        default:
            qWarning("QPixmap::grabWindow(): Unsupported screen depth %d",
                     qt_screen->depth());
            break;
        }
    } else {
        qWarning("QPixmap::grabWindow(): Unable to copy pixels from framebuffer");
    }

    QWSDisplay::ungrab();

    return pixmap;
}



/*!
    \internal
*/
QRgb * QPixmap::clut() const
{
    return data->image.colorTable().data();
}

/*!
    \internal
*/
int QPixmap::numCols() const
{
    return data->image.numColors();
}

/*!
    \internal
    \since 4.1
*/
const uchar *QPixmap::qwsBits() const
{
    return data->image.bits();
}

/*!
    \internal
    \since 4.1
*/
int QPixmap::qwsBytesPerLine() const
{
    return data->image.bytesPerLine();
}

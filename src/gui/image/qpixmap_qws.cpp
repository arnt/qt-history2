/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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

    QImage::Endian endian = (QSysInfo::ByteOrder == QSysInfo::LittleEndian ?
                             QImage::LittleEndian : QImage::BigEndian);

    QWSDisplay::grab(false);
    QImage img(qt_screen->base(),
               qt_screen->width(), qt_screen->height(),
               qt_screen->depth(), qt_screen->linestep(),
               qt_screen->clut(), qt_screen->numCols(),
               endian);
    QPixmap pixmap = fromImage(img.copy(grabRect));
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

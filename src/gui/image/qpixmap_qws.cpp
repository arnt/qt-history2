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

#include "qmemorymanager_qws.h"
#include <private/qpaintengine_raster_p.h>

QPixmap QPixmap::grabWindow(WId window, int x, int y, int w, int h)
{
    Q_UNUSED(window);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(w);
    Q_UNUSED(h);

    qWarning("QPixmap::grabWindow");

    QPixmap pm;
#if 0
    QWidget *widget = QWidget::find(window);
    if (widget) {
        if (w <= 0 || h <= 0) {
            if (w == 0 || h == 0)
                return pm;
            if (w < 0)
                w = widget->width() - x;
            if (h < 0)
                h = widget->height() - y;
        }
        pm.resize(w, h);
        QWSPaintEngine *pe=new QWSPaintEngine;
        if (pe) {
            pe->begin(&pm);
            pe->blt(*widget,0,0,w,h,x,y);
            pe->end();
        }
        delete pe;
    }
#endif
    return pm;
}



/*!
    \internal
*/
const unsigned char * QPixmap::qwsScanLine(int i) const
{
    return data->image.scanLine(i);
}

/*!
    \internal
*/
int QPixmap::qwsBytesPerLine() const
{
    return data->image.bytesPerLine();
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

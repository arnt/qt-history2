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

#ifndef QWINDOWSURFACE_RASTER_P_H
#define QWINDOWSURFACE_RASTER_P_H

#include <qglobal.h>
#include "private/qwindowsurface_p.h"

class QPaintDevice;
class QPoint;
class QRegion;
class QRegion;
class QSize;
class QWidget;
struct QRasterWindowSurfacePrivate;

class QRasterWindowSurface : public QWindowSurface
{
public:
    QRasterWindowSurface(QWidget *widget);
    ~QRasterWindowSurface();

    QPaintDevice *paintDevice();
    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);

    void resize(const QSize &size);
    void release();

    void scroll(const QRegion &area, int dx, int dy);

    QSize size() const;

private:
    QRasterWindowSurfacePrivate *d_ptr;
};

#endif // QWINDOWSURFACE_RASTER_P_H

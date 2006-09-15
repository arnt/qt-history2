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

#ifndef SVGALIBPAINTDEVICE_H
#define SVGALIBPAINTDEVICE_H

#include "svgalibpaintengine.h"
#include <private/qpaintengine_raster_p.h>
#include <qscreen_qws.h>

class SvgalibPaintDevice : public QCustomRasterPaintDevice
{
public:
    SvgalibPaintDevice(QWidget *w);
    ~SvgalibPaintDevice();

    void* memory() const { return QScreen::instance()->base(); }

    QPaintEngine *paintEngine() const { return pengine; }
    int metric(PaintDeviceMetric m) const;

private:
    SvgalibPaintEngine *pengine;
};

#endif // SVGALIBPAINTDEVICE_H

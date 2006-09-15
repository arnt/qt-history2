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

#ifndef SVGALIBPAINTENGINE_H
#define SVGALIBPAINTENGINE_H

#include <private/qpaintengine_raster_p.h>

class SvgalibPaintEngine : public QRasterPaintEngine
{
public:
    SvgalibPaintEngine();
    ~SvgalibPaintEngine();

    bool begin(QPaintDevice *device);
    bool end();
    void updateState(const QPaintEngineState &state);
    void drawRects(const QRect *rects, int rectCount);

private:
    void setClip(const QRegion &region);
    void updateClip();

    QPen pen;
    bool simplePen;
    QBrush brush;
    bool simpleBrush;
    QMatrix matrix;
    bool simpleMatrix;
    QRegion clip;
    bool clipEnabled;
    bool simpleClip;
    bool opaque;
    bool aliased;
    bool sourceOver;
    QPaintDevice *device;
};

#endif // SVGALIBPAINTENGINE_H

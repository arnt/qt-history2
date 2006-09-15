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

#include "svgalibpaintengine.h"

#include <QColor>
#include <vga.h>
#include <vgagl.h>

SvgalibPaintEngine::SvgalibPaintEngine()
{
}

SvgalibPaintEngine::~SvgalibPaintEngine()
{
}

bool SvgalibPaintEngine::begin(QPaintDevice *dev)
{
    device = dev;
    pen = Qt::NoPen;
    simplePen = true;
    brush = Qt::NoBrush;
    simpleBrush = true;
    matrix = QMatrix();
    simpleMatrix = true;
    setClip(QRect(0, 0, device->width(), device->height()));
    simpleClip = true;
    opaque = true;
    aliased = true;
    sourceOver = true;

    return QRasterPaintEngine::begin(dev);
}

bool SvgalibPaintEngine::end()
{
    gl_setclippingwindow(0, 0, device->width() - 1, device->height() - 1);
    return QRasterPaintEngine::end();
}

void SvgalibPaintEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();

    if (flags & DirtyTransform) {
        matrix = state.matrix();
        simpleMatrix = (matrix.m12() == 0 && matrix.m21() == 0);
    }

    if (flags & DirtyPen) {
        pen = state.pen();
        simplePen = (pen.width() == 0 || pen.widthF() <= 1)
                    && (pen.style() == Qt::NoPen || pen.style() == Qt::SolidLine)
                    && (pen.color().alpha() == 255);
    }

    if (flags & DirtyBrush) {
        brush = state.brush();
        simpleBrush = (brush.style() == Qt::SolidPattern
                       || brush.style() == Qt::NoBrush)
                      && (brush.color().alpha() == 255);
    }

    if (flags & DirtyClipRegion)
        setClip(state.clipRegion());

    if (flags & DirtyClipEnabled) {
        clipEnabled = state.isClipEnabled();
        updateClip();
    }

    if (flags & DirtyClipPath) {
        setClip(QRegion());
        simpleClip = false;
    }

    if (flags & DirtyCompositionMode) {
        const QPainter::CompositionMode m = state.compositionMode();
        sourceOver = (m == QPainter::CompositionMode_SourceOver);
    }

    if (flags & DirtyOpacity)
        opaque = (state.opacity() == 256);

    if (flags & DirtyHints)
        aliased = !(state.renderHints() & QPainter::Antialiasing);

    QRasterPaintEngine::updateState(state);
}

void SvgalibPaintEngine::setClip(const QRegion &region)
{
    if (region.isEmpty())
        clip = QRect(0, 0, device->width(), device->height());
    else
        clip = matrix.map(region) & QRect(0, 0, device->width(), device->height());
    clipEnabled = true;
    updateClip();
}

void SvgalibPaintEngine::updateClip()
{
    QRegion clipRegion = QRect(0, 0, device->width(), device->height());

    if (!systemClip().isEmpty())
        clipRegion &= systemClip();
    if (clipEnabled)
        clipRegion &= clip;

    simpleClip = (clipRegion.rects().size() <= 1);

    const QRect r = clipRegion.boundingRect();
    gl_setclippingwindow(r.left(), r.top(),
                         r.x() + r.width(),
                         r.y() + r.height());
}

void SvgalibPaintEngine::drawRects(const QRect *rects, int rectCount)
{
    const bool canAccelerate = simplePen && simpleBrush && simpleMatrix
                               && simpleClip && opaque && aliased
                               && sourceOver;
    if (!canAccelerate) {
        QRasterPaintEngine::drawRects(rects, rectCount);
        return;
    }

    for (int i = 0; i < rectCount; ++i) {
        const QRect r = matrix.mapRect(rects[i]);
        if (brush != Qt::NoBrush) {
            gl_fillbox(r.left(), r.top(), r.width(), r.height(),
                       brush.color().rgba());
        }
        if (pen != Qt::NoPen) {
            const int c = pen.color().rgba();
            gl_hline(r.left(), r.top(), r.right(), c);
            gl_hline(r.left(), r.bottom(), r.right(), c);
            gl_line(r.left(), r.top(), r.left(), r.bottom(), c);
            gl_line(r.right(), r.top(), r.right(), r.bottom(), c);
        }
    }
}

/****************************************************************************
**
** Definition of QPaintEngine(for Mac) private data.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPAINTENGINE_MAC_P_H
#define QPAINTENGINE_MAC_P_H

#include "qpaintengine_p.h"

/*****************************************************************************
  QuickDraw Private data
 *****************************************************************************/
class paintevent_item;
class QQuickDrawPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QQuickDrawPaintEngine)
public:
    QQuickDrawPaintEnginePrivate()
        : QPaintEnginePrivate()
    {
        saved = 0;
        paintevent = 0;
        clip.serial = 0;
        clip.dirty = locked = unclipped = false;
        clip.pdev = clip.paintable = QRegion();
        brush_style_pix = 0;
        offx = offy = 0;
    }

    struct {
        QPen pen;
        QBrush brush;
        QRegion clip;
        struct {
            QPoint origin;
            Qt::BGMode mode;
            QBrush brush;
        } bg;
    } current;

    int offx, offy;
    QPixmap *brush_style_pix;
    uint unclipped : 1, locked : 1;
    QMacSavedPortInfo *saved;
    paintevent_item *paintevent;

    struct {
        QRegion pdev, paintable;
        uint dirty : 1, serial : 15;
    } clip;
};

/*****************************************************************************
  Private data
 *****************************************************************************/
class QCoreGraphicsPaintEnginePrivate : public QQuickDrawPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QQuickDrawPaintEngine)
public:
    QCoreGraphicsPaintEnginePrivate()
    {
        hd = 0;
        shading = 0;
    }

    //state info (shared with QD)

    //cg strucutres
    CGContextRef hd;
    CGShadingRef shading;

    //internal functions
    enum { CGStroke=0x01, CGEOFill=0x02, CGFill=0x04 };
    inline void drawPath(uchar ops) {
        Q_ASSERT((ops & (CGFill|CGEOFill)) != (CGFill|CGEOFill)); //can't really happen
        if((ops & CGFill) && current.brush.style() == Qt::LinearGradientPattern) {
            CGContextSaveGState(hd);
            CGContextClip(hd);
            CGContextDrawShading(hd, shading);
            CGContextRestoreGState(hd);
            ops &= ~CGFill;
        }
        if((ops & (CGFill|CGEOFill)) && current.brush.style() == Qt::NoBrush)
            ops &= ~CGFill;
        if((ops & CGStroke) && current.pen.style() == Qt::NoPen)
            ops &= ~CGStroke;

        CGPathDrawingMode mode;
        if((ops & (CGStroke|CGFill)) == (CGStroke|CGFill))
            mode = kCGPathFillStroke;
        else if((ops & (CGStroke|CGEOFill)) == (CGStroke|CGEOFill))
            mode = kCGPathEOFillStroke;
        else if(ops & CGStroke)
            mode = kCGPathStroke;
        else if(ops & CGEOFill)
            mode = kCGPathEOFill;
        else if(ops & CGFill)
            mode = kCGPathFill;
        else //nothing to do..
            return;
        CGContextDrawPath(hd, mode);
    }
};

#endif // QPAINTENGINE_MAC_P_H

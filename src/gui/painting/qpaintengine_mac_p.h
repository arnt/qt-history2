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
    void drawPath(uchar ops, CGMutablePathRef path = 0);
};

#endif // QPAINTENGINE_MAC_P_H

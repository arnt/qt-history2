/****************************************************************************
**
** Definition of QPaintEngine(for Windows) private data.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWIN32PAINTENGINE_P_H
#define QWIN32PAINTENGINE_P_H

#include <windows.h>

#include "qnamespace.h"
#include "qpaintengine_p.h"

#define COLOR_VALUE(c) ((d->flags & RGBColor) ? RGB(c.red(),c.green(),c.blue()) : c.pixel())

// Typedefs for GDI+
class QtGpGraphics { };
class QtGpMatrix { };
class QtGpRegion { };
class QtGpPen { };
class QtGpBrush { };
class QtGpSolidFill : public QtGpBrush { };
class QtGpPath { };
class QtGpImage { };
class QtGpBitmap : public QtGpImage { };

struct QtGpStartupInput { Q_UINT32 version; void *cb; BOOL b1; BOOL b2; };

struct QtGpRect
{
    QtGpRect(const QRect &r) : x(r.x()), y(r.y()), w(r.width()), h(r.height()) { }
    int x, y, w, h;
};

class QGdiplusPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QGdiplusPaintEngine)
public:
    QGdiplusPaintEnginePrivate() :
        hwnd(0),
        hdc(0),
        graphics(0),
        bitmapDevice(0),
        pen(0),
        brush(0),
        cachedSolidBrush(0),
        usesTempDC(false),
        usePen(false),
        temporaryBrush(false),
        antiAliasEnabled(false)
    {
    }

    HWND hwnd;
    HDC hdc;

    QtGpGraphics *graphics;
    QtGpBitmap *bitmapDevice;
    QtGpPen *pen;
    QtGpBrush *brush;

    QtGpSolidFill *cachedSolidBrush;

    uint usesTempDC : 1;
    uint usePen : 1;
    uint temporaryBrush : 1;
    uint antiAliasEnabled : 1;
};

class Q_GUI_EXPORT QWin32PaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWin32PaintEngine)
public:
    QWin32PaintEnginePrivate() :
        hwnd(0),
        hdc(0),
        hpen(0),
        hfont(0),
        hbrush(0),
        hbrushbm(0),
        holdpal(0),
        flags(0),
        penRef(0),
        brushRef(0),
        nocolBrush(false),
        pixmapBrush(false),
        usesWidgetDC(false),
        rasterOp(Qt::CopyROP),
        pStyle(Qt::SolidLine),
        pWidth(0),
        pColor(0),
        bColor(0),
        gdiplusInUse(false),
        gdiplusEngine(0)
    {
    }

    HWND hwnd;
    HDC hdc;
    HPEN hpen;
    HFONT hfont;
    HBRUSH hbrush;
    HBITMAP hbrushbm;
    HPALETTE holdpal;
    uint flags;

    void *penRef;
    void *brushRef;

    uint nocolBrush:1;
    uint pixmapBrush:1;
    uint usesWidgetDC:1;

    Qt::RasterOp rasterOp;
    Qt::PenStyle pStyle;
    int pWidth;
    COLORREF pColor;
    COLORREF bColor;

    uint fontFlags;

    bool usesGdiplus() { return gdiplusInUse && gdiplusEngine; }
    void beginGdiplus();
    void endGdiplus();
    uint gdiplusInUse : 1;
    QGdiplusPaintEngine *gdiplusEngine;
};

#endif

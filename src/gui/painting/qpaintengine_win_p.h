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

#ifndef QPAINTENGINE_WIN_P_H
#define QPAINTENGINE_WIN_P_H

#include <windows.h>

#include "qnamespace.h"
#include "qpaintengine_p.h"
#include <private/qpainter_p.h>

class QPainterPathPrivate;

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
class QtGpFont { };

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
        temporaryBrush(false)
    {
    }

    HWND hwnd;
    HDC hdc;

    QtGpGraphics *graphics;
    QtGpBitmap *bitmapDevice;
    QtGpPen *pen;
    QtGpBrush *brush;

    QtGpSolidFill *cachedSolidBrush;

    QColor penColor;
    QColor brushColor;

    uint usesTempDC : 1;
    uint usePen : 1;
    uint temporaryBrush : 1;
};

class Q_GUI_EXPORT QWin32PaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWin32PaintEngine)
public:

    QWin32PaintEnginePrivate() :
        hwnd(0),
        hdc(0),
        hpen(0),
        hbrush(0),
        hbrushbm(0),
        holdpal(0),
        flags(0),
        penRef(0),
        brushRef(0),
        nocolBrush(false),
        pixmapBrush(false),
        usesWidgetDC(false),
        forceGdi(false),
        forceGdiplus(false),
        noNativeXform(false),
        advancedMode(false),
        advancedModeUsed(false),
        penStyle(Qt::SolidLine),
        brushStyle(Qt::SolidPattern),
        pWidth(0),
        pColor(0),
        bColor(0),
        txop(QPainterPrivate::TxNone),
        gdiplusInUse(false),
        gdiplusEngine(0)
    {
    }

    HWND hwnd;
    HDC hdc;
    HPEN hpen;
    HBRUSH hbrush;
    HBITMAP hbrushbm;
    HPALETTE holdpal;
    uint flags;

    void *penRef;
    void *brushRef;

    uint nocolBrush:1;
    uint pixmapBrush:1;
    uint usesWidgetDC:1;

    uint forceGdi:1;
    uint forceGdiplus:1;
    uint noNativeXform:1;
    uint advancedMode:1;        // Set if running in advanced graphics mode
    uint advancedModeUsed:1;    // Set if advancedMode was used once.

    Qt::PenStyle penStyle;
    Qt::BrushStyle brushStyle;
    int pWidth;
    COLORREF pColor;
    COLORREF bColor;
    QBrush brush;
    QPen pen;
    Qt::BGMode bgMode;

    QMatrix matrix;
    QPainterPrivate::TransformationCodes txop;

    /*!
     Switches the paint engine into GDI+ mode
    */
    void beginGdiplus();

    /*!
      Returns true if GDI+ is currently in use
    */
    inline bool usesGdiplus() { return gdiplusInUse && gdiplusEngine; }

    /*!
      Returns true if the engine has any property set that requires it to
      use GDI+ for rendering
    */
    inline bool requiresGdiplus() {
        return !forceGdi && forceGdiplus;
    }

    /*!
      Convenience function for checking if the engine should switch to/from
      GDI+. Returns true if GDI+ is in use after the checking is done.
    */
    inline bool tryGdiplus() {
        if (requiresGdiplus() && !usesGdiplus()) {
            beginGdiplus();
        }
        return usesGdiplus();
    }

    /*!
      Fill rect with current gradient brush, using native function call
    */
    void fillGradient(const QRect &r);

    /*!
      Fill the rect current alpha brush
    */
    void fillAlpha(const QRect &r, const QColor &c);

    /*!
      Composes the path on the current device context
    */
    void composeGdiPath(const QPainterPath &p);

    uint gdiplusInUse : 1;
    QGdiplusPaintEngine *gdiplusEngine;
};

#endif

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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qt_windows.h>

#include "qnamespace.h"

#include <private/qpaintengine_p.h>
#include <private/qpainter_p.h>
#include <private/qpolygonclipper_p.h>

class Q_GUI_EXPORT QWin32PaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QWin32PaintEngine)
public:
    QWin32PaintEngine();
    ~QWin32PaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updateState(const QPaintEngineState &state);

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &origin);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
    void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);
    void updateRenderHints(QPainter::RenderHints hints);

    void drawTextItem(const QPointF &p, const QTextItem &textItem);

    void drawTextItemMulti(const QPointF &p, const QTextItem &textItem);
    void drawTextItemWin(const QPointF &p, const QTextItem &textItem);

    void drawPath(const QPainterPath &path);

    inline Type type() const { return QPaintEngine::Windows; }

    static void initialize();
    static void cleanup();

    QPainter::RenderHints supportedRenderHints() const;

    HDC getDC() const;
    void releaseDC(HDC hdc) const;

protected:
    QWin32PaintEngine(QWin32PaintEnginePrivate &dptr, PaintEngineFeatures caps);

protected:
    friend class QPainter;
};

class QPaintDevice;
class QPainterPath;
class QPainterState;
class QTextEngine;
class QTextLayout;
class QWin32PaintEnginePrivate;
class QPainterPathPrivate;

struct qt_float_point
{
    float x, y;
    operator POINT () const { POINT pt = { qRound(x), qRound(y) }; return pt; }
};

class QWin32PaintEnginePrivate : public QPaintEnginePrivate
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
        nocolBrush(false),
        pixmapBrush(false),
        usesWidgetDC(false),
        advancedMode(false),
        ellipseHack(false),
        penStyle(Qt::SolidLine),
        brushStyle(Qt::SolidPattern),
        pWidth(0),
        pColor(0),
        bColor(0),
        txop(QPainterPrivate::TxNone)
    {
    }

    HWND hwnd;
    HDC hdc;
    HPEN hpen;
    HBRUSH hbrush;
    HBITMAP hbrushbm;
    HPALETTE holdpal;
    uint flags;

    uint nocolBrush:1;
    uint pixmapBrush:1;
    uint usesWidgetDC:1;

    uint noNativeXform:1;
    uint advancedMode:1;        // Set if running in advanced graphics mode
    uint ellipseHack:1;         // Used to work around ellipse bug in GDI

    Qt::PenStyle penStyle;
    Qt::BrushStyle brushStyle;
    float pWidth;
    COLORREF pColor;
    COLORREF bColor;
    QBrush brush;
    QPen pen;
    Qt::BGMode bgMode;

    QMatrix matrix;
    QMatrix invMatrix;
    QPainterPrivate::TransformationCodes txop;

    QPaintEngine::PaintEngineFeatures oldFeatureSet;

    QPolygonClipper<qt_float_point, POINT, float> polygonClipper;

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

    void setNativeMatrix(const QMatrix &matrix);
};

#endif // QPAINTENGINE_WIN_P_H

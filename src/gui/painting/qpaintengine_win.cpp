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

#include "qbitmap.h"
#include "qbrush.h"
#include "qcolormap.h"
#include "qlibrary.h"
#include "qpaintdevice.h"
#include "qpainter.h"
#include "qpainter_p.h"
#include "qpainterpath.h"
#include "qpainterpath_p.h"
#include "qpen.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qpolygon.h"
#include "qt_windows.h"
#include "qtextlayout.h"
#include "qvarlengtharray.h"
#include "qwidget.h"

#include <private/qfontengine_p.h>
#include <private/qpaintengine_win_p.h>
#include <private/qtextengine_p.h>
#include <private/qwidget_p.h>

#include <qdebug.h>

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979
#endif

//## this is only defined if winver > 5 on mingw
#ifndef GRADIENT_FILL_TRIANGLE
#define GRADIENT_FILL_TRIANGLE 0x02
#endif

#define QT_NO_NATIVE_XFORM
// #define QT_NO_NATIVE_GRADIENT
// #define QT_NO_NATIVE_PATH
// #define QT_NO_NATIVE_ALPHA

// #define QT_DEBUG_DRAW

#define d d_func()
#define q q_func()

// For alpha blending, we must load the AlphaBlend() function at run time.
#if !defined (AC_SRC_ALPHA)
#define AC_SRC_ALPHA 0x01
#endif

#if !defined (SHADEBLENDCAPS)
#define SHADEBLENDCAPS 120
#endif

#if !defined (SB_CONST_ALPHA)
#define SB_CONST_ALPHA 0x00000001
#endif

#if !defined (SB_GRAD_TRI)
#define SB_GRAD_TRI         0x00000020
#endif

// True if GDI+ and its function could be resolved...
static bool qt_gdiplus_support = false;
static bool qt_resolved_gdiplus = false;

static void qt_resolve_gdiplus();
static QPaintEngine::PaintEngineFeatures qt_decide_paintengine_features();

static QSysInfo::WinVersion qt_winver = QSysInfo::WV_NT;

#define COLOR_VALUE(c) RGB(c.red(),c.green(),c.blue())

/*****************************************************************************
  QPainter internal pen and brush cache

  The cache makes a significant contribution to speeding up drawing.
  Setting a new pen or brush specification will make the painter look for
  an existing pen or brush with the same attributes instead of creating
  a new pen or brush. The cache structure is optimized for fast lookup.
  Only solid line pens with line width 0 and solid brushes are cached.
 *****************************************************************************/
struct QHDCObj                                        // cached pen or brush
{
    HANDLE  obj;
    uint    pix;
    int            count;
    int            hits;
};

const int        cache_size = 29;                // multiply by 4
static QHDCObj *pen_cache_buf;
static QHDCObj *pen_cache[4*cache_size];
static QHDCObj *brush_cache_buf;
static QHDCObj *brush_cache[4*cache_size];
static bool        cache_init = false;

static HPEN stock_nullPen;
static HPEN stock_blackPen;
static HPEN stock_whitePen;
static HBRUSH stock_nullBrush;
static HBRUSH stock_blackBrush;
static HBRUSH stock_whiteBrush;
static HFONT  stock_sysfont;

static QHDCObj stock_dummy;
static void  *stock_ptr = (void *)&stock_dummy;

static void init_cache()
{
    if (cache_init)
        return;
    int i;
    QHDCObj *h;
    cache_init = true;
    h = pen_cache_buf = new QHDCObj[4*cache_size];
    memset(h, 0, 4*cache_size*sizeof(QHDCObj));
    for (i=0; i<4*cache_size; i++)
        pen_cache[i] = h++;
    h = brush_cache_buf = new QHDCObj[4*cache_size];
    memset(h, 0, 4*cache_size*sizeof(QHDCObj));
    for (i=0; i<4*cache_size; i++)
        brush_cache[i] = h++;
}

QRegion* paintEventClipRegion = 0;
QPaintDevice* paintEventDevice = 0;

void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region)
{
    if (!paintEventClipRegion)
        paintEventClipRegion = new QRegion(region);
    else
        *paintEventClipRegion = region;
    paintEventDevice = dev;
}

void qt_clear_paintevent_clipping()
{
    delete paintEventClipRegion;
    paintEventClipRegion = 0;
    paintEventDevice = 0;
}

// #define CACHE_STAT
#if defined(CACHE_STAT)
#include "qtextstream.h"

static int c_numhits        = 0;
static int c_numcreates = 0;
static int c_numfaults        = 0;
#endif

static void cleanup_cache()
{
    if (!cache_init)
        return;
    int i;
#if defined(CACHE_STAT)
    qDebug("Number of cache hits = %d", c_numhits);
    qDebug("Number of cache creates = %d", c_numcreates);
    qDebug("Number of cache faults = %d", c_numfaults);
    qDebug("PEN CACHE");
    for (i=0; i<cache_size; i++) {
        QString            str;
        QTextStream s(str,QIODevice::WriteOnly);
        s << i << ": ";
        for (int j=0; j<4; j++) {
            QHDCObj *h = pen_cache[i*4+j];
            s << (h->obj ? 'X' : '-') << ',' << h->hits << ','
              << h->count << '\t';
        }
        s << '\0';
        qDebug(str);
    }
    qDebug("BRUSH CACHE");
    for (i=0; i<cache_size; i++) {
        QString            str;
        QTextStream s(str,QIODevice::WriteOnly);
        s << i << ": ";
        for (int j=0; j<4; j++) {
            QHDCObj *h = brush_cache[i*4+j];
            s << (h->obj ? 'X' : '-') << ',' << h->hits << ','
              << h->count << '\t';
        }
        s << '\0';
        qDebug(str);
    }
#endif
    for (i=0; i<4*cache_size; i++) {
        if (pen_cache[i]->obj)
            DeleteObject(pen_cache[i]->obj);
        if (brush_cache[i]->obj)
            DeleteObject(brush_cache[i]->obj);
    }
    delete [] pen_cache_buf;
    delete [] brush_cache_buf;
    cache_init = false;
}


static bool obtain_obj(void **ref, HANDLE *obj, uint pix, QHDCObj **cache,
                        bool is_pen)
{
    if (!cache_init)
        init_cache();

    int             k = (pix % cache_size) * 4;
    QHDCObj *h = cache[k];
    QHDCObj *prev = 0;

#define NOMATCH (h->obj && h->pix != pix)

    if (NOMATCH) {
        prev = h;
        h = cache[++k];
        if (NOMATCH) {
            prev = h;
            h = cache[++k];
            if (NOMATCH) {
                prev = h;
                h = cache[++k];
                if (NOMATCH) {
                    if (h->count == 0) {        // steal this pen/brush
#if defined(CACHE_STAT)
                        c_numcreates++;
#endif
                        h->pix         = pix;
                        h->count = 1;
                        h->hits         = 1;
                        DeleteObject(h->obj);
                        if (is_pen)
                            h->obj = CreatePen(PS_SOLID, 0, pix);
                        else
                            h->obj = CreateSolidBrush(pix);
                        cache[k]   = prev;
                        cache[k-1] = h;
                        *ref = (void *)h;
                        *obj = h->obj;
                        return true;
                    } else {                        // all objects in use
#if defined(CACHE_STAT)
                        c_numfaults++;
#endif
                        *ref = 0;
                        return false;
                    }
                }
            }
        }
    }

#undef NOMATCH

    *ref = (void *)h;

    if (h->obj) {                                // reuse existing pen/brush
#if defined(CACHE_STAT)
        c_numhits++;
#endif
        *obj = h->obj;
        h->count++;
        h->hits++;
        if (prev && h->hits > prev->hits) {        // maintain LRU order
            cache[k]   = prev;
            cache[k-1] = h;
        }
    } else {                                        // create new pen/brush
#if defined(CACHE_STAT)
        c_numcreates++;
#endif
        if (is_pen)
            h->obj = CreatePen(PS_SOLID, 0, pix);
        else
            h->obj = CreateSolidBrush(pix);
        h->pix         = pix;
        h->count = 1;
        h->hits         = 1;
        *obj = h->obj;
    }
    return true;
}

static inline void release_obj(void *ref)
{
    ((QHDCObj*)ref)->count--;
}

static inline bool obtain_pen(void **ref, HPEN *pen, uint pix)
{
    return obtain_obj(ref, (HANDLE*)pen, pix, pen_cache, true);
}

static inline bool obtain_brush(void **ref, HBRUSH *brush, uint pix)
{
    return obtain_obj(ref, (HANDLE*)brush, pix, brush_cache, false);
}

#define release_pen        release_obj
#define release_brush        release_obj

/********************************************************************************
 * GDI functions we need to dynamically resolve due for 9x based.
 */

typedef int (__stdcall *PtrModifyWorldTransform)(HDC hdc, const XFORM *, uint);
typedef int (__stdcall *PtrSetGraphicsMode)(HDC hdc, uint);
typedef int (__stdcall *PtrSetWorldTransform)(HDC hdc, const XFORM *);

typedef int (__stdcall *PtrAlphaBlend)(HDC, int, int, int, int,
                                           HDC, int, int, int, int,
                                           BLENDFUNCTION);
typedef int (__stdcall *PtrGradientFill)(HDC, PTRIVERTEX, ulong, void *, ulong, ulong);

static PtrModifyWorldTransform qModifyWorldTransform = 0;
static PtrSetGraphicsMode qSetGraphicsMode = 0;
static PtrSetWorldTransform qSetWorldTransform = 0;

static PtrAlphaBlend qAlphaBlend = 0;
static PtrGradientFill qGradientFill = 0;


void qt_resolve_gdi_functions()
{
    static bool resolved = false;
    if (resolved)
        return;
    resolved = true;

    QLibrary gdi32("gdi32");
    gdi32.load();
    if (gdi32.isLoaded()) {
        qSetGraphicsMode = (PtrSetGraphicsMode) gdi32.resolve("SetGraphicsMode");
        qSetWorldTransform = (PtrSetWorldTransform) gdi32.resolve("SetWorldTransform");
        qModifyWorldTransform = (PtrModifyWorldTransform) gdi32.resolve("ModifyWorldTransform");
    }

    QLibrary img32("msimg32");
    img32.load();
    if (img32.isLoaded()) {
        qAlphaBlend = (PtrAlphaBlend) img32.resolve("AlphaBlend");
        qGradientFill = (PtrGradientFill) img32.resolve("GradientFill");
    }

#ifdef QT_DEBUG_DRAW
    printf("qSetGraphicsMode..........: %p\n", qSetGraphicsMode);
    printf("qSetWorldTransform........: %p\n", qSetWorldTransform);
    printf("qModifyWorldTransform.....: %p\n", qModifyWorldTransform);
    printf("qAlphaBlend...............: %p\n", qAlphaBlend);
#endif
}

QWin32PaintEngine::QWin32PaintEngine(QWin32PaintEnginePrivate &dptr,
                                     PaintEngineFeatures caps)
    : QPaintEngine(dptr, caps)
{
    d->flags |= IsStartingUp;
}

QWin32PaintEngine::QWin32PaintEngine()
    : QPaintEngine(*(new QWin32PaintEnginePrivate), qt_decide_paintengine_features())
{
    d->flags |= IsStartingUp;
}

QWin32PaintEngine::~QWin32PaintEngine()
{
    delete d->gdiplusEngine;
}

bool QWin32PaintEngine::begin(QPaintDevice *pdev)
{
    if (isActive()) {                                // already active painting
        qWarning("QWin32PaintEngine::begin: Painter is already active."
               "\n\tYou must end() the painter before a second begin()\n");
        return true;
    }

    // Store old features set since we change it when switching to GDI+.
    d->oldFeatureSet = gccaps;

    d->forceGdiplus = false;
    d->forceGdi = false;
    d->ellipseHack = false;
    d->advancedMode = false;

    setActive(true);

    Q_ASSERT(pdev);
    if (!d->hdc) {
        d->hdc = pdev->getDC();
        if (pdev->devType() == QInternal::Widget) {
            QWidget *w = static_cast<QWidget *>(pdev);
            d->usesWidgetDC = (d->hdc != 0);
            if (!d->usesWidgetDC) {
                if (w->testAttribute(Qt::WA_PaintUnclipped)) {
                    d->hdc = GetWindowDC(w->winId());
                    if (w->isTopLevel()) {
                        int dx = w->geometry().x() - w->frameGeometry().x();
                        int dy = w->geometry().y() - w->frameGeometry().y();
#ifndef Q_OS_TEMP
                        SetWindowOrgEx(d->hdc, -dx, -dy, 0);
#else
                        //                    MoveWindow(w->winId(), w->frameGeometry().x(), w->frameGeometry().y(), w->frameGeometry().width(), w->frameGeometry().height(), false);
                        //                    MoveWindow(w->winId(), w->frameGeometry().x() - 50, w->frameGeometry().y() - 50, w->frameGeometry().width(), w->frameGeometry().height(), false);
#endif
                    }
                } else {
                    d->hdc = GetDC(w->isDesktop() ? 0 : w->winId());
                }
                const_cast<QWidgetPrivate *>(w->d)->hd = (Qt::HANDLE)d->hdc;
            }
        }
        Q_ASSERT(d->hdc);
    }

    QRegion *region = paintEventClipRegion;
    if (region && pdev == paintEventDevice)
        SelectClipRgn(d->hdc, region->handle());

    HPALETTE hpal = QColormap::hPal();
    if (hpal) {
        d->holdpal = SelectPalette(d->hdc, hpal, true);
        RealizePalette(d->hdc);
    }

    SetBkMode(d->hdc, TRANSPARENT);
    SetTextAlign(d->hdc, TA_BASELINE);
    SetTextColor(d->hdc, RGB(0, 0, 0));
    SelectObject(d->hdc, stock_nullBrush);
    SelectObject(d->hdc, stock_blackPen);

    setDirty(QPaintEngine::DirtyBackground);
    setDirty(QPaintEngine::DirtyBrush);

    d->pColor = RGB(0, 0, 0);
    d->bColor = RGB(0, 0, 0);

    // force a call to switch advanced mode on/off
    d->setNativeMatrix(QMatrix());

    // GDI does not support drawing on pixmaps with alpha channel so we
    // try to use GDI+
    if (d->pdev->devType() == QInternal::Pixmap && static_cast<QPixmap*>(d->pdev)->hasAlphaChannel())
        d->forceGdiplus = true;

    return true;
}

bool QWin32PaintEngine::end()
{
    if (!isActive()) {
        qWarning("QWin32PaintEngine::end: Missing begin() or begin() failed");
        return false;
    }

    if (d->gdiplusEngine) {
        d->gdiplusEngine->end();
        delete d->gdiplusEngine;
        d->gdiplusEngine = 0;
    }

    //     killPStack();

    if (d->hpen) {
        SelectObject(d->hdc, stock_blackPen);
        if (d->penRef) {
            release_pen(d->penRef);
            d->penRef = 0;
        } else {
            DeleteObject(d->hpen);
        }
        d->hpen = 0;
    }
    if (d->hbrush) {
        SelectObject(d->hdc, stock_nullBrush);
        if (d->brushRef) {
            release_brush(d->brushRef);
            d->brushRef = 0;
        } else {
            DeleteObject(d->hbrush);
            if (d->hbrushbm && !d->pixmapBrush)
                DeleteObject(d->hbrushbm);
        }
        d->hbrush = 0;
        d->hbrushbm = 0;
        d->pixmapBrush = d->nocolBrush = false;
    }

    SelectObject(d->hdc, stock_sysfont);

    if (d->holdpal) {
        SelectPalette(d->hdc, d->holdpal, true);
        RealizePalette(d->hdc);
    }

    if (d->pdev->devType() == QInternal::Widget && !d->usesWidgetDC) {
        QWidget *w = static_cast<QWidget*>(d->pdev);
        ReleaseDC(w->isDesktop() ? 0 : w->winId(), d->hdc);
        const_cast<QWidgetPrivate*>(w->d)->hd = 0;
    } else {
        d->pdev->releaseDC(d->hdc);
    }

    SelectClipRgn(d->hdc, 0);

    d->hdc = 0;

    d->matrix = QMatrix();
    d->noNativeXform = false;
    d->advancedMode = false;
    d->penStyle = Qt::SolidLine;
    d->brushStyle = Qt::SolidPattern;
    d->txop = QPainterPrivate::TxNone;

    // Restore features set in case GDI+ has been used.
    gccaps = d->oldFeatureSet;
    setActive(false);
    return true;
}

void QWin32PaintEngine::drawLine(const QLineF &line)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " -> QWin32PAintEngine::drawLine()" << line;
#endif
    Q_ASSERT(isActive());

    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawLine(line);
        return;
    }

    float x1 = line.startX(), x2 = line.endX(), y1 = line.startY(), y2 = line.endY();
    bool plot_pixel = false;
    plot_pixel = (d->pWidth == 0) && (d->penStyle == Qt::SolidLine);
    if (plot_pixel) {
        if (x1 == x2) {                                // vertical
            if (y1 < y2)
                y2++;
            else
                y2--;
            plot_pixel = false;
        } else if (y1 == y2) {                        // horizontal
            if (x1 < x2)
                x2++;
            else
                x2--;
            plot_pixel = false;
        }
    }
    bool path = false;
#ifndef Q_OS_TEMP
    QT_WA({
        if (d->pWidth > 1) {
            // on DOS based systems caps and joins are only supported on paths, so let's use them.
            BeginPath(d->hdc);
            path = true;
        }
    }, { });
    MoveToEx(d->hdc, x1, y1, 0);
    LineTo(d->hdc, x2, y2);
    if (path) {
        EndPath(d->hdc);
        StrokePath(d->hdc);
        MoveToEx(d->hdc, x2, y2, 0);
    }
    if (plot_pixel)
        SetPixelV(d->hdc, x2, y2, d->pColor);
#else
    POINT pts[2];
    pts[0].x = x1;  pts[0].y = y1;
    pts[1].x = x2;  pts[1].y = y2;
    if (x1 != x2)
        pts[1].x += (x2 > x1 ? 1 : -1);
    if (y1 != y2)
        pts[1].y += (y2 > y1 ? 1 : -1);

    Polyline(d->hdc, pts, 2);
    if (plot_pixel)
        SetPixel(d->hdc, x2, y2, d->pColor);
    internalCurrentPos = QPoint(x2, y2);
#endif
}

void QWin32PaintEngine::drawRect(const QRectF &r)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QWin32PaintEngine::drawRect()" << r;
#endif

#ifdef QT_NO_NATIVE_GRADIENT
    Q_ASSERT(d->brushStyle != Qt::LinearGradientPattern);
#endif // QT_NO_NATIVE_GRADIENT

    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawRect(r);
        return;
    }
    float w = r.width(), h = r.height();

    if (!d->advancedMode) {
        ++w;
        ++h;
    }

    bool outlineOnly = false;
    if (d->brushStyle == Qt::LinearGradientPattern) {
        d->fillGradient(r.toRect());
        outlineOnly = true;
    }
#ifndef QT_NO_NATIVE_ALPHA
    else if (d->brushStyle == Qt::SolidPattern && d->brush.color().alpha() != 255) {
        d->fillAlpha(r.toRect(), d->brush.color());
        outlineOnly = true;
    }
#endif

    if (outlineOnly) {
        if (d->penStyle != Qt::NoPen) {
            SelectObject(d->hdc, stock_nullBrush);
            Rectangle(d->hdc, qRound(r.x()), qRound(r.y()), qRound(r.x() + w), qRound(r.y() + h));
            SelectObject(d->hdc, d->hbrush);
        }
        return;
    }

    if (d->nocolBrush) {
        COLORREF old = SetTextColor(d->hdc, d->bColor);
        Rectangle(d->hdc, qRound(r.x()), qRound(r.y()), qRound(r.x()+w), qRound(r.y()+h));
        SetTextColor(d->hdc, old);
    } else {
        Rectangle(d->hdc, qRound(r.x()), qRound(r.y()), qRound(r.x()+w), qRound(r.y()+h));
    }
}

void QWin32PaintEngine::drawPoint(const QPointF &p)
{
#ifdef QT_DEBUG_DRAW
    printf(" - QWin32PaintEngine::drawPoint() (%.2f, %.2f)\n", p.x(), p.y());
#endif
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPoint(p);
        return;
    }

    if (d->penStyle != Qt::NoPen)
#ifndef Q_OS_TEMP
        SetPixelV(d->hdc, qRound(p.x()), qRound(p.y()), d->pColor);
#else
        SetPixel(d->hdc, p.x(), p.y(), d->pColor);
#endif
}

void QWin32PaintEngine::drawEllipse(const QRectF &r)
{
#ifdef QT_DEBUG_DRAW
    printf(" - QWin32PaintEngine::drawEllipse() (%.2f, %.2f, %.2f, %.2f)\n",
           r.x(), r.y(), r.width(), r.height());
#endif
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawEllipse(r);
        return;
    }

#ifndef QT_NO_NATIVE_PATH
    if ((d->brushStyle == Qt::SolidPattern && d->brush.color().alpha() != 255)
        || d->brushStyle == Qt::LinearGradientPattern) {
        QPainterPath path;
        path.addEllipse(r.x(), r.y(), r.width(), r.height());
        drawPath(path);
        return;
    }
#endif // QT_NO_NATIVE_PATH

    // Ellipse sizes differ depending on whether we have been in ADVANCED mode or not
    // so make sure it is the case.
    if (!d->ellipseHack) {
        QPainterPrivate::TransformationCodes txop = d->txop;
        d->txop = QPainterPrivate::TxScale;
        d->setNativeMatrix(QMatrix(2, 0, 0, 2, 0, 0));
        d->txop = txop;
        d->setNativeMatrix(QMatrix());
        d->ellipseHack = true;
    }

    HGDIOBJ oldPen;
    int reduction = 0;
    if (d->penStyle == Qt::NoPen && d->brushStyle != Qt::NoBrush) {
        // ellipse outlines look so much better that this is worth it
        oldPen = SelectObject(d->hdc, CreatePen(PS_SOLID, 0, d->bColor));
        reduction = 1;
    }

    Ellipse(d->hdc, qRound(r.x()), qRound(r.y()),
            qRound(r.x() + r.width() - reduction),
            qRound(r.y() + r.height() - reduction));

    // Restore the state and delete the temporary pen.
    if (d->penStyle == Qt::NoPen)
        DeleteObject(SelectObject(d->hdc, oldPen));
}

void QWin32PaintEngine::drawPolygon(const QPolygon &p, PolygonDrawMode mode)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " -> QWin32PaintEngine::drawPolygon()" << p.size() << mode;
    for (int i=0; i<p.size(); ++i)
        qDebug() << " --->" << p.at(i);
#endif
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPolygon(p, mode);
        return;
    }

    if (mode == PolylineMode) {
        int x1, y1, x2, y2;
        int npoints = p.size();
        x1 = qRound(p.at(npoints-2).x());
        y1 = qRound(p.at(npoints-2).y());
        x2 = qRound(p.at(npoints-1).x());
        y2 = qRound(p.at(npoints-1).y());
        bool plot_pixel = false;
        QT_WA({
            plot_pixel = (d->pWidth == 0) && (d->penStyle == Qt::SolidLine);
        } , {
            plot_pixel = (d->pWidth <= 1) && (d->penStyle == Qt::SolidLine);
        });

        if (plot_pixel) {
            if (x1 == x2) {                                // vertical
                if (y1 < y2)
                    y2++;
                else
                    y2--;
                plot_pixel = false;
            } else if (y1 == y2) {                        // horizontal
                if (x1 < x2)
                    x2++;
                else
                    x2--;
                plot_pixel = false;
            }
        }
        if (plot_pixel) {
            Polyline(d->hdc, (POINT*)p.toPointArray().data(), npoints);
#ifndef Q_OS_TEMP
            SetPixelV(d->hdc, x2, y2, d->pColor);
#else
            SetPixel(d->hdc, x2, y2, d->pColor);
#endif
        } else {
            QPointArray copy = p.toPointArray();
            copy.setPoint(npoints-1, x2, y2);
            Polyline(d->hdc, (POINT*)(copy.data()), npoints);
        }
        return;
    }

#ifndef QT_NO_NATIVE_PATH
    // Fall back to path implementation for gradient and alpha brushes..
    if (d->brushStyle == Qt::LinearGradientPattern ||
        d->brushStyle == Qt::SolidPattern && d->brush.color().alpha() != 255) {
        QPainterPath path;
        path.addPolygon(p);
        drawPath(path);
        return;
    }
#endif // QT_NO_NATIVE_PATH

#ifndef Q_OS_TEMP
    if (mode == WindingMode)                    // set to winding fill mode
        SetPolyFillMode(d->hdc, WINDING);
#endif
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->bColor);
    Polygon(d->hdc, (POINT*)p.toPointArray().data(), p.size());
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->pColor);
#ifndef Q_OS_TEMP
    if (mode == WindingMode)                    // set to normal fill mode
        SetPolyFillMode(d->hdc, ALTERNATE);
#endif

}

void QWin32PaintEngine::drawPath(const QPainterPath &p)
{
#ifdef QT_NO_NATIVE_PATH
    Q_ASSERT(!"QWin32PaintEngine::drawPath(), QT_NO_NATIVE_PATH is defined...\n");
    return;
#endif
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPath(p);
        return;
    }

    d->composeGdiPath(p);

    if (p.fillRule() == Qt::WindingFill)
        SetPolyFillMode(d->hdc, WINDING);

    bool pen = d->penStyle != Qt::NoPen;
    bool brush = d->brushStyle != Qt::NoBrush;

    // Workaround for filling gradients
    if (d->brushStyle == Qt::LinearGradientPattern
        || (d->brushStyle == Qt::SolidPattern && d->brush.color().alpha() != 255)) {
        HRGN oldRegion = 0;
        int gotRegion = GetClipRgn(d->hdc, oldRegion);
        SelectClipPath(d->hdc, RGN_AND);
        if (d->brushStyle == Qt::LinearGradientPattern)
            d->fillGradient(p.boundingRect().toRect());
        else
            d->fillAlpha(p.boundingRect().toRect(), d->brush.color());
        brush = false;
        if (gotRegion <= 0) { // No path originally
            SelectClipRgn(d->hdc, 0);
        } else if (gotRegion == 1){ // Reset and release original path
            Q_ASSERT(oldRegion);
            SelectClipRgn(d->hdc, oldRegion);
            DeleteObject(oldRegion);
        }
        if (pen)
            d->composeGdiPath(p);
    }

    if (pen && brush)
        StrokeAndFillPath(d->hdc);
    else if (pen)
        StrokePath(d->hdc);
    else if (brush)
        FillPath(d->hdc);

    if (p.fillRule() == Qt::WindingFill)
        SetPolyFillMode(d->hdc, ALTERNATE);


}


void QWin32PaintEngine::initialize()
{
    qt_resolve_gdi_functions();
    stock_nullPen    = (HPEN)GetStockObject(NULL_PEN);
    stock_blackPen   = (HPEN)GetStockObject(BLACK_PEN);
    stock_whitePen   = (HPEN)GetStockObject(WHITE_PEN);
    stock_nullBrush  = (HBRUSH)GetStockObject(NULL_BRUSH);
    stock_blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
    stock_whiteBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    stock_sysfont    = (HFONT)GetStockObject(SYSTEM_FONT);
    init_cache();
}


void QWin32PaintEngine::cleanup()
{
    cleanup_cache();
    if (qt_gdiplus_support) {
        QGdiplusPaintEngine::cleanup();
    }
}


void QWin32PaintEngine::drawPixmap(const QRectF &rf, const QPixmap &sourcePm, const QRectF &srf,
                                   Qt::PixmapDrawingMode mode)
{
#if defined QT_DEBUG_DRAW
    printf(" - QWin32PaintEngine::drawPixmap(), [%.2f,%.2f,%.2f,%.2f], size=[%d,%d], "
           "sr=[%.2f,%.2f,%.2f,%.2f], mode=%d\n",
           rf.x(), rf.y(), rf.width(), rf.height(),
           sourcePm.width(), sourcePm.height(),
           srf.x(), srf.y(), srf.width(), srf.height(),
           mode);
#endif

    QRect r = rf.toRect();
    QRect sr = srf.toRect();

    Q_ASSERT(isActive());

    bool stretch = r.width() != sr.width() || r.height() != sr.height();

    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPixmap(r, sourcePm, sr, mode);
        return;
    }

    const QPixmap *pixmap = &sourcePm;
    bool srcIsAlpha = pixmap->hasAlphaChannel();
    bool targetIsAlpha = d->pdev->devType() == QInternal::Pixmap
                         && static_cast<QPixmap *>(d->pdev)->hasAlphaChannel();

    // Source needs to be alpha if target is.
    if (targetIsAlpha && !srcIsAlpha) {
        QString key = QString("qt_src_not_alpha: sn=%1").arg(pixmap->serialNumber());
        QPixmap *cached = QPixmapCache::find(key);
        if (!cached) {
            QImage im = pixmap->toImage();
            im = im.convertDepth(32);
            im.setAlphaBuffer(true);
            QPixmap tmp = im;
            QPixmapCache::insert(key, tmp);
            pixmap = QPixmapCache::find(key);
            Q_ASSERT_X(pixmap,
                       "QWin32PaintEngine::drawPixmap()",
                       "We just inserted pixmap into cache so it should have been there");
        }
    }

    if (d->pen.color().alpha() != 255 && pixmap->isQBitmap()) {
        QRegion region(*static_cast<const QBitmap*>(pixmap));
        region.translate(qRound(r.x()), qRound(r.y()));
        updateClipRegion(region, Qt::IntersectClip);
        d->fillAlpha(r, d->pen.color());
        setDirty(DirtyClip);

        return;
    }

    QPixmap *pm = (QPixmap*)pixmap;
    QBitmap *mask = (QBitmap*)pm->mask();
    if (!mask && pixmap->isQBitmap() && d->bgMode == Qt::TransparentMode)
       mask = (QBitmap*) pm;

    HDC pm_dc = pm->getDC();

    if (mode == Qt::CopyPixmapNoMask)
        mask = 0;

    Q_ASSERT(pm_dc);

    if (qAlphaBlend && (srcIsAlpha || targetIsAlpha) && mode == Qt::ComposePixmap) {
        const BLENDFUNCTION bf = { AC_SRC_OVER,       // BlendOp
                                   0,                 // BlendFlags, must be zero
                                   255,               // SourceConstantAlpha, we use pr pixel
                                   AC_SRC_ALPHA       // AlphaFormat
        };
        if (!qAlphaBlend(d->hdc, r.x(), r.y(), r.width(), r.height(),
                         pm_dc, sr.x(), sr.y(), sr.width(), sr.height(), bf)) {
            qErrnoWarning("QWin32PaintEngine::drawPixmap, AlphaBlend failed");
        }
    } else if (mask && mode == Qt::ComposePixmap) {
        if (stretch) {
            QImage imageData = pixmap->toImage();
            QImage imageMask = imageData.createAlphaMask();
            QBitmap tmpbm = imageMask;
            QBitmap bm(sr.width(), sr.height());
            {
                QPainter p(&bm);
                p.drawPixmap(QRectF(0, 0, srf.width(), srf.height()).toRect(),
                             tmpbm, sr, Qt::CopyPixmapNoMask);
            }
            QMatrix xform = QMatrix(r.width()/(double)sr.width(), 0,
                                      0, r.height()/(double)sr.height(),
                                      0, 0);
            bm = bm.transform(xform);
            QRegion region(bm);
            region.translate(r.x(), r.y());
            if (state->painter->hasClipping())
                region &= state->painter->clipRegion();
            state->painter->save();
            state->painter->setClipRegion(region);
            updateState(state);
            StretchBlt(d->hdc, r.x(), r.y(), r.width(), r.height(),
                       pm_dc, sr.x(), sr.y(), sr.width(), sr.height(),
                       SRCCOPY);
            state->painter->restore();
        } else {
            if (!MaskBlt(d->hdc, r.x(), r.y(), sr.width(), sr.height(),
                         pm_dc, sr.x(), sr.y(), mask->hbm(), sr.x(), sr.y(),
                         MAKEROP4(0x00aa0000, SRCCOPY)))
                qErrnoWarning("QWin32PaintEngine::drawPixmap: MaskBlt failed");
        }
    } else {
        if (stretch) {
            Q_ASSERT((GetDeviceCaps(d->hdc, RASTERCAPS) & RC_STRETCHBLT) != 0);
            if (!StretchBlt(d->hdc, r.x(), r.y(), r.width(), r.height(),
                            pm_dc, sr.x(), sr.y(), sr.width(), sr.height(),
                            SRCCOPY)) {
                qErrnoWarning("QWin32PaintEngine::drawPixmap: StretchBlt failed");
            }
        } else {
            Q_ASSERT((GetDeviceCaps(d->hdc, RASTERCAPS) & RC_BITBLT) != 0);
            if (!BitBlt(d->hdc, r.x(), r.y(), sr.width(), sr.height(),
                        pm_dc, sr.x(), sr.y(),
                        SRCCOPY)) {
                qErrnoWarning("QWin32PaintEngine::drawPixmap: BitBlt failed");
            }
        }
    }
    pm->releaseDC(pm_dc);
}

void QWin32PaintEngine::drawTextItem(const QPointF &pos, const QTextItem &ti)
{
#ifdef QT_DEBUG_DRAW
        printf(" - QWin32PaintEngine::drawTextItem(), (%.2f,%.2f), string=%s\n",
               pos.x(), pos.y(), QString::fromRawData(ti.chars, ti.num_chars).latin1());
#endif

    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawTextItem(pos, ti);
        return;
    }

    QPointF p = pos;

    if (d->txop > QPainterPrivate::TxTranslate) {
        d->setNativeMatrix(d->matrix);
    } else if (d->txop == QPainterPrivate::TxTranslate) {
        p = p * d->matrix;
    }

    QFontEngine *fe = ti.fontEngine;
    QPainterState *state = painterState();

    double scale = 1.;
    int angle = 0;
    bool transform = false;
    float x = p.x();
    float y = p.y();

    if (state->txop >= QPainterPrivate::TxScale
        && !(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) {
        // Draw rotated and sheared text on Windows 95, 98

        // All versions can draw rotated text natively. Scaling can be done with window/viewport transformations.
        // Shearing transformations are done by QPainter.

        // rotation + scale + translation
        scale = sqrt(state->matrix.m11()*state->matrix.m22()
                      - state->matrix.m12()*state->matrix.m21());
        angle = qRound(1800*acos(state->matrix.m11()/scale)/M_PI);
        if (state->matrix.m12() < 0)
            angle = 3600 - angle;

        transform = true;
    }

    if (ti.flags & (QTextItem::Underline|QTextItem::StrikeOut) || scale != 1. || angle) {
        LOGFONT lf = fe->logfont;
        lf.lfUnderline = (ti.flags & QTextItem::Underline);
        lf.lfStrikeOut = (ti.flags & QTextItem::StrikeOut);
        if (angle) {
            lf.lfOrientation = -angle;
            lf.lfEscapement = -angle;
        }
        if (scale != 1.) {
            lf.lfHeight = (int) (lf.lfHeight*scale);
            lf.lfWidth = (int) (lf.lfWidth*scale);
        }
        HFONT hf = QT_WA_INLINE(CreateFontIndirectW(&lf), CreateFontIndirectA((LOGFONTA*)&lf));
        SelectObject(d->hdc, hf);
    } else {
        SelectObject(d->hdc, fe->hfont);
    }

    unsigned int options =  fe->ttf ? ETO_GLYPH_INDEX : 0;

    QGlyphLayout *glyphs = ti.glyphs;

#if 0
    // ###### should be moved to the printer GC
    if(p->pdev->devType() == QInternal::Printer) {
        // some buggy printer drivers can't handle glyph indices correctly for latin1
        // If the string is pure latin1, we output the string directly, not the glyph indices.
        // There must be a better way to get this working, but currently I can't think of one.
        const QChar *uc = engine->string.unicode() + ti.position;
        int l = engine->length(si - &engine->items[0]);
        int i = 0;
        bool latin = (l == ti.num_glyphs);
        while (latin && i < l) {
            if(uc[i].unicode() >= 0x100)
                latin = false;
            ++i;
        }
        if(latin) {
            glyphs = (glyph_t *)uc;
            options = 0;
        }
    }
#endif

    int xo = qRound(x);

    if (!(ti.flags & QTextItem::RightToLeft)) {
        // hack to get symbol fonts working on Win95. See also QFontEngine constructor
        if (fe->useTextOutA) {
            // can only happen if !ttf
            for(int i = 0; i < ti.num_glyphs; i++) {
                QString str(QChar(glyphs->glyph));
                QByteArray cstr = str.toLocal8Bit();
                TextOutA(d->hdc, x + qRound(glyphs->offset.x()), y + qRound(glyphs->offset.y()),
                         cstr.data(), cstr.length());
                x += qRound(glyphs->advance.x());
                glyphs++;
            }
        } else {
            bool haveOffsets = false;
            float w = 0;
            for(int i = 0; i < ti.num_glyphs; i++) {
                if (glyphs[i].offset.x() != 0 || glyphs[i].offset.y() != 0 || glyphs[i].space_18d6 != 0) {
                    haveOffsets = true;
                    break;
                }
                w += glyphs[i].advance.x();
            }

            if (haveOffsets || transform) {
                for(int i = 0; i < ti.num_glyphs; i++) {
                    wchar_t chr = glyphs->glyph;
                    float xp = x + glyphs->offset.x();
                    float yp = y + glyphs->offset.y();
                    if (transform)
                        state->painter->matrix().map(xp, yp, &xp, &yp);
                    ExtTextOutW(d->hdc, qRound(xp), qRound(yp), options, 0, &chr, 1, 0);
                    x += glyphs->advance.x() + ((float)glyphs->space_18d6) / 64.;
                    y += glyphs->advance.y();
                    glyphs++;
                }
            } else {
                // fast path
                QVarLengthArray<wchar_t> g(ti.num_glyphs);
                for (int i = 0; i < ti.num_glyphs; ++i)
                    g[i] = glyphs[i].glyph;
                // fast path
                ExtTextOutW(d->hdc,
                            qRound(x + glyphs->offset.x()),
                            qRound(y + glyphs->offset.y()),
                            options, 0, g.data(), ti.num_glyphs, 0);
                x += w;
            }
        }
    } else {
        int i = ti.num_glyphs;
        while(i--) {
            x += glyphs[i].advance.x() + ((float)glyphs[i].space_18d6) / 64.;
            y += glyphs[i].advance.y();
        }
        i = 0;
        while(i < ti.num_glyphs) {
            x -= glyphs[i].advance.x();
            y -= glyphs[i].advance.y();

            int xp = qRound(x+glyphs[i].offset.x());
            int yp = qRound(y+glyphs[i].offset.y());
            ExtTextOutW(d->hdc, xp, yp, options, 0, &glyphs[i].glyph, 1, 0);

            if (glyphs[i].nKashidas) {
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                ti.fontEngine->stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < glyphs[i].nKashidas; ++k) {
                    x -= g[0].advance.x();
                    y -= g[0].advance.y();

                    int xp = qRound(x+g[0].offset.x());
                    int yp = qRound(y+g[0].offset.y());
                    ExtTextOutW(d->hdc, xp, yp, options, 0, &g[0].glyph, 1, 0);
                }
            } else {
                x -= ((float)glyphs[i].space_18d6) / 64;
            }
            ++i;
        }
    }

    if (ti.flags & (QTextItem::Underline|QTextItem::StrikeOut) || scale != 1. || angle)
        DeleteObject(SelectObject(d->hdc, fe->hfont));

    if (ti.flags & (QTextItem::Overline)) {
        int lw = qRound(fe->lineThickness());
        int yp = qRound(y - fe->ascent() - 1);
        Rectangle(d->hdc, xo, yp, qRound(x), yp + lw);

    }

    if (d->txop >= QPainterPrivate::TxTranslate)
        d->setNativeMatrix(QMatrix());
}

void QWin32PaintEngine::updatePen(const QPen &pen)
{
#ifdef QT_DEBUG_DRAW
    static int counter = 0;
    printf(" - QWin32PaintEngine::updatePen(), style=%d, color=%p, calls=%d\n",
           pen.style(), pen.color().rgba(), ++counter);
#endif
    d->pen = pen;
    d->penStyle = pen.style();
    d->forceGdiplus |= d->penStyle != Qt::NoPen && pen.color().alpha() != 255;
    if (d->tryGdiplus()) {
        d->gdiplusEngine->updatePen(pen);
        return;
    }

    d->pColor = COLOR_VALUE(pen.color());
    d->pWidth = pen.width();
    bool cacheIt = (d->penStyle == PS_NULL || (d->penStyle == PS_SOLID && pen.width() == 0));
    HANDLE hpen_old;

    if (d->penRef) {
        release_pen(d->penRef);
        d->penRef = 0;
        hpen_old = 0;
    } else {
        hpen_old = d->hpen;
    }
    if (cacheIt) {
        if (d->penStyle == Qt::NoPen) {
            d->hpen = stock_nullPen;
            d->penRef = stock_ptr;
            goto set;
        }
        if (obtain_pen(&d->penRef, &d->hpen, d->pColor))
            goto set;
    }

    int s;

    switch (d->penStyle) {
    case Qt::NoPen:             s = PS_NULL;        break;
    case Qt::SolidLine:         s = PS_SOLID;        break;
    case Qt::DashLine:          s = PS_DASH;        break;
#ifndef Q_OS_TEMP
    case Qt::DotLine:           s = PS_DOT;         break;
    case Qt::DashDotLine:            s = PS_DASHDOT;         break;
    case Qt::DashDotDotLine:    s = PS_DASHDOTDOT;         break;
#endif
        default:
            s = PS_SOLID;
            qWarning("QPainter::updatePen: Invalid pen style");
    }
#ifndef Q_OS_TEMP
    if (((pen.width() != 0) || pen.width() > 1) &&
         (qt_winver & QSysInfo::WV_NT_based || d->penStyle == Qt::SolidLine)) {
        LOGBRUSH lb;
        lb.lbStyle = 0;
        lb.lbColor = d->pColor;
        lb.lbHatch = 0;
        int pst = PS_GEOMETRIC | s;
        switch (pen.capStyle()) {
            case Qt::SquareCap:
                pst |= PS_ENDCAP_SQUARE;
                break;
            case Qt::RoundCap:
                pst |= PS_ENDCAP_ROUND;
                break;
            case Qt::FlatCap:
                pst |= PS_ENDCAP_FLAT;
                break;
        }
        switch (pen.joinStyle()) {
            case Qt::BevelJoin:
                pst |= PS_JOIN_BEVEL;
                break;
            case Qt::RoundJoin:
                pst |= PS_JOIN_ROUND;
                break;
            case Qt::MiterJoin:
                pst |= PS_JOIN_MITER;
                break;
        }
        d->hpen = ExtCreatePen(pst, d->pWidth, &lb, 0, 0);
    }
    else
#endif
    {
        d->hpen = CreatePen(s, qRound(pen.width()), d->pColor);
    }

set:
    SetTextColor(d->hdc, d->pColor);
    SelectObject(d->hdc, d->hpen);
    if (hpen_old)
        DeleteObject(hpen_old);
    return;
}


void QWin32PaintEngine::updateBrush(const QBrush &brush, const QPointF &bgOrigin)
{
#ifdef QT_DEBUG_DRAW
    static int counter = 0;
    printf(" - QWin32PaintEngine::updateBrush(), style=%d, color=%p, calls=%d\n",
           brush.style(), brush.color().rgba(), ++counter);
#endif
    d->brush = brush;
    d->brushStyle = brush.style();
    if (d->tryGdiplus()) {
        d->gdiplusEngine->updateBrush(brush, bgOrigin);
        return;
    }

#ifndef Q_OS_TEMP
    static short d1_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static short d2_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static short d3_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static short d4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static short d5_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static short d6_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static short d7_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static short *dense_patterns[]
        = { d1_pat, d2_pat, d3_pat, d4_pat, d5_pat, d6_pat, d7_pat };
#else
    static DWORD d1_pat[]     = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static DWORD d2_pat[]     = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static DWORD d3_pat[]     = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static DWORD d4_pat[]     = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static DWORD d5_pat[]     = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static DWORD d6_pat[]     = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static DWORD d7_pat[]     = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static DWORD *dense_patterns[]
        = { d1_pat, d2_pat, d3_pat, d4_pat, d5_pat, d6_pat, d7_pat };

    static DWORD hor_pat[]    = { 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff };
    static DWORD ver_pat[]    = { 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef };
    static DWORD cross_pat[]  = { 0xef, 0xef, 0xef, 0x00, 0xef, 0xef, 0xef, 0xef };
    static DWORD bdiag_pat[]  = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };
    static DWORD fdiag_pat[]  = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
    static DWORD dcross_pat[] = { 0x7e, 0xbd, 0xdb, 0xe7, 0xe7, 0xdb, 0xbd, 0x7e };
    static DWORD *hatch_patterns[]
        = { hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };
#endif

    d->bColor           = COLOR_VALUE(brush.color());
    bool   cacheIt = d->brushStyle == Qt::NoBrush || d->brushStyle == Qt::SolidPattern;
    HBRUSH hbrush_old;

    if (d->brushRef) {
        release_brush(d->brushRef);
        d->brushRef = 0;
        hbrush_old = 0;
    } else {
        hbrush_old = d->hbrush;
    }
    if (cacheIt) {
        if (d->brushStyle == Qt::NoBrush) {
            d->hbrush = stock_nullBrush;
            d->brushRef = stock_ptr;
            SelectObject(d->hdc, d->hbrush);
            if (hbrush_old) {
                if (d->hbrushbm && !d->pixmapBrush)
                    DeleteObject(d->hbrushbm);
                DeleteObject(hbrush_old);
                d->hbrushbm = 0;
                d->pixmapBrush = d->nocolBrush = false;
            }
            return;
        }
        if (obtain_brush(&d->brushRef, &d->hbrush, d->bColor)) {
            SelectObject(d->hdc, d->hbrush);
            if (hbrush_old) {
                if (d->hbrushbm && !d->pixmapBrush)
                    DeleteObject(d->hbrushbm);
                DeleteObject(hbrush_old);
                d->hbrushbm = 0;
                d->pixmapBrush = d->nocolBrush = false;
            }
            return;
        }
    }

    HBITMAP hbrushbm_old    = d->hbrushbm;
    bool    pixmapBrush_old = d->pixmapBrush;

    d->pixmapBrush = d->nocolBrush = false;
    d->hbrushbm = 0;

    if (d->brushStyle == Qt::SolidPattern) {                        // create solid brush
        d->hbrush = CreateSolidBrush(d->bColor);
#ifndef Q_OS_TEMP
    } else if ((d->brushStyle >= Qt::Dense1Pattern && d->brushStyle <= Qt::Dense7Pattern) ||
                (d->brushStyle == Qt::CustomPattern)) {
        if (d->brushStyle == Qt::CustomPattern) {
            // The brush pixmap can never be a multi cell pixmap
            d->hbrushbm = brush.texture().hbm();
            d->pixmapBrush = true;
            d->nocolBrush = brush.texture().depth() == 1;
        } else {
            short *bm = dense_patterns[d->brushStyle - Qt::Dense1Pattern];
            d->hbrushbm = CreateBitmap(8, 8, 1, 1, bm);
            d->nocolBrush = true;
        }
        d->hbrush = CreatePatternBrush(d->hbrushbm);
        if (!d->pixmapBrush) // hbm is owned by the pixmap, and will be deleted by it later.
            DeleteObject(d->hbrushbm);
        d->hbrushbm = 0;
    } else {                                        // one of the hatch brushes
        int s;
        switch (d->brushStyle) {
            case Qt::HorPattern:
                s = HS_HORIZONTAL;
                break;
            case Qt::VerPattern:
                s = HS_VERTICAL;
                break;
            case Qt::CrossPattern:
                s = HS_CROSS;
                break;
            case Qt::BDiagPattern:
                s = HS_BDIAGONAL;
                break;
            case Qt::FDiagPattern:
                s = HS_FDIAGONAL;
                break;
            case Qt::DiagCrossPattern:
                s = HS_DIAGCROSS;
                break;
            default:
                s = HS_HORIZONTAL;
        }
        d->hbrush = CreateHatchBrush(s, d->bColor);
    }
#else
    } else {
        if (d->brushStyle == Qt::CustomPattern) {
            // The brush pixmap can never be a multi cell pixmap
            d->hbrushbm = brush.pixmap()->hbm();
            d->pixmapBrush = true;
            d->nocolBrush = brush.pixmap()->depth() == 1;
            d->hbrush = CreatePatternBrush(d->hbrushbm);
            DeleteObject(d->hbrushbm);
        } else {
            struct {
                BITMAPINFOHEADER bmi;
                COLORREF palette[2];
                DWORD bitmapData[8];

            } bitmapBrush;

            memset(&bitmapBrush, 0, sizeof(bitmapBrush));

            bitmapBrush.bmi.biSize     = sizeof(BITMAPINFOHEADER);
            bitmapBrush.bmi.biWidth    =
            bitmapBrush.bmi.biHeight   = 8;
            bitmapBrush.bmi.biPlanes   =
            bitmapBrush.bmi.biBitCount = 1;
            bitmapBrush.bmi.biClrUsed  = 0;
            QRgb *coltbl = (QRgb*)bitmapBrush.palette;
            coltbl[0] = d->bColor.rgba();
            coltbl[1] = Qt::color0.rgba();

            static DWORD *pattern = hatch_patterns[0]; // Qt::HorPattern

            if (d->brushStyle >= Qt::Dense1Pattern && d->brushStyle <= Qt::Dense7Pattern)
                pattern = dense_patterns[d->brushStyle - Qt::Dense1Pattern];
            else if (d->brushStyle >= Qt::HorPattern && d->brushStyle <= Qt::DiagCrossPattern)
                pattern = hatch_patterns[d->brushStyle - Qt::HorPattern];

            memcpy(bitmapBrush.bitmapData, pattern, 64);
            d->hbrush = CreateDIBPatternBrushPt(&bitmapBrush, DIB_RGB_COLORS);
        }
    }
#endif
    SetBrushOrgEx(d->hdc, qRound(bgOrigin.x()), qRound(bgOrigin.y()), 0);
    SelectObject(d->hdc, d->hbrush);

    if (hbrush_old) {
        if (hbrushbm_old && !pixmapBrush_old)
            DeleteObject(hbrushbm_old);        // delete last brush pixmap
        DeleteObject(hbrush_old);              // delete last brush
    }
}

void QWin32PaintEngine::updateBackground(Qt::BGMode mode, const QBrush &bgBrush)
{
    if (d->tryGdiplus()) {
        d->gdiplusEngine->updateBackground(mode, bgBrush);
        return;
    }
    Q_ASSERT(isActive());

    d->bgMode = mode;

    SetBkColor(d->hdc, COLOR_VALUE(bgBrush.color()));
    SetBkMode(d->hdc, mode == Qt::TransparentMode ? TRANSPARENT : OPAQUE);
}


void QWin32PaintEngine::updateMatrix(const QMatrix &mtx)
{
#ifdef QT_DEBUG_DRAW
    static int counter = 0;
    printf(" - QWin32PaintEngine::updateMatrix(), [%.1f %.1f %.1f %.1f %.1f %.1f], calls=%d\n",
           mtx.m11(), mtx.m12(), mtx.m21(), mtx.m22(), mtx.dx(), mtx.dy(), ++counter);
#endif

    if (d->tryGdiplus()) {
        d->gdiplusEngine->updateMatrix(mtx);
        return;
    }

    if (mtx.m12() != 0 || mtx.m21() != 0)
        d->txop = QPainterPrivate::TxRotShear;
    else if (mtx.m11() != 1 || mtx.m22() != 1)
        d->txop = QPainterPrivate::TxScale;
    else if (mtx.dx() != 0 || mtx.dy() != 0)
        d->txop = QPainterPrivate::TxTranslate;
    else
        d->txop = QPainterPrivate::TxNone;

    d->matrix = mtx;
    d->invMatrix = mtx.invert();

#ifndef QT_NO_NATIVE_XFORM
    d->setNativeMatrix(mtx);
#endif
}

static const int qt_clip_operations[] = {
    0,              // Qt::NoClip
    RGN_COPY,       // Qt::ReplaceClip
    RGN_AND,        // Qt::IntersectClip
    RGN_OR          // Qt::UniteClip
};

#ifdef QT_DEBUG_DRAW
static const char *qt_clip_operation_names[] = {
    "NoClip",
    "ReplaceClip",
    "IntersectClip",
    "UniteClip"
};
#endif

void QWin32PaintEngine::updateClipRegion(const QRegion &region, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    static int counter = 0;
    printf(" - QWin32PaintEngine::updateClipRegion, size=%d, [%d %d %d %d], %s, calls=%d\n",
           region.rects().size(),
           region.boundingRect().x(),
           region.boundingRect().y(),
           region.boundingRect().width(),
           region.boundingRect().height(),
           qt_clip_operation_names[op],
           ++counter);
#endif
    // Sanity check since we use it blindly below.
    Q_ASSERT(op >= 0 && op <= Qt::UniteClip);

    if (d->tryGdiplus()) {
        d->gdiplusEngine->updateClipRegion(region, op);
        return;
    }

    if (op == Qt::NoClip) {
        SelectClipRgn(d->hdc, 0);
        return;
    }

    QRegion rgn = region;

    // Setting an empty clip region on windows disables clipping, so we do the
    // nice hack of just setting a 1 pixel clip region far away to avoid anything
    // from being drawn.
    if (region.isEmpty())
        rgn = QRegion(-0x1000000, -0x1000000, 1, 1);
    ExtSelectClipRgn(d->hdc, rgn.handle(), qt_clip_operations[op]);
}

void QWin32PaintEngine::updateClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    QRectF bounds = path.boundingRect();
    printf(" - QWin32PaintEngine::updateClipPath, size=%d, [%.2f %.2f %.2f %.2f], %s\n",
           path.elementCount(), bounds.x(), bounds.y(), bounds.width(), bounds.height(),
           qt_clip_operation_names[op]);
#endif


#ifdef QT_NO_NATIVE_PATH
    QPaintEngine::updateClipPath(path, op);
    return;
#endif
    // Sanity check since we use it blindly below.
    Q_ASSERT(op >= 0 && op <= Qt::UniteClip);

    if (d->tryGdiplus()) {
        d->gdiplusEngine->updateClipPath(path, op);
        return;
    }

    if (op == Qt::NoClip) {
        SelectClipRgn(d->hdc, 0);
    } else if (path.isEmpty()) {
        updateClipRegion(QRegion(), op);
    } else {
        d->composeGdiPath(path
#ifndef QT_NO_NATIVE_XFORM
                          * d->matrix
#endif
                          );
        SelectClipPath(d->hdc, qt_clip_operations[op]);
    }
}

void QWin32PaintEngine::updateFont(const QFont &font)
{
    if (state->pfont)
        delete state->pfont;
#undef d
    state->pfont = new QFont(font.d, d_func()->pdev);
#define d d_func()
}

void QWin32PaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
#ifdef QT_DEBUG_DRAW
    printf(" - QWin32PaintEngine::updateRenderHints(), %p\n", hints);
#endif

    d->forceGdiplus |= (hints & QPainter::Antialiasing);
    if (d->tryGdiplus())
        d->gdiplusEngine->updateRenderHints(hints);
}

QPainter::RenderHints QWin32PaintEngine::supportedRenderHints() const
{
    if (qt_gdiplus_support)
        return QPainter::Antialiasing;
    return 0;
}

HDC QWin32PaintEngine::winHDC() const
{
    return d->hdc;
}

#define COLSCALE(x) uint( (x) * 0xff00 / 255.0 )

inline TRIVERTEX createVertex(int x, int y, const QColor &col)
{
    TRIVERTEX v;
    v.x = x;
    v.y = y;
    v.Alpha = COLSCALE(col.alpha());
    double premultiply = col.alpha() / 255.0;
    v.Red   = COLSCALE(col.red()*premultiply);
    v.Green = COLSCALE(col.green()*premultiply);
    v.Blue  = COLSCALE(col.blue()*premultiply);
    return v;
}

template <class T> void qt_swap(T &a, T &b) { T tmp=a; a=b; b=tmp; }

void QWin32PaintEnginePrivate::fillGradient(const QRect &rect)
{
#ifdef QT_NO_NATIVE_GRADIENT
    Q_ASSERT(!"QWin32PaintEnginePrivate::fillGradient()\n");
#endif
    Q_ASSERT(qGradientFill);

    QColor gcol1 = brush.color();
    QColor gcol2 = brush.gradientColor();

    bool useMemDC = !(rect.x() == 0
                     && rect.y() == 0
                     && rect.width() == pdev->width()
                     && rect.height() == pdev->height()
                     && gcol1.alpha() == 255
                     && gcol2.alpha() == 255);
    HDC memdc = hdc;
    HBITMAP bitmap;
    if (useMemDC) {
        memdc = CreateCompatibleDC(hdc);
        bitmap = CreateCompatibleBitmap(hdc, rect.width(), rect.height());
        SelectObject(memdc, bitmap);
    }

    Q_ASSERT(brush.style() == Qt::LinearGradientPattern);

    QPointF gstart = brush.gradientStart();
    QPointF gstop  = brush.gradientStop();

    gstart -= rect.topLeft();
    gstop -= rect.topLeft();

    int dx = qRound(gstop.x() - gstart.x());
    int dy = qRound(gstop.y() - gstart.y());

    int rw = rect.width();
    int rh = rect.height();

    if (qAbs(dx) > qAbs(dy)) { // Fill horizontally
        // Make sure we fill left to right.
        if (gstop.x() < gstart.x()) {
            qt_swap(gcol1, gcol2);
            qt_swap(gstart, gstop);
        }
        // Find the location where the lines covering the gradient intersect
        // the lines making up the top and bottom of the target rectangle.
        // Note: This might be outside the target rect, but that is ok.
        int xtop1, xtop2, xbot1, xbot2;
        if (dy == 0) {
            xtop1 = xbot1 = qRound(gstart.x());
            xtop2 = xbot2 = qRound(gstop.x());
        } else {
            double gamma = double(dx) / double(-dy);
            xtop1 = qRound((-gstart.y() + gamma * gstart.x() ) / gamma);
            xtop2 = qRound((-gstop.y()  + gamma * gstop.x()  ) / gamma);
            xbot1 = qRound((rh - gstart.y() + gamma * gstart.x() ) / gamma);
            xbot2 = qRound((rh - gstop.y()  + gamma * gstop.x()  ) / gamma);
            Q_ASSERT(xtop2 > xtop1);
        }

        // Fill the area to the left of the gradient
        TRIVERTEX polygon[4];
        int polyCount = 0;
	if (xtop1 > 0)
            polygon[polyCount++] = createVertex(0, 0, gcol1);
        polygon[polyCount++] = createVertex(xtop1+1, 0, gcol1);
        polygon[polyCount++] = createVertex(xbot1+1, rh, gcol1);
        if (xbot1 > 0)
            polygon[polyCount++] = createVertex(0, rh, gcol1);
        GRADIENT_TRIANGLE gtr[] = { { 0, 1, 2 }, { 2, 3, 0 } };
        int gtrCount = polyCount == 4 ? 2 : 1;
        qGradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);

        // Fill the area to the right of the gradient
        polyCount = 0;
	polygon[polyCount++] = createVertex(xtop2-1, 0, gcol2);
	if (xtop2 < rw)
	    polygon[polyCount++] = createVertex(rw, 0, gcol2);
	if (xbot2 < rw)
	    polygon[polyCount++] = createVertex(rw, rh, gcol2);
	polygon[polyCount++] = createVertex(xbot2-1, rh, gcol2);
        gtrCount = polyCount == 4 ? 2 : 1;
        qGradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);

        polygon[0] = createVertex(xtop1, 0, gcol1);
        polygon[1] = createVertex(xbot1, rh, gcol1);
        polygon[2] = createVertex(xbot2, rh, gcol2);
        polygon[3] = createVertex(xtop2, 0, gcol2);
        qGradientFill(memdc, polygon, 4, gtr, 2, GRADIENT_FILL_TRIANGLE);

    } else {
        // Fill Verticallty
        // Code below is a conceptually equal to the one above except that all
        // coords are swapped x <-> y.
        // Make sure we fill top to bottom...
        if (gstop.y() < gstart.y()) {
            qt_swap(gstart, gstop);
            qt_swap(gcol1, gcol2);
        }
        int yleft1, yleft2, yright1, yright2;
        if (dx == 0) {
            yleft1 = yright1 = qRound(gstart.y());
            yleft2 = yright2 = qRound(gstop.y());
        } else {
            double gamma = double(dy) / double(-dx);
            yleft1 = qRound((-gstart.x() + gamma * gstart.y()) / gamma);
            yleft2 = qRound((-gstop.x() + gamma * gstop.y()) / gamma);
            yright1 = qRound((rw - gstart.x() + gamma*gstart.y()) / gamma);
            yright2 = qRound((rw - gstop.x() + gamma*gstop.y()) / gamma);
            Q_ASSERT(yleft2 > yleft1);
        }

        TRIVERTEX polygon[4];
        int polyCount = 0;
        polygon[polyCount++] = createVertex(0, yleft1+1, gcol1);
	if (yleft1 > 0)
	    polygon[polyCount++] = createVertex(0, 0, gcol1);
	if (yright1 > 0)
	    polygon[polyCount++] = createVertex(rw, 0, gcol1);
	polygon[polyCount++] = createVertex(rw, yright1+1, gcol1);
        GRADIENT_TRIANGLE gtr[] = { { 0, 1, 2 }, { 2, 3, 0 } };
        int gtrCount = polyCount == 4 ? 2 : 1;
        qGradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);

        polyCount = 0;
	polygon[polyCount++] = createVertex(0, yleft2-1, gcol2);
	if (yleft2 < rh)
	    polygon[polyCount++] = createVertex(0, rh, gcol2);
	if (yright2 < rh)
	    polygon[polyCount++] = createVertex(rw, rh, gcol2);
	polygon[polyCount++] = createVertex(rw, yright2-1, gcol2);
        gtrCount = polyCount == 4 ? 2 : 1;
        qGradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);

        polygon[0] = createVertex(0, yleft1, gcol1);
        polygon[1] = createVertex(rw, yright1, gcol1);
        polygon[2] = createVertex(rw, yright2, gcol2);
        polygon[3] = createVertex(0, yleft2, gcol2);
        qGradientFill(memdc, polygon, 4, gtr, 2, GRADIENT_FILL_TRIANGLE);
    }

    if (useMemDC) {
        if (gcol1.alpha() != 255 || gcol2.alpha() != 255) {
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
            qAlphaBlend(hdc, rect.x(), rect.y(), rw, rh, memdc, 0, 0, rw, rh, bf);
        } else {
            BitBlt(hdc, rect.x(), rect.y(), rw, rh, memdc, 0, 0, SRCCOPY);
        }

        DeleteObject(bitmap);
        DeleteDC(memdc);
    }
}

void QWin32PaintEnginePrivate::fillAlpha(const QRect &r, const QColor &color)
{
#ifdef QT_NO_NATIVE_ALPHA
    Q_ASSERT(!"QWin32PaintEnginePrivate::fillAlpha()\n");
#endif
    Q_ASSERT(qAlphaBlend);

    HDC memdc = CreateCompatibleDC(hdc);
    HBITMAP bitmap = CreateCompatibleBitmap(hdc, r.width(), r.height());
    SelectObject(memdc, bitmap);
    SelectObject(memdc, CreateSolidBrush(RGB(color.red(), color.green(), color.blue())));
    SelectObject(memdc, stock_nullPen);

    Rectangle(memdc, 0, 0, r.width() + 1, r.height() + 1);

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, color.alpha(), 0 };
    if (!qAlphaBlend(hdc, r.x(), r.y(), r.width(), r.height(), memdc, 0, 0, r.width(), r.height(), bf))
        qErrnoWarning("QWin32PaintEngine::fillAlpha: AlphaBlend failed");

    DeleteObject(SelectObject(memdc, stock_nullBrush));
    DeleteObject(bitmap);
    DeleteDC(memdc);
}

void QWin32PaintEnginePrivate::composeGdiPath(const QPainterPath &path)
{
#ifdef QT_NO_NATIVE_PATH
    Q_ASSERT(!"QWin32PaintEnginePrivate::composeGdiPath()\n");
#endif
    if (!BeginPath(hdc))
        qErrnoWarning("QWin32PaintEngine::drawPath: BeginPath failed");

    // Drawing the subpaths
    int start = -1;
    for (int i=0; i<path.elementCount(); ++i) {
        const QPainterPath::Element &elm = path.elementAt(i);
        switch (elm.type) {
        case QPainterPath::MoveToElement:
            if (start >= 0
                && path.elementAt(start).x == path.elementAt(i-1).x
                && path.elementAt(start).y == path.elementAt(i-1).y)
                CloseFigure(hdc);
            start = i;
            MoveToEx(hdc, qRound(elm.x), qRound(elm.y), 0);
            break;
        case QPainterPath::LineToElement:
            LineTo(hdc, qRound(elm.x), qRound(elm.y));
            break;
        case QPainterPath::CurveToElement: {
            POINT pts[3] = {
                { qRound(elm.x), qRound(elm.y) },
                { qRound(path.elementAt(i+1).x), qRound(path.elementAt(i+1).y) },
                { qRound(path.elementAt(i+2).x), qRound(path.elementAt(i+2).y) }
            };
            i+=2;
            PolyBezierTo(hdc, pts, 3);
            break;
        }
        default:
            qFatal("QWin32PaintEngine::drawPath: Unhandled type: %d", elm.type);
        }
    }

    if (start >= 0
        && path.elementAt(start).x == path.elementAt(path.elementCount()-1).x
        && path.elementAt(start).y == path.elementAt(path.elementCount()-1).y)
        CloseFigure(hdc);

    if (!EndPath(hdc))
        qErrnoWarning("QWin32PaintEngine::drawPath: EndPath failed");
}


void QWin32PaintEnginePrivate::beginGdiplus()
{
    if (!qt_resolved_gdiplus)
        qt_resolve_gdiplus();

    if (!qt_gdiplus_support)
        return;

#ifdef QT_DEBUG_DRAW
    printf(" - QWin32PaintEnginePrivate::beginGdiplus()\n");
#endif

    ExtSelectClipRgn(hdc, 0, RGN_COPY);

    q->gccaps |= QPaintEngine::CoordTransform
                 | QPaintEngine::PenWidthTransform
                 | QPaintEngine::PixmapTransform
                 | QPaintEngine::ClipTransform;

    Q_ASSERT(!gdiplusEngine);
    gdiplusEngine = new QGdiplusPaintEngine();
    gdiplusEngine->begin(pdev);
    gdiplusEngine->state = q->state;
    gdiplusInUse = true;

    if (q->painter()->hasClipping()) {
        q->state->tmpClipOp = Qt::ReplaceClip;
        // Should be clip path
        q->state->tmpClipRegion = q->painter()->clipRegion();
    }

    q->setDirty(QPaintEngine::AllDirty);

    q->updateState(q->state);
}

void QWin32PaintEnginePrivate::setNativeMatrix(const QMatrix &mtx)
{
    QT_WA( {
        XFORM m;
        if (d->txop > QPainterPrivate::TxNone) {
            m.eM11 = mtx.m11();
            m.eM12 = mtx.m12();
            m.eM21 = mtx.m21();
            m.eM22 = mtx.m22();
            m.eDx  = mtx.dx();
            m.eDy  = mtx.dy();
            if (!qSetGraphicsMode(d->hdc, GM_ADVANCED))
                qErrnoWarning("QWin32PaintEngine::setNativeMatrix(), SetGraphicsMode failed");
            if (!SetWorldTransform(d->hdc, &m))
                qErrnoWarning("QWin32PaintEngine::setNativeMatrix(), SetWorldTransformation failed");
            d->advancedMode = true;
        } else {
            m.eM11 = m.eM22 = (float)1.0;
            m.eM12 = m.eM21 = m.eDx = m.eDy = (float)0.0;
            if (!qSetGraphicsMode(d->hdc, GM_ADVANCED))
                qErrnoWarning("QWin32PaintEngine::setNativeMatrix(), SetGraphicsMode failed");
            qModifyWorldTransform(d->hdc, &m, MWT_IDENTITY);
            if (!qSetGraphicsMode(d->hdc, GM_COMPATIBLE))
                qErrnoWarning("QWin32PaintEngine::setNativeMatrix(), SetGraphicsMode failed");
            d->advancedMode = false;
        }
    }, {
        // ### How about 9x??
    } );
}


static QPaintEngine::PaintEngineFeatures qt_decide_paintengine_features()
{
    if (!qt_resolved_gdiplus)
        qt_resolve_gdiplus();

    QPaintEngine::PaintEngineFeatures commonFeatures =
        QPaintEngine::UsesFontEngine
        | QPaintEngine::AlphaPixmap
        | QPaintEngine::PixmapScale
#ifndef QT_NO_NATIVE_XFORM
        | QPaintEngine::CoordTransform
        | QPaintEngine::PenWidthTransform
        | QPaintEngine::PixmapTransform
        | QPaintEngine::ClipTransform
#endif

#ifndef QT_NO_NATIVE_PATH
        | QPaintEngine::PainterPaths
#endif

#ifndef QT_NO_NATIVE_GRADIENT
        | QPaintEngine::LinearGradients
#endif
        ;

    int shadeCaps = GetDeviceCaps(qt_display_dc(), SHADEBLENDCAPS);
#ifndef QT_NO_NATIVE_ALPHA
    if ((shadeCaps & SB_CONST_ALPHA) || qt_gdiplus_support)
        commonFeatures |= QPaintEngine::AlphaFill;
    if (qt_gdiplus_support)
        commonFeatures |= QPaintEngine::AlphaFill | QPaintEngine::AlphaStroke;
#endif

    if (qt_gdiplus_support)
        commonFeatures |= QPaintEngine::FillAntialiasing | QPaintEngine::LineAntialiasing;

    return commonFeatures;
}


/*******************************************************************************
 *
 * And thus begindeth the GDI+ paint engine
 *
 ******************************************************************************/
extern "C" {
typedef int (WINAPI *PtrGdiplusStartup)(Q_UINT64 **, const QtGpStartupInput *, void *);
typedef void (WINAPI *PtrGdiplusShutdown)(Q_UINT64 *);

typedef int (__stdcall *PtrGdipGetDC)(QtGpGraphics *, HDC *hdc);
typedef int (__stdcall *PtrGdipReleaseDC)(QtGpGraphics *, HDC *hdc);
typedef int (__stdcall *PtrGdipCreateFromHDC) (HDC hdc, QtGpGraphics **);
typedef int (__stdcall *PtrGdipDeleteGraphics) (QtGpGraphics *);
typedef int (__stdcall *PtrGdipSetTransform) (QtGpGraphics *, QtGpMatrix *);
typedef int (__stdcall *PtrGdipSetClipRegion) (QtGpGraphics *, QtGpRegion *, int);
typedef int (__stdcall *PtrGdipSetClipPath) (QtGpGraphics *, QtGpPath *, int);
typedef int (__stdcall *PtrGdipResetClip) (QtGpGraphics *);
typedef int (__stdcall *PtrGdipSetSmoothingMode)(QtGpGraphics *, int);
typedef int (__stdcall *PtrGdipSetPixelOffsetMode)(QtGpGraphics *, int);
typedef int (__stdcall *PtrGdipFillEllipse) (QtGpGraphics *, QtGpBrush *,
                                             float x, float y, float w, float h);
typedef int (__stdcall *PtrGdipFillRectangle) (QtGpGraphics *, QtGpBrush *,
                                               float x, float y, float w, float h);
typedef int (__stdcall *PtrGdipFillPath) (QtGpGraphics *, QtGpBrush *, QtGpPath *);
typedef int (__stdcall *PtrGdipFillPolygon) (QtGpGraphics *, QtGpBrush *, const QPointF *, int, int);
typedef int (__stdcall *PtrGdipDrawRectangle) (QtGpGraphics *, QtGpPen *,
                                               float x, float y, float w, float h);
typedef int (__stdcall *PtrGdipDrawEllipse) (QtGpGraphics *, QtGpPen *,
                                             float x, float y, float w, float h);
typedef int (__stdcall *PtrGdipDrawImageRectRect) (QtGpGraphics *, QtGpImage *,
                                                   float x, float y, float w, float h,
                                                   float sx, float sy, float sw, float sh,
                                                   int srcUnit, const void *attr, void *callback,
                                                   void *callBackData);
typedef int (__stdcall *PtrGdipDrawLine) (QtGpGraphics *, QtGpPen *,
                                          float x1, float y1, float x2, float y2);
typedef int (__stdcall *PtrGdipDrawPath) (QtGpGraphics *, QtGpPen *, QtGpPath *);
typedef int (__stdcall *PtrGdipDrawPolygon) (QtGpGraphics *, QtGpPen *, const QPointF *, int);
typedef int (__stdcall *PtrGdipDrawDriverString) (QtGpGraphics *, Q_UINT16 *, int, const QtGpFont *,
                                                  const QtGpBrush *, const QPointF *, int,
                                                  const QtGpMatrix *);

typedef int (__stdcall *PtrGdipCreateMatrix2) (float, float, float, float, float, float, QtGpMatrix **);
typedef int (__stdcall *PtrGdipDeleteMatrix) (QtGpMatrix *);

typedef int (__stdcall *PtrGdipCreateRegionHrgn) (HRGN hRgn, QtGpRegion **);
typedef int (__stdcall *PtrGdipDeleteRegion) (QtGpRegion *);

typedef int (__stdcall *PtrGdipDeleteBrush) (QtGpBrush *);

typedef int (__stdcall *PtrGdipCreateSolidFill) (Q_UINT32 argb, QtGpBrush **);
typedef int (__stdcall *PtrGdipSetSolidFillColor) (QtGpSolidFill *, Q_UINT32 argb);
typedef int (__stdcall *PtrGdipCreateLineBrush) (QPointF *, QPointF *, Q_UINT32, Q_UINT32, uint, QtGpBrush **);

typedef int (__stdcall *PtrGdipCreatePen1) (Q_UINT32 argb, float width, int unit, QtGpPen **);
typedef int (__stdcall *PtrGdipDeletePen) (QtGpPen *);
typedef int (__stdcall *PtrGdipSetPenWidth) (QtGpPen *, float width);
typedef int (__stdcall *PtrGdipSetPenColor) (QtGpPen *, Q_UINT32 argb);
typedef int (__stdcall *PtrGdipSetPenDashStyle) (QtGpPen *, int dashStyle);

typedef int (__stdcall *PtrGdipCreatePath) (int fillMode, QtGpPath **path);
typedef int (__stdcall *PtrGdipDeletePath) (QtGpPath *path);
typedef int (__stdcall *PtrGdipAddPathLine) (QtGpPath *path,
                                             float x1, float y1, float x2, float y2);
typedef int (__stdcall *PtrGdipAddPathArc) (QtGpPath *path,
                                            float x, float y, float w, float h,
                                            float startAngle, float sweepAngle);
typedef int (__stdcall *PtrGdipAddPathBezier)(QtGpPath *path,
                                              float, float, float, float,
                                              float, float, float, float);
typedef int (__stdcall *PtrGdipClosePathFigure) (QtGpPath *path);
typedef int (__stdcall *PtrGdipSetPathFillMode) (QtGpPath *path, int fillmode);
typedef int (__stdcall *PtrGdipStartPathFigure) (QtGpPath *path);

typedef int (__stdcall *PtrGdipCreateBitmapFromHBITMAP)(HBITMAP, HPALETTE, QtGpBitmap **);
typedef int (__stdcall *PtrGdipCreateBitmapFromScan0)(int w, int h, int stride, int format,
                                                      BYTE *scan0, QtGpBitmap **);
typedef int (__stdcall *PtrGdipGetImageGraphicsContext)(QtGpImage *, QtGpGraphics **);
typedef int (__stdcall *PtrGdipGetImageWidth)(QtGpImage *, uint *);
typedef int (__stdcall *PtrGdipGetImageHeight)(QtGpImage *, uint *);
typedef int (__stdcall *PtrGdipDisposeImage)(QtGpImage *);

typedef int (__stdcall *PtrGdipCreateFontFromDC)(HDC hdc, QtGpFont **);
typedef int (__stdcall *PtrGdipCreateFontFromLogfontW) (HDC, const LOGFONTW *, QtGpFont **);
typedef int (__stdcall *PtrGdipDeleteFont)(QtGpFont *);
}

static PtrGdiplusStartup GdiplusStartup = 0;                 // GdiplusStartup
static PtrGdiplusShutdown GdiplusShutdown = 0;               // GdiplusStartup

static PtrGdipCreateFromHDC GdipCreateFromHDC = 0;           // Graphics::Graphics(hdc)
static PtrGdipDeleteGraphics GdipDeleteGraphics = 0;         // Graphics::~Graphics()
static PtrGdipGetDC GdipGetDC = 0;                           // Graphics::GetDC(hdc)
static PtrGdipReleaseDC GdipReleaseDC = 0;                   // Graphics::ReleaseDC()
static PtrGdipDrawEllipse GdipDrawEllipse = 0;               // Graphics::DrawEllipse(pen, x, y, w, h)
static PtrGdipDrawLine GdipDrawLine = 0;                     // Graphics::DrawLine(pen, x1, y1, x2, y2)
static PtrGdipDrawPath GdipDrawPath = 0;                     // Graphics::DrawPath(pen, path);
static PtrGdipDrawPolygon GdipDrawPolygon = 0;
static PtrGdipDrawRectangle GdipDrawRectangle = 0;           // Graphics::DrawRectangle(brush,x,y,w,h)
static PtrGdipDrawImageRectRect GdipDrawImageRectRect = 0;   // Graphics::DrawImage(image, r, sr);
static PtrGdipFillEllipse GdipFillEllipse = 0;               // Graphics::FillEllipse(brush, x, y, w, h)
static PtrGdipFillPath GdipFillPath = 0;                     // Graphics::FillPath(brush, path)
static PtrGdipFillPolygon GdipFillPolygon = 0;
static PtrGdipFillRectangle GdipFillRectangle = 0;           // Graphics::FillRectangle(brush,x,y,w,h)
static PtrGdipResetClip GdipResetClip = 0;                   // Graphics::ResetClip()
static PtrGdipSetClipRegion GdipSetClipRegion = 0;           // Graphics::SetClipRegion(region)
static PtrGdipSetClipPath GdipSetClipPath = 0;               // Graphics::SetClipPath(path)
static PtrGdipSetSmoothingMode GdipSetSmoothingMode = 0;     // Graphics::SetSmoothingMode(mode)
static PtrGdipSetPixelOffsetMode GdipSetPixelOffsetMode = 0; // Graphics::SetPixelOffsetMode(mode)
static PtrGdipSetTransform GdipSetTransform = 0;             // Graphics::SetTransform(matrix)
static PtrGdipDrawDriverString GdipDrawDriverString = 0;     // Graphics::DrawDriverString(...)

static PtrGdipCreateMatrix2 GdipCreateMatrix2 = 0;           // Matrix::Matrix(a, b, c, d, dx, dy)
static PtrGdipDeleteMatrix GdipDeleteMatrix = 0;             // Matrix::~Matrix()

static PtrGdipCreateRegionHrgn GdipCreateRegionHrgn = 0;     // Region::Region(hRgn)
static PtrGdipDeleteRegion GdipDeleteRegion = 0;             // Region::~Region()

static PtrGdipDeleteBrush GdipDeleteBrush = 0;               // Brush::~Brush()

static PtrGdipCreateSolidFill GdipCreateSolidFill = 0;       // SolidBrush::SolidBrush(argb)
static PtrGdipSetSolidFillColor GdipSetSolidFillColor = 0;   // SolidBrush::SetColor(argb)
static PtrGdipCreateLineBrush GdipCreateLineBrush = 0;       // LinearGradientBrush...


static PtrGdipCreatePen1 GdipCreatePen1 = 0;                 // Pen::Pen(color, width)
static PtrGdipDeletePen GdipDeletePen = 0;                   // Pen::~Pen()
static PtrGdipSetPenWidth GdipSetPenWidth = 0;               // Pen::SetWidth(width)
static PtrGdipSetPenColor GdipSetPenColor = 0;               // Pen::SetColor(argb)
static PtrGdipSetPenDashStyle GdipSetPenDashStyle = 0;       // Pen::SetDashStyle(style)

static PtrGdipCreatePath GdipCreatePath = 0;                 // Path::Path(fillMode)
static PtrGdipDeletePath GdipDeletePath = 0;                 // Path::~Path()
static PtrGdipAddPathLine GdipAddPathLine = 0;               // Path::AddLine(x1, y1, x2, y2)
static PtrGdipAddPathArc GdipAddPathArc = 0;                 // Path::AddArc(x, y, w, h, start, sweep)
static PtrGdipAddPathBezier GdipAddPathBezier = 0;           // Path::AddPathBezier(x1, y1, ... x4, y4)
static PtrGdipClosePathFigure GdipClosePathFigure = 0;       // Path::CloseFigure()
static PtrGdipSetPathFillMode GdipSetPathFillMode = 0;       // Path::SetFillMode(mode);
static PtrGdipStartPathFigure GdipStartPathFigure = 0;       // Path::StartFigure(mode);

static PtrGdipCreateBitmapFromHBITMAP GdipCreateBitmapFromHBITMAP = 0;  // Bitmap::Bitmap(hbm, hpalette)
static PtrGdipCreateBitmapFromScan0 GdipCreateBitmapFromScan0 = 0;      // Bitmap::Bitmap(w, h .. bits)
static PtrGdipGetImageGraphicsContext GdipGetImageGraphicsContext = 0;  // Graphics Image.getContext();
static PtrGdipGetImageWidth GdipGetImageWidth = 0;
static PtrGdipGetImageHeight GdipGetImageHeight = 0;
static PtrGdipDisposeImage GdipDisposeImage = 0;             // Image::~Image()

static PtrGdipCreateFontFromDC GdipCreateFontFromDC = 0;     // Font::Font(HDC);
static PtrGdipCreateFontFromLogfontW GdipCreateFontFromLogfontW = 0;    // Font::Font(HDC, HFONT)
static PtrGdipDeleteFont GdipDeleteFont = 0;                 // Font::~Font();

static void qt_resolve_gdiplus()
{
    if (qt_resolved_gdiplus)
        return;

    if (qgetenv("QT_FORCE_GDI")) {
        qt_resolved_gdiplus = true;
        return;
    }

    QLibrary lib("gdiplus");
    qt_resolved_gdiplus = true;

    lib.load();
    if (!lib.isLoaded())
        return;

    // Global functions
    GdiplusStartup           = (PtrGdiplusStartup)     lib.resolve("GdiplusStartup");
    GdiplusShutdown          = (PtrGdiplusShutdown)    lib.resolve("GdiplusShutdown");

    // Graphics functions
    GdipCreateFromHDC            = (PtrGdipCreateFromHDC)      lib.resolve("GdipCreateFromHDC");
    GdipDeleteGraphics           = (PtrGdipDeleteGraphics)     lib.resolve("GdipDeleteGraphics");
    GdipGetDC                    = (PtrGdipGetDC)              lib.resolve("GdipGetDC");
    GdipReleaseDC                = (PtrGdipReleaseDC)          lib.resolve("GdipReleaseDC");
    GdipSetTransform             = (PtrGdipSetTransform)       lib.resolve("GdipSetWorldTransform");
    GdipSetClipRegion            = (PtrGdipSetClipRegion)      lib.resolve("GdipSetClipRegion");
    GdipSetClipPath              = (PtrGdipSetClipPath)        lib.resolve("GdipSetClipPath");
    GdipResetClip                = (PtrGdipResetClip)          lib.resolve("GdipResetClip");
    GdipSetSmoothingMode         = (PtrGdipSetSmoothingMode)   lib.resolve("GdipSetSmoothingMode");
    GdipSetPixelOffsetMode       = (PtrGdipSetPixelOffsetMode) lib.resolve("GdipSetPixelOffsetMode");
    GdipFillEllipse              = (PtrGdipFillEllipse)        lib.resolve("GdipFillEllipse");
    GdipFillPath                 = (PtrGdipFillPath)           lib.resolve("GdipFillPath");
    GdipFillPolygon              = (PtrGdipFillPolygon)        lib.resolve("GdipFillPolygon");
    GdipFillRectangle            = (PtrGdipFillRectangle)      lib.resolve("GdipFillRectangle");
    GdipDrawEllipse              = (PtrGdipDrawEllipse)        lib.resolve("GdipDrawEllipse");
    GdipDrawLine                 = (PtrGdipDrawLine)           lib.resolve("GdipDrawLine");
    GdipDrawPath                 = (PtrGdipDrawPath)           lib.resolve("GdipDrawPath");
    GdipDrawPolygon              = (PtrGdipDrawPolygon)        lib.resolve("GdipDrawPolygon");
    GdipDrawRectangle            = (PtrGdipDrawRectangle)      lib.resolve("GdipDrawRectangle");
    GdipDrawImageRectRect        = (PtrGdipDrawImageRectRect)  lib.resolve("GdipDrawImageRectRect");
    GdipDrawDriverString         = (PtrGdipDrawDriverString)   lib.resolve("GdipDrawDriverString");

    // Matrix functions
    GdipCreateMatrix2            = (PtrGdipCreateMatrix2)      lib.resolve("GdipCreateMatrix2");
    GdipDeleteMatrix             = (PtrGdipDeleteMatrix)       lib.resolve("GdipDeleteMatrix");

    // Region functions
    GdipCreateRegionHrgn         = (PtrGdipCreateRegionHrgn)   lib.resolve("GdipCreateRegionHrgn");
    GdipDeleteRegion             = (PtrGdipDeleteRegion)       lib.resolve("GdipDeleteRegion");

    // Brush functions
    GdipDeleteBrush              = (PtrGdipDeleteBrush)        lib.resolve("GdipDeleteBrush");
    GdipCreateSolidFill          = (PtrGdipCreateSolidFill)    lib.resolve("GdipCreateSolidFill");
    GdipSetSolidFillColor        = (PtrGdipSetSolidFillColor)  lib.resolve("GdipSetSolidFillColor");
    GdipCreateLineBrush          = (PtrGdipCreateLineBrush)    lib.resolve("GdipCreateLineBrush");

    // Pen functions
    GdipCreatePen1               = (PtrGdipCreatePen1)         lib.resolve("GdipCreatePen1");
    GdipDeletePen                = (PtrGdipDeletePen)          lib.resolve("GdipDeletePen");
    GdipSetPenWidth              = (PtrGdipSetPenWidth)        lib.resolve("GdipSetPenWidth");
    GdipSetPenColor              = (PtrGdipSetPenColor)        lib.resolve("GdipSetPenColor");
    GdipSetPenDashStyle          = (PtrGdipSetPenDashStyle)    lib.resolve("GdipSetPenDashStyle");

    // Path functions
    GdipCreatePath               = (PtrGdipCreatePath)         lib.resolve("GdipCreatePath");
    GdipDeletePath               = (PtrGdipDeletePath)         lib.resolve("GdipDeletePath");
    GdipAddPathLine              = (PtrGdipAddPathLine)        lib.resolve("GdipAddPathLine");
    GdipAddPathArc               = (PtrGdipAddPathArc)         lib.resolve("GdipAddPathArc");
    GdipAddPathBezier            = (PtrGdipAddPathBezier)      lib.resolve("GdipAddPathBezier");
    GdipClosePathFigure          = (PtrGdipClosePathFigure)    lib.resolve("GdipClosePathFigure");
    GdipSetPathFillMode          = (PtrGdipSetPathFillMode)    lib.resolve("GdipSetPathFillMode");
    GdipStartPathFigure          = (PtrGdipStartPathFigure)    lib.resolve("GdipStartPathFigure");

    // Bitmap functions
    GdipCreateBitmapFromHBITMAP
        = (PtrGdipCreateBitmapFromHBITMAP) lib.resolve("GdipCreateBitmapFromHBITMAP");
    GdipCreateBitmapFromScan0
        = (PtrGdipCreateBitmapFromScan0) lib.resolve("GdipCreateBitmapFromScan0");
    GdipGetImageGraphicsContext =
        (PtrGdipGetImageGraphicsContext) lib.resolve("GdipGetImageGraphicsContext");
    GdipGetImageWidth           = (PtrGdipGetImageWidth)       lib.resolve("GdipGetImageWidth");
    GdipGetImageHeight          = (PtrGdipGetImageHeight)      lib.resolve("GdipGetImageHeight");
    GdipDisposeImage            = (PtrGdipDisposeImage)        lib.resolve("GdipDisposeImage");

    // Font functions
    GdipCreateFontFromDC         = (PtrGdipCreateFontFromDC)   lib.resolve("GdipCreateFontFromDC");
    GdipCreateFontFromLogfontW   = (PtrGdipCreateFontFromLogfontW)
                                   lib.resolve("GdipCreateFontFromLogfontW");
    GdipDeleteFont               = (PtrGdipDeleteFont)         lib.resolve("GdipDeleteFont");

    Q_ASSERT(GdiplusStartup);
    Q_ASSERT(GdiplusShutdown);
    Q_ASSERT(GdipCreateFromHDC);
    Q_ASSERT(GdipDeleteGraphics);
    Q_ASSERT(GdipGetDC);
    Q_ASSERT(GdipReleaseDC);
    Q_ASSERT(GdipSetTransform);
    Q_ASSERT(GdipSetClipRegion);
    Q_ASSERT(GdipSetClipPath);
    Q_ASSERT(GdipResetClip);
    Q_ASSERT(GdipSetSmoothingMode);
    Q_ASSERT(GdipSetPixelOffsetMode);
    Q_ASSERT(GdipFillEllipse);
    Q_ASSERT(GdipFillPath);
    Q_ASSERT(GdipFillPolygon);
    Q_ASSERT(GdipFillRectangle);
    Q_ASSERT(GdipDrawEllipse);
    Q_ASSERT(GdipDrawImageRectRect);
    Q_ASSERT(GdipDrawLine);
    Q_ASSERT(GdipDrawPath);
    Q_ASSERT(GdipDrawPolygon) ;
    Q_ASSERT(GdipDrawRectangle);
    Q_ASSERT(GdipDrawDriverString);
    Q_ASSERT(GdipCreateMatrix2);
    Q_ASSERT(GdipDeleteMatrix);
    Q_ASSERT(GdipCreateRegionHrgn);
    Q_ASSERT(GdipDeleteRegion);
    Q_ASSERT(GdipDeleteBrush);
    Q_ASSERT(GdipCreateSolidFill);
    Q_ASSERT(GdipSetSolidFillColor);
    Q_ASSERT(GdipCreateLineBrush);
    Q_ASSERT(GdipCreatePen1);
    Q_ASSERT(GdipDeletePen);
    Q_ASSERT(GdipSetPenWidth);
    Q_ASSERT(GdipSetPenColor);
    Q_ASSERT(GdipSetPenDashStyle);
    Q_ASSERT(GdipCreatePath);
    Q_ASSERT(GdipDeletePath);
    Q_ASSERT(GdipAddPathLine);
    Q_ASSERT(GdipAddPathArc);
    Q_ASSERT(GdipAddPathBezier);
    Q_ASSERT(GdipClosePathFigure);
    Q_ASSERT(GdipSetPathFillMode);
    Q_ASSERT(GdipStartPathFigure);
    Q_ASSERT(GdipCreateBitmapFromHBITMAP);
    Q_ASSERT(GdipCreateBitmapFromScan0);
    Q_ASSERT(GdipGetImageGraphicsContext);
    Q_ASSERT(GdipGetImageWidth);
    Q_ASSERT(GdipGetImageHeight);
    Q_ASSERT(GdipDisposeImage);
    Q_ASSERT(GdipCreateFontFromDC);
    Q_ASSERT(GdipCreateFontFromLogfontW);
    Q_ASSERT(GdipDeleteFont);

    QGdiplusPaintEngine::initialize();
    qt_gdiplus_support = true;
}


// Based on enums in GdiPlusEnums.h
static const int qt_penstyle_map[] = {
    -1 ,      // Qt::NoPen
     0 ,      // Qt::SolidLine
     1 ,      // Qt::DashLine
     2 ,      // Qt::DotLine
     3 ,      // Qt::DashDotLine
     4        // Qt::DashDotDotLine
};

static const int qt_hatchstyle_map[] = {
     -1 ,     // Qt::NoBrush
     -1 ,     // Qt::SolidPattern
     17 ,     // Qt::Dense1Pattern, hatch 90
     15 ,     // Qt::Dense2Pattern, hatch 75
     13 ,     // Qt::Dense3Pattern, hatch 60
     12 ,     // Qt::Dense4Pattern, hatch 50
     10 ,     // Qt::Dense5Pattern, hatch 30
      8 ,      // Qt::Dense6Pattern, hatch 20
      6 ,      // Qt::Dense7Pattern, hatch 05
      0 ,      // Qt::HorPattern
      1 ,      // Qt::VerPattern
      4 ,      // Qt::CrossPattern
      3 ,      // Qt::BDiagPattern
      2 ,      // Qt::FDiagPattern
      5        // Qt::DiagCrossPattern
};

static QtGpBitmap *qt_convert_to_gdipbitmap(const QPixmap *pixmap, QImage *ref = 0);

enum QualityMode
{
    QualityModeInvalid   = -1,
    QualityModeDefault   = 0,
    QualityModeLow       = 1, // Best performance
    QualityModeHigh      = 2  // Best rendering quality
};

enum PixelOffsetMode {
    PixelOffsetModeInvalid     = QualityModeInvalid,
    PixelOffsetModeDefault     = QualityModeDefault,
    PixelOffsetModeHighSpeed   = QualityModeLow,
    PixelOffsetModeHighQuality = QualityModeHigh,
    PixelOffsetModeNone,    // No pixel offset
    PixelOffsetModeHalf     // Offset by -0.5, -0.5 for fast anti-alias perf
};

enum DriverStringOptions
{
    DriverStringOptionsCmapLookup             = 1,
    DriverStringOptionsVertical               = 2,
    DriverStringOptionsRealizedAdvance        = 4,
    DriverStringOptionsLimitSubpixel          = 8
};

QGdiplusPaintEngine::QGdiplusPaintEngine()
    : QPaintEngine(*(new QGdiplusPaintEnginePrivate))
{
}

QGdiplusPaintEngine::~QGdiplusPaintEngine()
{
}

/* Start painting for this device.
 */
bool QGdiplusPaintEngine::begin(QPaintDevice *pdev)
{
    d->pdev = pdev;
    // Verify the presence of an HDC
    d->hdc = pdev->getDC();
    if(!d->hdc)
        qDebug() << "QGdiplusPaintEngine::begin(), unsupported paint device..." << pdev->devType();

    GdipCreateFromHDC(d->hdc, &d->graphics);

    Q_ASSERT(d->graphics);

//     d->pen = new Pen(Color(0, 0, 0), 0);
    Q_ASSERT(!d->pen);
    GdipCreatePen1(0xff000000, 0, 0, &d->pen);
    Q_ASSERT(d->pen);

    setActive(true);

    return true;
}

bool QGdiplusPaintEngine::end()
{
//     delete d->graphics;
//     delete d->pen;
//     delete d->brush;
    GdipDeleteGraphics(d->graphics);
    GdipDeletePen(d->pen);
    GdipDeleteBrush(d->brush);

    if (d->bitmapDevice) {
        GdipDisposeImage(d->bitmapDevice);
        d->bitmapDevice = 0;
    }

    if (d->cachedSolidBrush != d->brush) {
//         delete d->cachedSolidBrush;
        GdipDeleteBrush(d->cachedSolidBrush);
    }

    d->graphics = 0;
    d->pen = 0;
    d->cachedSolidBrush = 0;
    d->brush = 0;

    d->pdev->releaseDC(d->hdc);
    return true;
}

void QGdiplusPaintEngine::updatePen(const QPen &pen)
{
#ifdef QT_DEBUG_DRAW
    printf(" - QGdiplusPaintEngine::updatePen(), style=%d, color=%p, width=%d\n",
           pen.style(), pen.color().rgba(), pen.width());
#endif
//     d->pen->SetWidth(ps->pen.width());
//     d->pen->SetColor(conv(ps->pen.color()));
    d->penColor = pen.color();

    int status;
    status = GdipSetPenWidth(d->pen, pen.widthF());
    Q_ASSERT(status == 0);
    status = GdipSetPenColor(d->pen, pen.color().rgba());
    Q_ASSERT(status == 0);

    Qt::PenStyle style = pen.style();
    if (style == Qt::NoPen) {
        d->usePen = false;
    } else {
        Q_ASSERT(style >= 0 && style <= 5);
        d->usePen = true;
//         d->pen->SetDashStyle(qt_penstyle_map[style]);
        GdipSetPenDashStyle(d->pen, qt_penstyle_map[style]);
    }
}

void QGdiplusPaintEngine::updateBrush(const QBrush &brush, const QPointF &)
{
    d->brushColor = brush.color();
    if (d->temporaryBrush) {
        d->temporaryBrush = false;
//         delete d->brush;
        GdipDeleteBrush(d->brush);
        d->brush = 0;
    }

    switch (brush.style()) {
    case Qt::NoBrush:
        d->brush = 0;
        break;
    case Qt::SolidPattern:
        if (!d->cachedSolidBrush) {
//             d->cachedSolidBrush = new SolidBrush(conv(brush.color()));
            GdipCreateSolidFill(brush.color().rgba(), (QtGpBrush**)(&d->cachedSolidBrush));
            d->brush = d->cachedSolidBrush;
        } else {
//             d->cachedSolidBrush->SetColor(conv(brush.color()));
            GdipSetSolidFillColor(d->cachedSolidBrush, brush.color().rgba());
            d->brush = d->cachedSolidBrush;
        }
        break;
    case Qt::LinearGradientPattern: {
        QPointF p1 = brush.gradientStart();
        QPointF p2 = brush.gradientStop();
        GdipCreateLineBrush(&p1, &p2, brush.color().rgb(), brush.gradientColor().rgb(), 0, &d->brush);
//         d->brush = new LinearGradientBrush(conv(b.gradientStart()), conv(b.gradientStop()),
//                                            conv(b.color()), conv(b.gradientColor()));
        d->temporaryBrush = true;
        break;
    }
    case Qt::CustomPattern: {
//         QPixmap *pm = brush.pixmap();
//         if (pm) {
//             Bitmap *bm = new Bitmap(pm->hbm(), (HPALETTE)0);
//             d->brush = new TextureBrush(bm, WrapModeTile);
//             d->temporaryBrush = true;
//         }
        break;
    }
    default: // HatchBrush
        Q_ASSERT(brush.style() > Qt::SolidPattern && brush.style() < Qt::CustomPattern);
//         d->brush = new HatchBrush(qt_hatchstyle_map[brush.style()],
//                                   conv(brush.color()),
//                                   conv(bgBrush.color()));
//         d->graphics->SetRenderingOrigin(bgOrigin.x(), bgOrigin.y());
//         d->temporaryBrush = true;
    }
}

void QGdiplusPaintEngine::updateFont(const QFont &)
{
}

void QGdiplusPaintEngine::updateBackground(Qt::BGMode, const QBrush &)
{
}

void QGdiplusPaintEngine::updateMatrix(const QMatrix &qm)
{
#ifdef QT_DEBUG_DRAW
    printf(" --- QGdiplus2PaintEngine::updateMatrix(), [%.1f %.1f %.1f %.1f %.1f %.1f]\n",
           qm.m11(), qm.m12(), qm.m21(), qm.m22(), qm.dx(), qm.dy());
#endif
//     Matrix m(qm.m11(), qm.m12(), qm.m21(), qm.m22(), qm.dx(), qm.dy());
//     d->graphics->SetTransform(&m);
    QtGpMatrix *m = 0;
    GdipCreateMatrix2(qm.m11(), qm.m12(), qm.m21(), qm.m22(), qm.dx(), qm.dy(), &m);
    GdipSetTransform(d->graphics, m);
    GdipDeleteMatrix(m);
}

void QGdiplusPaintEngine::updateClipRegion(const QRegion &qtClip, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    printf(" --- QGdiplusPaintEngine::updateClipRegion(), bounds=[%d, %d, %d, %d], op=%d\n",
           qtClip.boundingRect().x(),
           qtClip.boundingRect().y(),
           qtClip.boundingRect().width(),
           qtClip.boundingRect().height(),
           op
           );
#endif
    if (op == Qt::NoClip) {
        GdipResetClip(d->graphics);
    } else {
        if (op == Qt::ReplaceClip)
            GdipResetClip(d->graphics);

        QRegion qtRegion = qtClip.isEmpty() ? QRegion(-0x1000000, -0x1000000, 1, 1) : qtClip;
        QtGpRegion *region = 0;
        GdipCreateRegionHrgn(qtRegion.handle(), &region);
        GdipSetClipRegion(d->graphics, region,
                          op-1  // Same enum values in GDI+, but +1 due to Qt::NoClip
                          );
        GdipDeleteRegion(region);

    }
}

void QGdiplusPaintEngine::updateClipPath(const QPainterPath &clipPath, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    QRect bounds = clipPath.boundingRect().toRect();
    printf(" --- QGdiplusPaintEngine::updateClipPath(), bounds=[%d, %d, %d, %d], op=%d\n",
           bounds.x(),
           bounds.y(),
           bounds.width(),
           bounds.height(),
           op
           );
#endif
    if (op == Qt::NoClip) {
        GdipResetClip(d->graphics);
    } else {
        if (op == Qt::ReplaceClip)
            GdipResetClip(d->graphics);
        if (clipPath.isEmpty()) {
            updateClipRegion(QRegion(), op);
            return;
        }
        QtGpPath *path = d->composeGdiplusPath(clipPath);
        GdipSetClipPath(d->graphics, path,
                        op-1  // Same enum values in GDI+, but +1 due to Qt::NoClip
                        );
        GdipDeletePath(path);
    }
}

void QGdiplusPaintEngine::drawLine(const QLineF &line)
{
    if (d->usePen) {
//         d->graphics->DrawLine(d->pen, p1.x(), p1.y(), p2.x(), p2.y());
        GdipDrawLine(d->graphics, d->pen, line.startX(), line.startY(), line.endX(), line.endY());
    }
}

void QGdiplusPaintEngine::drawRect(const QRectF &r)
{
    if (d->brush) {
        GdipFillRectangle(d->graphics, d->brush, r.x(), r.y(), r.width(), r.height());
    }
    if (d->usePen) {
        GdipDrawRectangle(d->graphics, d->pen, r.x(), r.y(), r.width(), r.height());
    }
}

void QGdiplusPaintEngine::drawPoint(const QPointF &p)
{
    if (d->usePen) {
        GdipSetSolidFillColor(d->cachedSolidBrush, d->penColor.rgb());
        GdipFillRectangle(d->graphics, d->cachedSolidBrush, p.x(), p.y(), 1, 1);
        GdipSetSolidFillColor(d->cachedSolidBrush, d->brushColor.rgb());
    }
}

void QGdiplusPaintEngine::drawEllipse(const QRectF &r)
{
    if (d->brush) {
//         d->graphics->FillEllipse(d->brush, r.x(), r.y(), r.width(), r.height());
        GdipFillEllipse(d->graphics, d->brush, r.x(), r.y(),
                         r.width(), r.height());
    }
    if (d->usePen) {
//         d->graphics->DrawEllipse(d->pen, r.x(), r.y(), r.width(), r.height());
        GdipDrawEllipse(d->graphics, d->pen, r.x(), r.y(),
                         r.width(), r.height());
    }
}

void QGdiplusPaintEngine::drawPolygon(const QPolygon &p, PolygonDrawMode mode)
{
//     if (d->usePen || d->brush) {
//         Point *p = new Point[npoints];
//         for (int i=0; i<npoints; ++i)
//             p[i] = Point(pa[index+i].x(), pa[index+i].y());
//         if (d->usePen)
//             d->graphics->DrawPolygon(d->pen, p, npoints);
//         if (d->brush)
//             d->graphics->FillPolygon(d->brush, p, npoints,
//                                      winding ? FillModeWinding : FillModeAlternate);
//         delete [] p;
//     }

    if (d->usePen && mode == PolylineMode) {
        QtGpPath *path = 0;
        GdipCreatePath(0, &path);
        for (int i=1; i<p.size(); ++i)
            GdipAddPathLine(path, p.at(i-1).x(), p.at(i-1).y(), p.at(i).x(), p.at(i).y());
        GdipDrawPath(d->graphics, d->pen, path);
        GdipDeletePath(path);
        return;
    }

    if (d->brush) {
        GdipFillPolygon(d->graphics, d->brush, p.data(), p.size(),
                         mode == WindingMode
                         ? 1 // FillModeWinding
                         : 0 // FillModeAlternate
                         );
    }
    if (d->usePen)
        GdipDrawPolygon(d->graphics, d->pen, p.data(), p.size());
}



void QGdiplusPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                                     Qt::PixmapDrawingMode mode)
{
    Q_UNUSED(mode);
    if (pm.isQBitmap()) {
        const QBitmap *bitmap = static_cast<const QBitmap *>(&pm);
        QRegion rgn(*bitmap);
        QPainter *p = painter();
        p->save();
        p->translate(r.x(), r.y());
        p->setClipRegion(QRegion(*bitmap));
        p->fillRect(0, 0, r.width(), r.height(), p->pen().color());
        p->restore();
        return;
    }

    QImage backupPixels;
    QtGpBitmap *bitmap = qt_convert_to_gdipbitmap(&pm, &backupPixels);
    GdipDrawImageRectRect(d->graphics, bitmap,
                           r.x(), r.y(), r.width(), r.height(),
                           sr.x(), sr.y(), sr.width(), sr.height(),
                           2, // UnitPixel
                           0, 0, 0);
    GdipDisposeImage(bitmap);
}


void QGdiplusPaintEngine::drawPath(const QPainterPath &p)
{
#ifdef QT_NO_NATIVE_PATH
    Q_ASSERT(!"QGdiplusPaintEngine::drawPath(), QT_NO_NATIVE_PATH is defined...\n");
    return;
#endif
    QtGpPath *path = d->composeGdiplusPath(p);

    if (d->brush)
        GdipFillPath(d->graphics, d->brush, path);
    if (d->usePen)
        GdipDrawPath(d->graphics, d->pen, path);

    GdipDeletePath(path);
}

void QGdiplusPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    // We switch the pixel offset mode to avoid antialiased edges on rect fills.
    if (hints & QPainter::Antialiasing) {
        GdipSetSmoothingMode(d->graphics, 2);
        GdipSetPixelOffsetMode(d->graphics, PixelOffsetModeHalf);
    } else {
        GdipSetPixelOffsetMode(d->graphics, PixelOffsetModeDefault);
        GdipSetSmoothingMode(d->graphics, 1);
    }

    // QPaintEngine::setRenderHints() gets called on QWin32PaintEngine so
    // we make our own local copy..
    d->renderhints = hints;
}


/* Some required initialization of GDI+ needed prior to
   doing anything GDI+ related. Called by qt_init() in
   qapplication_win.cpp
*/
static Q_UINT64 *gdiplusToken = 0;
void QGdiplusPaintEngine::initialize()
{
    QtGpStartupInput input = { 1, 0, false, false };
    GdiplusStartup(&gdiplusToken, &input, 0);
}

void QGdiplusPaintEngine::cleanup()
{
    GdiplusShutdown(gdiplusToken);
}


void QGdiplusPaintEngine::drawTextItem(const QPointF &p, const QTextItem &ti)
{
    HDC hdc;
    GdipGetDC(d->graphics, &hdc);
    QtGpFont *font;
    GdipCreateFontFromLogfontW(hdc, &ti.fontEngine->logfont, &font);
    GdipReleaseDC(d->graphics, &hdc);

    GdipSetSolidFillColor(d->cachedSolidBrush, d->penColor.rgba());

    QVarLengthArray<unsigned short> glyphs(ti.num_glyphs);
    QVarLengthArray<QPointF> positions(ti.num_glyphs);
    QPointF pos = p;

    if(ti.flags & QTextItem::RightToLeft) {
        int i = ti.num_glyphs;
        while(i--) {
            pos.rx() += ti.glyphs[i].advance.x() + ((float)ti.glyphs[i].space_18d6)/64.;
            pos.ry() += ti.glyphs[i].advance.y();
        }
        i = 0;
        int nGlyphs = 0;
        while(i < ti.num_glyphs) {
            pos -= ti.glyphs[i].advance;

            positions[nGlyphs] = pos + ti.glyphs[i].offset;
            glyphs[nGlyphs++] = ti.glyphs[i].glyph;

            if (ti.glyphs[i].nKashidas) {
                positions.resize(positions.size() + ti.glyphs[i].nKashidas);
                glyphs.resize(glyphs.size() + ti.glyphs[i].nKashidas);
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                ti.fontEngine->stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < ti.glyphs[i].nKashidas; ++k) {
                    pos -= g[0].advance;

                    positions[nGlyphs] = pos;
                    glyphs[nGlyphs++] = g[0].glyph;
                }
            } else {
                pos.rx() -= ((float)ti.glyphs[i].space_18d6)/64.;
            }
            ++i;
        }
    } else {
        for (int i=0; i<ti.num_glyphs; ++i)
            glyphs[i] = ti.glyphs[i].glyph;
        for (int i=0; i<ti.num_glyphs; ++i) {
            positions[i] = pos + ti.glyphs[i].offset;
            pos += ti.glyphs[i].advance;
            pos.rx() += ((float)ti.glyphs[i].space_18d6)/64.;
        }
    }
    const uint flags = 0;

    GdipDrawDriverString(d->graphics, glyphs.data(), ti.num_glyphs, font, 
                         d->cachedSolidBrush, positions.data(), flags, 0);

    GdipSetSolidFillColor(d->cachedSolidBrush, d->brushColor.rgb());
    GdipDeleteFont(font);
}

static QtGpBitmap *qt_convert_to_gdipbitmap(const QPixmap *pixmap, QImage *image)
{
    QtGpBitmap *bitmap = 0;
    if (!pixmap->hasAlpha()) {
        GdipCreateBitmapFromHBITMAP(pixmap->hbm(), HPALETTE(0), &bitmap);
        return bitmap;
    } else { // 1 bit masks or 8 bit alpha...
        *image = pixmap->toImage();
        int pf;
        int depth = image->depth();

        if (depth == 32) {
            pf = (10 | (32 << 8) | 0x00260000); // PixelFormat32bppARGB;
        } else if (depth == 16) {
            pf = (7 | (16 << 8) | 0x00060000);  // PixelFormat16bppARGB1555;
        } else if (depth == 8) {
            *image = image->convertDepth(32);
            pf = (10 | (32 << 8) | 0x00260000); // PixelFormat32bppARGB;
        } else {
            qDebug() << "QGdiplusPaintEngine::drawPixmap(), unsupported depth:" << image->depth();
            return 0;
        }
        GdipCreateBitmapFromScan0(pixmap->width(), pixmap->height(), image->bytesPerLine(),
                                  pf, image->bits(), &bitmap);
    }
    return bitmap;
}

QtGpPath *QGdiplusPaintEnginePrivate::composeGdiplusPath(const QPainterPath &p)
{
    QtGpPath *path = 0;
    GdipCreatePath(0, &path);

    QPointF prev, start;

    // Drawing the subpaths
    for (int i=0; i<p.elementCount(); ++i) {
        const QPainterPath::Element &elm = p.elementAt(i);
        switch (elm.type) {
        case QPainterPath::MoveToElement:
            if (i>0 && start == prev)
                GdipClosePathFigure(path);
            GdipStartPathFigure(path);
            start = prev = QPointF(elm.x, elm.y);
            break;
        case QPainterPath::LineToElement:
            GdipAddPathLine(path,
                            prev.x(), prev.y(),
                            elm.x, elm.y);
            prev = QPointF(elm.x, elm.y);
            break;
        case QPainterPath::CurveToElement:
            Q_ASSERT(p.elementAt(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(p.elementAt(i+2).type == QPainterPath::CurveToDataElement);
            GdipAddPathBezier(path,
                              prev.x(), prev.y(),
                              elm.x, elm.y,
                              p.elementAt(i+1).x, p.elementAt(i+1).y,
                              p.elementAt(i+2).x, p.elementAt(i+2).y);
            i += 2;
            prev = QPointF(p.elementAt(i).x, p.elementAt(i).y);
            break;
        default:
            qFatal("QGdiplusPaintEngine::drawPath(), unhandled type: %d", elm.type);
        }
    }

    GdipSetPathFillMode(path, p.fillRule() == Qt::WindingFill ? 1 : 0);

    if (start == prev)
        GdipClosePathFigure(path);

    return path;
}



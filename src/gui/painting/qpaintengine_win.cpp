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
#include "qlibrary.h"
#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qpaintengine_win.h"
#include "qpaintengine_win_p.h"
#include "qpainter.h"
#include "qpainter_p.h"
#include "qpainterpath.h"
#include "qpainterpath_p.h"
#include "qpen.h"
#include "qpixmap.h"
#include "qt_windows.h"
#include "qtextlayout.h"
#include "qwidget.h"

#include <private/qfontengine_p.h>
#include <private/qtextengine_p.h>
#include <private/qwidget_p.h>

#include <qdebug.h>

#include <math.h>

// #define NO_NATIVE_XFORM
#define d d_func()
#define q q_func()

// For alpha blending, we must load the AlphaBlend() function at run time.
#if !defined(AC_SRC_ALPHA)
#define AC_SRC_ALPHA 0x01
#endif

// True if GDI+ and its function could be resolved...
static bool qt_gdiplus_support = false;
static bool qt_resolved_gdiplus = false;

static void qt_resolve_gdiplus();
static QPaintEngine::PaintEngineFeatures qt_decide_paintengine_features();

static QSysInfo::WinVersion qt_winver = QSysInfo::WV_NT;

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
        QTextStream s(str,IO_WriteOnly);
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
        QTextStream s(str,IO_WriteOnly);
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

    d->penAlphaColor = false;
    d->brushAlphaColor = false;

    setActive(true);
    d->pdev = pdev;

    if (pdev->devType() == QInternal::Widget) {
        QWidget *w = (QWidget*)pdev;
        d->usesWidgetDC = (w->winHDC() != 0);
        if (d->usesWidgetDC) {
            d->hdc = w->winHDC();                        // during paint event
        } else {
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
    } else if (pdev->devType() == QInternal::Pixmap) {
        d->hdc = static_cast<QPixmap *>(pdev)->winHDC();
    }
    Q_ASSERT(d->hdc);

    QRegion *region = paintEventClipRegion;
    QRect r = region ? region->boundingRect() : QRect();
    if (region)
        SelectClipRgn(d->hdc, region->handle());

    if (QColor::hPal()) {
        d->holdpal = SelectPalette(d->hdc, QColor::hPal(), true);
        RealizePalette(d->hdc);
    }

    SetBkMode(d->hdc, TRANSPARENT);
    SetTextAlign(d->hdc, TA_BASELINE);
    return true;
}

bool QWin32PaintEngine::end()
{
    if (!isActive()) {
        qWarning("QWin32PaintEngine::end: Missing begin() or begin() failed");
        return false;
    }

    if (d->gdiplusEngine)
        d->endGdiplus();

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

    if (d->pdev->devType() == QInternal::Widget) {
        if (!d->usesWidgetDC) {
            QWidget *w = (QWidget*)d->pdev;
            ReleaseDC(w->isDesktop() ? 0 : w->winId(), d->hdc);
            const_cast<QWidgetPrivate*>(w->d)->hd = 0;
        }
    } else if (d->pdev->devType() == QInternal::Pixmap) {
        QPixmap *pm = (QPixmap *)d->pdev;
        if (pm->optimization() == QPixmap::MemoryOptim &&
             (qt_winver & QSysInfo::WV_DOS_based))
            pm->allocCell();
    }

    if (GetGraphicsMode(d->hdc)==GM_ADVANCED) {
        if (!ModifyWorldTransform(d->hdc, 0, MWT_IDENTITY))
            qWarning("QWin32PaintEngine::end(). ModifyWorldTransform failed: code: %d\n", GetLastError());
        SetGraphicsMode(d->hdc, GM_COMPATIBLE);
    }

    d->hdc = 0;

    d->matrix = QWMatrix();
    d->penAlphaColor = false;
    d->brushAlphaColor = false;
    d->noNativeXform = false;
    d->advancedMode = false;
    d->penStyle = Qt::SolidLine;
    d->brushStyle = Qt::SolidPattern;
    d->txop = QPainter::TxNone;

    setActive(false);
    return true;
}

void QWin32PaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    Q_ASSERT(isActive());

    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawLine(p1, p2);
        return;
    }

    int x1 = p1.x(), x2 = p2.x(), y1 = p1.y(), y2 = p2.y();
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

void QWin32PaintEngine::drawRect(const QRect &r)
{
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawRect(r);
        return;
    }
    int w = r.width(), h = r.height();

    // Due to inclusive rectangles when using GM_ADVANCED
    if (d->advancedMode) {
        --w;
        --h;
    }
    if (d->penStyle == Qt::NoPen) {
        w++;
        h++;
    }

    bool outlineOnly = false;
    if (d->brushStyle == Qt::LinearGradientPattern) {
        d->fillGradient(r);
        outlineOnly = true;
    } else if (d->brushStyle == Qt::SolidPattern && d->brush.color().alpha() != 255) {
        d->fillAlpha(r);
        outlineOnly = true;
    }

    if (outlineOnly) {
        if (d->penStyle != Qt::NoPen) {
            SelectObject(d->hdc, stock_nullBrush);
            Rectangle(d->hdc, r.x(), r.y(), r.x() + w, r.y() + h);
            SelectObject(d->hdc, d->hbrush);
        }
        return;
    }

    if (d->nocolBrush) {
        SetTextColor(d->hdc, d->bColor);
        Rectangle(d->hdc, r.x(), r.y(), r.x()+w, r.y()+h);
        SetTextColor(d->hdc, d->pColor);
    } else {
        Rectangle(d->hdc, r.x(), r.y(), r.x()+w, r.y()+h);
    }
}

void QWin32PaintEngine::drawPoint(const QPoint &p)
{
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPoint(p);
        return;
    }

    if (d->penStyle != Qt::NoPen)
#ifndef Q_OS_TEMP
        SetPixelV(d->hdc, p.x(), p.y(), d->pColor);
#else
        SetPixel(d->hdc, p.x(), p.y(), d->pColor);
#endif
}

void QWin32PaintEngine::drawPoints(const QPointArray &pts, int index, int npoints)
{
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPoints(pts, index, npoints);
        return;
    }
    QPointArray pa = pts;
    if (d->penStyle != Qt::NoPen) {
        for (int i=0; i<npoints; i++) {
#ifndef Q_OS_TEMP
            SetPixelV(d->hdc, pa[index+i].x(), pa[index+i].y(),
                       d->pColor);
#else
            SetPixel(d->hdc, pa[index+i].x(), pa[index+i].y(),
                       d->pColor);
#endif
        }
    }
}

void QWin32PaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawRoundRect(r, xRnd, yRnd);
        return;
    }

    if (xRnd <= 0 || yRnd <= 0) {
        drawRect(r);                        // draw normal rectangle
        return;
    }
    if (xRnd >= 100)                                // fix ranges
        xRnd = 99;
    if (yRnd >= 100)
        yRnd = 99;

    int w = r.width(), h = r.height();
#ifdef NO_NATIVE_XFORM
    if (d->penStyle == Qt::NoPen) {
        ++w;
        ++h;
    }
#else
    --w;
    --h;
#endif

    if (d->nocolBrush)
        SetTextColor(d->hdc, d->bColor);
    RoundRect(d->hdc, r.x(), r.y(), r.x()+w, r.y()+h,
              w*xRnd/100, h*yRnd/100);
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->pColor);
}

void QWin32PaintEngine::drawEllipse(const QRect &r)
{
    Q_ASSERT(isActive());
     if (d->tryGdiplus()) {
        d->gdiplusEngine->drawEllipse(r);
        return;
    }
   // Workaround for Windows GDI
//     QPen oldPen = d->cpen;
//     if (d->cpen == PS_NULL) {
// //         setPen(d->bColor);
// //         updatePen();
//     }
    int w = r.width();
    int h = r.height();
    --w;
    --h;

    if (d->nocolBrush)
        SetTextColor(d->hdc, d->bColor);
    if (w == 1 && h == 1)
        drawPoint(r.topLeft());
    else
        Ellipse(d->hdc, r.x(), r.y(), r.x()+w, r.y()+h);
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->pColor);

//     if (oldPen == PS_NULL)
//         setPen(oldPen);
}

void QWin32PaintEngine::drawArc(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawArc(r, a, alen);
        return;
    }

    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if (alen < 0.0) {                                // swap angles
        double t = ra1;
        ra1 = ra2;
        ra2 = t;
    }

    int w = r.width(), h = r.height();
#ifdef NO_NATIVE_XFORM
    if (d->penStyle == Qt::NoPen) {
        ++w;
        ++h;
    }
#else
    --w;
    --h;
#endif

    if (d->pWidth > 3) {
        // work around a bug in the windows Arc method that
        // sometimes draw wrong cap styles.
        if (w % 2)
            w += 1;
        if (h % 2)
            h += 1;
    }

    double w2 = 0.5*w;
    double h2 = 0.5*h;
    int xS = qRound(w2 + (cos(ra1)*w2) + r.x());
    int yS = qRound(h2 - (sin(ra1)*h2) + r.y());
    int xE = qRound(w2 + (cos(ra2)*w2) + r.x());
    int yE = qRound(h2 - (sin(ra2)*h2) + r.y());
    if (QABS(alen) < 90*16) {
        if ((xS == xE) && (yS == yE)) {
            // don't draw a whole circle
            return;
        }
    }
#ifndef Q_OS_TEMP
    Arc(d->hdc, r.x(), r.y(), r.x()+w, r.y()+h, xS, yS, xE, yE);
#else
    QPointArray pa;
    pa.makeArc(r.x(), r.y(), w, h, a, alen);        // arc polyline
    drawPolyInternal(pa, false);
#endif
}

void QWin32PaintEngine::drawPie(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPie(r, a, alen);
        return;
    }

    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if (alen < 0.0) {                                // swap angles
        double t = ra1;
        ra1 = ra2;
        ra2 = t;
    }

    int w = r.width(), h = r.height();
#ifdef NO_NATIVE_XFORM
    if (d->penStyle == Qt::NoPen) {
        ++w;
        ++h;
    }
#else
    --w;
    --h;
#endif

    double w2 = 0.5*w;
    double h2 = 0.5*h;
    int xS = qRound(w2 + (cos(ra1)*w) + r.x());
    int yS = qRound(h2 - (sin(ra1)*h) + r.y());
    int xE = qRound(w2 + (cos(ra2)*w) + r.x());
    int yE = qRound(h2 - (sin(ra2)*h) + r.y());
    if (QABS(alen) < 90*16) {
        if ((xS == xE) && (yS == yE)) {
            // don't draw a whole circle
            return;
        }
    }
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->bColor);
#ifndef Q_OS_TEMP
    Pie(d->hdc, r.x(), r.y(), r.right()+1, r.bottom()+1, xS, yS, xE, yE);
#endif
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->pColor);
}

void QWin32PaintEngine::drawChord(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if (ra2 < 0.0) {                                // swap angles
        double t = ra1;
        ra1 = ra2;
        ra2 = t;
    }

    int w = r.width(), h = r.height();
#ifdef NO_NATIVE_XFORM
    if (d->penStyle == Qt::NoPen) {
        ++w;
        ++h;
    }
#else
    --w;
    --h;
#endif

    double w2 = 0.5*w;
    double h2 = 0.5*h;
    int xS = qRound(w2 + (cos(ra1)*w) + r.x());
    int yS = qRound(h2 - (sin(ra1)*h) + r.y());
    int xE = qRound(w2 + (cos(ra2)*w) + r.x());
    int yE = qRound(h2 - (sin(ra2)*h) + r.y());
    if (QABS(alen) < 90*16) {
        if ((xS == xE) && (yS == yE)) {
            // don't draw a whole circle
            return;
        }
    }
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->bColor);
#ifndef Q_OS_TEMP
    Chord(d->hdc, r.x(), r.y(), r.right()+1, r.bottom()+1, xS, yS, xE, yE);
#endif
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->pColor);
}


void QWin32PaintEngine::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    Q_ASSERT(isActive());
    QPointArray pa = a;

    int         x1, y1, x2, y2;
    uint i = index;
    uint pixel = d->pColor;
    bool maybe_plot_pixel = false;
    QT_WA({
        maybe_plot_pixel = (d->pWidth == 0) && (d->penStyle == Qt::SolidLine);
    } , {
        maybe_plot_pixel = (d->pWidth <= 1) && (d->penStyle == Qt::SolidLine);
    });

    while (nlines--) {
        pa.point(i++, &x1, &y1);
        pa.point(i++, &x2, &y2);
        if (x1 == x2) {                        // vertical
            if (y1 < y2)
                y2++;
            else
                y2--;
        } else if (y1 == y2) {                // horizontal
            if (x1 < x2)
                x2++;
            else
                x2--;
        } else if (maybe_plot_pixel) {        // draw last pixel
#ifndef Q_OS_TEMP
            SetPixelV(d->hdc, x2, y2, pixel);
#else
            SetPixel(d->hdc, x2, y2, pixel);
#endif
        }

#ifndef Q_OS_TEMP
        MoveToEx(d->hdc, x1, y1, 0);
        LineTo(d->hdc, x2, y2);
#else
        // PolyLine from x1, y1 to x2, y2.
        POINT linePts[2] = { { x1, y1 }, { x2, y2 } };
        Polyline(d->hdc, linePts, 2);
        internalCurrentPos = QPoint(x2, y2);
#endif
    }
}

void QWin32PaintEngine::drawPolyline(const QPointArray &a, int index, int npoints)
{
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPolyline(a, index, npoints);
        return;
    }
    QPointArray pa = a;
    int x1, y1, x2, y2, xsave, ysave;
    pa.point(index+npoints-2, &x1, &y1);        // last line segment
    pa.point(index+npoints-1, &x2, &y2);
    xsave = x2; ysave = y2;
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
        Polyline(d->hdc, (POINT*)(pa.data()+index), npoints);
#ifndef Q_OS_TEMP
        SetPixelV(d->hdc, x2, y2, d->pColor);
#else
        SetPixel(d->hdc, x2, y2, d->pColor);
#endif
    } else {
        pa.setPoint(index+npoints-1, x2, y2);
        Polyline(d->hdc, (POINT*)(pa.data()+index), npoints);
        pa.setPoint(index+npoints-1, xsave, ysave);
    }
}

void QWin32PaintEngine::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
{
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPolygon(a, winding, index, npoints);
        return;
    }

    QPointArray pa = a;
#ifndef Q_OS_TEMP
    if (winding)                                // set to winding fill mode
        SetPolyFillMode(d->hdc, WINDING);
#endif
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->bColor);
    Polygon(d->hdc, (POINT*)(pa.data()+index), npoints);
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->pColor);
#ifndef Q_OS_TEMP
    if (winding)                                // set to normal fill mode
        SetPolyFillMode(d->hdc, ALTERNATE);
#endif

}

void QWin32PaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawConvexPolygon(pa, index, npoints);
        return;
    }
    // Any efficient way?
    drawPolygon(pa,false,index,npoints);

}

#ifndef QT_NO_BEZIER
void QWin32PaintEngine::drawCubicBezier(const QPointArray &a, int index)
{
    Q_ASSERT(isActive());
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawCubicBezier(a, index);
        return;
    }
    if ((int)a.size() - index < 4) {
        qWarning("QPainter::drawCubicBezier: Cubic Bezier needs 4 control "
                 "points");
        return;
    }
    QPointArray pa(a);
#ifndef Q_OS_TEMP
    PolyBezier(d->hdc, (POINT*)(pa.data()+index), 4);
#endif
}
#endif

void QWin32PaintEngine::drawPath(const QPainterPath &p)
{
    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPath(p);
        return;
    }

    const QPainterPathPrivate *pd = p.d;

    d->composeGdiPath(pd);

    if (p.fillMode() == QPainterPath::Winding)
        SetPolyFillMode(d->hdc, WINDING);

    bool pen = d->penStyle != Qt::NoPen;
    bool brush = d->brushStyle != Qt::NoBrush;

    // Workaround for filling gradients
    if (d->brushStyle == Qt::LinearGradientPattern
        || (d->brushStyle == Qt::SolidPattern && d->brushAlphaColor)) {
        HRGN oldRegion = 0;
        int gotRegion = GetClipRgn(d->hdc, oldRegion);
        Q_ASSERT(gotRegion >= 0);
        SelectClipPath(d->hdc, RGN_AND);
        if (d->brushStyle == Qt::LinearGradientPattern)
            d->fillGradient(p.boundingRect());
        else
            d->fillAlpha(p.boundingRect());
        brush = false;
        if (gotRegion == 0) { // No path originally
            SelectClipRgn(d->hdc, 0);
        } else if (gotRegion == 1){ // Reset and release original path
            Q_ASSERT(oldRegion);
            SelectClipRgn(d->hdc, oldRegion);
            DeleteObject(oldRegion);
        }
        if (pen)
            d->composeGdiPath(pd);
    }

    if (pen && brush)
        StrokeAndFillPath(d->hdc);
    else if (pen)
        StrokePath(d->hdc);
    else if (brush)
        FillPath(d->hdc);

    if (p.fillMode() == QPainterPath::Winding)
        SetPolyFillMode(d->hdc, ALTERNATE);


}


void QWin32PaintEngine::initialize()
{
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


void QWin32PaintEngine::drawPolyInternal(const QPointArray &a, bool close)
{
    Q_ASSERT(isActive());
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->bColor);
    if (close) {
        Polygon(d->hdc, (POINT*)a.data(), a.size());
    } else {
        Polyline(d->hdc, (POINT*)a.data(), a.size());
    }
    if (d->nocolBrush)
        SetTextColor(d->hdc, d->pColor);
}

void QWin32PaintEngine::drawPixmap(const QRect &r, const QPixmap &pixmap, const QRect &sr,
                                   Qt::PixmapDrawingMode mode)
{
    Q_ASSERT(isActive());

    bool stretch = r.width() != sr.width() || r.height() != sr.height();

    if (d->tryGdiplus()) {
        d->gdiplusEngine->drawPixmap(r, pixmap, sr, mode);
        return;
    }

    QPixmap *pm = (QPixmap*)&pixmap;
    QBitmap *mask = (QBitmap*)pm->mask();
    if (!mask && pixmap.isQBitmap() && d->bgMode == Qt::TransparentMode)
       mask = (QBitmap*) pm;

    HDC pm_dc;
    int pm_offset;
    if (pm->isMultiCellPixmap()) {
        pm_dc = pm->multiCellHandle();
        pm_offset = pm->multiCellOffset();
    } else {
        pm_dc = pm->winHDC();
        pm_offset = 0;
    }

    if (mode == Qt::CopyPixmapNoMask)
        mask = 0;

    Q_ASSERT(pm_dc);

    // sanity check...
    if (GetGraphicsMode(pm_dc) != GM_COMPATIBLE) {
        XFORM xform;
        if (!GetWorldTransform(pm_dc, &xform)) {
            qSystemWarning("QWin32PaintEngine::drawPixmap, getting source xform failed");
            return;
        }
        if (xform.eM12 != 0 || xform.eM21 != 0) {
            qWarning("QWin32PaintEngine::drawPixmap, cannot draw pixmap with matrix");
            return;
        }
    }

    if (pixmap.hasAlphaChannel() && mode == Qt::ComposePixmap) {
        BLENDFUNCTION bf = { AC_SRC_OVER,       // BlendOp
                             0,                 // BlendFlags, must be zero
                             255,               // SourceConstantAlpha, we use pr pixel
                             AC_SRC_ALPHA       // AlphaFormat
        };
        if (!AlphaBlend(d->hdc, r.x(), r.y(), r.width(), r.height(),
                        pm_dc, sr.x(), sr.y(), sr.width(), sr.height(),
                        bf)) {
            qSystemWarning("QWin32PaintEngine::drawPixmap, AlphaBlend failed...");
            return;
        }
    } else if (mask && mode == Qt::ComposePixmap) {
        if (stretch) {
            QImage imageData(pixmap);
            QImage imageMask = imageData.createAlphaMask();
            QBitmap tmpbm = imageMask;
            QBitmap bm(sr.width(), sr.height());
            {
                QPainter p(&bm);
                p.drawPixmap(QRect(0, 0, sr.width(), sr.height()), tmpbm, sr, Qt::CopyPixmapNoMask);
            }
            QWMatrix xform = QWMatrix(r.width()/(double)sr.width(), 0,
                                      0, r.height()/(double)sr.height(),
                                      0, 0);
            bm = bm.xForm(xform);
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
                         pm_dc, sr.x(), sr.y()+pm_offset,
                         mask->hbm(), sr.x(), sr.y()+pm_offset,
                         MAKEROP4(0x00aa0000, SRCCOPY)))
                qSystemWarning("QWin32PaintEngine::drawPixmap, MaskBlt failed");
        }
    } else {
        if (stretch) {
            Q_ASSERT((GetDeviceCaps(d->hdc, RASTERCAPS) & RC_STRETCHBLT) != 0);
            if (!StretchBlt(d->hdc, r.x(), r.y(), r.width(), r.height(),
                            pm_dc, sr.x(), sr.y(), sr.width(), sr.height(),
                            SRCCOPY)) {
                qSystemWarning("QWin32PaintEngine::drawPixmap, StretchBlt failed");
            }
        } else {
            Q_ASSERT((GetDeviceCaps(d->hdc, RASTERCAPS) & RC_BITBLT) != 0);
            if (!BitBlt(d->hdc, r.x(), r.y(), sr.width(), sr.height(),
                        pm_dc, sr.x(), sr.y(),
                        SRCCOPY)) {
                qSystemWarning("QWin32PaintEngine::drawPixmap, BitBlt failed");
            }
        }
    }
}

void QWin32PaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textFlags)
{
    // We cannot render text in GDI+ mode, so turn it of if it is currently used...
    bool usesGdiplus = d->usesGdiplus();
    bool oldForceGdi = d->forceGdi;
    if (usesGdiplus) {
        d->forceGdi = true;
        d->endGdiplus();
        setDirty(AllDirty);
        updateState(state);
    }

    HDC oldHdc = ti.fontEngine->hdc;
    ti.fontEngine->hdc = d->hdc;
    QPaintEngine::drawTextItem(p, ti, textFlags);
    ti.fontEngine->hdc = oldHdc;

    if (usesGdiplus)
        d->forceGdi = oldForceGdi;
}


void QWin32PaintEngine::updatePen(const QPen &pen)
{
    d->penStyle = pen.style();
    d->penAlphaColor = d->penStyle != Qt::NoPen && pen.color().alpha() != 255;
    if (d->tryGdiplus()) {
        d->gdiplusEngine->updatePen(pen);
        return;
    }

    int old_pix = d->pColor;
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
        d->hpen = CreatePen(s, pen.width(), d->pColor);
    }

set:
    if (old_pix != d->pColor)
        SetTextColor(d->hdc, d->pColor);
    SelectObject(d->hdc, d->hpen);
    if (hpen_old)
        DeleteObject(hpen_old);
    return;
}


void QWin32PaintEngine::updateBrush(const QBrush &brush, const QPoint &bgOrigin)
{
    d->brush = brush;
    d->brushStyle = brush.style();
    d->brushAlphaColor = (d->brushStyle != Qt::NoBrush
                          && d->brushStyle != Qt::LinearGradientPattern
                          && brush.color().alpha() != 255);
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
                DeleteObject(hbrush_old);
                if (d->hbrushbm && !d->pixmapBrush)
                    DeleteObject(d->hbrushbm);
                d->hbrushbm = 0;
                d->pixmapBrush = d->nocolBrush = false;
            }
            return;
        }
        if (obtain_brush(&d->brushRef, &d->hbrush, d->bColor)) {
            SelectObject(d->hdc, d->hbrush);
            if (hbrush_old) {
                DeleteObject(hbrush_old);
                if (d->hbrushbm && !d->pixmapBrush)
                    DeleteObject(d->hbrushbm);
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
            d->hbrushbm = brush.pixmap()->hbm();
            d->pixmapBrush = true;
            d->nocolBrush = brush.pixmap()->depth() == 1;
        } else {
            short *bm = dense_patterns[d->brushStyle - Qt::Dense1Pattern];
            d->hbrushbm = CreateBitmap(8, 8, 1, 1, bm);
            d->nocolBrush = true;
        }
        d->hbrush = CreatePatternBrush(d->hbrushbm);
        DeleteObject(d->hbrushbm);
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
            coltbl[0] = d->bColor.rgb();
            coltbl[1] = Qt::color0.rgb();

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
    SetBrushOrgEx(d->hdc, bgOrigin.x(), bgOrigin.y(), 0);
    SelectObject(d->hdc, d->hbrush);

    if (hbrush_old) {
        DeleteObject(hbrush_old);                // delete last brush
        if (hbrushbm_old && !pixmapBrush_old)
            DeleteObject(hbrushbm_old);        // delete last brush pixmap
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


void QWin32PaintEngine::updateXForm(const QWMatrix &mtx)
{
#ifdef NO_NATIVE_XFORM
    return;
#endif

    if (d->tryGdiplus()) {
        d->gdiplusEngine->updateXForm(mtx);
        return;
    }

    if (mtx.m12() != 0 || mtx.m21() != 0)
        d->txop = QPainter::TxRotShear;
    else if (mtx.m11() != 1 || mtx.m22() != 1)
        d->txop = QPainter::TxScale;
    else if (mtx.dx() != 0 || mtx.dy() != 0)
        d->txop = QPainter::TxTranslate;
    else
        d->txop = QPainter::TxNone;
    d->matrix = mtx;

    XFORM m;
    if (d->txop > QPainter::TxNone && !d->noNativeXform) {
        m.eM11 = mtx.m11();
        m.eM12 = mtx.m12();
        m.eM21 = mtx.m21();
        m.eM22 = mtx.m22();
        m.eDx  = mtx.dx();
        m.eDy  = mtx.dy();
        SetGraphicsMode(d->hdc, GM_ADVANCED);
        if (!SetWorldTransform(d->hdc, &m)) {
            qSystemWarning("QWin32PaintEngine::updateXForm(), SetWorldTransformation() failed..");
        }
        d->advancedMode = true;
    } else {
        m.eM11 = m.eM22 = (float)1.0;
        m.eM12 = m.eM21 = m.eDx = m.eDy = (float)0.0;
        SetGraphicsMode(d->hdc, GM_ADVANCED);
        ModifyWorldTransform(d->hdc, &m, MWT_IDENTITY);
        SetGraphicsMode(d->hdc, GM_COMPATIBLE);
        d->advancedMode = false;
    }
}


void QWin32PaintEngine::updateClipRegion(const QRegion &region, bool clipEnabled)
{
    if (d->tryGdiplus()) {
        d->gdiplusEngine->updateClipRegion(region, clipEnabled);
        return;
    }
    if (clipEnabled) {
        QRegion rgn =
#ifndef NO_NATIVE_XFORM
            d->matrix *
#endif
            region;

        // Setting an empty region as clip region on Win just dmainisables clipping completely.
        // To workaround this and get the same semantics on Win and Unix, we set a 1x1 pixel
        // clip region far out in coordinate space in this case.
        if (rgn.isEmpty())
            rgn = QRect(-0x1000000, -0x1000000, 1, 1);
        SelectClipRgn(d->hdc, rgn.handle());
    } else {
        SelectClipRgn(d->hdc, 0);
    }

}

void QWin32PaintEngine::updateFont(const QFont &font)
{
    if (d->tryGdiplus()) {
        d->gdiplusEngine->updateFont(font);
        return;
    }

    if (state->pfont)
        delete state->pfont;
#undef d
    state->pfont = new QFont(font.d, d_func()->pdev);
#define d d_func()
}

extern void qt_fill_tile(QPixmap *tile, const QPixmap &pixmap);
extern void qt_draw_tile(QPaintEngine *, int, int, int, int, const QPixmap &, int, int,
			 Qt::PixmapDrawingMode);

void QWin32PaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &p,
					Qt::PixmapDrawingMode mode)
{
    QBitmap *mask = (QBitmap *)pixmap.mask();

    int sw = pixmap.width();
    int sh = pixmap.height();

    if (sw*sh < 8192 && sw*sh < 16*r.width()*r.height()) {
        int tw = sw, th = sh;
        while (tw*th < 32678 && tw < r.width()/2)
            tw *= 2;
        while (tw*th < 32678 && th < r.height()/2)
            th *= 2;
        QPixmap tile;
        if (pixmap.hasAlphaChannel()) {
            QImage image(tw, th, 32);
            image.fill(QColor(0, 0, 0, 0).rgb());
            image.setAlphaBuffer(true);
            tile = image;
        } else {
            tile = QPixmap(tw, th, pixmap.depth(), QPixmap::BestOptim);
        }
        qt_fill_tile(&tile, pixmap);
        if (mask) {
            QBitmap tilemask(tw, th, false, QPixmap::NormalOptim);
            qt_fill_tile(&tilemask, *mask);
            tile.setMask(tilemask);
        }
        qt_draw_tile(this, r.x(), r.y(), r.width(), r.height(), tile, p.x(), p.y(), mode);
    } else {
        qt_draw_tile(this, r.x(), r.y(), r.width(), r.height(), pixmap, p.x(), p.y(), mode);
    }
}

void QWin32PaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    if (d->tryGdiplus())
        d->gdiplusEngine->updateRenderHints(hints);
}

QPainter::RenderHints QWin32PaintEngine::supportedRenderHints() const
{
    if (qt_gdiplus_support)
        return QPainter::LineAntialiasing;
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
    QColor gcol1 = brush.color();
    QColor gcol2 = brush.gradientColor();

    bool useMemDC = !(rect.x() == 0
                     && rect.y() == 0
                     && rect.width() == QPaintDeviceMetrics(pdev).width()
                     && rect.height() == QPaintDeviceMetrics(pdev).height()
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

    QPoint gstart = brush.gradientStart();
    QPoint gstop  = brush.gradientStop();

    gstart -= rect.topLeft();
    gstop -= rect.topLeft();


    int dx = gstop.x() - gstart.x();
    int dy = gstop.y() - gstart.y();

    int rw = rect.width();
    int rh = rect.height();

    if (QABS(dx) > QABS(dy)) { // Fill horizontally
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
            xtop1 = xbot1 = gstart.x();
            xtop2 = xbot2 = gstop.x();
        } else {
            double gamma = double(dx) / double(-dy);
            xtop1 = qRound((-gstart.y() + gamma * gstart.x() ) / gamma);
            xtop2 = qRound((-gstop.y()  + gamma * gstop.x()  ) / gamma);
            xbot1 = qRound((rh - gstart.y() + gamma * gstart.x() ) / gamma);
            xbot2 = qRound((rh - gstop.y()  + gamma * gstop.x()  ) / gamma);
            Q_ASSERT(xtop2 > xtop1);
        }

#ifndef QT_GRAD_NO_POLY
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
        GradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);

        // Fill the area to the right of the gradient
        polyCount = 0;
	polygon[polyCount++] = createVertex(xtop2-1, 0, gcol2);
	if (xtop2 < rw)
	    polygon[polyCount++] = createVertex(rw, 0, gcol2);
	if (xbot2 < rw)
	    polygon[polyCount++] = createVertex(rw, rh, gcol2);
	polygon[polyCount++] = createVertex(xbot2-1, rh, gcol2);
        gtrCount = polyCount == 4 ? 2 : 1;
        GradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);
#endif // QT_GRAD_NO_POLY

        polygon[0] = createVertex(xtop1, 0, gcol1);
        polygon[1] = createVertex(xbot1, rh, gcol1);
        polygon[2] = createVertex(xbot2, rh, gcol2);
        polygon[3] = createVertex(xtop2, 0, gcol2);
        GradientFill(memdc, polygon, 4, gtr, 2, GRADIENT_FILL_TRIANGLE);

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
            yleft1 = yright1 = gstart.y();
            yleft2 = yright2 = gstop.y();
        } else {
            double gamma = double(dy) / double(-dx);
            yleft1 = qRound((-gstart.x() + gamma * gstart.y()) / gamma);
            yleft2 = qRound((-gstop.x() + gamma * gstop.y()) / gamma);
            yright1 = qRound((rw - gstart.x() + gamma*gstart.y()) / gamma);
            yright2 = qRound((rw - gstop.x() + gamma*gstop.y()) / gamma);
            Q_ASSERT(yleft2 > yleft1);
        }

#ifndef QT_GRAD_NO_POLY
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
        GradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);

        polyCount = 0;
	polygon[polyCount++] = createVertex(0, yleft2-1, gcol2);
	if (yleft2 < rh)
	    polygon[polyCount++] = createVertex(0, rh, gcol2);
	if (yright2 < rh)
	    polygon[polyCount++] = createVertex(rw, rh, gcol2);
	polygon[polyCount++] = createVertex(rw, yright2-1, gcol2);
        gtrCount = polyCount == 4 ? 2 : 1;
        GradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);
#endif // QT_GRAD_NO_POLY

        polygon[0] = createVertex(0, yleft1, gcol1);
        polygon[1] = createVertex(rw, yright1, gcol1);
        polygon[2] = createVertex(rw, yright2, gcol2);
        polygon[3] = createVertex(0, yleft2, gcol2);
        GradientFill(memdc, polygon, 4, gtr, 2, GRADIENT_FILL_TRIANGLE);
    }


    if (useMemDC) {
        if (gcol1.alpha() != 255 || gcol2.alpha() != 255) {
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
            AlphaBlend(hdc, rect.x(), rect.y(), rw, rh, memdc, 0, 0, rw, rh, bf);
        } else {
            BitBlt(hdc, rect.x(), rect.y(), rw, rh, memdc, 0, 0, SRCCOPY);
        }

        DeleteObject(bitmap);
        DeleteDC(memdc);
    }
}

void QWin32PaintEnginePrivate::fillAlpha(const QRect &r)
{
    Q_ASSERT(brush.style() == Qt::SolidPattern);
    Q_ASSERT(brush.color().alpha() != 255);

    TRIVERTEX polygon[4];
    polygon[0] = createVertex(0, 0, brush.color());
    polygon[1] = createVertex(r.width(), 0, brush.color());
    polygon[2] = createVertex(r.width(), r.height(), brush.color());
    polygon[3] = createVertex(0, r.height(), brush.color());

    GRADIENT_TRIANGLE gTri[] = { { 0, 1, 2 }, { 2, 3, 0 } };

    HDC memdc = CreateCompatibleDC(hdc);
    HBITMAP bitmap = CreateCompatibleBitmap(hdc, r.width(), r.height());
    SelectObject(memdc, bitmap);

    GradientFill(memdc, polygon, 4, gTri, 2, GRADIENT_FILL_TRIANGLE);

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    AlphaBlend(hdc, r.x(), r.y(), r.width(), r.height(), memdc, 0, 0, r.width(), r.height(), bf);

    DeleteObject(bitmap);
    DeleteDC(memdc);
}

void QWin32PaintEnginePrivate::composeGdiPath(const QPainterPathPrivate *pd)
{
    if (!BeginPath(hdc))
        qSystemWarning("QWin32PaintEngine::drawPath(), begin path failed.");

    // Drawing the subpaths
    for (int i=0; i<pd->subpaths.size(); ++i) {
        const QPainterSubpath &sub = pd->subpaths.at(i);
        if (sub.elements.isEmpty())
            continue;
        QPoint firstPoint = sub.firstPoint();
        MoveToEx(hdc, firstPoint.x(), firstPoint.y(), 0);
        for (int j=0; j<sub.elements.size(); ++j) {
            const QPainterPathElement &elm = sub.elements.at(j);
            switch (elm.type) {
            case QPainterPathElement::Line: {
                LineTo(hdc, elm.lineData.x2, elm.lineData.y2);
                break;
            }
            case QPainterPathElement::Bezier: {
                POINT pts[3] = {
                    { elm.bezierData.x2, elm.bezierData.y2 },
                    { elm.bezierData.x3, elm.bezierData.y3 },
                    { elm.bezierData.x4, elm.bezierData.y4 }
                };
                PolyBezierTo(hdc, pts, 3);
                break;
            }
            case QPainterPathElement::Arc: {
                // Not supported on Win9x
                QPointArray array;
                array.makeArc(elm.arcData.x, elm.arcData.y,
                              elm.arcData.w, elm.arcData.h,
                              elm.arcData.start, elm.arcData.length);
                Q_ASSERT(array.size() > 1);
                POINT *pts = new POINT[array.size()-1];
                for (int pt=1; pt<array.size(); ++pt) {
                    pts[pt-1].x = array.at(pt).x();
                    pts[pt-1].y = array.at(pt).y();
                }
                PolylineTo(hdc, pts, array.size()-1);
                break;
            }
            default:
                qFatal("QWin32PaintEngine::drawPath(), unhandled subpath type: %d", elm.type);
            }
        }
        CloseFigure(hdc);
    }

    if (!EndPath(hdc))
        qSystemWarning("QWin32PaintEngine::drawPath(), end path failed");
}


void QWin32PaintEnginePrivate::beginGdiplus()
{
    if (!qt_resolved_gdiplus)
        qt_resolve_gdiplus();

    if (!qt_gdiplus_support)
        return;

    ModifyWorldTransform(hdc, 0, MWT_IDENTITY);
    SetGraphicsMode(hdc, GM_COMPATIBLE);

    if (!gdiplusEngine)
        gdiplusEngine = new QGdiplusPaintEngine();
    gdiplusEngine->begin(pdev);
    gdiplusEngine->state = q->state;
    gdiplusInUse = true;
    q->setDirty(QPaintEngine::DirtyFlags(QPaintEngine::AllDirty&~QPaintEngine::DirtyClip));
    q->updateState(q->state);
}

void QWin32PaintEnginePrivate::endGdiplus()
{
    Q_ASSERT(gdiplusEngine);
    gdiplusEngine->end();
    gdiplusInUse = false;

    q->setDirty(QPaintEngine::DirtyFlags(QPaintEngine::AllDirty&~QPaintEngine::DirtyClip));
}


static QPaintEngine::PaintEngineFeatures qt_decide_paintengine_features()
{
    if (!qt_resolved_gdiplus)
        qt_resolve_gdiplus();

    QPaintEngine::PaintEngineFeatures commonFeatures =
#ifndef NO_NATIVE_XFORM
        QPaintEngine::CoordTransform
        | QPaintEngine::PenWidthTransform
        | QPaintEngine::PixmapTransform
        | QPaintEngine::PixmapScale
        | QPaintEngine::ClipTransform
#endif
        | QPaintEngine::PainterPaths
        | QPaintEngine::UsesFontEngine
        | QPaintEngine::LinearGradients;

    commonFeatures |= QPaintEngine::SolidAlphaFill;



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

typedef int (__stdcall *PtrGdipCreateFromHDC) (HDC hdc, QtGpGraphics **);
typedef int (__stdcall *PtrGdipDeleteGraphics) (QtGpGraphics *);
typedef int (__stdcall *PtrGdipSetTransform) (QtGpGraphics *, QtGpMatrix *);
typedef int (__stdcall *PtrGdipSetClipRegion) (QtGpGraphics *, QtGpRegion *, int);
typedef int (__stdcall *PtrGdipResetClip) (QtGpGraphics *);
typedef int (__stdcall *PtrGdipSetSmoothingMode)(QtGpGraphics *, int);
typedef int (__stdcall *PtrGdipFillEllipseI) (QtGpGraphics *, QtGpBrush *, int x, int y, int w, int h);
typedef int (__stdcall *PtrGdipFillRectangleI) (QtGpGraphics *, QtGpBrush *, int x, int y, int w, int h);
typedef int (__stdcall *PtrGdipFillPath) (QtGpGraphics *, QtGpBrush *, QtGpPath *);
typedef int (__stdcall *PtrGdipFillPolygonI) (QtGpGraphics *, QtGpBrush *, const QPoint *, int, int);
typedef int (__stdcall *PtrGdipDrawRectangleI) (QtGpGraphics *, QtGpPen *, int x, int y, int w, int h);
typedef int (__stdcall *PtrGdipDrawEllipseI) (QtGpGraphics *, QtGpPen *, int x, int y, int w, int h);
typedef int (__stdcall *PtrGdipDrawImageRectRectI) (QtGpGraphics *, QtGpImage *,
                                                    int x, int y, int w, int h,
                                                    int sx, int sy, int sw, int sh,
                                                    int srcUnit, const void *attr, void *callback,
                                                    void *callBackData);
typedef int (__stdcall *PtrGdipDrawLineI) (QtGpGraphics *, QtGpPen *, int x1, int y1, int x2, int y2);
typedef int (__stdcall *PtrGdipDrawPath) (QtGpGraphics *, QtGpPen *, QtGpPath *);
typedef int (__stdcall *PtrGdipDrawPolygonI) (QtGpGraphics *, QtGpPen *, const QPoint *, int);


typedef int (__stdcall *PtrGdipCreateMatrix2) (float, float, float, float, float, float, QtGpMatrix **);
typedef int (__stdcall *PtrGdipDeleteMatrix) (QtGpMatrix *);

typedef int (__stdcall *PtrGdipCreateRegionHrgn) (HRGN hRgn, QtGpRegion **);
typedef int (__stdcall *PtrGdipDeleteRegion) (QtGpRegion *);

typedef int (__stdcall *PtrGdipDeleteBrush) (QtGpBrush *);

typedef int (__stdcall *PtrGdipCreateSolidFill) (Q_UINT32 argb, QtGpBrush **);
typedef int (__stdcall *PtrGdipSetSolidFillColor) (QtGpSolidFill *, Q_UINT32 argb);

typedef int (__stdcall *PtrGdipCreatePen1) (Q_UINT32 argb, float width, int unit, QtGpPen **);
typedef int (__stdcall *PtrGdipDeletePen) (QtGpPen *);
typedef int (__stdcall *PtrGdipSetPenWidth) (QtGpPen *, float width);
typedef int (__stdcall *PtrGdipSetPenColor) (QtGpPen *, Q_UINT32 argb);
typedef int (__stdcall *PtrGdipSetPenDashStyle) (QtGpPen *, int dashStyle);

typedef int (__stdcall *PtrGdipCreatePath) (int fillMode, QtGpPath **path);
typedef int (__stdcall *PtrGdipDeletePath) (QtGpPath *path);
typedef int (__stdcall *PtrGdipAddPathLine) (QtGpPath *path, float x1, float y1, float x2, float y2);
typedef int (__stdcall *PtrGdipAddPathArc) (QtGpPath *path, float x, float y, float w, float h,
                                            float startAngle, float sweepAngle);
typedef int (__stdcall *PtrGdipAddPathBezierI)(QtGpPath *path, int, int, int, int, int, int, int, int);
typedef int (__stdcall *PtrGdipClosePathFigure) (QtGpPath *path);

typedef int (__stdcall *PtrGdipCreateBitmapFromHBITMAP)(HBITMAP, HPALETTE, QtGpBitmap **);
typedef int (__stdcall *PtrGdipCreateBitmapFromScan0)(int w, int h, int stride, int format,
                                                      BYTE *scan0, QtGpBitmap **);
typedef int (__stdcall *PtrGdipGetImageGraphicsContext)(QtGpImage *, QtGpGraphics **);
typedef int (__stdcall *PtrGdipGetImageWidth)(QtGpImage *, uint *);
typedef int (__stdcall *PtrGdipGetImageHeight)(QtGpImage *, uint *);
typedef int (__stdcall *PtrGdipDisposeImage)(QtGpImage *);
}

static PtrGdiplusStartup GdiplusStartup = 0;                 // GdiplusStartup
static PtrGdiplusShutdown GdiplusShutdown = 0;               // GdiplusStartup

static PtrGdipCreateFromHDC GdipCreateFromHDC = 0;           // Graphics::Graphics(hdc)
static PtrGdipDeleteGraphics GdipDeleteGraphics = 0;         // Graphics::~Graphics()
static PtrGdipDrawEllipseI GdipDrawEllipseI = 0;             // Graphics::DrawEllipse(pen, x, y, w, h)
static PtrGdipDrawLineI GdipDrawLineI = 0;                   // Graphics::DrawLine(pen, x1, y1, x2, y2)
static PtrGdipDrawPath GdipDrawPath = 0;                     // Graphics::DrawPath(pen, path);
static PtrGdipDrawPolygonI GdipDrawPolygonI = 0;
static PtrGdipDrawRectangleI GdipDrawRectangleI = 0;         // Graphics::DrawRectangle(brush,x,y,w,h)
static PtrGdipDrawImageRectRectI GdipDrawImageRectRectI = 0; // Graphics::DrawImage(image, r, sr);
static PtrGdipFillEllipseI GdipFillEllipseI = 0;             // Graphics::FillEllipse(brush, x, y, w, h)
static PtrGdipFillPath GdipFillPath = 0;                     // Graphics::FillPath(brush, path)
static PtrGdipFillPolygonI GdipFillPolygonI = 0;
static PtrGdipFillRectangleI GdipFillRectangleI = 0;         // Graphics::FillRectangle(brush,x,y,w,h)
static PtrGdipResetClip GdipResetClip = 0;                   // Graphics::ResetClip()
static PtrGdipSetClipRegion GdipSetClipRegion = 0;           // Graphics::SetClipRegion(region)
static PtrGdipSetSmoothingMode GdipSetSmoothingMode = 0;     // Graphics::SetSmoothingMode(mode)
static PtrGdipSetTransform GdipSetTransform = 0;             // Graphics::SetTransform(matrix)

static PtrGdipCreateMatrix2 GdipCreateMatrix2 = 0;             // Matrix::Matrix(a, b, c, d, dx, dy)
static PtrGdipDeleteMatrix GdipDeleteMatrix = 0;             // Matrix::~Matrix()

static PtrGdipCreateRegionHrgn GdipCreateRegionHrgn = 0;     // Region::Region(hRgn)
static PtrGdipDeleteRegion GdipDeleteRegion = 0;             // Region::~Region()

static PtrGdipDeleteBrush GdipDeleteBrush = 0;               // Brush::~Brush()

static PtrGdipCreateSolidFill GdipCreateSolidFill = 0;       // SolidBrush::SolidBrush(argb)
static PtrGdipSetSolidFillColor GdipSetSolidFillColor = 0;   // SolidBrush::SetColor(argb)

static PtrGdipCreatePen1 GdipCreatePen1 = 0;                 // Pen::Pen(color, width)
static PtrGdipDeletePen GdipDeletePen = 0;                   // Pen::~Pen()
static PtrGdipSetPenWidth GdipSetPenWidth = 0;               // Pen::SetWidth(width)
static PtrGdipSetPenColor GdipSetPenColor = 0;               // Pen::SetColor(argb)
static PtrGdipSetPenDashStyle GdipSetPenDashStyle = 0;       // Pen::SetDashStyle(style)

static PtrGdipCreatePath GdipCreatePath = 0;                 // Path::Path(fillMode)
static PtrGdipDeletePath GdipDeletePath = 0;                 // Path::~Path()
static PtrGdipAddPathLine GdipAddPathLine = 0;               // Path::AddLine(x1, y1, x2, y2)
static PtrGdipAddPathArc GdipAddPathArc = 0;                 // Path::AddArc(x, y, w, h, start, sweep)
static PtrGdipAddPathBezierI GdipAddPathBezierI = 0;         // Path::AddPathBezier(x1, y1, ... x4, y4)
static PtrGdipClosePathFigure GdipClosePathFigure = 0;       // Path::CloseFigure()

static PtrGdipCreateBitmapFromHBITMAP GdipCreateBitmapFromHBITMAP = 0;  // Bitmap::Bitmap(hbm, hpalette)
static PtrGdipCreateBitmapFromScan0 GdipCreateBitmapFromScan0 = 0;      // Bitmap::Bitmap(w, h .. bits)
static PtrGdipGetImageGraphicsContext GdipGetImageGraphicsContext = 0;  // Graphics Image.getContext();
static PtrGdipGetImageWidth GdipGetImageWidth = 0;
static PtrGdipGetImageHeight GdipGetImageHeight = 0;
static PtrGdipDisposeImage GdipDisposeImage = 0;                        // Image::~Image()

static void qt_resolve_gdiplus()
{
    if (qt_resolved_gdiplus)
        return;

    if (getenv("QT_FORCE_GDI")) {
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
    GdipSetTransform             = (PtrGdipSetTransform)       lib.resolve("GdipSetWorldTransform");
    GdipSetClipRegion            = (PtrGdipSetClipRegion)      lib.resolve("GdipSetClipRegion");
    GdipResetClip                = (PtrGdipResetClip)          lib.resolve("GdipResetClip");
    GdipSetSmoothingMode         = (PtrGdipSetSmoothingMode)   lib.resolve("GdipSetSmoothingMode");
    GdipFillEllipseI             = (PtrGdipFillEllipseI)       lib.resolve("GdipFillEllipseI");
    GdipFillPath                 = (PtrGdipFillPath)           lib.resolve("GdipFillPath");
    GdipFillPolygonI             = (PtrGdipFillPolygonI)       lib.resolve("GdipFillPolygonI");
    GdipFillRectangleI           = (PtrGdipFillRectangleI)     lib.resolve("GdipFillRectangleI");
    GdipDrawEllipseI             = (PtrGdipDrawEllipseI)       lib.resolve("GdipDrawEllipseI");
    GdipDrawLineI                = (PtrGdipDrawLineI)          lib.resolve("GdipDrawLineI");
    GdipDrawPath                 = (PtrGdipDrawPath)           lib.resolve("GdipDrawPath");
    GdipDrawPolygonI             = (PtrGdipDrawPolygonI)       lib.resolve("GdipDrawPolygonI");
    GdipDrawRectangleI           = (PtrGdipDrawRectangleI)     lib.resolve("GdipDrawRectangleI");
    GdipDrawImageRectRectI       = (PtrGdipDrawImageRectRectI) lib.resolve("GdipDrawImageRectRectI");

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
    GdipAddPathBezierI           = (PtrGdipAddPathBezierI)     lib.resolve("GdipAddPathBezierI");
    GdipClosePathFigure          = (PtrGdipClosePathFigure)    lib.resolve("GdipClosePathFigure");

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

    Q_ASSERT(GdiplusStartup);
    Q_ASSERT(GdiplusShutdown);
    Q_ASSERT(GdipCreateFromHDC);
    Q_ASSERT(GdipDeleteGraphics);
    Q_ASSERT(GdipSetTransform);
    Q_ASSERT(GdipSetClipRegion);
    Q_ASSERT(GdipResetClip);
    Q_ASSERT(GdipSetSmoothingMode);
    Q_ASSERT(GdipFillEllipseI);
    Q_ASSERT(GdipFillPath);
    Q_ASSERT(GdipFillPolygonI);
    Q_ASSERT(GdipFillRectangleI);
    Q_ASSERT(GdipDrawEllipseI);
    Q_ASSERT(GdipDrawImageRectRectI);
    Q_ASSERT(GdipDrawLineI);
    Q_ASSERT(GdipDrawPath);
    Q_ASSERT(GdipDrawPolygonI);
    Q_ASSERT(GdipDrawRectangleI);
    Q_ASSERT(GdipCreateMatrix2);
    Q_ASSERT(GdipDeleteMatrix);
    Q_ASSERT(GdipCreateRegionHrgn);
    Q_ASSERT(GdipDeleteRegion);
    Q_ASSERT(GdipDeleteBrush);
    Q_ASSERT(GdipCreateSolidFill);
    Q_ASSERT(GdipSetSolidFillColor);
    Q_ASSERT(GdipCreatePen1);
    Q_ASSERT(GdipDeletePen);
    Q_ASSERT(GdipSetPenWidth);
    Q_ASSERT(GdipSetPenColor);
    Q_ASSERT(GdipSetPenDashStyle);
    Q_ASSERT(GdipCreatePath);
    Q_ASSERT(GdipDeletePath);
    Q_ASSERT(GdipAddPathLine);
    Q_ASSERT(GdipAddPathArc);
    Q_ASSERT(GdipAddPathBezierI);
    Q_ASSERT(GdipClosePathFigure);
    Q_ASSERT(GdipCreateBitmapFromHBITMAP);
    Q_ASSERT(GdipCreateBitmapFromScan0);
    Q_ASSERT(GdipGetImageGraphicsContext);
    Q_ASSERT(GdipGetImageWidth);
    Q_ASSERT(GdipGetImageHeight);
    Q_ASSERT(GdipDisposeImage);

    QGdiplusPaintEngine::initialize();
    qt_gdiplus_support = true;
}


// Based on enums in GdiPlusEnums.h
static const int qt_penstyle_map[] = {
    { -1 },     // Qt::NoPen
    { 0 },      // Qt::SolidLine
    { 1 },      // Qt::DashLine
    { 2 },      // Qt::DotLine
    { 3 },      // Qt::DashDotLine
    { 4 }       // Qt::DashDotDotLine
};

static const int qt_hatchstyle_map[] = {
    { -1 },     // Qt::NoBrush
    { -1 },     // Qt::SolidPattern
    { 17 },     // Qt::Dense1Pattern, hatch 90
    { 15 },     // Qt::Dense2Pattern, hatch 75
    { 13 },     // Qt::Dense3Pattern, hatch 60
    { 12 },     // Qt::Dense4Pattern, hatch 50
    { 10 },     // Qt::Dense5Pattern, hatch 30
    { 8 },      // Qt::Dense6Pattern, hatch 20
    { 6 },      // Qt::Dense7Pattern, hatch 05
    { 0 },      // Qt::HorPattern
    { 1 },      // Qt::VerPattern
    { 4 },      // Qt::CrossPattern
    { 3 },      // Qt::BDiagPattern
    { 2 },      // Qt::FDiagPattern
    { 5 }       // Qt::DiagCrossPattern
};

static QtGpBitmap *qt_convert_to_gdipbitmap(const QPixmap *pixmap, QImage *ref = 0);

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
    if (pdev->devType() == QInternal::Widget) {
        d->hdc = static_cast<QWidget *>(pdev)->winHDC();
        Q_ASSERT(d->hdc);
        //     d->graphics = new Graphis(hdc);
        GdipCreateFromHDC(d->hdc, &d->graphics);
    } else if (pdev->devType() == QInternal::Pixmap) {
        d->hdc = static_cast<QPixmap *>(pdev)->winHDC();
        GdipCreateFromHDC(d->hdc, &d->graphics);
    } else {
        qDebug() << "QGdiplusPaintEngine::begin(), unsupported paint device..." << pdev->devType();
    }

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

    return true;
}

void QGdiplusPaintEngine::updatePen(const QPen &pen)
{
//     d->pen->SetWidth(ps->pen.width());
//     d->pen->SetColor(conv(ps->pen.color()));
    int status;
    status = GdipSetPenWidth(d->pen, pen.width());
    Q_ASSERT(status == 0);
    status = GdipSetPenColor(d->pen, pen.color().rgb());
    Q_ASSERT(status == 0);

    Qt::PenStyle style = pen.style();
    if (style == Qt::NoPen) {
        d->usePen = false;
    } else {
        Q_ASSERT(style >= 0 && style < 5);
        d->usePen = true;
//         d->pen->SetDashStyle(qt_penstyle_map[style]);
        GdipSetPenDashStyle(d->pen, qt_penstyle_map[style]);
    }
}

void QGdiplusPaintEngine::updateBrush(const QBrush &brush, const QPoint &)
{
    QColor c = brush.color();
    if (d->temporaryBrush) {
        d->temporaryBrush = false;
//         delete d->brush;
        GdipDeleteBrush(d->brush);
    }

    switch (brush.style()) {
    case Qt::NoBrush:
        d->brush = 0;
        break;
    case Qt::SolidPattern:
        if (!d->cachedSolidBrush) {
//             d->cachedSolidBrush = new SolidBrush(conv(brush.color()));
            GdipCreateSolidFill(brush.color().rgb(), (QtGpBrush**)(&d->cachedSolidBrush));
            d->brush = d->cachedSolidBrush;
        } else {
//             d->cachedSolidBrush->SetColor(conv(brush.color()));
            GdipSetSolidFillColor(d->cachedSolidBrush, brush.color().rgb());
            d->brush = d->cachedSolidBrush;
        }
        break;
    case Qt::LinearGradientPattern: {
//         QBrush &b = brush;
//         d->brush = new LinearGradientBrush(conv(b.gradientStart()), conv(b.gradientStop()),
//                                            conv(b.color()), conv(b.gradientColor()));
//         d->temporaryBrush = true;
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

void QGdiplusPaintEngine::updateXForm(const QWMatrix &qm)
{
//     Matrix m(qm.m11(), qm.m12(), qm.m21(), qm.m22(), qm.dx(), qm.dy());
//     d->graphics->SetTransform(&m);
    QtGpMatrix *m = 0;
    GdipCreateMatrix2(qm.m11(), qm.m12(), qm.m21(), qm.m22(), qm.dx(), qm.dy(), &m);
    GdipSetTransform(d->graphics, m);
    GdipDeleteMatrix(m);
}

void QGdiplusPaintEngine::updateClipRegion(const QRegion &qtClip, bool enabled)
{
    if (enabled) {
//         Region r(ps->clipRegion.handle());
//         d->graphics->SetClip(&r, CombineModeReplace);
        QtGpRegion *region = 0;
        GdipCreateRegionHrgn(qtClip.handle(), &region);
        GdipSetClipRegion(d->graphics, region, 0);
        GdipDeleteRegion(region);
    } else {
//         d->graphics->ResetClip();
        GdipResetClip(d->graphics);
    }
}

void QGdiplusPaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    if (d->usePen) {
//         d->graphics->DrawLine(d->pen, p1.x(), p1.y(), p2.x(), p2.y());
        GdipDrawLineI(d->graphics, d->pen, p1.x(), p1.y(), p2.x(), p2.y());
    }
}

#define QT_GDIPLUS_SUBTRACT (d->usePen || ((d->renderhints & QPainter::LineAntialiasing) != 0) ? 1 : 0);

void QGdiplusPaintEngine::drawRect(const QRect &r)
{
    int subtract = QT_GDIPLUS_SUBTRACT;
    if (d->brush) {
//         d->graphics->FillRectangle(d->brush, r.x(), r.y(),
//                                    r.width()-subtract, r.height()-subtract);
        GdipFillRectangleI(d->graphics, d->brush, r.x(), r.y(), r.width()-subtract, r.height()-subtract);
    }
    if (d->usePen) {
//         d->graphics->DrawRectangle(d->pen, r.x(), r.y(),
//                                    r.width()-subtract, r.height()-subtract);
        GdipDrawRectangleI(d->graphics, d->pen, r.x(), r.y(), r.width()-subtract, r.height()-subtract);
    }
}

void QGdiplusPaintEngine::drawPoint(const QPoint &p)
{
    if (d->usePen)
        GdipDrawRectangleI(d->graphics, d->pen, p.x(), p.y(), 0, 0);
}

void QGdiplusPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{
    if (d->usePen)
        for (int i=0; i<npoints; ++i)
            GdipDrawRectangleI(d->graphics, d->pen, pa[index+i].x(), pa[index+i].y(), 0, 0);
}

void QGdiplusPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
//     GraphicsPath path(FillModeAlternate);
    QtGpPath *path = 0;
    GdipCreatePath(0, &path);

    int subtract = d->usePen ? 1 : 0;

    int top = r.y();
    int bottom = r.y() + r.height() - subtract;
    int left = r.x();
    int right = r.x() + r.width() - subtract;

    int horLength = (99 - xRnd) / 99.0 * r.width() / 1;
    int horStart  = r.x() + r.width() / 2 - horLength / 2;
    int horEnd = horStart + horLength;
    int arcWidth = r.width() - horLength;

    int verLength = (99 - yRnd) / 99.0 * r.height() / 1;
    int verStart  = r.y() + r.height() / 2 - verLength / 2;
    int verEnd = verStart + verLength;
    int arcHeight = r.width() - horLength;

//     path.AddLine(horStart, r.y(), horEnd, r.y());
//     path.AddArc(right - arcWidth, top, arcWidth, arcHeight, 270, 90);
//     path.AddLine(right, verStart, right, verEnd);
//     path.AddArc(right - arcWidth, bottom - arcHeight, arcWidth, arcHeight, 0, 90);
//     path.AddLine(horEnd, bottom, horStart, bottom);
//     path.AddArc(left, bottom - arcHeight, arcWidth, arcHeight, 90, 90);
//     path.AddLine(left, verEnd, left, verStart);
//     path.AddArc(left, top, arcWidth, arcHeight, 180, 90);
//     path.CloseFigure();

    GdipAddPathLine(path, horStart, r.y(), horEnd, r.y());
    GdipAddPathArc(path, right - arcWidth, top, arcWidth, arcHeight, 270, 90);
    GdipAddPathLine(path, right, verStart, right, verEnd);
    GdipAddPathArc(path, right - arcWidth, bottom - arcHeight, arcWidth, arcHeight, 0, 90);
    GdipAddPathLine(path, horEnd, bottom, horStart, bottom);
    GdipAddPathArc(path, left, bottom - arcHeight, arcWidth, arcHeight, 90, 90);
    GdipAddPathLine(path, left, verEnd, left, verStart);
    GdipAddPathArc(path, left, top, arcWidth, arcHeight, 180, 90);
    GdipClosePathFigure(path);

    if (d->brush) {
//         d->graphics->FillPath(d->brush, &path);
        GdipFillPath(d->graphics, d->brush, path);
    }
    if (d->usePen) {
//         d->graphics->DrawPath(d->pen, &path);
        GdipDrawPath(d->graphics, d->pen, path);
    }

    GdipDeletePath(path);
}

void QGdiplusPaintEngine::drawEllipse(const QRect &r)
{
    int subtract = QT_GDIPLUS_SUBTRACT;
    if (d->brush) {
//         d->graphics->FillEllipse(d->brush, r.x(), r.y(), r.width()-subtract, r.height()-subtract);
        GdipFillEllipseI(d->graphics, d->brush, r.x(), r.y(),
                         r.width()-subtract, r.height()-subtract);
    }
    if (d->usePen) {
//         d->graphics->DrawEllipse(d->pen, r.x(), r.y(), r.width()-subtract, r.height()-subtract);
        GdipDrawEllipseI(d->graphics, d->pen, r.x(), r.y(),
                         r.width()-subtract, r.height()-subtract);
    }
}

void QGdiplusPaintEngine::drawArc(const QRect &r, int a, int alen)
{
    Q_UNUSED(r);
    Q_UNUSED(a);
    Q_UNUSED(alen);
//     int subtract = d->usePen ? 1 : 0;
//     if (d->usePen) {
//         d->graphics->DrawArc(d->pen, r.x(), r.y(),
//                              r.width()-subtract, r.height()-subtract,
//                              -a/16.0, -alen/16.0);
//     }
}

void QGdiplusPaintEngine::drawPie(const QRect &r, int a, int alen)
{
    Q_UNUSED(r);
    Q_UNUSED(a);
    Q_UNUSED(alen);
//     int subtract = d->usePen ? 1 : 0;
//     if (d->brush) {
//         d->graphics->FillPie(d->brush, r.x(), r.y(),
//                              r.width()-subtract, r.height()-subtract,
//                              -a/16.0, -alen/16.0);
//     }
//     if (d->usePen) {
//         d->graphics->DrawPie(d->pen, r.x(), r.y(),
//                              r.width()-subtract, r.height()-subtract,
//                              -a/16.0, -alen/16.0);
//     }
}

void QGdiplusPaintEngine::drawChord(const QRect &r, int a, int alen)
{
    Q_UNUSED(r);
    Q_UNUSED(a);
    Q_UNUSED(alen);
//     GraphicsPath path(FillModeAlternate);
//     int subtract = d->usePen ? 1 : 0;
//     path.AddArc(r.x(), r.y(), r.width()-subtract, r.height()-subtract, -a/16.0, -alen/16.0);
//     path.CloseFigure();
//     if (d->brush)
//         d->graphics->FillPath(d->brush, &path);
//     if (d->usePen)
//         d->graphics->DrawPath(d->pen, &path);
}

void QGdiplusPaintEngine::drawLineSegments(const QPointArray &pa, int index, int nlines)
{
    Q_UNUSED(pa);
    Q_UNUSED(index);
    Q_UNUSED(nlines);
//     if (d->usePen) {
//         GraphicsPath path;
//         for (int i=0; i<nlines*2; i+=2) {
//             path.AddLine(pa[index+i].x(), pa[index+i].y(), pa[index+i+1].x(), pa[index+i+1].y());
//             path.CloseFigure();
//         }
//         d->graphics->DrawPath(d->pen, &path);
//     }
}

void QGdiplusPaintEngine::drawPolyline(const QPointArray &pa, int index, int npoints)
{
    Q_UNUSED(pa);
    Q_UNUSED(index);
    Q_UNUSED(npoints);
    if (d->usePen) {
//         GraphicsPath path;
//         for (int i=1; i<npoints; ++i)
//             path.AddLine(pa.at(index+i-1).x(), pa.at(index+i-1).y(),
//                          pa.at(index+i).x(), pa.at(index+i).y());
//         d->graphics->DrawPath(d->pen, &path);
        QtGpPath *path = 0;
        GdipCreatePath(0, &path);
        for (int i=1; i<npoints; ++i) {
            GdipAddPathLine(path,
                            pa.at(index+i-1).x(), pa.at(index+i-1).y(),
                            pa.at(index+i).x(), pa.at(index+i).y());
        }
        GdipDrawPath(d->graphics, d->pen, path);
        GdipDeletePath(path);
    }
}

void QGdiplusPaintEngine::drawPolygon(const QPointArray &pa, bool winding, int index, int npoints)
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
    if (d->usePen || d->brush) {
        if (d->brush) {
            GdipFillPolygonI(d->graphics, d->brush, pa.data()+index, npoints,
                             winding
                             ? 1 // FillModeWinding
                             : 0 // FillModeAlternate
                             );
        }
        if (d->usePen)
            GdipDrawPolygonI(d->graphics, d->pen, pa.data()+index, npoints);
    }
}

void QGdiplusPaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    drawPolygon(pa, true, index, npoints);
}



void QGdiplusPaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr,
                                     Qt::PixmapDrawingMode mode)
{
    Q_UNUSED(mode);
    if (pm.isQBitmap()) {
        const QBitmap *bitmap = static_cast<const QBitmap *>(&pm);
        QRegion rgn(*bitmap);
        QPainter *p = painter();
        p->save();
        p->translate(r.topLeft());
        p->setClipRegion(QRegion(*bitmap));
        p->fillRect(0, 0, r.width(), r.height(), p->pen().color());
        p->restore();
        return;
    }

    QImage backupPixels;
    QtGpBitmap *bitmap = qt_convert_to_gdipbitmap(&pm, &backupPixels);
    GdipDrawImageRectRectI(d->graphics, bitmap,
                           r.x(), r.y(), r.width(), r.height(),
                           sr.x(), sr.y(), sr.width(), sr.height(),
                           2, // UnitPixel
                           0, 0, 0);
    GdipDisposeImage(bitmap);
}

void QGdiplusPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pm,
                                          const QPoint &, Qt::PixmapDrawingMode)
{
    Q_UNUSED(r);
    Q_UNUSED(pm);
//     QImage image = pm.convertToImage();
//     Q_ASSERT(image.depth() == 32);
//     Bitmap bitmap(pm.width(), pm.height(), image.bytesPerLine(), PixelFormat32bppARGB,
//                   image.bits());
//     TextureBrush texture(&bitmap, WrapModeTile);
//     texture.TranslateTransform(r.x(), r.y());
//     int subtract = d->usePen ? 1 : 0;
//     d->graphics->FillRectangle(&texture, r.x(), r.y(), r.width()-subtract, r.height()-subtract);
}

#ifndef QT_NO_BEZIER
void QGdiplusPaintEngine::drawCubicBezier(const QPointArray &pa, int index)
{
    Q_UNUSED(pa);
    Q_UNUSED(index);
//     if (d->usePen) {
//         Point *p = new Point[pa.size()];
//         for (int i=0; i<pa.size() - index; ++i) {
//             p[i] = Point(pa[i+index].x(), pa[i+index].y());
//         }
//         if (d->usePen)
//             d->graphics->DrawCurve(d->pen, p, pa.size());
//         delete [] p;
//     }
}
#endif

void QGdiplusPaintEngine::drawPath(const QPainterPath &p)
{
    QtGpPath *path = 0;
    GdipCreatePath(0, &path);

    const QPainterPathPrivate *pd = p.d;

    // Drawing the subpaths
    for (int i=0; i<pd->subpaths.size(); ++i) {
        const QPainterSubpath &sub = pd->subpaths.at(i);
        if (sub.elements.isEmpty())
            continue;
        for (int j=0; j<sub.elements.size(); ++j) {
            const QPainterPathElement &elm = sub.elements.at(j);
            switch (elm.type) {
            case QPainterPathElement::Line: {
                GdipAddPathLine(path,
                                elm.lineData.x1, elm.lineData.y1,
                                elm.lineData.x2, elm.lineData.y2);
                break;
            }
            case QPainterPathElement::Bezier: {
                GdipAddPathBezierI(path,
                                   elm.bezierData.x1, elm.bezierData.y1,
                                   elm.bezierData.x2, elm.bezierData.y2,
                                   elm.bezierData.x3, elm.bezierData.y3,
                                   elm.bezierData.x4, elm.bezierData.y4);
                break;
            }
            case QPainterPathElement::Arc: {
                GdipAddPathArc(path,
                               elm.arcData.x, elm.arcData.y,
                               elm.arcData.w, elm.arcData.h,
                               elm.arcData.start/16.0, elm.arcData.length/16.0);
                break;
            }
            default:
                qFatal("QWin32PaintEngine::drawPath(), unhandled subpath type: %d", elm.type);
            }
        }
        GdipClosePathFigure(path);
    }

    if (d->brush)
        GdipFillPath(d->graphics, d->brush, path);
    if (d->usePen)
        GdipDrawPath(d->graphics, d->pen, path);

    GdipDeletePath(path);
}

void QGdiplusPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    GdipSetSmoothingMode(d->graphics, hints & QPainter::LineAntialiasing ? 2 : 1 );

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

static QtGpBitmap *qt_convert_to_gdipbitmap(const QPixmap *pixmap, QImage *image)
{
    QtGpBitmap *bitmap = 0;
    if (!pixmap->hasAlpha()) {
        GdipCreateBitmapFromHBITMAP(pixmap->hbm(), HPALETTE(0), &bitmap);
        return bitmap;
    } else { // 1 bit masks or 8 bit alpha...
        *image = pixmap->convertToImage();
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

/****************************************************************************
**
** Definition of QX11PaintEngine class.
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

#include "qplatformdefs.h"
#include "qpaintengine_x11.h"

#include "qfont.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qtextcodec.h"
#include "qpaintdevicemetrics.h"
#include "qpaintdevice.h" // tmp
#include "qpainter.h" // tmp
#include "qcoreevent.h"

#include "qpainter_p.h"
#include <qtextlayout.h>
#include <qvarlengtharray.h>
#include <private/qfontdata_p.h>
#include <private/qfontengine_p.h>
#include <private/qtextengine_p.h>

#include "qpen.h"
#include "qcolor.h"
#include "qfont.h"

#include "qmath_p.h"

#include <private/qpaintengine_p.h>
#include "qpaintengine_x11_p.h"

// #define GC_CACHE_STAT
#if defined(GC_CACHE_STAT)
#include "qtextstream.h"
#include "qbuffer.h"
#endif

#include <private/qt_x11_p.h>

#define d d_func()
#define q q_func()

extern Drawable qt_x11Handle(const QPaintDevice *pd);
extern QX11Info *qt_x11Info(const QPaintDevice *pd);

// paintevent magic to provide Windows semantics on X11
static QRegion* paintEventClipRegion = 0;
static QPaintDevice* paintEventDevice = 0;

void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region)
{
    if (!paintEventClipRegion)
        paintEventClipRegion = new QRegion(region);
    else
        *paintEventClipRegion = region;
    paintEventDevice = dev;
}

void qt_intersect_paintevent_clipping(QPaintDevice *dev, QRegion &region)
{
    if (dev == paintEventDevice && paintEventClipRegion)
        region &= *paintEventClipRegion;
}

void qt_clear_paintevent_clipping()
{
    delete paintEventClipRegion;
    paintEventClipRegion = 0;
    paintEventDevice = 0;
}

// hack, so we don't have to make QRegion::clipRectangles() public or include
// X11 headers in qregion.h
inline void *qt_getClipRects(const QRegion &r, int &num)
{
    return r.clipRectangles(num);
}

static inline void x11SetClipRegion(Display *dpy, GC gc, GC gc2, Qt::HANDLE draw, const QRegion &r)
{
    int num;
    XRectangle *rects = (XRectangle *)qt_getClipRects(r, num);

    if (gc)
        XSetClipRectangles( dpy, gc, 0, 0, rects, num, YXBanded );
    if (gc2)
        XSetClipRectangles( dpy, gc2, 0, 0, rects, num, YXBanded );

#ifndef QT_NO_XFT
    if (draw)
        XftDrawSetClipRectangles((XftDraw *) draw, 0, 0, rects, num);
#else
    Q_UNUSED(draw);
#endif // QT_NO_XFT
}

static inline void x11ClearClipRegion(Display *dpy, GC gc, GC gc2, Qt::HANDLE draw)
{
    if (gc)
        XSetClipMask(dpy, gc, XNone);
    if (gc2)
        XSetClipMask(dpy, gc2, XNone);

#ifndef QT_NO_XFT
    if (draw)
        XftDrawSetClip((XftDraw *) draw, 0);
#else
    Q_UNUSED(draw);
#endif // QT_NO_XFT
}

/*
 * GC cache stuff
 *
 */

/*****************************************************************************
  QPainter internal GC (Graphics Context) allocator.

  The GC allocator offers two functions; alloc_gc() and free_gc() that
  reuse GC objects instead of calling XCreateGC() and XFreeGC(), which
  are a whole lot slower.
 *****************************************************************************/

struct QGC
{
    GC   gc;
    char in_use;
    bool mono;
    int scrn;
};

const  int  gc_array_size = 256;
static QGC  gc_array[gc_array_size];            // array of GCs
static bool gc_array_init = false;


static void init_gc_array()
{
    if (!gc_array_init) {
        memset(gc_array, 0, gc_array_size*sizeof(QGC));
        gc_array_init = true;
    }
}

static void cleanup_gc_array(Display *dpy)
{
    register QGC *p = gc_array;
    int i = gc_array_size;
    if (gc_array_init) {
        while (i--) {
            if (p->gc)                        // destroy GC
                XFreeGC(dpy, p->gc);
            p++;
        }
        gc_array_init = false;
    }
}

// #define DONT_USE_GC_ARRAY

static GC alloc_gc(Display *dpy, int scrn, Drawable hd, bool monochrome=false,
                    bool privateGC = false)
{
#if defined(DONT_USE_GC_ARRAY)
    privateGC = true;                           // will be slower
#endif
    if (privateGC) {
        GC gc = XCreateGC(dpy, hd, 0, 0);
        XSetGraphicsExposures(dpy, gc, False);
        return gc;
    }
    register QGC *p = gc_array;
    int i = gc_array_size;
    if (!gc_array_init)                       // not initialized
        init_gc_array();
    while (i--) {
        if (!p->gc) {                         // create GC (once)
            p->gc = XCreateGC(dpy, hd, 0, 0);
            p->scrn = scrn;
            XSetGraphicsExposures(dpy, p->gc, False);
            p->in_use = false;
            p->mono   = monochrome;
        }
        if (!p->in_use && p->mono == monochrome && p->scrn == scrn) {
            p->in_use = true;                   // available/compatible GC
            return p->gc;
        }
        p++;
    }
    qWarning("QPainter: Internal error; no available GC");
    GC gc = XCreateGC(dpy, hd, 0, 0);
    XSetGraphicsExposures(dpy, gc, False);
    return gc;
}

static void free_gc(Display *dpy, GC gc, bool privateGC = false)
{
#if defined(DONT_USE_GC_ARRAY)
    privateGC = true;                           // will be slower
#endif
    if (privateGC) {
        Q_ASSERT(dpy != 0);
        XFreeGC(dpy, gc);
        return;
    }
    register QGC *p = gc_array;
    int i = gc_array_size;
    if (gc_array_init) {
        while (i--) {
            if (p->gc == gc) {
                p->in_use = false;              // set available
                XSetClipMask(dpy, gc, XNone);  // make it reusable
                XSetFunction(dpy, gc, GXcopy);
                XSetFillStyle(dpy, gc, FillSolid);
                XSetTSOrigin(dpy, gc, 0, 0);
                return;
            }
            p++;
        }
    }
}



/*****************************************************************************
  QPainter internal GC (Graphics Context) cache for solid pens and
  brushes.

  The GC cache makes a significant contribution to speeding up
  drawing.  Setting new pen and brush colors will make the painter
  look for another GC with the same color instead of changing the
  color value of the GC currently in use. The cache structure is
  optimized for fast lookup.  Only solid line pens with line width 0
  and solid brushes are cached.

  In addition, stored GCs may have an implicit clipping region
  set. This prevents any drawing outside paint events. Both
  updatePen() and updateBrush() keep track of the validity of this
  clipping region by storing the clip_serial number in the cache.

*****************************************************************************/

struct QGCC                                     // cached GC
{
    GC gc;
    uint pix;
    int count;
    int hits;
    uint clip_serial;
    int scrn;
};

const  int   gc_cache_size = 29;                // multiply by 4
static QGCC *gc_cache_buf;
static QGCC *gc_cache[4*gc_cache_size];
static bool  gc_cache_init;
static uint gc_cache_clip_serial;


static void init_gc_cache()
{
    if (!gc_cache_init) {
        gc_cache_init = true;
        gc_cache_clip_serial = 0;
        QGCC *g = gc_cache_buf = new QGCC[4*gc_cache_size];
        memset(g, 0, 4*gc_cache_size*sizeof(QGCC));
        for (int i=0; i<4*gc_cache_size; i++)
            gc_cache[i] = g++;
    }
}


#if defined(GC_CACHE_STAT)
static int g_numhits    = 0;
static int g_numcreates = 0;
static int g_numfaults  = 0;
#endif


static void cleanup_gc_cache()
{
    if (!gc_cache_init)
        return;
#if defined(GC_CACHE_STAT)
    qDebug("Number of cache hits = %d", g_numhits);
    qDebug("Number of cache creates = %d", g_numcreates);
    qDebug("Number of cache faults = %d", g_numfaults);
    for (int i=0; i<gc_cache_size; i++) {
        QByteArray    str;
        QBuffer     buf(str);
        buf.open(IO_ReadWrite);
        QTextStream s(&buf);
        s << i << ": ";
        for (int j=0; j<4; j++) {
            QGCC *g = gc_cache[i*4+j];
            s << (g->gc ? 'X' : '-') << ',' << g->hits << ','
              << g->count << '\t';
        }
        s << '\0';
        qDebug(str);
        buf.close();
    }
#endif
    delete [] gc_cache_buf;
    gc_cache_init = false;
}


static bool obtain_gc(void **ref, GC *gc, uint pix, Display *dpy, int scrn,
                       Qt::HANDLE hd, uint painter_clip_serial)
{
    if (!gc_cache_init)
        init_gc_cache();

    int   k = (pix % gc_cache_size) * 4;
    QGCC *g = gc_cache[k];
    QGCC *prev = 0;

#define NOMATCH (g->gc && (g->pix != pix || g->scrn != scrn || \
                 (g->clip_serial > 0 && g->clip_serial != painter_clip_serial)))

    if (NOMATCH) {
        prev = g;
        g = gc_cache[++k];
        if (NOMATCH) {
            prev = g;
            g = gc_cache[++k];
            if (NOMATCH) {
                prev = g;
                g = gc_cache[++k];
                if (NOMATCH) {
                    if (g->count == 0 && g->scrn == scrn) {    // steal this GC
                        g->pix   = pix;
                        g->count = 1;
                        g->hits  = 1;
                        g->clip_serial = 0;
                        XSetForeground(dpy, g->gc, pix);
                        XSetClipMask(dpy, g->gc, XNone);
                        gc_cache[k]   = prev;
                        gc_cache[k-1] = g;
                        *ref = (void *)g;
                        *gc = g->gc;
                        return true;
                    } else {                    // all GCs in use
#if defined(GC_CACHE_STAT)
                        g_numfaults++;
#endif
                        *ref = 0;
                        return false;
                    }
                }
            }
        }
    }

#undef NOMATCH

    *ref = (void *)g;

    if (g->gc) {                              // reuse existing GC
#if defined(GC_CACHE_STAT)
        g_numhits++;
#endif
        *gc = g->gc;
        g->count++;
        g->hits++;
        if (prev && g->hits > prev->hits) {   // maintain LRU order
            gc_cache[k]   = prev;
            gc_cache[k-1] = g;
        }
        return true;
    } else {                                    // create new GC
#if defined(GC_CACHE_STAT)
        g_numcreates++;
#endif
        g->gc = alloc_gc(dpy, scrn, hd, false);
        g->scrn = scrn;
        g->pix = pix;
        g->count = 1;
        g->hits = 1;
        g->clip_serial = 0;
        *gc = g->gc;
        return false;
    }
}

static inline void release_gc(void *ref)
{
    ((QGCC*)ref)->count--;
}

// ########
void qt_erase_background(QPaintDevice *pd, int screen,
                         int x, int y, int w, int h,
                         const QBrush &brush, int xoff, int yoff)
{
    if (brush.style() == Qt::LinearGradientPattern) {
	QPainter p(pd);
	QPoint rd;
 	QPainter::redirected(pd, &rd);
 	p.fillRect(rd.x(), rd.y(), w, h, brush);
	return;
    }

    Qt::HANDLE hd = qt_x11Handle(pd);
    Display *dpy = QX11Info::appDisplay();
    GC gc;
    void *penref = 0;
    ulong pixel = brush.color().pixel(screen);
    bool obtained = obtain_gc(&penref, &gc, pixel, dpy, screen, hd, gc_cache_clip_serial);

    if (!obtained && !penref) {
        gc = alloc_gc(dpy, screen, hd, false);
    } else {
        if (penref && ((QGCC*)penref)->clip_serial) {
            XSetClipMask(dpy, gc, XNone);
            ((QGCC*)penref)->clip_serial = 0;
        }
    }

    if (!obtained) {
        XSetForeground(dpy, gc, pixel);
    }

    if (brush.pixmap()) {
        XSetTile(dpy, gc, brush.pixmap()->handle());
        XSetFillStyle(dpy, gc, FillTiled);
        XSetTSOrigin(dpy, gc, x-xoff, y-yoff);
        XFillRectangle(dpy, hd, gc, x, y, w, h);
        XSetTSOrigin(dpy, gc, 0, 0);
        XSetFillStyle(dpy, gc, FillSolid);
    } else {
        XFillRectangle(dpy, hd, gc, x, y, w, h);
    }

    if (penref) {
        release_gc(penref);
    } else {
        free_gc(dpy, gc);
    }
}

void qt_draw_transformed_rect(QPaintEngine *pe,  int x, int y, int w,  int h, bool fill)
{
    QX11PaintEngine *p = static_cast<QX11PaintEngine *>(pe);
    QPainter *pp = p->painter();

    XPoint points[5];
    int xp = x,  yp = y;
    pp->map(xp, yp, &xp, &yp);
    points[0].x = xp;
    points[0].y = yp;
    xp = x + w; yp = y;
    pp->map(xp, yp, &xp, &yp);
    points[1].x = xp;
    points[1].y = yp;
    xp = x + w; yp = y + h;
    pp->map(xp, yp, &xp, &yp);
    points[2].x = xp;
    points[2].y = yp;
    xp = x; yp = y + h;
    pp->map(xp, yp, &xp, &yp);
    points[3].x = xp;
    points[3].y = yp;
    points[4] = points[0];

    if (fill)
        XFillPolygon(p->d->dpy, p->d->hd, p->d->gc, points, 4, Convex, CoordModeOrigin);
    else
        XDrawLines(p->d->dpy, p->d->hd, p->d->gc, points, 5, CoordModeOrigin);
}


void qt_draw_background(QPaintEngine *pe, int x, int y, int w,  int h)
{
    QX11PaintEngine *p = static_cast<QX11PaintEngine *>(pe);
    XSetForeground(p->d->dpy, p->d->gc, p->d->bg_brush.color().pixel(p->d->scrn));
    qt_draw_transformed_rect(p, x, y, w, h, true);
    XSetForeground(p->d->dpy, p->d->gc, p->d->cpen.color().pixel(p->d->scrn));
}
// ########

/*
 * QX11PaintEngine members
 */

QX11PaintEngine::QX11PaintEngine(QPaintDevice *target)
    : QPaintEngine(*(new QX11PaintEnginePrivate), DrawRects | UsesFontEngine | SolidAlphaFill)
{
    d->dpy = 0;
    d->scrn = 0;
    d->hd = 0;
    d->xft_hd = 0;
    d->xinfo = 0;
    d->pdev = target;
}

QX11PaintEngine::QX11PaintEngine(QX11PaintEnginePrivate &dptr, QPaintDevice *target)
    : QPaintEngine(dptr, DrawRects | UsesFontEngine | SolidAlphaFill)
{
    d->dpy = 0;
    d->scrn = 0;
    d->hd = 0;
    d->xft_hd = 0;
    d->xinfo = 0;
    d->pdev = target;
}

QX11PaintEngine::~QX11PaintEngine()
{
}

void QX11PaintEngine::initialize()
{
    init_gc_array();
    init_gc_cache();
}

void QX11PaintEngine::cleanup()
{
    cleanup_gc_cache();
    cleanup_gc_array(QX11Info::appDisplay());
    QPointArray::cleanBuffers();
}

bool QX11PaintEngine::begin(QPaintDevice *pdev)
{
    d->pdev = pdev;
    d->xinfo = qt_x11Info(pdev);
    d->hd = qt_x11Handle(pdev);
    if (pdev->devType() == QInternal::Widget)
        d->xft_hd = static_cast<const QWidget *>(pdev)->xftDrawHandle();
    else if (pdev->devType() == QInternal::Pixmap)
        d->xft_hd = static_cast<const QPixmap *>(pdev)->xftDrawHandle();

    Q_ASSERT(d->xinfo != 0);
    d->dpy = d->xinfo->display(); // get display variable
    d->scrn = d->xinfo->screen(); // get screen variable

    if (isActive()) {                         // already active painting
        qWarning("QX11PaintEngine::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()");
        return true;
    }

    setActive(true);

    QPixmap::x11SetDefaultScreen(d->xinfo->screen());

    assignf(IsActive | DirtyFont);
    if (d->pdev->devType() == QInternal::Pixmap)         // device is a pixmap
        static_cast<QPixmap *>(pdev)->detach(); // will modify it

    if (d->xinfo->depth() != QX11Info::appDepth(d->scrn)) { // non-standard depth
        setf(NoCache);
        setf(UsePrivateCx);
    }

    QWidget *w = d->pdev->devType() == QInternal::Widget ? static_cast<QWidget *>(d->pdev) : 0;
    if (w && w->testAttribute(Qt::WA_PaintUnclipped)) {  // paint direct on device
        setf(NoCache);
        setf(UsePrivateCx);
 	updatePen(QPen(Qt::black));
 	updateBrush(QBrush(Qt::white), QPoint());
        XSetSubwindowMode(d->dpy, d->gc, IncludeInferiors);
        XSetSubwindowMode(d->dpy, d->gc_brush, IncludeInferiors);
#ifndef QT_NO_XFT
        if (d->xft_hd)
                XftDrawSetSubwindowMode((XftDraw *) d->xft_hd, IncludeInferiors);
#endif
    } else if (d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = (QPixmap *)(pdev);
        if (!pm || pm->isNull()) {
            qWarning("QPainter::begin: Cannot paint null pixmap");
            end();
            return false;
        }
        bool mono = pm->depth() == 1;           // monochrome bitmap
        if (mono) {
            setf(MonoDev);
            // ### Port me !!!
//             ps->bgBrush = Qt::color0; // ### superhack - remove when fixed
//             ps->pen.setColor(Qt::color1);
        }
    }

    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    d->clip_serial = gc_cache_clip_serial++;

    return true;
}

bool QX11PaintEngine::end()
{
    setActive(false);
    if (d->pdev->devType() == QInternal::Widget  &&                 // #####
        ((QWidget*)d->pdev)->testAttribute(Qt::WA_PaintUnclipped)) {
        if (d->gc)
            XSetSubwindowMode(d->dpy, d->gc, ClipByChildren);
        if (d->gc_brush)
            XSetSubwindowMode(d->dpy, d->gc_brush, ClipByChildren);
    }

#if !defined(QT_NO_XFT)
    if (d->xft_hd) {
        // reset clipping/subwindow mode on our render picture
        XftDrawSetClip((XftDraw *) d->xft_hd, 0);
        XftDrawSetSubwindowMode((XftDraw *) d->xft_hd, ClipByChildren);
    }
#endif

    if (d->gc_brush) { // restore brush gc
        if (d->brushRef) {
            release_gc(d->brushRef);
            d->brushRef = 0;
        } else {
            free_gc(d->dpy, d->gc_brush, testf(UsePrivateCx));
        }
        d->gc_brush = 0;

    }
    if (d->gc) { // restore pen gc
        if (d->penRef) {
            release_gc(d->penRef);
            d->penRef = 0;
        } else {
            free_gc(d->dpy, d->gc, testf(UsePrivateCx));
        }
        d->gc = 0;
    }

    return true;
}

void QX11PaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    if (!isActive())
        return;
    if (d->cpen.style() != Qt::NoPen)
        XDrawLine(d->dpy, d->hd, d->gc, p1.x(), p1.y(), p2.x(), p2.y());
}

void QX11PaintEngine::drawRect(const QRect &r)
{
    if (!isActive())
        return;

#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
    ::Picture pict = d->xft_hd ? XftDrawPicture((XftDraw *) d->xft_hd) : 0;

    if (pict && d->cbrush.style() != Qt::NoBrush && d->cbrush.color().alpha() != 255) {
	XRenderColor xc;
	QColor qc = d->cbrush.color();

	const uint A = qc.alpha(),
		   R = qc.red(),
		   G = qc.green(),
		   B = qc.blue();

	xc.alpha = (A | A << 8);
	xc.red   = (R | R << 8) * xc.alpha / 0x10000;
	xc.green = (B | G << 8) * xc.alpha / 0x10000;
	xc.blue  = (B | B << 8) * xc.alpha / 0x10000;
	if (d->cpen.style() == Qt::NoPen) {
	    XRenderFillRectangle(d->dpy, PictOpOver, pict, &xc,
				 r.x(), r.y(), r.width(), r.height());
	    return;
	}
	int lw = d->cpen.width();
	int lw2 = (lw+1)/2;
	if (r.width() > lw && r.height() > lw)
	    XRenderFillRectangle(d->dpy, PictOpOver, pict, &xc,
				 r.x()+lw2, r.y()+lw2, r.width()-lw-1, r.height()-lw-1);
	if (d->cpen.style() != Qt::NoPen)
	    XDrawRectangle(d->dpy, d->hd, d->gc, r.x(), r.y(), r.width()-1, r.height()-1);
	return;
    }
#endif // !QT_NO_XFT && !QT_NO_XRENDER

    if (d->cbrush.style() != Qt::NoBrush) {
        if (d->cpen.style() == Qt::NoPen) {
            XFillRectangle(d->dpy, d->hd, d->gc_brush, r.x(), r.y(), r.width(), r.height());
            return;
        }
        int lw = d->cpen.width();
        int lw2 = (lw+1)/2;
        if (r.width() > lw && r.height() > lw)
            XFillRectangle(d->dpy, d->hd, d->gc_brush,
                           r.x()+lw2, r.y()+lw2, r.width()-lw-1, r.height()-lw-1);
    }
    if (d->cpen.style() != Qt::NoPen)
        XDrawRectangle(d->dpy, d->hd, d->gc, r.x(), r.y(), r.width()-1, r.height()-1);
}

void QX11PaintEngine::drawRects(const QList<QRect> &rects)
{
    if (!isActive())
        return;

    if (d->cbrush.style() != Qt::NoBrush && d->cpen.style() != Qt::NoPen) {
	for (int i = 0; i < rects.size(); ++i)
	    drawRect(rects.at(i));
    }

    QVarLengthArray<XRectangle> xrects(rects.size());
    for (int i = 0; i < rects.size(); ++i) {
	xrects[i].x = short(rects.at(i).x());
	xrects[i].y = short(rects.at(i).y());
	xrects[i].width = ushort(rects.at(i).width());
	xrects[i].height = ushort(rects.at(i).height());
    }

    if (d->cbrush.style() != Qt::NoBrush && d->cpen.style() == Qt::NoPen) {
	XFillRectangles(d->dpy, d->hd, d->gc_brush, xrects.data(), rects.size());
	return;
    }
    if (d->cpen.style() != Qt::NoPen && d->cbrush.style() == Qt::NoBrush) {
	for (int i = 0; i < rects.size(); ++i) { // ### this is kinda makes it useless but it's needed
	    xrects[i].width -= 1;
	    xrects[i].height -= 1;
	}
        XDrawRectangles(d->dpy, d->hd, d->gc, xrects.data(), rects.size());
    }
}

void QX11PaintEngine::drawPoint(const QPoint &p)
{
    if (!isActive())
        return;
    if (d->cpen.style() != Qt::NoPen)
        XDrawPoint(d->dpy, d->hd, d->gc, p.x(), p.y());
}

void QX11PaintEngine::drawPoints(const QPointArray &a, int index, int npoints)
{
    if (d->cpen.style() != Qt::NoPen)
        XDrawPoints(d->dpy, d->hd, d->gc, (XPoint*)(a.shortPoints(index, npoints)),
                    npoints, CoordModeOrigin);
}

void QX11PaintEngine::updatePen(const QPen &pen)
{
    d->cpen = pen;
    int ps = pen.style();
    bool cacheIt = !testf(ClipOn|MonoDev|NoCache) &&
                   (ps == Qt::NoPen || ps == Qt::SolidLine) &&
                   pen.width() == 0;

    bool obtained = false;
    bool internclipok = hasClipping();
    if (cacheIt) {
        if (d->gc) {
            if (d->penRef)
                release_gc(d->penRef);
            else
                free_gc(d->dpy, d->gc);
        }
        obtained = obtain_gc(&d->penRef, &d->gc, pen.color().pixel(d->scrn), d->dpy, d->scrn,
                             d->hd, d->clip_serial);
        if (!obtained && !d->penRef)
            d->gc = alloc_gc(d->dpy, d->scrn, d->hd, false);
    } else {
        if (d->gc) {
            if (d->penRef) {
                release_gc(d->penRef);
                d->penRef = 0;
                d->gc = alloc_gc(d->dpy, d->scrn, d->hd, testf(MonoDev));
            } else {
                internclipok = true;
            }
        } else {
            d->gc = alloc_gc(d->dpy, d->scrn, d->hd, testf(MonoDev), testf(UsePrivateCx));
        }
    }

    if (!internclipok) {
        if (d->pdev == paintEventDevice && paintEventClipRegion) {
            if (d->penRef &&((QGCC*)d->penRef)->clip_serial < gc_cache_clip_serial) {
                x11SetClipRegion(d->dpy, d->gc, 0, d->xft_hd, *paintEventClipRegion);
                ((QGCC*)d->penRef)->clip_serial = gc_cache_clip_serial;
            } else if (!d->penRef) {
                x11SetClipRegion(d->dpy, d->gc, 0, d->xft_hd, *paintEventClipRegion);
            }
        } else if (d->penRef && ((QGCC*)d->penRef)->clip_serial) {
            x11ClearClipRegion(d->dpy, d->gc, 0, d->xft_hd);
            ((QGCC*)d->penRef)->clip_serial = 0;
        }
    }

    if (obtained)
        return;

    char dashes[10];                            // custom pen dashes
    int dash_len = 0;                           // length of dash list
    int s = LineSolid;
    int cp = CapButt;
    int jn = JoinMiter;

    /*
      We are emulating Windows here.  Windows treats cpen.width() == 1
      (or 0) as a very special case.  The fudge variable unifies this
      case with the general case.
    */
    int dot = pen.width();                     // width of a dot
    int fudge = 1;
    bool allow_zero_lw = true;
    if (dot <= 1) {
        dot = 3;
        fudge = 2;
    }

    switch(ps) {
        case Qt::NoPen:
        case Qt::SolidLine:
            s = LineSolid;
            break;
        case Qt::DashLine:
            dashes[0] = fudge * 3 * dot;
            dashes[1] = fudge * dot;
            dash_len = 2;
            allow_zero_lw = false;
            break;
        case Qt::DotLine:
            dashes[0] = dot;
            dashes[1] = dot;
            dash_len = 2;
            allow_zero_lw = false;
            break;
        case Qt::DashDotLine:
            dashes[0] = 3 * dot;
            dashes[1] = fudge * dot;
            dashes[2] = dot;
            dashes[3] = fudge * dot;
            dash_len = 4;
            allow_zero_lw = false;
            break;
        case Qt::DashDotDotLine:
            dashes[0] = 3 * dot;
            dashes[1] = dot;
            dashes[2] = dot;
            dashes[3] = dot;
            dashes[4] = dot;
            dashes[5] = dot;
            dash_len = 6;
            allow_zero_lw = false;
    }
    Q_ASSERT(dash_len <= (int) sizeof(dashes));

    switch (pen.capStyle()) {
        case Qt::SquareCap:
            cp = CapProjecting;
            break;
        case Qt::RoundCap:
            cp = CapRound;
            break;
        case Qt::FlatCap:
        default:
            cp = CapButt;
            break;
    }
    switch (pen.joinStyle()) {
        case Qt::BevelJoin:
            jn = JoinBevel;
            break;
        case Qt::RoundJoin:
            jn = JoinRound;
            break;
        case Qt::MiterJoin:
        default:
            jn = JoinMiter;
            break;
    }

    XSetForeground(d->dpy, d->gc, pen.color().pixel(d->scrn));
    XSetBackground(d->dpy, d->gc, d->bg_col.pixel(d->scrn));

    if (dash_len) {                           // make dash list
        XSetDashes(d->dpy, d->gc, 0, dashes, dash_len);
        s = d->bg_mode == Qt::TransparentMode ? LineOnOffDash : LineDoubleDash;
    }
    XSetLineAttributes(d->dpy, d->gc,
                       (! allow_zero_lw && pen.width() == 0) ? 1 : pen.width(),
                       s, cp, jn);
}

QPixmap qt_pixmapForBrush(int brushStyle, bool invert); //in qbrush.cpp

void QX11PaintEngine::updateBrush(const QBrush &brush, const QPoint &origin)
{
    d->cbrush = brush;
    d->bg_origin = origin;

    int  bs = d->cbrush.style();
    int x = 0, y = 0;
    bool cacheIt = !testf(ClipOn|MonoDev|NoCache) &&
                   (bs == Qt::NoBrush || bs == Qt::SolidPattern) &&
                   x == 0 && y == 0;

    bool obtained = false;
    bool internclipok = hasClipping();
    if (cacheIt) {
        if (d->gc_brush) {
            if (d->brushRef)
                release_gc(d->brushRef);
            else
                free_gc(d->dpy, d->gc_brush);
        }
        obtained = obtain_gc(&d->brushRef, &d->gc_brush, d->cbrush.color().pixel(d->scrn), d->dpy,
                             d->scrn, d->hd, d->clip_serial);
        if (!obtained && !d->brushRef)
            d->gc_brush = alloc_gc(d->dpy, d->scrn, d->hd, false);
    } else {
        if (d->gc_brush) {
            if (d->brushRef) {
                release_gc(d->brushRef);
                d->brushRef = 0;
                d->gc_brush = alloc_gc(d->dpy, d->scrn, d->hd, testf(MonoDev));
            } else {
                internclipok = true;
            }
        } else {
            d->gc_brush = alloc_gc(d->dpy, d->scrn, d->hd, testf(MonoDev), testf(UsePrivateCx));
        }
    }

    if (!internclipok) {
        if (d->pdev == paintEventDevice && paintEventClipRegion) {
            if (d->brushRef &&((QGCC*)d->brushRef)->clip_serial < gc_cache_clip_serial) {
                x11SetClipRegion(d->dpy, d->gc_brush, 0, d->xft_hd, *paintEventClipRegion);
                ((QGCC*)d->brushRef)->clip_serial = gc_cache_clip_serial;
            } else if (!d->brushRef){
                x11SetClipRegion(d->dpy, d->gc_brush, 0, d->xft_hd, *paintEventClipRegion);
            }
        } else if (d->brushRef && ((QGCC*)d->brushRef)->clip_serial) {
            x11ClearClipRegion(d->dpy, d->gc_brush, 0, d->xft_hd);
            ((QGCC*)d->brushRef)->clip_serial = 0;
        }
    }

    if (obtained)
        return;


    XSetLineAttributes(d->dpy, d->gc_brush, 0, LineSolid, CapButt, JoinMiter);
    XSetForeground(d->dpy, d->gc_brush, d->cbrush.color().pixel(d->scrn));
    XSetBackground(d->dpy, d->gc_brush, d->bg_col.pixel(d->scrn));

    int s  = FillSolid;
    if (bs == Qt::CustomPattern || bs >= Qt::Dense1Pattern && bs <= Qt::DiagCrossPattern) {
        QPixmap pm;
        if (bs == Qt::CustomPattern)
            pm = *d->cbrush.pixmap();
        else
            pm = qt_pixmapForBrush(bs, true);
        pm.x11SetScreen(d->scrn);
        if (pm.depth() == 1) {
            XSetStipple(d->dpy, d->gc_brush, pm.handle());
            s = d->bg_mode == Qt::TransparentMode ? FillStippled : FillOpaqueStippled;
        } else {
            XSetTile(d->dpy, d->gc_brush, pm.handle());
            s = FillTiled;
        }
        XSetTSOrigin(d->dpy, d->gc_brush, origin.x(), origin.y());
    }
    XSetFillStyle(d->dpy, d->gc_brush, s);
}

void QX11PaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();
    w--;
    h--;
    if (w <= 0 || h <= 0) {
        if (w == 0 || h == 0)
            return;
        fix_neg_rect(&x, &y, &w, &h);
    }
    int rx = (w*xRnd)/200;
    int ry = (h*yRnd)/200;
    int rx2 = 2*rx;
    int ry2 = 2*ry;
    if (d->cbrush.style() != Qt::NoBrush) {          // draw filled round rect
        int dp, ds;
        if (d->cpen.style() == Qt::NoPen) {
            dp = 0;
            ds = 1;
        } else {
            dp = 1;
            ds = 0;
        }
#define SET_ARC(px, py, w, h, a1, a2) \
    a->x=px; a->y=py; a->width=w; a->height=h; a->angle1=a1; a->angle2=a2; a++
        XArc arcs[4];
        XArc *a = arcs;
        SET_ARC(x+w-rx2, y, rx2, ry2, 0, 90*64);
        SET_ARC(x, y, rx2, ry2, 90*64, 90*64);
        SET_ARC(x, y+h-ry2, rx2, ry2, 180*64, 90*64);
        SET_ARC(x+w-rx2, y+h-ry2, rx2, ry2, 270*64, 90*64);
        XFillArcs(d->dpy, d->hd, d->gc_brush, arcs, 4);
#undef SET_ARC
#define SET_RCT(px, py, w, h) \
    r->x=px; r->y=py; r->width=w; r->height=h; r++
        XRectangle rects[3];
        XRectangle *r = rects;
        SET_RCT(x+rx, y+dp, w-rx2, ry);
        SET_RCT(x+dp, y+ry, w+ds, h-ry2);
        SET_RCT(x+rx, y+h-ry, w-rx2, ry+ds);
        XFillRectangles(d->dpy, d->hd, d->gc_brush, rects, 3);
#undef SET_RCT
    }
    if (d->cpen.style() != Qt::NoPen) {              // draw outline
#define SET_ARC(px, py, w, h, a1, a2) \
    a->x=px; a->y=py; a->width=w; a->height=h; a->angle1=a1; a->angle2=a2; a++
        XArc arcs[4];
        XArc *a = arcs;
        SET_ARC(x+w-rx2, y, rx2, ry2, 0, 90*64);
        SET_ARC(x, y, rx2, ry2, 90*64, 90*64);
        SET_ARC(x, y+h-ry2, rx2, ry2, 180*64, 90*64);
        SET_ARC(x+w-rx2, y+h-ry2, rx2, ry2, 270*64, 90*64);
        XDrawArcs(d->dpy, d->hd, d->gc, arcs, 4);
#undef SET_ARC
#define SET_SEG(xp1, yp1, xp2, yp2) \
    s->x1=xp1; s->y1=yp1; s->x2=xp2; s->y2=yp2; s++
        XSegment segs[4];
        XSegment *s = segs;
        SET_SEG(x+rx, y, x+w-rx, y);
        SET_SEG(x+rx, y+h, x+w-rx, y+h);
        SET_SEG(x, y+ry, x, y+h-ry);
        SET_SEG(x+w, y+ry, x+w, y+h-ry);
        XDrawSegments(d->dpy, d->hd, d->gc, segs, 4);
#undef SET_SET
    }
}

void QX11PaintEngine::drawEllipse(const QRect &r)
{
    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();
    if (w == 1 && h == 1) {
        XDrawPoint(d->dpy, d->hd, (d->cpen.style() == Qt::NoPen) ? d->gc_brush : d->gc, x, y);
        return;
    }
    w--;
    h--;
    if (d->cbrush.style() != Qt::NoBrush) {          // draw filled ellipse
        XFillArc(d->dpy, d->hd, d->gc_brush, x, y, w, h, 0, 360*64);
        if (d->cpen.style() == Qt::NoPen) {
            XDrawArc(d->dpy, d->hd, d->gc_brush, x, y, w, h, 0, 360*64);
            return;
        }
    }
    if (d->cpen.style() != Qt::NoPen)                // draw outline
        XDrawArc(d->dpy, d->hd, d->gc, x, y, w, h, 0, 360*64);
}

void QX11PaintEngine::drawArc(const QRect &r, int a, int alen)
{
    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();
    w--;
    h--;
    if (w <= 0 || h <= 0)
        return;
    if (d->cpen.style() != Qt::NoPen)
        XDrawArc(d->dpy, d->hd, d->gc, x, y, w, h, a*4, alen*4);
}

void QX11PaintEngine::drawPie(const QRect &r, int a, int alen)
{
    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();

    // Make sure "a" is 0..360*16, as otherwise a*4 may overflow 16 bits.
    if (a > (360*16)) {
        a = a % (360*16);
    } else if (a < 0) {
        a = a % (360*16);
        if (a < 0) a += (360*16);
    }

    if (!isActive())
        return;

    XSetArcMode(d->dpy, d->gc_brush, ArcPieSlice);

    w--;
    h--;
    if (w <= 0 || h <= 0)
        return;

    GC g = d->gc;
    bool nopen = d->cpen.style() == Qt::NoPen;

    if (d->cbrush.style() != Qt::NoBrush) {          // draw filled pie
        XFillArc(d->dpy, d->hd, d->gc_brush, x, y, w, h, a*4, alen*4);
        if (nopen) {
            g = d->gc_brush;
            nopen = false;
        }
    }
    if (!nopen) {                             // draw pie outline
        double w2 = 0.5*w;                      // with, height in ellipsis
        double h2 = 0.5*h;
        double xc = (double)x+w2;
        double yc = (double)y+h2;
        double ra1 = Q_PI/2880.0*a;             // convert a, alen to radians
        double ra2 = ra1 + Q_PI/2880.0*alen;
        int xic = qRound(xc);
        int yic = qRound(yc);
        XDrawLine(d->dpy, d->hd, g, xic, yic,
                  qRound(xc + qcos(ra1)*w2), qRound(yc - qsin(ra1)*h2));
        XDrawLine(d->dpy, d->hd, g, xic, yic,
                  qRound(xc + qcos(ra2)*w2), qRound(yc - qsin(ra2)*h2));
        XDrawArc(d->dpy, d->hd, g, x, y, w, h, a*4, alen*4);
    }
}

void QX11PaintEngine::drawChord(const QRect &r, int a, int alen)
{
    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();

    XSetArcMode(d->dpy, d->gc_brush, ArcChord);

    w--;
    h--;
    if (w <= 0 || h <= 0)
        return;

    GC g = d->gc;
    bool nopen = d->cpen.style() == Qt::NoPen;

    if (d->cbrush.style() != Qt::NoBrush) {          // draw filled chord
        XFillArc(d->dpy, d->hd, d->gc_brush, x, y, w, h, a*4, alen*4);
        if (nopen) {
            g = d->gc_brush;
            nopen = false;
        }
    }
    if (!nopen) {                             // draw chord outline
        double w2 = 0.5*w;                      // with, height in ellipsis
        double h2 = 0.5*h;
        double xc = (double)x+w2;
        double yc = (double)y+h2;
        double ra1 = Q_PI/2880.0*a;             // convert a, alen to radians
        double ra2 = ra1 + Q_PI/2880.0*alen;
        XDrawLine(d->dpy, d->hd, g,
                  qRound(xc + qcos(ra1)*w2), qRound(yc - qsin(ra1)*h2),
                  qRound(xc + qcos(ra2)*w2), qRound(yc - qsin(ra2)*h2));
        XDrawArc(d->dpy, d->hd, g, x, y, w, h, a*4, alen*4);
    }

    XSetArcMode(d->dpy, d->gc_brush, ArcPieSlice);
}

void QX11PaintEngine::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    QPointArray pa = a;
    if (d->cpen.style() != Qt::NoPen)
        XDrawSegments(d->dpy, d->hd, d->gc,
                      (XSegment*)(pa.shortPoints(index, nlines*2)), nlines);
}

void QX11PaintEngine::drawPolyline(const QPointArray &a, int index, int npoints)
{
    QPointArray pa = a;
    if (d->cpen.style() != Qt::NoPen) {
        while(npoints > 65535) {
            XDrawLines(d->dpy, d->hd, d->gc, (XPoint*)(pa.shortPoints(index, 65535)),
                       65535, CoordModeOrigin);
            npoints -= 65535;
            index += 65535;
        }
        XDrawLines(d->dpy, d->hd, d->gc, (XPoint*)(pa.shortPoints(index, npoints)),
                   npoints, CoordModeOrigin);
    }
}

static int global_polygon_shape = Complex;

void QX11PaintEngine::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
{
    QPointArray pa = a;

    if (pa[index] != pa[index + npoints - 1]) {   // close open pointarray
        pa.detach();
        pa.resize(index + npoints + 1);
        pa.setPoint(index + npoints, pa[index]);
        ++npoints;
    }

#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
    if (d->cbrush.style() != Qt::NoBrush && d->cbrush.color().alpha() != 255) {
	XftColor xfc;
	QColor qc = d->cbrush.color();

	const uint A = qc.alpha(),
		   R = qc.red(),
		   G = qc.green(),
		   B = qc.blue();

	xfc.pixel = qc.pixel();
	xfc.color.alpha = (A | A << 8);
	xfc.color.red   = (R | R << 8) * xfc.color.alpha / 0x10000;
	xfc.color.green = (B | G << 8) * xfc.color.alpha / 0x10000;
	xfc.color.blue  = (B | B << 8) * xfc.color.alpha / 0x10000;
	::Picture src = d->xft_hd ? XftDrawSrcPicture((XftDraw *) d->xft_hd, &xfc) : 0;
	::Picture dst = d->xft_hd ? XftDrawPicture((XftDraw *) d->xft_hd) : 0;

	if (src && dst) {
	    XPointDouble *poly = new XPointDouble[pa.size()];
	    for (int i = 0; i < pa.size(); ++i) {
		poly[i].x = pa[i].x();
		poly[i].y = pa[i].y();
	    }
	    XRenderCompositeDoublePoly(d->dpy, PictOpOver, src, dst,
				       XRenderFindStandardFormat(d->dpy, PictStandardARGB32),
				       0, 0, 0, 0, poly, npoints, winding);
	    delete [] poly;

	    if (d->cpen.style() != Qt::NoPen) {              // draw outline
		XDrawLines(d->dpy, d->hd, d->gc, (XPoint*)(pa.shortPoints(index, npoints)),
			   npoints, CoordModeOrigin);
	    }
	    return;
	}
    }
#endif

    if (winding)                              // set to winding fill rule
        XSetFillRule(d->dpy, d->gc_brush, WindingRule);

    if (d->cbrush.style() != Qt::NoBrush) {          // draw filled polygon
        XFillPolygon(d->dpy, d->hd, d->gc_brush,
                     (XPoint*)(pa.shortPoints(index, npoints)),
                     npoints, global_polygon_shape, CoordModeOrigin);
    }
    if (d->cpen.style() != Qt::NoPen) {              // draw outline
        XDrawLines(d->dpy, d->hd, d->gc, (XPoint*)(pa.shortPoints(index, npoints)),
                   npoints, CoordModeOrigin);
    }
    if (winding)                              // set to normal fill rule
        XSetFillRule(d->dpy, d->gc_brush, EvenOddRule);
}

void QX11PaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    global_polygon_shape = Convex;
    drawPolygon(pa, false, index, npoints);
    global_polygon_shape = Complex;
}

void QX11PaintEngine::drawCubicBezier(const QPointArray &a, int index)
{
    if (!isActive())
        return;
    if (a.size() - index < 4) {
        qWarning("drawCubicBezier: Cubic Bezier needs 4 control points");
        return;
    }
    QPointArray pa(a);
    if (index != 0 || a.size() > 4) {
        pa = QPointArray(4);
        for (int i = 0; i < 4; i++)
            pa.setPoint(i, a.point(index + i));
    }
    if (d->cpen.style() != Qt::NoPen) {
        pa = pa.cubicBezier();
        XDrawLines(d->dpy, d->hd, d->gc, (XPoint*)pa.shortPoints(), pa.size(),
                   CoordModeOrigin);
    }
}

void QX11PaintEngine::drawPixmap(const QRect &r, const QPixmap &pixmap, const QRect &sr,
                                 Qt::BlendMode mode)
{
    int x = r.x();
    int y = r.y();
    int sx = sr.x();
    int sy = sr.y();
    int sw = sr.width();
    int sh = sr.height();
    // since we can't scale pixmaps this should always hold
    Q_ASSERT(r.width() == sr.width() && r.height() == sr.height());

    if (d->xinfo && d->xinfo->screen() != pixmap.x11Info()->screen()) {
        QPixmap* p = const_cast<QPixmap *>(&pixmap);
        p->x11SetScreen(d->xinfo->screen());
    }

    QPixmap::x11SetDefaultScreen(pixmap.x11Info()->screen());

    QBitmap *mask = 0;
    if(mode == Qt::AlphaBlend)
        mask = (QBitmap *)pixmap.mask();
    bool mono = pixmap.depth() == 1;

    if (mask && !hasClipping() && d->pdev != paintEventDevice) {
        if (mono) {                           // needs GCs pen // color
            bool selfmask = pixmap.data->selfmask;
            if (selfmask) {
                XSetFillStyle(d->dpy, d->gc, FillStippled);
                XSetStipple(d->dpy, d->gc, pixmap.handle());
            } else {
                XSetFillStyle(d->dpy, d->gc, FillOpaqueStippled);
                XSetStipple(d->dpy, d->gc, pixmap.handle());
                XSetClipMask(d->dpy, d->gc, mask->handle());
                XSetClipOrigin(d->dpy, d->gc, x-sx, y-sy);
            }
            XSetTSOrigin(d->dpy, d->gc, x-sx, y-sy);
            XFillRectangle(d->dpy, d->hd, d->gc, x, y, sw, sh);
            XSetTSOrigin(d->dpy, d->gc, 0, 0);
            XSetFillStyle(d->dpy, d->gc, FillSolid);
            if (!selfmask) {
                if (d->pdev == paintEventDevice && paintEventClipRegion) {
                    x11SetClipRegion(d->dpy, d->gc, 0, d->xft_hd, *paintEventClipRegion);
                } else {
                    x11ClearClipRegion(d->dpy, d->gc, 0, d->xft_hd);
                }
            }
        } else {
            bitBlt(d->pdev, x, y, &pixmap, sx, sy, sw, sh);
        }
        return;
    }

    QRegion rgn = d->crgn; // ### remove this

    if (mask) {                               // pixmap has clip mask
        // Implies that clipping is on, either explicit or implicit
        // Create a new mask that combines the mask with the clip region

        if (d->pdev == paintEventDevice && paintEventClipRegion) {
            if (hasClipping())
                rgn = rgn.intersect(*paintEventClipRegion);
            else
                rgn = *paintEventClipRegion;
        }

        QBitmap *comb = new QBitmap(sw, sh);
        comb->detach();
        GC cgc = qt_xget_temp_gc(pixmap.x11Info()->screen(), true);   // get temporary mono GC
        XSetForeground(d->dpy, cgc, 0);
        XFillRectangle(d->dpy, comb->handle(), cgc, 0, 0, sw, sh);
        XSetBackground(d->dpy, cgc, 0);
        XSetForeground(d->dpy, cgc, 1);
        int num;
        XRectangle *rects = (XRectangle *)qt_getClipRects(rgn, num);
        XSetClipRectangles(d->dpy, cgc, -x, -y, rects, num, Unsorted);
        XSetFillStyle(d->dpy, cgc, FillOpaqueStippled);
        XSetStipple(d->dpy, cgc, mask->handle());
        XSetTSOrigin(d->dpy, cgc, -sx, -sy);
        XFillRectangle(d->dpy, comb->handle(), cgc, 0, 0, sw, sh);
        XSetTSOrigin(d->dpy, cgc, 0, 0);         // restore cgc
        XSetFillStyle(d->dpy, cgc, FillSolid);
        XSetClipMask(d->dpy, cgc, XNone);
        mask = comb;                            // it's deleted below

        XSetClipMask(d->dpy, d->gc, mask->handle());
        XSetClipOrigin(d->dpy, d->gc, x, y);
    }

    if (mono) {
        XSetBackground(d->dpy, d->gc, d->bg_brush.color().pixel(d->scrn));
        XSetFillStyle(d->dpy, d->gc, FillOpaqueStippled);
        XSetStipple(d->dpy, d->gc, pixmap.handle());
        XSetTSOrigin(d->dpy, d->gc, x-sx, y-sy);
        XFillRectangle(d->dpy, d->hd, d->gc, x, y, sw, sh);
        XSetTSOrigin(d->dpy, d->gc, 0, 0);
        XSetFillStyle(d->dpy, d->gc, FillSolid);
    } else {
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
        ::Picture pict = d->xft_hd ? XftDrawPicture((XftDraw *) d->xft_hd) : 0;
        QPixmap *alpha = pixmap.data->alphapm;

        if (pict && pixmap.xftPictureHandle() &&
            alpha && alpha->xftPictureHandle()) {
            XRenderComposite(d->dpy, PictOpOver, pixmap.xftPictureHandle(),
                             alpha->xftPictureHandle(), pict,
                             sx, sy, sx, sy, x, y, sw, sh);
        } else
#endif // !QT_NO_XFT && !QT_NO_XRENDER
        {
            XCopyArea(d->dpy, pixmap.handle(), d->hd, d->gc, sx, sy, sw, sh, x, y);
        }
    }

    if (mask) {                               // restore clipping
        XSetClipOrigin(d->dpy, d->gc, 0, 0);
        int num;
        XRectangle *rects = (XRectangle *)qt_getClipRects(d->crgn, num);
        XSetClipRectangles(d->dpy, d->gc, 0, 0, rects, num, Unsorted);
        delete mask;                            // delete comb, created above
    }
}

void QX11PaintEngine::updateBackground(Qt::BGMode mode, const QBrush &bgBrush)
{
    Q_ASSERT(isActive());
    d->bg_mode = mode;
    d->bg_brush = bgBrush;
    d->bg_col = bgBrush.color();
    if (!d->penRef)
        updatePen(d->cpen);                            // update pen setting
    if (!d->brushRef)
        updateBrush(d->cbrush, d->bg_origin);                        // update brush setting
}

void QX11PaintEngine::updateXForm(const QWMatrix &mtx)
{
    if (mtx.m12() != 0 || mtx.m21() != 0)
        d->txop = QPainter::TxRotShear;
    else if (mtx.m11() != 1 || mtx.m22() != 1)
        d->txop = QPainter::TxScale;
    else if (mtx.dx() != 0 || mtx.dy() != 0)
        d->txop = QPainter::TxTranslate;
    else
        d->txop = QPainter::TxNone;
}

void QX11PaintEngine::updateClipRegion(const QRegion &clipRegion, bool clipEnabled)
{
    Q_ASSERT(isActive());

    clearf(ClipOn);
    d->crgn = clipRegion;

    if (clipEnabled) {
        setf(ClipOn);
        if (d->pdev == paintEventDevice && paintEventClipRegion)
            d->crgn = d->crgn.intersect(*paintEventClipRegion);
        if (d->penRef)
            updatePen(d->cpen);
        if (d->brushRef)
            updateBrush(d->cbrush, d->bg_origin);
        x11SetClipRegion(d->dpy, d->gc, d->gc_brush, d->xft_hd, d->crgn);
    } else {
        if (d->pdev == paintEventDevice && paintEventClipRegion) {
            x11SetClipRegion(d->dpy, d->gc, d->gc_brush, d->xft_hd, *paintEventClipRegion);
        } else {
            x11ClearClipRegion(d->dpy, d->gc, d->gc_brush, d->xft_hd);
        }
    }
}

void QX11PaintEngine::updateFont(const QFont &)
{
    clearf(DirtyFont);
    if (d->penRef)
        updatePen(d->cpen);                            // force a non-cached GC
}

Qt::HANDLE QX11PaintEngine::handle() const
{
    Q_ASSERT(isActive());
    Q_ASSERT(d->hd);
    return d->hd;
}

extern void qt_draw_tile(QPaintEngine *, int, int, int, int, const QPixmap &, int, int);

void QX11PaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &p)
{
    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();
    int sx = p.x();
    int sy = p.y();

    if (pixmap.mask() == 0 && pixmap.depth() > 1 && d->txop <= QPainter::TxTranslate) {
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
        ::Picture pict = d->xft_hd ? XftDrawPicture((XftDraw *) d->xft_hd) : 0;
        QPixmap *alpha = pixmap.data->alphapm;

        if (pict && pixmap.xftPictureHandle() && alpha && alpha->xftPictureHandle()) {
            // this is essentially drawTile() from above, inlined for
            // the XRenderComposite call
            int yPos, xPos, drawH, drawW, yOff, xOff;
            yPos = y;
            yOff = sy;
            while(yPos < y + h) {
                drawH = pixmap.height() - yOff;    // Cropping first row
                if (yPos + drawH > y + h)        // Cropping last row
                    drawH = y + h - yPos;
                xPos = x;
                xOff = sx;
                while(xPos < x + w) {
                    drawW = pixmap.width() - xOff; // Cropping first column
                    if (xPos + drawW > x + w)    // Cropping last column
                        drawW = x + w - xPos;
                    XRenderComposite(d->dpy, PictOpOver, pixmap.xftPictureHandle(),
                                     alpha->xftPictureHandle(), pict,
                                     xOff, yOff, xOff, yOff, xPos, yPos, drawW, drawH);
                    xPos += drawW;
                    xOff = 0;
                }
                yPos += drawH;
                yOff = 0;
            }
            return;
        }
#endif // !QT_NO_XFTQT_NO_XRENDER

        XSetTile(d->dpy, d->gc, pixmap.handle());
        XSetFillStyle(d->dpy, d->gc, FillTiled);
        XSetTSOrigin(d->dpy, d->gc, x-sx, y-sy);
        XFillRectangle(d->dpy, d->hd, d->gc, x, y, w, h);
        XSetTSOrigin(d->dpy, d->gc, 0, 0);
        XSetFillStyle(d->dpy, d->gc, FillSolid);
    } else {
        qt_draw_tile(this, x, y, w, h, pixmap, sx, sy);
    }
}

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

#include "qplatformdefs.h"

#include "private/qpixmap_p.h"

#include "qapplication.h"
#include "qfont.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qtextcodec.h"
#include "qcoreevent.h"
#include "qiodevice.h"

#include "qpainter_p.h"
#include <qtextlayout.h>
#include <qvarlengtharray.h>
#include <private/qfont_p.h>
#include <private/qtextengine_p.h>
#include <private/qpaintengine_x11_p.h>
#include <private/qfontengine_p.h>

#include "qpen.h"
#include "qcolor.h"
#include "qcolormap.h"
#include "qfont.h"

#include "qmath_p.h"

#include <private/qpaintengine_p.h>
#include "qpaintengine_x11_p.h"

#include <private/qt_x11_p.h>
#include <private/qnumeric_p.h>
#include <limits.h>

#include "qdebug.h"

#define d d_func()
#define q q_func()

extern Drawable qt_x11Handle(const QPaintDevice *pd);
extern const QX11Info *qt_x11Info(const QPaintDevice *pd);

// hack, so we don't have to make QRegion::clipRectangles() public or include
// X11 headers in qregion.h
inline void *qt_getClipRects(const QRegion &r, int &num)
{
    return r.clipRectangles(num);
}

static inline void x11SetClipRegion(Display *dpy, GC gc, GC gc2,
#ifndef QT_NO_XFT
                                    Picture picture,
#else
                                    Qt::HANDLE picture,
#endif
                                    const QRegion &r)
{
    int num;
    XRectangle *rects = (XRectangle *)qt_getClipRects(r, num);

    if (gc)
        XSetClipRectangles( dpy, gc, 0, 0, rects, num, YXBanded );
    if (gc2)
        XSetClipRectangles( dpy, gc2, 0, 0, rects, num, YXBanded );

#ifndef QT_NO_XFT
    if (picture)
        XRenderSetPictureClipRectangles(dpy, picture, 0, 0, rects, num);
#endif // QT_NO_XFT
}

static inline void x11ClearClipRegion(Display *dpy, GC gc, GC gc2,
#ifndef QT_NO_XFT
                                    Picture picture
#else
                                    Qt::HANDLE picture
#endif
                                      )
{
    if (gc)
        XSetClipMask(dpy, gc, XNone);
    if (gc2)
        XSetClipMask(dpy, gc2, XNone);

#ifndef QT_NO_XFT
    if (picture) {
        XRenderPictureAttributes attrs;
        attrs.clip_mask = XNone;
        XRenderChangePicture (dpy, picture, CPClipMask, &attrs);
    }
#endif // QT_NO_XFT
}

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
    Display *dpy = QX11Info::display();

    ulong pixel;
    if (pd->depth() == 1) {
        pixel = qGray(brush.color().rgb()) > 127 ? 0 : 1;
    } else if (pd->devType() == QInternal::Pixmap && pd->depth() == 32
               && X11->use_xrender && X11->has_xft) {
        pixel = brush.color().rgba();
    } else {
        pixel = QColormap::instance(screen).pixel(brush.color());
    }

    XGCValues vals;
    vals.graphics_exposures = false;
    vals.foreground = pixel;
    GC gc = XCreateGC(dpy, hd, GCForeground | GCGraphicsExposures, &vals);

    if (brush.texture().isNull()) {
        XFillRectangle(dpy, hd, gc, x, y, w, h);
    } else {
        XSetTile(dpy, gc, brush.texture().handle());
        XSetFillStyle(dpy, gc, FillTiled);
        XSetTSOrigin(dpy, gc, x-xoff, y-yoff);
        XFillRectangle(dpy, hd, gc, x, y, w, h);
        XSetTSOrigin(dpy, gc, 0, 0);
        XSetFillStyle(dpy, gc, FillSolid);
    }

    XFreeGC(dpy, gc);
}

void qt_draw_transformed_rect(QPaintEngine *pe, int x, int y, int w,  int h, bool fill)
{
    QX11PaintEngine *p = static_cast<QX11PaintEngine *>(pe);

    XPoint points[5];
    int xp = x,  yp = y;
    p->state->matrix.map(xp, yp, &xp, &yp);
    points[0].x = xp;
    points[0].y = yp;
    xp = x + w; yp = y;
    p->state->matrix.map(xp, yp, &xp, &yp);
    points[1].x = xp;
    points[1].y = yp;
    xp = x + w; yp = y + h;
    p->state->matrix.map(xp, yp, &xp, &yp);
    points[2].x = xp;
    points[2].y = yp;
    xp = x; yp = y + h;
    p->state->matrix.map(xp, yp, &xp, &yp);
    points[3].x = xp;
    points[3].y = yp;
    points[4] = points[0];

    if (fill)
        XFillPolygon(p->d->dpy, p->d->hd, p->d->gc, points, 4, Convex, CoordModeOrigin);
    else
        XDrawLines(p->d->dpy, p->d->hd, p->d->gc, points, 5, CoordModeOrigin);
}



#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)

/*
 * Polygon tesselator - can probably be optimized a bit more
 */

//#define QT_DEBUG_TESSELATOR
#define FloatToXFixed(i) (int)((i) * 65536)
#define IntToXFixed(i) ((i) << 16)

Q_DECLARE_TYPEINFO(XTrapezoid, Q_PRIMITIVE_TYPE);

// used by the edge point sort algorithm
static qreal currentY = 0.f;

struct QEdge {
    QPointF p1, p2;
    qreal m;
    qreal b;
    char winding;
};

Q_DECLARE_TYPEINFO(QEdge, Q_PRIMITIVE_TYPE);

static inline bool compareEdges(const QEdge *e1, const QEdge *e2)
{
    return e1->p1.y() < e2->p1.y();
}

static inline bool isEqual(const QPointF &p1, const QPointF &p2)
{
    return qAbs(p1.x()-p2.x()) < 0.0001 && qAbs(p1.y() - p2.y()) < 0.0001;
}

struct QIntersectionPoint {
    qreal x;
    const QEdge *edge;
};
Q_DECLARE_TYPEINFO(QIntersectionPoint, Q_PRIMITIVE_TYPE);

static inline bool compareIntersections(const QIntersectionPoint &i1, const QIntersectionPoint &i2)
{
    if (qAbs(i1.x - i2.x) > 0.01) { // x != other.x in 99% of the cases
        return i1.x < i2.x;
    } else {
        qreal x1 = !qIsFinite(i1.edge->b) ? i1.edge->p1.x() :
                   (currentY+1.f - i1.edge->b)*i1.edge->m;
        qreal x2 = !qIsFinite(i2.edge->b) ? i2.edge->p1.x() :
                   (currentY+1.f - i2.edge->b)*i2.edge->m;
        return x1 < x2;
    }
}

#ifdef QT_USE_FIXED_POINT
inline int qrealToXFixed(qreal f)
{ return f.value() << 8; }
#else
#define qrealToXFixed FloatToXFixed
#endif

static XTrapezoid QT_FASTCALL toXTrapezoid(XFixed y1, XFixed y2, const QEdge &left, const QEdge &right)
{
    XTrapezoid trap;
    trap.top = y1;
    trap.bottom = y2;
    trap.left.p1.y = qrealToXFixed(left.p1.y());
    trap.left.p2.y = qrealToXFixed(left.p2.y());
    trap.right.p1.y = qrealToXFixed(right.p1.y());
    trap.right.p2.y = qrealToXFixed(right.p2.y());
    trap.left.p1.x = qrealToXFixed(left.p1.x());
    trap.left.p2.x = qrealToXFixed(left.p2.x());
    trap.right.p1.x = qrealToXFixed(right.p1.x());
    trap.right.p2.x = qrealToXFixed(right.p2.x());
    return trap;
}

#ifdef QT_DEBUG_TESSELATOR
static void dump_edges(const QList<const QEdge *> &et)
{
    for (int x = 0; x < et.size(); ++x) {
        qDebug() << "edge#" << x << et.at(x)->p1 << et.at(x)->p2 << "b: " << et.at(x)->b << "m:" << et.at(x)->m << et.at(x);
    }
}

static void dump_trap(const XTrapezoid &t)
{
    qDebug() << "trap# t=" << t.top/65536.0 << "b=" << t.bottom/65536.0  << "h="
             << XFixedToDouble(t.bottom - t.top) << "\tleft p1: ("
             << XFixedToDouble(t.left.p1.x) << ","<< XFixedToDouble(t.left.p1.y)
             << ")" << "\tleft p2: (" << XFixedToDouble(t.left.p2.x) << ","
             << XFixedToDouble(t.left.p2.y) << ")" << "\n\t\t\t\tright p1:("
             << XFixedToDouble(t.right.p1.x) << "," << XFixedToDouble(t.right.p1.y) << ")"
             << "\tright p2:(" << XFixedToDouble(t.right.p2.x) << ","
             << XFixedToDouble(t.right.p2.y) << ")";
}
#endif


static void qt_tesselate_polygon(QVector<XTrapezoid> *traps, const QPointF *pg, int pgSize,
                                 bool winding)
{
    QVector<QEdge> edges;
    edges.reserve(128);
    qreal ymin(INT_MAX/256);
    qreal ymax(INT_MIN/256);

    Q_ASSERT(pg[0] == pg[pgSize-1]);
    // generate edge table
    for (int x = 0; x < pgSize-1; ++x) {
	QEdge edge;
	edge.winding = pg[x].y() > pg[x+1].y() ? 1 : -1;
	if (edge.winding > 0) {
	    edge.p1 = pg[x+1];
	    edge.p2 = pg[x];
	} else {
	    edge.p1 = pg[x];
	    edge.p2 = pg[x+1];
	}
	edge.m = (edge.p1.y() - edge.p2.y()) / (edge.p1.x() - edge.p2.x()); // line derivative
	edge.b = edge.p1.y() - edge.m * edge.p1.x(); // intersection with y axis
	edge.m = edge.m != 0.0 ? 1.0 / edge.m : 0.0; // inverted derivative
	edges.append(edge);
        ymin = qMin(ymin, edge.p1.y());
        ymax = qMax(ymax, edge.p2.y());
    }

    QList<const QEdge *> et; 	    // edge list
    for (int i = 0; i < edges.size(); ++i)
        et.append(&edges.at(i));

    // sort edge table by min y value
    qSort(et.begin(), et.end(), compareEdges);

    // eliminate shared edges
    for (int i = 0; i < et.size(); ++i) {
	for (int k = i+1; k < et.size(); ++k) {
            if (et.at(k)->p1.y() > et.at(i)->p1.y() + 0.0001)
                break;
   	    if (et.at(i)->winding != et.at(k)->winding &&
                isEqual(et.at(i)->p1, et.at(k)->p1) && isEqual(et.at(i)->p2, et.at(k)->p2)
		) {
 		et.removeAt(k);
		et.removeAt(i);
		--i;
		break;
	    }
	}
    }

    if (ymax <= ymin)
	return;
    QList<const QEdge *> aet; 	    // edges that intersects the current scanline

//     if (ymin < 0)
// 	ymin = 0;
//     if (paintEventClipRegion) // don't scan more lines than we have to
// 	ymax = paintEventClipRegion->boundingRect().height();

#ifdef QT_DEBUG_TESSELATOR
    qDebug("==> ymin = %f, ymax = %f", ymin, ymax);
#endif // QT_DEBUG_TESSELATOR

    currentY = ymin; // used by the less than op
    for (qreal y = ymin; y < ymax;) {
	// fill active edge table with edges that intersect the current line
	for (int i = 0; i < et.size(); ++i) {
            if (et.at(i)->p1.y() > y)
                break;
            aet.append(et.at(i));
            et.removeAt(i);
            --i;
	}

	// remove processed edges from active edge table
	for (int i = 0; i < aet.size(); ++i) {
	    if (aet.at(i)->p2.y() <= y) {
		aet.removeAt(i);
 		--i;
	    }
	}
        if (aet.size()%2 != 0) {
#ifndef QT_NO_DEBUG
            qWarning("QX11PaintEngine: aet out of sync - this should not happen.");
#endif
            return;
        }

	// done?
	if (!aet.size()) {
            if (!et.size()) {
                break;
	    } else {
 		y = et.at(0)->p1.y();
                continue;
	    }
        }

        // calculate the next y where we have to start a new set of trapezoids
	qreal next_y(INT_MAX/256);
 	for (int i = 0; i < aet.size(); ++i)
 	    if (aet.at(i)->p2.y() < next_y)
 		next_y = aet.at(i)->p2.y();

	if (et.size() && next_y > et.at(0)->p1.y())
	    next_y = et.at(0)->p1.y();

	for (int i = 0; i < aet.size(); ++i) {
	    for (int k = i+1; k < aet.size(); ++k) {
		qreal m1 = aet.at(i)->m;
		qreal b1 = aet.at(i)->b;
		qreal m2 = aet.at(k)->m;
		qreal b2 = aet.at(k)->b;

		if (qAbs(m1 - m2) < 0.001)
                    continue;

                // ### intersect is not calculated correctly when optimized with -O2 (gcc)
#ifndef QT_USE_FIXED_POINT
                volatile
#endif
                    qreal intersect;
                if (!qIsFinite(b1))
                    intersect = (1.f / m2) * aet.at(i)->p1.x() + b2;
                else if (!qIsFinite(b2))
                    intersect = (1.f / m1) * aet.at(k)->p1.x() + b1;
                else
                    intersect = (b1*m1 - b2*m2) / (m1 - m2);

 		if (intersect > y && intersect < next_y)
		    next_y = intersect;
	    }
	}

        XFixed yf, next_yf;
        yf = qrealToXFixed(y);
        next_yf = qrealToXFixed(next_y);

        if (yf == next_yf) {
            y = currentY = next_y;
            continue;
        }

#ifdef QT_DEBUG_TESSELATOR
        qDebug("###> y = %f, next_y = %f, %d active edges", y, next_y, aet.size());
        qDebug("===> edges");
        dump_edges(et);
        qDebug("===> active edges");
        dump_edges(aet);
#endif
	// calc intersection points
 	QVarLengthArray<QIntersectionPoint> isects(aet.size()+1);
 	for (int i = 0; i < isects.size()-1; ++i) {
 	    isects[i].x = qAbs(aet.at(i)->p1.x() - aet.at(i)->p2.x()) > 0.0001 ?
			  ((y - aet.at(i)->b)*aet.at(i)->m) : aet.at(i)->p1.x();
	    isects[i].edge = aet.at(i);
	}

	Q_ASSERT(isects.size()%2 == 1);

	// sort intersection points
 	qSort(&isects[0], &isects[isects.size()-1], compareIntersections);

        if (winding) {
            // winding fill rule
            for (int i = 0; i < isects.size()-1;) {
                int winding = 0;
                const QEdge *left = isects[i].edge;
                const QEdge *right = 0;
                winding += isects[i].edge->winding;
                for (++i; i < isects.size()-1 && winding != 0; ++i) {
                    winding += isects[i].edge->winding;
                    right = isects[i].edge;
                }
                if (!left || !right)
                    break;
                traps->append(toXTrapezoid(yf, next_yf, *left, *right));
            }
        } else {
            // odd-even fill rule
            for (int i = 0; i < isects.size()-2; i += 2)
                traps->append(toXTrapezoid(yf, next_yf, *isects[i].edge, *isects[i+1].edge));
        }
	y = currentY = next_y;
    }

#ifdef QT_DEBUG_TESSELATOR
    qDebug("==> number of trapezoids: %d - edge table size: %d\n", traps->size(), et.size());
#endif

    // optimize by unifying trapezoids that share left/right lines
    // and have a common top/bottom edge
//     for (int i = 0; i < tps.size(); ++i) {
// 	for (int k = i+1; k < tps.size(); ++k) {
// 	    if (i != k && tps.at(i).right == tps.at(k).right
// 		&& tps.at(i).left == tps.at(k).left
// 		&& (tps.at(i).top == tps.at(k).bottom
// 		    || tps.at(i).bottom == tps.at(k).top))
// 	    {
// 		tps[i].bottom = tps.at(k).bottom;
// 		tps.removeAt(k);
//                 i = 0;
// 		break;
// 	    }
// 	}
//     }
}
#endif // !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)


/*
 * QX11PaintEngine members
 */

QX11PaintEngine::QX11PaintEngine()
    : QPaintEngine(*(new QX11PaintEnginePrivate), UsesFontEngine
                   | CoordTransform
                   | PainterPaths
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
                   | AlphaStroke
                   | AlphaFillPolygon
                   | AlphaFill
                   | AlphaPixmap
                   | FillAntialiasing
                   | LineAntialiasing
#endif
        )
{
    d->dpy = 0;
    d->scrn = 0;
    d->hd = 0;
    d->xft_hd = 0;
    d->xinfo = 0;
    if (!X11->use_xrender) {
        gccaps &= ~AlphaStroke;
        gccaps &= ~AlphaFillPolygon;
        gccaps &= ~AlphaFill;
        gccaps &= ~AlphaPixmap;
        gccaps &= ~FillAntialiasing;
        gccaps &= ~LineAntialiasing;
    }
}

QX11PaintEngine::QX11PaintEngine(QX11PaintEnginePrivate &dptr)
    : QPaintEngine(dptr, UsesFontEngine
                   | CoordTransform
                   | PainterPaths
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
                   | AlphaStroke
                   | AlphaFillPolygon
                   | AlphaFill
                   | AlphaPixmap
                   | FillAntialiasing
                   | LineAntialiasing
#endif
        )
{
    d->dpy = 0;
    d->scrn = 0;
    d->hd = 0;
    d->xft_hd = 0;
    d->xinfo = 0;
    if (!X11->use_xrender) {
        gccaps &= ~AlphaStroke;
        gccaps &= ~AlphaFillPolygon;
        gccaps &= ~AlphaFill;
        gccaps &= ~AlphaPixmap;
        gccaps &= ~FillAntialiasing;
        gccaps &= ~LineAntialiasing;
    }
}

QX11PaintEngine::~QX11PaintEngine()
{
}

void QX11PaintEngine::initialize()
{
}

void QX11PaintEngine::cleanup()
{
}

bool QX11PaintEngine::begin(QPaintDevice *pdev)
{
    d->pdev = pdev;
    d->xinfo = qt_x11Info(pdev);
    d->hd = qt_x11Handle(pdev);
#ifndef QT_NO_XFT
    if (pdev->devType() == QInternal::Widget) {
        d->xft_hd = (XftDraw *)static_cast<const QWidget *>(pdev)->xftDrawHandle();
        d->picture = (::Picture)static_cast<const QWidget *>(pdev)->xftPictureHandle();
    } else if (pdev->devType() == QInternal::Pixmap) {
        d->xft_hd = (XftDraw *)static_cast<const QPixmap *>(pdev)->xftDrawHandle();
        d->picture = (::Picture)static_cast<const QPixmap *>(pdev)->xftPictureHandle();
    }
#else
    d->xft_hd = 0;
    d->picture = 0;
#endif
    Q_ASSERT(d->xinfo != 0);
    d->dpy = d->xinfo->display(); // get display variable
    d->scrn = d->xinfo->screen(); // get screen variable

    if (isActive()) {                         // already active painting
        qWarning("QX11PaintEngine::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()");
        return false;
    }

    d->gc = XCreateGC(d->dpy, d->hd, 0, 0);
    d->gc_brush = XCreateGC(d->dpy, d->hd, 0, 0);
    d->use_path_fallback = false;

    // Set up the polygon clipper. Note: This will only work in
    // polyline mode as long as we have a buffer zone, since a
    // polyline may be clipped into several non-connected polylines.
    const int BUFFERZONE = 100;
    QRect devClipRect;
    QRegion sysClip = systemClip();
    if (!sysClip.isEmpty()) {
        devClipRect = sysClip.boundingRect();
        devClipRect.adjust(-BUFFERZONE, -BUFFERZONE, 2*BUFFERZONE, 2*BUFFERZONE);
    } else {
        devClipRect.setRect(-BUFFERZONE, -BUFFERZONE,
                            pdev->width() + 2*BUFFERZONE, pdev->height() + 2 * BUFFERZONE);
    }
    d->polygonClipper.setBoundingRect(devClipRect);
    d->floatClipper.setBoundingRect(devClipRect);
    setActive(true);

    QPixmap::x11SetDefaultScreen(d->xinfo->screen());

    assignf(IsActive | DirtyFont);

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
	XRenderPictureAttributes attrs;
	attrs.subwindow_mode = IncludeInferiors;
	XRenderChangePicture(d->dpy, d->picture, CPSubwindowMode, &attrs);
#endif
    } else if (d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = (QPixmap *)(pdev);
        if (!pm || pm->isNull()) {
            qWarning("QPainter::begin: Cannot paint null pixmap");
            end();
            return false;
        }
        if (pm->depth() == 1)
            setf(MonoDev);
    }

    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    setDirty(QPaintEngine::DirtyBackground);

    return true;
}

bool QX11PaintEngine::end()
{
    setActive(false);

#if !defined(QT_NO_XFT)
    if (d->picture) {
        // reset clipping/subwindow mode on our render picture
	XRenderPictureAttributes attrs;
	attrs.subwindow_mode = ClipByChildren;
        attrs.clip_mask = XNone;
	XRenderChangePicture(d->dpy, d->picture, CPClipMask|CPSubwindowMode, &attrs);
    }
#endif

    if (d->gc_brush) {
        XFreeGC(d->dpy, d->gc_brush);
        d->gc_brush = 0;
    }
    if (d->gc) {
        XFreeGC(d->dpy, d->gc);
        d->gc = 0;
    }

    return true;
}

void QX11PaintEngine::drawLines(const QLine *lines, int lineCount)
{
    Q_ASSERT(lines);
    Q_ASSERT(lineCount);
    if (d->use_path_fallback) {
        for (int i = 0; i < lineCount; ++i) {
            QPainterPath path(lines[i].p1());
            path.lineTo(lines[i].p2());
            drawPath(path);
        }
        return;
    }
    if (d->cpen.style() != Qt::NoPen) {
        for (int i = 0; i < lineCount; ++i)
            XDrawLine(d->dpy, d->hd, d->gc,
                      qRound(lines[i].x1()), qRound(lines[i].y1()),
                      qRound(lines[i].x2()), qRound(lines[i].y2()));
    }
}

void QX11PaintEngine::drawRects(const QRect *rects, int rectCount)
{
    Q_ASSERT(rects);
    Q_ASSERT(rectCount);

    if (d->use_path_fallback) {
        for (int i = 0; i < rectCount; ++i) {
            QPainterPath path;
            path.addRect(rects[i]);
            drawPath(path);
        }
        return;
    }

#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
    ::Picture pict = d->picture;

    if (X11->use_xrender && !testf(MonoDev) && pict && d->cbrush.style() != Qt::NoBrush
        && d->cbrush.color().alpha() != 255)
    {
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
        for (int i = 0; i < rectCount; ++i) {
            QRect r = rects[i].intersect(d->polygonClipper.boundingRect()).normalize();
            XRenderFillRectangle(d->dpy, PictOpOver, pict, &xc, r.x(), r.y(), r.width(), r.height());
            if (d->cpen.style() != Qt::NoPen)
                XDrawRectangle(d->dpy, d->hd, d->gc, r.x(), r.y(), r.width(), r.height());
        }
    } else
#endif // !QT_NO_XFT && !QT_NO_XRENDER
    {
        if (d->cbrush.style() != Qt::NoBrush && d->cpen.style() != Qt::NoPen) {
            for (int i = 0; i < rectCount; ++i) {
                QRect r = rects[i].intersect(d->polygonClipper.boundingRect()).normalize();
                if (d->cbrush.style() != Qt::NoBrush)
                    XFillRectangle(d->dpy, d->hd, d->gc_brush, r.x(), r.y(), r.width(), r.height());
                if (d->cpen.style() != Qt::NoPen)
                    XDrawRectangle(d->dpy, d->hd, d->gc, r.x(), r.y(), r.width(), r.height());
            }
        } else {
            QVarLengthArray<XRectangle> xrects(rectCount);
            for (int i = 0; i < rectCount; ++i) {
                QRect r = rects[i].intersect(d->polygonClipper.boundingRect()).normalize();
                xrects[i].x = short(r.x());
                xrects[i].y = short(r.y());
                xrects[i].width = ushort(r.width());
                xrects[i].height = ushort(r.height());
            }

            if (d->cbrush.style() != Qt::NoBrush && d->cpen.style() == Qt::NoPen) {
                XFillRectangles(d->dpy, d->hd, d->gc_brush, xrects.data(), rectCount);
            } else if (d->cpen.style() != Qt::NoPen && d->cbrush.style() == Qt::NoBrush) {
                XDrawRectangles(d->dpy, d->hd, d->gc, xrects.data(), rectCount);
            }
        }
    }
}

void QX11PaintEngine::drawPoints(const QPoint *points, int pointCount)
{
    Q_ASSERT(points);
    Q_ASSERT(pointCount);

    for (int i = 0; i < pointCount; ++i) {
        QPointF xformed = d->matrix.map(points[i]);
        if (d->cpen.style() != Qt::NoPen)
            XDrawPoint(d->dpy, d->hd, d->gc, qRound(xformed.x()), qRound(xformed.y()));
    }
}

QPainter::RenderHints QX11PaintEngine::supportedRenderHints() const
{
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
    if (X11->use_xrender)
        return QPainter::Antialiasing;
#endif
    return QFlag(0);
}

void QX11PaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    if ((d->txop > QPainterPrivate::TxNone)
        || (renderHints() & QPainter::Antialiasing))
        d->use_path_fallback = true;
    else
        d->use_path_fallback = false;

#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
    if (X11->use_xrender && d->picture) {
        XRenderPictureAttributes attrs;
        attrs.poly_edge = (hints & QPainter::Antialiasing) ? PolyEdgeSmooth : PolyEdgeSharp;
        XRenderChangePicture(d->dpy, d->picture, CPPolyEdge, &attrs);
    }
#else
    Q_UNUSED(hints);
#endif
}

void QX11PaintEngine::updatePen(const QPen &pen)
{
    d->cpen = pen;
    int ps = pen.style();
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

    ulong mask = GCForeground | GCBackground | GCGraphicsExposures | GCLineWidth
                 | GCLineStyle | GCCapStyle | GCJoinStyle;
    XGCValues vals;
    vals.graphics_exposures = false;
    if (d->pdev->depth() == 1) {
        vals.foreground = qGray(pen.color().rgb()) > 127 ? 0 : 1;
        vals.background = qGray(d->bg_col.rgb()) > 127 ? 0 : 1;
    } else if (d->pdev->devType() == QInternal::Pixmap && d->pdev->depth() == 32
        && X11->use_xrender && X11->has_xft) {
        vals.foreground = pen.color().rgba();
        vals.background = d->bg_col.rgba();
    } else {
        QColormap cmap = QColormap::instance(d->scrn);
        vals.foreground = cmap.pixel(pen.color());
        vals.background = cmap.pixel(d->bg_col);
    }
    vals.line_width = (! allow_zero_lw && pen.width() == 0) ? 1 : pen.width();
    vals.cap_style = cp;
    vals.join_style = jn;
    if (dash_len) {
        s = d->bg_mode == Qt::TransparentMode ? LineOnOffDash : LineDoubleDash;
    }
    vals.line_style = s;
    XChangeGC(d->dpy, d->gc, mask, &vals);

    if (dash_len)
        XSetDashes(d->dpy, d->gc, 0, dashes, dash_len);

    if (!hasClipping()) { // if clipping is set the paintevent clip region is merged with the clip region
        QRegion sysClip = systemClip();
        if (!sysClip.isEmpty())
            x11SetClipRegion(d->dpy, d->gc, 0, d->picture, sysClip);
        else
            x11ClearClipRegion(d->dpy, d->gc, 0, d->picture);
    }
}

void QX11PaintEngine::updateBrush(const QBrush &brush, const QPointF &origin)
{
    d->cbrush = brush;
    d->bg_origin = origin;

    int s  = FillSolid;
    ulong mask = GCForeground | GCBackground | GCGraphicsExposures
                 | GCLineStyle | GCCapStyle | GCJoinStyle | GCFillStyle;
    XGCValues vals;
    vals.graphics_exposures = false;
    if (d->pdev->depth() == 1) {
        vals.foreground = qGray(d->cbrush.color().rgb()) > 127 ? 0 : 1;
        vals.background = qGray(d->bg_col.rgb()) > 127 ? 0 : 1;
    } else if (d->pdev->devType() == QInternal::Pixmap && d->pdev->depth() == 32
        && X11->use_xrender && X11->has_xft) {
        vals.foreground = d->cbrush.color().rgba();
        vals.background = d->bg_col.rgba();
    } else {
        QColormap cmap = QColormap::instance(d->scrn);
        vals.foreground = cmap.pixel(d->cbrush.color());
        vals.background = cmap.pixel(d->bg_col);
    }
    vals.cap_style = CapButt;
    vals.join_style = JoinMiter;
    vals.line_style = LineSolid;

    vals.fill_style = s;
    XChangeGC(d->dpy, d->gc_brush, mask, &vals);
    if (!hasClipping()) {
        QRegion sysClip = systemClip();
        if (!sysClip.isEmpty())
            x11SetClipRegion(d->dpy, d->gc_brush, 0, d->picture, sysClip);
        else
            x11ClearClipRegion(d->dpy, d->gc_brush, 0, d->picture);
    }
}

void QX11PaintEngine::drawEllipse(const QRect &rect)
{
    if (d->use_path_fallback) {
        QPainterPath path;
        path.addEllipse(rect);
        drawPath(path);
        return;
    }

    int x = rect.x();
    int y = rect.y();
    int w = rect.width();
    int h = rect.height();
    if (w < 1 || h < 1)
        return;
    if (w == 1 && h == 1) {
        XDrawPoint(d->dpy, d->hd, (d->cpen.style() == Qt::NoPen) ? d->gc_brush : d->gc, x, y);
        return;
    }
    if (d->cbrush.style() != Qt::NoBrush) {          // draw filled ellipse
        if (d->cpen.style() == Qt::NoPen) {
            XFillArc(d->dpy, d->hd, d->gc_brush, x, y, w-1, h-1, 0, 360*64);
            XDrawArc(d->dpy, d->hd, d->gc_brush, x, y, w-1, h-1, 0, 360*64);
            return;
        } else{
            XFillArc(d->dpy, d->hd, d->gc_brush, x, y, w, h, 0, 360*64);
        }
    }
    if (d->cpen.style() != Qt::NoPen)                // draw outline
        XDrawArc(d->dpy, d->hd, d->gc, x, y, w, h, 0, 360*64);
}

#ifndef QT_NO_XRENDER
::Picture getSolidFill(int screen, const XRenderColor &color)
{
    for (int i = 0; i < X11->solid_fill_count; ++i) {
        if (X11->solid_fills[i].screen == screen
            && X11->solid_fills[i].color.alpha == color.alpha
            && X11->solid_fills[i].color.red == color.red
            && X11->solid_fills[i].color.green == color.green
            && X11->solid_fills[i].color.blue == color.blue)
            return X11->solid_fills[i].picture;
    }
    // none found, replace one
    int i = rand() % 16;

    if (X11->solid_fills[i].screen != screen && X11->solid_fills[i].picture) {
	XRenderFreePicture (X11->display, X11->solid_fills[i].picture);
	X11->solid_fills[i].picture = 0;
    }

    if (!X11->solid_fills[i].picture) {

	Pixmap pixmap = XCreatePixmap (X11->display, RootWindow (X11->display, screen), 1, 1, 32);
        XRenderPictureAttributes attrs;
	attrs.repeat = True;
	X11->solid_fills[i].picture = XRenderCreatePicture (X11->display, pixmap,
                                                            XRenderFindStandardFormat(X11->display, PictStandardARGB32),
                                                            CPRepeat, &attrs);
	XFreePixmap (X11->display, pixmap);
    }

    X11->solid_fills[i].color = color;
    X11->solid_fills[i].screen = screen;
    XRenderFillRectangle (X11->display, PictOpSrc, X11->solid_fills[i].picture, &color, 0, 0, 1, 1);
    return X11->solid_fills[i].picture;
}
#endif

void QX11PaintEnginePrivate::fillPolygon(const QPointF *polygonPoints, int pointCount,
                                         QX11PaintEnginePrivate::GCMode gcMode,
                                         QPaintEngine::PolygonDrawMode mode)
{
    int clippedCount = 0;
    qt_XPoint *clippedPoints = 0;

    QBrush fill;
    GC fill_gc;
    if (gcMode == QX11PaintEnginePrivate::BrushGC) {
        fill = cbrush;
        fill_gc = gc_brush;
    } else {
        fill = QBrush(cpen.brush());
        fill_gc = gc;
    }

#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
    bool antialias = q->renderHints() & QPainter::Antialiasing;
    if (X11->use_xrender && fill.style() != Qt::NoBrush &&
        (antialias || fill.color().alpha() != 255))
    {
        QPixmap gpix;
        int x_offset = 0;
        XRenderColor color;
        QColor qc = fill.color();

        const uint A = qc.alpha(),
                   R = qc.red(),
                   G = qc.green(),
                   B = qc.blue();

        color.alpha = (A | A << 8);
        color.red   = (R | R << 8) * color.alpha / 0x10000;
        color.green = (B | G << 8) * color.alpha / 0x10000;
        color.blue  = (B | B << 8) * color.alpha / 0x10000;
        ::Picture src = getSolidFill(d->scrn, color);

        if (src && d->picture) {
            int cCount;
            qt_float_point *cPoints;
            floatClipper.clipPolygon((qt_float_point *)polygonPoints, pointCount, &cPoints, &cCount);
            if (cCount > 0) {
                QVector<XTrapezoid> traps;
                traps.reserve(128);
                qt_tesselate_polygon(&traps, (QPointF *)cPoints, cCount,
                                     mode == QPaintEngine::WindingMode);

                XRenderPictureAttributes attrs;
                attrs.poly_edge = antialias ? PolyEdgeSmooth : PolyEdgeSharp;
                XRenderChangePicture(dpy, d->picture, CPPolyEdge, &attrs);
                XRenderCompositeTrapezoids(dpy, PictOpOver, src, d->picture,
                                           antialias ? XRenderFindStandardFormat(dpy, PictStandardA8) : 0,
                                           x_offset, 0, traps.constData(), traps.size());
            }
        }
    } else
#endif
        if (fill.style() != Qt::NoBrush) {
            if (mode == QPaintEngine::WindingMode)
                XSetFillRule(dpy, fill_gc, WindingRule);
            polygonClipper.clipPolygon((qt_float_point *) polygonPoints, pointCount,
                                       &clippedPoints, &clippedCount);
            if (clippedCount > 0)
                XFillPolygon(dpy, d->hd, fill_gc,
                             (XPoint *) clippedPoints, clippedCount,
                             mode == QPaintEngine::ConvexMode ? Convex : Complex, CoordModeOrigin);

            if (mode == QPaintEngine::WindingMode)
                XSetFillRule(dpy, fill_gc, EvenOddRule);
        }
}

void QX11PaintEnginePrivate::strokePolygon(const QPointF *polygonPoints, int pointCount)
{
   if (cpen.style() != Qt::NoPen) {
       int clippedCount = 0;
       qt_XPoint *clippedPoints = 0;
       d->polygonClipper.clipPolygon((qt_float_point *) polygonPoints, pointCount,
                                     &clippedPoints, &clippedCount, false);
       if (clippedCount > 0)
           XDrawLines(dpy, hd, gc, (XPoint *) clippedPoints, clippedCount, CoordModeOrigin);
    }
}

void QX11PaintEngine::drawPolygon(const QPointF *polygonPoints, int pointCount, PolygonDrawMode mode)
{
    if (d->use_path_fallback) {
        QPainterPath path(polygonPoints[0]);
        for (int i = 1; i < pointCount; ++i)
            path.lineTo(polygonPoints[i]);
        if (mode == PolylineMode) {
            QBrush oldBrush = d->cbrush;
            d->cbrush = QBrush(Qt::NoBrush);
            path.setFillRule(Qt::WindingFill);
            drawPath(path);
            d->cbrush = oldBrush;
        } else {
            path.setFillRule(mode == OddEvenMode ? Qt::OddEvenFill : Qt::WindingFill);
            path.closeSubpath();
            drawPath(path);
        }
        return;
    }
    if (mode != PolylineMode && d->cbrush.style() != Qt::NoBrush)
        d->fillPolygon(polygonPoints, pointCount, QX11PaintEnginePrivate::BrushGC, mode);

    if (d->cpen.style() != Qt::NoPen)
        d->strokePolygon(polygonPoints, pointCount);
}

void QX11PaintEngine::drawPath(const QPainterPath &path)
{
    if (path.isEmpty())
        return;
    if (d->cbrush.style() != Qt::NoBrush) {
        QPolygonF poly = path.toFillPolygon(state->matrix);
        d->fillPolygon(poly.data(), poly.size(), QX11PaintEnginePrivate::BrushGC,
                       path.fillRule() == Qt::OddEvenFill ? OddEvenMode : WindingMode);
    }

    if (d->cpen.style() != Qt::NoPen
        && (d->cpen.color().alpha() != 255
            || (d->cpen.widthF() > 0)
            || (renderHints() & QPainter::Antialiasing))) {
        QPainterPathStroker stroker;
        stroker.setDashPattern(d->cpen.style());
        stroker.setCapStyle(d->cpen.capStyle());
        stroker.setJoinStyle(d->cpen.joinStyle());
        QPainterPath stroke;
        qreal width = d->cpen.widthF();
        QPolygonF poly;
        if (width == 0) {
            stroker.setWidth(1);
            stroke = stroker.createStroke(path * state->matrix);
            if (stroke.isEmpty())
                return;
            poly = stroke.toFillPolygon();
        } else {
            stroker.setWidth(width);
            stroker.setCurveThreshold(width / (2 * 10 * d->matrix.m11() * d->matrix.m22()));
            stroke = stroker.createStroke(path);
            if (stroke.isEmpty())
                return;
            poly = stroke.toFillPolygon(state->matrix);
        }
        d->fillPolygon(poly.data(), poly.size(), QX11PaintEnginePrivate::PenGC, WindingMode);
    } else if (d->cpen.style() != Qt::NoPen) {
        // if we have a pen width of 0 - use XDrawLine() for speed
        QList<QPolygonF> polys = path.toSubpathPolygons(state->matrix);
        for (int i = 0; i < polys.size(); ++i)
            d->strokePolygon(polys.at(i).data(), polys.at(i).size());
    }
}

void qt_bit_blt(QPaintDevice *dst, int dx, int dy,
		const QPaintDevice *src, int sx, int sy, int sw, int sh,
		bool ignoreMask = false)
{
    if (!src || !dst) {
        Q_ASSERT(src != 0);
        Q_ASSERT(dst != 0);
        return;
    }
    if (!qt_x11Handle(src) || src->isExtDev())
        return;

    QPoint redirection_offset;
    const QPaintDevice *redirected = QPainter::redirected(dst, &redirection_offset);
    if (redirected) {
        dst = const_cast<QPaintDevice*>(redirected);
        dx -= redirection_offset.x();
        dy -= redirection_offset.y();
    }

    int ts = src->devType();                        // from device type
    int td = dst->devType();                        // to device type

    const QX11Info *src_xf = qt_x11Info(src),
                   *dst_xf = qt_x11Info(dst);

    Q_ASSERT(src_xf != 0 && dst_xf != 0);

    Display *dpy = src_xf->display();

    if (sw <= 0) {                                // special width
        if (sw < 0)
            sw = src->metric(QPaintDevice::PdmWidth) - sx;
        else
            return;
    }
    if (sh <= 0) {                                // special height
        if (sh < 0)
            sh = src->metric(QPaintDevice::PdmHeight) - sy;
        else
            return;
    }

    if (dst->paintingActive() && dst->isExtDev()) {
        QPixmap *pm;                                // output to picture/printer
        bool tmp_pm = true;
        if (ts == QInternal::Pixmap) {
            pm = (QPixmap*)src;
            if (sx != 0 || sy != 0 ||
		sw != pm->width() || sh != pm->height() || ignoreMask) {
                QPixmap *tmp = new QPixmap(sw, sh, pm->depth());
                qt_bit_blt(tmp, 0, 0, pm, sx, sy, sw, sh, true);
#if 0
                // ############## PIXMAP
                if (pm->mask() && !ignoreMask) {
                    QBitmap mask(sw, sh);
                    qt_bit_blt(&mask, 0, 0, pm->mask(), sx, sy, sw, sh, true);
                    tmp->setMask(mask);
                }
#endif
                pm = tmp;
            } else {
                tmp_pm = false;
            }
        } else if (ts == QInternal::Widget) {// bitBlt to temp pixmap
            pm = new QPixmap(sw, sh);
            qt_bit_blt(pm, 0, 0, src, sx, sy, sw, sh);
        } else {
            qWarning("bitBlt: Cannot bitBlt from device");
            return;
        }
	if (pm && dst->paintEngine())
	    dst->paintEngine()->drawPixmap(QRect(dx, dy, -1, -1), *pm, QRect(0, 0, -1, -1));

        if (tmp_pm)
            delete pm;
        return;
    }

    switch (ts) {
    case QInternal::Widget:
    case QInternal::Pixmap:
    case QInternal::System:                        // OK, can blt from these
        break;
    default:
        qWarning("bitBlt: Cannot bitBlt from device type %x", ts);
        return;
    }
    switch (td) {
    case QInternal::Widget:
    case QInternal::Pixmap:
    case QInternal::System:                        // OK, can blt to these
        break;
    default:
        qWarning("bitBlt: Cannot bitBlt to device type %x", td);
        return;
    }

    if (qt_x11Handle(dst) == 0) {
        qWarning("bitBlt: Cannot bitBlt to device");
        return;
    }

    bool mono_src;
    bool mono_dst;
    bool include_inferiors = false;
    bool graphics_exposure = false;
    QPixmap *src_pm;
    QBitmap *mask;

    if (ts == QInternal::Pixmap) {
        src_pm = (QPixmap*)src;
        if (src_pm->x11Info().screen() != dst_xf->screen())
            src_pm->x11SetScreen(dst_xf->screen());
        mono_src = src_pm->depth() == 1;
        mask = ignoreMask ? 0 : src_pm->data->mask;
    } else {
        src_pm = 0;
        mono_src = false;
        mask = 0;
        include_inferiors = ((QWidget*)src)->testAttribute(Qt::WA_PaintUnclipped);
        graphics_exposure = td == QInternal::Widget;
    }
    if (td == QInternal::Pixmap) {
        if (dst_xf->screen() != src_xf->screen())
            ((QPixmap*)dst)->x11SetScreen(src_xf->screen());
        mono_dst = ((QPixmap*)dst)->depth() == 1;
        ((QPixmap*)dst)->detach();                // changes shared pixmap
    } else {
        mono_dst = false;
        include_inferiors = include_inferiors ||
                            ((QWidget*)dst)->testAttribute(Qt::WA_PaintUnclipped);
    }

    if (mono_dst && !mono_src) {        // dest is 1-bit pixmap, source is not
        qWarning("bitBlt: Incompatible destination pixmap");
        return;
    }

#ifndef QT_NO_XRENDER
    if (X11->use_xrender && X11->has_xft && src_pm && !mono_src) {
        // use RENDER to do the blit
	Qt::HANDLE src_pict, dst_pict;
	if (src->devType() == QInternal::Widget)
	    src_pict = static_cast<const QWidget *>(src)->xftPictureHandle();
	else
	    src_pict = static_cast<const QPixmap *>(src)->xftPictureHandle();
	if (dst->devType() == QInternal::Widget)
	    dst_pict = static_cast<const QWidget *>(dst)->xftPictureHandle();
	else
	    dst_pict = static_cast<const QPixmap *>(dst)->xftPictureHandle();
        Q_ASSERT_X(dst_pict && src_pict, "qt_bit_blt", "internal error");

        XRenderPictureAttributes pattr;
        ulong picmask = 0;
        if (include_inferiors) {
            pattr.subwindow_mode = IncludeInferiors;
            picmask |= CPSubwindowMode;
        }
        if (graphics_exposure) {
            pattr.graphics_exposures = true;
            picmask |= CPGraphicsExposure;
        }
        if (picmask)
            XRenderChangePicture(dpy, dst_pict, picmask, &pattr);
        XRenderComposite(dpy, !ignoreMask ? PictOpOver : PictOpSrc, src_pict,
                         (mask && !ignoreMask) ? mask->xftPictureHandle() : 0,
                         dst_pict, sx, sy, sx, sy, dx, dy, sw, sh);
        // restore attributes
        pattr.subwindow_mode = ClipByChildren;
        pattr.graphics_exposures = false;
        if (picmask)
            XRenderChangePicture(dpy, dst_pict, picmask, &pattr);
        return;
    }
#endif


    if (mask && !mono_src) {                        // fast masked blt
        // Create a new mask GC. If BestOptim, we store the mask GC
        // with the mask (not at the pixmap). This way, many pixmaps
        // which have a common mask will be optimized at no extra cost.
        GC gc = XCreateGC(dpy, qt_x11Handle(dst), 0, 0);
        XSetGraphicsExposures(dpy, gc, False);
        XSetClipMask(dpy, gc, mask->handle());
        XSetClipOrigin(dpy, gc, dx-sx, dy-sy);
        if (include_inferiors) {
            XSetSubwindowMode(dpy, gc, IncludeInferiors);
            XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
            XSetSubwindowMode(dpy, gc, ClipByChildren);
        } else {
            XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
        }

        XFreeGC(dpy, gc);
        return;
    }

    GC gc = XCreateGC(dpy, qt_x11Handle(dst), 0, 0);
    if (mono_src && mono_dst && src == dst) { // dst and src are the same bitmap
        XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
    } else if (mono_src) {                        // src is bitmap
        XGCValues gcvals;
        ulong          valmask = GCBackground | GCForeground | GCFillStyle |
				 GCStipple | GCTileStipXOrigin | GCTileStipYOrigin;
        if (td == QInternal::Widget) {        // set GC colors
            QWidget *w = (QWidget *)dst;
            QColormap cmap = QColormap::instance(dst_xf->screen());
            gcvals.background = cmap.pixel(w->palette().color(w->backgroundRole()));
            gcvals.foreground = cmap.pixel(w->palette().color(w->foregroundRole()));
            if (include_inferiors) {
                valmask |= GCSubwindowMode;
                gcvals.subwindow_mode = IncludeInferiors;
            }
        } else if (mono_dst) {
            gcvals.background = 0;
            gcvals.foreground = 1;
        } else if (td == QInternal::Pixmap && dst->depth() == 32
                   && X11->use_xrender && X11->has_xft) {
            gcvals.background = 0xffffffff; // white, fully opaque
            gcvals.foreground = 0xff000000; // black, fully opaque
        } else {
            gcvals.background = WhitePixel(dpy, dst_xf->screen());
            gcvals.foreground = BlackPixel(dpy, dst_xf->screen());
        }

        gcvals.fill_style  = FillOpaqueStippled;
        gcvals.stipple = qt_x11Handle(src);
        gcvals.ts_x_origin = dx - sx;
	gcvals.ts_y_origin = dy - sy;

        if (mask) {
            if (((QPixmap*)src)->data->selfmask) {
                gcvals.fill_style = FillStippled;
            } else {
                XSetClipMask(dpy, gc, mask->handle());
                XSetClipOrigin(dpy, gc, dx-sx, dy-sy);
	    }
	}

        XChangeGC(dpy, gc, valmask, &gcvals);
	XFillRectangle(dpy, qt_x11Handle(dst), gc, dx, dy, sw, sh);
    } else {                                        // src is pixmap/widget
	if (graphics_exposure)                // widget to widget
	    XSetGraphicsExposures(dpy, gc, True);
	if (include_inferiors)
	    XSetSubwindowMode(dpy, gc, IncludeInferiors);
        XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
    }
    XFreeGC(dpy, gc);
}


void QX11PaintEngine::drawPixmap(const QRectF &r, const QPixmap &pixmap, const QRectF &sr,
                                 Qt::PixmapDrawingMode mode)
{
    int x = qRound(r.x());
    int y = qRound(r.y());
    int sx = qRound(sr.x());
    int sy = qRound(sr.y());
    int sw = qRound(sr.width());
    int sh = qRound(sr.height());
    // since we can't scale pixmaps this should always hold
    Q_ASSERT(r.width() == sr.width() && r.height() == sr.height());

    if (d->xinfo && d->xinfo->screen() != pixmap.x11Info().screen()) {
        QPixmap* p = const_cast<QPixmap *>(&pixmap);
        p->x11SetScreen(d->xinfo->screen());
    }

    QPixmap::x11SetDefaultScreen(pixmap.x11Info().screen());

    // ################### PIXMAP: should be able to do this better...
    QBitmap mask;
    if(mode != Qt::CopyPixmapNoMask && !pixmap.hasAlphaChannel())
        mask = pixmap.mask();
    bool mono_src = pixmap.depth() == 1;
    // bool mono_dst = d->pdev->depth() == 1;

    if (mono_src && mode == Qt::CopyPixmap) {
	qt_bit_blt(d->pdev, x, y, &pixmap, sx, sy, sw, sh, true);
        return;
    }

    if (!mask.isNull() && !hasClipping() && systemClip().isEmpty()) {
        if (mono_src) {                           // needs GCs pen // color
            bool selfmask = pixmap.data->selfmask;
            if (selfmask) {
                XSetFillStyle(d->dpy, d->gc, FillStippled);
                XSetStipple(d->dpy, d->gc, pixmap.handle());
            } else {
                XSetFillStyle(d->dpy, d->gc, FillOpaqueStippled);
                XSetStipple(d->dpy, d->gc, pixmap.handle());
                XSetClipMask(d->dpy, d->gc, mask.handle());
                XSetClipOrigin(d->dpy, d->gc, x-sx, y-sy);
            }
            XSetTSOrigin(d->dpy, d->gc, x-sx, y-sy);
            XFillRectangle(d->dpy, d->hd, d->gc, x, y, sw, sh);
            XSetTSOrigin(d->dpy, d->gc, 0, 0);
            XSetFillStyle(d->dpy, d->gc, FillSolid);
            if (!selfmask) {
                QRegion sysClip = systemClip();
                if (!sysClip.isEmpty()) {
                    x11SetClipRegion(d->dpy, d->gc, 0, d->picture, sysClip);
                } else {
                    x11ClearClipRegion(d->dpy, d->gc, 0, d->picture);
                }
            }
        } else {
	    qt_bit_blt(d->pdev, x, y, &pixmap, sx, sy, sw, sh, mode == Qt::CopyPixmapNoMask ? true : false);
            if (mode == Qt::CopyPixmap && d->pdev->devType() == QInternal::Pixmap) {
                QPixmap *px = static_cast<QPixmap *>(d->pdev);
                px->data->alpha = pixmap.data->alpha;
            }
        }
        return;
    }


    if (!mask.isNull()) { // pixmap has clip mask
        // Implies that clipping is on, either explicit or implicit
        // Create a new mask that combines the mask with the clip region

	QRegion rgn = d->crgn;
        QRegion sysClip = systemClip();
        if (!sysClip.isEmpty()) {
            if (hasClipping())
                rgn = rgn.intersect(sysClip);
            else
                rgn = sysClip;
        }

        QBitmap comb(sw, sh);
        GC cgc = XCreateGC(d->dpy, comb.handle(), 0, 0);
        XSetForeground(d->dpy, cgc, 0);
        XFillRectangle(d->dpy, comb.handle(), cgc, 0, 0, sw, sh);
        XSetBackground(d->dpy, cgc, 0);
        XSetForeground(d->dpy, cgc, 1);
        int num;
        XRectangle *rects = (XRectangle *)qt_getClipRects(rgn, num);
        XSetClipRectangles(d->dpy, cgc, -x, -y, rects, num, Unsorted);
        XSetFillStyle(d->dpy, cgc, FillOpaqueStippled);
        XSetStipple(d->dpy, cgc, mask.handle());
        XSetTSOrigin(d->dpy, cgc, -sx, -sy);
        XFillRectangle(d->dpy, comb.handle(), cgc, 0, 0, sw, sh);
        XFreeGC(d->dpy, cgc);
        mask = comb;                            // it's deleted below

        XSetClipMask(d->dpy, d->gc, mask.handle());
        XSetClipOrigin(d->dpy, d->gc, x, y);
    }
    /* if (mono_src && mono_dst) {
        XCopyArea(d->dpy, pixmap.handle(), d->hd, d->gc, sx, sy, sw, sh, x, y);
        } else*/
    if (mono_src) {
        XSetClipMask(d->dpy, d->gc, pixmap.handle());
        XSetClipOrigin(d->dpy, d->gc, x-sx, y-sy);
        if (d->pdev->depth() == 1) {
            XSetBackground(d->dpy, d->gc, qGray(d->bg_brush.color().rgb()) > 127 ? 0 : 1);
            XSetForeground(d->dpy, d->gc, qGray(d->cpen.color().rgb()) > 127 ? 0 : 1);
        } else if (d->pdev->devType() == QInternal::Pixmap && d->pdev->depth() == 32
                   && X11->use_xrender && X11->has_xft) {
            XSetBackground(d->dpy, d->gc, d->bg_brush.color().rgba());
            XSetForeground(d->dpy, d->gc, d->cpen.color().rgba());
        } else {
            QColormap cmap = QColormap::instance(d->scrn);
            XSetBackground(d->dpy, d->gc, cmap.pixel(d->bg_brush.color()));
            XSetForeground(d->dpy, d->gc, cmap.pixel(d->cpen.color()));
        }
        XFillRectangle(d->dpy, d->hd, d->gc, x, y, sw, sh);
        XSetClipMask(d->dpy, d->gc, XNone);
        XSetClipOrigin(d->dpy, d->gc, 0, 0);
    } else {
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
        ::Picture src_pict = pixmap.xftPictureHandle();
        if (X11->use_xrender && X11->has_xft && src_pict && d->picture) {
            ::Picture msk_pict = ((mode == Qt::ComposePixmap
                                   && !pixmap.data->alpha
                                   && pixmap.data->mask)
                                  ? pixmap.data->mask->xftPictureHandle()
                                  : 0);
            XRenderComposite(d->dpy, mode == Qt::ComposePixmap ? PictOpOver : PictOpSrc,
                             src_pict, msk_pict, d->picture, sx, sy, sx, sy, x, y, sw, sh);
            if (mode == Qt::CopyPixmap && d->pdev->devType() == QInternal::Pixmap) {
                QPixmap *px = static_cast<QPixmap *>(d->pdev);
                px->data->alpha = pixmap.data->alpha;
            }
        } else
#endif // !QT_NO_XFT && !QT_NO_XRENDER
            {
                XCopyArea(d->dpy, pixmap.handle(), d->hd, d->gc, sx, sy, sw, sh, x, y);
            }
    }

    if (!mask.isNull()) { // restore clipping
        XSetClipOrigin(d->dpy, d->gc, 0, 0);
        int num;
        XRectangle *rects = (XRectangle *)qt_getClipRects(d->crgn, num);
        if (num == 0)
            XSetClipMask(d->dpy, d->gc, XNone);
        else
            XSetClipRectangles(d->dpy, d->gc, 0, 0, rects, num, Unsorted);
    }
}

void QX11PaintEngine::updateBackground(Qt::BGMode mode, const QBrush &bgBrush)
{
    Q_ASSERT(isActive());
    d->bg_mode = mode;
    d->bg_brush = bgBrush;
    d->bg_col = bgBrush.color();
    updatePen(d->cpen);
    updateBrush(d->cbrush, d->bg_origin);
}

void QX11PaintEngine::updateMatrix(const QMatrix &mtx)
{
    d->matrix = mtx;
    if (mtx.m12() != 0 || mtx.m21() != 0)
        d->txop = QPainterPrivate::TxRotShear;
    else if (mtx.m11() != 1 || mtx.m22() != 1)
        d->txop = QPainterPrivate::TxScale;
    else if (mtx.dx() != 0 || mtx.dy() != 0)
        d->txop = QPainterPrivate::TxTranslate;
    else
        d->txop = QPainterPrivate::TxNone;

    if ((d->txop > QPainterPrivate::TxNone)
        || (renderHints() & QPainter::Antialiasing))
        d->use_path_fallback = true;
    else
        d->use_path_fallback = false;
}

void QX11PaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    QRegion sysClip = systemClip();
    if (op == Qt::NoClip) {
        clearf(ClipOn);
        d->crgn = QRegion();
        if (!sysClip.isEmpty()) {
            x11SetClipRegion(d->dpy, d->gc, d->gc_brush, d->picture, sysClip);
        } else {
            x11ClearClipRegion(d->dpy, d->gc, d->gc_brush, d->picture);
        }
        return;
    }

    switch (op) {
    case Qt::IntersectClip:
        if (testf(ClipOn)) {
            d->crgn &= clipRegion;
            break;
        }
        // fall through
    case Qt::ReplaceClip:
        if (!sysClip.isEmpty())
            d->crgn = clipRegion.intersect(sysClip);
        else
            d->crgn = clipRegion;
        break;
    case Qt::UniteClip:
        d->crgn |= clipRegion;
        break;
    case Qt::NoClip:
        break;
    }

    setf(ClipOn);
    x11SetClipRegion(d->dpy, d->gc, d->gc_brush, d->picture, d->crgn);
}

void QX11PaintEngine::updateFont(const QFont &)
{
    clearf(DirtyFont);
}

Qt::HANDLE QX11PaintEngine::handle() const
{
    Q_ASSERT(isActive());
    Q_ASSERT(d->hd);
    return d->hd;
}

extern void qt_draw_tile(QPaintEngine *, qreal, qreal, qreal, qreal, const QPixmap &,
                         qreal, qreal, Qt::PixmapDrawingMode mode);

void QX11PaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &p,
				      Qt::PixmapDrawingMode mode)
{
    int x = qRound(r.x());
    int y = qRound(r.y());
    int w = qRound(r.width());
    int h = qRound(r.height());
    int sx = qRound(p.x());
    int sy = qRound(p.y());

    if (pixmap.depth() > 1 && d->txop <= QPainterPrivate::TxTranslate && pixmap.hasAlphaChannel()) {
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
        if (d->picture && pixmap.xftPictureHandle()) {
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
                    XRenderComposite(d->dpy, (pixmap.data->alpha ? PictOpOver : PictOpSrc),
                                     pixmap.xftPictureHandle(), XNone,
                                     d->picture, xOff, yOff, xOff, yOff, xPos, yPos, drawW, drawH);
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
	qt_draw_tile(this, x, y, w, h, pixmap, sx, sy, mode);
    }
}

static void drawLines(QPaintEngine *p, const QTextItemInt &ti, int baseline, int x1, int w)
{
    int lw = qRound(ti.fontEngine->lineThickness());
    if (ti.flags & QTextItem::Underline) {
        int pos = qRound(ti.fontEngine->underlinePosition());
        qt_draw_transformed_rect(p, x1, baseline+pos, w, lw, true);
    }
    if (ti.flags & QTextItem::Overline) {
        int pos = qRound(ti.fontEngine->ascent()+1);
        if (!pos) pos = 1;
        qt_draw_transformed_rect(p, x1, baseline-pos, w, lw, true);
    }
    if (ti.flags & QTextItem::StrikeOut) {
        int pos = qRound(ti.fontEngine->ascent()/3);
        if (!pos) pos = 1;
        qt_draw_transformed_rect(p, x1, baseline-pos, w, lw, true);
    }
}


void QX11PaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    if (!ti.num_glyphs)
        return;

    switch(ti.fontEngine->type()) {
    case QFontEngine::Multi:
        drawMulti(p, ti);
        break;
    case QFontEngine::Box:
        drawBox(p, ti);
        break;
    case QFontEngine::XLFD:
        drawXLFD(p, ti);
        break;
#ifndef QT_NO_XFT
    case QFontEngine::Xft:
        drawXft(p, ti);
        break;
#endif
    default:
        Q_ASSERT(false);
    }
}

void QX11PaintEngine::drawMulti(const QPointF &p, const QTextItemInt &ti)
{
    QFontEngineMulti *multi = static_cast<QFontEngineMulti *>(ti.fontEngine);
    QGlyphLayout *glyphs = ti.glyphs;
    int which = glyphs[0].glyph >> 24;

    qreal x = p.x();
    qreal y = p.y();

    int start = 0;
    int end, i;
    for (end = 0; end < ti.num_glyphs; ++end) {
        const int e = glyphs[end].glyph >> 24;
        if (e == which)
            continue;

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

        // draw the text
        QTextItemInt ti2 = ti;
        ti2.glyphs = ti.glyphs + start;
        ti2.num_glyphs = end - start;
        ti2.fontEngine = multi->engine(which);
        ti2.f = ti.f;
        drawTextItem(QPointF(x, y), ti2);

        // reset the high byte for all glyphs and advance to the next sub-string
        const int hi = which << 24;
        for (i = start; i < end; ++i) {
            glyphs[i].glyph = hi | glyphs[i].glyph;
            x += glyphs[i].advance.x();
        }

        // change engine
        start = end;
        which = e;
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

    // draw the text
    QTextItemInt ti2 = ti;
    ti2.glyphs = ti.glyphs + start;
    ti2.num_glyphs = end - start;
    ti2.fontEngine = multi->engine(which);
    ti2.f = ti.f;
    drawTextItem(QPointF(x,y), ti2);

    // reset the high byte for all glyphs
    const int hi = which << 24;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;
}

void QX11PaintEngine::drawBox(const QPointF &p, const QTextItemInt &ti)
{
    int size = qRound(ti.fontEngine->ascent());
    int x = qRound(p.x());
    int y = qRound(p.y());
    int s = size - 3;

    if (d->txop > QPainterPrivate::TxTranslate) {
        for (int k = 0; k < ti.num_glyphs; k++) {
            qt_draw_transformed_rect(this, x, y, s, s, false);
            x += size;
        }
    } else {
        if (d->txop == QPainterPrivate::TxTranslate) {
            x += qRound(state->matrix.dx());
            y += qRound(state->matrix.dy());
        }
        XRectangle rects[64];

        int gl = 0;
        while (gl < ti.num_glyphs) {
            int toDraw = qMin(64, ti.num_glyphs-gl);
            int adv = toDraw * size;
            if (x + adv < SHRT_MAX && x > SHRT_MIN) {
                for (int k = 0; k < toDraw; k++) {
                    rects[k].x = x + (k * size);
                    rects[k].y = y - size + 2;
                    rects[k].width = rects[k].height = s;
                }
                XDrawRectangles(d->dpy, d->hd, d->gc, rects, toDraw);
            }
            gl += toDraw;
            x += adv;
        }
    }

    if (ti.flags)
        ::drawLines(this, ti, qRound(p.y()), qRound(p.x()), ti.num_glyphs*size);
}


void QX11PaintEngine::drawXLFD(const QPointF &p, const QTextItemInt &si)
{

    QFontEngineXLFD *xlfd = static_cast<QFontEngineXLFD *>(si.fontEngine);
    Qt::HANDLE font_id = xlfd->fontStruct()->fid;
    if (d->txop > QPainterPrivate::TxTranslate) {
        // XServer or font don't support server side transformations, need to do it by hand
        QPaintEngine::drawTextItem(p, si);
        return;
    }

    qreal x = p.x();
    qreal y = p.y();

    if (d->txop == QPainterPrivate::TxTranslate) {
        x += state->matrix.dx();
        y += state->matrix.dy();
    }

    XSetFont(d->dpy, d->gc, font_id);

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush(Qt::white);
    glyph_metrics_t ci = boundingBox(glyphs, si.num_glyphs);
    p->drawRect(x + ci.x, y + ci.y, ci.width, ci.height);
    p->drawRect(x + ci.x, y + 100 + ci.y, ci.width, ci.height);
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height);
    p->restore();
    int xp = x;
    int yp = y;
#endif

    QGlyphLayout *glyphs = si.glyphs;

    if (si.flags)
        ::drawLines(this, si, qRound(p.y()), qRound(p.x()), qRound(si.width));

    QVarLengthArray<XChar2b> chars(si.num_glyphs);

    for (int i = 0; i < si.num_glyphs; i++) {
        chars[i].byte1 = glyphs[i].glyph >> 8;
        chars[i].byte2 = glyphs[i].glyph & 0xff;
    }

    if (si.flags & QTextItem::RightToLeft) {
        int i = si.num_glyphs;
        while(i--) {
            x += glyphs[i].advance.x() + qreal(glyphs[i].space_18d6)/qreal(64);
            y += glyphs[i].advance.y();
        }
        i = 0;
        while(i < si.num_glyphs) {
            x -= glyphs[i].advance.x();
            y -= glyphs[i].advance.y();

            int xp = qRound(x+glyphs[i].offset.x());
            int yp = qRound(y+glyphs[i].offset.y());
            if (xp < SHRT_MAX && xp > SHRT_MIN)
                XDrawString16(d->dpy, d->hd, d->gc, xp, yp, chars.data()+i, 1);

            if (glyphs[i].nKashidas) {
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                si.fontEngine->stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < glyphs[i].nKashidas; ++k) {
                    x -= g[0].advance.x();
                    y -= g[0].advance.y();

                    int xp = qRound(x+g[0].offset.x());
                    int yp = qRound(y+g[0].offset.y());
                    if (xp < SHRT_MAX && xp > SHRT_MIN)
                        XDrawString16(d->dpy, d->hd, d->gc, xp, yp, chars.data()+i, 1);
                }
            } else {
                x -= qreal(glyphs[i].space_18d6)/qreal(64);
            }
            ++i;
        }
    } else {
        int i = 0;
        while(i < si.num_glyphs) {
            int xp = qRound(x+glyphs[i].offset.x());
            int yp = qRound(y+glyphs[i].offset.y());
            if (xp < SHRT_MAX && xp > SHRT_MIN)
                XDrawString16(d->dpy, d->hd, d->gc, xp, yp, chars.data()+i, 1);
            x += glyphs[i].advance.x() + qreal(glyphs[i].space_18d6)/qreal(64);
            y += glyphs[i].advance.y();
            i++;
        }
    }

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen(Qt::red);
    for (int i = 0; i < si.num_glyphs; i++) {
        glyph_metrics_t ci = boundingBox(glyphs[i].glyph);
        p->drawRect(x + ci.x + glyphs[i].offset.x, y + 100 + ci.y + glyphs[i].offset.y, ci.width, ci.height);
        qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
               ci.xoff, ci.yoff, glyphs[i].offset.x, glyphs[i].offset.y,
               glyphs[i].advance.x, glyphs[i].advance.y);
        x += glyphs[i].advance.x;
        y += glyphs[i].advance.y;
    }
    p->restore();
#endif
}

#ifndef QT_NO_XFT
void QX11PaintEngine::drawXft(const QPointF &p, const QTextItemInt &si)
{
    qreal xpos = p.x();
    qreal ypos = p.y();

    QFontEngineXft *xft = static_cast<QFontEngineXft *>(si.fontEngine);
    XftFont *fnt = d->txop >= QPainterPrivate::TxScale ? xft->transformedFont(state->matrix) : xft->xftFont();
    XftDraw *draw = d->xft_hd;
    int screen = d->scrn;

    bool transform = d->txop >= QPainterPrivate::TxScale;

    if (d->txop == QPainterPrivate::TxTranslate) {
        xpos += state->matrix.dx();
        ypos += state->matrix.dy();
    }

    QPointF pos(xpos, ypos);

    QGlyphLayout *glyphs = si.glyphs;

    const QColor &pen = d->cpen.color();
    XftColor col;
    col.color.red = pen.red () | pen.red() << 8;
    col.color.green = pen.green () | pen.green() << 8;
    col.color.blue = pen.blue () | pen.blue() << 8;
    col.color.alpha = 0xffff;
    // ###
    col.pixel = QColormap::instance(screen).pixel(pen);

#ifdef FONTENGINE_DEBUG
    qDebug("===== drawing %d glyphs reverse=%s ======", si.num_glyphs,
           si.flags & QTextItem::RightToLeft ?"true":"false");
    p->painter()->save();
    p->painter()->setBrush(Qt::white);
    glyph_metrics_t ci = boundingBox(glyphs, si.num_glyphs);
    p->painter()->drawRect(x + ci.x, y + ci.y, ci.width, ci.height);
    p->painter()->drawLine(x + ci.x, y, ci.width, y);
    p->painter()->drawRect(x + ci.x, y + 100 + ci.y, ci.width, ci.height);
    p->painter()->drawLine(x + ci.x, y + 100, ci.width, y + 100);
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height);
    p->painter()->restore();
#endif

    if (si.flags)
        ::drawLines(this, si, p.toPoint().y(), p.toPoint().x(), qRound(si.width));

    QVarLengthArray<XftGlyphSpec,256> glyphSpec(si.num_glyphs);

#ifdef FONTENGINE_DEBUG
    p->painter()->save();
    p->painter()->setPen(Qt::red);
#endif

    int nGlyphs = 0;

    if (si.flags & QTextItem::RightToLeft) {
        int i = si.num_glyphs;
        while(i--) {
            pos.rx() += glyphs[i].advance.x() + qreal(glyphs[i].space_18d6)/qreal(64);
            pos.ry() += glyphs[i].advance.y();
        }
        i = 0;
        while(i < si.num_glyphs) {
            pos -= glyphs[i].advance;

            QPointF gpos = pos + glyphs[i].offset;
            if (transform)
                gpos = gpos * state->matrix;
            int xp = qRound(gpos.x());
            int yp = qRound(gpos.y());
            if (xp > SHRT_MIN && xp < SHRT_MAX) {
                glyphSpec[nGlyphs].x = xp;
                glyphSpec[nGlyphs].y = yp;
                glyphSpec[nGlyphs].glyph = glyphs[i].glyph;
                ++nGlyphs;
            }
            if (glyphs[i].nKashidas) {
                glyphSpec.resize(glyphSpec.size() + glyphs[i].nKashidas);
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                si.fontEngine->stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < glyphs[i].nKashidas; ++k) {
                    pos -= g[0].advance;

                    QPointF gpos(pos);
                    if (transform)
                        gpos = gpos * state->matrix;
                    int xp = qRound(gpos.x());
                    int yp = qRound(gpos.y());
                    if (xp > SHRT_MIN && xp < SHRT_MAX) {
                        glyphSpec[nGlyphs].x = xp;
                        glyphSpec[nGlyphs].y = yp;
                        glyphSpec[nGlyphs].glyph = g[0].glyph;
                    }
                    ++nGlyphs;
                }
            } else {
                pos.rx() -= qreal(glyphs[i].space_18d6)/qreal(64);
            }
#ifdef FONTENGINE_DEBUG
            glyph_metrics_t ci = boundingBox(glyphs[i].glyph);
            p->painter()->drawRect(x + ci.x + glyphs[i].offset.x, y + 100 + ci.y + glyphs[i].offset.y, ci.width, ci.height);
            qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
                   ci.xoff, ci.yoff, glyphs[i].offset.x, glyphs[i].offset.y, glyphs[i].advance.x, glyphs[i].advance.y);
#endif
            ++i;
        }
    } else {
        int i = 0;
        while (i < si.num_glyphs) {
            QPointF gpos = pos;
            gpos += glyphs[i].offset;
            if (transform)
                gpos = gpos * state->matrix;
            int xp = qRound(gpos.x());
            int yp = qRound(gpos.y());
            if (xp > SHRT_MIN && xp < SHRT_MAX) {
                glyphSpec[i].x = xp;
                glyphSpec[i].y = yp;
                glyphSpec[i].glyph = glyphs[i].glyph;
                ++nGlyphs;
            }
#ifdef FONTENGINE_DEBUG
            glyph_metrics_t ci = boundingBox(glyphs[i].glyph);
            qDebug("bounding %d ci[%x]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, glyphs[i].glyph,
                   ci.x, ci.y, ci.width, ci.height, ci.xoff, ci.yoff,
                   glyphs[i].offset.x, glyphs[i].offset.y, glyphs[i].advance.x, glyphs[i].advance.y);
#endif

            pos.rx() += glyphs[i].advance.x() + qreal(glyphs[i].space_18d6)/qreal(64);
            pos.ry() += glyphs[i].advance.y();
            ++i;
        }
    }

#ifdef FONTENGINE_DEBUG
    p->painter()->restore();
#endif

    int i = 0;
    while (i < nGlyphs) {
        int toDraw = qMin(64, nGlyphs-i);
        XftDrawGlyphSpec(draw, &col, fnt, glyphSpec.data()+i, toDraw);
        i += toDraw;
    }

}
#endif // !QT_NO_XFT

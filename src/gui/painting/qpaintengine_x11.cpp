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
                                    XftDraw *draw,
#else
                                    Qt::HANDLE draw,
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
    if (draw)
        XftDrawSetClipRectangles(draw, 0, 0, rects, num);
#else
    Q_UNUSED(draw);
#endif // QT_NO_XFT
}

static inline void x11ClearClipRegion(Display *dpy, GC gc, GC gc2,
#ifndef QT_NO_XFT
                                    XftDraw *draw
#else
                                    Qt::HANDLE draw
#endif
                                      )
{
    if (gc)
        XSetClipMask(dpy, gc, XNone);
    if (gc2)
        XSetClipMask(dpy, gc2, XNone);

#ifndef QT_NO_XFT
    if (draw)
        XftDrawSetClip(draw, 0);
#else
    Q_UNUSED(draw);
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
    ulong pixel = QColormap::instance(screen).pixel(brush.color());
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


void qt_draw_background(QPaintEngine *pe, int x, int y, int w,  int h)
{
    QX11PaintEngine *p = static_cast<QX11PaintEngine *>(pe);
    QColormap cmap = QColormap::instance(p->d->scrn);
    XSetForeground(p->d->dpy, p->d->gc, cmap.pixel(p->d->bg_brush.color()));
    qt_draw_transformed_rect(p, x, y, w, h, true);
    XSetForeground(p->d->dpy, p->d->gc, cmap.pixel(p->d->cpen.color()));
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
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
                   | AlphaFillPolygon
                   | AlphaPixmap
                   | FillAntialiasing
#endif
        )
{
    d->dpy = 0;
    d->scrn = 0;
    d->hd = 0;
    d->xft_hd = 0;
    d->xinfo = 0;
    if (!X11->use_xrender) {
        gccaps &= ~AlphaFillPolygon;
        gccaps &= ~AlphaPixmap;
        gccaps &= ~FillAntialiasing;
    }
}

QX11PaintEngine::QX11PaintEngine(QX11PaintEnginePrivate &dptr)
    : QPaintEngine(dptr, UsesFontEngine
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
                   | AlphaFillPolygon
                   | AlphaPixmap
                   | FillAntialiasing
#endif
        )
{
    d->dpy = 0;
    d->scrn = 0;
    d->hd = 0;
    d->xft_hd = 0;
    d->xinfo = 0;
    if (!X11->use_xrender) {
        gccaps &= ~AlphaFillPolygon;
        gccaps &= ~AlphaPixmap;
        gccaps &= ~FillAntialiasing;
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
    if (pdev->devType() == QInternal::Widget)
        d->xft_hd = (XftDraw *)static_cast<const QWidget *>(pdev)->xftDrawHandle();
    else if (pdev->devType() == QInternal::Pixmap)
        d->xft_hd = (XftDraw *)static_cast<const QPixmap *>(pdev)->xftDrawHandle();
#else
    d->xft_hd = 0;
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

    // Set up the polygon clipper. Note: This will only work in
    // polyline mode as long as we have a buffer zone, since a
    // polyline may be clipped into several non-connected polylines.
    const int BUFFERZONE = 100;
    QRect devClipRect;
    QRegion sysClip = systemClip();
    if (!sysClip.isEmpty()) {
        devClipRect = sysClip.boundingRect();
        devClipRect.addCoords(-BUFFERZONE, -BUFFERZONE, 2*BUFFERZONE, 2*BUFFERZONE);
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
        if (d->xft_hd)
                XftDrawSetSubwindowMode(d->xft_hd, IncludeInferiors);
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
    if (d->xft_hd) {
        // reset clipping/subwindow mode on our render picture
        XftDrawSetClip(d->xft_hd, 0);
        XftDrawSetSubwindowMode(d->xft_hd, ClipByChildren);
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

void QX11PaintEngine::drawLine(const QLineF &line)
{
    if (!isActive())
        return;
    if (d->cpen.style() != Qt::NoPen)
        XDrawLine(d->dpy, d->hd, d->gc,
                  qRound(line.startX()), qRound(line.startY()),
                  qRound(line.endX()), qRound(line.endY()));
}

void QX11PaintEngine::drawRect(const QRectF &rect)
{
    if (!isActive())
        return;

    QRect r = rect.toRect().intersect(d->polygonClipper.boundingRect()).normalize();

#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
    ::Picture pict = d->xft_hd ? XftDrawPicture(d->xft_hd) : 0;

    if (X11->use_xrender && !testf(MonoDev) && pict && d->cbrush.style() != Qt::NoBrush
        && d->cbrush.color().alpha() != 255) {
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
	    XRenderFillRectangle(d->dpy, PictOpOver, pict, &xc, r.x(), r.y(), r.width(), r.height());
	    return;
	}
        XRenderFillRectangle(d->dpy, PictOpOver, pict, &xc, r.x(), r.y(), r.width(), r.height());
	if (d->cpen.style() != Qt::NoPen)
	    XDrawRectangle(d->dpy, d->hd, d->gc, r.x(), r.y(), r.width(), r.height());
	return;
    }
#endif // !QT_NO_XFT && !QT_NO_XRENDER

    if (d->cbrush.style() != Qt::NoBrush) {
        if (d->cpen.style() == Qt::NoPen) {
            XFillRectangle(d->dpy, d->hd, d->gc_brush, r.x(), r.y(), r.width(), r.height());
            return;
        }
        XFillRectangle(d->dpy, d->hd, d->gc_brush, r.x(), r.y(), r.width(), r.height());
    }
    if (d->cpen.style() != Qt::NoPen)
        XDrawRectangle(d->dpy, d->hd, d->gc, r.x(), r.y(), r.width(), r.height());
}

void QX11PaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    if (!isActive())
        return;

    if (d->cbrush.style() != Qt::NoBrush && d->cpen.style() != Qt::NoPen) {
	for (int i = 0; i < rectCount; ++i)
	    drawRect(rects[i]);
        return;
    }

    QVarLengthArray<XRectangle> xrects(rectCount);
    for (int i = 0; i < rectCount; ++i) {
	xrects[i].x = short(qIntCast(rects[i].x()));
	xrects[i].y = short(qIntCast(rects[i].y()));
	xrects[i].width = ushort(qIntCast(rects[i].width()));
	xrects[i].height = ushort(qIntCast(rects[i].height()));
    }

    if (d->cbrush.style() != Qt::NoBrush && d->cpen.style() == Qt::NoPen) {
	XFillRectangles(d->dpy, d->hd, d->gc_brush, xrects.data(), rectCount);
    } else if (d->cpen.style() != Qt::NoPen && d->cbrush.style() == Qt::NoBrush) {
        XDrawRectangles(d->dpy, d->hd, d->gc, xrects.data(), rectCount);
    }
}

void QX11PaintEngine::drawPoint(const QPointF &p)
{
    if (!isActive())
        return;
    if (d->cpen.style() != Qt::NoPen)
        XDrawPoint(d->dpy, d->hd, d->gc, qRound(p.x()), qRound(p.y()));
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
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
    if (X11->use_xrender) {
        XRenderPictureAttributes attrs;
        attrs.poly_edge = (hints & QPainter::Antialiasing) ? PolyEdgeSmooth : PolyEdgeSharp;
	::Picture dst = d->xft_hd ? XftDrawPicture(d->xft_hd) : 0;
        if (dst)
            XRenderChangePicture(d->dpy, dst, CPPolyEdge, &attrs);
    }
#else
    Q_UNUSED(hints);
#endif
}

void QX11PaintEngine::updatePen(const QPen &pen)
{
    d->cpen = pen;
    int ps = pen.style();
    QColormap cmap = QColormap::instance(d->scrn);

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
    vals.foreground = cmap.pixel(pen.color());
    vals.background = cmap.pixel(d->bg_col);
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
            x11SetClipRegion(d->dpy, d->gc, 0, d->xft_hd, sysClip);
        else
            x11ClearClipRegion(d->dpy, d->gc, 0, d->xft_hd);
    }
}

QPixmap qt_pixmapForBrush(int brushStyle, bool invert); //in qbrush.cpp

void QX11PaintEngine::updateBrush(const QBrush &brush, const QPointF &origin)
{
    d->cbrush = brush;
    d->bg_origin = origin;

    int s  = FillSolid;
    int  bs = d->cbrush.style();
    QColormap cmap = QColormap::instance(d->scrn);
    ulong mask = GCForeground | GCBackground | GCGraphicsExposures
                 | GCLineStyle | GCCapStyle | GCJoinStyle | GCFillStyle;
    XGCValues vals;
    vals.graphics_exposures = false;
    vals.foreground = cmap.pixel(d->cbrush.color());
    vals.background = cmap.pixel(d->bg_col);
    vals.cap_style = CapButt;
    vals.join_style = JoinMiter;
    vals.line_style = LineSolid;

    if (bs == Qt::TexturePattern || bs >= Qt::Dense1Pattern && bs <= Qt::DiagCrossPattern) {
        QPixmap pm;
        if (bs == Qt::TexturePattern)
            pm = d->cbrush.texture();
        else
            pm = qt_pixmapForBrush(bs, true);
        pm.x11SetScreen(d->scrn);
        if (pm.depth() == 1) {
            mask |= GCStipple;
            vals.stipple = pm.handle();
            s = d->bg_mode == Qt::TransparentMode ? FillStippled : FillOpaqueStippled;
        } else {
            mask |= GCTile;
            vals.tile = pm.data->x11ConvertToDefaultDepth();
            s = FillTiled;
        }

        mask |= GCTileStipXOrigin | GCTileStipYOrigin;
        vals.ts_x_origin = qRound(origin.x());
        vals.ts_y_origin = qRound(origin.y());
    }
    vals.fill_style = s;
    XChangeGC(d->dpy, d->gc_brush, mask, &vals);
    if (!hasClipping()) {
        QRegion sysClip = systemClip();
        if (!sysClip.isEmpty())
            x11SetClipRegion(d->dpy, d->gc, d->gc_brush, d->xft_hd, sysClip);
        else
            x11ClearClipRegion(d->dpy, d->gc_brush, 0, d->xft_hd);
    }
}

void QX11PaintEngine::drawEllipse(const QRectF &rect)
{
    if (renderHints() & QPainter::Antialiasing) {
        QPainterPath path;
        path.addEllipse(rect);
        QPolygonF poly = path.toFillPolygon();
        drawPolygon(poly.constData(), poly.size(), QPaintEngine::OddEvenMode);
        return;
    }
    QRect r = rect.toRect();
    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();
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

void QX11PaintEngine::drawPolygon(const QPointF *polygonPoints, int pointCount, PolygonDrawMode mode)
{
    int clippedCount = 0;
    qt_XPoint *clippedPoints = 0;

    if (mode != PolylineMode) {

#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
        bool smooth_edges = renderHints() & QPainter::Antialiasing;
        if (X11->use_xrender && d->cbrush.style() != Qt::NoBrush &&
            (smooth_edges || d->cbrush.color().alpha() != 255))
        {
            QPixmap gpix;
            ::Picture src = 0;
            ::Picture dst = d->xft_hd ? XftDrawPicture(d->xft_hd) : 0;
            int x_offset = 0;
            XftColor xfc;
            QColor qc = d->cbrush.color();

            const uint A = qc.alpha(),
                       R = qc.red(),
                       G = qc.green(),
                       B = qc.blue();

            xfc.pixel = QColormap::instance(d->scrn).pixel(qc);
            xfc.color.alpha = (A | A << 8);
            xfc.color.red   = (R | R << 8) * xfc.color.alpha / 0x10000;
            xfc.color.green = (B | G << 8) * xfc.color.alpha / 0x10000;
            xfc.color.blue  = (B | B << 8) * xfc.color.alpha / 0x10000;
            src = d->xft_hd ? XftDrawSrcPicture(d->xft_hd, &xfc) : 0;

            if (src && dst) {
                int cCount;
                qt_float_point *cPoints;
                d->floatClipper.clipPolygon((qt_float_point *)polygonPoints, pointCount, &cPoints, &cCount);
                if (cCount > 0) {
                    QVector<XTrapezoid> traps;
                    traps.reserve(128);
                    qt_tesselate_polygon(&traps, (QPointF *)cPoints, cCount, mode == WindingMode);

                    XRenderPictureAttributes attrs;
                    attrs.poly_edge = smooth_edges ? PolyEdgeSmooth : PolyEdgeSharp;
                    XRenderChangePicture(d->dpy, dst, CPPolyEdge, &attrs);
                    XRenderCompositeTrapezoids(d->dpy, PictOpOver, src, dst,
                                               XRenderFindStandardFormat(d->dpy, PictStandardA8),
                                               x_offset, 0, traps.constData(), traps.size());
                }
            }
        } else
#endif
            if (d->cbrush.style() != Qt::NoBrush) {
                if (mode == WindingMode)
                    XSetFillRule(d->dpy, d->gc_brush, WindingRule);
                d->polygonClipper.clipPolygon((qt_float_point *) polygonPoints, pointCount,
                                              &clippedPoints, &clippedCount);
                if (clippedCount > 0)
                    XFillPolygon(d->dpy, d->hd, d->gc_brush,
                                 (XPoint *) clippedPoints, clippedCount,
                                 mode == ConvexMode ? Convex : Complex, CoordModeOrigin);

                if (mode == WindingMode)
                    XSetFillRule(d->dpy, d->gc_brush, EvenOddRule);
            }
    }

    if (d->cpen.style() != Qt::NoPen) {
        if (!clippedPoints) // already clipped?
            d->polygonClipper.clipPolygon((qt_float_point *) polygonPoints, pointCount,
                                          &clippedPoints, &clippedCount, mode != PolylineMode);
        if (clippedCount > 0)
            XDrawLines(d->dpy, d->hd, d->gc, (XPoint *) clippedPoints, clippedCount, CoordModeOrigin);
    }
}

void QX11PaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    QPolygonF p;
    p.reserve(pointCount);
    for (int i=0; i<pointCount; ++i)
        p << points[i];
    drawPolygon(p.data(), pointCount, mode);
}

//
// Internal functions for simple GC caching for blt'ing masked pixmaps.
// This cache is used when the pixmap optimization is set to Normal
// and the pixmap size doesn't exceed 128x128.
//

static bool      init_mask_gc = false;
static const int max_mask_gcs = 11;                // suitable for hashing

struct mask_gc {
    GC        gc;
    int mask_no;
};

static mask_gc gc_vec[max_mask_gcs];


static void cleanup_mask_gc()
{
    Display *dpy = QX11Info::display();
    init_mask_gc = false;
    for (int i=0; i<max_mask_gcs; i++) {
        if (gc_vec[i].gc)
            XFreeGC(dpy, gc_vec[i].gc);
    }
}

static GC cache_mask_gc(Display *dpy, Drawable hd, int mask_no, Pixmap mask)
{
    if (!init_mask_gc) {                        // first time initialization
        init_mask_gc = true;
        qAddPostRoutine(cleanup_mask_gc);
        for (int i=0; i<max_mask_gcs; i++)
            gc_vec[i].gc = 0;
    }
    mask_gc *p = &gc_vec[mask_no % max_mask_gcs];
    if (!p->gc || p->mask_no != mask_no) {        // not a perfect match
        if (!p->gc) {                                // no GC
            p->gc = XCreateGC(dpy, hd, 0, 0);
            XSetGraphicsExposures(dpy, p->gc, False);
        }
        XSetClipMask(dpy, p->gc, mask);
        p->mask_no = mask_no;
    }
    return p->gc;
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
                if (pm->mask() && !ignoreMask) {
                    QBitmap mask(sw, sh);
                    qt_bit_blt(&mask, 0, 0, pm->mask(), sx, sy, sw, sh, true);
                    tmp->setMask(mask);
                }
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
    if (src_pm && !mono_src) {
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
        if (dst_pict && src_pict) {
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
            XRenderComposite(dpy, (src_pm->data->alpha && !ignoreMask ? PictOpOver : PictOpSrc),
                             src_pict, XNone, dst_pict,
                             sx, sy, sx, sy, dx, dy, sw, sh);
            // restore attributes
            pattr.subwindow_mode = ClipByChildren;
            pattr.graphics_exposures = false;
            if (picmask)
                XRenderChangePicture(dpy, dst_pict, picmask, &pattr);
            return;
        }
    }
#endif

    GC gc;

    if (mask && !mono_src) {                        // fast masked blt
        bool temp_gc = false;
        if (mask->data->maskgc) {
            gc = (GC)mask->data->maskgc;        // we have a premade mask GC
        } else {
            if (false && src_pm->optimization() == QPixmap::NormalOptim) { // cache disabled
                // Compete for the global cache
                gc = cache_mask_gc(dpy, qt_x11Handle(dst),
				   mask->data->ser_no,
				   mask->handle());
            } else {
                // Create a new mask GC. If BestOptim, we store the mask GC
                // with the mask (not at the pixmap). This way, many pixmaps
                // which have a common mask will be optimized at no extra cost.
                gc = XCreateGC(dpy, qt_x11Handle(dst), 0, 0);
                XSetGraphicsExposures(dpy, gc, False);
		XSetClipMask(dpy, gc, mask->handle());
                if (src_pm->optimization() == QPixmap::BestOptim) {
                    mask->data->maskgc = gc;
                } else {
                    temp_gc = true;
                }
            }
        }
        XSetClipOrigin(dpy, gc, dx-sx, dy-sy);
        if (include_inferiors) {
            XSetSubwindowMode(dpy, gc, IncludeInferiors);
            XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
            XSetSubwindowMode(dpy, gc, ClipByChildren);
        } else {
            XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
        }

        if (temp_gc)                                // delete temporary GC
            XFreeGC(dpy, gc);
        return;
    }

    gc = XCreateGC(dpy, qt_x11Handle(dst), 0, 0);
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

    QBitmap *mask = 0;
    if(mode != Qt::CopyPixmapNoMask && !pixmap.hasAlphaChannel())
        mask = const_cast<QBitmap *>(pixmap.mask());
    bool mono = pixmap.depth() == 1;

    if (mono && mode == Qt::CopyPixmap) {
	qt_bit_blt(d->pdev, x, y, &pixmap, sx, sy, sw, sh, true);
        return;
    }

    if (mask && !hasClipping() && systemClip().isEmpty()) {
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
                QRegion sysClip = systemClip();
                if (!sysClip.isEmpty()) {
                    x11SetClipRegion(d->dpy, d->gc, 0, d->xft_hd, sysClip);
                } else {
                    x11ClearClipRegion(d->dpy, d->gc, 0, d->xft_hd);
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


    if (mask) { // pixmap has clip mask
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

        QBitmap *comb = new QBitmap(sw, sh);
        comb->detach();
        GC cgc = qt_xget_temp_gc(pixmap.x11Info().screen(), true);   // get temporary mono GC
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
        XSetClipMask(d->dpy, d->gc, pixmap.handle());
        XSetClipOrigin(d->dpy, d->gc, x-sx, y-sy);
        XSetBackground(d->dpy, d->gc, QColormap::instance(d->scrn).pixel(d->bg_brush.color()));
        XSetForeground(d->dpy, d->gc, QColormap::instance(d->scrn).pixel(d->cpen.color()));
        XFillRectangle(d->dpy, d->hd, d->gc, x, y, sw, sh);
        XSetClipMask(d->dpy, d->gc, XNone);
        XSetClipOrigin(d->dpy, d->gc, 0, 0);
    } else {
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
        ::Picture pict = d->xft_hd ? XftDrawPicture(d->xft_hd) : 0;
        if (mode == Qt::ComposePixmap && pict && pixmap.xftPictureHandle()) {
            XRenderComposite(d->dpy, (pixmap.data->alpha ? PictOpOver : PictOpSrc),
                             pixmap.xftPictureHandle(),
                             XNone, pict, sx, sy, sx, sy, x, y, sw, sh);
        } else
#endif // !QT_NO_XFT && !QT_NO_XRENDER
            {
                XCopyArea(d->dpy, pixmap.handle(), d->hd, d->gc, sx, sy, sw, sh, x, y);
                if (mode == Qt::CopyPixmap && d->pdev->devType() == QInternal::Pixmap) {
                    QPixmap *px = static_cast<QPixmap *>(d->pdev);
                    px->data->alpha = pixmap.data->alpha;
                }
            }
    }

    if (mask) { // restore clipping
        XSetClipOrigin(d->dpy, d->gc, 0, 0);
        int num;
        XRectangle *rects = (XRectangle *)qt_getClipRects(d->crgn, num);
        if (num == 0)
            XSetClipMask(d->dpy, d->gc, XNone);
        else
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
    updatePen(d->cpen);
    updateBrush(d->cbrush, d->bg_origin);
}

void QX11PaintEngine::updateMatrix(const QMatrix &mtx)
{
    if (mtx.m12() != 0 || mtx.m21() != 0)
        d->txop = QPainterPrivate::TxRotShear;
    else if (mtx.m11() != 1 || mtx.m22() != 1)
        d->txop = QPainterPrivate::TxScale;
    else if (mtx.dx() != 0 || mtx.dy() != 0)
        d->txop = QPainterPrivate::TxTranslate;
    else
        d->txop = QPainterPrivate::TxNone;
}

void QX11PaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    QRegion sysClip = systemClip();
    if (op == Qt::NoClip) {
        clearf(ClipOn);
        d->crgn = QRegion();
        if (!sysClip.isEmpty()) {
            x11SetClipRegion(d->dpy, d->gc, d->gc_brush, d->xft_hd, sysClip);
        } else {
            x11ClearClipRegion(d->dpy, d->gc, d->gc_brush, d->xft_hd);
        }
        return;
    }

    switch (op) {
    case Qt::ReplaceClip:
        if (!sysClip.isEmpty())
            d->crgn = clipRegion.intersect(sysClip);
        else
            d->crgn = clipRegion;
        break;
    case Qt::IntersectClip:
        if (testf(ClipOn))
            d->crgn &= clipRegion;
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
    x11SetClipRegion(d->dpy, d->gc, d->gc_brush, d->xft_hd, d->crgn);
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

    if (pixmap.mask() == 0 && pixmap.depth() > 1 && d->txop <= QPainterPrivate::TxTranslate) {
#if !defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
        ::Picture pict = d->xft_hd ? XftDrawPicture(d->xft_hd) : 0;
        if (pict && pixmap.xftPictureHandle()) {
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
                                     pict, xOff, yOff, xOff, yOff, xPos, yPos, drawW, drawH);
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
    qreal scale = si.fontEngine->scale();
    if ( d->txop > QPainterPrivate::TxTranslate || scale < 0.9999 || scale > 1.0001  ) {
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

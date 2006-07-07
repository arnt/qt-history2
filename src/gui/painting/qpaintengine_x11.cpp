/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"

#include "private/qpixmap_p.h"

#include "qapplication.h"
#include "qdebug.h"
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

#include "qmath_p.h"

#include <private/qpaintengine_p.h>
#include "qpaintengine_x11_p.h"

#include <private/qt_x11_p.h>
#include <private/qnumeric_p.h>
#include <limits.h>

extern Drawable qt_x11Handle(const QPaintDevice *pd);
extern const QX11Info *qt_x11Info(const QPaintDevice *pd);
extern QPixmap qt_pixmapForBrush(int brushStyle, bool invert); //in qbrush.cpp

#undef X11 // defined in qt_x11_p.h
/*!
    Returns the X11 specific pen GC for the painter \a p. Note that
    QPainter::begin() must be called before this function returns a
    valid GC.
*/
Q_GUI_EXPORT GC qt_x11_get_pen_gc(QPainter *p)
{
    if (p && p->paintEngine()
        && p->paintEngine()->isActive()
        && p->paintEngine()->type() == QPaintEngine::X11) {
        return static_cast<QX11PaintEngine *>(p->paintEngine())->d_func()->gc;
    }
    return 0;
}

/*!
    Returns the X11 specific brush GC for the painter \a p. Note that
    QPainter::begin() must be called before this function returns a
    valid GC.
*/
Q_GUI_EXPORT GC qt_x11_get_brush_gc(QPainter *p)
{
    if (p && p->paintEngine()
        && p->paintEngine()->isActive()
        && p->paintEngine()->type() == QPaintEngine::X11) {
        return static_cast<QX11PaintEngine *>(p->paintEngine())->d_func()->gc_brush;
    }
    return 0;
}
#define X11 qt_x11Data

#ifndef QT_NO_XRENDER
static const int compositionModeToRenderOp[QPainter::CompositionMode_Xor + 1] = {
    PictOpOver, //CompositionMode_SourceOver,
    PictOpOverReverse, //CompositionMode_DestinationOver,
    PictOpClear, //CompositionMode_Clear,
    PictOpSrc, //CompositionMode_Source,
    PictOpDst, //CompositionMode_Destination,
    PictOpIn, //CompositionMode_SourceIn,
    PictOpInReverse, //CompositionMode_DestinationIn,
    PictOpOut, //CompositionMode_SourceOut,
    PictOpOutReverse, //CompositionMode_DestinationOut,
    PictOpAtop, //CompositionMode_SourceAtop,
    PictOpAtopReverse, //CompositionMode_DestinationAtop,
    PictOpXor //CompositionMode_Xor
};

static inline int qpainterOpToXrender(QPainter::CompositionMode mode)
{
    Q_ASSERT(mode <= QPainter::CompositionMode_Xor);
    return compositionModeToRenderOp[mode];
}
#endif

// hack, so we don't have to make QRegion::clipRectangles() public or include
// X11 headers in qregion.h
void *qt_getClipRects(const QRegion &r, int &num)
{
    return r.clipRectangles(num);
}

static inline void x11SetClipRegion(Display *dpy, GC gc, GC gc2,
#ifndef QT_NO_XRENDER
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

#ifndef QT_NO_XRENDER
    if (picture)
        XRenderSetPictureClipRectangles(dpy, picture, 0, 0, rects, num);
#else
    Q_UNUSED(picture);
#endif // QT_NO_XRENDER
}


static inline void x11ClearClipRegion(Display *dpy, GC gc, GC gc2,
#ifndef QT_NO_XRENDER
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

#ifndef QT_NO_XRENDER
    if (picture) {
        XRenderPictureAttributes attrs;
        attrs.clip_mask = XNone;
        XRenderChangePicture (dpy, picture, CPClipMask, &attrs);
    }
#else
    Q_UNUSED(picture);
#endif // QT_NO_XRENDER
}


#define DITHER_SIZE 16
static const uchar base_dither_matrix[DITHER_SIZE][DITHER_SIZE] = {
  {   0,192, 48,240, 12,204, 60,252,  3,195, 51,243, 15,207, 63,255 },
  { 128, 64,176,112,140, 76,188,124,131, 67,179,115,143, 79,191,127 },
  {  32,224, 16,208, 44,236, 28,220, 35,227, 19,211, 47,239, 31,223 },
  { 160, 96,144, 80,172,108,156, 92,163, 99,147, 83,175,111,159, 95 },
  {   8,200, 56,248,  4,196, 52,244, 11,203, 59,251,  7,199, 55,247 },
  { 136, 72,184,120,132, 68,180,116,139, 75,187,123,135, 71,183,119 },
  {  40,232, 24,216, 36,228, 20,212, 43,235, 27,219, 39,231, 23,215 },
  { 168,104,152, 88,164,100,148, 84,171,107,155, 91,167,103,151, 87 },
  {   2,194, 50,242, 14,206, 62,254,  1,193, 49,241, 13,205, 61,253 },
  { 130, 66,178,114,142, 78,190,126,129, 65,177,113,141, 77,189,125 },
  {  34,226, 18,210, 46,238, 30,222, 33,225, 17,209, 45,237, 29,221 },
  { 162, 98,146, 82,174,110,158, 94,161, 97,145, 81,173,109,157, 93 },
  {  10,202, 58,250,  6,198, 54,246,  9,201, 57,249,  5,197, 53,245 },
  { 138, 74,186,122,134, 70,182,118,137, 73,185,121,133, 69,181,117 },
  {  42,234, 26,218, 38,230, 22,214, 41,233, 25,217, 37,229, 21,213 },
  { 170,106,154, 90,166,102,150, 86,169,105,153, 89,165,101,149, 85 }
};

static QPixmap qt_patternForAlpha(uchar alpha)
{
    QPixmap pm;
    QString key = QLatin1String("$qt-alpha-brush$") + QString::number(alpha);
    if (!QPixmapCache::find(key, pm)) {
        // #### why not use a mono image here????
        QImage pattern(DITHER_SIZE, DITHER_SIZE, QImage::Format_ARGB32);
        pattern.fill(0xffffffff);
        for (int y = 0; y < DITHER_SIZE; ++y) {
            for (int x = 0; x < DITHER_SIZE; ++x) {
                if (base_dither_matrix[x][y] <= alpha)
                    pattern.setPixel(x, y, 0x00000000);
            }
        }
        pm = QBitmap::fromImage(pattern);
        QPixmapCache::insert(key, pm);
    }
    return pm;
}

#if !defined(QT_NO_XRENDER)

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
    XPointFixed p1, p2;
    qreal m;
    qreal b;
    signed char winding;
};

Q_DECLARE_TYPEINFO(QEdge, Q_PRIMITIVE_TYPE);

static inline bool compareEdges(const QEdge *e1, const QEdge *e2)
{
    return e1->p1.y < e2->p1.y;
}

static inline bool isEqual(const XPointFixed &p1, const XPointFixed &p2)
{
    return ((p1.x == p2.x) && (p1.y == p2.y));
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
        qreal x1 = !qIsFinite(i1.edge->b) ? XFixedToDouble(i1.edge->p1.x) :
                   (currentY+1.f - i1.edge->b)*i1.edge->m;
        qreal x2 = !qIsFinite(i2.edge->b) ? XFixedToDouble(i2.edge->p1.x) :
                   (currentY+1.f - i2.edge->b)*i2.edge->m;
        return x1 < x2;
    }
}

#define qrealToXFixed FloatToXFixed

static XTrapezoid QT_FASTCALL toXTrapezoid(XFixed y1, XFixed y2, const QEdge &left, const QEdge &right)
{
    XTrapezoid trap;
    trap.top = y1;
    trap.bottom = y2;
    trap.left.p1.y = left.p1.y;
    trap.left.p2.y = left.p2.y;
    trap.right.p1.y = right.p1.y;
    trap.right.p2.y = right.p2.y;
    trap.left.p1.x = left.p1.x;
    trap.left.p2.x = left.p2.x;
    trap.right.p1.x = right.p1.x;
    trap.right.p2.x = right.p2.x;
    return trap;
}

#ifdef QT_DEBUG_TESSELATOR
static QPointF xf_to_qt(XPointFixed pt)
{
    return QPointF(XFixedToDouble(pt.x), XFixedToDouble(pt.y));
}

static void dump_edges(const QList<const QEdge *> &et)
{
    for (int x = 0; x < et.size(); ++x) {
        qDebug() << "edge#" << x << xf_to_qt(et.at(x)->p1) << xf_to_qt(et.at(x)->p2) << "b: " << et.at(x)->b << "m:" << et.at(x)->m << et.at(x);
    }
}

static void dump_trap(const XTrapezoid &t)
{
    qDebug() << "trap# t=" << XFixedToDouble(t.top) << "b=" << XFixedToDouble(t.bottom)  << "h="
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
                                 bool winding, QRect *br)
{
    QVector<QEdge> edges;
    edges.reserve(128);
    qreal ymin(INT_MAX/256);
    qreal ymax(INT_MIN/256);
    qreal xmin(INT_MAX/256);
    qreal xmax(INT_MIN/256);

    Q_ASSERT(pg[0] == pg[pgSize-1]);
    // generate edge table
    for (int x = 0; x < pgSize-1; ++x) {
	QEdge edge;
	edge.winding = pg[x].y() > pg[x+1].y() ? 1 : -1;
        QPointF p1, p2;
	if (edge.winding > 0) {
	    p1 = pg[x+1];
	    p2 = pg[x];
	} else {
	    p1 = pg[x];
	    p2 = pg[x+1];
	}
        edge.p1.x = XDoubleToFixed(p1.x());
        edge.p1.y = XDoubleToFixed(p1.y());
        edge.p2.x = XDoubleToFixed(p2.x());
        edge.p2.y = XDoubleToFixed(p2.y());

	edge.m = (p1.y() - p2.y()) / (p1.x() - p2.x()); // line derivative
	edge.b = p1.y() - edge.m * p1.x(); // intersection with y axis
	edge.m = edge.m != 0.0 ? 1.0 / edge.m : 0.0; // inverted derivative
	edges.append(edge);
        xmin = qMin(xmin, XFixedToDouble(edge.p1.x));
        xmax = qMax(xmax, XFixedToDouble(edge.p2.x));
        ymin = qMin(ymin, XFixedToDouble(edge.p1.y));
        ymax = qMax(ymax, XFixedToDouble(edge.p2.y));
    }
    br->setX(qRound(xmin));
    br->setY(qRound(ymin));
    br->setWidth(qRound(xmax - xmin));
    br->setHeight(qRound(ymax - ymin));

    QList<const QEdge *> et; 	    // edge list
    for (int i = 0; i < edges.size(); ++i)
        et.append(&edges.at(i));

    // sort edge table by min y value
    qSort(et.begin(), et.end(), compareEdges);

    // eliminate shared edges
    for (int i = 0; i < et.size(); ++i) {
	for (int k = i+1; k < et.size(); ++k) {
            const QEdge *edgeI = et.at(i);
            const QEdge *edgeK = et.at(k);
            if (edgeK->p1.y > edgeI->p1.y)
                break;
   	    if (edgeI->winding != edgeK->winding &&
                isEqual(edgeI->p1, edgeK->p1) && isEqual(edgeI->p2, edgeK->p2)
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
            const QEdge *edge = et.at(i);
            if (edge->p1.y > XDoubleToFixed(y))
                break;
            aet.append(edge);
            et.removeAt(i);
            --i;
	}

	// remove processed edges from active edge table
	for (int i = 0; i < aet.size(); ++i) {
	    if (aet.at(i)->p2.y <= XDoubleToFixed(y)) {
		aet.removeAt(i);
 		--i;
	    }
	}
        if (aet.size()%2 != 0) {
#ifndef QT_NO_DEBUG
            qWarning("QX11PaintEngine: Internal error: aet out of sync");
#endif
            return;
        }

	// done?
	if (!aet.size()) {
            if (!et.size()) {
                break;
	    } else {
 		y = XFixedToDouble(et.at(0)->p1.y);
                continue;
	    }
        }

        // calculate the next y where we have to start a new set of trapezoids
	qreal next_y(INT_MAX/256);
 	for (int i = 0; i < aet.size(); ++i) {
            const QEdge *edge = aet.at(i);
 	    if (XFixedToDouble(edge->p2.y) < next_y)
 		next_y = XFixedToDouble(edge->p2.y);
        }

	if (et.size() && next_y > XFixedToDouble(et.at(0)->p1.y))
	    next_y = XFixedToDouble(et.at(0)->p1.y);

        int aetSize = aet.size();
	for (int i = 0; i < aetSize; ++i) {
	    for (int k = i+1; k < aetSize; ++k) {
                const QEdge *edgeI = aet.at(i);
                const QEdge *edgeK = aet.at(k);
		qreal m1 = edgeI->m;
		qreal b1 = edgeI->b;
		qreal m2 = edgeK->m;
		qreal b2 = edgeK->b;

		if (qAbs(m1 - m2) < 0.001)
                    continue;

                // ### intersect is not calculated correctly when optimized with -O2 (gcc)
                volatile qreal intersect;
                if (!qIsFinite(b1))
                    intersect = (1.f / m2) * XFixedToDouble(edgeI->p1.x) + b2;
                else if (!qIsFinite(b2))
                    intersect = (1.f / m1) * XFixedToDouble(edgeK->p1.x) + b1;
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
            const QEdge *edge = aet.at(i);
 	    isects[i].x = (edge->p1.x != edge->p2.x) ?
			  ((y - edge->b)*edge->m) : XFixedToDouble(edge->p1.x);
	    isects[i].edge = edge;
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

    for (int i = 0; i < traps->size(); ++i)
        dump_trap(traps->at(i));
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

#endif // !defined(QT_NO_XRENDER)


#ifndef QT_NO_XRENDER
static Picture getPatternFill(int screen, const QBrush &b, const QBrush &bg, bool opaque_bg)
{
    if (!X11->use_xrender)
        return XNone;

    XRenderColor color = X11->preMultiply(b.color());
    XRenderColor bg_color;

    if (opaque_bg)
        bg_color = X11->preMultiply(bg.color());
    else
        bg_color = X11->preMultiply(QColor(0, 0, 0, 0));

    for (int i = 0; i < X11->pattern_fill_count; ++i) {
        if (X11->pattern_fills[i].screen == screen
            && X11->pattern_fills[i].opaque == opaque_bg
            && X11->pattern_fills[i].style == b.style()
            && X11->pattern_fills[i].color.alpha == color.alpha
            && X11->pattern_fills[i].color.red == color.red
            && X11->pattern_fills[i].color.green == color.green
            && X11->pattern_fills[i].color.blue == color.blue
            && X11->pattern_fills[i].bg_color.alpha == bg_color.alpha
            && X11->pattern_fills[i].bg_color.red == bg_color.red
            && X11->pattern_fills[i].bg_color.green == bg_color.green
            && X11->pattern_fills[i].bg_color.blue == bg_color.blue)
            return X11->pattern_fills[i].picture;
    }
    // none found, replace one
    int i = rand() % 16;

    if (X11->pattern_fills[i].screen != screen && X11->pattern_fills[i].picture) {
	XRenderFreePicture (X11->display, X11->pattern_fills[i].picture);
	X11->pattern_fills[i].picture = 0;
    }

    if (!X11->pattern_fills[i].picture) {
        Pixmap pixmap = XCreatePixmap (X11->display, RootWindow (X11->display, screen), 8, 8, 32);
        XRenderPictureAttributes attrs;
        attrs.repeat = True;
        X11->pattern_fills[i].picture = XRenderCreatePicture (X11->display, pixmap,
                                                              XRenderFindStandardFormat(X11->display, PictStandardARGB32),
                                                              CPRepeat, &attrs);
        XFreePixmap (X11->display, pixmap);
    }

    X11->pattern_fills[i].screen = screen;
    X11->pattern_fills[i].color = color;
    X11->pattern_fills[i].bg_color = bg_color;
    X11->pattern_fills[i].opaque = opaque_bg;
    X11->pattern_fills[i].style = b.style();

    XRenderFillRectangle(X11->display, PictOpSrc, X11->pattern_fills[i].picture, &bg_color, 0, 0, 8, 8);

    QPixmap pattern(qt_pixmapForBrush(b.style(), true));
    XRenderPictureAttributes attrs;
    attrs.repeat = true;
    XRenderChangePicture(X11->display, pattern.x11PictureHandle(), CPRepeat, &attrs);

    Picture fill_fg = X11->getSolidFill(screen, b.color());
    XRenderComposite(X11->display, PictOpOver, fill_fg, pattern.x11PictureHandle(),
                     X11->pattern_fills[i].picture,
                     0, 0, 0, 0, 0, 0, 8, 8);

    return X11->pattern_fills[i].picture;
}

static void qt_render_bitmap(Display *dpy, int scrn, Picture src, Picture dst,
                      int sx, int sy, int x, int y, int sw, int sh,
                      const QPen &pen, const QBrush &brush, bool opaque_bg)
{
    if (opaque_bg) {
        Picture fill_bg = X11->getSolidFill(scrn, brush.color());
        XRenderComposite(dpy, PictOpOver,
                         fill_bg, 0, dst, sx, sy, sx, sy, x, y, sw, sh);
    }
    Picture fill_fg = X11->getSolidFill(scrn, pen.color());
    XRenderComposite(dpy, PictOpOver,
                     fill_fg, src, dst, sx, sy, sx, sy, x, y, sw, sh);
}
#endif

void QX11PaintEnginePrivate::init()
{
    dpy = 0;
    scrn = 0;
    hd = 0;
    picture = 0;
    xinfo = 0;
#ifndef QT_NO_XRENDER
    current_brush = 0;
    composition_mode = PictOpOver;
#endif
}

void QX11PaintEnginePrivate::setupAdaptedOrigin(const QPoint &p)
{
    if (adapted_pen_origin)
        XSetTSOrigin(dpy, gc, p.x(), p.y());
    if (adapted_brush_origin)
        XSetTSOrigin(dpy, gc_brush, p.x(), p.y());
}

void QX11PaintEnginePrivate::resetAdaptedOrigin()
{
    if (adapted_pen_origin)
        XSetTSOrigin(dpy, gc, 0, 0);
    if (adapted_brush_origin)
        XSetTSOrigin(dpy, gc_brush, 0, 0);
}

void QX11PaintEnginePrivate::clipPolygon_dev(const QPolygonF &poly, QPolygonF *clipped_poly)
{
    int clipped_count = 0;
    qt_float_point *clipped_points = 0;
    polygonClipper.clipPolygon((qt_float_point *) poly.data(), poly.size(),
                               &clipped_points, &clipped_count);
    clipped_poly->resize(clipped_count);
    for (int i=0; i<clipped_count; ++i)
        (*clipped_poly)[i] = *((QPointF *)(&clipped_points[i]));
}

static QPaintEngine::PaintEngineFeatures qt_decide_features()
{
    QPaintEngine::PaintEngineFeatures features =
        QPaintEngine::PrimitiveTransform
        | QPaintEngine::PatternBrush
        | QPaintEngine::AlphaBlend
        | QPaintEngine::PainterPaths;

    if (X11->use_xrender) {
        features |= QPaintEngine::Antialiasing;
        features |= QPaintEngine::PorterDuff;
#if 0
        if (X11->xrender_version > 10) {
            features |= QPaintEngine::LinearGradientFill;
            // ###
        }
#endif
    }

    return features;
}

/*
 * QX11PaintEngine members
 */

QX11PaintEngine::QX11PaintEngine()
    : QPaintEngine(*(new QX11PaintEnginePrivate), qt_decide_features())
{
    d_func()->init();
}

QX11PaintEngine::QX11PaintEngine(QX11PaintEnginePrivate &dptr)
    : QPaintEngine(dptr, qt_decide_features())
{
    d_func()->init();
}

QX11PaintEngine::~QX11PaintEngine()
{
}

bool QX11PaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QX11PaintEngine);
    d->xinfo = qt_x11Info(pdev);
#ifndef QT_NO_XRENDER
    if (pdev->devType() == QInternal::Widget) {
        d->picture = (::Picture)static_cast<const QWidget *>(pdev)->x11PictureHandle();
    } else if (pdev->devType() == QInternal::Pixmap) {
        const QPixmap *pm = static_cast<const QPixmap *>(pdev);
        if (X11->use_xrender && pm->data->d != 32 && pm->data->x11_mask)
            pm->data->convertToARGB32();
        d->picture = (::Picture)static_cast<const QPixmap *>(pdev)->x11PictureHandle();
    }
#else
    d->picture = 0;
#endif
    d->hd = qt_x11Handle(pdev);

    Q_ASSERT(d->xinfo != 0);
    d->dpy = d->xinfo->display(); // get display variable
    d->scrn = d->xinfo->screen(); // get screen variable

    d->bg_col = Qt::white;
    d->bg_mode = Qt::TransparentMode;
    d->crgn = QRegion();
    d->gc = XCreateGC(d->dpy, d->hd, 0, 0);
    d->gc_brush = XCreateGC(d->dpy, d->hd, 0, 0);
    d->has_alpha_brush = false;
    d->has_alpha_pen = false;
    d->has_clipping = false;
    d->has_complex_xform = false;
    d->has_custom_pen = false;
    d->matrix = QMatrix();
    d->pdev_depth = d->pdev->depth();
    d->render_hints = 0;
    d->txop = QPainterPrivate::TxNone;
    d->use_path_fallback = false;
#if !defined(QT_NO_XRENDER)
    d->composition_mode = PictOpOver;
#endif
    d->xlibMaxLinePoints = 32762; // a safe number used to avoid, call to XMaxRequestSize(d->dpy) - 3;
    d->opacity = 1;

    // Set up the polygon clipper. Note: This will only work in
    // polyline mode as long as we have a buffer zone, since a
    // polyline may be clipped into several non-connected polylines.
    const int BUFFERZONE = 1000;
    QRect devClipRect(-BUFFERZONE, -BUFFERZONE,
                      pdev->width() + 2*BUFFERZONE, pdev->height() + 2*BUFFERZONE);
    d->polygonClipper.setBoundingRect(devClipRect);

    QPixmap::x11SetDefaultScreen(d->xinfo->screen());

    QWidget *w = d->pdev->devType() == QInternal::Widget ? static_cast<QWidget *>(d->pdev) : 0;
    if (w && w->testAttribute(Qt::WA_PaintUnclipped)) {  // paint direct on device
 	updatePen(QPen(Qt::black));
 	updateBrush(QBrush(Qt::white), QPoint());
        XSetSubwindowMode(d->dpy, d->gc, IncludeInferiors);
        XSetSubwindowMode(d->dpy, d->gc_brush, IncludeInferiors);
#ifndef QT_NO_XRENDER
        XRenderPictureAttributes attrs;
        attrs.subwindow_mode = IncludeInferiors;
        XRenderChangePicture(d->dpy, d->picture, CPSubwindowMode, &attrs);
#endif
    }

    setDirty(QPaintEngine::DirtyClipRegion);
    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    setDirty(QPaintEngine::DirtyBackground);

    return true;
}

bool QX11PaintEngine::end()
{
    Q_D(QX11PaintEngine);

#if !defined(QT_NO_XRENDER)
    if (d->picture) {
        // reset clipping/subwindow mode on our render picture
	XRenderPictureAttributes attrs;
	attrs.subwindow_mode = ClipByChildren;
        attrs.clip_mask = XNone;
	XRenderChangePicture(d->dpy, d->picture, CPClipMask|CPSubwindowMode, &attrs);
    }
#endif

    if (d->gc_brush && d->pdev->painters < 2) {
        XFreeGC(d->dpy, d->gc_brush);
        d->gc_brush = 0;
    }

    if (d->gc && d->pdev->painters < 2) {
        XFreeGC(d->dpy, d->gc);
        d->gc = 0;
    }

    return true;
}

static bool clipLine(QLineF *line, const QRect &rect)
{
    qreal x1 = line->x1();
    qreal x2 = line->x2();
    qreal y1 = line->y1();
    qreal y2 = line->y2();

    qreal left = rect.x();
    qreal right = rect.x() + rect.width() - 1;
    qreal top = rect.y();
    qreal bottom = rect.y() + rect.height() - 1;

    enum { Left, Right, Top, Bottom };
    // clip the lines, after cohen-sutherland, see e.g. http://www.nondot.org/~sabre/graphpro/line6.html
    int p1 = ((x1 < left) << Left)
             | ((x1 > right) << Right)
             | ((y1 < top) << Top)
             | ((y1 > bottom) << Bottom);
    int p2 = ((x2 < left) << Left)
             | ((x2 > right) << Right)
             | ((y2 < top) << Top)
             | ((y2 > bottom) << Bottom);

    if (p1 & p2)
        // completely outside
        return false;

    if (p1 | p2) {
        qreal dx = x2 - x1;
        qreal dy = y2 - y1;

        // clip x coordinates
        if (x1 < left) {
            y1 += dy/dx * (left - x1);
            x1 = left;
        } else if (x1 > right) {
            y1 -= dy/dx * (x1 - right);
            x1 = right;
        }
        if (x2 < left) {
            y2 += dy/dx * (left - x2);
            x2 = left;
        } else if (x2 > right) {
            y2 -= dy/dx * (x2 - right);
            x2 = right;
        }
        p1 = ((y1 < top) << Top)
             | ((y1 > bottom) << Bottom);
        p2 = ((y2 < top) << Top)
             | ((y2 > bottom) << Bottom);
        if (p1 & p2)
            return false;
        // clip y coordinates
        if (y1 < top) {
            x1 += dx/dy * (top - y1);
            y1 = top;
        } else if (y1 > bottom) {
            x1 -= dx/dy * (y1 - bottom);
            y1 = bottom;
        }
        if (y2 < top) {
            x2 += dx/dy * (top - y2);
            y2 = top;
        } else if (y2 > bottom) {
            x2 -= dx/dy * (y2 - bottom);
            y2 = bottom;
        }
        *line = QLineF(QPointF(x1, y1), QPointF(x2, y2));
    }
    return true;
}

void QX11PaintEngine::drawLines(const QLine *lines, int lineCount)
{
    Q_ASSERT(lines);
    Q_ASSERT(lineCount);
    Q_D(QX11PaintEngine);
    if (d->has_alpha_brush
        || d->has_alpha_pen
        || d->has_custom_pen
        || (d->cpen.widthF() > 0 && d->has_complex_xform)
        || (d->render_hints & QPainter::Antialiasing)) {
        for (int i = 0; i < lineCount; ++i) {
            QPainterPath path(lines[i].p1());
            path.lineTo(lines[i].p2());
            drawPath(path);
        }
        return;
    }

    if (d->has_pen) {
        for (int i = 0; i < lineCount; ++i) {
            QLineF linef;
            if (d->txop == QPainterPrivate::TxNone) {
                linef = lines[i];
            } else {
                linef = QLineF(d->matrix.map(lines[i].p1()), d->matrix.map(lines[i].p2()));
            }
            if (clipLine(&linef, d->polygonClipper.boundingRect())) {
                XDrawLine(d->dpy, d->hd, d->gc,
                          qFloor(linef.x1()), qFloor(linef.y1()),
                          qFloor(linef.x2()), qFloor(linef.y2()));
            }
        }
    }
}

void QX11PaintEngine::drawRects(const QRect *rects, int rectCount)
{
    Q_D(QX11PaintEngine);
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

    QRect clip(d->polygonClipper.boundingRect());
    QPoint offset(qRound(d->matrix.dx()), qRound(d->matrix.dy()));
#if !defined(QT_NO_XRENDER)
    ::Picture pict = d->picture;

    if (X11->use_xrender && pict && d->has_brush && d->pdev_depth != 1
        && (d->has_texture || d->has_alpha_brush))
    {
        XRenderColor xc;
        if (!d->has_texture && !d->has_pattern)
            xc = X11->preMultiply(d->cbrush.color());

        for (int i = 0; i < rectCount; ++i) {
            QRect r(rects[i]);
            if (d->txop == QPainterPrivate::TxTranslate)
                r.translate(offset);
            r = r.intersected(clip);
            if (r.isEmpty())
                continue;
            if (d->has_texture || d->has_pattern) {
                XRenderComposite(d->dpy, d->composition_mode, d->current_brush, 0, pict,
                                 qRound(r.x() - d->bg_origin.x()), qRound(r.y() - d->bg_origin.y()),
                                 0, 0, r.x(), r.y(), r.width(), r.height());
            } else {
                XRenderFillRectangle(d->dpy, d->composition_mode, pict, &xc, r.x(), r.y(), r.width(), r.height());
            }
            if (d->has_pen)
                XDrawRectangle(d->dpy, d->hd, d->gc, r.x(), r.y(), r.width(), r.height());
        }
    } else
#endif // !QT_NO_XRENDER
    {
        if (d->has_brush & d->has_pen) {
            for (int i = 0; i < rectCount; ++i) {
                QRect r(rects[i]);
                if (d->txop == QPainterPrivate::TxTranslate)
                    r.translate(offset);
                r = r.intersected(clip);
                if (r.isEmpty())
                    continue;
                d->setupAdaptedOrigin(r.topLeft());
                if (d->has_brush)
                    XFillRectangle(d->dpy, d->hd, d->gc_brush, r.x(), r.y(), r.width(), r.height());
                if (d->has_pen)
                    XDrawRectangle(d->dpy, d->hd, d->gc, r.x(), r.y(), r.width(), r.height());
            }
            d->resetAdaptedOrigin();
        } else {
            QVarLengthArray<XRectangle> xrects(rectCount);
            int numClipped = rectCount;
            for (int i = 0; i < rectCount; ++i) {
                QRect r(rects[i]);
                if (d->txop == QPainterPrivate::TxTranslate)
                    r.translate(offset);
                r = r.intersected(clip);
                if (r.isEmpty()) {
                    --numClipped;
                    continue;
                }
                xrects[i].x = short(r.x());
                xrects[i].y = short(r.y());
                xrects[i].width = ushort(r.width());
                xrects[i].height = ushort(r.height());

            }
            if (numClipped) {
                d->setupAdaptedOrigin(rects[0].topLeft());
                if (d->has_brush && !d->has_pen)
                    XFillRectangles(d->dpy, d->hd, d->gc_brush, xrects.data(), numClipped);
                else if (d->has_pen && !d->has_brush)
                    XDrawRectangles(d->dpy, d->hd, d->gc, xrects.data(), numClipped);
                d->resetAdaptedOrigin();
            }
        }
    }
}

void QX11PaintEngine::drawPoints(const QPoint *points, int pointCount)
{
    Q_ASSERT(points);
    Q_ASSERT(pointCount);
    Q_D(QX11PaintEngine);

    if (!d->has_pen)
        return;

    if (d->cpen.widthF() > .0f
        || d->has_alpha_brush
        || d->has_alpha_pen
        || d->has_custom_pen
        || (d->render_hints & QPainter::Antialiasing)) {
        const QPoint *end = points + pointCount;
        while (points < end) {
            QPainterPath path;
            path.moveTo(*points);
            path.lineTo(points->x()+.005, points->y());
            drawPath(path);
            ++points;
        }
        return;
    }

    static const int BUF_SIZE = 1024;
    XPoint xPoints[BUF_SIZE];
    int i = 0, j = 0;
    while (i < pointCount) {
        while (i < pointCount && j < BUF_SIZE) {
            const QPoint &xformed = d->matrix.map(points[i]);
            int x = xformed.x();
            int y = xformed.y();
            if (x >= SHRT_MIN && y >= SHRT_MIN && x < SHRT_MAX && y < SHRT_MAX) {
                xPoints[j].x = x;
                xPoints[j].y = y;
                ++j;
            }
            ++i;
        }
        if (j)
            XDrawPoints(d->dpy, d->hd, d->gc, xPoints, j, CoordModeOrigin);

        j = 0;
    }
}

void QX11PaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_ASSERT(points);
    Q_ASSERT(pointCount);
    Q_D(QX11PaintEngine);

    if (!d->has_pen)
        return;

    if (d->cpen.widthF() > .0f
        || d->has_alpha_brush
        || d->has_alpha_pen
        || d->has_custom_pen
        || (d->render_hints & QPainter::Antialiasing)) {
        const QPointF *end = points + pointCount;
        while (points < end) {
            QPainterPath path;
            path.moveTo(*points);
            path.lineTo(points->x() + 0.005, points->y());
            drawPath(path);
            ++points;
        }
        return;
    }

    static const int BUF_SIZE = 1024;
    XPoint xPoints[BUF_SIZE];
    int i = 0, j = 0;
    while (i < pointCount) {
        while (i < pointCount && j < BUF_SIZE) {
            const QPointF &xformed = d->matrix.map(points[i]);
            int x = qFloor(xformed.x());
            int y = qFloor(xformed.y());
            if (x >= SHRT_MIN && y >= SHRT_MIN && x < SHRT_MAX && y < SHRT_MAX) {
                xPoints[j].x = x;
                xPoints[j].y = y;
                ++j;
            }
            ++i;
        }
        if (j)
            XDrawPoints(d->dpy, d->hd, d->gc, xPoints, j, CoordModeOrigin);

        j = 0;
    }
}

QPainter::RenderHints QX11PaintEngine::supportedRenderHints() const
{
#if !defined(QT_NO_XRENDER)
    if (X11->use_xrender)
        return QPainter::Antialiasing;
#endif
    return QFlag(0);
}

void QX11PaintEngine::updateState(const QPaintEngineState &state)
{
    Q_D(QX11PaintEngine);
    QPaintEngine::DirtyFlags flags = state.state();


    if (flags & DirtyOpacity) {
        d->opacity = state.opacity();
        if (d->opacity > 1)
            d->opacity = 1;
        if (d->opacity < 0)
            d->opacity = 0;
        // Force update pen/brush as to get proper alpha colors propagated
        flags |= DirtyPen;
        flags |= DirtyBrush;
    }

    if (flags & DirtyTransform) updateMatrix(state.matrix());
    if (flags & (DirtyBackground | DirtyBackgroundMode))
        updateBackground(state.backgroundMode(), state.backgroundBrush());
    if (flags & DirtyPen) updatePen(state.pen());
    if (flags & (DirtyBrush | DirtyBrushOrigin)) updateBrush(state.brush(), state.brushOrigin());
    if (flags & DirtyFont) updateFont(state.font());

    if (state.state() & DirtyClipEnabled) {
        if (state.isClipEnabled()) {
            QPolygonF clip_poly_dev(d->matrix.map(painter()->clipPath().toFillPolygon()));
            QPolygonF clipped_poly_dev;
            d->clipPolygon_dev(clip_poly_dev, &clipped_poly_dev);
            updateClipRegion_dev(QRegion(clipped_poly_dev.toPolygon()), Qt::ReplaceClip);
        } else {
            updateClipRegion_dev(QRegion(), Qt::NoClip);
        }
    }

    if (flags & DirtyClipPath) {
        QPolygonF clip_poly_dev(d->matrix.map(state.clipPath().toFillPolygon()));
        QPolygonF clipped_poly_dev;
        d->clipPolygon_dev(clip_poly_dev, &clipped_poly_dev);
        updateClipRegion_dev(QRegion(clipped_poly_dev.toPolygon(), state.clipPath().fillRule()),
                             state.clipOperation());
    } else if (flags & DirtyClipRegion) {
        updateClipRegion_dev(d->matrix.map(state.clipRegion()), state.clipOperation());
    }
    if (flags & DirtyHints) updateRenderHints(state.renderHints());
#if !defined(QT_NO_XRENDER)
    if (flags & DirtyCompositionMode) {
        d->composition_mode =
            qpainterOpToXrender(state.compositionMode());
    }
#endif
    d->decidePathFallback();
}

void QX11PaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    Q_D(QX11PaintEngine);
    d->render_hints = hints;
    if ((d->txop > QPainterPrivate::TxTranslate)
        || (hints & QPainter::Antialiasing))
        d->use_path_fallback = true;
    else
        d->use_path_fallback = false;

#if !defined(QT_NO_XRENDER)
    if (X11->use_xrender && d->picture) {
        XRenderPictureAttributes attrs;
        attrs.poly_edge = (hints & QPainter::Antialiasing) ? PolyEdgeSmooth : PolyEdgeSharp;
        XRenderChangePicture(d->dpy, d->picture, CPPolyEdge, &attrs);
    }
#endif
}

void QX11PaintEngine::updatePen(const QPen &pen)
{
    Q_D(QX11PaintEngine);
    d->cpen = pen;
    int cp = CapButt;
    int jn = JoinMiter;
    int ps = pen.style();

    if (d->opacity < 1.0) {
        QColor c = d->cpen.color();
        c.setAlpha(qRound(c.alpha()*d->opacity));
        d->cpen.setColor(c);
    }

    d->has_pen = (ps != Qt::NoPen);
    d->has_alpha_pen = (pen.color().alpha() != 255);

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

    d->adapted_pen_origin = false;

    char dashes[10];                            // custom pen dashes
    int dash_len = 0;                           // length of dash list
    int xStyle = LineSolid;

    /*
      We are emulating Windows here.  Windows treats cpen.width() == 1
      (or 0) as a very special case.  The fudge variable unifies this
      case with the general case.
    */
    qreal pen_width = pen.widthF();
    int scale =  qRound(pen_width < 1 ? 1 : pen_width);
    int space = (pen_width < 1 ? 1 : (2 * scale));
    int dot = 1 * scale;
    int dash = 4 * scale;

    d->has_custom_pen = false;

    switch (ps) {
    case Qt::NoPen:
    case Qt::SolidLine:
        xStyle = LineSolid;
	break;
    case Qt::DashLine:
	dashes[0] = dash;
	dashes[1] = space;
	dash_len = 2;
        xStyle = LineOnOffDash;
	break;
    case Qt::DotLine:
	dashes[0] = dot;
	dashes[1] = space;
	dash_len = 2;
        xStyle = LineOnOffDash;
	break;
    case Qt::DashDotLine:
	dashes[0] = dash;
	dashes[1] = space;
	dashes[2] = dot;
	dashes[3] = space;
	dash_len = 4;
        xStyle = LineOnOffDash;
	break;
    case Qt::DashDotDotLine:
	dashes[0] = dash;
	dashes[1] = space;
	dashes[2] = dot;
	dashes[3] = space;
	dashes[4] = dot;
	dashes[5] = space;
	dash_len = 6;
        xStyle = LineOnOffDash;
        break;
    case Qt::CustomDashLine:
        d->has_custom_pen = true;
        break;
    }

    ulong mask = GCForeground | GCBackground | GCGraphicsExposures | GCLineWidth
                 | GCCapStyle | GCJoinStyle | GCLineStyle;
    XGCValues vals;
    vals.graphics_exposures = false;
    if (d->pdev_depth == 1) {
        vals.foreground = qGray(pen.color().rgb()) > 127 ? 0 : 1;
        vals.background = qGray(d->bg_col.rgb()) > 127 ? 0 : 1;
    } else if (d->pdev->devType() == QInternal::Pixmap && d->pdev_depth == 32
        && X11->use_xrender) {
        vals.foreground = pen.color().rgba();
        vals.background = d->bg_col.rgba();
    } else {
        QColormap cmap = QColormap::instance(d->scrn);
        vals.foreground = cmap.pixel(pen.color());
        vals.background = cmap.pixel(d->bg_col);
    }


    vals.line_width = qRound(pen.widthF());
    vals.cap_style = cp;
    vals.join_style = jn;
    vals.line_style = xStyle;

    XChangeGC(d->dpy, d->gc, mask, &vals);

    if (dash_len) { // make dash list
        XSetDashes(d->dpy, d->gc, 0, dashes, dash_len);
    }

    if (!d->has_clipping) { // if clipping is set the paintevent clip region is merged with the clip region
        QRegion sysClip = systemClip();
        if (!sysClip.isEmpty())
            x11SetClipRegion(d->dpy, d->gc, 0, d->picture, sysClip);
        else
            x11ClearClipRegion(d->dpy, d->gc, 0, d->picture);
    }
}

void QX11PaintEngine::updateBrush(const QBrush &brush, const QPointF &origin)
{
    Q_D(QX11PaintEngine);
    d->cbrush = brush;
    d->bg_origin = origin;
    d->adapted_brush_origin = false;
#if !defined(QT_NO_XRENDER)
    d->current_brush = 0;
#endif
    if (d->opacity < 1.0) {
        QColor c = d->cbrush.color();
        c.setAlpha(qRound(c.alpha()*d->opacity));
        d->cbrush.setColor(c);
    }

    int s  = FillSolid;
    int  bs = d->cbrush.style();
    d->has_brush = (bs != Qt::NoBrush);
    d->has_pattern = bs >= Qt::Dense1Pattern && bs <= Qt::DiagCrossPattern;
    d->has_texture = bs == Qt::TexturePattern;
    d->has_alpha_brush = brush.color().alpha() != 255;

    ulong mask = GCForeground | GCBackground | GCGraphicsExposures
                 | GCLineStyle | GCCapStyle | GCJoinStyle | GCFillStyle;
    XGCValues vals;
    vals.graphics_exposures = false;
    if (d->pdev_depth == 1) {
        vals.foreground = qGray(d->cbrush.color().rgb()) > 127 ? 0 : 1;
        vals.background = qGray(d->bg_col.rgb()) > 127 ? 0 : 1;
    } else if (X11->use_xrender && d->pdev->devType() == QInternal::Pixmap
               && d->pdev_depth == 32) {
        vals.foreground = d->cbrush.color().rgba();
        vals.background = d->bg_col.rgba();
    } else {
        QColormap cmap = QColormap::instance(d->scrn);
        vals.foreground = cmap.pixel(d->cbrush.color());
        vals.background = cmap.pixel(d->bg_col);

        if (!d->has_pattern && !brush.isOpaque()) {
            QPixmap pattern = qt_patternForAlpha(brush.color().alpha());
            mask |= GCStipple;
            vals.stipple = pattern.handle();
            s = FillStippled;
            d->adapted_brush_origin = true;
        }
    }
    vals.cap_style = CapButt;
    vals.join_style = JoinMiter;
    vals.line_style = LineSolid;

    if (d->has_pattern || d->has_texture) {
        QPixmap pm;
        if (bs == Qt::TexturePattern) {
            pm = d->cbrush.texture();
#if !defined(QT_NO_XRENDER)
            if (X11->use_xrender) {
                XRenderPictureAttributes attrs;
                attrs.repeat = true;
                XRenderChangePicture(d->dpy, d->cbrush.texture().x11PictureHandle(), CPRepeat, &attrs);
            }
#endif
        } else {
            pm = qt_pixmapForBrush(bs, true);
        }
        pm.x11SetScreen(d->scrn);
        if (pm.depth() == 1) {
            mask |= GCStipple;
            vals.stipple = pm.handle();
            s = d->bg_mode == Qt::TransparentMode ? FillStippled : FillOpaqueStippled;
#if !defined(QT_NO_XRENDER)
            if (X11->use_xrender) {
                d->bitmap_texture = QPixmap(pm.size());

                if (d->bg_mode == Qt::OpaqueMode)
                    d->bitmap_texture.fill(d->bg_brush.color());
                else
                    d->bitmap_texture.fill(Qt::transparent);

                ::Picture src  = X11->getSolidFill(d->scrn, d->cbrush.color());
                XRenderComposite(d->dpy, PictOpSrc, src, pm.x11PictureHandle(),
                                 d->bitmap_texture.x11PictureHandle(),
                                 0, 0, pm.width(), pm.height(),
                                 0, 0, pm.width(), pm.height());

                XRenderPictureAttributes attrs;
                attrs.repeat = true;
                XRenderChangePicture(d->dpy, d->bitmap_texture.x11PictureHandle(), CPRepeat, &attrs);

                d->current_brush = d->bitmap_texture.x11PictureHandle();
            }
#endif
        } else {
            mask |= GCTile;
#ifndef QT_NO_XRENDER
            if (d->pdev_depth == 32 && pm.depth() != 32)
                pm.data->convertToARGB32();
#endif
            vals.tile = (pm.depth() == d->pdev_depth
                         ? pm.handle()
                         : pm.data->x11ConvertToDefaultDepth());
            s = FillTiled;
#if !defined(QT_NO_XRENDER)
            d->current_brush = d->cbrush.texture().x11PictureHandle();
#endif
        }

        mask |= GCTileStipXOrigin | GCTileStipYOrigin;
        vals.ts_x_origin = qRound(origin.x());
        vals.ts_y_origin = qRound(origin.y());
    }
#if !defined(QT_NO_XRENDER)
    else if (d->has_alpha_brush) {
        d->current_brush = X11->getSolidFill(d->scrn, d->cbrush.color());
    }
#endif

    vals.fill_style = s;
    XChangeGC(d->dpy, d->gc_brush, mask, &vals);
    if (!d->has_clipping) {
        QRegion sysClip = systemClip();
        if (!sysClip.isEmpty())
            x11SetClipRegion(d->dpy, d->gc_brush, 0, d->picture, sysClip);
        else
            x11ClearClipRegion(d->dpy, d->gc_brush, 0, d->picture);
    }
}

void QX11PaintEngine::drawEllipse(const QRect &rect)
{
    Q_D(QX11PaintEngine);
    QRect devclip(SHRT_MIN, SHRT_MIN, SHRT_MAX*2 - 1, SHRT_MAX*2 - 1);
    QRect r(rect);
    if (d->txop == QPainterPrivate::TxTranslate)
        r.translate(qRound(d->matrix.dx()), qRound(d->matrix.dy()));
    if (d->use_path_fallback || devclip.intersected(r) != r) {
        QPainterPath path;
        path.addEllipse(rect);
        drawPath(path);
        return;
    }

    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();
    if (w < 1 || h < 1)
        return;
    if (w == 1 && h == 1) {
        XDrawPoint(d->dpy, d->hd, d->has_pen ? d->gc : d->gc_brush, x, y);
        return;
    }
    d->setupAdaptedOrigin(rect.topLeft());
    if (d->has_brush) {          // draw filled ellipse
        if (!d->has_pen) {
            XFillArc(d->dpy, d->hd, d->gc_brush, x, y, w-1, h-1, 0, 360*64);
            XDrawArc(d->dpy, d->hd, d->gc_brush, x, y, w-1, h-1, 0, 360*64);
            return;
        } else{
            XFillArc(d->dpy, d->hd, d->gc_brush, x, y, w, h, 0, 360*64);
        }
    }
    if (d->has_pen)                // draw outline
        XDrawArc(d->dpy, d->hd, d->gc, x, y, w, h, 0, 360*64);
    d->resetAdaptedOrigin();
}



void QX11PaintEnginePrivate::fillPolygon_translated(const QPointF *polygonPoints, int pointCount,
                                                    QX11PaintEnginePrivate::GCMode gcMode,
                                                    QPaintEngine::PolygonDrawMode mode)
{

    QVarLengthArray<QPointF> translated_points(pointCount);
    QPointF offset(matrix.dx(), matrix.dy());
    for (int i = 0; i < pointCount; ++i)
        translated_points[i] = polygonPoints[i] + offset;
    fillPolygon_dev(translated_points.data(), pointCount, gcMode, mode);
}

#ifndef QT_NO_XRENDER
static void qt_XRenderCompositeTrapezoids(Display *dpy,
                                          int op,
                                          Picture src,
                                          Picture dst,
                                          _Xconst XRenderPictFormat *maskFormat,
                                          int xSrc,
                                          int ySrc,
                                          const QVector<XTrapezoid> &traps)
{
    const int MAX_TRAPS = 50000;
    int traps_left = traps.size();
    while (traps_left) {
        int to_draw = traps_left;
        if (to_draw > MAX_TRAPS)
            to_draw = MAX_TRAPS;
        XRenderCompositeTrapezoids(dpy, op, src, dst,
                                   maskFormat,
                                   xSrc, ySrc,
                                   traps.constData()+traps.size()-traps_left, to_draw);
        traps_left -= to_draw;
    }
}
#endif

void QX11PaintEnginePrivate::fillPolygon_dev(const QPointF *polygonPoints, int pointCount,
                                             QX11PaintEnginePrivate::GCMode gcMode,
                                             QPaintEngine::PolygonDrawMode mode)
{
    int clippedCount = 0;
    qt_float_point *clippedPoints = 0;

#ifndef QT_NO_XRENDER
    //can change if we switch to pen if gcMode != BrushGC
    bool has_fill_texture = has_texture;
    bool has_fill_pattern = has_pattern;
    ::Picture src;
#endif
    QBrush fill;
    GC fill_gc;
    if (gcMode == BrushGC) {
        fill = cbrush;
        fill_gc = gc_brush;
#ifndef QT_NO_XRENDER
        if (current_brush)
            src = current_brush;
        else
            src = X11->getSolidFill(scrn, fill.color());
#endif
    } else {
        fill = QBrush(cpen.brush());
        fill_gc = gc;
#ifndef QT_NO_XRENDER
        //we use the pens brush
        has_fill_texture = (fill.style() == Qt::TexturePattern);
        has_fill_pattern = (fill.style() >= Qt::Dense1Pattern && fill.style() <= Qt::DiagCrossPattern);
        if (has_fill_texture)
            src = fill.texture().x11PictureHandle();
        else if (has_fill_pattern)
            src = getPatternFill(scrn, fill, bg_brush, bg_mode == Qt::OpaqueMode);
        else
            src = X11->getSolidFill(scrn, fill.color());
#endif
    }

    polygonClipper.clipPolygon((qt_float_point *) polygonPoints, pointCount,
                               &clippedPoints, &clippedCount);

#ifndef QT_NO_XRENDER
    bool solid_fill = fill.color().alpha() == 255;
    if (has_fill_texture && fill.texture().depth() == 1 && solid_fill) {
        has_fill_texture = false;
        has_fill_pattern = true;
    }

    bool antialias = render_hints & QPainter::Antialiasing;
    if (X11->use_xrender
        && picture
        && !has_fill_pattern
        && (clippedCount > 0)
        && (fill.style() != Qt::NoBrush)
        && (has_fill_texture || antialias || !solid_fill || has_alpha_pen != has_alpha_brush))
    {
        QVector<XTrapezoid> traps;
        traps.reserve(128);
        QRect br;
        qt_tesselate_polygon(&traps, (QPointF *)clippedPoints, clippedCount,
                             mode == QPaintEngine::WindingMode, &br);
        if (traps.size() > 0) {
            XRenderPictureAttributes attrs;
            attrs.poly_edge = antialias ? PolyEdgeSmooth : PolyEdgeSharp;
            XRenderChangePicture(dpy, picture, CPPolyEdge, &attrs);

            if (has_fill_texture) {
                if (fill.texture().depth() == 1) {
                    for (int i=0; i < traps.size(); ++i) {
                        int x_offset = int(XFixedToDouble(traps.at(i).left.p1.x) - bg_origin.x());
                        int y_offset = int(XFixedToDouble(traps.at(i).left.p1.y) - bg_origin.y());
                        XRenderCompositeTrapezoids(dpy, composition_mode, src, picture,
                                                   antialias ? XRenderFindStandardFormat(dpy, PictStandardA8) : 0,
                                                   x_offset, y_offset,
                                                   traps.constData() + i, 1);
                    }
                } else {
                    int mask_w = br.width() + (br.x() > 0 ? br.x() : 0);
                    int mask_h = br.height() + (br.y() > 0 ? br.y() : 0);
                    Pixmap mask = XCreatePixmap (dpy, RootWindow(dpy, scrn),
                                                 mask_w, mask_h, antialias ? 8 : 1);
                    Picture mask_picture = XRenderCreatePicture (dpy, mask,
                                                                 antialias ? XRenderFindStandardFormat(dpy, PictStandardA8)
                                                                 : XRenderFindStandardFormat(dpy, PictStandardA1),
                                                                 CPPolyEdge, &attrs);
                    XRenderColor transparent;
                    transparent.red = 0;
                    transparent.green = 0;
                    transparent.blue = 0;
                    transparent.alpha = 0;
                    XRenderFillRectangle(dpy, PictOpSrc, mask_picture, &transparent, 0, 0, mask_w, mask_h);

                    Picture mask_src = X11->getSolidFill(scrn, Qt::white);
                    qt_XRenderCompositeTrapezoids(dpy, PictOpOver, mask_src, mask_picture,
                                                  antialias ? XRenderFindStandardFormat(dpy, PictStandardA8) : 0,
                                                  0, 0,
                                                  traps);
                    XRenderComposite(dpy, composition_mode, src, mask_picture, picture,
                                     qRound(bg_origin.x()), qRound(bg_origin.y()),
                                     0, 0,
                                     0, 0,
                                     mask_w, mask_h);
                    XFreePixmap(dpy, mask);
                    XRenderFreePicture(dpy, mask_picture);
                }
            } else {
                qt_XRenderCompositeTrapezoids(dpy, composition_mode, src, picture,
                                              antialias ? XRenderFindStandardFormat(dpy, PictStandardA8) : 0,
                                              0, 0,
                                              traps);
            }
        }
    } else
#endif
        if (fill.style() != Qt::NoBrush) {
            if (clippedCount > 0) {
                QVarLengthArray<XPoint> xpoints(clippedCount);
                for (int i = 0; i < clippedCount; ++i) {
                    xpoints[i].x = qFloor(clippedPoints[i].x);
                    xpoints[i].y = qFloor(clippedPoints[i].y);
                }
                if (mode == QPaintEngine::WindingMode)
                    XSetFillRule(dpy, fill_gc, WindingRule);
                setupAdaptedOrigin(QPoint(xpoints[0].x, xpoints[0].y));
                XFillPolygon(dpy, hd, fill_gc,
                             xpoints.data(), clippedCount,
                             mode == QPaintEngine::ConvexMode ? Convex : Complex, CoordModeOrigin);
                resetAdaptedOrigin();
                if (mode == QPaintEngine::WindingMode)
                    XSetFillRule(dpy, fill_gc, EvenOddRule);
            }
        }
}

void QX11PaintEnginePrivate::strokePolygon_translated(const QPointF *polygonPoints, int pointCount, bool close)
{
    QVarLengthArray<QPointF> translated_points(pointCount);
    QPointF offset(matrix.dx(), matrix.dy());
    for (int i = 0; i < pointCount; ++i)
        translated_points[i] = polygonPoints[i] + offset;
    strokePolygon_dev(translated_points.data(), pointCount, close);
}

void QX11PaintEnginePrivate::strokePolygon_dev(const QPointF *polygonPoints, int pointCount, bool close)
{
    int clippedCount = 0;
    qt_float_point *clippedPoints = 0;
    polygonClipper.clipPolygon((qt_float_point *) polygonPoints, pointCount,
                               &clippedPoints, &clippedCount, close);

    if (clippedCount > 0) {
        QVarLengthArray<XPoint> xpoints(clippedCount);
        for (int i = 0; i < clippedCount; ++i) {
            xpoints[i].x = qFloor(clippedPoints[i].x);
            xpoints[i].y = qFloor(clippedPoints[i].y);
        }
        uint numberPoints = qMin(clippedCount, xlibMaxLinePoints);
        XPoint *pts = xpoints.data();
        XDrawLines(dpy, hd, gc, pts, numberPoints, CoordModeOrigin);
        pts += numberPoints;
        clippedCount -= numberPoints;
        numberPoints = qMin(clippedCount, xlibMaxLinePoints-1);
        while (clippedCount) {
            XDrawLines(dpy, hd, gc, pts-1, numberPoints+1, CoordModeOrigin);
            pts += numberPoints;
            clippedCount -= numberPoints;
            numberPoints = qMin(clippedCount, xlibMaxLinePoints-1);
        }
    }
}

void QX11PaintEngine::drawPolygon(const QPointF *polygonPoints, int pointCount, PolygonDrawMode mode)
{
    Q_D(QX11PaintEngine);
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
    if (mode != PolylineMode && d->has_brush)
        d->fillPolygon_translated(polygonPoints, pointCount, QX11PaintEnginePrivate::BrushGC, mode);

    if (d->has_pen)
        d->strokePolygon_translated(polygonPoints, pointCount, mode != PolylineMode);
}


void QX11PaintEnginePrivate::fillPath(const QPainterPath &path, QX11PaintEnginePrivate::GCMode gc_mode, bool transform)
{
    QList<QPolygonF> polys = path.toFillPolygons(transform ? matrix : QMatrix());
    for (int i = 0; i < polys.size(); ++i) {
        fillPolygon_dev(polys.at(i).data(), polys.at(i).size(), gc_mode,
                        path.fillRule() == Qt::OddEvenFill ? QPaintEngine::OddEvenMode : QPaintEngine::WindingMode);
    }
}

void QX11PaintEngine::drawPath(const QPainterPath &path)
{
    Q_D(QX11PaintEngine);
    if (path.isEmpty())
        return;
    bool adjust_coords = d->has_alpha_pen && !(d->render_hints & QPainter::Antialiasing);
    QMatrix old_matrix = d->matrix;
    if (adjust_coords) {
        d->matrix = QMatrix(d->matrix.m11(), d->matrix.m12(), d->matrix.m21(), d->matrix.m22(),
                            d->matrix.dx() + 0.5f, d->matrix.dy() + 0.5f);
    }
    if (d->has_brush)
        d->fillPath(path, QX11PaintEnginePrivate::BrushGC, true);

    if (d->has_pen
        && ((X11->use_xrender && (d->has_alpha_pen
                                  || (d->render_hints & QPainter::Antialiasing)))
            || (d->cpen.widthF() > 0 && d->txop > QPainterPrivate::TxTranslate)
            || (d->cpen.style() > Qt::SolidLine))) {
        QPainterPathStroker stroker;
        if (d->cpen.style() == Qt::CustomDashLine)
            stroker.setDashPattern(d->cpen.dashPattern());
        else
            stroker.setDashPattern(d->cpen.style());
        stroker.setCapStyle(d->cpen.capStyle());
        stroker.setJoinStyle(d->cpen.joinStyle());
        QPainterPath stroke;
        qreal width = d->cpen.widthF();
        QPolygonF poly;
        // necessary to get aliased alphablended primitives to be drawn correctly
        if (width == 0) {
            stroker.setWidth(1);
            stroke = stroker.createStroke(path * d->matrix);
            if (stroke.isEmpty())
                return;
            stroke.setFillRule(Qt::WindingFill);
            d->fillPath(stroke, QX11PaintEnginePrivate::PenGC, false);
        } else {
            stroker.setWidth(width);
            stroke = stroker.createStroke(path);
            if (stroke.isEmpty())
                return;
            stroke.setFillRule(Qt::WindingFill);
            d->fillPath(stroke, QX11PaintEnginePrivate::PenGC, true);
        }
    } else if (d->has_pen) {
        // if we have a pen width of 0 - use XDrawLine() for speed
        QList<QPolygonF> polys = path.toSubpathPolygons(d->matrix);
        for (int i = 0; i < polys.size(); ++i)
            d->strokePolygon_dev(polys.at(i).data(), polys.at(i).size(), false);
    }
    if (adjust_coords)
        d->matrix = old_matrix;
}

void QX11PaintEngine::drawImage(const QRectF &r, const QImage &image, const QRectF &sr, Qt::ImageConversionFlags flags)
{
    Q_D(QX11PaintEngine);

    if (!image.hasAlphaChannel()
        && d->pdev_depth >= 24 && image.depth() == 32
        && r.size() == sr.size())
    {
        int sx = qRound(sr.x());
        int sy = qRound(sr.y());
        int x = qRound(r.x());
        int y = qRound(r.y());
        int w = qRound(r.width());
        int h = qRound(r.height());
        XImage *xi;
        xi = XCreateImage(d->dpy, (Visual *) d->xinfo->visual(), d->pdev_depth, ZPixmap,
                          0, (char *) image.scanLine(sy)+sx*sizeof(uint), w, h, 32, image.bytesPerLine());
        XPutImage(d->dpy, d->hd, d->gc, xi, 0, 0, x, y, w, h);
        xi->data = 0; // QImage owns these bits
        XDestroyImage(xi);
    } else {
        QPaintEngine::drawImage(r, image, sr, flags);
    }
}

void QX11PaintEngine::drawPixmap(const QRectF &r, const QPixmap &pixmap, const QRectF &_sr)
{
    Q_D(QX11PaintEngine);
    QRectF sr = _sr;
    if (r.size() != sr.size()) {
        QImage image = pixmap.toImage();
        image = image.copy(sr.toRect());
        image = image.scaled(qRound(r.width()), qRound(r.height()), Qt::IgnoreAspectRatio,
                             d->render_hints & QPainter::SmoothPixmapTransform
                             ? Qt::SmoothTransformation : Qt::FastTransformation);
        sr = QRectF(0, 0, r.width(), r.height());
        // this recursive call here prevents us from doing a pixmap assignment,
        // and thus triggering a deep copy if pixmap is being painted on
        // Nice trick to speed such things up...
        drawPixmap(r, QPixmap::fromImage(image), sr);
        return;
    }

    int x = qRound(r.x());
    int y = qRound(r.y());
    int sx = qRound(sr.x());
    int sy = qRound(sr.y());
    int sw = qRound(sr.width());
    int sh = qRound(sr.height());

    if ((d->xinfo && d->xinfo->screen() != pixmap.x11Info().screen())
        || (pixmap.x11Info().screen() != DefaultScreen(X11->display))) {
        QPixmap* p = const_cast<QPixmap *>(&pixmap);
        p->x11SetScreen(d->xinfo ? d->xinfo->screen() : DefaultScreen(X11->display));
    }

    QPixmap::x11SetDefaultScreen(pixmap.x11Info().screen());

#ifndef QT_NO_XRENDER
    ::Picture src_pict = pixmap.data->picture;
    if (src_pict && d->picture) {
        if (pixmap.data->d == 1 && (d->has_alpha_pen || d->bg_brush != Qt::NoBrush)) {
            qt_render_bitmap(d->dpy, d->scrn, src_pict, d->picture,
                             sx, sy, x, y, sw, sh, d->cpen, d->bg_brush,
                             d->bg_mode == Qt::OpaqueMode);
            return;
        } else if (pixmap.data->d != 1 && (pixmap.data->d == 32 || pixmap.data->d != d->pdev_depth)) {
            XRenderComposite(d->dpy, d->composition_mode,
                             src_pict, 0, d->picture, sx, sy, 0, 0, x, y, sw, sh);
            return;
        }
    }
#endif

    bool mono_src = pixmap.data->d == 1;
    bool mono_dst = d->pdev_depth == 1;
    bool restore_clip = false;

    if (pixmap.data->x11_mask) { // pixmap has a mask
        QBitmap comb(sw, sh);
        GC cgc = XCreateGC(d->dpy, comb.handle(), 0, 0);
        XSetForeground(d->dpy, cgc, 0);
        XFillRectangle(d->dpy, comb.handle(), cgc, 0, 0, sw, sh);
        XSetBackground(d->dpy, cgc, 0);
        XSetForeground(d->dpy, cgc, 1);
        if (!d->crgn.isEmpty()) {
            int num;
            XRectangle *rects = (XRectangle *)qt_getClipRects(d->crgn, num);
            XSetClipRectangles(d->dpy, cgc, -x, -y, rects, num, Unsorted);
        }
        XSetFillStyle(d->dpy, cgc, FillOpaqueStippled);
        XSetTSOrigin(d->dpy, cgc, -sx, -sy);
        XSetStipple(d->dpy, cgc, pixmap.data->x11_mask);
        XFillRectangle(d->dpy, comb.handle(), cgc, 0, 0, sw, sh);
        XFreeGC(d->dpy, cgc);

        XSetClipOrigin(d->dpy, d->gc, x, y);
        XSetClipMask(d->dpy, d->gc, comb.handle());
        restore_clip = true;
    }

    if (mono_src) {
        if (d->bg_mode == Qt::OpaqueMode) {
            if (mono_dst)
                XSetForeground(d->dpy, d->gc, qGray(d->bg_brush.color().rgb()) > 127 ? 0 : 1);
            else
                XSetForeground(d->dpy, d->gc, QColormap::instance(d->scrn).pixel(d->bg_brush.color()));
            XFillRectangle(d->dpy, d->hd, d->gc, x, y, sw, sh);
        }

        if (!d->crgn.isEmpty()) {
            Pixmap comb = XCreatePixmap(d->dpy, d->hd, sw, sh, 1);
            GC cgc = XCreateGC(d->dpy, comb, 0, 0);
            XSetForeground(d->dpy, cgc, 0);
            XFillRectangle(d->dpy, comb, cgc, 0, 0, sw, sh);
            int num;
            XRectangle *rects = (XRectangle *)qt_getClipRects(d->crgn, num);
            XSetClipRectangles(d->dpy, cgc, -x, -y, rects, num, Unsorted);
            XCopyArea(d->dpy, pixmap.handle(), comb, cgc, 0, 0, sw, sh, 0, 0);
            XFreeGC(d->dpy, cgc);

            XSetClipMask(d->dpy, d->gc, comb);
            XSetClipOrigin(d->dpy, d->gc, x, y);
            XFreePixmap(d->dpy, comb);
        } else {
            XSetClipMask(d->dpy, d->gc, pixmap.handle());
            XSetClipOrigin(d->dpy, d->gc, x, y);
        }

        if (mono_dst) {
            XSetBackground(d->dpy, d->gc, qGray(d->bg_brush.color().rgb()) > 127 ? 0 : 1);
            XSetForeground(d->dpy, d->gc, qGray(d->cpen.color().rgb()) > 127 ? 0 : 1);
        } else {
            QColormap cmap = QColormap::instance(d->scrn);
            XSetBackground(d->dpy, d->gc, cmap.pixel(d->bg_brush.color()));
            XSetForeground(d->dpy, d->gc, cmap.pixel(d->cpen.color()));
        }
        XFillRectangle(d->dpy, d->hd, d->gc, x, y, sw, sh);
        restore_clip = true;
    } else {
        XCopyArea(d->dpy, pixmap.handle(), d->hd, d->gc, sx, sy, sw, sh, x, y);
    }

    if (d->pdev->devType() == QInternal::Pixmap) {
        const QPixmap *px = static_cast<const QPixmap*>(d->pdev);
        Pixmap src_mask = pixmap.data->x11_mask;
        Pixmap dst_mask = px->data->x11_mask;
        if (dst_mask) {
            GC cgc = XCreateGC(d->dpy, dst_mask, 0, 0);
            if (src_mask) { // copy src mask into dst mask
                XCopyArea(d->dpy, pixmap.data->x11_mask, px->data->x11_mask, cgc, sx, sy, sw, sh, x, y);
            } else { // no src mask, but make sure the area copied is opaque in dest
                XSetBackground(d->dpy, cgc, 0);
                XSetForeground(d->dpy, cgc, 1);
                XFillRectangle(d->dpy, px->data->x11_mask, cgc, x, y, sw, sh);
            }
            XFreeGC(d->dpy, cgc);
        }
    }

    if (restore_clip) {
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
    Q_D(QX11PaintEngine);
    d->bg_mode = mode;
    d->bg_brush = bgBrush;
    if (d->opacity < 1.0) {
        QColor c = d->bg_brush.color();
        c.setAlpha(qRound(c.alpha()*d->opacity));
        d->bg_brush.setColor(c);
    }
    d->bg_col = d->bg_brush.color();
    updatePen(d->cpen);
    updateBrush(d->cbrush, d->bg_origin);
}

void QX11PaintEngine::updateMatrix(const QMatrix &mtx)
{
    Q_D(QX11PaintEngine);
    d->matrix = mtx;
    if (mtx.m12() != 0 || mtx.m21() != 0)
        d->txop = QPainterPrivate::TxRotShear;
    else if (mtx.m11() != 1 || mtx.m22() != 1)
        d->txop = QPainterPrivate::TxScale;
    else if (mtx.dx() != 0 || mtx.dy() != 0)
        d->txop = QPainterPrivate::TxTranslate;
    else
        d->txop = QPainterPrivate::TxNone;

    d->has_complex_xform = (d->txop > QPainterPrivate::TxTranslate);
}

/*
   NB! the clip region is expected to be in dev coordinates
*/
void QX11PaintEngine::updateClipRegion_dev(const QRegion &clipRegion, Qt::ClipOperation op)
{
    Q_D(QX11PaintEngine);
    QRegion sysClip = systemClip();
    if (op == Qt::NoClip) {
        d->has_clipping = false;
        d->crgn = sysClip;
        if (!sysClip.isEmpty()) {
            x11SetClipRegion(d->dpy, d->gc, d->gc_brush, d->picture, sysClip);
        } else {
            x11ClearClipRegion(d->dpy, d->gc, d->gc_brush, d->picture);
        }
        return;
    }

    switch (op) {
    case Qt::IntersectClip:
        if (d->has_clipping) {
            d->crgn &= clipRegion;
            break;
        }
        // fall through
    case Qt::ReplaceClip:
        if (!sysClip.isEmpty())
            d->crgn = clipRegion.intersected(sysClip);
        else
            d->crgn = clipRegion;
        break;
    case Qt::UniteClip:
        d->crgn |= clipRegion;
        if (!sysClip.isEmpty())
            d->crgn = d->crgn.intersected(sysClip);
        break;
    default:
        break;
    }
    d->has_clipping = true;
    x11SetClipRegion(d->dpy, d->gc, d->gc_brush, d->picture, d->crgn);
}

void QX11PaintEngine::updateFont(const QFont &)
{
}

Qt::HANDLE QX11PaintEngine::handle() const
{
    Q_D(const QX11PaintEngine);
    Q_ASSERT(isActive());
    Q_ASSERT(d->hd);
    return d->hd;
}

extern void qt_draw_tile(QPaintEngine *, qreal, qreal, qreal, qreal, const QPixmap &,
                         qreal, qreal);

void QX11PaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &p)
{
    int x = qRound(r.x());
    int y = qRound(r.y());
    int w = qRound(r.width());
    int h = qRound(r.height());
    int sx = qRound(p.x());
    int sy = qRound(p.y());

    Q_D(QX11PaintEngine);
#ifndef QT_NO_XRENDER
    if (X11->use_xrender && d->picture && pixmap.x11PictureHandle()) {
        // this is essentially qt_draw_tile(), inlined for
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
                if (pixmap.depth() == 1) {
                    qt_render_bitmap(d->dpy, d->scrn, pixmap.x11PictureHandle(), d->picture,
                                     xOff, yOff, xPos, yPos, drawW, drawH, d->cpen, d->bg_brush,
                                     d->bg_mode == Qt::OpaqueMode);
                } else {
                    XRenderComposite(d->dpy, d->composition_mode,
                                     pixmap.x11PictureHandle(), XNone, d->picture,
                                     xOff, yOff, 0, 0, xPos, yPos, drawW, drawH);
                }
                xPos += drawW;
                xOff = 0;
            }
            yPos += drawH;
            yOff = 0;
        }
    } else
#endif // !QT_NO_XRENDER
        if (pixmap.depth() > 1 && !pixmap.data->x11_mask) {
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

void QX11PaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

    switch(ti.fontEngine->type()) {
    case QFontEngine::Box:
        drawBox(p, ti);
        break;
    case QFontEngine::XLFD:
        drawXLFD(p, ti);
        break;
#ifndef QT_NO_FONTCONFIG
    case QFontEngine::Freetype:
        drawFreetype(p, ti);
        break;
#endif
    default:
        Q_ASSERT(false);
    }
}


void QX11PaintEngine::drawBox(const QPointF &p, const QTextItemInt &ti)
{
    if (!ti.num_glyphs)
        return;

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QMatrix matrix;
    matrix.translate(p.x(), p.y());
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);
    if (glyphs.size() == 0)
        return;

    int size = qRound(ti.fontEngine->ascent());
    QSize s(size - 3, size - 3);

    painter()->save();
    painter()->setBrush(Qt::NoBrush);
    QPen pen = painter()->pen();
    pen.setWidthF(ti.fontEngine->lineThickness().toReal());
    painter()->setPen(pen);
    for (int k = 0; k < positions.size(); k++)
        painter()->drawRect(QRectF(positions[k].toPointF(), s));
    painter()->restore();
}

void QX11PaintEngine::drawXLFD(const QPointF &p, const QTextItemInt &ti)
{
    Q_D(QX11PaintEngine);

    if (d->txop > QPainterPrivate::TxTranslate) {
        // XServer or font don't support server side transformations, need to do it by hand
        QPaintEngine::drawTextItem(p, ti);
        return;
    }

    if (!ti.num_glyphs)
        return;

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QMatrix matrix = d->matrix;
    matrix.translate(p.x(), p.y());
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);
    if (glyphs.size() == 0)
        return;

    QFontEngineXLFD *xlfd = static_cast<QFontEngineXLFD *>(ti.fontEngine);
    Qt::HANDLE font_id = xlfd->fontStruct()->fid;

    XSetFont(d->dpy, d->gc, font_id);

    QVarLengthArray<XChar2b> chars(glyphs.size());

    for (int i = 0; i < glyphs.size(); i++) {
        int xp = qRound(positions[i].x);
        int yp = qRound(positions[i].y);
        if (xp < SHRT_MAX && xp > SHRT_MIN &&  yp > SHRT_MIN && yp < SHRT_MAX) {
            XChar2b ch;
            ch.byte1 = glyphs[i] >> 8;
            ch.byte2 = glyphs[i] & 0xff;
            XDrawString16(d->dpy, d->hd, d->gc, xp, yp, &ch, 1);
        }
    }
}

#ifndef QT_NO_FONTCONFIG
void QX11PaintEngine::core_render_glyph(QFontEngineFT *fe, int xp, int yp, uint g)
{
    Q_D(QX11PaintEngine);
    if (xp < SHRT_MIN || xp > SHRT_MAX  || yp < SHRT_MIN || yp > SHRT_MAX
        || d->cpen.style() == Qt::NoPen)
        return;

    QFontEngineFT::Glyph *glyph = fe->loadGlyph(g, QFontEngineFT::Format_Mono);
    // #### fix case where we don't get a glyph
    if (!glyph)
        return;

    Q_ASSERT(glyph->format == QFontEngineFT::Format_Mono);

    const int rectcount = 256;
    XRectangle rects[rectcount];
    int n = 0;
    int h = glyph->height;
    xp += glyph->x;
    yp += -glyph->y + glyph->height;
    int pitch = ((glyph->width + 31) & ~31) >> 3;

    uchar *src = glyph->data;
    while (h--) {
        for (int x = 0; x < glyph->width; ++x) {
            bool set = src[x >> 3] & (0x80 >> (x & 7));
            if (set) {
                XRectangle r = { xp + x, yp - h, 1, 1 };
                while (x < glyph->width-1 && src[(x+1) >> 3] & (0x80 >> ((x+1) & 7))) {
                    ++x;
                    ++r.width;
                }

                rects[n] = r;
                ++n;
            }
            if (n == rectcount) {
                d->setupAdaptedOrigin(QPoint(rects[0].x, rects[0].y));
                XFillRectangles(d->dpy, d->hd, d->gc, rects, n);
                n = 0;
                d->resetAdaptedOrigin();
            }
        }
        src += pitch;
    }
    if (n) {
        d->setupAdaptedOrigin(QPoint(rects[0].x, rects[0].y));
        XFillRectangles(d->dpy, d->hd, d->gc, rects, n);
        d->resetAdaptedOrigin();
    }
}

void QX11PaintEngine::drawFreetype(const QPointF &p, const QTextItemInt &ti)
{
    Q_D(QX11PaintEngine);
    if (!ti.num_glyphs)
        return;

    QFontEngineFT *ft = static_cast<QFontEngineFT *>(ti.fontEngine);

    if (!d->cpen.isSolid() || ft->drawAsOutline()) {
        QPaintEngine::drawTextItem(p, ti);
    }

    QFixed xpos = QFixed::fromReal(p.x() + d->matrix.dx());
    QFixed ypos = QFixed::fromReal(p.y() + d->matrix.dy());

    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QMatrix matrix = d->matrix;
    matrix.translate(p.x(), p.y());
    ft->getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);
    if (glyphs.count() == 0)
        return;

    bool drawTransformed = false;
#ifndef QT_NO_XRENDER
    const bool xrenderPath = (X11->use_xrender
                              && !(d->pdev->devType() == QInternal::Pixmap
                              && static_cast<const QPixmap *>(d->pdev)->data->type == QPixmap::BitmapType));

    GlyphSet transformedGlyphSet = 0;
    if (d->txop >= QPainterPrivate::TxScale
        && xrenderPath) {
        drawTransformed = ft->loadTransformedGlyphSet(glyphs.data(), glyphs.size(), d->matrix, &transformedGlyphSet);
    }
#endif

    if ((d->txop >= QPainterPrivate::TxScale && !drawTransformed)) {
        QPaintEngine::drawTextItem(p, ti);
        return;
    }

#ifndef QT_NO_XRENDER
    if (xrenderPath) {

        GlyphSet glyphSet = drawTransformed ? transformedGlyphSet : ft->fnt.glyphSet;

        const QColor &pen = d->cpen.color();
        ::Picture src = X11->getSolidFill(d->scrn, pen);
        XRenderPictFormat *maskFormat = XRenderFindStandardFormat(X11->display, ft->xglyph_format);

        enum { t_min = SHRT_MIN >> 1, t_max = SHRT_MAX >> 1};
        QFixed xp = positions[0].x;
        QFixed yp = positions[0].y;

        // better return instead of crashing the X server
        if (xp.toInt() < t_min || xp.toInt() > t_max
            || yp.toInt() < t_min || yp.toInt() > t_max)
            return;

        XGlyphElt32 elt;
        elt.glyphset = glyphSet;
        elt.chars = &glyphs[0];
        elt.nchars = 1;
        elt.xOff = qRound(xp);
        elt.yOff = qRound(yp);
        for (int i = 1; i < glyphs.size(); ++i) {
            QFontEngineFT::Glyph *g = ft->cachedGlyph(glyphs[i - 1]);
            if (g
                && positions[i].x == xp + g->advance
                && positions[i].y == yp) {
                elt.nchars++;
                xp += g->advance;
            } else {
                xp = positions[i].x;
                yp = positions[i].y;

                XRenderCompositeText32(X11->display, PictOpOver, src, d->picture,
                                       maskFormat, 0, 0, 0, 0,
                                       &elt, 1);
                elt.chars = &glyphs[i];
                elt.nchars = 1;
                elt.xOff = qRound(xp);
                elt.yOff = qRound(yp);
            }
        }
        XRenderCompositeText32(X11->display, PictOpOver, src, d->picture,
                               maskFormat, 0, 0, 0, 0,
                               &elt, 1);

        return;

    }
#endif
    ft->lockFace();
    int i = 0;
    while (i < glyphs.size()) {
        core_render_glyph(ft, qRound(positions[i].x), qRound(positions[i].y), glyphs[i]);
        ++i;
    }
    ft->unlockFace();
}
#endif // !QT_NO_XRENDER


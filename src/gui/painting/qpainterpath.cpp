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

#include "qpainterpath.h"
#include "qpainterpath_p.h"

#include <qbitmap.h>
#include <private/qobject_p.h>
#include <qlist.h>
#include <qpointarray.h>
#include <qmatrix.h>
#include <qvarlengtharray.h>
#include <qpen.h>

#include <qdebug.h>

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// This value is used to determine the length of control point vectors
// when approximating arc segments as curves. The factor is multiplied
// with the radius of the circle.
#define KAPPA 0.5522847498

// #define QPP_DEBUG

void qt_find_ellipse_coords(const QRectF &r, float angle, float length,
                            QPointF* startPoint, QPointF *endPoint)
{
#define ANGLE(t) ((t) * 2 * M_PI / 360.0)
    float a = r.width() / 2.0;
    float b = r.height() / 2.0;

    if (startPoint) {
        *startPoint = r.center()
                      + QPointF(a * cos(ANGLE(angle)), -b * sin(ANGLE(angle)));
    }
    if (endPoint) {
        *endPoint = r.center()
                    + QPointF(a * cos(ANGLE(angle + length)), -b * sin(ANGLE(angle + length)));
    }
}

void QPainterSubpath::close()
{
    Q_ASSERT(!elements.isEmpty());
    if (currentPoint != startPoint)
        lineTo(startPoint);
}

void QPainterSubpath::lineTo(const QPointF &p)
{
#ifdef QPP_DEBUG
    printf("   -> lineTo: (%.2f,%.2f), current=(%.2f, %.2f)\n",
           p.x(), p.y(), currentPoint.x(), currentPoint.y());
#endif

    elements.append(QPainterPathElement::line(p.x(), p.y()));
    currentPoint = p;

}

void QPainterSubpath::curveTo(const QPointF &c1, const QPointF &c2, const QPointF &end)
{
#ifdef QPP_DEBUG
    printf("   -> curveTo: end=(%.2f,%.2f), c1=(%.2f,%.2f), c2=(%.2f,%.2f)\n",
           end.x(), end.y(), c1.x(), c1.y(), c2.x(), c2.y());
#endif
    elements.append(QPainterPathElement::curve(c1.x(), c1.y(), c2.x(), c2.y(),
                                               end.x(), end.y()));
    currentPoint = end;
}

void QPainterSubpath::arcTo(const QRectF &rect, float startAngle, float sweepLength)
{
#ifdef QPP_DEBUG
    printf("   -> arcTo: rect=(%.1f,%.1f,%.1f,%.1f), angle=%.1f, len=%.1f\n",
           rect.x(), rect.y(), rect.width(), rect.height(),
           startAngle, sweepLength);
#endif

#define ANGLE(t) ((t) * 2 * M_PI / 360.0)
    float a = rect.width() / 2.0;
    float b = rect.height() / 2.0;

    int iterations = int((sweepLength + 89) / 90);
    float clength = sweepLength / iterations;
    float cosangle1, sinangle1, cosangle2, sinangle2;
    for (int i=0; i<iterations; ++i) {
        float cangle = startAngle + i * sweepLength / iterations;

        cosangle1 = cos(ANGLE(cangle));
        sinangle1 = sin(ANGLE(cangle));
        cosangle2 = cos(ANGLE(cangle + clength));
        sinangle2 = sin(ANGLE(cangle + clength));

        // Find the start and end point of the curve.
        QPointF startPoint = rect.center() + QPointF(a * cosangle1, -b * sinangle1);
        QPointF endPoint = rect.center() + QPointF(a * cosangle2, -b * sinangle2);

        // The derived at the start and end point.
        float sdx = -a * sinangle1;
        float sdy = -b * cosangle1;
        float edx = -a * sinangle2;
        float edy = -b * cosangle2;

        // Creating the tangent lines.
        QLineF controlLine1(startPoint, startPoint + QPointF(sdx, sdy));
        QLineF controlLine2(endPoint, endPoint - QPointF(edx, edy));

        // Adjust their length to fit the magic KAPPA length.
        controlLine1.setLength(controlLine1.length() * KAPPA);
        controlLine2.setLength(controlLine2.length() * KAPPA);

        if (startPoint != currentPoint)
            lineTo(startPoint);
        curveTo(controlLine1.end(), controlLine2.end(), endPoint);
    }
}


QPointArray QPainterSubpath::toPolygon(const QMatrix &matrix) const
{
    if (elements.isEmpty())
        return QPointArray();
    QPointArray p;
    fflush(stdout);
    p << (startPoint * matrix).toPoint();
    for (int i=0; i<elements.size(); ++i) {
        const QPainterPathElement &elm = elements.at(i);
        switch (elm.type) {
        case QPainterPathElement::Line:
            p << (QPointF(elm.lineData.x, elm.lineData.y) * matrix).toPoint();
            break;
        case QPainterPathElement::Curve: {
            QPointArray pa;
            pa << (p.isEmpty() ? (startPoint * matrix).toPoint() : p.last());
            pa << (QPointF(elm.curveData.c1x, elm.curveData.c1y) * matrix).toPoint();
            pa << (QPointF(elm.curveData.c2x, elm.curveData.c2y) * matrix).toPoint();
            pa << (QPointF(elm.curveData.ex, elm.curveData.ey) * matrix).toPoint();
            p += pa.cubicBezier();
            break;
        }
        default:
            qFatal("QPainterSubpath::toPolygon() unhandled case...: %d", elements.at(i).type);
        }
    }
    return p;
}


/*!
  \internal

  Converts all the curves in the path to linear polylines.
*/
QList<QPointArray> QPainterPathPrivate::flatten(const QMatrix &matrix, FlattenInclusion incl)
{
    QList<QPointArray> flatCurves;
    if (!flatCurves.isEmpty() || subpaths.isEmpty())
        return flatCurves;

    bool includeUnclosed = incl & UnclosedSubpaths;
    for (int i=0; i<subpaths.size(); ++i)
        if (includeUnclosed || subpaths.at(i).isClosed())
            flatCurves.append(subpaths.at(i).toPolygon(matrix));

    return flatCurves;
}

/*!
    \internal

    Converts the flattened path to a polygon that can be used for filling
*/

QPointArray QPainterPathPrivate::toFillPolygon(const QMatrix &matrix)
{
    QPointArray fillPoly;
    QList<QPointArray> polygons = flatten(matrix, AllSubpaths);
    for (int i=0; i<polygons.size(); ++i) {
        if (polygons.at(i).isEmpty())
            continue;
        fillPoly += polygons.at(i);
        if (polygons.at(i).first() != polygons.at(i).last())
            fillPoly += polygons.at(i).first();
        if (!polygons.at(0).isEmpty())
            fillPoly += polygons.at(0).at(0);
    }
    return fillPoly;
}

#define MAX_INTERSECTIONS 256

/*!
  \internal

  Convenience function used by scanToBitmap to set bits in a scanline...
*/

static inline void qt_painterpath_setbits(int from, int to, uchar *scanLine, uint fgPixel)
{
    if (to < from)
        return;

    int entryByte = from / 8;
    int exitByte = to / 8;

    // special case for ranges less than a byte.
    if (exitByte == entryByte) {
        uint entryPart = ((0xff << (from%8))&0xff);
        uint exitPart = (0xff >> (8-(to%8)));
        *(scanLine + entryByte) |= entryPart & exitPart & 0xff;
    } else {
        // First byte in this scan segment...
        *(scanLine + entryByte) |= ((0xff << (from%8))&0xff);

        // Fill areas between entry and exit bytes.
        if (exitByte > entryByte + 1)
            memset(scanLine+entryByte+1,  fgPixel, exitByte-entryByte-1);

        // Last byte in this scan segment...
        *(scanLine + exitByte) |= (0xff >> (8-(to%8)));
    }

}

#if 0
/*!
  \internal

  Scans the path to a bitmap that can be used to define filling. The insides
  of the bitmap will be filled with foreground color and the outsides
  will be filled with background color.

  The cliprectangle \a clip is used to clip the scan area down to the part that
  is currently visible. The clip is specified in painter coordinates. The
  matrix \a xform defines the world matrix.

  The algorithm for this works by first flattening the path to linear
  segments stored in point arrays (flatCurves). We then intersect the
  bounding rect of all flat curves with the supplied clip rect to determine
  the area to scan convert.

  For each scan line we check for intersection with the lines (note to
  self, we could probably reduce the number of line intersection
  checks by sorting the lines top->bottom and only checking the ones
  we know might intersect). We register the xcoord of each
  intersection in the isects array. At the end we sort the
  intersections from left to right, and fill in based on current
  fill rule..
*/
QBitmap QPainterPathPrivate::scanToBitmap(const QRect &clipRect,
                                          const QMatrix &xform,
                                          QRect *boundingRect)
{
    QList<QPointArray> flatCurves = flatten(xform, ClosedSubpaths);

    QRect pathBounds;
    for (int fc=0; fc<flatCurves.size(); ++fc)
        pathBounds |= flatCurves.at(fc).boundingRect();

    QRegion scanRegion(pathBounds);
    if (clipRect.isValid())
        scanRegion &= xform.mapToRegion(clipRect);
    QRect scanRect = scanRegion.boundingRect();
    if (boundingRect)
        *boundingRect = scanRect;
    if (!scanRect.isValid())
        return QBitmap();

    const uint bgPixel = QColor(Qt::color1).rgb();
    const uint fgPixel = QColor(Qt::color0).rgb();

    QImage image(scanRect.width(), scanRect.height(), 1, 2, QImage::LittleEndian);
    image.fill(bgPixel);
    int isects[MAX_INTERSECTIONS];
    QVarLengthArray<int, 1024> windingNumbers(scanRect.width());
    int numISects;
    for (int y=0; y<scanRect.height(); ++y) {
        int scanLineY = y + scanRect.y();
        numISects = 0;
        for (int c=0; c<flatCurves.size(); ++c) {
            QPointArray curve = flatCurves.at(c);
            if (!scanRect.intersects(curve.boundingRect()))
                continue;
            Q_ASSERT(curve.size()>=2);
            QPointF p1 = curve.at(curve.size()-1);
            for (int i=0; i<curve.size(); ++i) {
                QPointF p2 = curve.at(i);

                // Does the line cross the scan line?
                if ((p1.y() <= scanLineY && p2.y() > scanLineY)
                    || (p1.y() > scanLineY && p2.y() <= scanLineY)) {
                    Q_ASSERT(numISects<MAX_INTERSECTIONS);

                    // Find intersection and add to set of intersetions for this scanline
                    // Horizontal lines are skipped since their end points are covered
                    // by other lines, and adding them would inverse the results
                    if (p1.y() != p2.y()) {
                        double idelta = (p2.x()-p1.x()) / double(p2.y()-p1.y());
                        isects[numISects] =
                            qRound((scanLineY - p1.y()) * idelta + p1.x()) - scanRect.x();
                        windingNumbers[isects[numISects]] = ((p2.y() > p1.y()) ? 1 : -1);
                        ++numISects;
                    }
                }
                p1 = p2;
            }
        }

        if (numISects <= 0)
            continue;

        // There is always an even number of intersections in closed curves.
        Q_ASSERT(numISects%2 == 0);

        // Sort the intersection entries...
        qHeapSort(&isects[0], &isects[numISects]);

        uchar *scanLine = image.scanLine(y);

        if (fillMode == QPainterPath::OddEven) {
            for (int i=0; i<numISects; i+=2) {
                int from = qMax(0, isects[i]);
                int to = qMin(scanRect.width(), isects[i+1]);
                qt_painterpath_setbits(from, to, scanLine, fgPixel);
            }
        } else { // Winding fill rule
            for (int i=0; i<numISects; ) {
                int windingNumber = 0;
                int from = qMax(0, isects[i]);
                int to = 0;
                windingNumber += windingNumbers[isects[i]];
                for (++i; i<numISects && windingNumber != 0; ++i) {
                    windingNumber += windingNumbers[isects[i]];
                    to = qMin(scanRect.width(), isects[i]);
                }
                qt_painterpath_setbits(from, to, scanLine, fgPixel);
            }
        }
    }
    QBitmap bm;
    bm.convertFromImage(image);
    return bm;
}
#endif

#ifdef QPP_DEBUG
static void qt_path_debug_subpath(const QPainterSubpath &sp)
{
    printf("SUBPATH: start=(%.2f,%.2f), current=(%.2f,%.2f)\n",
           sp.startPoint.x(), sp.startPoint.y(),
           sp.currentPoint.x(), sp.currentPoint.y());

    for (int i=0; i<sp.elements.size(); ++i) {
        const QPainterPathElement &elm = sp.elements.at(i);
        switch (elm.type) {
        case QPainterPathElement::Line:
            printf(" ---> LINE: (%.2f, %.2f)\n", elm.lineData.x, elm.lineData.y);
            break;
        }
    }
}
#endif

static QLineF::IntersectType qt_path_stroke_join(QPainterSubpath *sp,
                                QPainterPathElement *prev,
                                const QLineF &ml,
                                Qt::PenJoinStyle joinStyle)
{
    // Check for overlap
    QLineF pline(sp->lastCurrent(), QPointF(prev->lineData.x, prev->lineData.y));
    QPointF isect;
    QLineF::IntersectType type = pline.intersect(ml, &isect);
    if (type == QLineF::BoundedIntersection) {
        prev->lineData.x = isect.x();
        prev->lineData.y = isect.y();
    } else {
        switch (joinStyle) {
        case Qt::MiterJoin:
            sp->lineTo(ml.start());
            break;
        case Qt::BevelJoin:
            prev->lineData.x = isect.x();
            prev->lineData.y = isect.y();
            break;
        case Qt::RoundJoin:
            sp->curveTo(isect, isect, ml.start());
            break;
        default:
            break;
        }
    }
    return type;
}

static void qt_path_stroke_line(const QPainterPathElement &elm,
                                QPainterSubpath *sp,
                                const QPointF &lastPoint,
                                float penWidth,
                                Qt::PenJoinStyle joinStyle)
{
#ifdef QPP_DEBUG
    printf(" -> stroking line from (%.2f, %.2f) -> (%.2f, %.2f)\n",
           lastPoint.x(), lastPoint.y(), elm.lineData.x, elm.lineData.y);
#endif

    QLineF line(lastPoint, QPointF(elm.lineData.x, elm.lineData.y));
    QLineF normal = line.normalVector();
    normal.setLength(penWidth);
    QLineF ml(line);
    ml.moveBy(normal);

    if (!sp->elements.isEmpty()) {
        QPainterPathElement &prev = sp->elements.last();
        if (prev.type == QPainterPathElement::Line) {
            if (sp->currentPoint != ml.start()) {
                qt_path_stroke_join(sp, &prev, ml, joinStyle);
            }
        } else {
            printf("%d : %s - curve not supported\n", __LINE__, __FILE__);
        }
    } else {
        sp->startPoint = ml.start();
    }
    sp->lineTo(ml.end());
}

static QPointF qt_path_stroke_to_element(const QPainterPathElement &elm,
                                         QPainterSubpath *subpath,
                                         const QPointF &lastPoint,
                                         float penWidth,
                                         Qt::PenJoinStyle joinStyle)
{
    switch (elm.type) {
    case QPainterPathElement::Line: {
        qt_path_stroke_line(elm, subpath, lastPoint, penWidth, joinStyle);
        return QPointF(elm.lineData.x, elm.lineData.y);
    }
    case QPainterPathElement::Curve:
        // Nothing yet..
        break;
    default:
        // Nothing...
        break;
    }
    return QPointF();
}



/*!
    \internal
*/

QPainterPath QPainterPathPrivate::createStroke(const QPen &pen)
{
#ifdef QPP_DEBUG
    printf("QPainterPathPrivate::createStrok()\n");
#endif

    QPainterPath stroke;

    Qt::PenJoinStyle joinStyle = pen.joinStyle();
    float penWidth = pen.width() / 2.0;

    for (int spi=0; spi<subpaths.size(); ++spi) {
        const QPainterSubpath &subpath = subpaths.at(spi);

        if (subpath.elements.isEmpty())
            continue;

        QPainterSubpath usegs; // "up" segments, positive offset
        QPainterSubpath dsegs; // "down" segments, negative offset

        QPointF uLastPt = subpath.startPoint;
        QPointF dLastPt = subpath.elements.last().end();

        int nelms = subpath.elements.size()-1;
        for (int elmi=0; elmi<subpath.elements.size(); ++elmi) {
            uLastPt = qt_path_stroke_to_element(subpath.elements.at(elmi), &usegs, uLastPt,
                                                penWidth, joinStyle);
            if (nelms<1) {
                QPainterPathElement fakeLineToStart =
                    QPainterPathElement::line(subpath.startPoint.x(), subpath.startPoint.y());
                dLastPt = qt_path_stroke_to_element(fakeLineToStart, &dsegs, dLastPt,
                                                    penWidth, joinStyle);
            } else {
                dLastPt = qt_path_stroke_to_element(subpath.elements.at(--nelms), &dsegs, dLastPt,
                                                    penWidth, joinStyle);
            }
        }

        if (!subpath.isClosed()) {
            usegs.lineTo(dsegs.startPoint);
            dsegs.lineTo(usegs.startPoint);
            usegs.elements += dsegs.elements;
            stroke.d_func()->subpaths.append(usegs);
#ifdef QPP_DEBUG
        qt_path_debug_subpath(usegs);
#endif
        } else {
            if (!usegs.isClosed()) {
                QLineF::IntersectType type = qt_path_stroke_join(&usegs, &usegs.elements.last(),
                                                                 QLineF(usegs.startPoint,
                                                                        usegs.elements.first().end()),
                                                                 joinStyle);
                if (type == QLineF::BoundedIntersection)
                    usegs.startPoint = usegs.elements.last().end();
                if (!usegs.isClosed())
                    usegs.close();
            }

            if (!dsegs.isClosed()) {
                QLineF::IntersectType type = qt_path_stroke_join(&dsegs, &dsegs.elements.last(),
                                                                 QLineF(dsegs.startPoint,
                                                                        dsegs.elements.first().end()),
                                                                 joinStyle);
                if (type == QLineF::BoundedIntersection)
                    dsegs.startPoint = dsegs.elements.last().end();
                if (!dsegs.isClosed())
                    dsegs.close();
            }

#ifdef QPP_DEBUG
        qt_path_debug_subpath(usegs);
        qt_path_debug_subpath(dsegs);
#endif
            stroke.d_func()->subpaths.append(usegs);
            stroke.d_func()->subpaths.append(dsegs);
        }
    }
    stroke.setFillMode(QPainterPath::Winding);
    return stroke;
}

#define d d_func()
#define q q_func()

/*!
    \class QPainterPath
    \brief The QPainterPath class specifies a graphical shape.

    A painter path is an object composed of a number of graphical
    building blocks, such as rectangles, ellipses, lines, and curves.
    A painter path can be used for filling and outlining, and also for
    clipping. The main advantage of painter paths over normal drawing
    operations is that it is possible to build up non-linear shapes
    which can be drawn later one go.

    Building blocks can be joined in closed sub-paths, such as a
    rectangle or an ellipse, or they can exist independently as unclosed
    sub-paths, although an unclosed path will not be filled.

    Below is a code snippet that shows how a path can be used. The
    painter in this case has a pen width of 3 and a light blue brush.
    We first add a rectangle, which becomes a closed sub-path.  We
    then add two bezier curves, and finally draw the entire path.

    \code
    QPainterPath path;
    path.addRect(20, 20, 80, 80);

    path.moveTo(0, 0);
    path.curveTo(99, 0,  50, 50,  99, 99);
    path.curveTo(0, 99,  50, 50,  0, 0);

    painter.drawPath(path);
    \endcode
*/

/*!
    \enum QPainterPath::FillMode

    Specifies which method should be used to fill the path.

    \value OddEven Specifies that the region is filled using the odd
    even fill rule. With this rule, one determines wheter a point is
    inside the path as follows: Draw a horizontal line from the point
    to outside the path and count the number of intersections. If the
    number of intersections is an odd number the point is inside the
    path. This mode is the default.

    \value Winding Specifies that the region is filled using the non
    zero winding rule. With this rule, one determines wheter a point
    is inside the path as follows: Draw a horizontal line from the
    path to the outside of the path. Determine the direction of the
    path in each intersection point, up or down. The winding number is
    determined by summing the direction of each intersection. If the
    number is non zero, the point is inside the path. This fill mode
    can also in most cases be considered as the intersection of closed
    shapes.
*/

/*!
 * Creates a new empty QPainterPath.
 */
QPainterPath::QPainterPath()
{
    d_ptr = new QPainterPathPrivate;
    d->subpaths.append(QPainterSubpath());
}

/*!
    Creates a new painter path that is a copy of the \a other painter
    path.
*/
QPainterPath::QPainterPath(const QPainterPath &other)
    : d_ptr(new QPainterPathPrivate(*other.d_ptr))
{
}

/*!
    Assigns the \a other painter path to this painter path.
*/
QPainterPath &QPainterPath::operator=(const QPainterPath &other)
{
    *d_ptr = *other.d_ptr;
    return *this;
}

/*!
    Destructs the painter path.
*/
QPainterPath::~QPainterPath()
{
    delete d;
}

/*!
    Closes the current subpath. If the subpath does not contain any
    elements, the function does nothing. A new subpath is
    automatically begun when the current subpath is closed. The
    current point of the new path is in 0, 0.

    \sa beginSubpath()
 */
void QPainterPath::closeSubpath()
{
    if (d->subpaths.last().elements.isEmpty())
        return;
    d->subpaths.last().close();
    d->subpaths.append(QPainterSubpath());
}

/*!
    \fn void QPainterPath::moveTo(float x, float y)

    \overload

    Moves the current point to (\a{x}, \a{y}).
*/

/*!
    Moves the current point to \a p. Moving the current point
    will also start a new subpath. The previously current path
    will not be closed implicitly before the new one is started.
*/
void QPainterPath::moveTo(const QPointF &p)
{
#ifdef QPP_DEBUG
    printf("QPainterPath::moveTo() (%.2f,%.2f)\n", p.x(), p.y());
#endif
    if (d->subpaths.last().elements.isEmpty()) {
#ifdef QPP_DEBUG
        printf(" -> modify current\n", p.x(), p.y());
#endif
        d->subpaths.last().startPoint = p;
        d->subpaths.last().currentPoint = p;
        return;
    }
    d->subpaths.append(QPainterSubpath(p));
}

/*!
    \fn void QPainterPath::lineTo(float x, float y)

    \overload

    Draws a line to point (\a{x}, \a{y}).
*/

/*!
    Adds a straight line from the current point to the point \a p. The current
    point is moved to \a p when this function returns.
 */
void QPainterPath::lineTo(const QPointF &p)
{
    d->subpaths.last().lineTo(p);
}

/*!
    \fn void QPainterPath::curveTo(float ctrlPt1x, float ctrlPt1y, float ctrlPt2x,
                                   float ctrlPt2y, float endPtx, float endPty);

    \overload

    Adds a Bezier curve with control points (\a{ctrlPt1x},
    \a{ctrlPt1y}), (\a{ctrlPt2x}, \a{ctrlPt2y}), and end points,
    (\a{endPtx}, \a{endPty}).
*/

/*!
    Adds a Bezier curve starting in the current point with control points \a c1, and \a c2,
    ending in \a e. The current point is moved to \a e when this function returns.

*/
void QPainterPath::curveTo(const QPointF &c1, const QPointF &c2, const QPointF &e)
{
    d->subpaths.last().curveTo(c1, c2, e);
}

/*!
    \fn void QPainterPath::arcTo(float x, float y, float width, float
    height, float startAngle, float sweepLength)

    \overload

    The arc's rectangle is at point (\a{x}, \a{y}) and has the given
    \a width and \a height.
*/

/*!
    Creates an  arc which occupies the notional rectangle \a rect,
    starting at the given \a startAngle, and extending for \a
    sweepLength degrees anti-clockwise. Angles are specified
    in degrees. This function connects the current point to
    the starting point of the arc if they are not already connected.
*/
void QPainterPath::arcTo(const QRectF &rect, float startAngle, float sweepLength)
{
    d->subpaths.last().arcTo(rect, startAngle, sweepLength);
}

/*!
    \fn void QPainterPath::addRect(float x, float y, float width, float height)

    \overload

    Adds a rectangle at position (\a{x}, \a{y}), with the given \a
    width and \a height.

*/

/*!
    Adds rectangle, \a r, as a closed subpath to this path. The
    rectangle is added as a clockwise set of lines. An empty subpath
    with current position at (0, 0) is in use after this function
    returns.
*/
void QPainterPath::addRect(const QRectF &r)
{
    moveTo(r.topLeft());
    QPainterSubpath &sp = d->subpaths.last();
    sp.lineTo(r.topRight());
    sp.lineTo(r.bottomRight());
    sp.lineTo(r.bottomLeft());
    closeSubpath();
}

#if 0
void QPainterPath::transform(const QMatrix &matrix)
{
    for (int i=0; i<d->subpaths.size(); ++i) {
        QPainterSubpath &sp = d->subpaths[i];
        sp.startPoint = sp.startPoint * matrix;
        for (int j=0; j<sp.elements.size(); j++) {
            QPainterPathElement &elm = sp.elements[j];
            switch (elm.type) {
            case QPainterPathElement::Line: {
                QPointF np = QPointF(elm.lineData.x, elm.lineData.y) * matrix;
                elm.lineData.x = np.x();
                elm.lineData.y = np.y();
            }
            case QPainterPathElement::Curve: {
                QPointF p = QPointF(elm.curveData.c1x. elm.curveData.c1y) * matrix;
                elm.curveData.c1x = p.x();
                elm.curveData.c1y = p.y();

                p = QPointF(elm.curveData.c2x. elm.curveData.c2y) * matrix;
                elm.curveData.c2x = p.x();
                elm.curveData.c2y = p.y();

                p = QPointF(elm.curveData.ex. elm.curveData.ey) * matrix;
                elm.curveData.ex = p.x();
                elm.curveData.ey = p.y();
            }

        }
    }
}
#endif

/*!
    Returns the fill mode of the painter path. The default fill mode
    is OddEven.

    \sa FillMode, setFillMode()
*/
QPainterPath::FillMode QPainterPath::fillMode() const
{
    return d->fillMode;
}

/*!
    Sets the fill mode of the painterpath to \a fillMode.

    \sa FillMode, fillMode
*/
void QPainterPath::setFillMode(QPainterPath::FillMode fillMode)
{
    d->fillMode = fillMode;
}

/*!
    Returns the bounding rectangle of this painter path
*/
QRectF QPainterPath::boundingRect() const
{
    if (d->subpaths.isEmpty())
        return QRectF();
    QRectF rect;
    for (int j=0; j<d->subpaths.size(); ++j) {
        QPointArray pa = d->subpaths.at(j).toPolygon(QMatrix());
        rect |= pa.boundingRect();
    }
    return rect;
}

/*!
    Returns true if there are no elements in this path
*/
bool QPainterPath::isEmpty() const
{
    return d->subpaths.isEmpty();
}

/*!
    Create an outline for the path with the given \a width.
*/
QPainterPath QPainterPath::createPathOutline(int width)
{
    return d->createStroke(QPen(Qt::black, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

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

#include <qdebug.h>

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**********************************************************************
 * class: QPainterPathElement
 */
QPointFloat QPainterPathElement::firstPoint() const
{
    switch (type) {
    case Line:
        return QPointFloat(lineData.x1, lineData.y1);
    case Bezier:
        return QPointFloat(bezierData.x1, bezierData.y1);
    case Arc:
        return QPointFloat(arcData.fpx, arcData.fpy);
    }
    Q_ASSERT(0);
    return QPointFloat();
}

/**********************************************************************
 * class: QPainterSubpath
 */
void QPainterSubpath::connectLast(const QPointFloat &p)
{
    if (elements.size() > 0 && p != lastPoint) {
        addLine(QLineFloat(lastPoint, p));
    }
}


void QPainterSubpath::close()
{
    Q_ASSERT(!elements.isEmpty());
    QPainterPathElement firstElement = elements.at(0);
    connectLast(firstElement.firstPoint());
}

void QPainterSubpath::addLine(const QLineFloat &l)
{
    connectLast(l.start());
    lastPoint = l.end();

    QPainterPathElement elm;
    elm.type = QPainterPathElement::Line;
    elm.lineData.x1 = l.startX();
    elm.lineData.y1 = l.startY();
    elm.lineData.x2 = l.endX();
    elm.lineData.y2 = l.endY();
    elements.append(elm);

}

void QPainterSubpath::addBezier(const QPointFloat &p1, const QPointFloat &p2,
                                const QPointFloat &p3, const QPointFloat &p4)
{
    connectLast(p1);
    lastPoint = p4;

    QPainterPathElement elm;
    elm.type = QPainterPathElement::Bezier;
    elm.bezierData.x1 = p1.x();
    elm.bezierData.y1 = p1.y();
    elm.bezierData.x2 = p2.x();
    elm.bezierData.y2 = p2.y();
    elm.bezierData.x3 = p3.x();
    elm.bezierData.y3 = p3.y();
    elm.bezierData.x4 = p4.x();
    elm.bezierData.y4 = p4.y();
    elements.append(elm);
}

void QPainterSubpath::addArc(const QRectFloat &rect, float angle, float alen)
{
    float a = rect.width() / 2.0;
    float b = rect.height() / 2.0;

    QPointFloat startPoint(a * cos(angle), -b * sin(angle));
    QPointFloat endPoint(a * cos(angle + alen), -b * sin(angle + alen));

    startPoint += rect.center();
    endPoint   += rect.center();

    connectLast(startPoint);
    lastPoint = endPoint;

    QPainterPathElement elm;
    elm.type           = QPainterPathElement::Arc;
    elm.arcData.x      = rect.x();
    elm.arcData.y      = rect.y();
    elm.arcData.w      = rect.width();
    elm.arcData.h      = rect.height();
    elm.arcData.start  = angle;
    elm.arcData.length = alen;
    elm.arcData.fpx    = startPoint.x();
    elm.arcData.fpy    = startPoint.y();
    elm.arcData.lpx    = endPoint.x();
    elm.arcData.lpy    = endPoint.y();
    elements.append(elm);
}


QPointArray QPainterSubpath::toPolygon(const QMatrix &matrix) const
{
    if (elements.isEmpty())
        return QPointArray();
    QPointArray p;
    fflush(stdout);
    p << (matrix * elements.at(0).firstPoint()).toPoint();
    for (int i=0; i<elements.size(); ++i) {
        const QPainterPathElement &elm = elements.at(i);
        switch (elm.type) {
        case QPainterPathElement::Line:
            p << (matrix * QPointFloat(elm.lineData.x2, elm.lineData.y2)).toPoint();
            break;
        case QPainterPathElement::Bezier: {
            QPointArray pa;
            pa << (matrix * QPointFloat(elm.bezierData.x1, elm.bezierData.y1)).toPoint();
            pa << (matrix * QPointFloat(elm.bezierData.x2, elm.bezierData.y2)).toPoint();
            pa << (matrix * QPointFloat(elm.bezierData.x3, elm.bezierData.y3)).toPoint();
            pa << (matrix * QPointFloat(elm.bezierData.x4, elm.bezierData.y4)).toPoint();
            p += pa.cubicBezier();
            break;
        }
        case QPainterPathElement::Arc: {
            QPointArray ar;
            ar.makeArc(int(elm.arcData.x), int(elm.arcData.y),
                       int(elm.arcData.w), int(elm.arcData.h),
                       int(elm.arcData.start / M_PI * 360),
                       int(elm.arcData.length / M_PI * 360),
                       matrix);
            p += ar;
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
QList<QPointArray> QPainterPathPrivate::flatten(const QMatrix &matrix)
{
    QList<QPointArray> flatCurves;
    if (!flatCurves.isEmpty() || subpaths.isEmpty())
        return flatCurves;

    for (int i=0; i<subpaths.size(); ++i)
        if (subpaths.at(i).isClosed())
            flatCurves.append(subpaths.at(i).toPolygon(matrix));

    return flatCurves;
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
    QList<QPointArray> flatCurves = flatten(xform);

    QRect pathBounds;
    for (int fc=0; fc<flatCurves.size(); ++fc)
        pathBounds |= flatCurves.at(fc).boundingRect();

    QRegion scanRegion(pathBounds);
    if (clipRect.isValid())
        scanRegion &= xform * clipRect;
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
            QPointFloat p1 = curve.at(curve.size()-1);
            for (int i=0; i<curve.size(); ++i) {
                QPointFloat p2 = curve.at(i);

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

/*!
    \internal
*/

QPainterPath QPainterPathPrivate::createPathOutline(int penWidth)
{
    float pw2 = penWidth/2.0;

    QPainterPath outline;
    outline.setFillMode(QPainterPath::Winding);


    for (int sp=0; subpaths.size(); ++sp) {
        const QPainterSubpath &subpath = subpaths.at(sp);
        QPainterSubpath side;
        for (int elmno=0; subpath.elements.size(); ++elmno) {
            const QPainterPathElement &elm = subpath.elements.at(elmno);
            switch (elm.type) {

            case QPainterPathElement::Line:
                {
                    QLineFloat l(elm.lineData.x1, elm.lineData.y1, elm.lineData.x2, elm.lineData.y2);
                    QLineFloat nv = l.normalVector();
                    nv.setLength(pw2);
                    l.moveBy(nv);
                    side.addLine(l);
                }
                break;

            case QPainterPathElement::Arc:
                printf("QPainterPathElement::createPathOutline() - elm is arc\n");
                break;
            case QPainterPathElement::Bezier:
                printf("QPainterPathElement::createPathOutline() - elm is bezier\n");
                break;

            default:
                Q_ASSERT(0);
                break;
            }
        }
        outline.d_func()->subpaths.append(side);
    }

    return outline;
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
    path.addRect(20, 20, 60, 60);
    path.addBezier(0, 0,  99, 0,  50, 50,  99, 99);
    path.addBezier(99, 99,  0, 99,  50, 50,  0, 0);
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
    Begins a new subpath in the existing path. If the path has no
    elements this function does nothing. If a subpath is already in
    progress this function does not automatically close it.

    \sa closeSubpath()
 */
void QPainterPath::beginSubpath()
{
    if (d->subpaths.last().elements.isEmpty())
        return;
    d->subpaths.append(QPainterSubpath());
}


/*!
    Closes the current subpath. If the subpath does not contain
    any elements, the function does nothing. A new subpath is
    automatically begun when the current subpath is closed.

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
    \fn void QPainterPath::addLine(int x1, int y1, int x2, int y2)

    \overload

    Adds a line from point (\a{x1}, \a{y1}) to point (\a{x2}, \a{y2}).
*/

/*!
    \fn void QPainterPath::addLine(int x, int y)

    \overload

    Adds a line from the last point drawn to the point (\a{x}, \a{y}).
*/

/*!
    Adds a straight line defined by the start point \a p1 and the end
    point \a p2 to the path.
 */
void QPainterPath::addLine(const QLineFloat &line)
{
    d->subpaths.last().addLine(line);
}

/*!
    \overload

    Adds a straight line from the last point to point \a p.
 */
void QPainterPath::addLine(const QPointFloat &p)
{
    d->subpaths.last().addLine(QLineFloat(d->subpaths.last().lastPoint, p));
}

/*!
    \fn void QPainterPath::addRect(int x, int y, int width, int height)

    \overload

    Adds a rectangle at point (\a{x}, \a{y}) with the given \a width
    and \a height.
*/

/*!
    \fn void QPainterPath::addRect(const QPointFloat &topLeft, const QPointFloat &bottomRight)

    \overload

    Adds a rectangle at point \a topLeft that extends to the point \a
    bottomRight.
*/

/*!
    \fn void QPainterPath::addRect(const QPointFloat &topLeft, const QSize &size)

    \overload

    Adds a rectangle at point \a topLeft of the given \a size.
*/


/*!
    Adds the given \a rect to the path. The \a rect is closed and is
    not considered to be part of the current subpath.
 */
void QPainterPath::addRect(const QRectFloat &rect)
{
    QPainterSubpath subpath;
    subpath.addLine(QLineFloat(rect.topLeft(), rect.topRight()));
    subpath.addLine(QLineFloat(rect.bottomRight(), rect.bottomLeft()));
    subpath.close();
    d->subpaths.prepend(subpath);
}


/*!
    \fn void QPainterPath::addBezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)

    \overload

    Adds a Bezier curve with control points (\a{x1}, \a{y1}), (\a{x2},
    \a{y2}), (\a{x3}, \a{y3}), and (\a{x4}, \a{y4}).
*/

/*!
    Adds a Bezier curve with control points \a p1, \a p2, \a p3, and
    \a p4, to the path.
*/
void QPainterPath::addBezier(const QPointFloat &p1, const QPointFloat &p2,
                             const QPointFloat &p3, const QPointFloat &p4)
{
    d->subpaths.last().addBezier(p1, p2, p3, p4);
}

/*!
    \fn void QPainterPath::addArc(int x, int y, int width, int height,
    int startAngle, int sweepLength)

    \overload

    Adds an arc which occupies the notional rectangle specified by the
    given position (\a{x}, \a{y}), \a width and \a height.
*/

/*!
    Adds an arc which occupies the notional rectangle \a rect,
    starting at the given \a startAngle, and extending for \a
    sweepLength degrees anti-clockwise.
*/
void QPainterPath::addArc(const QRectFloat &rect, float startAngle, float sweepLength)
{
    d->subpaths.last().addArc(rect, startAngle, sweepLength);
}

/*!
    Returns the fill mode of the painter path. The default fill mode
    is OddEven.

    \sa FillMode, setFillMode
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
QRectFloat QPainterPath::boundingRect() const
{
    if (d->subpaths.isEmpty())
        return QRectFloat();
    QRectFloat rect;
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

QPainterPath QPainterPath::createPathOutline(int width)
{
    return d->createPathOutline(width);
}

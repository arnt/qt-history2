#include "qpainterpath.h"
#include "qpainterpath_p.h"

#include <qbitmap.h>
#include <private/qobject_p.h>
#include <qlist.h>
#include <qpointarray.h>

#include <qdebug.h>

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**********************************************************************
 * class: QPainterPathElement
 */
QPoint QPainterPathElement::firstPoint() const
{
    switch (type) {
    case Line:
	return QPoint(lineData.x1, lineData.y1);
    case Bezier:
        return QPoint(bezierData.x1, bezierData.y1);
    case Arc:
        return QPoint(arcData.fpx, arcData.fpy);
    }
    qFatal("QPainterPathElement::firstPoint(), unhandled type: %d", type);
    return QPoint(0, 0);
}

/**********************************************************************
 * class: QPainterSubpath
 */
void QPainterSubpath::connectLast(const QPoint &p)
{
    if (elements.size() > 0 && p != lastPoint) {
	addLine(lastPoint, p);
    }
}


void QPainterSubpath::close()
{
    Q_ASSERT(!elements.isEmpty());
    QPainterPathElement firstElement = elements.at(0);
    connectLast(firstElement.firstPoint());
}

void QPainterSubpath::addLine(const QPoint &p1, const QPoint &p2)
{
    connectLast(p1);
    lastPoint = p2;

    QPainterPathElement elm;
    elm.type = QPainterPathElement::Line;
    elm.lineData.x1 = p1.x();
    elm.lineData.y1 = p1.y();
    elm.lineData.x2 = p2.x();
    elm.lineData.y2 = p2.y();
    elements.append(elm);

}

void QPainterSubpath::addBezier(const QPoint &p1, const QPoint &p2, const QPoint &p3, const QPoint &p4)
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

void QPainterSubpath::addArc(const QRect &rect, int startAngle, int arcLength)
{
#define RHAT sqrt( (rx*rx*ry*ry) / ( ry*ry*sintheta + ry*ry*costheta ) )
    const int THETAFACTOR = int(2 * M_PI / (16.0 * 360));

    double rx = rect.width() / 2.0;
    double ry = rect.height() / 2.0;
    double theta = startAngle * THETAFACTOR;
    double cx = rect.x() + rx;
    double cy = rect.y() + ry;
    double sintheta = sin(theta);
    double costheta = cos(theta);
    double rhat = RHAT;

    QPoint firstPoint(int(cx + rhat * costheta), int(cy + rhat * sintheta));
    theta = (startAngle + arcLength) * THETAFACTOR;
    sintheta = sin(theta);
    costheta = cos(theta);
    rhat =  RHAT;
    QPoint lastPoint(int(cx + rhat * costheta), int(cy + rhat * sintheta));

    connectLast(firstPoint);
    this->lastPoint = lastPoint;

    QPainterPathElement elm;
    elm.type           = QPainterPathElement::Arc;
    elm.arcData.x      = rect.x();
    elm.arcData.y      = rect.y();
    elm.arcData.w      = rect.width();
    elm.arcData.h      = rect.height();
    elm.arcData.start  = startAngle;
    elm.arcData.length = arcLength;
    elm.arcData.fpx    = firstPoint.x();
    elm.arcData.fpy    = firstPoint.y();
    elm.arcData.lpx    = lastPoint.x();
    elm.arcData.lpy    = lastPoint.y();
    elements.append(elm);
}


QPointArray QPainterSubpath::toPolygon() const
{
    if (elements.isEmpty())
	return QPointArray();
    QPointArray p;
    fflush(stdout);
    p << elements.at(0).firstPoint();
    for (int i=0; i<elements.size(); ++i) {
        const QPainterPathElement &elm = elements.at(i);
	switch (elm.type) {
	case QPainterPathElement::Line:
	    p << QPoint(elm.lineData.x2, elm.lineData.y2);
	    break;
        case QPainterPathElement::Bezier: {
	    QPointArray pa;
	    pa.setPoints(4, elm.bezierData.x1, elm.bezierData.y1,
			 elm.bezierData.x2, elm.bezierData.y2,
			 elm.bezierData.x3, elm.bezierData.y3,
			 elm.bezierData.x4, elm.bezierData.y4);
	    p += pa.cubicBezier();
	    break;
	}
        case QPainterPathElement::Arc: {
            QPointArray ar;
            ar.makeArc(elm.arcData.x, elm.arcData.y,
                       elm.arcData.w, elm.arcData.h,
                       elm.arcData.start, elm.arcData.length);
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
void QPainterPathPrivate::flatten()
{
    if (!flatCurves.isEmpty() || subpaths.isEmpty())
        return;

    for (int i=0; i<subpaths.size(); ++i)
        if (subpaths.at(i).isClosed())
            flatCurves.append(subpaths.at(i).toPolygon());
}

#define MAX_INTERSECTIONS 256

/*!
  \internal

  Scans the path to a bitmap that can be used to define filling. The insides
  of the bitmap will be filled with foreground color and the outsides
  will be filled with background color.

  The cliprectangle \a clip is used to clip the scan area down to the part that
  is currently visible. The clip is specified in painter coordinates. The
  matrix \a xform defines the world matrix. \pathPos

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
                                          const QWMatrix &/*xform*/,
                                          QRect *boundingRect)
{
    Q_ASSERT(!bits);

    flatten();

    QRect pathBounds;
    for (int fc=0; fc<flatCurves.size(); ++fc)
        pathBounds |= flatCurves.at(fc).boundingRect();

     QRect scanRect = pathBounds;
    if (clipRect.isValid())
        scanRect &= clipRect;
    *boundingRect = scanRect;
    if (!scanRect.isValid())
        return QBitmap();

    const uint bgPixel = QColor(Qt::color1).rgb();
    const uint fgPixel = QColor(Qt::color0).rgb();

    QImage image(scanRect.width(), scanRect.height(), 1, 2, QImage::LittleEndian);
    image.fill(bgPixel);
    int isects[MAX_INTERSECTIONS];
    int numISects;
    for (int y=0; y<scanRect.height(); ++y) {
        int scanLineY = y + scanRect.y();
        numISects = 0;
        for (int c=0; c<flatCurves.size(); ++c) {
            QPointArray curve = flatCurves.at(c);
            if (!scanRect.intersects(curve.boundingRect()))
                continue;
            Q_ASSERT(curve.size()>=2);
            QPoint p1 = curve.at(curve.size()-1);
            for (int i=0; i<curve.size(); ++i) {
                QPoint p2 = curve.at(i);

                // Does the line cross the scan line?
                if ((p1.y() <= scanLineY && p2.y() > scanLineY)
                    || (p1.y() > scanLineY && p2.y() <= scanLineY)) {
                    Q_ASSERT(numISects<MAX_INTERSECTIONS);

                    // Find intersection and add to set of intersetions for this scanline
                    // Horizontal lines are skipped since their end points are covered
                    // by other lines, and adding them would inverse the results
                    if (p1.y() != p2.y()) {
                        double idelta = (p2.x()-p1.x()) / double(p2.y()-p1.y());
                        isects[numISects++] =
                            qRound((scanLineY - p1.y()) * idelta + p1.x()) - scanRect.x();
                    }
                }
                p1 = p2;
            }
        }

        // Sort the intersection entries...
        qHeapSort(&isects[0], &isects[numISects]);

        uchar *scanLine = image.scanLine(y);
        for (int i=0; i<numISects; i+=2) {
            int from = qMax(0, isects[i]);
            int to = qMin(scanRect.width(), isects[i+1]);

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
                    memset(scanLine + entryByte + 1,  fgPixel, exitByte - entryByte - 1);

                // Last byte in this scan segment...
                *(scanLine + exitByte) |= (0xff >> (8-(to%8)));

            }
        }
    }
    QBitmap bm;
    bm.convertFromImage(image);
    return bm;
}

#define d d_func()
#define q q_func()

/**********************************************************************
 * class: QPainterPath
 */
QPainterPath::QPainterPath()
{
    d_ptr = new QPainterPathPrivate;
    d->subpaths.append(QPainterSubpath());
}

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
    Adds a straight line defined by the start point \a p1 and the end
    point \a p2 to the path.
 */
void QPainterPath::addLine(const QPoint &p1, const QPoint &p2)
{
    d->subpaths.last().addLine(p1, p2);
}

/*!
    \overload

    Adds a straight line from the last point to point \a p.
 */
void QPainterPath::addLine(const QPoint &p)
{
    d->subpaths.last().addLine(d->subpaths.last().lastPoint, p);
}

/*!
    Adds the given \a rect to the path. The \a rect is closed and is
    not considered to be part of the current subpath.
 */
void QPainterPath::addRect(const QRect &rect)
{
    QPainterSubpath subpath;
    int offset = QRect::rectangleMode();
    subpath.addLine(rect.topLeft(), rect.topRight() - QPoint(offset, 0));
    subpath.addLine(rect.bottomRight() - QPoint(offset, offset),
                    rect.bottomLeft() - QPoint(0, offset));
    subpath.close();
    d->subpaths.prepend(subpath);
}

/*!
    Adds a Bezier curve with control points \a p1, \a p2, \a p3, and
    \a p4, to the path.
*/
void QPainterPath::addBezier(const QPoint &p1, const QPoint &p2, const QPoint &p3, const QPoint &p4)
{
    d->subpaths.last().addBezier(p1, p2, p3, p4);
}

/*!
    \overload

    Adds the Bezier curve specified by the point array \a pa to the
    path. The point array <b>must</b> contain exactly four points or
    the function will give a warning and do nothing.
*/
void QPainterPath::addBezier(const QPointArray &pa)
{
    if (pa.size() != 4) {
        qWarning("QPainterPath::addBezier(const QPointArray &), array does not contain four elements");
        return;
    }
    addBezier(pa.at(0), pa.at(1), pa.at(2), pa.at(3));
}

void QPainterPath::addArc(const QRect &rect, int startAngle, int sweepLength)
{
    d->subpaths.last().addArc(rect, startAngle, sweepLength);
}

QPainterPath::FillMode QPainterPath::fillMode() const
{
    return d->fillMode;
}

void QPainterPath::setFillMode(QPainterPath::FillMode fillMode)
{
    d->fillMode = fillMode;
}


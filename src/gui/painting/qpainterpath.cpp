#include "qpainterpath.h"
#include "qpainterpath_p.h"

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
    const int THETAFACOTR = 2 * M_PI / (16.0 * 360);

    double rx = rect.width() / 2.0;
    double ry = rect.height() / 2.0;
    double theta = startAngle * THETAFACOTR;
    double cx = rect.x() + rx;
    double cy = rect.y() + ry;
    double sintheta = sin(theta);
    double costheta = cos(theta);
    double rhat = RHAT;

    QPoint firstPoint(cx + rhat * costheta, cy + rhat * sintheta);
    theta = (startAngle + arcLength) * THETAFACOTR;
    sintheta = sin(theta);
    costheta = cos(theta);
    rhat =  RHAT;
    QPoint lastPoint(cx + rhat * costheta, cy + rhat * sintheta);

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
        case QPainterPathElement::Bezier:
            p << QPoint(elm.bezierData.x2, elm.bezierData.y2);
            p << QPoint(elm.bezierData.x3, elm.bezierData.y3);
            p << QPoint(elm.bezierData.x4, elm.bezierData.y4);
            break;
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
 * Begins a new subpath. A path is already begun when the path
 * is started. If a path is begun and no elements are added to
 * it, the function does nothing.
 * The current subpath is not automatically closed when a new
 * subpath is begun.
 */
void QPainterPath::beginSubpath()
{
    if (d->subpaths.last().elements.isEmpty())
	return;
    d->subpaths.append(QPainterSubpath());
}


/*!
 * Closes the current subpath. If the sub path does not contain
 * any elements, the function does nothing. A new subpath is
 * automatically begun when the current is closed.
 */
void QPainterPath::closeSubpath()
{
    if (d->subpaths.last().elements.isEmpty())
	return;
    d->subpaths.last().close();
    d->subpaths.append(QPainterSubpath());
}


/*!
 * Adds the line defined by the starting point \a p1 and endpoint
 * \a p2 to the path.
 */
void QPainterPath::addLine(const QPoint &p1, const QPoint &p2)
{
    d->subpaths.last().addLine(p1, p2);
}

/*!
 * Adds a line from the last point to p
 */
void QPainterPath::addLine(const QPoint &p)
{
    d->subpaths.last().addLine(d->subpaths.last().lastPoint, p);
}

/*!
 * Adds the rect to the path. Each rect is closed and is not
 * considered part of the current subpath.
 */
void QPainterPath::addRect(const QRect &rect)
{
    QPainterSubpath subpath;
    subpath.addLine(rect.topLeft(), rect.topRight());
    subpath.addLine(rect.bottomRight(), rect.bottomLeft());
    subpath.close();
    d->subpaths.prepend(subpath);
}

/*!
 * Adds a Bezier curve to the path.
 */
void QPainterPath::addBezier(const QPoint &p1, const QPoint &p2, const QPoint &p3, const QPoint &p4)
{
    d->subpaths.last().addBezier(p1, p2, p3, p4);
}

/*!
 * Adds the Bezier curve specified by \pa to the path. The point array MUST contain
 * exactly four elements or the function will give a warning and do nothing.
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


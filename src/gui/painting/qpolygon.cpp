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

#include "qpolygon.h"
#include "qrect.h"
#include "qdatastream.h"
#include "qmatrix.h"
#include "qdebug.h"
#include "qpointarray.h"

#include <stdarg.h>

/*!
    \class QPolygon QPolygon.h
    \brief The QPolygon class provides a vector of points.
    \reentrant

    \ingroup multimedia
    \ingroup shared

    A QPolygon is a QVector\<QPoint\>. It is implicitly shared. In
    addition to the functions provided by QVector, QPolygon
    provides some point-specific functions.

    For convenient reading and writing of the point data use
    setPoints(), putPoints(), point(), and setPoint().

    For geometry operations use boundingRect() and translate(). There
    is also the QMatrix::map() function for more general
    transformations of QPolygons. You can also create arcs and
    ellipses with makeArc() and makeEllipse().

    Among others, QPolygon is used by QPainter::drawLineSegments(),
    QPainter::drawPolyline(), QPainter::drawPolygon() and
    QPainter::drawCubicBezier().

    \sa QPainter QMatrix QVector
*/


/*****************************************************************************
  QPolygon member functions
 *****************************************************************************/

/*!
    \fn QPolygon::QPolygon()

    Constructs a polygon with no points.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygon::QPolygon(int size)

    Constructs a polygon with \a size points. Makes a polygon with no points
    if \a size == 0.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygon::QPolygon(const QPolygon &other)

    Copy constructor. Constructs a copy of the \a other polygon.
*/

/*!
    \fn QPolygon::QPolygon(const QVector<QPointF> &vector)

    Constructs a polygon from the given \a vector of points.
*/

/*!
    \fn QPolygon::QPolygon(const QRectF &rect)

    Constructs a closed polygon from the rectangle specified by \a rect.

    The point array just contains the four vertices of the rectangle in
    clockwise order starting and ending with the top-left vertex.

*/

QPolygon::QPolygon(const QRectF &r)
{
    reserve(5);
    append(QPointF(r.x(), r.y()));
    append(QPointF(r.x() + r.width(), r.y()));
    append(QPointF(r.x() + r.width(), r.y() + r.height()));
    append(QPointF(r.x(), r.y() + r.height()));
    append(QPointF(r.x(), r.y()));
}


/*!
    \fn QPolygon::~QPolygon()

    Destroys the point array.
*/


/*!
    Translates all points in the polygon by the given \a offset.
*/

void QPolygon::translate(const QPointF &offset)
{
    register QPointF *p = data();
    register int i = size();
    while (i--) {
        *p += offset;
        ++p;
    }
}

/*!
    \fn void QPolygon::translate(float dx, float dy)
    \overload

    Translates all points in the polygon by (\a{dx}, \a{dy}).
*/

/*!
    \fn bool QPolygon::isClosed() const

    Returns true if the polygon is closed; otherwise returns false.
*/

/*!
    Returns the bounding rectangle of the polygon, or QRect(0,0,0,0) if the
    array is empty.
*/

QRectF QPolygon::boundingRect() const
{
    if (isEmpty())
        return QRectF(0, 0, 0, 0);
    register const QPointF *pd = constData();
    float minx, maxx, miny, maxy;
    minx = maxx = pd->x();
    miny = maxy = pd->y();
    ++pd;
    for (int i = 1; i < size(); ++i) {
        if (pd->x() < minx)
            minx = pd->x();
        else if (pd->x() > maxx)
            maxx = pd->x();
        if (pd->y() < miny)
            miny = pd->y();
        else if (pd->y() > maxy)
            maxy = pd->y();
        ++pd;
    }
    return QRectF(minx,miny, maxx - minx, maxy - miny);
}

/*!
    Returns the polygons vertices as a list of points.

    \sa fromPointArray()
*/

QPointArray QPolygon::toPointArray() const
{
    QPointArray pa;
    pa.reserve(size());
    for (int i=0; i<size(); ++i)
        pa.append(at(i).toPoint());
    return pa;
}

/*!
    \fn QPolygon QPolygon::fromPointArray(const QPointArray &array)

    Constructs a polygon from the given point \a array. The polygon will
    not be closed.

    \sa toPointArray()
*/

QPolygon QPolygon::fromPointArray(const QPointArray &a)
{
    QPolygon p;
    p.reserve(a.size());
    for (int i=0; i<a.size(); ++i)
        p.append(a.at(i));
    return p;
}


/*****************************************************************************
  QPolygon stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QPolygon

    Writes the point array, \a a to the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QPolygon &a)
{
    Q_UINT32 len = a.size();
    uint i;

    s << len;
    for (i = 0; i < len; ++i)
        s << a.at(i);
    return s;
}

/*!
    \relates QPolygon

    Reads a point array, \a a from the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QPolygon &a)
{
    Q_UINT32 len;
    uint i;

    s >> len;
    a.reserve(a.size() + (int)len);
    QPointF p;
    for (i = 0; i < len; ++i) {
        s >> p;
        a.insert(i, p);
    }
    return s;
}
#endif //QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QPolygon &a)
{
#ifndef Q_NO_STREAMING_DEBUG
    dbg.nospace() << "QPolygon(";
    for (int i = 0; i < a.count(); ++i)
        dbg.nospace() << a.at(i);
    dbg.nospace() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support the streaming of QDebug");
    return dbg;
    Q_UNUSED(a);
#endif
}
#endif


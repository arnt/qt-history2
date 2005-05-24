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
#include "qpainterpath.h"
#include "qvariant.h"
#include "qpainterpath_p.h"
#include "qbezier_p.h"

#include <stdarg.h>

/*!
    \class QPolygon
    \brief The QPolygon class provides a vector of points.
    \reentrant

    \ingroup multimedia
    \ingroup shared

    A QPolygon is a QVector\<QPoint\>. It is \l{implicitly shared}. In
    addition to the functions provided by QVector, QPolygon
    provides some point-specific functions.

    The easiest way to add points to a QPolygon is to use QVector's streaming
    operators, as illustrated below:

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto STREAM
    \skipto QPolygon
    \printuntil QPoint

    All other forms of manipulating a vector are available too, along with some
    compatibilty functions from Qt 3.

    For geometry operations use boundingRect() and translate(). There
    is also the QMatrix::map() function for more general
    transformations of QPolygons.

    Among others, QPolygon is used by QPainter::drawLineSegments(),
    QPainter::drawPolyline(), QPainter::drawPolygon() and
    QPainter::drawCubicBezier().

    \sa QPainter QMatrix QVector QPolygonF
*/


/*****************************************************************************
  QPolygon member functions
 *****************************************************************************/

/*!
    \fn QPolygon::QPolygon()

    Constructs an empty point array.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygon::QPolygon(int size)

    Constructs a point array with room for \a size points. Makes an
    empty array if \a size == 0.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygon::QPolygon(const QPolygon &a)

    Constructs a copy of the point array \a a.
*/

/*!
    \fn QPolygon::QPolygon(const QVector<QPoint> &pts)

    Constructs a pointarray containing a copy of the points specified
    in \a pts.
*/

/*!
    Constructs a point array from the rectangle \a r.

    If \a closed is false, then the point array just contains the
    following four points of the rectangle ordered clockwise. The
    bottom-right point is located at (r.x() + r.width(), r.y() +
    r.height()).

    If \a closed is true, then a fifth point is set to r.topLeft().
*/

QPolygon::QPolygon(const QRect &r, bool closed)
{
    reserve(closed ? 5 : 4);
    *this << QPoint(r.x(), r.y())
          << QPoint(r.x() + r.width(), r.y())
          << QPoint(r.x() + r.width(), r.y() + r.height())
          << QPoint(r.x(), r.y() + r.height());
    if (closed)
        *this << QPoint(r.left(), r.top());
}

/*!
  \internal
  Constructs a point array with \a nPoints points, taken from the
  \a points array.

  Equivalent to setPoints(nPoints, points).
*/

QPolygon::QPolygon(int nPoints, const int *points)
{
    setPoints(nPoints, points);
}


/*!
    \fn QPolygon::~QPolygon()

    Destroys the point array.
*/


/*!
    Translate all points in the array by (\a{dx}, \a{dy}).
*/

void QPolygon::translate(int dx, int dy)
{
    register QPoint *p = data();
    register int i = size();
    QPoint pt(dx, dy);
    while (i--) {
        *p += pt;
        ++p;
    }
}

/*! \fn void QPolygon::translate(const QPoint &offset)
    \overload

    Translate all points in the array by \a offset.
*/


/*!
    Reads the coordinates of the point at position \a index within the array
    and writes them into \c{*}\a{x} and \c{*}\a{y} if they are valid pointers.
*/

void QPolygon::point(int index, int *x, int *y) const
{
    QPoint p = at(index);
    if (x)
        *x = (int)p.x();
    if (y)
        *y = (int)p.y();
}

/*!
    \fn QPoint QPolygon::point(int index) const

    \overload

    Returns the point at position \a index within the array.
*/

/*!
    \fn void QPolygon::setPoint(int i, const QPoint &p)

    \overload

    Sets the point at array index \a i to \a p.

    \sa putPoints() setPoints()
*/

/*!
    \fn void QPolygon::setPoint(int index, int x, int y)

    Sets the point at position \a index in the array to (\a{x}, \a{y}).

    \sa putPoints() setPoints()
*/

/*!
    Resizes the array to \a nPoints and sets the points in the array to
    the values taken from \a points.

    The example code creates an array with two points (10, 20) and
    (30, 40):

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto SETPOINTS
    \skipto static
    \printuntil setPoints

    \sa resize() setPoint() putPoints()
*/

void QPolygon::setPoints(int nPoints, const int *points)
{
    resize(nPoints);
    int i = 0;
    while (nPoints--) {
        setPoint(i++, *points, *(points+1));
        points += 2;
    }
}

/*!
    \overload

    Resizes the array to \a nPoints and sets the points in the array
    to the values taken from the variable argument list.

    The example code creates an array with two points (10, 20) and
    (30, 40):

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto SETPOINTS2
    \skipto QPolygon
    \printuntil setPoints

    The points are given as a sequence of integers, starting with \a
    firstx then \a firsty, and so on.

    \sa resize(), putPoints() setPoint()
*/

void QPolygon::setPoints(int nPoints, int firstx, int firsty, ...)
{
    va_list ap;
    resize(nPoints);
    setPoint(0, firstx, firsty);
    int i = 0, x, y;
    va_start(ap, firsty);
    while (--nPoints) {
        x = va_arg(ap, int);
        y = va_arg(ap, int);
        setPoint(++i, x, y);
    }
    va_end(ap);
}

/*!
  \overload
  \internal
  Copies \a nPoints points from the \a points coord array into
  this point array, and resizes the point array if
  \c{index+nPoints} exceeds the size of the array.

    \sa resize() setPoint()
*/

void QPolygon::putPoints(int index, int nPoints, const int *points)
{
    if (index + nPoints > size())
        resize(index + nPoints);
    int i = index;
    while (nPoints--) {
        setPoint(i++, *points, *(points+1));
        points += 2;
    }
}

/*!
    Copies \a nPoints points from the variable argument list into this
    point array from position \a index, and resizes the point array if
    \c{index+nPoints} exceeds the size of the array.

    The example code creates an array with three points (4,5), (6,7)
    and (8,9), by expanding the array from 1 to 3 points:

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto PUTPOINTS
    \skipto QPolygon
    \printuntil putPoints

    This has the same result, but here putPoints overwrites rather
    than extends:
    \quotefromfile snippets/polygon/polygon.cpp
    \skipto PUTPOINTS2
    \skipto QPolygon
    \printuntil putPoints(1, 1

    The points are given as a sequence of integers, starting with \a
    firstx then \a firsty, and so on.

    \sa setPoints()
*/

void QPolygon::putPoints(int index, int nPoints, int firstx, int firsty, ...)
{
    va_list ap;
    if (index + nPoints > size())
        resize(index + nPoints);
    if (nPoints <= 0)
        return;
    setPoint(index, firstx, firsty);
    int i = index, x, y;
    va_start(ap, firsty);
    while (--nPoints) {
        x = va_arg(ap, int);
        y = va_arg(ap, int);
        setPoint(++i, x, y);
    }
    va_end(ap);
}


/*!
    \overload

    This version of the function copies \a nPoints from \a from into
    this array, starting at \a index in this array and \a fromIndex in
    \a from. \a fromIndex is 0 by default.

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto PUTPOINTS3
    \skipto QPolygon
    \printuntil polygon1 is now

    \sa setPoints()
*/

void QPolygon::putPoints(int index, int nPoints, const QPolygon & from, int fromIndex)
{
    if (index + nPoints > size())
        resize(index + nPoints);
    if (nPoints <= 0)
        return;
    int n = 0;
    while(n < nPoints) {
        setPoint(index + n, from[fromIndex+n]);
        ++n;
    }
}


/*!
    Returns the bounding rectangle of the points in the array, or
    QRect(0, 0, 0, 0) if the array is empty.
*/

QRect QPolygon::boundingRect() const
{
    if (isEmpty())
        return QRect(0, 0, 0, 0);
    register const QPoint *pd = constData();
    int minx, maxx, miny, maxy;
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
    return QRect(QPoint(minx,miny), QPoint(maxx,maxy));
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QPolygon &a)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QPolygon(";
    for (int i = 0; i < a.count(); ++i)
        dbg.nospace() << a.at(i);
    dbg.nospace() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QPolygon to QDebug");
    return dbg;
    Q_UNUSED(a);
#endif
}
#endif


/*!
    \class QPolygonF qpolygon.h
    \brief The QPolygonF class provides a vector of floating point points.
    \reentrant

    \ingroup multimedia
    \ingroup shared

    A QPolygonF is a QVector\<QPointF\>. It is \l{implicitly shared}. In
    addition to the functions provided by QVector, QPolygonF
    provides some point-specific functions.

    The easiest way to add points to a QPolygonF is to use QVector's streaming
    operators, as illustrated below:

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto STREAMF
    \skipto QPolygonF
    \printuntil QPointF

    All other forms of manipulating a vector are available too.

    For geometry operations use boundingRect() and translate(). There
    is also the QMatrix::map() function for more general
    transformations of QPolygonFs.

    Among others, QPolygonF is used by QPainter::drawLineSegments(),
    QPainter::drawPolyline(), QPainter::drawPolygon() and
    QPainter::drawCubicBezier().

    \sa QPainter QMatrix QVector QPolygon
*/


/*****************************************************************************
  QPolygonF member functions
 *****************************************************************************/

/*!
    \fn QPolygonF::QPolygonF()

    Constructs a polygon with no points.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygonF::QPolygonF(int size)

    Constructs a polygon with \a size points. Makes a polygon with no points
    if \a size == 0.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygonF::QPolygonF(const QPolygonF &other)

    Copy constructor. Constructs a copy of the \a other polygon.
*/

/*!
    \fn QPolygonF::QPolygonF(const QVector<QPointF> &vector)

    Constructs a polygon from the given \a vector of points.
*/

/*!
    \fn QPolygonF::QPolygonF(const QRectF &rect)

    Constructs a closed polygon from the rectangle specified by \a rect.

    The point array just contains the four vertices of the rectangle in
    clockwise order starting and ending with the top-left vertex.

    \sa QPolygon::QPolygon() isClosed()
*/

QPolygonF::QPolygonF(const QRectF &r)
{
    reserve(5);
    append(QPointF(r.x(), r.y()));
    append(QPointF(r.x() + r.width(), r.y()));
    append(QPointF(r.x() + r.width(), r.y() + r.height()));
    append(QPointF(r.x(), r.y() + r.height()));
    append(QPointF(r.x(), r.y()));
}

/*!
    \fn QPolygonF::QPolygonF(const QPolygon &a)

    Constructs a float based polygon from the int based polygon
    specified by \a a.

    \sa toPolygon()
*/

QPolygonF::QPolygonF(const QPolygon &a)
{
    reserve(a.size());
    for (int i=0; i<a.size(); ++i)
        append(a.at(i));
}

/*!
    \fn QPolygonF::~QPolygonF()

    Destroys the point array.
*/


/*!
    Translate all points in the polygon by the given \a offset.
*/

void QPolygonF::translate(const QPointF &offset)
{
    register QPointF *p = data();
    register int i = size();
    while (i--) {
        *p += offset;
        ++p;
    }
}

/*!
    \fn void QPolygonF::translate(qreal dx, qreal dy)
    \overload

    Translates all points in the polygon by (\a{dx}, \a{dy}).
*/

/*!
    \fn bool QPolygonF::isClosed() const

    Returns true if the polygon is closed; otherwise returns false.

    A polygon is said to be closed if its start point and end point are equal.

    \sa QVector::first() QVector::last()
*/

/*!
    Returns the bounding rectangle of the polygon, or QRectF(0,0,0,0) if the
    array is empty.
*/

QRectF QPolygonF::boundingRect() const
{
    if (isEmpty())
        return QRectF(0, 0, 0, 0);
    register const QPointF *pd = constData();
    qreal minx, maxx, miny, maxy;
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
    Returns a QPolygon by converting each QPointF to a QPoint.

    \sa QPointF::toPoint()
*/

QPolygon QPolygonF::toPolygon() const
{
    QPolygon a;
    a.reserve(size());
    for (int i=0; i<size(); ++i)
        a.append(at(i).toPoint());
    return a;
}

/*!
   Returns the polygon as a QVariant
*/
QPolygon::operator QVariant() const
{
    return QVariant(QVariant::Polygon, this);
}

/*****************************************************************************
  QPolygonF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QPolygonF

    Writes the point array, \a a to the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QPolygonF &a)
{
    quint32 len = a.size();
    uint i;

    s << len;
    for (i = 0; i < len; ++i)
        s << a.at(i);
    return s;
}

/*!
    \relates QPolygonF

    Reads a point array, \a a from the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QPolygonF &a)
{
    quint32 len;
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

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QPolygonF &a)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QPolygonF(";
    for (int i = 0; i < a.count(); ++i)
        dbg.nospace() << a.at(i);
    dbg.nospace() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QPolygonF to QDebug");
    return dbg;
    Q_UNUSED(a);
#endif
}
#endif


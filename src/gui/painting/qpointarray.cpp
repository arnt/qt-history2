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

#include "qpointarray.h"
#include "qrect.h"
#include "qdatastream.h"
#include "qmatrix.h"
#include "qdebug.h"
#include "qpainterpath.h"
#include "qpainterpath_p.h"
#include "qbezier_p.h"
#include <stdarg.h>

const double Q_PI = 3.14159265358979323846;

/*!
    \class QPointArray qpointarray.h
    \brief The QPointArray class provides a vector of points.
    \reentrant

    \ingroup multimedia
    \ingroup shared

    A QPointArray is a QVector\<QPoint\>. It is implicitly shared. In
    addition to the functions provided by QVector, QPointArray
    provides some point-specific functions.

    For convenient reading and writing of the point data use
    setPoints(), putPoints(), point(), and setPoint().

    For geometry operations use boundingRect() and translate(). There
    is also the QMatrix::map() function for more general
    transformations of QPointArrays. You can also create arcs and
    ellipses with makeArc() and makeEllipse().

    Among others, QPointArray is used by QPainter::drawLineSegments(),
    QPainter::drawPolyline(), QPainter::drawPolygon() and
    QPainter::drawCubicBezier().

    \sa QPainter QMatrix QVector
*/


/*****************************************************************************
  QPointArray member functions
 *****************************************************************************/

/*!
    \fn QPointArray::QPointArray()

    Constructs a null point array.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPointArray::QPointArray(int size)

    Constructs a point array with room for \a size points. Makes a
    null array if \a size == 0.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPointArray::QPointArray(const QPointArray &a)

    Constructs a copy of the point array \a a.
*/

/*!
    \fn QPointArray::QPointArray(const QVector<QPoint> &pts)

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

QPointArray::QPointArray(const QRect &r, bool closed)
{
    setPoints(4,
              r.x(),  r.y(),
              r.x() + r.width(), r.y(),
              r.x() + r.width(), r.y() + r.height(),
              r.x(),  r.y() + r.height());
    if (closed) {
        resize(5);
        setPoint(4, r.left(), r.top());
    }
}

/*!
  \internal
  Constructs a point array with \a nPoints points, taken from the
  \a points array.

  Equivalent to setPoints(nPoints, points).
*/

QPointArray::QPointArray(int nPoints, const QCOORD *points)
{
    setPoints(nPoints, points);
}


/*!
    \fn QPointArray::~QPointArray()

    Destroys the point array.
*/


/*!
    Translates all points in the array by (\a{dx}, \a{dy}).
*/

void QPointArray::translate(int dx, int dy)
{
    register QPoint *p = data();
    register int i = size();
    QPoint pt(dx, dy);
    while (i--) {
        *p += pt;
        ++p;
    }
}

/*! \fn void QPointArray::translate(const QPoint &offset)
    \overload

    Translates all points in the array by \a offset.
*/


/*!
    Reads the coordinates of the point at position \a index within the
    array and writes them into \c{*}\a{x} and \c{*}\a{y}.
*/

void QPointArray::point(int index, int *x, int *y) const
{
    QPoint p = at(index);
    if (x)
        *x = (int)p.x();
    if (y)
        *y = (int)p.y();
}

/*!
    \fn QPoint QPointArray::point(int index) const

    \overload

    Returns the point at position \a index within the array.
*/

/*!
    \fn void QPointArray::setPoint(int i, const QPoint &p)

    \overload

    Sets the point at array index \a i to \a p.
*/

/*!
    \fn void QPointArray::setPoint(int index, int x, int y)

    Sets the point at position \a index in the array to (\a{x}, \a{y}).
*/

/*!
    Resizes the array to \a nPoints and sets the points in the array to
    the values taken from \a points.

    The example code creates an array with two points (10, 20) and
    (30, 40):

    \code
        static const QCOORD points[] = { 10, 20, 30, 40 };
        QPointArray a;
        a.setPoints(2, points);
    \endcode

    \c QCOORD is a typedef for a signed integer type with at least 32
    bits.

    \sa resize()
*/

void QPointArray::setPoints(int nPoints, const QCOORD *points)
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

    \code
        QPointArray a;
        a.setPoints(2, 10, 20, 30, 40);
    \endcode

    The points are given as a sequence of integers, starting with \a
    firstx then \a firsty, and so on.

    \sa resize(), putPoints()
*/

void QPointArray::setPoints(int nPoints, int firstx, int firsty, ...)
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

/*! \overload
  \internal
  Copies \a nPoints points from the \a points coord array into
  this point array, and resizes the point array if
  \c{index+nPoints} exceeds the size of the array.

*/

void QPointArray::putPoints(int index, int nPoints, const QCOORD *points)
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

    \code
        QPointArray a(1);
        a[0] = QPoint(4, 5);
        a.putPoints(1, 2, 6,7, 8,9); // index == 1, points == 2
    \endcode

    This has the same result, but here putPoints overwrites rather
    than extends:
    \code
        QPointArray a(3);
        a.putPoints(0, 3, 4,5, 0,0, 8,9);
        a.putPoints(1, 1, 6,7);
    \endcode

    The points are given as a sequence of integers, starting with \a
    firstx then \a firsty, and so on.
*/

void QPointArray::putPoints(int index, int nPoints, int firstx, int firsty, ...)
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

    \code
        QPointArray a;
        a.putPoints(0, 3, 1,2, 0,0, 5,6);
        // a is now the three-point array (1,2, 0,0, 5,6);
        QPointArray b;
        b.putPoints(0, 3, 4,4, 5,5, 6,6);
        // b is now (4,4, 5,5, 6,6);
        a.putPoints(2, 3, b);
        // a is now (1,2, 0,0, 4,4, 5,5, 6,6);
    \endcode
*/

void QPointArray::putPoints(int index, int nPoints, const QPointArray & from, int fromIndex)
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

QRect QPointArray::boundingRect() const
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


#ifdef QT_COMPAT
/*!
    Sets the points of the array to those describing an arc of an
    ellipse with size, width \a w by height \a h, and position (\a x,
    \a y), starting from angle \a a1 and spanning by angle \a a2. The
    resulting array has sufficient resolution for pixel accuracy (see
    the overloaded function which takes an additional QMatrix
    parameter).

    Angles are specified in 16ths of a degree, i.e. a full circle
    equals 5760 (16*360). Positive values mean counter-clockwise,
    whereas negative values mean the clockwise direction. Zero degrees
    is at the 3 o'clock position.

    See the \link qcanvas.html#anglediagram angle diagram\endlink.
*/
#ifndef QT_NO_WMATRIX
void QPointArray::makeArc(int x, int y, int w, int h, int a1, int a2)
{
    QRectF r(x, y, w, h);
    QPointF startPoint;
    qt_find_ellipse_coords(r, a1, a2, &startPoint, 0);

    QPainterPath path(startPoint);
    path.arcTo(r, a1, a2);
    *this = path.toSubpathPolygons().at(0).toPointArray();
}
#endif

#ifndef QT_NO_TRANSFORMATIONS
/*!
    \overload

    Sets the points of the array to those describing an arc of an
    ellipse with width \a w and height \a h and position (\a x, \a y),
    starting from angle \a a1, and spanning angle by \a a2, and
    transformed by the matrix \a xf. The resulting array has
    sufficient resolution for pixel accuracy.

    Angles are specified in 16ths of a degree, i.e. a full circle
    equals 5760 (16 * 360). Positive values mean counter-clockwise,
    whereas negative values mean the clockwise direction. Zero
    degrees is at the 3 o'clock position.

    \sa \link qcanvas.html#anglediagram angle diagram\endlink
*/
void QPointArray::makeArc(int x, int y, int w, int h, int a1, int a2, const QMatrix &xf)
{
    QRectF r(x, y, w, h);
    QPointF startPoint;
    qt_find_ellipse_coords(r, a1, a2, &startPoint, 0);

    QPainterPath path(startPoint);
    path.arcTo(r, a1, a2);
    path = path * xf;
    *this = path.toSubpathPolygons().at(0).toPointArray();
}

#endif // QT_NO_TRANSFORMATIONS

/*!
    Sets the points of the array to those describing an ellipse with
    size, width \a w by height \a h, and position (\a x, \a y).

    The returned array has sufficient resolution for use as pixels.
*/
void QPointArray::makeEllipse(int x, int y, int w, int h)
{
    QPainterPath path;
    path.addEllipse(x, y, w, h);
    *this = path.toSubpathPolygons().at(0).toPointArray();
}

#ifndef QT_NO_BEZIER

/*!
    Returns the Bezier points for the four control points in this
    array.
*/

QPointArray QPointArray::cubicBezier() const
{
    if (size() != 4) {
	qWarning( "QPointArray::bezier: The array must have 4 control points" );
        return QPointArray();
    }
    QPolygon polygon = QBezier(at(0), at(1), at(2), at(3)).toPolygon();
    return polygon.toPointArray();
}
#endif //QT_NO_BEZIER

#endif // QT_COMPAT

#ifndef QT_NO_DEBUG_OUTPUT
QDebug operator<<(QDebug dbg, const QPointArray &a)
{
#ifndef Q_NO_STREAMING_DEBUG
    dbg.nospace() << "QPointArray(";
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

/*!
   \fn QPointArray QPointArray::copy() const

   Use simple assignment instead.
*/

/*!
   \fn bool QPointArray::isNull()

   Use isEmpty() instead.
*/


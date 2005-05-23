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

#include "qdatastream.h"
#include "qdebug.h"
#include "qmath_p.h"
#include "qmatrix.h"
#include "qregion.h"
#include "qpainterpath.h"

#include <limits.h>

#ifndef QT_NO_MATRIX

/*!
    \class QMatrix qmatrix.h
    \brief The QMatrix class specifies 2D transformations of a
    coordinate system.

    \ingroup multimedia

    The standard coordinate system of a \link QPaintDevice paint
    device\endlink has the origin located at the top-left position. \e
    x values increase to the right; \e y values increase downward.

    This coordinate system is the default for the QPainter, which
    renders graphics in a paint device. A user-defined coordinate
    system can be specified by setting a QMatrix for the painter.

    Example:
    \quotefromfile snippets/matrix/matrix.cpp
    \skipto PAINT
    \skipto paintEvent
    \printuntil }

    A matrix specifies how to translate, scale, shear or rotate the
    graphics; the actual transformation is performed by the drawing
    routines in QPainter and by QPixmap::xForm().

    The QMatrix class contains a 3x3 matrix of the form:
    \table
    \row \i m11 \i m12 \i 0
    \row \i m21 \i m22 \i 0
    \row \i dx  \i dy  \i 1
    \endtable

    A matrix transforms a point in the plane to another point:
    \code
        x' = m11*x + m21*y + dx
        y' = m22*y + m12*x + dy
    \endcode

    The point \e (x, y) is the original point, and \e (x', y') is the
    transformed point. \e (x', y') can be transformed back to \e (x,
    y) by performing the same operation on the \link
    QMatrix::inverted() inverted matrix\endlink.

    The elements \e dx and \e dy specify horizontal and vertical
    translation. The elements \e m11 and \e m22 specify horizontal and
    vertical scaling. The elements \e m12 and \e m21 specify
    horizontal and vertical shearing.

    The identity matrix has \e m11 and \e m22 set to 1; all others are
    set to 0. This matrix maps a point to itself.

    Translation is the simplest transformation. Setting \e dx and \e
    dy will move the coordinate system \e dx units along the X axis
    and \e dy units along the Y axis.

    Scaling can be done by setting \e m11 and \e m22. For example,
    setting \e m11 to 2 and \e m22 to 1.5 will double the height and
    increase the width by 50%.

    Shearing is controlled by \e m12 and \e m21. Setting these
    elements to values different from zero will twist the coordinate
    system.

    Rotation is achieved by carefully setting both the shearing
    factors and the scaling factors. The QMatrix also has a function
    that sets \link rotate() rotation \endlink directly.

    QMatrix lets you combine transformations like this:
    \quotefromfile snippets/matrix/matrix.cpp
    \skipto COMBINE
    \skipto QMatrix
    \printuntil scale

    Here's the same example using basic matrix operations:
    \quotefromfile snippets/matrix/matrix.cpp
    \skipto OPERATIONS
    \skipto double
    \printuntil combine

    The matrix can also be transformed using the map() functions, and
    transformed points, rectangles, etc., can be obtained using map(),
    mapRect(), mapToRegion(), and mapToPolygon() functions.

    \l QPainter has functions to translate, scale, shear and rotate the
    coordinate system without using a QMatrix. Although these
    functions are very convenient, it can be more efficient to build a
    QMatrix and call QPainter::setMatrix() if you want to perform
    more than a single transform operation.

    \sa QPainter::setMatrix(), QPixmap::xForm()
*/


// some defines to inline some code
#define MAPDOUBLE(x, y, nx, ny) \
{ \
    qreal fx = x; \
    qreal fy = y; \
    nx = _m11*fx + _m21*fy + _dx; \
    ny = _m12*fx + _m22*fy + _dy; \
}

#define MAPINT(x, y, nx, ny) \
{ \
    qreal fx = x; \
    qreal fy = y; \
    nx = qRound(_m11*fx + _m21*fy + _dx); \
    ny = qRound(_m12*fx + _m22*fy + _dy); \
}

/*****************************************************************************
  QMatrix member functions
 *****************************************************************************/

/*!
    Constructs an identity matrix. All elements are set to zero except
    \e m11 and \e m22 (scaling), which are set to 1.
*/

QMatrix::QMatrix()
{
    _m11 = _m22 = 1.0;
    _m12 = _m21 = _dx = _dy = 0.0;
}

/*!
    Constructs a matrix with the elements, \a m11, \a m12, \a m21, \a
    m22, \a dx and \a dy.
*/

QMatrix::QMatrix(qreal m11, qreal m12, qreal m21, qreal m22,
                    qreal dx, qreal dy)
{
    _m11 = m11;         _m12 = m12;
    _m21 = m21;         _m22 = m22;
    _dx         = dx;         _dy  = dy;
}


/*! Constructs a matrix with the elements from the matrix \a matrix
 */
QMatrix::QMatrix(const QMatrix &matrix)
{
    *this = matrix;
}

/*!
    Sets the matrix elements to the specified values, \a m11, \a m12,
    \a m21, \a m22, \a dx and \a dy.
*/

void QMatrix::setMatrix(qreal m11, qreal m12, qreal m21, qreal m22,
                          qreal dx, qreal dy)
{
    _m11 = m11;         _m12 = m12;
    _m21 = m21;         _m22 = m22;
    _dx         = dx;         _dy  = dy;
}


/*!
    \fn qreal QMatrix::m11() const

    Returns the X scaling factor.
*/

/*!
    \fn qreal QMatrix::m12() const

    Returns the vertical shearing factor.
*/

/*!
    \fn qreal QMatrix::m21() const

    Returns the horizontal shearing factor.
*/

/*!
    \fn qreal QMatrix::m22() const

    Returns the Y scaling factor.
*/

/*!
    \fn qreal QMatrix::dx() const

    Returns the horizontal translation.
*/

/*!
    \fn qreal QMatrix::dy() const

    Returns the vertical translation.
*/


/*!
    Transforms (\a{x}, \a{y}) to (\c{*}\a{tx}, \c{*}\a{ty}) using the
    following formulas:

    \code
        *tx = m11*x + m21*y + dx
        *ty = m22*y + m12*x + dy
    \endcode
*/

void QMatrix::map(qreal x, qreal y, qreal *tx, qreal *ty) const
{
    MAPDOUBLE(x, y, *tx, *ty);
}



/*!
    \overload

    Transforms (\a{x}, \a{y}) to (\c{*}\a{tx}, \c{*}\a{ty}) using the formulas:

    \code
        *tx = m11*x + m21*y + dx  (rounded to the nearest integer)
        *ty = m22*y + m12*x + dy  (rounded to the nearest integer)
    \endcode
*/

void QMatrix::map(int x, int y, int *tx, int *ty) const
{
    MAPINT(x, y, *tx, *ty);
}



/*!
    Returns the transformed rectangle \a rect rounded to the neares integer.

    The bounding rectangle is returned if rotation or shearing has
    been specified.

    If you need to know the exact region \a rect maps to use \l
    operator*().

    \sa operator*()
*/
QRect QMatrix::mapRect(const QRect &rect) const
{
    QRect result;
    if (_m12 == 0.0F && _m21 == 0.0F) {
        int x = qRound(_m11*rect.x() + _dx);
        int y = qRound(_m22*rect.y() + _dy);
        int w = qRound(_m11*rect.width());
        int h = qRound(_m22*rect.height());
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        result = QRect(x, y, w, h);
    } else {
        // see mapToPolygon for explanations of the algorithm.
        qreal x0, y0;
        qreal x, y;
        MAPDOUBLE(rect.left(), rect.top(), x0, y0);
        qreal xmin = x0;
        qreal ymin = y0;
        qreal xmax = x0;
        qreal ymax = y0;
        MAPDOUBLE(rect.right() + 1, rect.top(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.right() + 1, rect.bottom() + 1, x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.left(), rect.bottom() + 1, x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        qreal w = xmax - xmin;
        qreal h = ymax - ymin;
        xmin -= (xmin - x0) / w;
        ymin -= (ymin - y0) / h;
        xmax -= (xmax - x0) / w;
        ymax -= (ymax - y0) / h;
        result = QRect(qRound(xmin), qRound(ymin), qRound(xmax)-qRound(xmin)+1, qRound(ymax)-qRound(ymin)+1);
    }
    return result;
}

/*!
    Returns the transformed rectangle \a rect.

    The bounding rectangle is returned if rotation or shearing has
    been specified.

    If you need to know the exact region \a rect maps to use \l
    operator*().

    \sa operator*()
*/
QRectF QMatrix::mapRect(const QRectF &rect) const
{
    QRectF result;
    if (_m12 == 0.0F && _m21 == 0.0F) {
        qreal x = _m11*rect.x() + _dx;
        qreal y = _m22*rect.y() + _dy;
        qreal w = _m11*rect.width();
        qreal h = _m22*rect.height();
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        result = QRectF(x, y, w, h);
    } else {
        qreal x0, y0;
        qreal x, y;
        MAPDOUBLE(rect.x(), rect.y(), x0, y0);
        qreal xmin = x0;
        qreal ymin = y0;
        qreal xmax = x0;
        qreal ymax = y0;
        MAPDOUBLE(rect.x() + rect.width(), rect.y(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.x() + rect.width(), rect.y() + rect.width(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.x(), rect.y() + rect.height(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        result = QRectF(xmin, ymin, xmax-xmin, ymax - ymin);
    }
    return result;
}


/*!
    \fn QPoint operator*(const QPoint &p, const QMatrix &m)

    \relates QMatrix

    This is the same as \a{m}.mapRect(\a{p}).
*/

/*!
    \overload

    Transforms \a p to using these formulas:

    \code
        retx = m11*px + m21*py + dx  (rounded to the nearest integer)
        rety = m22*py + m12*px + dy  (rounded to the nearest integer)
    \endcode
*/
QPoint QMatrix::map(const QPoint &p) const
{
    qreal fx = p.x();
    qreal fy = p.y();
    return QPoint(qRound(_m11*fx + _m21*fy + _dx),
                   qRound(_m12*fx + _m22*fy + _dy));
}

/*!
  \fn QPointF operator*(const QPointF &p, const QMatrix &m)

  \relates QMatrix

  Same as \a{m}.map(\a{p}).

  \sa QMatrix::map()
*/

/*!
    \overload

    Transforms \a point using these formulas:

    \code
        retx = m11*px + m21*py + dx
        rety = m22*py + m12*px + dy
    \endcode
*/
QPointF QMatrix::map(const QPointF &point) const
{
    qreal fx = point.x();
    qreal fy = point.y();
    return QPointF(_m11*fx + _m21*fy + _dx, _m12*fx + _m22*fy + _dy);
}

/*!
    \overload

    Transforms both ends of \a line using these formulas:

    \code
        retx = m11 * px + m21 * py + dx
        rety = m22 * py + m12 * px + dy
    \endcode
*/
QLineF QMatrix::map(const QLineF &line) const
{
    return QLineF(map(line.p1()), map(line.p2()));
}

/*!
    \overload

    Transforms both ends of \a line using these formulas:

    \code
        retx = m11 * px + m21 * py + dx (rounded to the nearest integer)
        rety = m22 * py + m12 * px + dy (rounded to the nearest integer)
    \endcode
*/
QLine QMatrix::map(const QLine &line) const
{
    return QLine(map(line.p1()), map(line.p2()));
}

/*!
    \fn QPolygon operator*(const QPolygon &a, const QMatrix &m)

    \relates QMatrix

    This is the same as \a{m}.mapRect(\a{a}).

    \sa QMatrix::mapRect()
*/

/*!
    \overload

    Returns the point array \a a transformed by calling map for each point.
*/
QPolygon QMatrix::map(const QPolygon &a) const
{
    int size = a.size();
    int i;
    QPolygonF p(size);
    const QPoint *da = a.constData();
    QPointF *dp = p.data();
    qreal xmin = qreal(INT_MAX);
    qreal ymin = xmin;
    qreal xmax = qreal(INT_MIN);
    qreal ymax = xmax;
    int xminp = 0;
    int yminp = 0;
    for(i = 0; i < size; i++) {
        dp[i].xp = da[i].x();
        dp[i].yp = da[i].y();
        if (dp[i].xp < xmin) {
            xmin = dp[i].xp;
            xminp = i;
        }
        if (dp[i].yp < ymin) {
            ymin = dp[i].yp;
            yminp = i;
        }
        xmax = qMax(xmax, dp[i].xp);
        ymax = qMax(ymax, dp[i].yp);
    }
    qreal w = qMax<qreal>(xmax - xmin, 1.);
    qreal h = qMax<qreal>(ymax - ymin, 1.);
    for(i = 0; i < size; i++) {
        dp[i].xp += (dp[i].xp - xmin)/w;
        dp[i].yp += (dp[i].yp - ymin)/h;
        MAPDOUBLE(dp[i].xp, dp[i].yp, dp[i].xp, dp[i].yp);
    }

    // now apply correction back for transformed values...
    xmin = qreal(INT_MAX/256);
    ymin = xmin;
    xmax = qreal(INT_MIN/256);
    ymax = xmax;
    for(i = 0; i < size; i++) {
        xmin = qMin<qreal>(xmin, dp[i].xp);
        ymin = qMin<qreal>(ymin, dp[i].yp);
        xmax = qMax<qreal>(xmax, dp[i].xp);
        ymax = qMax<qreal>(ymax, dp[i].yp);
    }
    w = qMax<qreal>(xmax - xmin, 1.);
    h = qMax<qreal>(ymax - ymin, 1.);

    QPolygon result(size);
    QPoint *dr = result.data();
    for(i = 0; i < size; i++) {
        dr[i].setX(qRound(dp[i].xp - (dp[i].xp - dp[xminp].xp)/w));
        dr[i].setY(qRound(dp[i].yp - (dp[i].yp - dp[yminp].yp)/h));
    }
    return result;
}

/*!
    \overload

    Returns the point array \a a transformed by calling map for each point.
*/
QPolygonF QMatrix::map(const QPolygonF &a) const
{
    int size = a.size();
    int i;
    QPolygonF p(size);
    const QPointF *da = a.constData();
    QPointF *dp = p.data();
    for(i = 0; i < size; i++) {
        MAPDOUBLE(da[i].xp, da[i].yp, dp[i].xp, dp[i].yp);
    }
    return p;
}

/*!
    \fn QRegion operator*(const QRegion &r, const QMatrix &m)

    \relates QMatrix

    This is the same as \a{m}.mapRect(\a{r}).

    \sa QMatrix::mapRect()
*/

/*!
    \overload

    Transforms the region \a r.

    Calling this method can be rather expensive, if rotations or
    shearing are used.
*/
QRegion QMatrix::map(const QRegion &r) const
{
    if (_m11 == 1.0 && _m22 == 1.0 && _m12 == 0.0 && _m21 == 0.0) { // translate or identity
        if (_dx == 0.0 && _dy == 0.0) // Identity
            return r;
        QRegion copy(r);
        copy.translate(qRound(_dx), qRound(_dy));
        return copy;
    }

    QPainterPath p;
    p.addRegion(r);
    p = map(p);
    return p.toFillPolygon().toPolygon();
}

/*!
    \overload

    Transforms the painter path \a path.
*/
QPainterPath QMatrix::map(const QPainterPath &path) const
{
    if (path.isEmpty())
        return QPainterPath();

    QPainterPath copy = path;

    // Translate or identity
    if (_m11 == 1.0 && _m22 == 1.0 && _m12 == 0.0 && _m21 == 0.0) {

        // Translate
        if (_dx != 0.0 || _dy != 0.0) {
            copy.detach();
            for (int i=0; i<path.elementCount(); ++i) {
                QPainterPath::Element &e = copy.d_ptr->elements[i];
                e.x += _dx;
                e.y += _dy;
            }
        }

    // Full xform
    } else {
        copy.detach();
        for (int i=0; i<path.elementCount(); ++i) {
            QPainterPath::Element &e = copy.d_ptr->elements[i];
            qreal fx = e.x, fy = e.y;
            e.x = _m11*fx + _m21*fy + _dx;
            e.y =  _m12*fx + _m22*fy + _dy;
        }
    }

    return copy;
}

/*!
    \fn QRegion QMatrix::mapToRegion(const QRect &rect) const

    Returns the transformed rectangle \a rect.

    A rectangle which has been rotated or sheared may result in a
    non-rectangular region being returned.

    Calling this method can be expensive, if rotations or shearing are
    used. If you just need to know the bounding rectangle of the
    returned region, use mapRect() which is a lot faster than this
    function.

    \sa QMatrix::mapRect()
*/
#ifdef QT3_SUPPORT
QRegion QMatrix::mapToRegion(const QRect &rect) const
{
    QRegion result;
    if (isIdentity()) {
        result = rect;
    } else if (m12() == 0.0F && m21() == 0.0F) {
        int x = qRound(m11()*rect.x() + dx());
        int y = qRound(m22()*rect.y() + dy());
        int w = qRound(m11()*rect.width());
        int h = qRound(m22()*rect.height());
        if (w < 0) {
            w = -w;
            x -= w - 1;
        }
        if (h < 0) {
            h = -h;
            y -= h - 1;
        }
        result = QRect(x, y, w, h);
    } else {
        result = QRegion(mapToPolygon(rect));
    }
    return result;

}
#endif
/*!
    Returns the transformed rectangle \a rect as a polygon.

    Polygons and rectangles behave slightly differently
    when transformed (due to integer rounding), so
    \c{matrix.map(QPolygon(rect))} is not always the same as
    \c{matrix.mapToPolygon(rect)}.
*/
QPolygon QMatrix::mapToPolygon(const QRect &rect) const
{
    QPolygon a(4);
    qreal x[4], y[4];
    if (_m12 == 0.0F && _m21 == 0.0F) {
        x[0] = _m11*rect.x() + _dx;
        y[0] = _m22*rect.y() + _dy;
        qreal w = _m11*rect.width();
        qreal h = _m22*rect.height();
        if (w < 0) {
            w = -w;
            x[0] -= w;
        }
        if (h < 0) {
            h = -h;
            y[0] -= h;
        }
        x[1] = x[0]+w;
        x[2] = x[1];
        x[3] = x[0];
        y[1] = y[0];
        y[2] = y[0]+h;
        y[3] = y[2];
    } else {
        qreal right = rect.x() + rect.width();
        qreal bottom = rect.y() + rect.height();
        MAPDOUBLE(rect.x(), rect.y(), x[0], y[0]);
        MAPDOUBLE(right, rect.y(), x[1], y[1]);
        MAPDOUBLE(right, bottom, x[2], y[2]);
        MAPDOUBLE(rect.x(), bottom, x[3], y[3]);
    }
#if 0
    int i;
    for(i = 0; i< 4; i++)
        qDebug("coords(%d) = (%f/%f) (%d/%d)", i, x[i], y[i], qRound(x[i]), qRound(y[i]));
    qDebug("width=%f, height=%f", sqrt((x[1]-x[0])*(x[1]-x[0]) + (y[1]-y[0])*(y[1]-y[0])),
            sqrt((x[0]-x[3])*(x[0]-x[3]) + (y[0]-y[3])*(y[0]-y[3])));
#endif
    // all coordinates are correctly, tranform to a pointarray
    // (rounding to the next integer)
    a.setPoints(4, qRound(x[0]), qRound(y[0]),
                 qRound(x[1]), qRound(y[1]),
                 qRound(x[2]), qRound(y[2]),
                 qRound(x[3]), qRound(y[3]));
    return a;
}

/*!
    Resets the matrix to an identity matrix.

    All elements are set to zero, except \e m11 and \e m22 (scaling)
    which are set to 1.

    \sa isIdentity()
*/

void QMatrix::reset()
{
    _m11 = _m22 = 1.0;
    _m12 = _m21 = _dx = _dy = 0.0;
}

/*!
  \fn bool QMatrix::isIdentity() const

    Returns true if the matrix is the identity matrix; otherwise returns false.

    \sa reset()
*/

/*!
    Moves the coordinate system \a dx along the x axis and \a dy along
    the y axis.

    Returns a reference to the matrix.

    \sa scale(), shear(), rotate()
*/

QMatrix &QMatrix::translate(qreal dx, qreal dy)
{
    _dx += dx*_m11 + dy*_m21;
    _dy += dy*_m22 + dx*_m12;
    return *this;
}

/*!
    Scales the coordinate system unit by \a sx horizontally and \a sy
    vertically.

    Returns a reference to the matrix.

    \sa translate(), shear(), rotate()
*/

QMatrix &QMatrix::scale(qreal sx, qreal sy)
{
    _m11 *= sx;
    _m12 *= sx;
    _m21 *= sy;
    _m22 *= sy;
    return *this;
}

/*!
    Shears the coordinate system by \a sh horizontally and \a sv
    vertically.

    Returns a reference to the matrix.

    \sa translate(), scale(), rotate()
*/

QMatrix &QMatrix::shear(qreal sh, qreal sv)
{
    qreal tm11 = sv*_m21;
    qreal tm12 = sv*_m22;
    qreal tm21 = sh*_m11;
    qreal tm22 = sh*_m12;
    _m11 += tm11;
    _m12 += tm12;
    _m21 += tm21;
    _m22 += tm22;
    return *this;
}

const qreal deg2rad = qreal(0.017453292519943295769);        // pi/180

/*!
    Rotates the coordinate system \a a degrees counterclockwise.

    Returns a reference to the matrix.

    \sa translate(), scale(), shear()
*/

QMatrix &QMatrix::rotate(qreal a)
{
    qreal b = deg2rad*a;                        // convert to radians
    qreal sina = sin(b);                // fast and convenient
    qreal cosa = cos(b);
    qreal tm11 = cosa*_m11 + sina*_m21;
    qreal tm12 = cosa*_m12 + sina*_m22;
    qreal tm21 = -sina*_m11 + cosa*_m21;
    qreal tm22 = -sina*_m12 + cosa*_m22;
    _m11 = tm11; _m12 = tm12;
    _m21 = tm21; _m22 = tm22;
    return *this;
}

/*!
    \fn bool QMatrix::isInvertible() const

    Returns true if the matrix is invertible; otherwise returns false.

    \sa inverted()
*/

/*!
    \fn qreal QMatrix::det() const

    Returns the matrix's determinant.
*/

/*!
    \fn QMatrix QMatrix::invert(bool *invertible)

    Call inverted(\a invertible) instead.
*/

/*!
    Returns the inverted matrix.

    If the matrix is singular (not invertible), the identity matrix is
    returned.

    If \a invertible is not 0: the value of \c{*}\a{invertible} is set
    to true if the matrix is invertible; otherwise \c{*}\a{invertible}
    is set to false.

    \sa isInvertible()
*/

QMatrix QMatrix::inverted(bool *invertible) const
{
    qreal determinant = det();
    if (determinant == 0.0) {
        if (invertible)
            *invertible = false;                // singular matrix
        QMatrix defaultMatrix;
        return defaultMatrix;
    }
    else {                                        // invertible matrix
        if (invertible)
            *invertible = true;
        qreal dinv = 1.0/determinant;
        QMatrix imatrix((_m22*dinv),        (-_m12*dinv),
                          (-_m21*dinv), (_m11*dinv),
                          ((_m21*_dy - _m22*_dx)*dinv),
                          ((_m12*_dx - _m11*_dy)*dinv));
        return imatrix;
    }
}


/*!
    Returns true if this matrix is equal to \a m; otherwise returns false.
*/

bool QMatrix::operator==(const QMatrix &m) const
{
    return _m11 == m._m11 &&
           _m12 == m._m12 &&
           _m21 == m._m21 &&
           _m22 == m._m22 &&
           _dx == m._dx &&
           _dy == m._dy;
}

/*!
    Returns true if this matrix is not equal to \a m; otherwise returns false.
*/

bool QMatrix::operator!=(const QMatrix &m) const
{
    return _m11 != m._m11 ||
           _m12 != m._m12 ||
           _m21 != m._m21 ||
           _m22 != m._m22 ||
           _dx != m._dx ||
           _dy != m._dy;
}

/*!
    Returns the result of multiplying this matrix by matrix \a m.
*/

QMatrix &QMatrix::operator *=(const QMatrix &m)
{
    qreal tm11 = _m11*m._m11 + _m12*m._m21;
    qreal tm12 = _m11*m._m12 + _m12*m._m22;
    qreal tm21 = _m21*m._m11 + _m22*m._m21;
    qreal tm22 = _m21*m._m12 + _m22*m._m22;

    qreal tdx  = _dx*m._m11  + _dy*m._m21 + m._dx;
    qreal tdy =  _dx*m._m12  + _dy*m._m22 + m._dy;

    _m11 = tm11; _m12 = tm12;
    _m21 = tm21; _m22 = tm22;
    _dx = tdx; _dy = tdy;
    return *this;
}

/*!
    \overload

    Returns the product of \e this * \a m.

    Note that matrix multiplication is not commutative, i.e. a*b !=
    b*a.
*/

QMatrix QMatrix::operator *(const QMatrix &m) const
{
    QMatrix result = *this;
    result *= m;
    return result;
}

/*!
    Assigns matrix \a matrix's values to this matrix.
*/
QMatrix &QMatrix::operator=(const QMatrix &matrix)
{
    _m11 = matrix._m11;
    _m12 = matrix._m12;
    _m21 = matrix._m21;
    _m22 = matrix._m22;
    _dx  = matrix._dx;
    _dy  = matrix._dy;
    return *this;
}

Q_GUI_EXPORT QPainterPath operator *(const QPainterPath &p, const QMatrix &m)
{
    return m.map(p);
}


/*****************************************************************************
  QMatrix stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QMatrix

    Writes the matrix \a m to the stream \a s and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QMatrix &m)
{
    if (s.version() == 1) {
        s << (float)m.m11() << (float)m.m12() << (float)m.m21()
          << (float)m.m22() << (float)m.dx()  << (float)m.dy();
    } else {
        s << m.m11()
          << m.m12()
          << m.m21()
          << m.m22()
          << m.dx()
          << m.dy();
    }
    return s;
}

/*!
    \relates QMatrix

    Reads the matrix \a m from the stream \a s and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QMatrix &m)
{
    if (s.version() == 1) {
        float m11, m12, m21, m22, dx, dy;
        s >> m11;  s >> m12;  s >> m21;  s >> m22;
        s >> dx;   s >> dy;
        m.setMatrix(m11, m12, m21, m22, dx, dy);
    }
    else {
        double m11, m12, m21, m22, dx, dy;
        s >> m11;
        s >> m12;
        s >> m21;
        s >> m22;
        s >> dx;
        s >> dy;
        m.setMatrix(m11, m12, m21, m22, dx, dy);
    }
    return s;
}
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QMatrix &m)
{
    dbg.nospace() << "QMatrix("
                  << "11=" << m.m11()
                  << " 12=" << m.m12()
                  << " 21=" << m.m21()
                  << " 22=" << m.m22()
                  << " dx=" << m.dx()
                  << " dy=" << m.dy()
                  << ")";
    return dbg.space();
}
#endif

/*!
    \compat

    \fn QRect QMatrix::map(const QRect &rect) const

    Use mapRect() instead.
*/

#endif // QT_NO_WMATRIX

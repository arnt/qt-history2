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
    \code
        MyWidget::paintEvent(QPaintEvent *)
        {
            QPainter p;                   // our painter
            QMatrix m;                   // our transformation matrix
            m.rotate(22.5);               // rotated coordinate system
            p.begin(this);                // start painting
            p.setMatrix(m);          // use rotated coordinate system
            p.drawText(30,20, "detator"); // draw rotated text at 30,20
            p.end();                      // painting done
        }
    \endcode

    A matrix specifies how to translate, scale, shear or rotate the
    graphics; the actual transformation is performed by the drawing
    routines in QPainter and by QPixmap::xForm().

    The QMatrix class contains a 3x3 matrix of the form:
    <table align=center border=1 cellpadding=1 cellspacing=0>
    <tr align=center><td>m11</td><td>m12</td><td>&nbsp;0 </td></tr>
    <tr align=center><td>m21</td><td>m22</td><td>&nbsp;0 </td></tr>
    <tr align=center><td>dx</td> <td>dy</td> <td>&nbsp;1 </td></tr>
    </table>

    A matrix transforms a point in the plane to another point:
    \code
        x' = m11*x + m21*y + dx
        y' = m22*y + m12*x + dy
    \endcode

    The point \e (x, y) is the original point, and \e (x', y') is the
    transformed point. \e (x', y') can be transformed back to \e (x,
    y) by performing the same operation on the \link
    QMatrix::invert() inverted matrix\endlink.

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
    \code
        QMatrix m;           // identity matrix
        m.translate(10, -20); // first translate (10,-20)
        m.rotate(25);         // then rotate 25 degrees
        m.scale(1.2, 0.7);    // finally scale it
    \endcode

    Here's the same example using basic matrix operations:
    \code
        double a    = pi/180 * 25;         // convert 25 to radians
        double sina = sin(a);
        double cosa = cos(a);
        QMatrix m1(1, 0, 0, 1, 10, -20);  // translation matrix
        QMatrix m2(cosa, sina,            // rotation matrix
                    -sina, cosa, 0, 0);
        QMatrix m3(1.2, 0, 0, 0.7, 0, 0); // scaling matrix
        QMatrix m;
        m = m3 * m2 * m1;                  // combine all transformations
    \endcode

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
    double fx = x; \
    double fy = y; \
    nx = _m11*fx + _m21*fy + _dx; \
    ny = _m12*fx + _m22*fy + _dy; \
}

#define MAPFLOAT(x, y, nx, ny) \
{ \
    float fx = x; \
    float fy = y;                \
    nx = _m11*fx + _m21*fy + _dx; \
    ny = _m12*fx + _m22*fy + _dy; \
}

#define MAPINT(x, y, nx, ny) \
{ \
    double fx = x; \
    double fy = y; \
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

QMatrix::QMatrix(double m11, double m12, double m21, double m22,
                    double dx, double dy)
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

void QMatrix::setMatrix(double m11, double m12, double m21, double m22,
                          double dx, double dy)
{
    _m11 = m11;         _m12 = m12;
    _m21 = m21;         _m22 = m22;
    _dx         = dx;         _dy  = dy;
}


/*!
    \fn double QMatrix::m11() const

    Returns the X scaling factor.
*/

/*!
    \fn double QMatrix::m12() const

    Returns the vertical shearing factor.
*/

/*!
    \fn double QMatrix::m21() const

    Returns the horizontal shearing factor.
*/

/*!
    \fn double QMatrix::m22() const

    Returns the Y scaling factor.
*/

/*!
    \fn double QMatrix::dx() const

    Returns the horizontal translation.
*/

/*!
    \fn double QMatrix::dy() const

    Returns the vertical translation.
*/


/*!
    \overload

    Transforms (\a{x}, \a{y}) to (\c{*}\a{tx}, \c{*}\a{ty}) using the
    following formulas:

    \code
        *tx = m11*x + m21*y + dx
        *ty = m22*y + m12*x + dy
    \endcode
*/

void QMatrix::map(double x, double y, double *tx, double *ty) const
{
    MAPDOUBLE(x, y, *tx, *ty);
}

/*!
    \overload

    Transforms (\a{x}, \a{y}) to (\c{*}\a{tx}, \c{*}\a{ty}) using the
    following formulas:

    \code
        *tx = m11*x + m21*y + dx
        *ty = m22*y + m12*x + dy
    \endcode
*/

void QMatrix::map(float x, float y, float *tx, float *ty) const
{
    MAPFLOAT(x, y, *tx, *ty);
}


/*!
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
    Returns the transformed rectangle \a rect.

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
            x -= w-1;
        }
        if (h < 0) {
            h = -h;
            y -= h-1;
        }
        result = QRect(x, y, w, h);
    } else {
        // see mapToPolygon for explanations of the algorithm.
        double x0, y0;
        double x, y;
        MAPDOUBLE(rect.left(), rect.top(), x0, y0);
        double xmin = x0;
        double ymin = y0;
        double xmax = x0;
        double ymax = y0;
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
        double w = xmax - xmin;
        double h = ymax - ymin;
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
        float x = _m11*rect.x() + _dx;
        float y = _m22*rect.y() + _dy;
        float w = _m11*rect.width();
        float h = _m22*rect.height();
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
        double x0, y0;
        double x, y;
        MAPDOUBLE(rect.x(), rect.y(), x0, y0);
        double xmin = x0;
        double ymin = y0;
        double xmax = x0;
        double ymax = y0;
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
    double fx = p.x();
    double fy = p.y();
    return QPoint(qRound(_m11*fx + _m21*fy + _dx),
                   qRound(_m12*fx + _m22*fy + _dy));
}

/*!
  \fn QPointF operator*(const QPointF &p, const QMatrix &m)

  \relates QMatrix

  Same as m.map(p).
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
    double fx = point.x();
    double fy = point.y();
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
    return QLineF(map(line.start()), map(line.end()));
}

/*!
    \fn QPointArray operator*(const QPointArray &a, const QMatrix &m)

    \relates QMatrix

    This is the same as \a{m}.mapRect(\a{a}).
*/

/*!
    \overload

    Returns the point array \a a transformed by calling map for each point.
*/
QPointArray QMatrix::map(const QPointArray &a) const
{
    int size = a.size();
    int i;
    QPolygon p(size);
    const QPoint *da = a.constData();
    QPointF *dp = p.data();
    float xmin = (float)INT_MAX;
    float ymin = xmin;
    float xmax = (float)INT_MIN;
    float ymax = xmax;
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
    float w = qMax(xmax - xmin, 1.);
    float h = qMax(ymax - ymin, 1.);
    for(i = 0; i < size; i++) {
        dp[i].xp += (dp[i].xp - xmin)/w;
        dp[i].yp += (dp[i].yp - ymin)/h;
        MAPDOUBLE(dp[i].xp, dp[i].yp, dp[i].xp, dp[i].yp);
    }

    // now apply correction back for transformed values...
    xmin = (float)INT_MAX;
    ymin = xmin;
    xmax = (float)INT_MIN;
    ymax = xmax;
    for(i = 0; i < size; i++) {
        xmin = qMin(xmin, dp[i].xp);
        ymin = qMin(ymin, dp[i].yp);
        xmax = qMax(xmax, dp[i].xp);
        ymax = qMax(ymax, dp[i].yp);
    }
    w = qMax(xmax - xmin, 1.);
    h = qMax(ymax - ymin, 1.);

    QPointArray result(size);
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
QPolygon QMatrix::map(const QPolygon &a) const
{
    int size = a.size();
    int i;
    QPolygon p(size);
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
        copy.translate((int)_dx, (int)_dy);
        return copy;
    }

    QVector<QRect> rects = r.rects();
    QRegion result;
    register QRect *rect = rects.data();
    register int i = rects.size();
    if (m12() == 0.0F && m21() == 0.0F) {
        // simple case, no rotation
        while (i) {
            int x = qRound(m11()*rect->x() + dx());
            int y = qRound(m22()*rect->y() + dy());
            int w = qRound(m11()*rect->width());
            int h = qRound(m22()*rect->height());
            if (w < 0) {
                w = -w;
                x -= w-1;
            }
            if (h < 0) {
                h = -h;
                y -= h-1;
            }
            *rect = QRect(x, y, w, h);
            rect++;
            i--;
        }
        result.setRects(rects.data(), rects.size());
    } else {
        while (i) {
            result |= mapToRegion(*rect);
            rect++;
            i--;
        }
    }

    return result;
}

/*
    \overload

    Transforms the painter path \a path.
*/
QPainterPath QMatrix::map(const QPainterPath &path) const
{
    QPainterPath copy = path;

    for (int i=0; i<path.elementCount(); ++i) {
        QPainterPath::Element &e = copy.elements[i];
        float fx = e.x, fy = e.y;
        e.x = _m11*fx + _m21*fy + _dx;
        e.y =  _m12*fx + _m22*fy + _dy;
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

/*!
    Returns the transformed rectangle \a rect as a polygon.

    Polygons and rectangles behave slightly differently
    when transformed (due to integer rounding), so
    \c{matrix.map(QPointArray(rect))} is not always the same as
    \c{matrix.mapToPolygon(rect)}.
*/
QPointArray QMatrix::mapToPolygon(const QRect &rect) const
{
    QPointArray a(4);
    double x[4], y[4];
    if (_m12 == 0.0F && _m21 == 0.0F) {
        x[0] = qRound(_m11*rect.x() + _dx);
        y[0] = qRound(_m22*rect.y() + _dy);
        double w = qRound(_m11*rect.width());
        double h = qRound(_m22*rect.height());
        if (w < 0) {
            w = -w;
            x[0] -= w - 1.;
        }
        if (h < 0) {
            h = -h;
            y[0] -= h - 1.;
        }
        x[1] = x[0]+w-1;
        x[2] = x[1];
        x[3] = x[0];
        y[1] = y[0];
        y[2] = y[0]+h-1;
        y[3] = y[2];
    } else {
        MAPINT(rect.left(), rect.top(), x[0], y[0]);
        MAPINT(rect.right() + 1, rect.top(), x[1], y[1]);
        MAPINT(rect.right() + 1, rect.bottom() + 1, x[2], y[2]);
        MAPINT(rect.left(), rect.bottom() + 1, x[3], y[3]);

        /*
        Including rectangles as we have are evil.

        We now have a rectangle that is one pixel to wide and one to
        high. the tranformed position of the top-left corner is
        correct. All other points need some adjustments.

        Doing this mathematically exact would force us to calculate some square roots,
        something we don't want for the sake of speed.

        Instead we use an approximation, that converts to the correct
        answer when m12 -> 0 and m21 -> 0, and accept smaller
        errors in the general transformation case.

        The solution is to calculate the width and height of the
        bounding rect, and scale the points 1/2/3 by (xp-x0)/xw pixel direction
        to point 0.
        */

        double xmin = x[0];
        double ymin = y[0];
        double xmax = x[0];
        double ymax = y[0];
        int i;
        for(i = 1; i< 4; i++) {
            xmin = qMin(xmin, x[i]);
            ymin = qMin(ymin, y[i]);
            xmax = qMax(xmax, x[i]);
            ymax = qMax(ymax, y[i]);
        }
        double w = xmax - xmin;
        double h = ymax - ymin;

        for(i = 1; i < 4; i++) {
            x[i] -= (x[i] - x[0])/w;
            y[i] -= (y[i] - y[0])/h;
        }
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

QMatrix &QMatrix::translate(double dx, double dy)
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

QMatrix &QMatrix::scale(double sx, double sy)
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

QMatrix &QMatrix::shear(double sh, double sv)
{
    double tm11 = sv*_m21;
    double tm12 = sv*_m22;
    double tm21 = sh*_m11;
    double tm22 = sh*_m12;
    _m11 += tm11;
    _m12 += tm12;
    _m21 += tm21;
    _m22 += tm22;
    return *this;
}

const double deg2rad = 0.017453292519943295769;        // pi/180

/*!
    Rotates the coordinate system \a a degrees counterclockwise.

    Returns a reference to the matrix.

    \sa translate(), scale(), shear()
*/

QMatrix &QMatrix::rotate(double a)
{
    double b = deg2rad*a;                        // convert to radians
    double sina = qSin(b);                // fast and convenient
    double cosa = qCos(b);
    double tm11 = cosa*_m11 + sina*_m21;
    double tm12 = cosa*_m12 + sina*_m22;
    double tm21 = -sina*_m11 + cosa*_m21;
    double tm22 = -sina*_m12 + cosa*_m22;
    _m11 = tm11; _m12 = tm12;
    _m21 = tm21; _m22 = tm22;
    return *this;
}

/*!
    \fn bool QMatrix::isInvertible() const

    Returns true if the matrix is invertible; otherwise returns false.

    \sa invert()
*/

/*!
    \fn double QMatrix::det() const

    Returns the matrix's determinant.
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

QMatrix QMatrix::invert(bool *invertible) const
{
    double determinant = det();
    if (determinant == 0.0) {
        if (invertible)
            *invertible = false;                // singular matrix
        QMatrix defaultMatrix;
        return defaultMatrix;
    }
    else {                                        // invertible matrix
        if (invertible)
            *invertible = true;
        double dinv = 1.0/determinant;
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
    double tm11 = _m11*m._m11 + _m12*m._m21;
    double tm12 = _m11*m._m12 + _m12*m._m22;
    double tm21 = _m21*m._m11 + _m22*m._m21;
    double tm22 = _m21*m._m12 + _m22*m._m22;

    double tdx  = _dx*m._m11  + _dy*m._m21 + m._dx;
    double tdy =  _dx*m._m12  + _dy*m._m22 + m._dy;

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
    if (s.version() == 1)
        s << (float)m.m11() << (float)m.m12() << (float)m.m21()
          << (float)m.m22() << (float)m.dx()  << (float)m.dy();
    else
        s << m.m11() << m.m12() << m.m21() << m.m22()
          << m.dx() << m.dy();
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
        s >> m11;  s >> m12;  s >> m21;  s >> m22;
        s >> dx;   s >> dy;
        m.setMatrix(m11, m12, m21, m22, dx, dy);
    }
    return s;
}
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG
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
    \fn QRect QMatrix::map(const QRect &rect) const

    Use mapRect() instead.
*/

#endif // QT_NO_WMATRIX

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

#include "qwmatrix.h"

#ifndef QT_NO_WMATRIX

/*!
    \class QWMatrix qwmatrix.h
    \brief The QWMatrix class specifies 2D transformations of a
    coordinate system.

    \ingroup multimedia

    The standard coordinate system of a \link QPaintDevice paint
    device\endlink has the origin located at the top-left position. \e
    x values increase to the right; \e y values increase downward.

    This coordinate system is the default for the QPainter, which
    renders graphics in a paint device. A user-defined coordinate
    system can be specified by setting a QWMatrix for the painter.

    Example:
    \code
        MyWidget::paintEvent(QPaintEvent *)
        {
            QPainter p;                   // our painter
            QWMatrix m;                   // our transformation matrix
            m.rotate(22.5);               // rotated coordinate system
            p.begin(this);                // start painting
            p.setWorldMatrix(m);          // use rotated coordinate system
            p.drawText(30,20, "detator"); // draw rotated text at 30,20
            p.end();                      // painting done
        }
    \endcode

    A matrix specifies how to translate, scale, shear or rotate the
    graphics; the actual transformation is performed by the drawing
    routines in QPainter and by QPixmap::xForm().

    The QWMatrix class contains a 3x3 matrix of the form:
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
    QWMatrix::invert() inverted matrix\endlink.

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
    factors and the scaling factors. The QWMatrix also has a function
    that sets \link rotate() rotation \endlink directly.

    QWMatrix lets you combine transformations like this:
    \code
        QWMatrix m;           // identity matrix
        m.translate(10, -20); // first translate (10,-20)
        m.rotate(25);         // then rotate 25 degrees
        m.scale(1.2, 0.7);    // finally scale it
    \endcode

    Here's the same example using basic matrix operations:
    \code
        double a    = pi/180 * 25;         // convert 25 to radians
        double sina = sin(a);
        double cosa = cos(a);
        QWMatrix m1(1, 0, 0, 1, 10, -20);  // translation matrix
        QWMatrix m2(cosa, sina,            // rotation matrix
                    -sina, cosa, 0, 0);
        QWMatrix m3(1.2, 0, 0, 0.7, 0, 0); // scaling matrix
        QWMatrix m;
        m = m3 * m2 * m1;                  // combine all transformations
    \endcode

    The matrix can also be transformed using the map() functions, and
    transformed points, rectangles, etc., can be obtained using map(),
    mapRect(), mapToRegion(), and mapToPolygon() functions.

    \l QPainter has functions to translate, scale, shear and rotate the
    coordinate system without using a QWMatrix. Although these
    functions are very convenient, it can be more efficient to build a
    QWMatrix and call QPainter::setWorldMatrix() if you want to perform
    more than a single transform operation.

    \sa QPainter::setWorldMatrix(), QPixmap::xForm()
*/


/*****************************************************************************
  QWMatrix member functions
 *****************************************************************************/

/*!
  \fn QWMatrix::QWMatrix()

    Constructs an identity matrix. All elements are set to zero except
    \e m11 and \e m22 (scaling), which are set to 1.
*/

/*!
  \fn QWMatrix::QWMatrix(double m11, double m12, double m21, double m22,
                    double dx, double dy)

    Constructs a matrix with the elements, \a m11, \a m12, \a m21, \a
    m22, \a dx and \a dy.
*/


/*!
  \fn QWMatrix::QWMatrix(const QWMatrix &matrix)

  Constructs a matrix with the elements from the matrix \a matrix
 */




/*!
  \fn QWMatrix &QWMatrix::translate(double dx, double dy)

    Moves the coordinate system \a dx along the X-axis and \a dy along
    the Y-axis.

    Returns a reference to the matrix.

    \sa scale(), shear(), rotate()
*/


/*!
  \fn QWMatrix &QWMatrix::scale(double sx, double sy)

    Scales the coordinate system unit by \a sx horizontally and \a sy
    vertically.

    Returns a reference to the matrix.

    \sa translate(), shear(), rotate()
*/

/*!
  \fn QWMatrix &QWMatrix::shear(double sh, double sv)

    Shears the coordinate system by \a sh horizontally and \a sv
    vertically.

    Returns a reference to the matrix.

    \sa translate(), scale(), rotate()
*/


/*!
  \fn QWMatrix &QWMatrix::rotate(double a)

    Rotates the coordinate system \a a degrees counterclockwise.

    Returns a reference to the matrix.

    \sa translate(), scale(), shear()
*/


#endif // QT_NO_WMATRIX


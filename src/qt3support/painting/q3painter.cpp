/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3painter.h"
#include "qpaintengine.h"

#include <private/qpainter_p.h>

/*!
    \class Q3Painter q3painter.h
    \brief The Q3Painter class is a Qt 3 compatibility wrapper for QPainter.

    \compat

    Prior to Qt 4, QPainter specialized the pen drawing for rectangle
    based functions (in particular: drawRect, drawEllipse,
    drawRoundRect, drawArc, drawChord and drawPie). When stroking a
    rectangle of width 10, the pen would draw a rectangle of width 10.
    Drawing a polygon defined by the corner points of the same
    rectangle the stroke would have a width of 11.

    The reason for this is best explained using the picture below:

    \img q3painter_rationale.png

    As we can see, stroking the rectangle so it gets a width of 10,
    means the pen is drawn on a rectangle on width 9. The polygon,
    however follows a consistent model.

    In Qt 4, all rectangle based functions have changed to follow the
    polygon approach, which means that the rectangle defines the size of
    the fill, and the pen follows the edges of the shape. For pen widths
    of 0 and 1 this means that the stroke will be inside the shape on the
    left and the top and outside on the bottom and right.

    The reason for the change in Qt 4 is so that we provide consistency
    for all drawing functions even with complex transformations.
*/

/*!
    \fn Q3Painter::Q3Painter()

    Constructs a Q3Painter.
*/

/*!
    \fn Q3Painter::Q3Painter(QPaintDevice *pdev)

    Constructs a Q3Painter that operates on device \a pdev.
*/

/*!
    \internal
*/
QRect Q3Painter::adjustedRectangle(const QRect &r)
{
    QRect rect = r.normalized();
    int subtract = d_func()->rectSubtraction();
    if (subtract != 0)
        rect.setSize(QSize(rect.width() - subtract, rect.height() - subtract));
    return rect;
}


/*!
    \fn void Q3Painter::drawRect(int x, int y, int w, int h)

    \overload

    Draws the rectangle that fits inside the bounds specified by \a x,
    \a y, \a w and \a h using the current pen and brush.
*/

/*!
    \fn void Q3Painter::drawRect(const QRect &r)

    Draws a rectangle that fits inside the rectangle \a r using the
    current pen and brush.

*/



/*!
    \fn Q3Painter::drawEllipse(const QRect &r)

    Draws the ellipse that fits inside the bounds \a r using the
    current pen and brush.

*/

/*!
    \fn Q3Painter::drawEllipse(int x, int y, int width, int height)

    \overload

    Draws an ellipse that fits inside the bounds specified by \a x,
    \a y, \a width and \a height using the current pen and brush.

*/

/*!
    \fn void Q3Painter::drawPie(int x, int y, int w, int h, int
    startAngle, int spanAngle)

    \overload

    Draws a pie segment that fits inside the bounds (\a{x}, \a{y},
    \a{w}, \a{h}) with the given \a startAngle and \a spanAngle.
*/

/*!
    \fn void Q3Painter::drawPie(const QRect &r, int a, int alen)

    Draws a pie defined by the rectangle \a r, the start angle \a a
    and the arc length \a alen.

    The pie is filled with the current brush().

    The angles \a a and \a alen are 1/16th of a degree, i.e. a full
    circle equals 5760 (16*360). Positive values of \a a and \a alen
    mean counter-clockwise while negative values mean the clockwise
    direction. Zero degrees is at the 3 o'clock position.

    \sa drawArc(), drawChord()
*/

/*!
    \fn void Q3Painter::drawArc(int x, int y, int w, int h, int
    startAngle, int spanAngle)

    \overload

    Draws the arc that fits inside the rectangle (\a{x}, \a{y}, \a{w},
    \a{h}), with the given \a startAngle and \a spanAngle.
*/

/*!
    \fn void Q3Painter::drawArc(const QRect &r, int a, int alen)

    Draws an arc defined by the rectangle \a r, the start angle \a a
    and the arc length \a alen.

    The angles \a a and \a alen are 1/16th of a degree, i.e. a full
    circle equals 5760 (16*360). Positive values of \a a and \a alen
    mean counter-clockwise while negative values mean the clockwise
    direction. Zero degrees is at the 3 o'clock position.

    Example:
    \code
        QPainter p(myWidget);
        p.drawArc(QRect(10,10, 70,100), 100*16, 160*16); // draws a "(" arc
    \endcode

    \sa drawPie(), drawChord()
*/

/*!
    \fn void Q3Painter::drawChord(int x, int y, int w, int h, int
    startAngle, int spanAngle)

    \overload

    Draws a chord that fits inside the rectangle (\a{x}, \a{y}, \a{w},
    \a{h}) with the given \a startAngle and \a spanAngle.
*/


/*!
    \fn void Q3Painter::drawChord(const QRect &r, int a, int alen)

    Draws a chord defined by the rectangle \a r, the start angle \a a
    and the arc length \a alen.

    The chord is filled with the current brush().

    The angles \a a and \a alen are 1/16th of a degree, i.e. a full
    circle equals 5760 (16*360). Positive values of \a a and \a alen
    mean counter-clockwise while negative values mean the clockwise
    direction. Zero degrees is at the 3 o'clock position.

    \sa drawArc(), drawPie()
*/

/*!
    \fn void Q3Painter::drawRoundRect(const QRect &r, int xrnd, int yrnd)

    Draws a rounded rect that fits into the bounds \a r using the current
    pen and brush. The parameters \a xrnd and \a yrnd specifies the roundness
    in x and y direction.
*/

/*!
    \fn void Q3Painter::drawRoundRect(int x, int y, int w, int h, int xrnd, int yrnd)

    \overload

    Draws a rounded rect that fits into the bounds \a x, \a y, \a w
    and \a h using the current pen and brush. The parameters \a xrnd
    and \a yrnd specifies the roundness in x and y direction.
*/

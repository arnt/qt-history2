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

#include "qrect.h"
#include "qdatastream.h"
#include "qdebug.h"

/*!
    \class QRect
    \brief The QRect class defines a rectangle in the plane.

    \ingroup multimedia

    A rectangle is internally represented as an upper-left corner and
    a bottom-right corner, but it is normally expressed as an
    upper-left corner and a size.

    Note that the size (width and height) of a rectangle might be
    different from what you are used to. If the top-left corner and
    the bottom-right corner are the same, the height and the width of
    the rectangle will both be 1.

    Generally, \e{width = right - left + 1} and \e{height = bottom -
    top + 1}. We designed it this way to make it correspond to
    rectangular spaces used by drawing functions in which the width
    and height denote a number of pixels. For example, drawing a
    rectangle with width and height 1 draws a single pixel.

    The default coordinate system has origin (0, 0) in the top-left
    corner. The positive direction of the y axis is down, and the
    positive x axis is from left to right.

    A QRect can be constructed with a set of left, top, width and
    height integers, from two \l{QPoint}s or from a QPoint and a QSize.
    After creation the dimensions can be changed, e.g. with setLeft(),
    setRight(), setTop() and setBottom(), or by setting sizes, e.g.
    setWidth(), setHeight() and setSize(). The dimensions can also be
    changed with the move functions, e.g. moveBy(), moveCenter(),
    moveBottomRight(), etc. You can also add coordinates to a
    rectangle with addCoords().

    You can test to see if a QRect contains a specific point with
    contains(). You can also test to see if two QRects intersect with
    intersects() (see also intersect()). To get the bounding rectangle
    of two QRects use unite().

    \sa QPoint, QSize
*/

/*****************************************************************************
  QRect member functions
 *****************************************************************************/

/*!
    \fn QRect::QRect()

    Constructs an invalid rectangle.
*/

/*!
  \fn QRect::QRect(const QPoint &topLeft, const QPoint &bottomRight)

    Constructs a rectangle with \a topLeft as the top-left corner and
    \a bottomRight as the bottom-right corner.
*/


/*!
  \fn QRect::QRect(const QPoint &topLeft, const QSize &size)

    Constructs a rectangle with \a topLeft as the top-left corner and
    \a size as the rectangle size.
*/


/*!
    \fn QRect::QRect(int left, int top, int width, int height)

    Constructs a rectangle with the \a top, \a left corner and \a
    width and \a height.

    Example (creates three identical rectangles):
    \code
        QRect r1(QPoint(100,200), QPoint(110,215));
        QRect r2(QPoint(100,200), QSize(11,16));
        QRect r3(100, 200, 11, 16);
    \endcode
*/


/*!
    \fn bool QRect::isNull() const

    Returns true if the rectangle is a null rectangle; otherwise
    returns false.

    A null rectangle has both the width and the height set to 0, that
    is right() == left() - 1 and bottom() == top() - 1.

    Note that if right() == left() and bottom() == top(), then the
    rectangle has width 1 and height 1.

    A null rectangle is also empty.

    A null rectangle is not valid.

    \sa isEmpty(), isValid()
*/

/*!
    \fn bool QRect::isEmpty() const

    Returns true if the rectangle is empty; otherwise returns false.

    An empty rectangle has a left() \> right() or top() \> bottom().

    An empty rectangle is not valid. \c{isEmpty() == !isValid()}

    \sa isNull(), isValid(), normalize()
*/

/*!
    \fn bool QRect::isValid() const

    Returns true if the rectangle is valid; otherwise returns false.

    A valid rectangle has a left() \<= right() and top() \<= bottom().

    Note that non-trivial operations like intersections are not defined
    for invalid rectangles.

    \c{isValid() == !isEmpty()}

    \sa isNull(), isEmpty(), normalize()
*/


/*!
    Returns a normalized rectangle, i.e. a rectangle that has a
    non-negative width and height.

    It swaps left and right if left() \> right(), and swaps top and
    bottom if top() \> bottom().

    \sa isValid()
*/

QRect QRect::normalize() const
{
    QRect r;
    if (x2 < x1) {                                // swap bad x values
        r.x1 = x2;
        r.x2 = x1;
    } else {
        r.x1 = x1;
        r.x2 = x2;
    }
    if (y2 < y1) {                                // swap bad y values
        r.y1 = y2;
        r.y2 = y1;
    } else {
        r.y1 = y1;
        r.y2 = y2;
    }
    return r;
}


/*!
    \fn int QRect::left() const

    Returns the left coordinate of the rectangle. Identical to x().

    \sa setLeft(), right(), topLeft(), bottomLeft()
*/

/*!
    \fn int QRect::top() const

    Returns the top coordinate of the rectangle. Identical to y().

    \sa setTop(), bottom(), topLeft(), topRight()
*/

/*!
    \fn int QRect::right() const

    Returns the right coordinate of the rectangle.

    \sa setRight(), left(), topRight(), bottomRight()
*/

/*!
    \fn int QRect::bottom() const

    Returns the bottom coordinate of the rectangle.

    \sa setBottom(), top(), bottomLeft(), bottomRight()
*/

/*!
    \fn int &QRect::rLeft()

    Returns a reference to the left coordinate of the rectangle.

    \sa rTop(), rRight(), rBottom()
*/

/*!
    \fn int &QRect::rTop()

    Returns a reference to the top coordinate of the rectangle.

    \sa rLeft(),  rRight(), rBottom()
*/

/*!
    \fn int &QRect::rRight()

    Returns a reference to the right coordinate of the rectangle.

    \sa rLeft(), rTop(), rBottom()
*/

/*!
    \fn int &QRect::rBottom()

    Returns a reference to the bottom coordinate of the rectangle.

    \sa rLeft(), rTop(), rRight()
*/

/*!
    \fn int QRect::x() const

    Returns the left coordinate of the rectangle. Identical to left().

    \sa left(), y(), setX()
*/

/*!
    \fn int QRect::y() const

    Returns the top coordinate of the rectangle. Identical to top().

    \sa top(), x(), setY()
*/

/*!
    \fn void QRect::setLeft(int pos)

    Sets the left edge of the rectangle to \a pos. May change the
    width, but will never change the right edge of the rectangle.

    Identical to setX().

    \sa left(), setTop(), setWidth()
*/

/*!
    \fn void QRect::setTop(int pos)

    Sets the top edge of the rectangle to \a pos. May change the
    height, but will never change the bottom edge of the rectangle.

    Identical to setY().

    \sa top(), setBottom(), setHeight()
*/

/*!
    \fn void QRect::setRight(int pos)

    Sets the right edge of the rectangle to \a pos. May change the
    width, but will never change the left edge of the rectangle.

    \sa right(), setLeft(), setWidth()
*/

/*!
    \fn void QRect::setBottom(int pos)

    Sets the bottom edge of the rectangle to \a pos. May change the
    height, but will never change the top edge of the rectangle.

    \sa bottom(), setTop(), setHeight()
*/

/*!
    \fn void QRect::setX(int x)

    Sets the x position of the rectangle (its left end) to \a x. May
    change the width, but will never change the right edge of the
    rectangle.

    Identical to setLeft().

    \sa x(), setY()
*/

/*!
    \fn void QRect::setY(int y)

    Sets the y position of the rectangle (its top) to \a y. May change
    the height, but will never change the bottom edge of the
    rectangle.

    Identical to setTop().

    \sa y(), setX()
*/

/*!
  \fn void QRect::setTopLeft(const QPoint &p)

    Set the top-left corner of the rectangle to \a p. May change
    the size, but will the never change the bottom-right corner of
    the rectangle.

    \sa topLeft(), moveTopLeft(), setBottomRight(), setTopRight(), setBottomLeft()
*/

/*!
  \fn void QRect::setBottomRight(const QPoint &p)

    Set the bottom-right corner of the rectangle to \a p. May change
    the size, but will the never change the top-left corner of
    the rectangle.

    \sa bottomRight(), moveBottomRight(), setTopLeft(), setTopRight(), setBottomLeft()
*/

/*!
  \fn void QRect::setTopRight(const QPoint &p)

    Set the top-right corner of the rectangle to \a p. May change
    the size, but will the never change the bottom-left corner of
    the rectangle.

    \sa topRight(), moveTopRight(), setTopLeft(), setBottomRight(), setBottomLeft()
*/

/*!
  \fn void QRect::setBottomLeft(const QPoint &p)

    Set the bottom-left corner of the rectangle to \a p. May change
    the size, but will the never change the top-right corner of
    the rectangle.

    \sa bottomLeft(), moveBottomLeft(), setTopLeft(), setBottomRight(), setTopRight()
*/

/*!
    \fn QPoint QRect::topLeft() const

    Returns the top-left position of the rectangle.

    \sa setTopLeft(), moveTopLeft(), bottomRight(), left(), top()
*/

/*!
    \fn QPoint QRect::bottomRight() const

    Returns the bottom-right position of the rectangle.

    \sa setBottomRight(), moveBottomRight(), topLeft(), right(), bottom()
*/

/*!
    \fn QPoint QRect::topRight() const

    Returns the top-right position of the rectangle.

    \sa setTopRight(), moveTopRight(), bottomLeft(), top(), right()
*/

/*!
    \fn QPoint QRect::bottomLeft() const

    Returns the bottom-left position of the rectangle.

    \sa setBottomLeft(), moveBottomLeft(), topRight(), bottom(), left()
*/

/*!
    \fn QPoint QRect::center() const

    Returns the center point of the rectangle.

    \sa moveCenter(), topLeft(), bottomRight(), topRight(), bottomLeft()
*/


/*!
  \fn void QRect::getRect(int *x, int *y, int *w, int *h) const

    Extracts the rectangle parameters as the position \c{*}\a{x},
    \c{*}\a{y} and width \c{*}\a{w} and height \c{*}\a{h}.

    \sa setRect(), coords()
*/


/*!
  \fn void QRect::getCoords(int *xp1, int *yp1, int *xp2, int *yp2) const

    Extracts the rectangle parameters as the top-left point
    \c{*}\a{xp1}, \c{*}\a{yp1} and the bottom-right point
    \c{*}\a{xp2}, \c{*}\a{yp2}.

    \sa setCoords(), rect()
*/

/*!
  \fn void QRect::rect(int *x, int *y, int *w, int *h) const

  Use getRect() instead.
*/


/*!
  \fn void QRect::coords(int *xp1, int *yp1, int *xp2, int *yp2) const

  Use getCoords() instead.

*/

/*!
  \fn void QRect::moveLeft(int pos)

    Sets the left position of the rectangle to \a pos, leaving the
    size unchanged.

    \sa left(), setLeft(), moveTop(), moveRight(), moveBottom()
*/

/*!
  \fn void QRect::moveTop(int pos)

    Sets the top position of the rectangle to \a pos, leaving the
    size unchanged.

    \sa top(), setTop(), moveLeft(), moveRight(), moveBottom()
*/


/*!
  \fn void QRect::moveRight(int pos)

    Sets the right position of the rectangle to \a pos, leaving the
    size unchanged.

    \sa right(), setRight(), moveLeft(), moveTop(), moveBottom()
*/


/*!
  \fn void QRect::moveBottom(int pos)

    Sets the bottom position of the rectangle to \a pos, leaving the
    size unchanged.

    \sa bottom(), setBottom(), moveLeft(), moveTop(), moveRight()
*/


/*!
  \fn void QRect::moveTopLeft(const QPoint &p)

    Sets the top-left position of the rectangle to \a p, leaving the
    size unchanged.

    \sa topLeft(), setTopLeft(), moveBottomRight(), moveTopRight(), moveBottomLeft()
*/


/*!
  \fn void QRect::moveBottomRight(const QPoint &p)

    Sets the bottom-right position of the rectangle to \a p, leaving
    the size unchanged.

    \sa bottomRight(), setBottomRight(), moveTopLeft(), moveTopRight(), moveBottomLeft()
*/


/*!
  \fn void QRect::moveTopRight(const QPoint &p)

    Sets the top-right position of the rectangle to \a p, leaving the
    size unchanged.

    \sa topRight(), setTopRight(), moveTopLeft(), moveBottomRight(), moveBottomLeft()
*/


/*!
  \fn void QRect::moveBottomLeft(const QPoint &p)

    Sets the bottom-left position of the rectangle to \a p, leaving
    the size unchanged.

    \sa bottomLeft(), setBottomLeft(), moveTopLeft(), moveBottomRight(), moveTopRight()
*/



/*!
    Sets the center point of the rectangle to \a p, leaving the size
    unchanged.

    \sa center(), moveTopLeft(), moveBottomRight(), moveTopRight(), moveBottomLeft()
*/

void QRect::moveCenter(const QPoint &p)
{
    int w = x2 - x1;
    int h = y2 - y1;
    x1 = p.x() - w/2;
    y1 = p.y() - h/2;
    x2 = x1 + w;
    y2 = y1 + h;
}

/*!
  \fn void QRect::moveBy(int dx, int dy)

  \obsolete

  \sa QRect::translate()
*/

/*!
  \fn void QRect::moveBy(const QPoint &)

  \obsolete

  \sa QRect::translate()
*/

/*!
  \fn void QRect::moveTo(int x, int y)

  Moves the top left corner of the rectangle to \a x and \a y, without
  changing the rectangles size.
*/

/*!
  \fn void QRect::moveTo(const QPoint &pt)

  Moves the top left corner of the rectangle to \a pt, without
  changing the rectangles size.
*/

/*!
  \fn void QRect::translate(int dx, int dy)

    Moves the rectangle \a dx along the x axis and \a dy along the y
    axis, relative to the current position. Positive values move the
    rectangle to the right and down.

    \sa moveTopLeft()
*/


/*!
    \fn void QRect::translate(const QPoint &p)

    \overload

    Moves the rectangle \a{p}\e{.x()} along the x axis and
    \a{p}\e{.y()} along the y axis, relative to the current position.
    Positive values move the rectangle to the right and down.

    \sa moveTopLeft()
*/


/*!
  \fn  void QRect::setRect(int x, int y, int w, int h)

  Sets the coordinates of the rectangle's top-left corner to (\a{x},
  \a{y}), and its size to (\a{w}, \a{h}).

  \sa rect(), setCoords()
*/


/*!
  \fn void QRect::setCoords(int xp1, int yp1, int xp2, int yp2)

    Sets the coordinates of the rectangle's top-left corner to
    (\a{xp1}, \a{yp1}), and the coordinates of its bottom-right corner to
    (\a{xp2}, \a{yp2}).

    \sa coords(), setRect()
*/


/*!
  \fn void QRect::addCoords(int xp1, int yp1, int xp2, int yp2)

    Adds \a xp1, \a yp1, \a xp2 and \a yp2 respectively to the
    existing coordinates of the rectangle.
*/

/*! \fn QRect QRect::adjusted(int xp1, int yp1, int xp2, int yp2) const

    Returns a new rectangle with \a xp1, \a yp1, \a xp2 and \a yp2
    added to the existing coordinates of this rectangle.
*/

/*!
    \fn QSize QRect::size() const

    Returns the size of the rectangle.

    \sa width(), height()
*/

/*!
    \fn int QRect::width() const

    Returns the width of the rectangle. The width includes both the
    left and right edges, i.e. width = right - left + 1.

    \sa height(), size(), setHeight()
*/

/*!
    \fn int QRect::height() const

    Returns the height of the rectangle. The height includes both the
    top and bottom edges, i.e. height = bottom - top + 1.

    \sa width(), size(), setHeight()
*/

/*!
  \fn void QRect::setWidth(int w)

    Sets the width of the rectangle to \a w. The right edge is
    changed, but not the left edge.

    \sa width(), setLeft(), setRight(), setSize()
*/


/*!
  \fn void QRect::setHeight(int h)

    Sets the height of the rectangle to \a h. The top edge is not
    moved, but the bottom edge may be moved.

    \sa height(), setTop(), setBottom(), setSize()
*/


/*!
  \fn void QRect::setSize(const QSize &s)

    Sets the size of the rectangle to \a s. The top-left corner is not
    moved.

    \sa size(), setWidth(), setHeight()
*/


/*!
  \fn bool QRect::contains(const QPoint &p, bool proper) const

    Returns true if the point \a p is inside or on the edge of the
    rectangle; otherwise returns false.

    If \a proper is true, this function returns true only if \a p is
    inside (not on the edge).
*/


/*!
    \fn bool QRect::contains(int x, int y, bool proper) const
    \overload

    Returns true if the point \a x, \a y is inside this rectangle;
    otherwise returns false.

    If \a proper is true, this function returns true only if the point
    is entirely inside (not on the edge).
*/

/*!
    \fn bool QRect::contains(int x, int y) const
    \overload

    Returns true if the point \a x, \a y is inside this rectangle;
    otherwise returns false.
*/

/*!
    \overload

    Returns true if the rectangle \a r is inside this rectangle;
    otherwise returns false.

    If \a proper is true, this function returns true only if \a r is
    entirely inside (not on the edge).

    \sa unite(), intersect(), intersects()
*/

bool QRect::contains(const QRect &r, bool proper) const
{
    if (proper)
        return r.x1 > x1 && r.x2 < x2 && r.y1 > y1 && r.y2 < y2;
    else
        return r.x1 >= x1 && r.x2 <= x2 && r.y1 >= y1 && r.y2 <= y2;
}

/*!
  \fn QRect& QRect::operator|=(const QRect &r)

    Unites this rectangle with rectangle \a r.
*/

/*!
  \fn QRect& QRect::operator&=(const QRect &r)

    Intersects this rectangle with rectangle \a r.
*/


/*!
    Returns the bounding rectangle of this rectangle and rectangle \a
    r.

    The bounding rectangle of a nonempty rectangle and an empty or
    invalid rectangle is defined to be the nonempty rectangle.

    \sa operator|=(), operator&(), intersects(), contains()
*/

QRect QRect::operator|(const QRect &r) const
{
    if (isValid()) {
        if (r.isValid()) {
            QRect tmp;
            tmp.setLeft(  qMin(x1, r.x1));
            tmp.setRight( qMax(x2, r.x2));
            tmp.setTop(          qMin(y1, r.y1));
            tmp.setBottom(qMax(y2, r.y2));
            return tmp;
        } else {
            return *this;
        }
    } else {
        return r;
    }
}

/*!
  \fn QRect QRect::unite(const QRect &r) const

    Returns the bounding rectangle of this rectangle and rectangle \a
    r. \c{r.unite(s)} is equivalent to \c{r|s}.
*/


/*!
    Returns the intersection of this rectangle and rectangle \a r.

    Returns an empty rectangle if there is no intersection.

    \sa operator&=(), operator|(), isEmpty(), intersects(), contains()
*/

QRect QRect::operator&(const QRect &r) const
{
    QRect tmp;
    tmp.x1 = qMax(x1, r.x1);
    tmp.x2 = qMin(x2, r.x2);
    tmp.y1 = qMax(y1, r.y1);
    tmp.y2 = qMin(y2, r.y2);
    return tmp;
}

/*!
  \fn QRect QRect::intersect(const QRect &r) const

    Returns the intersection of this rectangle and rectangle \a r.
    \c{r.intersect(s)} is equivalent to \c{r&s}.
*/

/*!
    Returns true if this rectangle intersects with rectangle \a r
    (there is at least one pixel that is within both rectangles);
    otherwise returns false.

    \sa intersect(), contains()
*/

bool QRect::intersects(const QRect &r) const
{
    return (qMax(x1, r.x1) <= qMin(x2, r.x2) &&
             qMax(y1, r.y1) <= qMin(y2, r.y2));
}


/*!
    \fn bool operator==(const QRect &r1, const QRect &r2)

    \relates QRect

    Returns true if the rectangles \a r1 and \a r2 are equal; otherwise returns
    false.
*/


/*!
    \fn bool operator!=(const QRect &r1, const QRect &r2)

    \relates QRect

    Returns true if the rectangles \a r1 and \a r2 are different; otherwise
    returns false.
*/


/*****************************************************************************
  QRect stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QRect &rectangle)

    \relates QRect

    Writes the \a rectangle to the \a stream, and returns a reference to the
    stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QRect &r)
{
    if (s.version() == 1)
        s << (Q_INT16)r.left() << (Q_INT16)r.top()
          << (Q_INT16)r.right() << (Q_INT16)r.bottom();
    else
        s << (Q_INT32)r.left() << (Q_INT32)r.top()
          << (Q_INT32)r.right() << (Q_INT32)r.bottom();
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QRect &rectangle)

    \relates QRect

    Reads a \a rectangle from the \a stream, and returns a reference to the
    stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QRect &r)
{
    if (s.version() == 1) {
        Q_INT16 x1, y1, x2, y2;
        s >> x1; s >> y1; s >> x2; s >> y2;
        r.setCoords(x1, y1, x2, y2);
    }
    else {
        Q_INT32 x1, y1, x2, y2;
        s >> x1; s >> y1; s >> x2; s >> y2;
        r.setCoords(x1, y1, x2, y2);
    }
    return s;
}

#endif // QT_NO_DATASTREAM


#ifndef QT_NO_DEBUG_OUTPUT
QDebug operator<<(QDebug dbg, const QRect &r) {
    dbg.nospace() << "QRect(" << r.x() << ',' << r.y() << ','
                  << r.width() << ',' << r.height() << ')';
    return dbg.space();
}
#endif

/*!
    \class QRectF
    \brief The QRectF class defines a rectangle in the plane using floating
    point coordinates for accuracy.

    \ingroup multimedia

    Rectangles are used throughout Qt to describe the area covered by visible
    items. They specify both the geometry of widgets and the extent of items
    that are painted using a QPainter. QRectF provides a variant of the QRect
    class that defines the position and dimension of the rectangle using
    \c float values for accuracy.

    A rectangle is internally represented as the positions of its upper-left
    and bottom-right corners, but it is normally expressed as the position of
    its upper-left corner and a size.

    Note that the size (width and height) of a rectangle might be
    different from what you expect. If the top-left corner and
    the bottom-right corner are the same, the height and the width of
    the rectangle will both be 1.

    Generally, \e{width = right - left + 1} and \e{height = bottom -
    top + 1}. We designed it this way to make it correspond to
    rectangular spaces used by drawing functions where the width
    and height denote a whole number of pixels. For example, drawing a
    rectangle with width and height 1 draws a single pixel.

    The default coordinate system has origin (0, 0) in the top-left
    corner. The positive direction of the x axis is from left to right,
    and the positive direction of the y axis is from top to bottom.

    A QRectF can be constructed with a set of floats specifying the
    coordinates of the top-left corner of the rectangle and its
    dimensions from two QPointF values, or from a QPointF and a QSizeF.
    After creation, the dimensions can be changed with setLeft(),
    setRight(), setTop(), and setBottom(), or by setting sizes with
    setWidth(), setHeight(), and setSize(). The dimensions can also be
    changed with the move functions, such as moveBy(), moveCenter(),
    and moveBottomRight(). You can also add coordinates to a rectangle
    with addCoords().

    You can test to see if a rectangle contains a specific point with
    the contains() function. You can also test to see if two rectangles
    intersect with the intersects() function (see also intersect()).
    To get the bounding rectangle of two rectangles use unite().

    \sa QPointF QSizeF
*/

/*****************************************************************************
  QRectF member functions
 *****************************************************************************/

/*!
    \fn QRectF::QRectF()

    Constructs an invalid rectangle.
*/

/*!
    \fn QRectF::QRectF(const QPointF &topLeft, const QSizeF &size)

    Constructs a rectangle with \a topLeft as the top-left corner and
    \a size as the rectangle size.
*/

/*!
    \fn QRectF::QRectF(float x, float y, float width, float height)

    Constructs a rectangle with the top-left corner at (\a x, \a y) and
    dimensions specified by the \a width and \a height.

    Example (creates three identical rectangles):
    \code
        QRectF r1(QPoint(100,200), QPoint(110,215));
        QRectF r2(QPoint(100,200), QSize(11,16));
        QRectF r3(100, 200, 11, 16);
    \endcode
*/

/*!
    \fn QRectF::QRectF(const QRect &rect)

    Constructs a rectangle with floating point accuracy from the given
    \a rect.
*/

/*!
    \fn bool QRectF::isNull() const

    Returns true if the rectangle is a null rectangle; otherwise
    returns false.

    A null rectangle has both the width and the height set to 0.

    A null rectangle is also empty and invalid.

    \sa isEmpty() isValid()
*/

/*!
    \fn bool QRectF::isEmpty() const

    Returns true if the rectangle is empty; otherwise returns false.

    An empty rectangle has a width() \<= 0 or height() \<= 0.

    An empty rectangle is not valid. \c{isEmpty() == !isValid()}

    \sa isNull() isValid() normalize()
*/

/*!
    \fn bool QRectF::isValid() const

    Returns true if the rectangle is valid; otherwise returns false.

    A valid rectangle has a width() \> 0 and height() \> 0.

    Note that non-trivial operations like intersections are not defined
    for invalid rectangles.

    \c{isValid() == !isEmpty()}

    \sa isNull() isEmpty() normalize()
*/


/*!
    Returns a normalized rectangle; i.e. a rectangle that has a
    non-negative width and height.

    It swaps left and right if width() \< 0, and swaps top and
    bottom if height() \< 0.

    \sa isValid()
*/

QRectF QRectF::normalize() const
{
    QRectF r = *this;
    if (r.w < 0) {
        r.xp = r.xp + r.w;
        r.w = -r.w;
    }
    if (r.h < 0) {
        r.yp = r.yp + r.h;
        r.h = -r.h;
    }
    return r;
}

/*!
    \fn float QRectF::x() const

    Returns the x-coordinate at the left edge of the rectangle.
    Identical to left().

    \sa left() y() setX()
*/

/*!
    \fn float QRectF::y() const

    Returns the y-coordinate at the top edge of the rectangle.
    Identical to top().

    \sa top() x() setY()
*/

/*!
    \fn QPointF QRectF::center() const

    Returns the center point of the rectangle.

    \sa moveCenter(), topLeft(), bottomRight(), topRight(), bottomLeft()
*/


/*!
    \fn void QRectF::getRect(float *x, float *y, float *w, float *h) const

    Extracts the position of the rectangle's top-left corner to \c{*}\a{x},
    \c{*}\a{y}, its width to \c{*}\a{w}, and its height to \c{*}\a{h}.

    \sa setRect() coords()
*/


/*!
    \fn void QRectF::getCoords(float *xp1, float *yp1, float *xp2, float *yp2) const

    Extracts the rectangle parameters as the top-left point
    \c{*}\a{xp1}, \c{*}\a{yp1} and the bottom-right point
    \c{*}\a{xp2}, \c{*}\a{yp2}.

    \sa setCoords() rect()
*/

/*
  \fn void QRectF::rect(float *x, float *y, float *w, float *h) const

  Use getRect() instead.
*/


/*
  \fn void QRectF::coords(float *xp1, float *yp1, float *xp2, float *yp2) const

  Use getCoords() instead.

*/


/*!
  \fn void QRectF::moveTo(float x, float y)

  Moves the top left corner of the rectangle to \a x and \a y, without
  changing the rectangles size.
*/

/*!
  \fn void QRectF::moveTo(const QPointF &pt)

  Moves the top left corner of the rectangle to \a pt, without
  changing the rectangles size.
*/

/*!
    \fn void QRectF::translate(float dx, float dy)

    Moves the rectangle \a dx along the x axis and \a dy along the y
    axis, relative to the current position. Positive values move the
    rectangle to the right and downwards.
*/


/*!
    \fn void QRectF::translate(const QPointF &p)

    \overload

    Moves the rectangle \a{p}\e{.x()} along the x axis and
    \a{p}\e{.y()} along the y axis, relative to the current position.
    Positive values move the rectangle to the right and downwards.
*/


/*!
  \fn void QRectF::setRect(float x, float y, float width, float height)

  Sets the position of the rectangle's top-left corner to (\a{x},
  \a{y}), and resizes it to the specified \a width and \a height.

  \sa rect(), setCoords()
*/


/*!
    \fn void QRectF::setCoords(float xp1, float yp1, float xp2, float yp2)

    Sets the position of the rectangle's top-left corner to
    (\a{xp1}, \a{yp1}), and the position of its bottom-right corner to
    (\a{xp2}, \a{yp2}).

    \sa coords() setRect()
*/


/*!
    \fn void QRectF::addCoords(float xp1, float yp1, float xp2, float yp2)

    Adds (\a xp1, \a yp1) to the existing position of the rectangle's
    top-left corner, and (\a xp2, \a yp2) to the position of its
    bottom-right corner.
*/

/*! \fn QRectF QRectF::adjusted(float xp1, float yp1, float xp2, float yp2) const

    Returns a new rectangle with (\a xp1, \a yp1) added to the
    existing position of the rectangle's top-left corner, and (\a xp2,
    \a yp2) added to the position of its bottom-right corner.
*/

/*!
    \fn QSizeF QRectF::size() const

    Returns the size of the rectangle to floating point accuracy.

    \sa width(), height()
*/

/*!
    \fn float QRectF::width() const

    Returns the width of the rectangle. The width includes both the
    left and right edges; i.e. width = right - left + 1.

    \sa height() size() setHeight()
*/

/*!
    \fn float QRectF::height() const

    Returns the height of the rectangle. The height includes both the
    top and bottom edges; i.e. height = bottom - top + 1.

    \sa width() size() setHeight()
*/

/*!
    \fn void QRectF::setWidth(float width)

    Sets the \a width of the rectangle. If the width is different to the old
    width, only the rectangle's right edge is moved. The left edge is not moved.

    \sa width() setLeft() setRight() setSize()
*/


/*!
    \fn void QRectF::setHeight(float height)

    Sets the \a height of the rectangle. The top edge is not moved, but the
    bottom edge may be moved.

    \sa height() setTop() setBottom() setSize()
*/


/*!
    \fn void QRectF::setSize(const QSizeF &size)

    Sets the size of the rectangle to \a size. The top-left corner is
    not moved.

    \sa size(), setWidth(), setHeight()
*/


/*!
    \fn bool QRectF::contains(const QPointF &point) const

    Returns true if the given \a point is inside or on the edge of the
    rectangle; otherwise returns false.
*/


/*!
    \fn bool QRectF::contains(float x, float y) const
    \overload

    Returns true if the point (\a x, \a y) is inside this rectangle;
    otherwise returns false.
*/

/*!
    \fn bool QRectF::contains(const QRectF &rectangle) const

    \overload

    Returns true if the given \a rectangle is inside this rectangle;
    otherwise returns false.

    \sa unite() intersect() intersects()
*/

bool QRectF::contains(const QRectF &r) const
{
        return r.xp >= xp && r.xp + r.w <= xp + w
            && r.yp >= yp && r.yp + r.h <= yp + h;
}

/*!
    \fn QRectF& QRectF::operator|=(const QRectF &other)

    Unites this rectangle with the \a other rectangle, and returns the result.

    \sa operator&=()
*/

/*!
    \fn QRectF& QRectF::operator&=(const QRectF &other)

    Intersects this rectangle with the \a other rectangle, and returns the
    result.

    \sa operator|=()
*/


/*!
    \fn QRectF QRectF::operator|(const QRectF &other) const

    Returns the bounding rectangle of this rectangle and the \a other rectangle.

    The bounding rectangle of a non-empty rectangle and an empty (or
    invalid) rectangle is defined to be the non-empty rectangle.

    \sa operator|=() operator&() intersects() contains()
*/

QRectF QRectF::operator|(const QRectF &r) const
{
    if (!isValid())
        return r;
    if (!r.isValid())
        return *this;

    QRectF tmp;
    tmp.xp = qMin(xp, r.xp);
    tmp.yp = qMin(yp, r.yp);
    tmp.w = qMax(xp + w, r.xp + r.w) - tmp.xp;
    tmp.h = qMax(yp + h, r.yp + r.h) - tmp.yp;
    return tmp;
}

/*!
    \fn QRectF QRectF::unite(const QRectF &other) const

    Returns the bounding rectangle of this rectangle and the \a other
    rectangle. \c{r.unite(s)} is equivalent to \c{r|s}.

    \sa intersect()
*/


/*!
    \fn QRectF QRectF::operator &(const QRectF &other) const

    Returns the intersection of this rectangle and the \a other rectangle.

    Returns an empty rectangle if there is no intersection.

    \sa operator&=() operator|() isEmpty() intersects() contains()
*/

QRectF QRectF::operator&(const QRectF &r) const
{
    QRectF tmp;
    tmp.xp = qMax(xp, r.xp);
    tmp.yp = qMax(yp, r.yp);
    tmp.w = qMin(xp + w, r.xp + r.w) - tmp.xp;
    tmp.h = qMin(yp + h, r.yp + r.h) - tmp.yp;
    return tmp;
}

/*!
    \fn QRectF QRectF::intersect(const QRectF &other) const

    Returns the intersection of this rectangle and the \a other rectangle.
    \c{r.intersect(s)} is equivalent to \c{r&s}.

    \sa unite()
*/

/*!
    \fn bool QRectF::intersects(const QRectF &rectangle) const

    Returns true if this rectangle intersects with the given \a rectangle
    (there is at least one pixel that is within both rectangles);
    otherwise returns false.

    \sa intersect() contains()
*/

bool QRectF::intersects(const QRectF &r) const
{
    return qMax(xp, r.xp) <= qMin(xp + w, r.xp + r.w)
        && qMax(yp, r.yp) <= qMin(yp + h, r.yp + r.h);
}

/*!
    \fn QRect QRectF::toRect() const

    Returns a rectangle with integer precision.
    Note that the coordinates in the returned rectangle are less precise
    than those in the floating point rectangle.
*/


/*!
    \fn bool operator==(const QRectF &r1, const QRectF &r2)

    \relates QRectF

    Returns true if the rectangles \a r1 and \a r2 are equal; otherwise returns
    false.
*/


/*!
    \fn bool operator!=(const QRectF &r1, const QRectF &r2)

    \relates QRectF

    Returns true if the rectangles \a r1 and \a r2 are different; otherwise
    returns false.
*/

/*****************************************************************************
  QRectF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QRectF &rectangle)

    \relates QRectF

    Writes the \a rectangle to the \a stream, and returns a reference to the
    stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QRectF &r)
{
    s << r.x() << r.y() << r.width() << r.height();
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QRectF &rectangle)

    \relates QRectF

    Reads a \a rectangle from the \a stream, and returns a reference to the
    stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QRectF &r)
{
    float x, y, w, h;
    s >> x; s >> y; s >> w; s >> h;
    r.setRect(x, y, w, h);
    return s;
}

#endif // QT_NO_DATASTREAM


#ifndef QT_NO_DEBUG_OUTPUT
QDebug operator<<(QDebug dbg, const QRectF &r) {
    dbg.nospace() << "QRectF(" << r.x() << ',' << r.y() << ','
                  << r.width() << ',' << r.height() << ')';
    return dbg.space();
}
#endif


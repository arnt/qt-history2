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

#include "qrectfloat.h"
#include "qdatastream.h"
#include "qdebug.h"

/*!
    \class QRectF
    \brief The QRectF class defines a rectangle in the plane using floating
    point coordinates for accuracy.

    \ingroup multimedia

    A rectangle is internally represented as an upper-left corner and
    a bottom-right corner, but it is normally expressed as an
    upper-left corner and a size.

    The coordinate type is float.

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

    A QRectF can be constructed with a set of left, top, width and
    height floats, from two QPointF or from a QPointF and a QSizeF.
    After creation the dimensions can be changed, e.g. with setLeft(),
    setRight(), setTop() and setBottom(), or by setting sizes, e.g.
    setWidth(), setHeight() and setSize(). The dimensions can also be
    changed with the move functions, e.g. moveBy(), moveCenter(),
    moveBottomRight(), etc. You can also add coordinates to a
    rectangle with addCoords().

    You can test to see if a QRectF contains a specific point with
    contains(). You can also test to see if two QRectFs intersect with
    intersects() (see also intersect()). To get the bounding rectangle
    of two QRectFs use unite().

    \sa QPointF, QSizeF
*/

/*****************************************************************************
  QRectF member functions
 *****************************************************************************/

/*!
    \fn QRectF::QRectF()

    Constructs an invalid rectangle.
*/

/*!
  \fn QRectF::QRectF(const QPoint &topLeft, const QPoint &bottomRight)

    Constructs a rectangle with \a topLeft as the top-left corner and
    \a bottomRight as the bottom-right corner.
*/


/*!
  \fn QRectF::QRectF(const QPoint &topLeft, const QSize &size)

    Constructs a rectangle with \a topLeft as the top-left corner and
    \a size as the rectangle size.
*/


/*!
    \fn QRectF::QRectF(float left, float top, float width, float height)

    Constructs a rectangle with the \a top, \a left corner and \a
    width and \a height.

    Example (creates three identical rectangles):
    \code
        QRectF r1(QPoint(100,200), QPoint(110,215));
        QRectF r2(QPoint(100,200), QSize(11,16));
        QRectF r3(100, 200, 11, 16);
    \endcode
*/


/*!
    \fn bool QRectF::isNull() const

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
    \fn bool QRectF::isEmpty() const

    Returns true if the rectangle is empty; otherwise returns false.

    An empty rectangle has a left() \> right() or top() \> bottom().

    An empty rectangle is not valid. \c{isEmpty() == !isValid()}

    \sa isNull(), isValid(), normalize()
*/

/*!
    \fn bool QRectF::isValid() const

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
    \fn float QRectF::left() const

    Returns the left coordinate of the rectangle. Identical to x().

    \sa setLeft(), right(), topLeft(), bottomLeft()
*/

/*!
    \fn float QRectF::top() const

    Returns the top coordinate of the rectangle. Identical to y().

    \sa setTop(), bottom(), topLeft(), topRight()
*/

/*!
    \fn float QRectF::right() const

    Returns the right coordinate of the rectangle.

    \sa setRight(), left(), topRight(), bottomRight()
*/

/*!
    \fn float QRectF::bottom() const

    Returns the bottom coordinate of the rectangle.

    \sa setBottom(), top(), bottomLeft(), bottomRight()
*/

/*!
    \fn QCOORD &QRectF::rLeft()

    Returns a reference to the left coordinate of the rectangle.

    \sa rTop(), rRight(), rBottom()
*/

/*!
    \fn QCOORD &QRectF::rTop()

    Returns a reference to the top coordinate of the rectangle.

    \sa rLeft(),  rRight(), rBottom()
*/

/*!
    \fn QCOORD &QRectF::rRight()

    Returns a reference to the right coordinate of the rectangle.

    \sa rLeft(), rTop(), rBottom()
*/

/*!
    \fn QCOORD &QRectF::rBottom()

    Returns a reference to the bottom coordinate of the rectangle.

    \sa rLeft(), rTop(), rRight()
*/

/*!
    \fn float QRectF::x() const

    Returns the left coordinate of the rectangle. Identical to left().

    \sa left(), y(), setX()
*/

/*!
    \fn float QRectF::y() const

    Returns the top coordinate of the rectangle. Identical to top().

    \sa top(), x(), setY()
*/

/*!
    \fn void QRectF::setLeft(float pos)

    Sets the left edge of the rectangle to \a pos. May change the
    width, but will never change the right edge of the rectangle.

    Identical to setX().

    \sa left(), setTop(), setWidth()
*/

/*!
    \fn void QRectF::setTop(float pos)

    Sets the top edge of the rectangle to \a pos. May change the
    height, but will never change the bottom edge of the rectangle.

    Identical to setY().

    \sa top(), setBottom(), setHeight()
*/

/*!
    \fn void QRectF::setRight(float pos)

    Sets the right edge of the rectangle to \a pos. May change the
    width, but will never change the left edge of the rectangle.

    \sa right(), setLeft(), setWidth()
*/

/*!
    \fn void QRectF::setBottom(float pos)

    Sets the bottom edge of the rectangle to \a pos. May change the
    height, but will never change the top edge of the rectangle.

    \sa bottom(), setTop(), setHeight()
*/

/*!
    \fn void QRectF::setX(float x)

    Sets the x position of the rectangle (its left end) to \a x. May
    change the width, but will never change the right edge of the
    rectangle.

    Identical to setLeft().

    \sa x(), setY()
*/

/*!
    \fn void QRectF::setY(float y)

    Sets the y position of the rectangle (its top) to \a y. May change
    the height, but will never change the bottom edge of the
    rectangle.

    Identical to setTop().

    \sa y(), setX()
*/

/*!
  \fn void QRectF::setTopLeft(const QPoint &p)

    Set the top-left corner of the rectangle to \a p. May change
    the size, but will the never change the bottom-right corner of
    the rectangle.

    \sa topLeft(), moveTopLeft(), setBottomRight(), setTopRight(), setBottomLeft()
*/

/*!
  \fn void QRectF::setBottomRight(const QPoint &p)

    Set the bottom-right corner of the rectangle to \a p. May change
    the size, but will the never change the top-left corner of
    the rectangle.

    \sa bottomRight(), moveBottomRight(), setTopLeft(), setTopRight(), setBottomLeft()
*/

/*!
  \fn void QRectF::setTopRight(const QPoint &p)

    Set the top-right corner of the rectangle to \a p. May change
    the size, but will the never change the bottom-left corner of
    the rectangle.

    \sa topRight(), moveTopRight(), setTopLeft(), setBottomRight(), setBottomLeft()
*/

/*!
  \fn void QRectF::setBottomLeft(const QPoint &p)

    Set the bottom-left corner of the rectangle to \a p. May change
    the size, but will the never change the top-right corner of
    the rectangle.

    \sa bottomLeft(), moveBottomLeft(), setTopLeft(), setBottomRight(), setTopRight()
*/

/*!
    \fn QPoint QRectF::topLeft() const

    Returns the top-left position of the rectangle.

    \sa setTopLeft(), moveTopLeft(), bottomRight(), left(), top()
*/

/*!
    \fn QPoint QRectF::bottomRight() const

    Returns the bottom-right position of the rectangle.

    \sa setBottomRight(), moveBottomRight(), topLeft(), right(), bottom()
*/

/*!
    \fn QPoint QRectF::topRight() const

    Returns the top-right position of the rectangle.

    \sa setTopRight(), moveTopRight(), bottomLeft(), top(), right()
*/

/*!
    \fn QPoint QRectF::bottomLeft() const

    Returns the bottom-left position of the rectangle.

    \sa setBottomLeft(), moveBottomLeft(), topRight(), bottom(), left()
*/

/*!
    \fn QPoint QRectF::center() const

    Returns the center point of the rectangle.

    \sa moveCenter(), topLeft(), bottomRight(), topRight(), bottomLeft()
*/


/*!
  \fn void QRectF::getRect(float *x, float *y, float *w, float *h) const

    Extracts the rectangle parameters as the position \c{*}\a{x},
    \c{*}\a{y} and width \c{*}\a{w} and height \c{*}\a{h}.

    \sa setRect(), coords()
*/


/*!
  \fn void QRectF::getCoords(float *xp1, float *yp1, float *xp2, float *yp2) const

    Extracts the rectangle parameters as the top-left point
    \c{*}\a{xp1}, \c{*}\a{yp1} and the bottom-right point
    \c{*}\a{xp2}, \c{*}\a{yp2}.

    \sa setCoords(), rect()
*/

/*!
  \fn void QRectF::rect(float *x, float *y, float *w, float *h) const

  Use getRect() instead.
*/


/*!
  \fn void QRectF::coords(float *xp1, float *yp1, float *xp2, float *yp2) const

  Use getCoords() instead.

*/

/*!
  \fn void QRectF::moveLeft(float pos)

    Sets the left position of the rectangle to \a pos, leaving the
    size unchanged.

    \sa left(), setLeft(), moveTop(), moveRight(), moveBottom()
*/

/*!
  \fn void QRectF::moveTop(float pos)

    Sets the top position of the rectangle to \a pos, leaving the
    size unchanged.

    \sa top(), setTop(), moveLeft(), moveRight(), moveBottom()
*/


/*!
  \fn void QRectF::moveRight(float pos)

    Sets the right position of the rectangle to \a pos, leaving the
    size unchanged.

    \sa right(), setRight(), moveLeft(), moveTop(), moveBottom()
*/


/*!
  \fn void QRectF::moveBottom(float pos)

    Sets the bottom position of the rectangle to \a pos, leaving the
    size unchanged.

    \sa bottom(), setBottom(), moveLeft(), moveTop(), moveRight()
*/


/*!
  \fn void QRectF::moveTopLeft(const QPoint &p)

    Sets the top-left position of the rectangle to \a p, leaving the
    size unchanged.

    \sa topLeft(), setTopLeft(), moveBottomRight(), moveTopRight(), moveBottomLeft()
*/


/*!
  \fn void QRectF::moveBottomRight(const QPoint &p)

    Sets the bottom-right position of the rectangle to \a p, leaving
    the size unchanged.

    \sa bottomRight(), setBottomRight(), moveTopLeft(), moveTopRight(), moveBottomLeft()
*/


/*!
  \fn void QRectF::moveTopRight(const QPoint &p)

    Sets the top-right position of the rectangle to \a p, leaving the
    size unchanged.

    \sa topRight(), setTopRight(), moveTopLeft(), moveBottomRight(), moveBottomLeft()
*/


/*!
  \fn void QRectF::moveBottomLeft(const QPoint &p)

    Sets the bottom-left position of the rectangle to \a p, leaving
    the size unchanged.

    \sa bottomLeft(), setBottomLeft(), moveTopLeft(), moveBottomRight(), moveTopRight()
*/



/*!
    Sets the center point of the rectangle to \a p, leaving the size
    unchanged.

    \sa center(), moveTopLeft(), moveBottomRight(), moveTopRight(), moveBottomLeft()
*/

void QRectF::moveCenter(const QPointF &p)
{
    xp = p.x() - w/2;
    yp = p.y() - h/2;
}


/*!
  \fn void QRectF::moveBy(float dx, float dy)

    Moves the rectangle \a dx along the x axis and \a dy along the y
    axis, relative to the current position. Positive values move the
    rectangle to the right and down.

    \sa moveTopLeft()
*/


/*!
    \fn void QRectF::moveBy(const QPoint &p)

    \overload

    Moves the rectangle \a{p}\e{.x()} along the x axis and
    \a{p}\e{.y()} along the y axis, relative to the current position.
    Positive values move the rectangle to the right and down.

    \sa moveTopLeft()
*/


/*!
  \fn  void QRectF::setRect(float x, float y, float w, float h)

  Sets the coordinates of the rectangle's top-left corner to (\a{x},
  \a{y}), and its size to (\a{w}, \a{h}).

  \sa rect(), setCoords()
*/


/*!
  \fn void QRectF::setCoords(float xp1, float yp1, float xp2, float yp2)

    Sets the coordinates of the rectangle's top-left corner to
    (\a{xp1}, \a{yp1}), and the coordinates of its bottom-right corner to
    (\a{xp2}, \a{yp2}).

    \sa coords(), setRect()
*/


/*!
  \fn void QRectF::addCoords(float xp1, float yp1, float xp2, float yp2)

    Adds \a xp1, \a yp1, \a xp2 and \a yp2 respectively to the
    existing coordinates of the rectangle.
*/


/*!
    \fn QSize QRectF::size() const

    Returns the size of the rectangle.

    \sa width(), height()
*/

/*!
    \fn float QRectF::width() const

    Returns the width of the rectangle. The width includes both the
    left and right edges, i.e. width = right - left + 1.

    \sa height(), size(), setHeight()
*/

/*!
    \fn float QRectF::height() const

    Returns the height of the rectangle. The height includes both the
    top and bottom edges, i.e. height = bottom - top + 1.

    \sa width(), size(), setHeight()
*/

/*!
  \fn void QRectF::setWidth(float w)

    Sets the width of the rectangle to \a w. The right edge is
    changed, but not the left edge.

    \sa width(), setLeft(), setRight(), setSize()
*/


/*!
  \fn void QRectF::setHeight(float h)

    Sets the height of the rectangle to \a h. The top edge is not
    moved, but the bottom edge may be moved.

    \sa height(), setTop(), setBottom(), setSize()
*/


/*!
  \fn void QRectF::setSize(const QSize &s)

    Sets the size of the rectangle to \a s. The top-left corner is not
    moved.

    \sa size(), setWidth(), setHeight()
*/


/*!
  \fn bool QRectF::contains(const QPoint &p, bool proper) const

    Returns true if the point \a p is inside or on the edge of the
    rectangle; otherwise returns false.

    If \a proper is true, this function returns true only if \a p is
    inside (not on the edge).
*/


/*!
    \fn bool QRectF::contains(float x, float y, bool proper) const
    \overload

    Returns true if the point \a x, \a y is inside this rectangle;
    otherwise returns false.

    If \a proper is true, this function returns true only if the point
    is entirely inside (not on the edge).
*/

/*!
    \fn bool QRectF::contains(float x, float y) const
    \overload

    Returns true if the point \a x, \a y is inside this rectangle;
    otherwise returns false.
*/

/*!
    \overload

    Returns true if the rectangle \a r is inside this rectangle;
    otherwise returns false.

    \sa unite(), intersect(), intersects()
*/

bool QRectF::contains(const QRectF &r) const
{
        return r.xp >= xp && r.xp + r.w <= xp + w
            && r.yp >= yp && r.yp + r.h <= yp + h;
}

/*!
  \fn QRectF& QRectF::operator|=(const QRectF &r)

    Unites this rectangle with rectangle \a r.
*/

/*!
  \fn QRectF& QRectF::operator&=(const QRectF &r)

    Intersects this rectangle with rectangle \a r.
*/


/*!
    Returns the bounding rectangle of this rectangle and rectangle \a
    r.

    The bounding rectangle of a nonempty rectangle and an empty or
    invalid rectangle is defined to be the nonempty rectangle.

    \sa operator|=(), operator&(), intersects(), contains()
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
  \fn QRectF QRectF::unite(const QRectF &r) const

    Returns the bounding rectangle of this rectangle and rectangle \a
    r. \c{r.unite(s)} is equivalent to \c{r|s}.
*/


/*!
    Returns the intersection of this rectangle and rectangle \a r.

    Returns an empty rectangle if there is no intersection.

    \sa operator&=(), operator|(), isEmpty(), intersects(), contains()
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
  \fn QRectF QRectF::intersect(const QRectF &r) const

    Returns the intersection of this rectangle and rectangle \a r.
    \c{r.intersect(s)} is equivalent to \c{r&s}.
*/

/*!
    Returns true if this rectangle intersects with rectangle \a r
    (there is at least one pixel that is within both rectangles);
    otherwise returns false.

    \sa intersect(), contains()
*/

bool QRectF::intersects(const QRectF &r) const
{
    return qMax(xp, r.xp) <= qMin(xp + w, r.xp + r.w)
        && qMax(yp, r.yp) <= qMin(yp + h, r.yp + r.h);
}


/*!
  \fn bool operator==(const QRectF &r1, const QRectF &r2)

    \relates QRectF

    Returns true if \a r1 and \a r2 are equal; otherwise returns false.
*/


/*!
  \fn bool operator!=(const QRectF &r1, const QRectF &r2)

    \relates QRectF

    Returns true if \a r1 and \a r2 are different; otherwise returns false.
*/


/*****************************************************************************
  QRectF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QRectF

    Writes the QRectF, \a r, to the stream \a s, and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QRectF &r)
{
    s << r.left() << r.top() << r.width() << r.height();
    return s;
}

/*!
    \relates QRectF

    Reads a QRectF from the stream \a s into rect \a r and returns a
    reference to the stream.

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


#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QRectF &r) {
    dbg.nospace() << "QRectF(" << r.x() << ',' << r.y() << ','
                  << r.width() << ',' << r.height() << ')';
    return dbg.space();
}
#endif


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

#include "qsizefloat.h"
#include "qdatastream.h"
#include "qdebug.h"

/*!
    \class QSizeFloat
    \brief The QSizeFloat class defines the size of a two-dimensional object
    using floating point values for accuracy.

    \ingroup multimedia

    A size is specified by a width and a height.

    The coordinate type is float.

    The size can be set in the constructor and changed with
    setWidth(), setHeight(), or scale(), or using operator+=(),
    operator-=(), operator*=() and operator/=(), etc. You can swap the
    width and height with transpose(). You can get a size which holds
    the maximum height and width of two sizes using expandedTo(), and
    the minimum height and width of two sizes using boundedTo().

    \sa QSize QPointFloat QRectFloat
*/


/*****************************************************************************
  QSizeFloat member functions
 *****************************************************************************/

/*!
    \fn QSizeFloat::QSizeFloat()

    Constructs a size with invalid width and height.

    \sa isValid() setWidth() setHeight()
*/

/*!
    \fn QSizeFloat::QSizeFloat(int w, int h)

    Constructs a size with width \a w and height \a h.
*/

/*!
    \fn bool QSizeFloat::isNull() const

    Returns true if the width is 0 and the height is 0; otherwise
    returns false.

    \sa isValid() isEmpty() width() height()
*/

/*!
    \fn bool QSizeFloat::isEmpty() const

    Returns true if the width is less than or equal to 0, or the height is
    less than or equal to 0; otherwise returns false.

    \sa isNull() isValid() width() height()
*/

/*!
    \fn bool QSizeFloat::isValid() const

    Returns true if the width is equal to or greater than 0 and the height is
    equal to or greater than 0; otherwise returns false.

    \sa isNull() isEmpty() width() height()
*/

/*!
    \fn int QSizeFloat::width() const

    Returns the width.

    \sa height()
*/

/*!
    \fn int QSizeFloat::height() const

    Returns the height.

    \sa width()
*/

/*!
    \fn void QSizeFloat::setWidth(int w)

    Sets the width to \a w.

    \sa width() setHeight() expandedTo() boundedTo() scale() transpose()
*/

/*!
    \fn void QSizeFloat::setHeight(int h)

    Sets the height to \a h.

    \sa height() setWidth() expandedTo() boundedTo() scale() transpose()
*/

/*!
    Swaps the width and height values.

    \sa expandedTo() boundedTo() setWidth() setHeight()
*/

void QSizeFloat::transpose()
{
    float tmp = wd;
    wd = ht;
    ht = tmp;
}

/*!
  \fn void QSizeFloat::scale(float w, float h, Qt::ScaleMode mode)

    Scales the size to a rectangle of width \a w and height \a h according
    to the Qt::ScaleMode \a mode.

    \list
    \i If \a mode is \c Qt::ScaleFree, the size is set to (\a w, \a h).
    \i If \a mode is \c Qt::ScaleMin, the current size is scaled to a rectangle
       as large as possible inside (\a w, \a h), preserving the aspect ratio.
    \i If \a mode is \c Qt::ScaleMax, the current size is scaled to a rectangle
       as small as possible outside (\a w, \a h), preserving the aspect ratio.
    \endlist

    Example:
    \code
        QSizeFloat t1(10, 12);
        t1.scale(60, 60, QSizeFloat::ScaleFree);
        // t1 is (60, 60)

        QSizeFloat t2(10, 12);
        t2.scale(60, 60, QSizeFloat::ScaleMin);
        // t2 is (50, 60)

        QSizeFloat t3(10, 12);
        t3.scale(60, 60, QSizeFloat::ScaleMax);
        // t3 is (60, 72)
    \endcode

    \sa boundedTo() expandedTo() setWidth() setHeight()
*/

/*!
    \overload

    Equivalent to scale(\a{s}.width(), \a{s}.height(), \a mode).
*/
void QSizeFloat::scale(const QSizeFloat &s, Qt::ScaleMode mode)
{
    if (mode == Qt::ScaleFree) {
        wd = s.wd;
        ht = s.ht;
    } else {
        bool useHeight;
        float rw = s.ht * wd / ht;

        if (mode == Qt::ScaleMin) {
            useHeight = (rw <= s.wd);
        } else { // mode == Qt::ScaleMax
            useHeight = (rw >= s.wd);
        }

        if (useHeight) {
            wd = rw;
            ht = s.ht;
        } else {
            wd = s.wd;
            ht = s.wd * ht / wd;
        }
    }
}

/*!
    \fn QCOORD &QSizeFloat::rwidth()

    Returns a reference to the width.

    Using a reference makes it possible to directly manipulate the width.

    Example:
    \code
        QSizeFloat s(100, 10);
        s.rwidth() += 20;                // s becomes (120,10)
    \endcode

    \sa rheight() setWidth()
*/

/*!
    \fn QCOORD &QSizeFloat::rheight()

    Returns a reference to the height.

    Using a reference makes it possible to directly manipulate the height.

    Example:
    \code
        QSizeFloat s(100, 10);
        s.rheight() += 5;                // s becomes (100,15)
    \endcode

    \sa rwidth() setHeight()
*/

/*!
    \fn QSizeFloat &QSizeFloat::operator+=(const QSizeFloat &s)

    Adds \a s to the size and returns a reference to this size.

    Example:
    \code
        QSizeFloat s( 3, 7);
        QSizeFloat r(-1, 4);
        s += r;                        // s becomes (2,11)
    \endcode
*/

/*!
    \fn QSizeFloat &QSizeFloat::operator-=(const QSizeFloat &s)

    Subtracts \a s from the size and returns a reference to this size.

    Example:
    \code
        QSizeFloat s( 3, 7);
        QSizeFloat r(-1, 4);
        s -= r;                        // s becomes (4,3)
    \endcode
*/

/*!
    \fn QSizeFloat &QSizeFloat::operator*=(int c)

    Multiplies both the width and height by \a c and returns a
    reference to the size.

    \sa scale()
*/

/*!
    \fn QSizeFloat &QSizeFloat::operator*=(double c)

    \overload

    Multiplies both the width and height by \a c and returns a reference to
    the size.

    Note that the result is truncated.
*/

/*!
    \fn bool operator==(const QSizeFloat &s1, const QSizeFloat &s2)

    \relates QSizeFloat

    Returns true if \a s1 and \a s2 are equal; otherwise returns false.
*/

/*!
    \fn bool operator!=(const QSizeFloat &s1, const QSizeFloat &s2)

    \relates QSizeFloat

    Returns true if \a s1 and \a s2 are different; otherwise returns false.
*/

/*!
    \fn const QSizeFloat operator+(const QSizeFloat &s1, const QSizeFloat &s2)

    \relates QSizeFloat

    Returns the sum of \a s1 and \a s2; each component is added separately.

    \sa scale()
*/

/*!
    \fn const QSizeFloat operator-(const QSizeFloat &s1, const QSizeFloat &s2)

    \relates QSizeFloat

    Returns \a s2 subtracted from \a s1; each component is subtracted
    separately.

    \sa scale()
*/

/*!
    \fn const QSizeFloat operator*(const QSizeFloat &s, int c)

    \relates QSizeFloat

    Multiplies \a s by \a c and returns the result.

    \sa scale()
*/

/*!
    \fn const QSizeFloat operator*(int c, const QSizeFloat &s)

    \overload
    \relates QSizeFloat

    Multiplies \a s by \a c and returns the result.

    \sa scale()
*/

/*!
    \fn const QSizeFloat operator*(const QSizeFloat &s, double c)

    \overload
    \relates QSizeFloat

    Multiplies \a s by \a c and returns the result.

    \sa scale()
*/

/*!
    \fn const QSizeFloat operator*(double c, const QSizeFloat &s)

    \overload
    \relates QSizeFloat

    Multiplies \a s by \a c and returns the result.

    \sa scale()
*/

/*!
    \fn QSizeFloat &QSizeFloat::operator/=(int c)

    Divides both the width and height by \a c and returns a reference
    to the size.

    \sa scale()
*/

/*!
    \fn QSizeFloat &QSizeFloat::operator/=(double c)

    \overload

    Divides both the width and height by \a c and returns a reference to the
    size.

    Note that the result is truncated.

    \sa scale()
*/

/*!
    \fn const QSizeFloat operator/(const QSizeFloat &s, int c)

    \relates QSizeFloat

    Divides \a s by \a c and returns the result.

    \sa scale()
*/

/*!
    \fn const QSizeFloat operator/(const QSizeFloat &s, double c)

    \relates QSizeFloat
    \overload

    Divides \a s by \a c and returns the result.

    Note that the result is truncated.

    \sa scale()
*/

/*!
    \fn QSizeFloat QSizeFloat::expandedTo(const QSizeFloat & otherSize) const

    Returns a size with the maximum width and height of this size and
    \a otherSize.

    \sa boundedTo() scale() setWidth() setHeight()
*/

/*!
    \fn QSizeFloat QSizeFloat::boundedTo(const QSizeFloat & otherSize) const

    Returns a size with the minimum width and height of this size and
    \a otherSize.

    \sa expandedTo() scale() setWidth() setHeight()
*/



/*****************************************************************************
  QSizeFloat stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QSizeFloat

    Writes the size \a sz to the stream \a s and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QSizeFloat &sz)
{
    s << sz.width() << sz.height();
    return s;
}

/*!
    \relates QSizeFloat

    Reads the size from the stream \a s into size \a sz and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QSizeFloat &sz)
{
    float w, h;
    s >> w;  sz.rwidth() = w;
    s >> h;  sz.rheight() = h;
    return s;
}
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QSizeFloat &s) {
    dbg.nospace() << "QSizeFloat(" << s.width() << ',' << s.height() << ')';
    return dbg.space();
}
#endif

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

#include "qsize.h"
#include "qdatastream.h"
#include "qdebug.h"

/*!
    \class QSize
    \brief The QSize class defines the size of a two-dimensional object.

    \ingroup multimedia

    A size is specified by a width and a height.

    The size can be set in the constructor and changed with setWidth(),
    setHeight(), or scale(), or using arithmetic operators. You can swap the
    width and height with transpose(). You can get a size which holds the
    maximum height and width of two sizes using expandedTo(), and the minimum
    height and width of two sizes using boundedTo().


    \sa QPoint, QRect QSizeF
*/


/*****************************************************************************
  QSize member functions
 *****************************************************************************/

/*!
    \fn QSize::QSize()

    Constructs a size with an invalid width and height.

    \sa isValid() setWidth() setHeight()
*/

/*!
    \fn QSize::QSize(int w, int h)

    Constructs a size with width \a w and height \a h.
*/

/*!
    \fn bool QSize::isNull() const

    Returns true if the width is 0 and the height is 0; otherwise
    returns false.

    \sa isValid() isEmpty() width() height()
*/

/*!
    \fn bool QSize::isEmpty() const

    Returns true if the width is less than or equal to 0, or the height is
    less than or equal to 0; otherwise returns false.

    \sa isNull() isValid() width() height()
*/

/*!
    \fn bool QSize::isValid() const

    Returns true if the width is equal to or greater than 0 and the height is
    equal to or greater than 0; otherwise returns false.

    \sa isNull() isEmpty() width() height()
*/

/*!
    \fn int QSize::width() const

    Returns the width.

    \sa height() setWidth()
*/

/*!
    \fn int QSize::height() const

    Returns the height.

    \sa width() setHeight()
*/

/*!
    \fn void QSize::setWidth(int w)

    Sets the width to \a w.

    \sa width() rwidth() setHeight() expandedTo() boundedTo() scale() transpose()
*/

/*!
    \fn void QSize::setHeight(int h)

    Sets the height to \a h.

    \sa height() rheight() setWidth() expandedTo() boundedTo() scale() transpose()
*/

/*!
    Swaps the width and height values.

    \sa expandedTo() boundedTo() setWidth() setHeight()
*/

void QSize::transpose()
{
    int tmp = wd;
    wd = ht;
    ht = tmp;
}

/*!
  \fn void QSize::scale(int w, int h, Qt::AspectRatioMode mode)

    Scales the size to a rectangle of width \a w and height \a h according
    to the Qt::AspectRatioMode \a mode.

    \list
    \i If \a mode is \c Qt::IgnoreAspectRatio, the size is set to (\a w, \a h).
    \i If \a mode is \c Qt::KeepAspectRatio, the current size is scaled to a rectangle
       as large as possible inside (\a w, \a h), preserving the aspect ratio.
    \i If \a mode is \c Qt::KeepAspectRatioByExpanding, the current size is scaled to a rectangle
       as small as possible outside (\a w, \a h), preserving the aspect ratio.
    \endlist

    Example:
    \code
        QSize t1(10, 12);
        t1.scale(60, 60, QSize::IgnoreAspectRatio);
        // t1 is (60, 60)

        QSize t2(10, 12);
        t2.scale(60, 60, QSize::KeepAspectRatio);
        // t2 is (50, 60)

        QSize t3(10, 12);
        t3.scale(60, 60, QSize::KeepAspectRatioByExpanding);
        // t3 is (60, 72)
    \endcode

    \sa boundedTo() expandedTo() setWidth() setHeight()
*/

/*!
    \overload

    Equivalent to scale(\a{s}.width(), \a{s}.height(), \a mode).
*/
void QSize::scale(const QSize &s, Qt::AspectRatioMode mode)
{
    if (mode == Qt::IgnoreAspectRatio) {
        wd = s.wd;
        ht = s.ht;
    } else {
        bool useHeight;
        int rw = s.ht * wd / ht;

        if (mode == Qt::KeepAspectRatio) {
            useHeight = (rw <= s.wd);
        } else { // mode == Qt::KeepAspectRatioByExpanding
            useHeight = (rw >= s.wd);
        }

        if (useHeight) {
            wd = rw;
            ht = s.ht;
        } else {
            ht = s.wd * ht / wd;
            wd = s.wd;
        }
    }
}

/*!
    \fn int &QSize::rwidth()

    Returns a reference to the width.

    Using a reference makes it possible to directly manipulate the width.

    Example:
    \code
        QSize s(100, 10);
        s.rwidth() += 20;                // s becomes (120,10)
    \endcode

    \sa rheight() setWidth()
*/

/*!
    \fn int &QSize::rheight()

    Returns a reference to the height.

    Using a reference makes it possible to directly manipulate the height.

    Example:
    \code
        QSize s(100, 10);
        s.rheight() += 5;                // s becomes (100,15)
    \endcode

    \sa rwidth() setHeight()
*/

/*!
    \fn QSize &QSize::operator+=(const QSize &s)

    Adds \a s to the size and returns a reference to this size.

    Example:
    \code
        QSize s( 3, 7);
        QSize r(-1, 4);
        s += r;                        // s becomes (2,11)
    \endcode
*/

/*!
    \fn QSize &QSize::operator-=(const QSize &s)

    Subtracts \a s from the size and returns a reference to this size.

    Example:
    \code
        QSize s( 3, 7);
        QSize r(-1, 4);
        s -= r;                        // s becomes (4,3)
    \endcode
*/

/*!
    \fn QSize &QSize::operator*=(qreal coeff)

    \overload

    Multiplies both the width and height by \a coeff and returns a
    reference to the size.

    Note that the result is rounded to the nearest integer.
*/

/*!
    \fn bool operator==(const QSize &s1, const QSize &s2)

    \relates QSize

    Returns true if \a s1 and \a s2 are equal; otherwise returns false.
*/

/*!
    \fn bool operator!=(const QSize &s1, const QSize &s2)

    \relates QSize

    Returns true if \a s1 and \a s2 are different; otherwise returns false.
*/

/*!
    \fn const QSize operator+(const QSize &s1, const QSize &s2)

    \relates QSize

    Returns the sum of \a s1 and \a s2; each component is added separately.
*/

/*!
    \fn const QSize operator-(const QSize &s1, const QSize &s2)

    \relates QSize

    Returns \a s2 subtracted from \a s1; each component is subtracted
    separately.
*/

/*!
    \fn const QSize operator*(const QSize &size, qreal coeff)

    \relates QSize

    Multiplies \a size by \a coeff and returns the result rounded to the nearest integer.

    \sa QSize::scale()
*/

/*!
    \fn const QSize operator*(qreal coeff, const QSize &size)

    \overload
    \relates QSize

    Multiplies \a size by \a coeff and returns the result rounded to the nearest integer.

    \sa QSize::scale()
*/

/*!
    \fn QSize &QSize::operator/=(qreal coeff)

    \overload

    Divides both the width and height by \a coeff and returns a
    reference to the size.

    Note that the result is rounded to the nearest integer.

    \sa QSize::scale()
*/

/*!
    \fn const QSize operator/(const QSize &size, qreal divisor)

    \relates QSize
    \overload

    Divides \a size by \a divisor and returns the result.

    Note that the result is rounded to the nearest integer.

    \sa QSize::scale()
*/

/*!
    \fn QSize QSize::expandedTo(const QSize & otherSize) const

    Returns a size with the maximum width and height of this size and
    \a otherSize.

    \sa boundedTo() scale() setWidth() setHeight()
*/

/*!
    \fn QSize QSize::boundedTo(const QSize & otherSize) const

    Returns a size with the minimum width and height of this size and
    \a otherSize.

    \sa expandedTo() scale() setWidth() setHeight()
*/



/*****************************************************************************
  QSize stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QSize

    Writes the size \a sz to the stream \a s and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QSize &sz)
{
    if (s.version() == 1)
        s << (qint16)sz.width() << (qint16)sz.height();
    else
        s << (qint32)sz.width() << (qint32)sz.height();
    return s;
}

/*!
    \relates QSize

    Reads the size from the stream \a s into size \a sz and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QSize &sz)
{
    if (s.version() == 1) {
        qint16 w, h;
        s >> w;  sz.rwidth() = w;
        s >> h;  sz.rheight() = h;
    }
    else {
        qint32 w, h;
        s >> w;  sz.rwidth() = w;
        s >> h;  sz.rheight() = h;
    }
    return s;
}
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QSize &s) {
    dbg.nospace() << "QSize(" << s.width() << ',' << s.height() << ')';
    return dbg.space();
}
#endif



/*!
    \class QSizeF
    \brief The QSizeF class defines the size of a two-dimensional object
    using floating point values for accuracy.

    \ingroup multimedia

    A size is specified by a width and a height.

    The coordinate type is qreal.

    The size can be set in the constructor and changed with setWidth(),
    setHeight(), or scale(), or using arithmetic operators.  You can swap the
    width and height with transpose(). You can get a size which holds the
    maximum height and width of two sizes using expandedTo(), and the minimum
    height and width of two sizes using boundedTo().

    \sa QSize QPointF QRectF
*/


/*****************************************************************************
  QSizeF member functions
 *****************************************************************************/

/*!
    \fn QSizeF::QSizeF()

    Constructs an invalid size.

    \sa isValid() setWidth() setHeight()
*/

/*!
    \fn QSizeF::QSizeF(const QSize &size)

    Constructs a size with floating point accuracy from the given \a size.
*/

/*!
    \fn QSizeF::QSizeF(qreal width, qreal height)

    Constructs a size with width \a width and height \a height.
*/

/*!
    \fn bool QSizeF::isNull() const

    Returns true if the width is 0 and the height is 0; otherwise
    returns false.

    \sa isValid() isEmpty() width() height()
*/

/*!
    \fn bool QSizeF::isEmpty() const

    Returns true if the width is less than or equal to 0, or the height is
    less than or equal to 0; otherwise returns false.

    \sa isNull() isValid() width() height()
*/

/*!
    \fn bool QSizeF::isValid() const

    Returns true if the width is equal to or greater than 0 and the height is
    equal to or greater than 0; otherwise returns false.

    \sa isNull() isEmpty() width() height()
*/

/*!
    \fn int QSizeF::width() const

    Returns the width.

    \sa height() setWidth()
*/

/*!
    \fn int QSizeF::height() const

    Returns the height.

    \sa width() setHeight()
*/

/*!
    \fn void QSizeF::setWidth(qreal width)

    Sets the width to \a width.

    \sa width() rwidth() setHeight() expandedTo() boundedTo() scale() transpose()
*/

/*!
    \fn void QSizeF::setHeight(qreal height)

    Sets the height to \a height.

    \sa height() setWidth() expandedTo() boundedTo() scale() transpose()
*/

/*!
    \fn QSize QSizeF::toSize() const

    Returns a size with integer precision. Note that the coordinates in the
    returned size will be rounded to the nearest integer.
*/

/*!
    Swaps the width and height values.

    \sa expandedTo() boundedTo() setWidth() setHeight()
*/

void QSizeF::transpose()
{
    qreal tmp = wd;
    wd = ht;
    ht = tmp;
}

/*!
  \fn void QSizeF::scale(qreal w, qreal h, Qt::AspectRatioMode mode)

    Scales the size to a rectangle of width \a w and height \a h according
    to the Qt::AspectRatioMode \a mode.

    \list
    \i If \a mode is \c Qt::IgnoreAspectRatio, the size is set to (\a w, \a h).
    \i If \a mode is \c Qt::KeepAspectRatio, the current size is scaled to a rectangle
       as large as possible inside (\a w, \a h), preserving the aspect ratio.
    \i If \a mode is \c Qt::KeepAspectRatioByExpanding, the current size is scaled to a rectangle
       as small as possible outside (\a w, \a h), preserving the aspect ratio.
    \endlist

    Example:
    \code
        QSizeF t1(10, 12);
        t1.scale(60, 60, QSizeF::IgnoreAspectRatio);
        // t1 is (60, 60)

        QSizeF t2(10, 12);
        t2.scale(60, 60, QSizeF::KeepAspectRatio);
        // t2 is (50, 60)

        QSizeF t3(10, 12);
        t3.scale(60, 60, QSizeF::KeepAspectRatioByExpanding);
        // t3 is (60, 72)
    \endcode

    \sa boundedTo() expandedTo() setWidth() setHeight()
*/

/*!
    \overload

    Equivalent to scale(\a{s}.width(), \a{s}.height(), \a mode).
*/
void QSizeF::scale(const QSizeF &s, Qt::AspectRatioMode mode)
{
    if (mode == Qt::IgnoreAspectRatio) {
        wd = s.wd;
        ht = s.ht;
    } else {
        bool useHeight;
        qreal rw = s.ht * wd / ht;

        if (mode == Qt::KeepAspectRatio) {
            useHeight = (rw <= s.wd);
        } else { // mode == Qt::KeepAspectRatioByExpanding
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
    \fn int &QSizeF::rwidth()

    Returns a reference to the width.

    Using a reference makes it possible to directly manipulate the width.

    Example:
    \code
        QSizeF s(100.3, 10);
        s.rwidth() += 20.5;                // s becomes (120.8,10)
    \endcode

    \sa rheight() setWidth()
*/

/*!
    \fn int &QSizeF::rheight()

    Returns a reference to the height.

    Using a reference makes it possible to directly manipulate the height.

    Example:
    \code
        QSizeF s(100, 10.2);
        s.rheight() += 5.5;                // s becomes (100,15.7)
    \endcode

    \sa rwidth() setHeight()
*/

/*!
    \fn QSizeF &QSizeF::operator+=(const QSizeF &s)

    Adds \a s to the size and returns a reference to this size.

    Example:
    \code
        QSizeF s( 3, 7);
        QSizeF r(-1, 4);
        s += r;                        // s becomes (2,11)
    \endcode
*/

/*!
    \fn QSizeF &QSizeF::operator-=(const QSizeF &s)

    Subtracts \a s from the size and returns a reference to this size.

    Example:
    \code
        QSizeF s( 3, 7);
        QSizeF r(-1, 4);
        s -= r;                        // s becomes (4,3)
    \endcode
*/

/*!
    \fn QSizeF &QSizeF::operator*=(qreal coeff)

    \overload

    Multiplies both the width and height by \a coeff and returns a
    reference to the size.
*/

/*!
    \fn bool operator==(const QSizeF &s1, const QSizeF &s2)

    \relates QSizeF

    Returns true if \a s1 and \a s2 are equal; otherwise returns false.
*/

/*!
    \fn bool operator!=(const QSizeF &s1, const QSizeF &s2)

    \relates QSizeF

    Returns true if \a s1 and \a s2 are different; otherwise returns false.
*/

/*!
    \fn const QSizeF operator+(const QSizeF &s1, const QSizeF &s2)

    \relates QSizeF

    Returns the sum of \a s1 and \a s2; each component is added separately.
*/

/*!
    \fn const QSizeF operator-(const QSizeF &s1, const QSizeF &s2)

    \relates QSizeF

    Returns \a s2 subtracted from \a s1; each component is subtracted
    separately.
*/

/*!
    \fn const QSizeF operator*(const QSizeF &size, qreal coeff)

    \overload
    \relates QSizeF

    Multiplies \a size by \a coeff and returns the result.

    \sa QSize::scale()
*/

/*!
    \fn const QSizeF operator*(qreal c, const QSizeF &s)

    \overload
    \relates QSizeF

    Multiplies \a s by \a c and returns the result.

    \sa QSize::scale()
*/

/*!
    \fn QSizeF &QSizeF::operator/=(qreal divisor)

    \overload

    Divides both the width and height by \a divisor and returns a reference to the
    size.

    \sa QSize::scale()
*/

/*!
    \fn const QSizeF operator/(const QSizeF &size, qreal divisor)

    \relates QSizeF
    \overload

    Divides \a size by \a divisor and returns the result.

    \sa QSize::scale()
*/

/*!
    \fn QSizeF QSizeF::expandedTo(const QSizeF & otherSize) const

    Returns a size with the maximum width and height of this size and
    \a otherSize.

    \sa boundedTo() scale() setWidth() setHeight()
*/

/*!
    \fn QSizeF QSizeF::boundedTo(const QSizeF & otherSize) const

    Returns a size with the minimum width and height of this size and
    \a otherSize.

    \sa expandedTo() scale() setWidth() setHeight()
*/



/*****************************************************************************
  QSizeF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QSizeF

    Writes the size \a sz to the stream \a s and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QSizeF &sz)
{
    s << sz.width() << sz.height();
    return s;
}

/*!
    \relates QSizeF

    Reads the size from the stream \a s into size \a sz and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QSizeF &sz)
{
    double w, h;
    s >> w;
    s >> h;
    sz.setWidth(w);
    sz.setHeight(h);
    return s;
}
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QSizeF &s) {
    dbg.nospace() << "QSizeF(" << s.width() << ',' << s.height() << ')';
    return dbg.space();
}
#endif

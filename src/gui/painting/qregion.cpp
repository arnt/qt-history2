/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qregion.h"
#include "qpolygon.h"
#include "qbuffer.h"
#include "qdatastream.h"
#include "qvariant.h"

#include <qdebug.h>

/*!
    \class QRegion
    \brief The QRegion class specifies a clip region for a painter.

    \ingroup multimedia

    QRegion is used with QPainter::setClipRegion() to limit the paint
    area to what needs to be painted. There is also a
    QWidget::repaint() function that takes a QRegion parameter.
    QRegion is the best tool for reducing flicker.

    A region can be created from a rectangle, an ellipse, a polygon or
    a bitmap. Complex regions may be created by combining simple
    regions using unite(), intersect(), subtract(), or eor() (exclusive
    or). You can move a region using translate().

    You can test whether a region isEmpty() or if it
    contains() a QPoint or QRect. The bounding rectangle can be found
    with boundingRect().

    The function rects() gives a decomposition of the region into
    rectangles.

    Example of using complex regions:
    \code
        void MyWidget::paintEvent(QPaintEvent *)
        {
            QPainter p;                       // our painter
            QRegion r1(QRect(100,100,200,80), // r1 = elliptic region
                       QRegion::Ellipse);
            QRegion r2(QRect(100,120,90,30)); // r2 = rectangular region
            QRegion r3 = r1.intersect(r2);    // r3 = intersection
            p.begin(this);                    // start painting widget
            p.setClipRegion(r3);              // set clip region
            ...                               // paint clipped graphics
            p.end();                          // painting done
        }
    \endcode

    QRegion is an \link shclass.html implicitly shared\endlink class.

    \warning Due to window system limitations, the whole coordinate space for a
    region is limited to the points between -32767 and 32767 on Windows
    95/98/ME. You can circumvent this limitation by using a QPainterPath.

    \section1 Additional License Information

    On Qt/X11 and Qtopia Core, parts of this class rely on code obtained
    under the following license:

    \legalese
    Copyright (c) 1987  X Consortium

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
    X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
    AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the name of the X Consortium shall not be
    used in advertising or otherwise to promote the sale, use or other dealings
    in this Software without prior written authorization from the X Consortium.
    \endlegalese

    \raw HTML
    <hr />
    \endraw

    \legalese
    Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                            All Rights Reserved

    Permission to use, copy, modify, and distribute this software and its
    documentation for any purpose and without fee is hereby granted,
    provided that the above copyright notice appear in all copies and that
    both that copyright notice and this permission notice appear in
    supporting documentation, and that the name of Digital not be
    used in advertising or publicity pertaining to distribution of the
    software without specific, written prior permission.

    DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
    ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
    DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
    ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
    WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
    ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
    SOFTWARE.
    \endlegalese

    \sa QPainter::setClipRegion(), QPainter::setClipRect(), QPainterPath
*/


/*!
    \enum QRegion::RegionType

    Specifies the shape of the region to be created.

    \value Rectangle  the region covers the entire rectangle.
    \value Ellipse  the region is an ellipse inside the rectangle.
*/

/*!
    \fn void QRegion::translate(const QPoint &point)

    \overload

    Translates the region \a{point}\e{.x()} along the x axis and
    \a{point}\e{.y()} along the y axis, relative to the current
    position. Positive values move the region to the right and down.

    Translates to the given \a point.
*/

/*!
    \fn Handle QRegion::handle() const

    Returns a platform-specific region handle. The \c Handle type is
    \c HRGN on Windows, \c Region on X11, and \c RgnHandle on Mac OS
    X.

    \warning This function is not portable.
*/

/*****************************************************************************
  QRegion member functions
 *****************************************************************************/

/*!
    Constructs a rectangular or elliptic region.

    If \a t is \c Rectangle, the region is the filled rectangle (\a x,
    \a y, \a w, \a h). If \a t is \c Ellipse, the region is the filled
    ellipse with center at (\a x + \a w / 2, \a y + \a h / 2) and size
    (\a w ,\a h).
*/
QRegion::QRegion(int x, int y, int w, int h, RegionType t)
{
    QRegion tmp(QRect(x, y, w, h), t);
    tmp.d->ref.ref();
    d = tmp.d;
}

#ifdef QT3_SUPPORT
/*!
    Use the constructor tha takes a Qt::FillRule as the second
    argument instead.
*/
QRegion::QRegion(const QPolygon &pa, bool winding)
{
    new (this) QRegion(pa, winding ? Qt::WindingFill : Qt::OddEvenFill);
}
#endif

/*!
    Detaches from shared region data to make sure that this region is
    the only one referring to the data.

    \sa copy(), \link shclass.html shared classes\endlink
*/

void QRegion::detach()
{
    if (d->ref != 1)
        *this = copy();
}

// duplicates in qregion_win.cpp and qregion_wce.cpp
#define QRGN_SETRECT          1                // region stream commands
#define QRGN_SETELLIPSE       2                //  (these are internal)
#define QRGN_SETPTARRAY_ALT   3
#define QRGN_SETPTARRAY_WIND  4
#define QRGN_TRANSLATE        5
#define QRGN_OR               6
#define QRGN_AND              7
#define QRGN_SUB              8
#define QRGN_XOR              9
#define QRGN_RECTS            10


#ifndef QT_NO_DATASTREAM
/*
    Executes region commands in the internal buffer and rebuilds the
    original region.

    We do this when we read a region from the data stream.

    If \a ver is non-0, uses the format version \a ver on reading the
    byte array.
*/

void QRegion::exec(const QByteArray &buffer, int ver)
{
    QByteArray copy = buffer;
    QDataStream s(&copy, QIODevice::ReadOnly);
    if (ver)
        s.setVersion(ver);
    QRegion rgn;
#ifndef QT_NO_DEBUG
    int test_cnt = 0;
#endif
    while (!s.atEnd()) {
        qint32 id;
        if (s.version() == 1) {
            int id_int;
            s >> id_int;
            id = id_int;
        } else {
            s >> id;
        }
#ifndef QT_NO_DEBUG
        if (test_cnt > 0 && id != QRGN_TRANSLATE)
            qWarning("QRegion::exec: Internal error");
        test_cnt++;
#endif
        if (id == QRGN_SETRECT || id == QRGN_SETELLIPSE) {
            QRect r;
            s >> r;
            rgn = QRegion(r, id == QRGN_SETRECT ? Rectangle : Ellipse);
        } else if (id == QRGN_SETPTARRAY_ALT || id == QRGN_SETPTARRAY_WIND) {
            QPolygon a;
            s >> a;
            rgn = QRegion(a, id == QRGN_SETPTARRAY_WIND ? Qt::WindingFill : Qt::OddEvenFill);
        } else if (id == QRGN_TRANSLATE) {
            QPoint p;
            s >> p;
            rgn.translate(p.x(), p.y());
        } else if (id >= QRGN_OR && id <= QRGN_XOR) {
            QByteArray bop1, bop2;
            QRegion r1, r2;
            s >> bop1;
            r1.exec(bop1);
            s >> bop2;
            r2.exec(bop2);

            switch (id) {
                case QRGN_OR:
                    rgn = r1.unite(r2);
                    break;
                case QRGN_AND:
                    rgn = r1.intersect(r2);
                    break;
                case QRGN_SUB:
                    rgn = r1.subtract(r2);
                    break;
                case QRGN_XOR:
                    rgn = r1.eor(r2);
                    break;
            }
        } else if (id == QRGN_RECTS) {
            // (This is the only form used in Qt 2.0)
            quint32 n;
            s >> n;
            QRect r;
            for (int i=0; i<(int)n; i++) {
                s >> r;
                rgn = rgn.unite(QRegion(r));
            }
        }
    }
    *this = rgn;
}


/*****************************************************************************
  QRegion stream functions
 *****************************************************************************/

/*!
    \relates QRegion

    Writes the region \a r to the stream \a s and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QRegion &r)
{
    QVector<QRect> a = r.rects();
    if (a.isEmpty()) {
        s << (quint32)0;
    } else {
        if (s.version() == 1) {
            int i;
            for (i = a.size() - 1; i > 0; --i) {
                s << (quint32)(12 + i * 24);
                s << (int)QRGN_OR;
            }
            for (i = 0; i < a.size(); ++i) {
                s << (quint32)(4+8) << (int)QRGN_SETRECT << a[i];
            }
        } else {
            s << (quint32)(4 + 4 + 16 * a.size()); // 16: storage size of QRect
            s << (qint32)QRGN_RECTS;
            s << a;
        }
    }
    return s;
}

/*!
    \relates QRegion

    Reads a region from the stream \a s into \a r and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QRegion &r)
{
    QByteArray b;
    s >> b;
    r.exec(b, s.version());
    return s;
}
#endif //QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug s, const QRegion &r)
{
    QVector<QRect> rects = r.rects();
    s.nospace() << "QRegion(size=" << rects.size() << "), "
                << "bounds = " << r.boundingRect() << "\n";
    for (int i=0; i<rects.size(); ++i)
        s << "- " << i << rects.at(i) << "\n";
    return s;
}
#endif


// These are not inline - they can be implemented better on some platforms
//  (eg. Windows at least provides 3-variable operations).  For now, simple.


/*!
    Applies the unite() function to this region and \a r. \c r1|r2 is
    equivalent to \c r1.unite(r2)

    \sa unite(), operator+()
*/
const QRegion QRegion::operator|(const QRegion &r) const
    { return unite(r); }

/*!
    Applies the unite() function to this region and \a r. \c r1+r2 is
    equivalent to \c r1.unite(r2)

    \sa unite(), operator|()
*/
const QRegion QRegion::operator+(const QRegion &r) const
    { return unite(r); }

/*!
    Applies the intersect() function to this region and \a r. \c r1&r2
    is equivalent to \c r1.intersect(r2)

    \sa intersect()
*/
const QRegion QRegion::operator&(const QRegion &r) const
    { return intersect(r); }

/*!
    Applies the subtract() function to this region and \a r. \c r1-r2
    is equivalent to \c r1.subtract(r2)

    \sa subtract()
*/
const QRegion QRegion::operator-(const QRegion &r) const
    { return subtract(r); }

/*!
    Applies the eor() function to this region and \a r. \c r1^r2 is
    equivalent to \c r1.eor(r2)

    \sa eor()
*/
const QRegion QRegion::operator^(const QRegion &r) const
    { return eor(r); }

/*!
    Applies the unite() function to this region and \a r and assigns
    the result to this region. \c r1|=r2 is equivalent to \c
    r1=r1.unite(r2).

    \sa unite()
*/
QRegion& QRegion::operator|=(const QRegion &r)
    { return *this = *this | r; }

/*!
    Applies the unite() function to this region and \a r and assigns
    the result to this region. \c r1+=r2 is equivalent to \c
    r1=r1.unite(r2).

    \sa intersect()
*/
QRegion& QRegion::operator+=(const QRegion &r)
    { return *this = *this + r; }

/*!
    Applies the intersect() function to this region and \a r and
    assigns the result to this region. \c r1&=r2 is equivalent to \c
    r1=r1.intersect(r2).

    \sa intersect()
*/
QRegion& QRegion::operator&=(const QRegion &r)
    { return *this = *this & r; }

/*!
    Applies the subtract() function to this region and \a r and
    assigns the result to this region. \c r1-=r2 is equivalent to \c
    r1=r1.subtract(r2).

    \sa subtract()
*/
QRegion& QRegion::operator-=(const QRegion &r)
    { return *this = *this - r; }

/*!
    Applies the eor() function to this region and \a r and
    assigns the result to this region. \c r1^=r2 is equivalent to \c
    r1=r1.eor(r2).

    \sa eor()
*/
QRegion& QRegion::operator^=(const QRegion &r)
    { return *this = *this ^ r; }

/*!
    \fn bool QRegion::operator!=(const QRegion &other) const

    Returns true if this region is different from the \a other region;
    otherwise returns false.
*/

/*!
   Returns the region as a QVariant
*/
QRegion::operator QVariant() const
{
    return QVariant(QVariant::Region, this);
}

/*!
    \fn bool QRegion::isNull() const

    Use isEmpty() instead.
*/


/*!
    \fn QRegion QRegion::translated(const QPoint &p) const
    \overload
    \since 4.1

    Returns a copy of the regtion that is translated \a{p}\e{.x()}
    along the x axis and \a{p}\e{.y()} along the y axis, relative to
    the current position.  Positive values move the rectangle to the
    right and down.

    \sa translate()
*/

/*!
    \since 4.1

    Returns a copy of the region that is translated \a dx along the
    x axis and \a dy along the y axis, relative to the current
    position. Positive values move the region to the right and
    down.

    \sa translate()
*/

QRegion
QRegion::translated(int dx, int dy) const
{
    QRegion ret(*this);
    ret.translate(dx, dy);
    return ret;
}

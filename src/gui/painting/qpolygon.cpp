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

#include "qpolygon.h"
#include "qrect.h"
#include "qdatastream.h"
#include "qmatrix.h"
#include "qdebug.h"
#include "qpointarray.h"

#include <stdarg.h>

/*!
    \class QPolygon QPolygon.h
    \brief The QPolygon class provides a vector of points.
    \reentrant

    \ingroup multimedia
    \ingroup shared

    A QPolygon is a QVector\<QPoint\>. It is implicitly shared. In
    addition to the functions provided by QVector, QPolygon
    provides some point-specific functions.

    For convenient reading and writing of the point data use
    setPoints(), putPoints(), point(), and setPoint().

    For geometry operations use boundingRect() and translate(). There
    is also the QMatrix::map() function for more general
    transformations of QPolygons. You can also create arcs and
    ellipses with makeArc() and makeEllipse().

    Among others, QPolygon is used by QPainter::drawLineSegments(),
    QPainter::drawPolyline(), QPainter::drawPolygon() and
    QPainter::drawCubicBezier().

    \sa QPainter QMatrix QVector
*/


/*****************************************************************************
  QPolygon member functions
 *****************************************************************************/

/*!
    \fn QPolygon::QPolygon()

    Constructs a null point array.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygon::QPolygon(int size)

    Constructs a point array with room for \a size points. Makes a
    null array if \a size == 0.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygon::QPolygon(const QPolygon &a)

    Constructs a copy of the point array \a a.
*/

/*!
    Constructs a point array from the rectangle \a r.

    If \a closed is false, then the point array just contains the
    following four points in the listed order: r.topLeft(),
    r.topRight(), r.bottomRight() and r.bottomLeft().

    If \a closed is true, then a fifth point is set to r.topLeft().
*/

QPolygon::QPolygon(const QRectF &r)
{
    reserve(5);
    append(r.topLeft());
    append(r.topRight());
    append(r.bottomRight());
    append(r.bottomLeft());
    append(r.topLeft());
}


/*!
    \fn QPolygon::~QPolygon()

    Destroys the point array.
*/


/*!
    Translates all points in the array by (\a{dx}, \a{dy}).
*/

void QPolygon::translate(const QPointF &pt)
{
    register QPointF *p = data();
    register int i = size();
    while (i--) {
        *p += pt;
        ++p;
    }
}

/*! \fn void QPolygon::translate(const QPointF &offset)
    \overload

    Translates all points in the array by \a offset.
*/



/*!
    Returns the bounding rectangle of the points in the array, or
    QRect(0,0,0,0) if the array is empty.
*/

QRectF QPolygon::boundingRect() const
{
    if (isEmpty())
        return QRectF(0, 0, 0, 0);
    register const QPointF *pd = constData();
    float minx, maxx, miny, maxy;
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
    return QRectF(QPointF(minx,miny), QPointF(maxx,maxy));
}

QPointArray QPolygon::toPointArray() const
{
    QPointArray pa(size());
    for (int i=0; i<size(); ++i)
        pa.replace(i, at(i).toPoint());
    return pa;
}

QPolygon QPolygon::fromPointArray(const QPointArray &a)
{
    QPolygon p(a.size());
    for (int i=0; i<a.size(); ++i)
        p.replace(i, a.at(i));
    return p;
}


/*****************************************************************************
  QPolygon stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QPolygon

    Writes the point array, \a a to the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QPolygon &a)
{
    Q_UINT32 len = a.size();
    uint i;

    s << len;
    for (i = 0; i < len; ++i)
        s << a.at(i);
    return s;
}

/*!
    \relates QPolygon

    Reads a point array, \a a from the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QPolygon &a)
{
    Q_UINT32 len;
    uint i;

    s >> len;
    a.resize((int)len);
    QPointF p;
    for (i = 0; i < len; ++i) {
        s >> p;
        a.insert(i, p);
    }
    return s;
}
#endif //QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QPolygon &a)
{
#ifndef Q_NO_STREAMING_DEBUG
    dbg.nospace() << "QPolygon(";
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


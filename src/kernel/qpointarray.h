/****************************************************************************
**
** Definition of QPointArray class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPOINTARRAY_H
#define QPOINTARRAY_H

#ifndef QT_H
#include "qvector.h"
#include "qpoint.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)
//Q_TEMPLATE_EXTERN template class Q_EXPORT QMemArray<QPoint>;
#endif

class Q_EXPORT QPointArray : public QVector<QPoint>
{
public:
    inline QPointArray() {}
    inline ~QPointArray() {}
    inline QPointArray(int size) : QVector<QPoint>(size) {}
    inline QPointArray(const QPointArray &a) : QVector<QPoint>(a) {}
    QPointArray(const QRect &r, bool closed=FALSE);
    QPointArray(int nPoints, const QCOORD *points);

#ifndef QT_NO_COMPAT
    inline QPointArray copy() const
	{ return *this; }
    inline bool isNull() { return isEmpty(); }
#endif

    void translate(int dx, int dy);
    QRect boundingRect() const;

    void point(int i, int *x, int *y) const;
    QPoint point(int i) const;
    void setPoint(int index, int x, int y);
    void setPoint(int index, const QPoint &p);
    void setPoints(int nPoints, const QCOORD *points);
    void setPoints(int nPoints, int firstx, int firsty, ...);
    void putPoints(int index, int nPoints, const QCOORD *points);
    void putPoints(int index, int nPoints, int firstx, int firsty, ...);
    void putPoints(int index, int nPoints, const QPointArray & from, int fromIndex=0);

    void makeArc(int x, int y, int w, int h, int a1, int a2);
    void makeEllipse(int x, int y, int w, int h);
    void makeArc(int x, int y, int w, int h, int a1, int a2, const QWMatrix &matrix);
    QPointArray cubicBezier() const;

    void *shortPoints(int index = 0, int nPoints = -1) const;
    static void cleanBuffers();

private:
    // ### These are not thread safe.
    static int splen;
    static void *sp;
};


/*****************************************************************************
  QPointArray stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<(QDataStream &stream, const QPointArray &array);
Q_EXPORT QDataStream &operator>>(QDataStream &stream, QPointArray &array);
#endif

/*****************************************************************************
  Misc. QPointArray functions
 *****************************************************************************/

inline void QPointArray::setPoint(int index, const QPoint &p)
{
    (*this)[index] = p;
}

inline void QPointArray::setPoint(int index, int x, int y)
{
    (*this)[index] = QPoint(x, y);
}

inline QPoint QPointArray::point(int index) const
{
    return at(index);
}


#endif // QPOINTARRAY_H

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

#ifndef QPOINTARRAY_H
#define QPOINTARRAY_H

#include "qvector.h"
#include "qpoint.h"

class QMatrix;
class QRect;

class Q_GUI_EXPORT QPointArray : public QVector<QPoint>
{
public:
    inline QPointArray() {}
    inline ~QPointArray() {}
    inline QPointArray(int size) : QVector<QPoint>(size) {}
    inline QPointArray(const QPointArray &a) : QVector<QPoint>(a) {}
    inline QPointArray(const QVector<QPoint> &v) : QVector<QPoint>(v) {}
    QPointArray(const QRect &r, bool closed=false);
    QPointArray(int nPoints, const int *points);

#ifdef QT_COMPAT
    inline QT_COMPAT QPointArray copy() const { return *this; }
    inline QT_COMPAT bool isNull() { return isEmpty(); }
#endif

    void translate(int dx, int dy);
    void translate(const QPoint &offset);
    QRect boundingRect() const;

    void point(int i, int *x, int *y) const;
    QPoint point(int i) const;
    void setPoint(int index, int x, int y);
    void setPoint(int index, const QPoint &p);
    void setPoints(int nPoints, const int *points);
    void setPoints(int nPoints, int firstx, int firsty, ...);
    void putPoints(int index, int nPoints, const int *points);
    void putPoints(int index, int nPoints, int firstx, int firsty, ...);
    void putPoints(int index, int nPoints, const QPointArray & from, int fromIndex=0);

#ifdef QT_COMPAT
    QT_COMPAT void makeEllipse(int x, int y, int w, int h);
#ifndef QT_NO_WMATRIX
    QT_COMPAT void makeArc(int x, int y, int w, int h, int a1, int a2);
    QT_COMPAT void makeArc(int x, int y, int w, int h, int a1, int a2, const QMatrix &matrix);
#endif
#ifndef QT_NO_BEZIER
    QT_COMPAT QPointArray cubicBezier() const;
#endif
#endif
};

#ifndef QT_NO_DEBUG_OUTPUT
Q_GUI_EXPORT QDebug operator<<(QDebug, const QPointArray &);
#endif

/*****************************************************************************
  Misc. QPointArray functions
 *****************************************************************************/

inline void QPointArray::setPoint(int index, const QPoint &pt)
{ (*this)[index] = pt; }

inline void QPointArray::setPoint(int index, int x, int y)
{ (*this)[index] = QPoint(x, y); }

inline QPoint QPointArray::point(int index) const
{ return at(index); }

inline void QPointArray::translate(const QPoint &offset)
{ translate(offset.x(), offset.y()); }

#endif // QPOINTARRAY_H

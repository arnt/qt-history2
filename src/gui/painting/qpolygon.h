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

#ifndef QPOLYGON_H
#define QPOLYGON_H

#include "qvector.h"
#include "qpoint.h"
#include "qrect.h"

class QMatrix;
class QRect;

class Q_GUI_EXPORT QPolygon : public QVector<QPoint>
{
public:
    inline QPolygon() {}
    inline ~QPolygon() {}
    inline QPolygon(int size) : QVector<QPoint>(size) {}
    inline QPolygon(const QPolygon &a) : QVector<QPoint>(a) {}
    inline QPolygon(const QVector<QPoint> &v) : QVector<QPoint>(v) {}
    QPolygon(const QRect &r, bool closed=false);
    QPolygon(int nPoints, const int *points);

#ifdef QT_COMPAT
    inline QT_COMPAT QPolygon copy() const { return *this; }
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
    void putPoints(int index, int nPoints, const QPolygon & from, int fromIndex=0);

#ifdef QT_COMPAT
    QT_COMPAT void makeEllipse(int x, int y, int w, int h);
#ifndef QT_NO_WMATRIX
    QT_COMPAT void makeArc(int x, int y, int w, int h, int a1, int a2);
    QT_COMPAT void makeArc(int x, int y, int w, int h, int a1, int a2, const QMatrix &matrix);
#endif
#ifndef QT_NO_BEZIER
    QT_COMPAT QPolygon cubicBezier() const;
#endif
#endif
};

#ifndef QT_NO_DEBUG_OUTPUT
Q_GUI_EXPORT QDebug operator<<(QDebug, const QPolygon &);
#endif

/*****************************************************************************
  Misc. QPolygon functions
 *****************************************************************************/

inline void QPolygon::setPoint(int index, const QPoint &pt)
{ (*this)[index] = pt; }

inline void QPolygon::setPoint(int index, int x, int y)
{ (*this)[index] = QPoint(x, y); }

inline QPoint QPolygon::point(int index) const
{ return at(index); }

inline void QPolygon::translate(const QPoint &offset)
{ translate(offset.x(), offset.y()); }

class QRectF;

class Q_GUI_EXPORT QPolygonF : public QVector<QPointF>
{
public:
    inline QPolygonF() {}
    inline ~QPolygonF() {}
    inline QPolygonF(int size) : QVector<QPointF>(size) {}
    inline QPolygonF(const QPolygonF &a) : QVector<QPointF>(a) {}
    inline QPolygonF(const QVector<QPointF> &v) : QVector<QPointF>(v) {}
    QPolygonF(const QRectF &r);
    QPolygonF(const QPolygon &a);

    inline void translate(float dx, float dy);
    void translate(const QPointF &offset);

    QPolygon toPolygon() const;

    bool isClosed() const { return !isEmpty() && first() == last(); }

    QRectF boundingRect() const;
};

#ifndef QT_NO_DEBUG_OUTPUT
Q_GUI_EXPORT QDebug operator<<(QDebug, const QPolygonF &);
#endif

/*****************************************************************************
  QPolygonF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPolygonF &array);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPolygonF &array);
#endif

inline void QPolygonF::translate(float dx, float dy)
{ translate(QPointF(dx, dy)); }

#endif // QPOLYGONy_H

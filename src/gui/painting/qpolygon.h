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

class QPointArray;

class Q_GUI_EXPORT QPolygon : public QVector<QPointF>
{
public:
    inline QPolygon() {}
    inline ~QPolygon() {}
    inline QPolygon(int size) : QVector<QPointF>(size) {}
    inline QPolygon(const QPolygon &a) : QVector<QPointF>(a) {}
    inline QPolygon(const QVector<QPointF> &v) : QVector<QPointF>(v) {}
    QPolygon(const QRectF &r);

    inline void translate(float dx, float dy);
    void translate(const QPointF &offset);

    QPointArray toPointArray() const;
    static QPolygon fromPointArray(const QPointArray &a);

    QRectF boundingRect() const;
};

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QPolygon &);
#endif

/*****************************************************************************
  QPolygon stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPolygon &array);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPolygon &array);
#endif

inline void QPolygon::translate(float dx, float dy)
{ translate(QPointF(dx, dy)); }

#endif // QPOLYGONy_H

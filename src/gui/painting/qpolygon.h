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

    inline void translate(float dx, float dy);
    void translate(const QPointF &offset);

    QPointArray toPointArray() const;
    static QPolygonF fromPointArray(const QPointArray &a);

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

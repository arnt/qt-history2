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

#ifndef QPAINTERPATH_P_H
#define QPAINTERPATH_P_H

#include <qlist.h>
#include <qpainterpath.h>

class QPolygon;

class QPainterPathPrivate
{
    Q_DECLARE_PUBLIC(QPainterPath);
public:
    QPainterPathPrivate(QPainterPath *path) :
        q_ptr(path), cStart(0), fillRule(Qt::OddEvenFill)
    {
    }

    inline bool isClosed() const;
    inline void close();

    QPainterPath *q_ptr;
    int cStart;
    Qt::FillRule fillRule;
};

void qt_find_ellipse_coords(const QRectF &r, float angle, float length,
                            QPointF* startPoint, QPointF *endPoint);

inline bool QPainterPathPrivate::isClosed() const
{
    const QPainterPath::Element &first = q_func()->elements.at(cStart);
    const QPainterPath::Element &last = q_func()->elements.last();
    return first.x == last.x && first.y == last.y;
}

inline void QPainterPathPrivate::close()
{
    const QPainterPath::Element &first = q_func()->elements.at(cStart);
    const QPainterPath::Element &last = q_func()->elements.last();
    if (first.x != last.x || first.y != last.y)
        q_func()->lineTo(QPointF(first.x, first.y));
}

#endif //QPAINTERPATH_P_H

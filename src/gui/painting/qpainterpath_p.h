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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qlist.h>
#include <qregion.h>
#include <qpainterpath.h>

class QPolygonF;

class QPainterPathPrivate
{
    Q_DECLARE_PUBLIC(QPainterPath)
public:
    QPainterPathPrivate(QPainterPath *path) :
        q_ptr(path), cStart(0), fillRule(Qt::OddEvenFill)
    {
    }

    inline bool isClosed() const;
    inline void close();

    inline void makeDirty();

    QPainterPath *q_ptr;
    int cStart;
    Qt::FillRule fillRule;

    QRegion containsCache;
};

void Q_GUI_EXPORT qt_find_ellipse_coords(const QRectF &r, qreal angle, qreal length,
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

inline void QPainterPathPrivate::makeDirty()
{
    if (!containsCache.isEmpty())
        containsCache = QRegion();
}

#endif // QPAINTERPATH_P_H

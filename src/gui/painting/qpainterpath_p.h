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

class Q_GUI_EXPORT QSubpathIterator
{
public:
    QSubpathIterator(const QPainterPath *path);

    inline bool hasSubpath() const;
    inline QPointF nextSubpath();

    inline bool hasNext() const;
    inline QPainterPath::Element next();

private:
    const QPainterPath *m_path;
    int m_pos;
};

class Q_GUI_EXPORT QSubpathReverseIterator
{
public:
    QSubpathReverseIterator(const QPainterPath *path);

    inline bool hasSubpath() const;
    inline QPointF nextSubpath();

    inline bool hasNext() const;
    QPainterPath::Element next();

private:
    inline int indexOfSubpath(int startIndex);

    const QPainterPath *m_path;
    int m_pos;
    int m_start, m_end;
};

inline QSubpathIterator::QSubpathIterator(const QPainterPath *path)
     : m_path(path),
       m_pos(0)
{
}

inline bool QSubpathIterator::hasNext() const
{
    return m_pos < m_path->elementCount()
        && m_path->elementAt(m_pos).type != QPainterPath::MoveToElement;
}

inline bool QSubpathIterator::hasSubpath() const
{
    return m_pos + 1 < m_path->elementCount()
        && m_path->elementAt(m_pos).type == QPainterPath::MoveToElement;
}

inline QPointF QSubpathIterator::nextSubpath()
{
    Q_ASSERT(hasSubpath());
    const QPainterPath::Element &e = m_path->elementAt(m_pos);
    ++m_pos;
    Q_ASSERT(!hasSubpath());
    return QPointF(e.x, e.y);
}

inline QPainterPath::Element QSubpathIterator::next()
{
    Q_ASSERT(hasNext());
    return m_path->elementAt(m_pos++);
}

inline QSubpathReverseIterator::QSubpathReverseIterator(const QPainterPath *path)
     : m_path(path)
{
    m_start = 0;
    m_end = indexOfSubpath(1);
    m_pos = m_end;
}

inline bool QSubpathReverseIterator::hasSubpath() const
{
    return (m_start < m_end) & (m_pos == m_end);
}

inline bool QSubpathReverseIterator::hasNext() const
{
    return (m_start < m_end) & (m_pos < m_end) & (m_pos >= m_start);
}

inline QPointF QSubpathReverseIterator::nextSubpath()
{
    Q_ASSERT(hasSubpath());
    const QPainterPath::Element &e = m_path->elementAt(m_pos);
    --m_pos;
    return QPointF(e.x, e.y);
}

inline int QSubpathReverseIterator::indexOfSubpath(int i)
{
    int max = m_path->elementCount();
    while (i < max && m_path->elementAt(i).type != QPainterPath::MoveToElement)
        ++i;
    return i - 1;
}

#endif // QPAINTERPATH_P_H

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

class QPainterPathData : public QPainterPathPrivate
{
public:
    QPainterPathData() :
        cStart(0), fillRule(Qt::OddEvenFill)
    {
        ref = 1;
    }

    QPainterPathData(const QPainterPathData &other) :
        QPainterPathPrivate(), cStart(other.cStart), fillRule(other.fillRule), containsCache(other.containsCache)
    {
        ref = 1;
        elements = other.elements;
    }

    inline bool isClosed() const;
    inline void close();

    inline void makeDirty();

    int cStart;
    Qt::FillRule fillRule;

    QRegion containsCache;
};


void Q_GUI_EXPORT qt_find_ellipse_coords(const QRectF &r, qreal angle, qreal length,
                                         QPointF* startPoint, QPointF *endPoint);

inline bool QPainterPathData::isClosed() const
{
    const QPainterPath::Element &first = elements.at(cStart);
    const QPainterPath::Element &last = elements.last();
    return first.x == last.x && first.y == last.y;
}

inline void QPainterPathData::close()
{
    Q_ASSERT(ref == 1);
    const QPainterPath::Element &first = elements.at(cStart);
    const QPainterPath::Element &last = elements.last();
    if (first.x != last.x || first.y != last.y) {
        QPainterPath::Element e = { first.x, first.y, QPainterPath::LineToElement };
        elements << e;
    }
}

inline void QPainterPathData::makeDirty()
{
    if (!containsCache.isEmpty())
        containsCache = QRegion();
}

/*******************************************************************************
 * class QSubpathIterator
 * Iterates through a path, subpath by subpath, element by element.
 */
class QSubpathIterator
{
public:
    QSubpathIterator(const QPainterPath *path);

    inline bool hasSubpath() const;
    inline QPointF nextSubpath();

    inline bool hasNext() const;
    inline QPainterPath::Element next();

    inline int index() const { return m_pos; }

private:
    const QPainterPath *m_path;
    int m_pos;
};

/*******************************************************************************
 * QSubpathReverseIterator
 *
 * Iterates through all subpaths backwards. The order of the subpaths
 * are the same as their order in the original path.
 */
class QSubpathReverseIterator
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

/*******************************************************************************
 * QSubpathFlatIterator
 *
 * Iterates through all subpaths, element by element, but converts any
 * curve element to a number of line segments so all elements retrieved
 * are of type QPainterPath::LineToElement.
 */
class QSubpathFlatIterator
{
public:
    QSubpathFlatIterator(const QPainterPath *path);

    inline bool hasSubpath() const;
    inline QPointF nextSubpath();

    inline bool hasNext() const;
    QPainterPath::Element next();

private:
    const QPainterPath *m_path;
    int m_pos;
    QPolygonF m_curve;
    int m_curve_index;
};


/*******************************************************************************
 * QSubpathIterator inline implemetations
 */

inline QSubpathIterator::QSubpathIterator(const QPainterPath *path)
     : m_path(path),
       m_pos(0)
{
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

inline bool QSubpathIterator::hasNext() const
{
    return m_pos < m_path->elementCount()
        && m_path->elementAt(m_pos).type != QPainterPath::MoveToElement;
}

inline QPainterPath::Element QSubpathIterator::next()
{
    Q_ASSERT(hasNext());
    return m_path->elementAt(m_pos++);
}


/*******************************************************************************
 * QSubpathReverseIterator inline implementations
 */

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

inline QPointF QSubpathReverseIterator::nextSubpath()
{
    Q_ASSERT(hasSubpath());
    const QPainterPath::Element &e = m_path->elementAt(m_pos);
    --m_pos;
    return QPointF(e.x, e.y);
}

inline bool QSubpathReverseIterator::hasNext() const
{
    return (m_start < m_end) & (m_pos < m_end) & (m_pos >= m_start);
}

inline int QSubpathReverseIterator::indexOfSubpath(int i)
{
    int max = m_path->elementCount();
    while (i < max && m_path->elementAt(i).type != QPainterPath::MoveToElement)
        ++i;
    return i - 1;
}


/*******************************************************************************
 * QSubpathFlatIterator inline implemetations
 */

inline QSubpathFlatIterator::QSubpathFlatIterator(const QPainterPath *path)
     : m_path(path),
       m_pos(0),
       m_curve_index(-1)
{

}

inline bool QSubpathFlatIterator::hasNext() const
{
    return m_curve_index >= 0 || (m_pos < m_path->elementCount()
                                  && m_path->elementAt(m_pos).type != QPainterPath::MoveToElement);
}

inline bool QSubpathFlatIterator::hasSubpath() const
{
    return m_pos + 1 < m_path->elementCount()
        && m_path->elementAt(m_pos).type == QPainterPath::MoveToElement;
}

inline QPointF QSubpathFlatIterator::nextSubpath()
{
    Q_ASSERT(hasSubpath());
    const QPainterPath::Element &e = m_path->elementAt(m_pos);
    ++m_pos;
    Q_ASSERT(!hasSubpath());
    return QPointF(e.x, e.y);
}

#endif // QPAINTERPATH_P_H

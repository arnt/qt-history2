/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "QtGui/qwidget.h"
#include "QtGui/qtabbar.h"
#include "QtGui/qstyle.h"
#include "QtCore/qvariant.h"
#include "qdockarealayout_p.h"
#include "qdockwidget.h"
#include "qmainwindow.h"
#include "qwidgetanimator_p.h"
#include "qmainwindowlayout_p.h"
#include "qdockwidget_p.h"
#include <private/qlayoutengine_p.h>
#include <qdebug.h>

#include <qpainter.h>
#include <qstyleoption.h>

#ifndef QT_NO_DOCKWIDGET

enum { StateFlagVisible = 1, StateFlagFloating = 2 };

/******************************************************************************
** QDockAreaLayoutItem
*/

QDockAreaLayoutItem::QDockAreaLayoutItem(QLayoutItem *_widgetItem)
    : widgetItem(_widgetItem), subinfo(0), pos(0), size(-1), gap(false),
        keep_size(false)
{
}

QDockAreaLayoutItem::QDockAreaLayoutItem(QDockAreaLayoutInfo *_subinfo)
    : widgetItem(0), subinfo(_subinfo), pos(0), size(-1), gap(false),
        keep_size(false)
{
}

QDockAreaLayoutItem::QDockAreaLayoutItem(const QDockAreaLayoutItem &other)
    : widgetItem(other.widgetItem), subinfo(0), pos(other.pos),
        size(other.size), gap(other.gap), keep_size(other.keep_size)
{
    if (other.subinfo != 0)
        subinfo = new QDockAreaLayoutInfo(*other.subinfo);
    Q_ASSERT(widgetItem == 0 || subinfo == 0);
}

QDockAreaLayoutItem::~QDockAreaLayoutItem()
{
    delete subinfo;
}

bool QDockAreaLayoutItem::skip() const
{
    if (gap)
        return false;

    if (widgetItem != 0)
        return widgetItem->isEmpty();

    if (subinfo != 0) {
        for (int i = 0; i < subinfo->item_list.count(); ++i) {
            if (!subinfo->item_list.at(i).skip())
                return false;
        }
    }

    return true;
}

QSize QDockAreaLayoutItem::minimumSize() const
{
    if (widgetItem != 0)
        return widgetItem->minimumSize();
    if (subinfo != 0)
        return subinfo->minimumSize();
    return QSize(0, 0);
}

QSize QDockAreaLayoutItem::maximumSize() const
{
    if (widgetItem != 0)
        return widgetItem->maximumSize();
    if (subinfo != 0)
        return subinfo->maximumSize();
    return QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

bool QDockAreaLayoutItem::expansive(Qt::Orientation o) const
{
    if (gap)
        return false;
    if (widgetItem != 0)
        return ((widgetItem->expandingDirections() & o) == o);
    if (subinfo != 0)
        return subinfo->expansive(o);
    return false;
}

QSize QDockAreaLayoutItem::sizeHint() const
{
    if (widgetItem != 0)
        return widgetItem->sizeHint();
    if (subinfo != 0)
        return subinfo->sizeHint();
    return QSize(-1, -1);
}

QDockAreaLayoutItem
    &QDockAreaLayoutItem::operator = (const QDockAreaLayoutItem &other)
{
    widgetItem = other.widgetItem;
    if (other.subinfo == 0)
        subinfo = 0;
    else
        subinfo = new QDockAreaLayoutInfo(*other.subinfo);
    pos = other.pos;
    size = other.size;
    gap = other.gap;

    return *this;
}

/******************************************************************************
** QDockAreaLayoutInfo
*/

static quintptr tabId(const QDockAreaLayoutItem &item)
{
    if (item.widgetItem == 0)
        return 0;
    return reinterpret_cast<quintptr>(item.widgetItem->widget());
}

QDockAreaLayoutInfo::QDockAreaLayoutInfo()
    : sep(0), dockPos(QInternal::LeftDock), o(Qt::Horizontal), rect(0, 0, -1, -1), mainWindow(0)
#ifndef QT_NO_TABBAR
    , tabbed(false), tabBar(0), tabBarShape(QTabBar::RoundedSouth)
#endif
{
}

QDockAreaLayoutInfo::QDockAreaLayoutInfo(int _sep, QInternal::DockPosition _dockPos,
                                            Qt::Orientation _o, int tbshape,
                                            QMainWindow *window)
    : sep(_sep), dockPos(_dockPos), o(_o), rect(0, 0, -1, -1), mainWindow(window)
#ifndef QT_NO_TABBAR
    , tabbed(false), tabBar(0), tabBarShape(static_cast<QTabBar::Shape>(tbshape))
#endif
{
#ifdef QT_NO_TABBAR
    Q_UNUSED(tbshape);
#endif
}

QSize QDockAreaLayoutInfo::size() const
{
    return isEmpty() ? QSize(0, 0) : rect.size();
}

void QDockAreaLayoutInfo::clear()
{
    item_list.clear();
    rect = QRect(0, 0, -1, -1);
#ifndef QT_NO_TABBAR
    tabbed = false;
    tabBar = 0;
#endif
}

bool QDockAreaLayoutInfo::isEmpty() const
{
    return next(-1) == -1;
}

QSize QDockAreaLayoutInfo::minimumSize() const
{
    if (isEmpty())
        return QSize(0, 0);

    int a = 0, b = 0;
    bool first = true;
    for (int i = 0; i < item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.skip())
            continue;

        QSize min_size = item.minimumSize();
#ifndef QT_NO_TABBAR
        if (tabbed) {
            a = qMax(a, pick(o, min_size));
        } else
#endif
        {
            if (!first)
                a += sep;
            a += pick(o, min_size);
        }
        b = qMax(b, perp(o, min_size));

        first = false;
    }

    QSize result;
    rpick(o, result) = a;
    rperp(o, result) = b;

#ifndef QT_NO_TABBAR
    if (tabbed) {
        QSize tbm = tabBarMinimumSize();
        switch (tabBarShape) {
            case QTabBar::RoundedNorth:
            case QTabBar::RoundedSouth:
                result.rheight() += tbm.height();
                result.rwidth() = qMax(tbm.width(), result.width());
                break;
            case QTabBar::RoundedEast:
            case QTabBar::RoundedWest:
                result.rheight() = qMax(tbm.height(), result.height());
                result.rwidth() += tbm.width();
                break;
            default:
                break;
        }
    }
#endif // QT_NO_TABBAR

    return result;
}

QSize QDockAreaLayoutInfo::maximumSize() const
{
    if (isEmpty())
        return QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    int a = 0, b = QWIDGETSIZE_MAX;
#ifndef QT_NO_TABBAR
    if (tabbed)
        a = QWIDGETSIZE_MAX;
#endif

    int min_perp = 0;

    bool first = true;
    for (int i = 0; i < item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.skip())
            continue;

        QSize max_size = item.maximumSize();
        min_perp = qMax(min_perp, perp(o, item.minimumSize()));

#ifndef QT_NO_TABBAR
        if (tabbed) {
            a = qMin(a, pick(o, max_size));
        } else
#endif
        {
            if (!first)
                a += sep;
            a += pick(o, max_size);
        }
        b = qMin(b, perp(o, max_size));

        a = qMin(a, int(QWIDGETSIZE_MAX));
        b = qMin(b, int(QWIDGETSIZE_MAX));

        first = false;
    }

    b = qMax(b, min_perp);

    QSize result;
    rpick(o, result) = a;
    rperp(o, result) = b;

#ifndef QT_NO_TABBAR
    if (tabbed) {
        QSize tbh = tabBarSizeHint();
        switch (tabBarShape) {
            case QTabBar::RoundedNorth:
            case QTabBar::RoundedSouth:
                result.rheight() += tbh.height();
                break;
            case QTabBar::RoundedEast:
            case QTabBar::RoundedWest:
                result.rwidth() += tbh.width();
                break;
            default:
                break;
        }
    }
#endif // QT_NO_TABBAR

    return result;
}

QSize QDockAreaLayoutInfo::sizeHint() const
{
    if (isEmpty())
        return QSize(0, 0);

    int a = 0, b = 0;
    bool prev_gap = false;
    bool first = true;
    int min_perp = 0;
    int max_perp = QWIDGETSIZE_MAX;
    for (int i = 0; i < item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.skip())
            continue;

        bool gap = item.gap;

        QSize size_hint = item.sizeHint();
        min_perp = qMax(min_perp, perp(o, item.minimumSize()));
        max_perp = qMin(max_perp, perp(o, item.maximumSize()));

#ifndef QT_NO_TABBAR
        if (tabbed) {
            a = qMax(a, gap ? item.size : pick(o, size_hint));
        } else
#endif
        {
            if (!first && !gap && !prev_gap)
                a += sep;
            a += gap ? item.size : pick(o, size_hint);
        }
        b = qMax(b, perp(o, size_hint));

        prev_gap = gap;
        first = false;
    }

    max_perp = qMax(max_perp, min_perp);
    b = qMax(b, min_perp);
    b = qMin(b, max_perp);

    QSize result;
    rpick(o, result) = a;
    rperp(o, result) = b;

#ifndef QT_NO_TABBAR
    if (tabbed) {
        QSize tbh = tabBarSizeHint();
        switch (tabBarShape) {
            case QTabBar::RoundedNorth:
            case QTabBar::RoundedSouth:
                result.rheight() += tbh.height();
                result.rwidth() = qMax(tbh.width(), result.width());
                break;
            case QTabBar::RoundedEast:
            case QTabBar::RoundedWest:
                result.rheight() = qMax(tbh.height(), result.height());
                result.rwidth() += tbh.width();
                break;
            default:
                break;
        }
    }
#endif // QT_NO_TABBAR

    return result;
}

bool QDockAreaLayoutInfo::expansive(Qt::Orientation o) const
{
    for (int i = 0; i < item_list.size(); ++i) {
        if (item_list.at(i).expansive(o))
            return true;
    }
    return false;
}

/* QDockAreaLayoutInfo::maximumSize() doesn't return the real max size. For example,
   if the layout is empty, it returns QWIDGETSIZE_MAX. This is so that empty dock areas
   don't constrain the size of the QMainWindow, but sometimes we really need to know the
   maximum size. Also, these functions take into account widgets that want to keep their
   size (f.ex. when they are hidden and then shown, they should not change size).
*/

static int realMinSize(const QDockAreaLayoutInfo &info)
{
    int result = 0;
    bool first = true;
    for (int i = 0; i < info.item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = info.item_list.at(i);
        if (item.skip())
            continue;

        int min = 0;
        if (item.keep_size && item.size != -1)
            min = item.size;
        else
            min = pick(info.o, item.minimumSize());

        if (!first)
            result += info.sep;
        result += min;

        first = false;
    }

    return result;
}

static int realMaxSize(const QDockAreaLayoutInfo &info)
{
    int result = 0;
    bool first = true;
    for (int i = 0; i < info.item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = info.item_list.at(i);
        if (item.skip())
            continue;

        int max = 0;
        if (item.keep_size && item.size != -1)
            max = item.size;
        else
            max = pick(info.o, item.maximumSize());

        if (!first)
            result += info.sep;
        result += max;

        if (result >= QWIDGETSIZE_MAX)
            return QWIDGETSIZE_MAX;

        first = false;
    }

    return result;
}

void QDockAreaLayoutInfo::fitItems()
{
#ifndef QT_NO_TABBAR
    if (tabbed) {
        return;
    }
#endif

    QVector<QLayoutStruct> layout_struct_list(item_list.size()*2);
    int j = 0;

    int size = pick(o, rect.size());
    int min_size = realMinSize(*this);
    int max_size = realMaxSize(*this);
    int last_index = -1;

    bool prev_gap = false;
    bool first = true;
    for (int i = 0; i < item_list.size(); ++i) {
        QDockAreaLayoutItem &item = item_list[i];
        if (item.skip())
            continue;

        bool gap = item.gap;
        if (!first && !gap && !prev_gap) {
            QLayoutStruct &ls = layout_struct_list[j++];
            ls.init();
            ls.minimumSize = sep;
            ls.maximumSize = sep;
            ls.sizeHint = sep;
            ls.empty = false;
        }

        if (item.keep_size) {
            // Check if the item can keep its size, without violating size constraints
            // of other items.

            if (size < min_size) {
                // There is too little space to keep this widget's size
                item.keep_size = false;
                min_size -= item.size;
                min_size += pick(o, item.minimumSize());
                min_size = qMax(0, min_size);
            } else if (size > max_size) {
                // There is too much space to keep this widget's size
                item.keep_size = false;
                max_size -= item.size;
                max_size += pick(o, item.maximumSize());
                max_size = qMin<int>(QWIDGETSIZE_MAX, max_size);
            }
        }

        last_index = j;
        QLayoutStruct &ls = layout_struct_list[j++];
        ls.init();
        ls.empty = false;
        if (gap || item.keep_size) {
            ls.minimumSize = ls.maximumSize = ls.sizeHint = item.size;
            ls.expansive = false;
            ls.stretch = 0;
        } else {
            ls.maximumSize = pick(o, item.maximumSize());
            ls.expansive = item.expansive(o);
            ls.minimumSize = pick(o, item.minimumSize());
            ls.sizeHint = item.size == -1 ? pick(o, item.sizeHint()) : item.size;
            ls.stretch = ls.expansive ? ls.sizeHint : 0;
        }

        item.keep_size = false;
        prev_gap = gap;
        first = false;
    }
    layout_struct_list.resize(j);

    // If there is more space than the widgets can take (due to maximum size constraints),
    // we detect it here and stretch the last widget to take up the rest of the space.
    if (size > max_size && last_index != -1) {
        layout_struct_list[last_index].maximumSize = QWIDGETSIZE_MAX;
        layout_struct_list[last_index].expansive = true;
    }

    qGeomCalc(layout_struct_list, 0, j, pick(o, rect.topLeft()), size, 0);

    j = 0;
    prev_gap = false;
    first = true;
    for (int i = 0; i < item_list.size(); ++i) {
        QDockAreaLayoutItem &item = item_list[i];
        if (item.skip())
            continue;

        bool gap = item.gap;
        if (!first && !gap && !prev_gap)
            ++j;

        const QLayoutStruct &ls = layout_struct_list.at(j++);
        item.size = ls.size;
        item.pos = ls.pos;

        if (item.subinfo != 0) {
            item.subinfo->rect = itemRect(i);
            item.subinfo->fitItems();
        }

        prev_gap = gap;
        first = false;
    }
}

static QInternal::DockPosition dockPos(const QRect &rect, const QPoint &_pos,
                                        Qt::Orientation o,
                                        bool nestingEnabled,
                                        QDockAreaLayoutInfo::TabMode tabMode)
{
    if (tabMode == QDockAreaLayoutInfo::ForceTabs)
        return QInternal::DockCount;

    QPoint pos = _pos - rect.topLeft();

    int x = pos.x();
    int y = pos.y();
    int w = rect.width();
    int h = rect.height();

    if (tabMode != QDockAreaLayoutInfo::NoTabs) {
        // is it in the center?
        if (nestingEnabled) {
        /*             2/3
                +--------------+
                |              |
                |   CCCCCCCC   |
           2/3  |   CCCCCCCC   |
                |   CCCCCCCC   |
                |              |
                +--------------+     */

            QRect center(w/6, h/6, 2*w/3, 2*h/3);
            if (center.contains(pos))
                return QInternal::DockCount;
        } else if (o == Qt::Horizontal) {
        /*             2/3
                +--------------+
                |   CCCCCCCC   |
                |   CCCCCCCC   |
                |   CCCCCCCC   |
                |   CCCCCCCC   |
                |   CCCCCCCC   |
                +--------------+     */

            if (x > w/6 && x < w*5/6)
                return QInternal::DockCount;
        } else {
        /*
                +--------------+
                |              |
           2/3  |CCCCCCCCCCCCCC|
                |CCCCCCCCCCCCCC|
                |              |
                +--------------+     */
            if (y > h/6 && y < 5*h/6)
                return QInternal::DockCount;
        }
    }

    // not in the center. which edge?
    if (nestingEnabled) {
        if (o == Qt::Horizontal) {
    /*       1/3  1/3 1/3
            +------------+     (we've already ruled out the center)
            |LLLLTTTTRRRR|
            |LLLLTTTTRRRR|
            |LLLLBBBBRRRR|
            |LLLLBBBBRRRR|
            +------------+    */

            if (x < w/3)
                return QInternal::LeftDock;
            if (x > 2*w/3)
                return QInternal::RightDock;
            if (y < h/2)
                return QInternal::TopDock;
            return QInternal::BottomDock;
        } else {
    /*      +------------+     (we've already ruled out the center)
        1/3 |TTTTTTTTTTTT|
            |LLLLLLRRRRRR|
        1/3 |LLLLLLRRRRRR|
        1/3 |BBBBBBBBBBBB|
            +------------+    */

            if (y < h/3)
                return QInternal::TopDock;
            if (y > 2*h/3)
                return QInternal::BottomDock;
            if (x < w/2)
                return QInternal::LeftDock;
            return QInternal::RightDock;
        }
    } else {
        if (o == Qt::Horizontal) {
            return x < w/2
                    ? QInternal::LeftDock
                    : QInternal::RightDock;
        } else {
            return y < h/2
                    ? QInternal::TopDock
                    : QInternal::BottomDock;
        }
    }
}

QList<int> QDockAreaLayoutInfo::gapIndex(const QPoint& _pos,
                        bool nestingEnabled, TabMode tabMode) const
{
    QList<int> result;
    QRect item_rect;
    int item_index = 0;

#ifndef QT_NO_TABBAR
    if (tabbed) {
        item_rect = tabContentRect();
    } else
#endif
    {
        int pos = pick(o, _pos);

        int last = -1;
        for (int i = 0; i < item_list.size(); ++i) {
            const QDockAreaLayoutItem &item = item_list.at(i);
            if (item.skip())
                continue;

            last = i;

            if (item.pos + item.size < pos)
                continue;

            if (item.subinfo != 0
#ifndef QT_NO_TABBAR
                && !item.subinfo->tabbed
#endif
                ) {
                result = item.subinfo->gapIndex(_pos, nestingEnabled,
                                                    tabMode);
                result.prepend(i);
                return result;
            }

            item_rect = itemRect(i);
            item_index = i;
            break;
        }

        if (item_rect.isNull()) {
            result.append(last + 1);
            return result;
        }
    }

    Q_ASSERT(!item_rect.isNull());

    QInternal::DockPosition dock_pos
        = ::dockPos(item_rect, _pos, o, nestingEnabled, tabMode);

    switch (dock_pos) {
        case QInternal::LeftDock:
            if (o == Qt::Horizontal)
                result << item_index;
            else
                result << item_index << 0; // this subinfo doesn't exist yet, but insertGap()
                                           // handles this by inserting it
            break;
        case QInternal::RightDock:
            if (o == Qt::Horizontal)
                result << item_index + 1;
            else
                result << item_index << 1;
            break;
        case QInternal::TopDock:
            if (o == Qt::Horizontal)
                result << item_index << 0;
            else
                result << item_index;
            break;
        case QInternal::BottomDock:
            if (o == Qt::Horizontal)
                result << item_index << 1;
            else
                result << item_index + 1;
            break;
        case  QInternal::DockCount:
            result << (-item_index - 1) << 0;   // negative item_index means "on top of"
                                                // -item_index - 1, insertGap()
                                                // will insert a tabbed subinfo
            break;
        default:
            break;
    }

    return result;
}

static inline int shrink(QLayoutStruct &ls, int delta)
{
    if (ls.empty)
        return 0;
    int old_size = ls.size;
    ls.size = qMax(ls.size - delta, ls.minimumSize);
    return old_size - ls.size;
}

static inline int grow(QLayoutStruct &ls, int delta)
{
    if (ls.empty)
        return 0;
    int old_size = ls.size;
    ls.size = qMin(ls.size + delta, ls.maximumSize);
    return ls.size - old_size;
}

static int separatorMove(QVector<QLayoutStruct> &list, int index, int delta, int sep)
{
    // adjust sizes
    int pos = -1;
    for (int i = 0; i < list.size(); ++i) {
        const QLayoutStruct &ls = list.at(i);
        if (!ls.empty) {
            pos = ls.pos;
            break;
        }
    }
    if (pos == -1)
        return 0;

    if (delta > 0) {
        int growlimit = 0;
        for (int i = 0; i<=index; ++i) {
            const QLayoutStruct &ls = list.at(i);
            if (ls.empty)
                continue;
            if (ls.maximumSize == QLAYOUTSIZE_MAX) {
                growlimit = QLAYOUTSIZE_MAX;
                break;
            }
            growlimit += ls.maximumSize - ls.size;
        }
        if (delta > growlimit)
            delta = growlimit;

        int d = 0;
        for (int i = index + 1; d < delta && i < list.count(); ++i)
            d += shrink(list[i], delta - d);
        delta = d;
        d = 0;
        for (int i = index; d < delta && i >= 0; --i)
            d += grow(list[i], delta - d);
    } else if (delta < 0) {
        int growlimit = 0;
        for (int i = index + 1; i < list.count(); ++i) {
            const QLayoutStruct &ls = list.at(i);
            if (ls.empty)
                continue;
            if (ls.maximumSize == QLAYOUTSIZE_MAX) {
                growlimit = QLAYOUTSIZE_MAX;
                break;
            }
            growlimit += ls.maximumSize - ls.size;
        }
        if (-delta > growlimit)
            delta = -growlimit;

        int d = 0;
        for (int i = index; d < -delta && i >= 0; --i)
            d += shrink(list[i], -delta - d);
        delta = -d;
        d = 0;
        for (int i = index + 1; d < -delta && i < list.count(); ++i)
            d += grow(list[i], -delta - d);
    }

    // adjust positions
    bool first = true;
    for (int i = 0; i < list.size(); ++i) {
        QLayoutStruct &ls = list[i];
        if (ls.empty)
            continue;
        if (!first)
            pos += sep;
        ls.pos = pos;
        pos += ls.size;
        first = false;
    }

    return delta;
}

int QDockAreaLayoutInfo::separatorMove(int index, int delta, QVector<QLayoutStruct> *cache)
{
#ifndef QT_NO_TABBAR
    Q_ASSERT(!tabbed);
#endif

    if (cache->isEmpty()) {
        QVector<QLayoutStruct> &list = *cache;
        list.resize(item_list.size());
        for (int i = 0; i < item_list.size(); ++i) {
            const QDockAreaLayoutItem &item = item_list.at(i);
            QLayoutStruct &ls = list[i];
            Q_ASSERT(!item.gap);
            if (item.skip()) {
                ls.empty = true;
            } else {
                ls.empty = false;
                ls.pos = item.pos;
                ls.size = item.size;
                ls.minimumSize = pick(o, item.minimumSize());
                ls.maximumSize = pick(o, item.maximumSize());
            }
        }
    }

    QVector<QLayoutStruct> list = *cache;

    delta = ::separatorMove(list, index, delta, sep);

    for (int i = 0; i < item_list.size(); ++i) {
        QDockAreaLayoutItem &item = item_list[i];
        if (item.skip())
            continue;
        QLayoutStruct &ls = list[i];
        item.size = ls.size;
        item.pos = ls.pos;

        if (item.subinfo != 0) {
            item.subinfo->rect = itemRect(i);
            item.subinfo->fitItems();
        }
    }

    return delta;
}

void QDockAreaLayoutInfo::unnest(int index)
{
    QDockAreaLayoutItem &item = item_list[index];
    if (item.subinfo == 0)
        return;
    if (item.subinfo->item_list.count() > 1)
        return;
    Q_ASSERT(item.subinfo->item_list.count() != 0);

    if (item.subinfo->item_list.count() == 1) {
        QDockAreaLayoutItem &child = item.subinfo->item_list.first();
        if (child.widgetItem != 0) {
            item.widgetItem = child.widgetItem;
            delete item.subinfo;
            item.subinfo = 0;
        } else if (child.subinfo != 0) {
            QDockAreaLayoutInfo *tmp = item.subinfo;
            item.subinfo = child.subinfo;
            child.subinfo = 0;
            tmp->item_list.clear();
            delete tmp;
        }
    }
}

void QDockAreaLayoutInfo::remove(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());

    if (path.count() > 1) {
        int index = path.takeFirst();
        QDockAreaLayoutItem &item = item_list[index];
        Q_ASSERT(item.subinfo != 0);
        item.subinfo->remove(path);
        unnest(index);
    } else {
        int index = path.first();
        item_list.removeAt(index);
    }
}

QLayoutItem *QDockAreaLayoutInfo::plug(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());

    int index = path.takeFirst();
    if (index < 0)
        index = -index - 1;

    if (!path.isEmpty()) {
        const QDockAreaLayoutItem &item = item_list.at(index);
        Q_ASSERT(item.subinfo != 0);
        return item.subinfo->plug(path);
    }

    QDockAreaLayoutItem &item = item_list[index];

    Q_ASSERT(item.widgetItem != 0);
    Q_ASSERT(item.gap);
    item.gap = false;

    QRect result;

#ifndef QT_NO_TABBAR
    if (tabbed) {
    } else
#endif
    {
        int prev = this->prev(index);
        int next = this->next(index);

        if (prev != -1 && !item_list.at(prev).gap) {
            item.pos += sep;
            item.size -= sep;
        }
        if (next != -1 && !item_list.at(next).gap)
            item.size -= sep;

        QPoint pos;
        rpick(o, pos) = item.pos;
        rperp(o, pos) = perp(o, rect.topLeft());
        QSize s;
        rpick(o, s) = item.size;
        rperp(o, s) = perp(o, rect.size());
        result = QRect(pos, s);
    }

    return item.widgetItem;
}

QLayoutItem *QDockAreaLayoutInfo::unplug(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());

    if (path.count() > 1) {
        int index = path.takeFirst();
        const QDockAreaLayoutItem &item = item_list.at(index);
        Q_ASSERT(item.subinfo != 0);
        return item.subinfo->unplug(path);
    }

    int index = path.first();
    QDockAreaLayoutItem &item = item_list[index];
    int prev = this->prev(index);
    int next = this->next(index);

    Q_ASSERT(!item.gap);
    item.gap = true;

#ifndef QT_NO_TABBAR
    if (tabbed) {
    } else
#endif
    {
        if (prev != -1 && !item_list.at(prev).gap) {
            item.pos -= sep;
            item.size += sep;
        }
        if (next != -1 && !item_list.at(next).gap)
            item.size += sep;
    }

    return item.widgetItem;
}

#ifndef QT_NO_TABBAR

quintptr QDockAreaLayoutInfo::currentTabId() const
{
    if (!tabbed || tabBar == 0)
        return 0;

    int index = tabBar->currentIndex();
    if (index == -1)
        return 0;

    return qvariant_cast<quintptr>(tabBar->tabData(index));
}

void QDockAreaLayoutInfo::setCurrentTab(QWidget *widget)
{
    setCurrentTabId(reinterpret_cast<quintptr>(widget));
}

void QDockAreaLayoutInfo::setCurrentTabId(quintptr id)
{
    if (!tabbed || tabBar == 0)
        return;

    for (int i = 0; i < tabBar->count(); ++i) {
        if (qvariant_cast<quintptr>(tabBar->tabData(i)) == id) {
            tabBar->setCurrentIndex(i);
            return;
        }
    }
}

#endif // QT_NO_TABBAR

static QRect dockedGeometry(QWidget *widget)
{
    int titleHeight = 0;

    QDockWidgetLayout *layout
        = qobject_cast<QDockWidgetLayout*>(widget->layout());
    if(layout != 0 && layout->nativeWindowDeco())
        titleHeight = layout->titleHeight();

    QRect result = widget->geometry();
    result.adjust(0, -titleHeight, 0, 0);
    return result;
}

bool QDockAreaLayoutInfo::insertGap(QList<int> path, QLayoutItem *dockWidgetItem)
{
    Q_ASSERT(!path.isEmpty());

    bool insert_tabbed = false;
    int index = path.takeFirst();
    if (index < 0) {
        insert_tabbed = true;
        index = -index - 1;
    }

//    dump(qDebug() << "insertGap() before:" << index << tabIndex, *this, QString());

    if (!path.isEmpty()) {
        QDockAreaLayoutItem &item = item_list[index];

        if (item.subinfo == 0
#ifndef QT_NO_TABBAR
            || item.subinfo->tabbed && !insert_tabbed
#endif
            ) {

            // this is not yet a nested layout - make it

            QDockAreaLayoutInfo *subinfo = item.subinfo;
            QLayoutItem *widgetItem = item.widgetItem;
            QRect r = subinfo == 0 ? dockedGeometry(widgetItem->widget()) : subinfo->rect;

            Qt::Orientation opposite = o == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal;
#ifdef QT_NO_TABBAR
            const int tabBarShape = 0;
#endif
            QDockAreaLayoutInfo *new_info
                = new QDockAreaLayoutInfo(sep, dockPos, opposite, tabBarShape, mainWindow);

            item.subinfo = new_info;
            item.widgetItem = 0;

            QDockAreaLayoutItem new_item
                = widgetItem == 0
                    ? QDockAreaLayoutItem(subinfo)
                    : QDockAreaLayoutItem(widgetItem);
            new_item.size = pick(opposite, r.size());
            new_item.pos = pick(opposite, r.topLeft());
            new_info->item_list.append(new_item);
#ifndef QT_NO_TABBAR
            if (insert_tabbed) {
                new_info->tabbed = true;
            }
#endif
        }

        bool result = item.subinfo->insertGap(path, dockWidgetItem);
        return result;
    }

    // create the gap item
    QDockAreaLayoutItem gap_item;
    gap_item.gap = true;
    gap_item.widgetItem = dockWidgetItem;   // so minimumSize(), maximumSize() and
                                            // sizeHint() will work
#ifndef QT_NO_TABBAR
    if (!tabbed)
#endif
    {
        int prev = this->prev(index);
        int next = this->next(index - 1);
        // find out how much space we have in the layout
        int space = 0;
        if (isEmpty()) {
            // I am an empty dock area, therefore I am a top-level dock area.
            switch (dockPos) {
                case QInternal::LeftDock:
                case QInternal::RightDock:
                    if (o == Qt::Vertical) {
                        // the "size" is the height of the dock area (remember we are empty)
                        space = pick(Qt::Vertical, rect.size());
                    } else {
                        space = pick(Qt::Horizontal, dockWidgetItem->widget()->size());
                    }
                    break;
                case QInternal::TopDock:
                case QInternal::BottomDock:
                default:
                    if (o == Qt::Horizontal) {
                        // the "size" is width of the dock area
                        space = pick(Qt::Horizontal, rect.size());
                    } else {
                        space = pick(Qt::Vertical, dockWidgetItem->widget()->size());
                    }
                    break;
            }
        } else {
            for (int i = 0; i < item_list.count(); ++i) {
                const QDockAreaLayoutItem &item = item_list.at(i);
                if (item.skip())
                    continue;
                Q_ASSERT(!item.gap);
                space += item.size - pick(o, item.minimumSize());
            }
        }

        // find the actual size of the gap
        int gap_size = 0;
        int sep_size = 0;
        if (isEmpty()) {
            gap_size = space;
            sep_size = 0;
        } else {
            QRect r = dockedGeometry(dockWidgetItem->widget());
            gap_size = pick(o, r.size());
            if (prev != -1 && !item_list.at(prev).gap)
                sep_size += sep;
            if (next != -1 && !item_list.at(next).gap)
                sep_size += sep;
        }
        if (gap_size + sep_size > space)
            gap_size = pick(o, gap_item.minimumSize());
        gap_item.size = gap_size + sep_size;
    }

    // finally, insert the gap
    item_list.insert(index, gap_item);

//    dump(qDebug() << "insertGap() after:" << index << tabIndex, *this, QString());

    return true;
}

QDockAreaLayoutInfo *QDockAreaLayoutInfo::info(QWidget *widget)
{
    for (int i = 0; i < item_list.count(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.skip())
            continue;

#ifndef QT_NO_TABBAR
        if (tabbed && widget == tabBar)
            return this;
#endif

        if (item.widgetItem != 0 && item.widgetItem->widget() == widget)
            return this;

        if (item.subinfo != 0) {
            if (QDockAreaLayoutInfo *result = item.subinfo->info(widget))
                return result;
        }
    }

    return 0;
}

QDockAreaLayoutInfo *QDockAreaLayoutInfo::info(QList<int> path)
{
    int index = path.takeFirst();
    if (index < 0)
        index = -index - 1;
    if (index >= item_list.count())
        return this;
    if (path.isEmpty() || item_list.at(index).subinfo == 0)
        return this;
    return item_list.at(index).subinfo->info(path);
}

QRect QDockAreaLayoutInfo::itemRect(int index) const
{
    const QDockAreaLayoutItem &item = item_list.at(index);

    if (item.skip())
        return QRect();

    QRect result;

#ifndef QT_NO_TABBAR
    if (tabbed) {
        if (tabId(item) == currentTabId())
            result = tabContentRect();
    } else
#endif
    {
        QPoint pos;
        rpick(o, pos) = item.pos;
        rperp(o, pos) = perp(o, rect.topLeft());
        QSize s;
        rpick(o, s) = item.size;
        rperp(o, s) = perp(o, rect.size());
        result = QRect(pos, s);
    }

    return result;
}

QRect QDockAreaLayoutInfo::itemRect(QList<int> path) const
{
    Q_ASSERT(!path.isEmpty());

    if (path.count() > 1) {
        const QDockAreaLayoutItem &item = item_list.at(path.takeFirst());
        Q_ASSERT(item.subinfo != 0);
        return item.subinfo->itemRect(path);
    }

    return itemRect(path.first());
}

QRect QDockAreaLayoutInfo::separatorRect(int index) const
{
#ifndef QT_NO_TABBAR
    if (tabbed)
        return QRect();
#endif

    const QDockAreaLayoutItem &item = item_list.at(index);
    if (item.skip())
        return QRect();

    QPoint pos = rect.topLeft();
    rpick(o, pos) = item.pos + item.size;
    QSize s = rect.size();
    rpick(o, s) = sep;

    return QRect(pos, s);
}

QRect QDockAreaLayoutInfo::separatorRect(QList<int> path) const
{
    Q_ASSERT(!path.isEmpty());

    if (path.count() > 1) {
        const QDockAreaLayoutItem &item = item_list.at(path.takeFirst());
        Q_ASSERT(item.subinfo != 0);
        return item.subinfo->separatorRect(path);
    }
    return separatorRect(path.first());
}

QList<int> QDockAreaLayoutInfo::findSeparator(const QPoint &_pos) const
{
#ifndef QT_NO_TABBAR
    if (tabbed)
        return QList<int>();
#endif

    int pos = pick(o, _pos);

    for (int i = 0; i < item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.skip() || item.gap)
            continue;

        if (item.pos + item.size > pos) {
            if (item.subinfo != 0) {
                QList<int> result = item.subinfo->findSeparator(_pos);
                if (!result.isEmpty()) {
                    result.prepend(i);
                    return result;
                } else {
                    return QList<int>();
                }
            }
        }

        int next = this->next(i);
        if (next == -1 || item_list.at(next).gap)
            continue;

        if (pos > item.pos + item.size && item.pos + item.size + sep > pos) {
            QList<int> result;
            result.append(i);
            return result;
        }
    }

    return QList<int>();
}

QList<int> QDockAreaLayoutInfo::indexOf(QWidget *widget) const
{
    for (int i = 0; i < item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);

        if (item.subinfo != 0) {
            QList<int> result = item.subinfo->indexOf(widget);
            if (!result.isEmpty()) {
                result.prepend(i);
                return result;
            }
            continue;
        }

        if (!item.gap && item.widgetItem->widget() == widget) {
            QList<int> result;
            result << i;
            return result;
        }
    }

    return QList<int>();
}

QMainWindowLayout *QDockAreaLayoutInfo::mainWindowLayout() const
{
    QMainWindowLayout *result = qobject_cast<QMainWindowLayout*>(mainWindow->layout());
    Q_ASSERT(result != 0);
    return result;
}

void QDockAreaLayoutInfo::apply(bool animate)
{
    QWidgetAnimator *widgetAnimator = mainWindowLayout()->widgetAnimator;

#ifndef QT_NO_TABBAR
    if (tabbed) {
        QRect tab_rect;
        QSize tbh = tabBarSizeHint();

        if (tabBarVisible) {
            switch (tabBarShape) {
                case QTabBar::RoundedNorth:
                    tab_rect = QRect(rect.left(), rect.top(), rect.width(), tbh.height());
                    break;
                case QTabBar::RoundedSouth:
                    tab_rect = QRect(rect.left(), rect.bottom() - tbh.height() + 1,
                                        rect.width(), tbh.height());
                    break;
                case QTabBar::RoundedEast:
                    tab_rect = QRect(rect.right() - tbh.width() + 1, rect.top(),
                                        tbh.width(), rect.height());
                    break;
                case QTabBar::RoundedWest:
                    tab_rect = QRect(rect.left(), rect.top(),
                                        tbh.width(), rect.height());
                    break;
                default:
                    break;
            }
        }

        widgetAnimator->animate(tabBar, tab_rect, animate);
    }
#endif // QT_NO_TABBAR

    for (int i = 0; i < item_list.size(); ++i) {
        QDockAreaLayoutItem &item = item_list[i];

        if (item.gap)
            continue;

        if (item.subinfo != 0) {
            item.subinfo->apply(animate);
            continue;
        }

        if (item.skip())
            continue;

        Q_ASSERT(item.widgetItem);
        QRect r = itemRect(i);
        QWidget *w = item.widgetItem->widget();

        QRect geo = w->geometry();
        widgetAnimator->animate(w, r, animate);
        if (w->isVisible()) {
            QDockWidget *dw = qobject_cast<QDockWidget*>(w);
            if (!r.isValid() && geo.right() >= 0 && geo.bottom() >= 0) {
                emit dw->visibilityChanged(false);
            } else if (r.isValid()
                        && (geo.right() < 0 || geo.bottom() < 0)) {
                emit dw->visibilityChanged(true);
            }
        }
    }
}

static void paintSep(QPainter *p, QWidget *w, const QRect &r, Qt::Orientation o, bool mouse_over)
{
    QStyleOption opt(0);
    opt.state = QStyle::State_None;
    if (w->isEnabled())
        opt.state |= QStyle::State_Enabled;
    if (o != Qt::Horizontal)
        opt.state |= QStyle::State_Horizontal;
    if (mouse_over)
        opt.state |= QStyle::State_MouseOver;
    opt.rect = r;
    opt.palette = w->palette();

    w->style()->drawPrimitive(QStyle::PE_IndicatorDockWidgetResizeHandle, &opt, p, w);
}

QRegion QDockAreaLayoutInfo::separatorRegion() const
{
    QRegion result;

    if (isEmpty())
        return result;
#ifndef QT_NO_TABBAR
    if (tabbed)
        return result;
#endif

    for (int i = 0; i < item_list.count(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);

        if (item.skip())
            continue;

        int next = this->next(i);

        if (item.subinfo)
            result |= item.subinfo->separatorRegion();

        if (next == -1)
            break;
        result |= separatorRect(i);
    }

    return result;
}

void QDockAreaLayoutInfo::paintSeparators(QPainter *p, QWidget *widget,
                                                    const QRegion &clip,
                                                    const QPoint &mouse) const
{
    if (isEmpty())
        return;
#ifndef QT_NO_TABBAR
    if (tabbed)
        return;
#endif

    for (int i = 0; i < item_list.count(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);

        if (item.skip())
            continue;

        int next = this->next(i);
        if (item.gap || next != -1 && item_list.at(next).gap)
            continue;

        if (item.subinfo) {
            if (clip.contains(item.subinfo->rect))
                item.subinfo->paintSeparators(p, widget, clip, mouse);
        }

        if (next == -1)
            break;
        QRect r = separatorRect(i);
        if (clip.contains(r))
            paintSep(p, widget, r, o, r.contains(mouse));
    }
}

int QDockAreaLayoutInfo::next(int index) const
{
    for (int i = index + 1; i < item_list.size(); ++i) {
        if (!item_list.at(i).skip())
            return i;
    }
    return -1;
}

int QDockAreaLayoutInfo::prev(int index) const
{
    for (int i = index - 1; i >= 0; --i) {
        if (!item_list.at(i).skip())
            return i;
    }
    return -1;
}

void QDockAreaLayoutInfo::tab(int index, QLayoutItem *dockWidgetItem)
{
#ifdef QT_NO_TABBAR
    Q_UNUSED(index);
    Q_UNUSED(dockWidgetItem);
#else
    if (tabbed) {
        item_list.append(QDockAreaLayoutItem(dockWidgetItem));
        updateTabBar();
        setCurrentTab(dockWidgetItem->widget());
    } else {
        QDockAreaLayoutInfo *new_info
            = new QDockAreaLayoutInfo(sep, dockPos, o, tabBarShape, mainWindow);
        item_list[index].subinfo = new_info;
        new_info->item_list.append(item_list.at(index).widgetItem);
        item_list[index].widgetItem = 0;
        new_info->item_list.append(dockWidgetItem);
        new_info->tabbed = true;
        new_info->updateTabBar();
        new_info->setCurrentTab(dockWidgetItem->widget());
    }
#endif // QT_NO_TABBAR
}

void QDockAreaLayoutInfo::split(int index, Qt::Orientation orientation,
                                       QLayoutItem *dockWidgetItem)
{
    if (orientation == o) {
        item_list.insert(index + 1, QDockAreaLayoutItem(dockWidgetItem));
    } else {
#ifdef QT_NO_TABBAR
        const int tabBarShape = 0;
#endif
        QDockAreaLayoutInfo *new_info
            = new QDockAreaLayoutInfo(sep, dockPos, orientation, tabBarShape, mainWindow);
        item_list[index].subinfo = new_info;
        new_info->item_list.append(item_list.at(index).widgetItem);
        item_list[index].widgetItem = 0;
        new_info->item_list.append(dockWidgetItem);
    }
}

QDockAreaLayoutItem &QDockAreaLayoutInfo::item(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());
    if (path.count() > 1) {
        QDockAreaLayoutItem &item = item_list[path.takeFirst()];
        Q_ASSERT(item.subinfo != 0);
        return item.subinfo->item(path);
    }
    return item_list[path.first()];
}

QLayoutItem *QDockAreaLayoutInfo::itemAt(int *x, int index) const
{
    for (int i = 0; i < item_list.count(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.subinfo) {
            if (QLayoutItem *ret = item.subinfo->itemAt(x, index))
                return ret;
        } else if (item.widgetItem) {
            if ((*x)++ == index)
                return item.widgetItem;
        }
    }
    return 0;
}

QLayoutItem *QDockAreaLayoutInfo::takeAt(int *x, int index)
{
    for (int i = 0; i < item_list.count(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.subinfo) {
            if (QLayoutItem *ret = item.subinfo->takeAt(x, index)) {
                unnest(i);
                return ret;
            }
        } else if (item.widgetItem) {
            if ((*x)++ == index) {
                QLayoutItem *ret = item.widgetItem;
                item_list.removeAt(i);
                return ret;
            }
        }
    }
    return 0;
}

void QDockAreaLayoutInfo::deleteAllLayoutItems()
{
    for (int i = 0; i < item_list.count(); ++i) {
        QDockAreaLayoutItem &item= item_list[i];
        if (item.subinfo) {
            item.subinfo->deleteAllLayoutItems();
        } else {
            delete item.widgetItem;
            item.widgetItem = 0;
        }
    }
}

void QDockAreaLayoutInfo::saveState(QDataStream &stream) const
{
#ifndef QT_NO_TABBAR
    if (tabbed) {
        stream << (uchar) TabMarker;

        // write the index in item_list of the widget that's currently on top.
        quintptr id = currentTabId();
        int index = -1;
        for (int i = 0; i < item_list.count(); ++i) {
            if (tabId(item_list.at(i)) == id) {
                index = i;
                break;
            }
        }
        stream << index;
    } else
#endif // QT_NO_TABBAR
    {
        stream << (uchar) SequenceMarker;
    }

    stream << (uchar) o << item_list.count();

    for (int i = 0; i < item_list.count(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.widgetItem != 0) {
            stream << (uchar) WidgetMarker;
            QWidget *w = item.widgetItem->widget();
            QString name = w->objectName();
            if (name.isEmpty()) {
                qWarning("QMainWindow::saveState(): 'objectName' not set for QDockWidget %p '%s;",
                         w, qPrintable(w->windowTitle()));
            }
            stream << name;

            uchar flags = 0;
            if (!w->isHidden())
                flags |= StateFlagVisible;
            if (w->isWindow())
                flags |= StateFlagFloating;
            stream << flags;

            if (w->isWindow()) {
                stream << w->x() << w->y() << w->width() << w->height();
            } else {
                stream << item.pos << item.size << pick(o, item.minimumSize())
                        << pick(o, item.maximumSize());
            }
        } else if (item.subinfo != 0) {
            stream << (uchar) SequenceMarker << item.pos << item.size << pick(o, item.minimumSize()) << pick(o, item.maximumSize());
            item.subinfo->saveState(stream);
        }
    }
}

bool QDockAreaLayoutInfo::restoreState(QDataStream &stream, QList<QDockWidget*> &widgets)
{
    uchar marker;
    stream >> marker;
    if (marker != TabMarker && marker != SequenceMarker)
        return false;

#ifndef QT_NO_TABBAR
    tabbed = marker == TabMarker;

    int index = -1;
    if (tabbed)
        stream >> index;
#endif

    uchar orientation;
    stream >> orientation;
    o = static_cast<Qt::Orientation>(orientation);

    int cnt;
    stream >> cnt;

    for (int i = 0; i < cnt; ++i) {
        uchar nextMarker;
        stream >> nextMarker;
        if (nextMarker == WidgetMarker) {
            QString name;
            uchar flags;
            stream >> name >> flags;
            if (name.isEmpty()) {
                int dummy;
                stream >> dummy >> dummy >> dummy >> dummy;
                continue;
            }

            QDockWidget *widget = 0;
            for (int j = 0; j < widgets.count(); ++j) {
                if (widgets.at(j)->objectName() == name) {
                    widget = widgets.takeAt(j);
                    break;
                }
            }
            if (widget == 0) {
                int dummy;
                stream >> dummy >> dummy >> dummy >> dummy;
                continue;
            }

            QDockAreaLayoutItem item(new QDockWidgetItem(widget));
            if (flags & StateFlagFloating) {
                widget->hide();
                widget->setFloating(true);
                int x, y, w, h;
                stream >> x >> y >> w >> h;
                widget->move(x, y);
                widget->resize(w, h);
                widget->setVisible(flags & StateFlagVisible);
            } else {
                int dummy;
                stream >> item.pos >> item.size >> dummy >> dummy;
//                qDebug() << widget << item.pos << item.size;
                widget->setFloating(false);
                widget->setVisible(flags & StateFlagVisible);
            }

            item_list.append(item);
        } else if (nextMarker == SequenceMarker) {
            int dummy;
#ifdef QT_NO_TABBAR
            const int tabBarShape = 0;
#endif
            QDockAreaLayoutInfo *info = new QDockAreaLayoutInfo(sep, dockPos, o,
                                                                tabBarShape, mainWindow);
            QDockAreaLayoutItem item(info);
            stream >> item.pos >> item.size >> dummy >> dummy;
            if (!info->restoreState(stream, widgets))
                return false;

            item_list.append(item);
        } else {
            return false;
        }
    }

#ifndef QT_NO_TABBAR
    if (tabbed && index >= 0 && index < item_list.count()) {
        updateTabBar();
        setCurrentTabId(tabId(item_list.at(index)));
    }
#endif

    return true;
}

#ifndef QT_NO_TABBAR
void QDockAreaLayoutInfo::updateTabBar() const
{
    if (!tabbed)
        return;

    QDockAreaLayoutInfo *that = const_cast<QDockAreaLayoutInfo*>(this);

    if (tabBar == 0) {
        that->tabBar = mainWindowLayout()->getTabBar();
        that->tabBar->setShape(static_cast<QTabBar::Shape>(tabBarShape));
        that->tabBar->setDrawBase(true);
    }

    bool blocked = tabBar->blockSignals(true);
    bool gap = false;

    int tab_idx = 0;
    bool changed = false;
    for (int i = 0; i < item_list.count(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.skip())
            continue;
        if (item.gap) {
            gap = true;
            continue;
        }
        if (item.widgetItem == 0)
            continue;

        QString title = item.widgetItem->widget()->windowTitle();
        quintptr id = tabId(item);
        if (tab_idx == tabBar->count()) {
            tabBar->insertTab(tab_idx, title);
#ifndef QT_NO_TOOLTIP
            tabBar->setTabToolTip(tab_idx, title);
#endif
            tabBar->setTabData(tab_idx, id);
            changed = true;
        } else if (qvariant_cast<quintptr>(tabBar->tabData(tab_idx)) != id) {
            if (tab_idx + 1 < tabBar->count()
                    && qvariant_cast<quintptr>(tabBar->tabText(tab_idx + 1)) == id)
                tabBar->removeTab(tab_idx);
            else {
                tabBar->insertTab(tab_idx, title);
#ifndef QT_NO_TOOLTIP
                tabBar->setTabToolTip(tab_idx, title);
#endif
                tabBar->setTabData(tab_idx, id);
            }
            changed = true;
        }

        if (title != tabBar->tabText(tab_idx)) {
            tabBar->setTabText(tab_idx, title);
#ifndef QT_NO_TOOLTIP
            tabBar->setTabToolTip(tab_idx, title);
#endif
            changed = true;
        }

        ++tab_idx;
    }

    while (tab_idx < tabBar->count()) {
        tabBar->removeTab(tab_idx);
        changed = true;
    }

    tabBar->blockSignals(blocked);

    that->tabBarVisible = gap || tabBar->count() > 1;

    if (changed || !tabBarMin.isValid() | !tabBarHint.isValid()) {
        that->tabBarMin = tabBar->minimumSizeHint();
        that->tabBarHint = tabBar->sizeHint();
    }
}

void QDockAreaLayoutInfo::setTabBarShape(int shape)
{
    if (shape == tabBarShape)
        return;
    tabBarShape = shape;
    if (tabBar != 0) {
        tabBar->setShape(static_cast<QTabBar::Shape>(shape));
        tabBarMin = QSize();
        tabBarHint = QSize();
    }

    for (int i = 0; i < item_list.count(); ++i) {
        QDockAreaLayoutItem &item = item_list[i];
        if (item.subinfo != 0)
            item.subinfo->setTabBarShape(shape);
    }
}

QSize QDockAreaLayoutInfo::tabBarMinimumSize() const
{
    if (!tabbed)
        return QSize(0, 0);

    updateTabBar();

    return tabBarMin;
}

QSize QDockAreaLayoutInfo::tabBarSizeHint() const
{
    if (!tabbed)
        return QSize(0, 0);

    updateTabBar();

    return tabBarHint;
}

QSet<QTabBar*> QDockAreaLayoutInfo::usedTabBars() const
{
    QSet<QTabBar*> result;

    if (tabbed) {
        updateTabBar();
        result.insert(tabBar);
    }

    for (int i = 0; i < item_list.count(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.subinfo != 0)
            result += item.subinfo->usedTabBars();
    }

    return result;
}

QRect QDockAreaLayoutInfo::tabContentRect() const
{
    if (!tabbed)
        return QRect();

    QRect result = rect;
    QSize tbh = tabBarSizeHint();

    if (tabBarVisible) {
        switch (tabBarShape) {
            case QTabBar::RoundedNorth:
                result.adjust(0, tbh.height(), 0, 0);
                break;
            case QTabBar::RoundedSouth:
                result.adjust(0, 0, 0, -tbh.height());
                break;
            case QTabBar::RoundedEast:
                result.adjust(0, 0, -tbh.width(), 0);
                break;
            case QTabBar::RoundedWest:
                result.adjust(tbh.width(), 0, 0, 0);
                break;
            default:
                break;
        }
    }

    return result;
}
#endif // QT_NO_TABBAR

/******************************************************************************
** QDockAreaLayout
*/

QDockAreaLayout::QDockAreaLayout(QMainWindow *win)
{
    mainWindow = win;
    sep = win->style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);
#ifndef QT_NO_TABBAR
    const int tabShape = QTabBar::RoundedSouth;
#else
    const int tabShape = 0;
#endif
    docks[QInternal::LeftDock]
        = QDockAreaLayoutInfo(sep, QInternal::LeftDock, Qt::Vertical, tabShape, win);
    docks[QInternal::RightDock]
        = QDockAreaLayoutInfo(sep, QInternal::RightDock, Qt::Vertical, tabShape, win);
    docks[QInternal::TopDock]
        = QDockAreaLayoutInfo(sep, QInternal::TopDock, Qt::Horizontal, tabShape, win);
    docks[QInternal::BottomDock]
        = QDockAreaLayoutInfo(sep, QInternal::BottomDock, Qt::Horizontal, tabShape, win);
    centralWidgetItem = 0;
    centralWidgetRect = QRect(0, 0, -1, -1);

    corners[Qt::TopLeftCorner] = Qt::TopDockWidgetArea;
    corners[Qt::TopRightCorner] = Qt::TopDockWidgetArea;
    corners[Qt::BottomLeftCorner] = Qt::BottomDockWidgetArea;
    corners[Qt::BottomRightCorner] = Qt::BottomDockWidgetArea;
}

bool QDockAreaLayout::isValid() const
{
    return rect.isValid();
}

void QDockAreaLayout::saveState(QDataStream &stream) const
{
    stream << (uchar) DockWidgetStateMarker;
    int cnt = 0;
    for (int i = 0; i < QInternal::DockCount; ++i) {
        if (!docks[i].item_list.isEmpty())
            ++cnt;
    }
    stream << cnt;
    for (int i = 0; i < QInternal::DockCount; ++i) {
        if (docks[i].item_list.isEmpty())
            continue;
        stream << i << docks[i].rect.size();
        docks[i].saveState(stream);
    }

    stream << centralWidgetRect.size();

    for (int i = 0; i < 4; ++i)
        stream << static_cast<int>(corners[i]);
}

bool QDockAreaLayout::restoreState(QDataStream &stream, const QList<QDockWidget*> &_dockwidgets)
{
    QList<QDockWidget*> dockwidgets = _dockwidgets;

    uchar dmarker;
    stream >> dmarker;
    if (dmarker != DockWidgetStateMarker)
        return false;
    int cnt;
    stream >> cnt;
    for (int i = 0; i < cnt; ++i) {
        int pos;
        stream >> pos;
        QSize size;
        stream >> size;
        docks[pos].rect = QRect(QPoint(0, 0), size);
        if (!docks[pos].restoreState(stream, dockwidgets)) {
            stream.setStatus(QDataStream::ReadCorruptData);
            return false;
        }
    }

    QSize size;
    stream >> size;
    centralWidgetRect = QRect(QPoint(0, 0), size);

    bool ok = stream.status() == QDataStream::Ok;

    if (ok) {
        int cornerData[4];
        for (int i = 0; i < 4; ++i)
            stream >> cornerData[i];
        if (stream.status() == QDataStream::Ok) {
            for (int i = 0; i < 4; ++i)
                corners[i] = static_cast<Qt::DockWidgetArea>(cornerData[i]);
        }
    }

    return ok;
}

QList<int> QDockAreaLayout::indexOf(QWidget *dockWidget) const
{
    for (int i = 0; i < QInternal::DockCount; ++i) {
        QList<int> result = docks[i].indexOf(dockWidget);
        if (!result.isEmpty()) {
            result.prepend(i);
            return result;
        }
    }
    return QList<int>();
}

QList<int> QDockAreaLayout::gapIndex(const QPoint &pos) const
{
    QMainWindow::DockOptions opts = mainWindow->dockOptions();
    bool nestingEnabled = opts & QMainWindow::AllowNestedDocks;
    QDockAreaLayoutInfo::TabMode tabMode = QDockAreaLayoutInfo::NoTabs;
#ifndef QT_NO_TABBAR
    if (opts & QMainWindow::AllowTabbedDocks
        || opts & QMainWindow::VerticalTabs)
        tabMode = QDockAreaLayoutInfo::AllowTabs;
    if (opts & QMainWindow::ForceTabbedDocks)
        tabMode = QDockAreaLayoutInfo::ForceTabs;

    if (tabMode == QDockAreaLayoutInfo::ForceTabs)
        nestingEnabled = false;
#endif


    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QDockAreaLayoutInfo &info = docks[i];

        if (!info.isEmpty() && info.rect.contains(pos)) {
            QList<int> result
                = docks[i].gapIndex(pos, nestingEnabled, tabMode);
            if (!result.isEmpty())
                result.prepend(i);
            return result;
        }
    }

    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QDockAreaLayoutInfo &info = docks[i];

        if (info.isEmpty()) {
            QRect r;
            switch (i) {
                case QInternal::LeftDock:
                    r = QRect(rect.left(), rect.top(), EmptyDropAreaSize, rect.height());
                    break;
                case QInternal::RightDock:
                    r = QRect(rect.right() - EmptyDropAreaSize, rect.top(),
                                EmptyDropAreaSize, rect.height());
                    break;
                case QInternal::TopDock:
                    r = QRect(rect.left(), rect.top(), rect.width(), EmptyDropAreaSize);
                    break;
                case QInternal::BottomDock:
                    r = QRect(rect.left(), rect.bottom() - EmptyDropAreaSize,
                                rect.width(), EmptyDropAreaSize);
                    break;
            }
            if (r.contains(pos))
                return QList<int>() << i << 0;
        }
    }

    return QList<int>();
}

QList<int> QDockAreaLayout::findSeparator(const QPoint &pos) const
{
    QList<int> result;
    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QDockAreaLayoutInfo &info = docks[i];
        if (info.isEmpty())
            continue;
        if (separatorRect(i).contains(pos)) {
            result << i;
            break;
        } else if (info.rect.contains(pos)) {
            result = docks[i].findSeparator(pos);
            if (!result.isEmpty()) {
                result.prepend(i);
                break;
            }
        }
    }

    return result;
}

QDockAreaLayoutInfo *QDockAreaLayout::info(QWidget *widget)
{
    for (int i = 0; i < QInternal::DockCount; ++i) {
        if (QDockAreaLayoutInfo *result = docks[i].info(widget))
            return result;
    }

    return 0;
}

QDockAreaLayoutInfo *QDockAreaLayout::info(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < QInternal::DockCount);

    if (path.isEmpty())
        return &docks[index];

    return docks[index].info(path);
}

const QDockAreaLayoutInfo *QDockAreaLayout::info(QList<int> path) const
{
    return const_cast<QDockAreaLayout*>(this)->info(path);
}

QDockAreaLayoutItem &QDockAreaLayout::item(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < QInternal::DockCount);
    return docks[index].item(path);
}

QRect QDockAreaLayout::itemRect(QList<int> path) const
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < QInternal::DockCount);
    return docks[index].itemRect(path);
}

QRect QDockAreaLayout::separatorRect(int index) const
{
    if (docks[index].isEmpty())
        return QRect();
    QRect r = docks[index].rect;
    switch (index) {
        case QInternal::LeftDock:
            return QRect(r.right() + 1, r.top(), sep, r.height());
        case QInternal::RightDock:
            return QRect(r.left() - sep, r.top(), sep, r.height());
        case QInternal::TopDock:
            return QRect(r.left(), r.bottom() + 1, r.width(), sep);
        case QInternal::BottomDock:
            return QRect(r.left(), r.top() - sep, r.width(), sep);
        default:
            break;
    }
    return QRect();
}

QRect QDockAreaLayout::separatorRect(QList<int> path) const
{
    Q_ASSERT(!path.isEmpty());

    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < QInternal::DockCount);

    if (path.isEmpty())
        return separatorRect(index);
    else
        return docks[index].separatorRect(path);
}

bool QDockAreaLayout::insertGap(QList<int> path, QLayoutItem *dockWidgetItem)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < QInternal::DockCount);
    return docks[index].insertGap(path, dockWidgetItem);
}

QLayoutItem *QDockAreaLayout::plug(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < QInternal::DockCount);
    return docks[index].plug(path);
}

QLayoutItem *QDockAreaLayout::unplug(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < QInternal::DockCount);
    return docks[index].unplug(path);
}

void QDockAreaLayout::remove(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < QInternal::DockCount);
    docks[index].remove(path);
}

static inline int qMin(int i1, int i2, int i3) { return qMin(i1, qMin(i2, i3)); }
static inline int qMax(int i1, int i2, int i3) { return qMax(i1, qMax(i2, i3)); }

void QDockAreaLayout::getGrid(QVector<QLayoutStruct> *_ver_struct_list,
                                QVector<QLayoutStruct> *_hor_struct_list)
{
    QSize center_hint(0, 0);
    QSize center_min(0, 0);
    bool have_central = centralWidgetItem != 0 && !centralWidgetItem->isEmpty();
    if (have_central) {
        center_hint = centralWidgetRect.size();
        if (!center_hint.isValid())
            center_hint = centralWidgetItem->sizeHint();
        center_min = centralWidgetItem->minimumSize();
    }

    QRect center_rect = rect;
    if (!docks[QInternal::LeftDock].isEmpty())
        center_rect.setLeft(rect.left() + docks[QInternal::LeftDock].rect.width() + sep);
    if (!docks[QInternal::TopDock].isEmpty())
        center_rect.setTop(rect.top() + docks[QInternal::TopDock].rect.height() + sep);
    if (!docks[QInternal::RightDock].isEmpty())
        center_rect.setRight(rect.right() - docks[QInternal::RightDock].rect.width() - sep);
    if (!docks[QInternal::BottomDock].isEmpty())
        center_rect.setBottom(rect.bottom() - docks[QInternal::BottomDock].rect.height() - sep);

    QSize left_hint = docks[QInternal::LeftDock].size();
    if (!left_hint.isValid())
        left_hint = docks[QInternal::LeftDock].sizeHint();
    QSize left_min = docks[QInternal::LeftDock].minimumSize();
    QSize left_max = docks[QInternal::LeftDock].maximumSize();
    left_hint = left_hint.boundedTo(left_max).expandedTo(left_min);

    QSize right_hint = docks[QInternal::RightDock].size();
    if (!right_hint.isValid())
        right_hint = docks[QInternal::RightDock].sizeHint();
    QSize right_min = docks[QInternal::RightDock].minimumSize();
    QSize right_max = docks[QInternal::RightDock].maximumSize();
    right_hint = right_hint.boundedTo(right_max).expandedTo(right_min);

    QSize top_hint = docks[QInternal::TopDock].size();
    if (!top_hint.isValid())
        top_hint = docks[QInternal::TopDock].sizeHint();
    QSize top_min = docks[QInternal::TopDock].minimumSize();
    QSize top_max = docks[QInternal::TopDock].maximumSize();
    top_hint = top_hint.boundedTo(top_max).expandedTo(top_min);

    QSize bottom_hint = docks[QInternal::BottomDock].size();
    if (!bottom_hint.isValid())
        bottom_hint = docks[QInternal::BottomDock].sizeHint();
    QSize bottom_min = docks[QInternal::BottomDock].minimumSize();
    QSize bottom_max = docks[QInternal::BottomDock].maximumSize();
    bottom_hint = bottom_hint.boundedTo(bottom_max).expandedTo(bottom_min);

    if (_ver_struct_list != 0) {
        QVector<QLayoutStruct> &ver_struct_list = *_ver_struct_list;
        ver_struct_list.resize(3);

        // top --------------------------------------------------

        ver_struct_list[0].stretch = 0;
        ver_struct_list[0].sizeHint = top_hint.height();
        ver_struct_list[0].minimumSize = top_min.height();
        ver_struct_list[0].maximumSize = top_max.height();
        ver_struct_list[0].expansive = false;
        ver_struct_list[0].empty = docks[QInternal::TopDock].isEmpty();
        ver_struct_list[0].pos = docks[QInternal::TopDock].rect.top();
        ver_struct_list[0].size = docks[QInternal::TopDock].rect.height();

        // center --------------------------------------------------

        ver_struct_list[1].stretch = center_hint.height();

        bool tl_significant = corners[Qt::TopLeftCorner] == Qt::TopDockWidgetArea
                                    || docks[QInternal::TopDock].isEmpty();
        bool bl_significant = corners[Qt::BottomLeftCorner] == Qt::BottomDockWidgetArea
                                    || docks[QInternal::BottomDock].isEmpty();
        bool tr_significant = corners[Qt::TopRightCorner] == Qt::TopDockWidgetArea
                                    || docks[QInternal::TopDock].isEmpty();
        bool br_significant = corners[Qt::BottomRightCorner] == Qt::BottomDockWidgetArea
                                    || docks[QInternal::BottomDock].isEmpty();

        int left = (tl_significant && bl_significant) ? left_hint.height() : 0;
        int right = (tr_significant && br_significant) ? right_hint.height() : 0;
        ver_struct_list[1].sizeHint = qMax(left, center_hint.height(), right);

        left = (tl_significant && bl_significant) ? left_min.height() : 0;
        right = (tr_significant && br_significant) ? right_min.height() : 0;
        ver_struct_list[1].minimumSize = qMax(left, center_min.height(), right);
        ver_struct_list[1].maximumSize = have_central ? QWIDGETSIZE_MAX : 0;
        ver_struct_list[1].expansive = have_central;
        ver_struct_list[1].empty = docks[QInternal::LeftDock].isEmpty()
                                        && !have_central
                                        && docks[QInternal::RightDock].isEmpty();
        ver_struct_list[1].pos = center_rect.top();
        ver_struct_list[1].size = center_rect.height();

        // bottom --------------------------------------------------

        ver_struct_list[2].stretch = 0;
        ver_struct_list[2].sizeHint = bottom_hint.height();
        ver_struct_list[2].minimumSize = bottom_min.height();
        ver_struct_list[2].maximumSize = bottom_max.height();
        ver_struct_list[2].expansive = false;
        ver_struct_list[2].empty = docks[QInternal::BottomDock].isEmpty();
        ver_struct_list[2].pos = docks[QInternal::BottomDock].rect.top();
        ver_struct_list[2].size = docks[QInternal::BottomDock].rect.height();

        for (int i = 0; i < 3; ++i) {
            ver_struct_list[i].sizeHint
                = qMax(ver_struct_list[i].sizeHint, ver_struct_list[i].minimumSize);
        }
    }

    if (_hor_struct_list != 0) {
        QVector<QLayoutStruct> &hor_struct_list = *_hor_struct_list;
        hor_struct_list.resize(3);

        // left --------------------------------------------------

        hor_struct_list[0].stretch = 0;
        hor_struct_list[0].sizeHint = left_hint.width();
        hor_struct_list[0].minimumSize = left_min.width();
        hor_struct_list[0].maximumSize = left_max.width();
        hor_struct_list[0].expansive = false;
        hor_struct_list[0].empty = docks[QInternal::LeftDock].isEmpty();
        hor_struct_list[0].pos = docks[QInternal::LeftDock].rect.left();
        hor_struct_list[0].size = docks[QInternal::LeftDock].rect.width();

        // center --------------------------------------------------

        hor_struct_list[1].stretch = center_hint.width();

        bool tl_significant = corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea
                                    || docks[QInternal::LeftDock].isEmpty();
        bool tr_significant = corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea
                                    || docks[QInternal::RightDock].isEmpty();
        bool bl_significant = corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea
                                    || docks[QInternal::LeftDock].isEmpty();
        bool br_significant = corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea
                                    || docks[QInternal::RightDock].isEmpty();

        int top = (tl_significant && tr_significant) ? top_hint.width() : 0;
        int bottom = (bl_significant && br_significant) ? bottom_hint.width() : 0;
        hor_struct_list[1].sizeHint = qMax(top, center_hint.width(), bottom);

        top = (tl_significant && tr_significant) ? top_min.width() : 0;
        bottom = (bl_significant && br_significant) ? bottom_min.width() : 0;
        hor_struct_list[1].minimumSize = qMax(top, center_min.width(), bottom);

        hor_struct_list[1].maximumSize = have_central ? QWIDGETSIZE_MAX : 0;
        hor_struct_list[1].expansive = have_central;
        hor_struct_list[1].empty = !have_central;
        hor_struct_list[1].pos = center_rect.left();
        hor_struct_list[1].size = center_rect.width();

        // right --------------------------------------------------

        hor_struct_list[2].stretch = 0;
        hor_struct_list[2].sizeHint = right_hint.width();
        hor_struct_list[2].minimumSize = right_min.width();
        hor_struct_list[2].maximumSize = right_max.width();
        hor_struct_list[2].expansive = false;
        hor_struct_list[2].empty = docks[QInternal::RightDock].isEmpty();
        hor_struct_list[2].pos = docks[QInternal::RightDock].rect.left();
        hor_struct_list[2].size = docks[QInternal::RightDock].rect.width();

        for (int i = 0; i < 3; ++i) {
            hor_struct_list[i].sizeHint
                = qMax(hor_struct_list[i].sizeHint, hor_struct_list[i].minimumSize);
        }
    }
}

void QDockAreaLayout::setGrid(QVector<QLayoutStruct> *ver_struct_list,
                                QVector<QLayoutStruct> *hor_struct_list)
{

    // top ---------------------------------------------------

    QRect r = docks[QInternal::TopDock].rect;
    if (hor_struct_list != 0) {
        r.setLeft(corners[Qt::TopLeftCorner] == Qt::TopDockWidgetArea
                    || docks[QInternal::LeftDock].isEmpty()
                        ? rect.left() : hor_struct_list->at(1).pos);
        r.setRight(corners[Qt::TopRightCorner] == Qt::TopDockWidgetArea
                    || docks[QInternal::RightDock].isEmpty()
                        ? rect.right() : hor_struct_list->at(2).pos - sep - 1);
    }
    if (ver_struct_list != 0) {
        r.setTop(rect.top());
        r.setBottom(ver_struct_list->at(1).pos - sep - 1);
    }
    docks[QInternal::TopDock].rect = r;
    docks[QInternal::TopDock].fitItems();

    // bottom ---------------------------------------------------

    r = docks[QInternal::BottomDock].rect;
    if (hor_struct_list != 0) {
        r.setLeft(corners[Qt::BottomLeftCorner] == Qt::BottomDockWidgetArea
                    || docks[QInternal::LeftDock].isEmpty()
                        ? rect.left() : hor_struct_list->at(1).pos);
        r.setRight(corners[Qt::BottomRightCorner] == Qt::BottomDockWidgetArea
                    || docks[QInternal::RightDock].isEmpty()
                        ? rect.right() : hor_struct_list->at(2).pos - sep - 1);
    }
    if (ver_struct_list != 0) {
        r.setTop(ver_struct_list->at(2).pos);
        r.setBottom(rect.bottom());
    }
    docks[QInternal::BottomDock].rect = r;
    docks[QInternal::BottomDock].fitItems();

    // left ---------------------------------------------------

    r = docks[QInternal::LeftDock].rect;
    if (hor_struct_list != 0) {
        r.setLeft(rect.left());
        r.setRight(hor_struct_list->at(1).pos - sep - 1);
    }
    if (ver_struct_list != 0) {
        r.setTop(corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea
                    || docks[QInternal::TopDock].isEmpty()
                        ? rect.top() : ver_struct_list->at(1).pos);
        r.setBottom(corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea
                    || docks[QInternal::BottomDock].isEmpty()
                        ? rect.bottom() : ver_struct_list->at(2).pos - sep - 1);
    }
    docks[QInternal::LeftDock].rect = r;
    docks[QInternal::LeftDock].fitItems();

    // right ---------------------------------------------------

    r = docks[QInternal::RightDock].rect;
    if (hor_struct_list != 0) {
        r.setLeft(hor_struct_list->at(2).pos);
        r.setRight(rect.right());
    }
    if (ver_struct_list != 0) {
        r.setTop(corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea
                    || docks[QInternal::TopDock].isEmpty()
                        ? rect.top() : ver_struct_list->at(1).pos);
        r.setBottom(corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea
                    || docks[QInternal::BottomDock].isEmpty()
                        ? rect.bottom() : ver_struct_list->at(2).pos - sep - 1);
    }
    docks[QInternal::RightDock].rect = r;
    docks[QInternal::RightDock].fitItems();

    // center ---------------------------------------------------

    if (hor_struct_list != 0) {
        centralWidgetRect.setLeft(hor_struct_list->at(1).pos);
        centralWidgetRect.setWidth(hor_struct_list->at(1).size);
    }
    if (ver_struct_list != 0) {
        centralWidgetRect.setTop(ver_struct_list->at(1).pos);
        centralWidgetRect.setHeight(ver_struct_list->at(1).size);
    }
}

void QDockAreaLayout::fitLayout()
{
    QVector<QLayoutStruct> ver_struct_list(3);
    QVector<QLayoutStruct> hor_struct_list(3);
    getGrid(&ver_struct_list, &hor_struct_list);

    qGeomCalc(ver_struct_list, 0, 3, rect.top(), rect.height(), sep);
    qGeomCalc(hor_struct_list, 0, 3, rect.left(), rect.width(), sep);

    setGrid(&ver_struct_list, &hor_struct_list);
}

void QDockAreaLayout::clear()
{
    for (int i = 0; i < QInternal::DockCount; ++i)
        docks[i].clear();

    rect = QRect(0, 0, -1, -1);
    centralWidgetRect = QRect(0, 0, -1, -1);
}

QSize QDockAreaLayout::sizeHint() const
{
    int left_sep = 0;
    int right_sep = 0;
    int top_sep = 0;
    int bottom_sep = 0;

    if (centralWidgetItem != 0) {
        left_sep = docks[QInternal::LeftDock].isEmpty() ? 0 : sep;
        right_sep = docks[QInternal::RightDock].isEmpty() ? 0 : sep;
        top_sep = docks[QInternal::TopDock].isEmpty() ? 0 : sep;
        bottom_sep = docks[QInternal::BottomDock].isEmpty() ? 0 : sep;
    }

    QSize left = docks[QInternal::LeftDock].sizeHint() + QSize(left_sep, 0);
    QSize right = docks[QInternal::RightDock].sizeHint() + QSize(right_sep, 0);
    QSize top = docks[QInternal::TopDock].sizeHint() + QSize(0, top_sep);
    QSize bottom = docks[QInternal::BottomDock].sizeHint() + QSize(0, bottom_sep);
    QSize center = centralWidgetItem == 0 ? QSize(0, 0) : centralWidgetItem->sizeHint();

    int row1 = top.width();
    int row2 = left.width() + center.width() + right.width();
    int row3 = bottom.width();
    int col1 = left.height();
    int col2 = top.height() + center.height() + bottom.height();
    int col3 = right.height();

    if (corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea)
        row1 += left.width();
    else
        col1 += top.height();

    if (corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea)
        row1 += right.width();
    else
        col3 += top.height();

    if (corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea)
        row3 += left.width();
    else
        col1 += bottom.height();

    if (corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea)
        row3 += right.width();
    else
        col3 += bottom.height();

    return QSize(qMax(row1, row2, row3), qMax(col1, col2, col3));
}

QSize QDockAreaLayout::minimumSize() const
{
    int left_sep = 0;
    int right_sep = 0;
    int top_sep = 0;
    int bottom_sep = 0;

    if (centralWidgetItem != 0) {
        left_sep = docks[QInternal::LeftDock].isEmpty() ? 0 : sep;
        right_sep = docks[QInternal::RightDock].isEmpty() ? 0 : sep;
        top_sep = docks[QInternal::TopDock].isEmpty() ? 0 : sep;
        bottom_sep = docks[QInternal::BottomDock].isEmpty() ? 0 : sep;
    }

    QSize left = docks[QInternal::LeftDock].minimumSize() + QSize(left_sep, 0);
    QSize right = docks[QInternal::RightDock].minimumSize() + QSize(right_sep, 0);
    QSize top = docks[QInternal::TopDock].minimumSize() + QSize(0, top_sep);
    QSize bottom = docks[QInternal::BottomDock].minimumSize() + QSize(0, bottom_sep);
    QSize center = centralWidgetItem == 0 ? QSize(0, 0) : centralWidgetItem->minimumSize();

    int row1 = top.width();
    int row2 = left.width() + center.width() + right.width();
    int row3 = bottom.width();
    int col1 = left.height();
    int col2 = top.height() + center.height() + bottom.height();
    int col3 = right.height();

    if (corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea)
        row1 += left.width();
    else
        col1 += top.height();

    if (corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea)
        row1 += right.width();
    else
        col3 += top.height();

    if (corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea)
        row3 += left.width();
    else
        col1 += bottom.height();

    if (corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea)
        row3 += right.width();
    else
        col3 += bottom.height();

    return QSize(qMax(row1, row2, row3), qMax(col1, col2, col3));
}

void QDockAreaLayout::addDockWidget(QInternal::DockPosition pos, QDockWidget *dockWidget,
                                             Qt::Orientation orientation)
{
    QLayoutItem *dockWidgetItem = new QDockWidgetItem(dockWidget);
    QDockAreaLayoutInfo &info = docks[pos];
    if (orientation == info.o || info.item_list.count() <= 1) {
        // empty dock areas, or dock areas containing exactly one widget can have their orientation
        // switched.
        info.o = orientation;

        QDockAreaLayoutItem new_item(dockWidgetItem);
        info.item_list.append(new_item);
#ifndef QT_NO_TABBAR
        if (info.tabbed && !new_item.skip()) {
            info.updateTabBar();
            info.setCurrentTabId(tabId(new_item));
        }
#endif
    } else {
#ifndef QT_NO_TABBAR
        int tbshape = QTabBar::RoundedSouth;
        switch (pos) {
            case QInternal::TopDock:
                tbshape = QTabBar::RoundedNorth;
                break;
            case QInternal::BottomDock:
                tbshape = QTabBar::RoundedSouth;
                break;
            case QInternal::RightDock:
                tbshape = QTabBar::RoundedEast;
                break;
            case QInternal::LeftDock:
                tbshape = QTabBar::RoundedWest;
                break;
            default:
                break;
        }
#else
        int tbshape = 0;
#endif
        QDockAreaLayoutInfo new_info(sep, pos, orientation, tbshape, mainWindow);
        new_info.item_list.append(new QDockAreaLayoutInfo(info));
        new_info.item_list.append(dockWidgetItem);
        info = new_info;
    }
}

void QDockAreaLayout::tabifyDockWidget(QDockWidget *first, QDockWidget *second)
{
    QList<int> path = indexOf(first);
    if (path.isEmpty())
        return;

    QDockAreaLayoutInfo *info = this->info(path);
    Q_ASSERT(info != 0);
    info->tab(path.last(), new QDockWidgetItem(second));
}

void QDockAreaLayout::splitDockWidget(QDockWidget *after,
                                               QDockWidget *dockWidget,
                                               Qt::Orientation orientation)
{
    QList<int> path = indexOf(after);
    if (path.isEmpty())
        return;

    QDockAreaLayoutInfo *info = this->info(path);
    Q_ASSERT(info != 0);
    info->split(path.last(), orientation, new QDockWidgetItem(dockWidget));
}

void QDockAreaLayout::apply(bool animate)
{
    QWidgetAnimator *widgetAnimator
        = qobject_cast<QMainWindowLayout*>(mainWindow->layout())->widgetAnimator;

    for (int i = 0; i < QInternal::DockCount; ++i)
        docks[i].apply(animate);
    if (centralWidgetItem != 0 && !centralWidgetItem->isEmpty()) {
        widgetAnimator->animate(centralWidgetItem->widget(), centralWidgetRect,
                                animate);
    }
}

void QDockAreaLayout::paintSeparators(QPainter *p, QWidget *widget,
                                                const QRegion &clip,
                                                const QPoint &mouse) const
{
    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QDockAreaLayoutInfo &dock = docks[i];
        if (dock.isEmpty())
            continue;
        QRect r = separatorRect(i);
        if (clip.contains(r)) {
            Qt::Orientation opposite = dock.o == Qt::Horizontal
                                        ? Qt::Vertical : Qt::Horizontal;
            paintSep(p, widget, r, opposite, r.contains(mouse));
        }
        if (clip.contains(dock.rect))
            dock.paintSeparators(p, widget, clip, mouse);
    }
}

QRegion QDockAreaLayout::separatorRegion() const
{
    QRegion result;

    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QDockAreaLayoutInfo &dock = docks[i];
        if (dock.isEmpty())
            continue;
        result |= separatorRect(i);
        result |= dock.separatorRegion();
    }

    return result;
}

int QDockAreaLayout::separatorMove(QList<int> separator, const QPoint &origin,
                                                const QPoint &dest,
                                                QVector<QLayoutStruct> *cache)
{
    int delta = 0;
    int index = separator.last();

    if (separator.count() > 1) {
        QDockAreaLayoutInfo *info = this->info(separator);
        delta = pick(info->o, dest - origin);
        if (delta != 0)
            delta = info->separatorMove(index, delta, cache);
        info->apply(false);
        return delta;
    }

    if (cache->isEmpty()) {
        QVector<QLayoutStruct> &list = *cache;

        if (index == QInternal::LeftDock || index == QInternal::RightDock)
            getGrid(0, &list);
        else
            getGrid(&list, 0);
    }

    QVector<QLayoutStruct> list = *cache;
    int sep_index = index == QInternal::LeftDock || index == QInternal::TopDock
                        ? 0 : 1;
    Qt::Orientation o = index == QInternal::LeftDock || index == QInternal::RightDock
                        ? Qt::Horizontal
                        : Qt::Vertical;

    delta = pick(o, dest - origin);
    delta = ::separatorMove(list, sep_index, delta, sep);

    if (index == QInternal::LeftDock || index == QInternal::RightDock)
        setGrid(0, &list);
    else
        setGrid(&list, 0);

    apply(false);

    return delta;
}

QLayoutItem *QDockAreaLayout::itemAt(int *x, int index) const
{
    Q_ASSERT(x != 0);

    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QDockAreaLayoutInfo &dock = docks[i];
        if (QLayoutItem *ret = dock.itemAt(x, index))
            return ret;
    }

    if (centralWidgetItem && (*x)++ == index)
        return centralWidgetItem;

    return 0;
}

QLayoutItem *QDockAreaLayout::takeAt(int *x, int index)
{
    Q_ASSERT(x != 0);

    for (int i = 0; i < QInternal::DockCount; ++i) {
        QDockAreaLayoutInfo &dock = docks[i];
        if (QLayoutItem *ret = dock.takeAt(x, index))
            return ret;
    }

    if (centralWidgetItem && (*x)++ == index) {
        QLayoutItem *ret = centralWidgetItem;
        centralWidgetItem = 0;
        return ret;
    }

    return 0;
}

void QDockAreaLayout::deleteAllLayoutItems()
{
    for (int i = 0; i < QInternal::DockCount; ++i)
        docks[i].deleteAllLayoutItems();
}

#ifndef QT_NO_TABBAR
QSet<QTabBar*> QDockAreaLayout::usedTabBars() const
{
    QSet<QTabBar*> result;
    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QDockAreaLayoutInfo &dock = docks[i];
        result += dock.usedTabBars();
    }
    return result;
}
#endif

QRect QDockAreaLayout::gapRect(QList<int> path) const
{
    const QDockAreaLayoutInfo *info = this->info(path);
    if (info == 0)
        return QRect();
    const QList<QDockAreaLayoutItem> &item_list = info->item_list;
    Qt::Orientation o = info->o;
    int index = path.last();
    if (index < 0 || index >= item_list.count())
        return QRect();
    const QDockAreaLayoutItem &item = item_list.at(index);
    if (!item.gap)
        return QRect();

    QRect result;

#ifndef QT_NO_TABBAR
    if (info->tabbed) {
        result = info->tabContentRect();
    } else
#endif
    {
        int pos = item.pos;
        int size = item.size;

        int prev = info->prev(index);
        int next = info->next(index);

        if (prev != -1 && !item_list.at(prev).gap) {
            pos += sep;
            size -= sep;
        }
        if (next != -1 && !item_list.at(next).gap)
            size -= sep;

        QPoint p;
        rpick(o, p) = pos;
        rperp(o, p) = perp(o, info->rect.topLeft());
        QSize s;
        rpick(o, s) = size;
        rperp(o, s) = perp(o, info->rect.size());

        result = QRect(p, s);
    }

    return result;
}

void QDockAreaLayout::keepSize(QDockWidget *w)
{
    QList<int> path = indexOf(w);
    if (path.isEmpty())
        return;
    QDockAreaLayoutItem &item = this->item(path);
    if (item.size != -1)
        item.keep_size = true;
}

#endif // QT_NO_DOCKWIDGET

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
#include "QtGui/qstyle.h"
#include "qdockwidgetlayout_p.h"
#include "qdockwidget.h"
#include "qmainwindow.h"
#include "qwidgetanimator_p.h"
#include "private/qlayoutengine_p.h"
#include <qdebug.h>

#include <qpainter.h>
#include <qstyleoption.h>

#ifndef QT_NO_DOCKWIDGET

enum { StateFlagVisible = 1, StateFlagFloating = 2 };

static void checkLayoutInfo(const QDockAreaLayoutInfo &info)
{
    return;
    int pos = pick(info.o, info.rect.topLeft());
    bool prev_gap = false;
    bool first = true;
    for (int i = 0; i < info.item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = info.item_list.at(i);
        if (item.skip())
            continue;

        bool gap = item.gap;
        if (!first && !gap && !prev_gap)
            pos += info.sep;

        if (item.pos != pos) {
            qDebug() << "##### checkLayoutInfo(): incorrect pos:"
                        << i << item.pos << pos;
        }

        pos += item.size;

        prev_gap = gap;
        first = false;
    }

    int bottom = pick(info.o, info.rect.bottomRight());
    --pos;
    if (pos != bottom) {
        qDebug() << "##### checkLayoutInfo(): incorrect bottom pos:"
                    << bottom << pos;
    }
}

#ifndef QT_NO_TEXTSTREAM

static void dump(QDebug debug, const QDockAreaLayoutInfo &info, QString indent);
void dump(QDebug debug, const QDockWidgetLayout &layout);

static void dump(QDebug debug, const QDockAreaLayoutItem &item, QString indent)
{
    debug << (const char*) indent.toLocal8Bit();
    if (item.skip())
        debug << "skip";
    if (item.gap)
        debug << "gap";
    debug << item.pos << item.size;
    if (item.widgetItem != 0)
        debug << item.widgetItem->widget();
    else if (item.subinfo != 0)
        dump(debug, *item.subinfo, indent);
    debug << '\n';
}

static void dump(QDebug debug, const QDockAreaLayoutInfo &info, QString indent)
{
    debug << "Info(\n";
    for (int i = 0; i < info.item_list.count(); ++i)
        dump(debug, info.item_list.at(i), indent + QLatin1String("  "));
    debug << (const char*) indent.toLocal8Bit() << ")\n";
}

void dump(QDebug debug, const QDockWidgetLayout &layout)
{
    debug << "Top " << layout.docks[QDockWidgetLayout::TopPos].rect << "\n";
    dump(debug, layout.docks[QDockWidgetLayout::TopPos], QString());
    debug << "Left " << layout.docks[QDockWidgetLayout::LeftPos].rect << "\n";
    dump(debug, layout.docks[QDockWidgetLayout::LeftPos], QString());
    debug << "Bottom " << layout.docks[QDockWidgetLayout::BottomPos].rect << "\n";
    dump(debug, layout.docks[QDockWidgetLayout::BottomPos], QString());
    debug << "Right " << layout.docks[QDockWidgetLayout::RightPos].rect << "\n";
    dump(debug, layout.docks[QDockWidgetLayout::RightPos], QString());
}
#endif // QT_NO_TEXTSTREAM

/******************************************************************************
** QDockAreaLayoutItem
*/

QDockAreaLayoutItem::QDockAreaLayoutItem(QWidgetItem *_widgetItem)
    : widgetItem(_widgetItem), subinfo(0), pos(0), size(-1), gap(false)
{
}

QDockAreaLayoutItem::QDockAreaLayoutItem(QDockAreaLayoutInfo *_subinfo)
    : widgetItem(0), subinfo(_subinfo), pos(0), size(-1), gap(false)
{
}

QDockAreaLayoutItem::QDockAreaLayoutItem(const QDockAreaLayoutItem &other)
    : widgetItem(other.widgetItem), subinfo(0), pos(other.pos), size(other.size), gap(other.gap)
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

    if (widgetItem != 0) {
        QWidget *widget = widgetItem->widget();
        return widget->isWindow() || widget->isHidden();
    }

    if (subinfo != 0) {
        for (int i = 0; i < subinfo->item_list.count(); ++i) {
            if (!subinfo->item_list.at(i).skip())
                return false;
        }
    }

    return true;
}

static QSize adjustForFrame(QWidget *widget)
{
    if (widget->isWindow())
        return QSize(0, 0);
    int fw = widget->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth);
    return QSize(2*fw, fw);
}

QSize QDockAreaLayoutItem::minimumSize() const
{
    if (widgetItem != 0)
        return qSmartMinSize(widgetItem) + adjustForFrame(widgetItem->widget());
    if (subinfo != 0)
        return subinfo->minimumSize();
    return QSize(0, 0);
}

QSize QDockAreaLayoutItem::maximumSize() const
{
    if (widgetItem != 0)
        return qSmartMaxSize(widgetItem);
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
    if (widgetItem != 0) {
        QSize result = gap
                ? widgetItem->widget()->sizeHint()
                : widgetItem->sizeHint();
        result += adjustForFrame(widgetItem->widget());
        return result;
    }
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

QDockAreaLayoutInfo::QDockAreaLayoutInfo()
    : sep(0), o(Qt::Horizontal), rect(0, 0, -1, -1), widgetAnimator(0)
{
}

QDockAreaLayoutInfo::QDockAreaLayoutInfo(int _sep, Qt::Orientation _o,
                                                        QWidgetAnimator *animator)
    : sep(_sep), o(_o), rect(0, 0, -1, -1), widgetAnimator(animator)
{
}

QSize QDockAreaLayoutInfo::size() const
{
    return isEmpty() ? QSize(0, 0) : rect.size();
}

void QDockAreaLayoutInfo::clear()
{
    item_list.clear();
    rect = QRect(0, 0, -1, -1);
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
    bool prev_gap = false;
    bool first = true;
    for (int i = 0; i < item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.skip())
            continue;

        bool gap = item.gap;
        if (!first)
            a += sep;

        QSize min_size = item.minimumSize();
//        a += gap ? item.size : pick(o, min_size);
        a += pick(o, min_size);
        b = qMax(b, perp(o, min_size));

        prev_gap = gap;
        first = false;
    }

    QSize result;
    rpick(o, result) = a;
    rperp(o, result) = b;
    return result;
}

QSize QDockAreaLayoutInfo::maximumSize() const
{
    if (isEmpty())
        return QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    int a = 0, b = 0;
    bool prev_gap = false;
    bool first = true;
    for (int i = 0; i < item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.skip())
            continue;

        bool gap = item.gap;
        if (!first)
            a += sep;

        QSize max_size = item.maximumSize();
        a += pick(o, max_size);
        a = qMin(a, int(QWIDGETSIZE_MAX));
        b = qMax(b, perp(o, max_size));

        prev_gap = gap;
        first = false;
    }

    QSize result;
    rpick(o, result) = a;
    rperp(o, result) = b;
    return result;
}

QSize QDockAreaLayoutInfo::sizeHint() const
{
    if (isEmpty())
        return QSize(0, 0);

    int a = 0, b = 0;
    bool prev_gap = false;
    bool first = true;
    for (int i = 0; i < item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.skip())
            continue;

        bool gap = item.gap;
        if (!first && !gap && !prev_gap)
            a += sep;

        QSize size_hint = item.sizeHint();
        a += gap ? item.size : pick(o, size_hint);
        b = qMax(b, perp(o, size_hint));

        prev_gap = gap;
        first = false;
    }

    QSize result;
    rpick(o, result) = a;
    rperp(o, result) = b;
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

void QDockAreaLayoutInfo::fitItems()
{
    QVector<QLayoutStruct> layout_struct_list(item_list.size()*2);
    int j = 0;

    bool prev_gap = false;
    bool first = true;
    for (int i = 0; i < item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.skip())
            continue;

        bool gap = item.gap;
        if (!first && !gap && !prev_gap) {
            QLayoutStruct &ls = layout_struct_list[j++];
            ls.empty = false;
            ls.init();
            ls.minimumSize = sep;
            ls.maximumSize = sep;
            ls.sizeHint = sep;
        }

        QLayoutStruct &ls = layout_struct_list[j++];
        ls.init();
        ls.empty = false;
        if (gap) {
            ls.minimumSize = ls.maximumSize = ls.sizeHint = item.size;
            ls.expansive = false;
            ls.stretch = 0;
        } else {
            ls.minimumSize = pick(o, item.minimumSize());
            ls.maximumSize = pick(o, item.maximumSize());
            ls.expansive = item.expansive(o);
            if (ls.expansive) {
                ls.sizeHint = ls.minimumSize;
                ls.stretch = item.size == -1 ? pick(o, item.sizeHint()) : item.size;
            } else {
                ls.sizeHint = item.size == -1 ? pick(o, item.sizeHint()) : item.size;
            }
        }

        prev_gap = gap;
        first = false;
    }
    layout_struct_list.resize(j);

    qGeomCalc(layout_struct_list, 0, j, pick(o, rect.topLeft()), pick(o, rect.size()), 0);

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

    checkLayoutInfo(*this);
}

static QDockWidgetLayout::DockPos dockPos(const QRect &rect, const QPoint &_pos,
                                                Qt::Orientation o, bool nestingEnabled)
{
    QPoint pos = _pos - rect.topLeft();

    int x = pos.x();
    int y = pos.y();
    int w = rect.width();
    int h = rect.height();

    if (nestingEnabled) {
        bool split1;
        bool split2;

        if (o == Qt::Horizontal) {
    /*       1/3   1/3  1/3
            +----+----+----+
            |     \  /     |
            |      \/      |
            |      /\      |
            |     /  \     |
            +----+----+----+    */
            split1 = y > (3*x - w)*h/w;
            split2 = y > -(3*x - 2*w)*h/w;
        } else {
    /*      +----+
        1/3 |    |
            +    +
            |\  /|
        1/3 | \/ |
            | /\ |
            |/  \|
            +    +
        1/3 |    |
            +----+    */
            split1 = y > h/3 + x*h/(3*w);
            split2 = y > 2*h/3 - x*h/(3*w);
        }

        if (split1 && split2)
            return QDockWidgetLayout::BottomPos;
        if (split1 && !split2)
            return QDockWidgetLayout::LeftPos;
        if (!split1 && split2)
            return QDockWidgetLayout::RightPos;
        // (!split1 && !split2)
        return QDockWidgetLayout::TopPos;
    } else {
        if (o == Qt::Horizontal) {
            return x < w/2
                    ? QDockWidgetLayout::LeftPos
                    : QDockWidgetLayout::RightPos;
        } else {
            return y < h/2
                    ? QDockWidgetLayout::TopPos
                    : QDockWidgetLayout::BottomPos;
        }
    }
}

QList<int> QDockAreaLayoutInfo::gapIndex(const QPoint& _pos, bool nestingEnabled) const
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

        if (item.subinfo != 0) {
            QList<int> result = item.subinfo->gapIndex(_pos, nestingEnabled);
            result.prepend(i);
            return result;
        }

        QList<int> result;
        QDockWidgetLayout::DockPos dock_pos
            = dockPos(itemRect(i), _pos, o, nestingEnabled);
        switch (dock_pos) {
            case QDockWidgetLayout::LeftPos:
                if (o == Qt::Horizontal)
                    result << i;
                else
                    result << i << 0;   // this subinfo doesn't exist yet, but insertGap()
                                        // handles this by inserting it
                break;
            case QDockWidgetLayout::RightPos:
                if (o == Qt::Horizontal)
                    result << i + 1;
                else
                    result << i << 1;
                break;
            case QDockWidgetLayout::TopPos:
                if (o == Qt::Horizontal)
                    result << i << 0;
                else
                    result << i;
                break;
            case QDockWidgetLayout::BottomPos:
                if (o == Qt::Horizontal)
                    result << i << 1;
                else
                    result << i + 1;
                break;
            default:
                break;
        }
        return result;
    }

    QList<int> result;
    result.append(last + 1);
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
        int d = 0;
        for (int i = index + 1; d < delta && i < list.count(); ++i)
            d += shrink(list[i], delta - d);
        delta = d;
        d = 0;
        for (int i = index; d < delta && i >= 0; --i)
            d += grow(list[i], delta - d);
        // ### handle case where maximum size hint is exceeded by spreading
        // remaining space equally among widgets
    } else if (delta < 0) {
        int d = 0;
        for (int i = index; d < -delta && i >= 0; --i)
            d += shrink(list[i], -delta - d);
        delta = -d;
        d = 0;
        for (int i = index + 1; d < -delta && i < list.count(); ++i)
            d += grow(list[i], -delta - d);
        // ### handle case where maximum size hint is exceeded by spreading
        // remaining space equally among widgets
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

int QDockAreaLayoutInfo::separatorMove(int index, int delta,
                                                QVector<QLayoutStruct> *cache)
{
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
        item_list.removeAt(path.first());
    }
}

QRect QDockAreaLayoutInfo::convertToWidget(QList<int> path, QWidgetItem *dockWidgetItem)
{
    Q_ASSERT(!path.isEmpty());

    if (path.count() > 1) {
        const QDockAreaLayoutItem &item = item_list.at(path.takeFirst());
        Q_ASSERT(item.subinfo != 0);
        return item.subinfo->convertToWidget(path, dockWidgetItem);
    }

    int index = path.first();
    QDockAreaLayoutItem &item = item_list[index];
    int prev = this->prev(index);
    int next = this->next(index);

    Q_ASSERT(item.gap);
    item.gap = false;
    Q_ASSERT(item.widgetItem == dockWidgetItem);

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
    QRect result(pos, s);

    return result;
}

QWidgetItem *QDockAreaLayoutInfo::convertToGap(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());

    if (path.count() > 1) {
        const QDockAreaLayoutItem &item = item_list.at(path.takeFirst());
        Q_ASSERT(item.subinfo != 0);
        return item.subinfo->convertToGap(path);
    }

    int index = path.first();
    QDockAreaLayoutItem &item = item_list[index];
    int prev = this->prev(index);
    int next = this->next(index);

    Q_ASSERT(!item.gap);
    item.gap = true;

    if (prev != -1 && !item_list.at(prev).gap) {
        item.pos -= sep;
        item.size += sep;
    }
    if (next != -1 && !item_list.at(next).gap)
        item.size += sep;

    return item.widgetItem;
}

bool QDockAreaLayoutInfo::insertGap(QList<int> path, QWidgetItem *dockWidgetItem)
{
    Q_ASSERT(!path.isEmpty());

    if (path.count() > 1) {
        QDockAreaLayoutItem &item = item_list[path.takeFirst()];
        if (item.subinfo == 0) {
            // this is not yet a nested layout - make it
            Qt::Orientation opposite = o == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal;
            QDockAreaLayoutInfo *new_info
                = new QDockAreaLayoutInfo(sep, opposite, widgetAnimator);
            item.subinfo = new_info;
            QDockAreaLayoutItem new_item(item.widgetItem);
            QRect r = item.widgetItem->geometry();
            new_item.size = pick(opposite, r.size());
            new_item.pos = pick(opposite, r.topLeft());
            new_info->item_list.append(new_item);
            item.widgetItem = 0;
        }

        bool result = item.subinfo->insertGap(path, dockWidgetItem);
        return result;
    }

    int index = path.first();
    int prev = this->prev(index);
    int next = this->next(index - 1);

    // find out how much space we have in the layout
    int space = 0;
    if (isEmpty()) {
        space = pick(o, rect.size());
    } else {
        for (int i = 0; i < item_list.count(); ++i) {
            const QDockAreaLayoutItem &item = item_list.at(i);
            if (item.skip())
                continue;
            Q_ASSERT(!item.gap);
            space += item.size - pick(o, item.minimumSize());
        }
    }

    // create the gap item
    QDockAreaLayoutItem gap_item;
    gap_item.gap = true;
    gap_item.widgetItem = dockWidgetItem; // so minimumSize(), maximumSize() and sizeHint() will work

    // find the actual size of the gap
    int gap_size = 0;
    int sep_size = 0;
    if (isEmpty()) {
        gap_size = space;
        sep_size = 0;
    } else {
        gap_size = pick(o, dockWidgetItem->geometry().size());
        if (prev != -1 && !item_list.at(prev).gap)
            sep_size += sep;
        if (next != -1 && !item_list.at(next).gap)
            sep_size += sep;
    }
    if (gap_size + sep_size > space)
        gap_size = pick(o, gap_item.minimumSize());
    gap_item.size = gap_size + sep_size;

    // finally, insert the gap
    item_list.insert(index, gap_item);

    return true;
}

QDockAreaLayoutInfo *QDockAreaLayoutInfo::info(QList<int> path)
{
    int index = path.takeFirst();
    if (index < 0 || index >= item_list.count())
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

    QPoint pos;
    rpick(o, pos) = item.pos;
    rperp(o, pos) = perp(o, rect.topLeft());
    QSize s;
    rpick(o, s) = item.size;
    rperp(o, s) = perp(o, rect.size());

    return QRect(pos, s);
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

QList<int> QDockAreaLayoutInfo::indexOf(QWidget *widget, IndexOfFlag flag) const
{
    for (int i = 0; i < item_list.size(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);

        if (item.subinfo != 0) {
            QList<int> result = item.subinfo->indexOf(widget, flag);
            if (!result.isEmpty()) {
                result.prepend(i);
                return result;
            }
            continue;
        }

        if (flag != IndexOfFindsAll) {
            if ((flag == IndexOfFindsVisible) == item.skip())
                continue;
        }

        if (item.widgetItem->widget() == widget) {
            QList<int> result;
            result << i;
            return result;
        }
    }

    return QList<int>();
}

void QDockAreaLayoutInfo::apply(bool animate)
{
    for (int i = 0; i < item_list.size(); ++i) {
        QDockAreaLayoutItem &item = item_list[i];
        if (item.gap || item.skip())
            continue;
        if (item.subinfo) {
            item.subinfo->apply(animate);
        } else {
            Q_ASSERT(item.widgetItem);
            QRect r = itemRect(i);
            widgetAnimator->animate(item.widgetItem->widget(), r, animate);
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

void QDockAreaLayoutInfo::split(int index, Qt::Orientation orientation,
                                       QWidgetItem *dockWidgetItem)
{
    if (orientation == o) {
        item_list.insert(index + 1, QDockAreaLayoutItem(dockWidgetItem));
    } else {
        QDockAreaLayoutInfo *new_info
            = new QDockAreaLayoutInfo(sep, orientation, widgetAnimator);
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
        }
        if ((*x)++ == index)
            return item.widgetItem;
    }
    return 0;
}

QLayoutItem *QDockAreaLayoutInfo::takeAt(int *x, int index)
{
    for (int i = 0; i < item_list.count(); ++i) {
        const QDockAreaLayoutItem &item = item_list.at(i);
        if (item.subinfo) {
            if (QLayoutItem *ret = item.subinfo->takeAt(x, index))
                return ret;
        }
        if ((*x)++ == index) {
            QLayoutItem *ret = item.widgetItem;
            item_list.removeAt(i);
            return ret;
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
    stream << (uchar) Marker << (uchar) o << item_list.count();
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
            stream << (uchar) Marker << item.pos << item.size << pick(o, item.minimumSize())
                        << pick(o, item.maximumSize());
            item.subinfo->saveState(stream);
        }
    }
}

bool QDockAreaLayoutInfo::restoreState(QDataStream &stream, const QList<QDockWidget*> &widgets)
{
    uchar marker;
    stream >> marker;
    if (marker != Marker)
        return false;

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
                qWarning("QMainWindow::restoreState: Cannot restore "
                         "a QDockWidget with an empty 'objectName'");
                int dummy;
                stream >> dummy >> dummy >> dummy >> dummy;
                continue;
            }

            QDockWidget *widget = 0;
            for (int j = 0; j < widgets.count(); ++j) {
                if (widgets.at(j)->objectName() == name) {
                    widget = widgets.at(j);
                    break;
                }
            }
            if (widget == 0) {
                qWarning("QMainWindow::restoreState(): cannot find a QDockWidget with "
                         "matching 'objectName' (looking for \"%s\").",
                         qPrintable(name));
                int dummy;
                stream >> dummy >> dummy >> dummy >> dummy;
                continue;
            }

            QDockAreaLayoutItem item(new QWidgetItem(widget));
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
                widget->setFloating(false);
                widget->setVisible(flags & StateFlagVisible);
            }

            item_list.append(item);
        } else if (nextMarker == Marker) {
            int dummy;
            QDockAreaLayoutInfo *info = new QDockAreaLayoutInfo(sep, o, widgetAnimator);
            QDockAreaLayoutItem item(info);
            stream >> item.pos >> item.size >> dummy >> dummy;
            if (!info->restoreState(stream, widgets))
                return false;
        } else {
            return false;
        }
    }
    return true;
}

/******************************************************************************
** QDockWidgetLayout
*/

QDockWidgetLayout::QDockWidgetLayout(QMainWindow *win,
                                        QWidgetAnimator *animator)
{
    sep = win->style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);
    widgetAnimator = animator;
    docks[LeftPos] = QDockAreaLayoutInfo(sep, Qt::Vertical, animator);
    docks[RightPos] = QDockAreaLayoutInfo(sep, Qt::Vertical, animator);
    docks[TopPos] = QDockAreaLayoutInfo(sep, Qt::Horizontal, animator);
    docks[BottomPos] = QDockAreaLayoutInfo(sep, Qt::Horizontal, animator);
    centralWidgetItem = 0;
    centralWidgetRect = QRect(0, 0, -1, -1);

    corners[Qt::TopLeftCorner] = Qt::TopDockWidgetArea;
    corners[Qt::TopRightCorner] = Qt::TopDockWidgetArea;
    corners[Qt::BottomLeftCorner] = Qt::BottomDockWidgetArea;
    corners[Qt::BottomRightCorner] = Qt::BottomDockWidgetArea;
}

bool QDockWidgetLayout::isValid() const
{
    return rect.isValid();
}

void QDockWidgetLayout::saveState(QDataStream &stream) const
{
    stream << (uchar) DockWidgetStateMarker;
    int cnt = 0;
    for (int i = 0; i < PosCount; ++i) {
        if (!docks[i].isEmpty())
            ++cnt;
    }
    stream << cnt;
    for (int i = 0; i < PosCount; ++i) {
        if (docks[i].isEmpty())
            continue;
        stream << i << docks[i].rect.size();
        docks[i].saveState(stream);
    }

    stream << centralWidgetRect.size();
}

bool QDockWidgetLayout::restoreState(QDataStream &stream, const QList<QDockWidget*> &dockwidgets)
{
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

    return stream.status() == QDataStream::Ok;
}

QList<int> QDockWidgetLayout::indexOf(QDockWidget *dockWidget, IndexOfFlag flag) const
{
    for (int i = 0; i < PosCount; ++i) {
        QList<int> result = docks[i].indexOf(dockWidget, flag);
        if (!result.isEmpty()) {
            result.prepend(i);
            return result;
        }
    }
    return QList<int>();
}

QList<int> QDockWidgetLayout::gapIndex(const QPoint &pos, bool nestingEnabled) const
{
    for (int i = 0; i < PosCount; ++i) {
        const QDockAreaLayoutInfo &info = docks[i];

        if (info.isEmpty()) {
            QRect r;
            switch (i) {
                case LeftPos:
                    r = QRect(rect.left(), rect.top(), EmptyDropAreaSize, rect.height());
                    break;
                case RightPos:
                    r = QRect(rect.right() - EmptyDropAreaSize, rect.top(),
                                EmptyDropAreaSize, rect.height());
                    break;
                case TopPos:
                    r = QRect(rect.left(), rect.top(), rect.width(), EmptyDropAreaSize);
                    break;
                case BottomPos:
                    r = QRect(rect.left(), rect.bottom() - EmptyDropAreaSize,
                                rect.width(), EmptyDropAreaSize);
                    break;
            }
            if (r.contains(pos))
                return QList<int>() << i << 0;
        } else if (info.rect.contains(pos)) {
            QList<int> result = docks[i].gapIndex(pos, nestingEnabled);
            if (!result.isEmpty()) {
                result.prepend(i);
                return result;
            }
        }
    }

    return QList<int>();
}

QList<int> QDockWidgetLayout::findSeparator(const QPoint &pos) const
{
    QList<int> result;
    for (int i = 0; i < PosCount; ++i) {
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

QDockAreaLayoutInfo *QDockWidgetLayout::info(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < PosCount);

    if (path.isEmpty())
        return &docks[index];

    return docks[index].info(path);
}

QDockAreaLayoutItem &QDockWidgetLayout::item(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < PosCount);
    return docks[index].item(path);
}

QRect QDockWidgetLayout::itemRect(QList<int> path) const
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < PosCount);
    return docks[index].itemRect(path);
}

QRect QDockWidgetLayout::separatorRect(int index) const
{
    if (docks[index].isEmpty())
        return QRect();
    QRect r = docks[index].rect;
    switch (index) {
        case LeftPos:
            return QRect(r.right() + 1, r.top(), sep, r.height());
        case RightPos:
            return QRect(r.left() - sep, r.top(), sep, r.height());
        case TopPos:
            return QRect(r.left(), r.bottom() + 1, r.width(), sep);
        case BottomPos:
            return QRect(r.left(), r.top() - sep, r.width(), sep);
        default:
            break;
    }
    return QRect();
}

QRect QDockWidgetLayout::separatorRect(QList<int> path) const
{
    Q_ASSERT(!path.isEmpty());

    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < PosCount);

    if (path.isEmpty())
        return separatorRect(index);
    else
        return docks[index].separatorRect(path);
}

bool QDockWidgetLayout::insertGap(QList<int> path, QWidgetItem *dockWidgetItem)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < PosCount);
    return docks[index].insertGap(path, dockWidgetItem);
}

QRect QDockWidgetLayout::convertToWidget(QList<int> path, QWidgetItem *dockWidgetItem)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < PosCount);
    return docks[index].convertToWidget(path, dockWidgetItem);
}

QWidgetItem *QDockWidgetLayout::convertToGap(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < PosCount);
    return docks[index].convertToGap(path);
}

void QDockWidgetLayout::remove(QList<int> path)
{
    Q_ASSERT(!path.isEmpty());
    int index = path.takeFirst();
    Q_ASSERT(index >= 0 && index < PosCount);
    docks[index].remove(path);
}

static inline int qMin(int i1, int i2, int i3) { return qMin(i1, qMin(i2, i3)); }
static inline int qMax(int i1, int i2, int i3) { return qMax(i1, qMax(i2, i3)); }

void QDockWidgetLayout::getGrid(QVector<QLayoutStruct> *_ver_struct_list,
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
    if (!docks[LeftPos].isEmpty())
        center_rect.setLeft(rect.left() + docks[LeftPos].rect.width() + sep);
    if (!docks[TopPos].isEmpty())
        center_rect.setTop(rect.top() + docks[TopPos].rect.height() + sep);
    if (!docks[RightPos].isEmpty())
        center_rect.setRight(rect.right() - docks[RightPos].rect.width() - sep - 1);
    if (!docks[BottomPos].isEmpty())
        center_rect.setBottom(rect.bottom() - docks[BottomPos].rect.height() - sep - 1);

    QSize left_hint = docks[LeftPos].size();
    if (!left_hint.isValid())
        left_hint = docks[LeftPos].sizeHint();
    QSize left_min = docks[LeftPos].minimumSize();
    QSize left_max = docks[LeftPos].maximumSize();
    int left_sep = docks[LeftPos].isEmpty() ? 0 : sep;

    QSize right_hint = docks[RightPos].size();
    if (!right_hint.isValid())
        right_hint = docks[RightPos].sizeHint();
    QSize right_min = docks[RightPos].minimumSize();
    QSize right_max = docks[RightPos].maximumSize();
    int right_sep = docks[RightPos].isEmpty() ? 0 : sep;

    QSize top_hint = docks[TopPos].size();
    if (!top_hint.isValid())
        top_hint = docks[TopPos].sizeHint();
    QSize top_min = docks[TopPos].minimumSize();
    QSize top_max = docks[TopPos].maximumSize();
    int top_sep = docks[TopPos].isEmpty() ? 0 : sep;

    QSize bottom_hint = docks[BottomPos].size();
    if (!bottom_hint.isValid())
        bottom_hint = docks[BottomPos].sizeHint();
    QSize bottom_min = docks[BottomPos].minimumSize();
    QSize bottom_max = docks[BottomPos].maximumSize();
    int bottom_sep = docks[BottomPos].isEmpty() ? 0 : sep;

    if (_ver_struct_list != 0) {
        QVector<QLayoutStruct> &ver_struct_list = *_ver_struct_list;
        ver_struct_list.resize(3);

        // top --------------------------------------------------

        ver_struct_list[0].stretch = 0;
        ver_struct_list[0].sizeHint = top_hint.height();
        ver_struct_list[0].minimumSize = top_min.height();
        ver_struct_list[0].maximumSize = top_max.height();
        ver_struct_list[0].expansive = false;
        ver_struct_list[0].empty = docks[TopPos].isEmpty();
        ver_struct_list[0].pos = docks[TopPos].rect.top();
        ver_struct_list[0].size = docks[TopPos].rect.height();

        // center --------------------------------------------------

        ver_struct_list[1].stretch = center_hint.height();
        int left = left_hint.height();
        if (corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea)
            left -= top_hint.height() + top_sep;
        if (corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea)
            left -= bottom_hint.height() + bottom_sep;
        int right = right_hint.height();
        if (corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea)
            right -= top_hint.height() + top_sep;
        if (corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea)
            right -= bottom_hint.height() + bottom_sep;
        ver_struct_list[1].sizeHint = qMax(left, center_hint.height(), right);

        left = left_min.height();
        if (corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea)
            left -= top_min.height() + top_sep;
        if (corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea)
            left -= bottom_min.height() + bottom_sep;
        right = right_min.height();
        if (corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea)
            right -= top_min.height() + top_sep;
        if (corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea)
            right -= bottom_min.height() + bottom_sep;
        ver_struct_list[1].minimumSize = qMax(left, center_min.height(), right);
        ver_struct_list[1].maximumSize = have_central ? QWIDGETSIZE_MAX : 0;
        ver_struct_list[1].expansive = have_central;
        ver_struct_list[1].empty = docks[LeftPos].isEmpty()
                                        && !have_central
                                        && docks[RightPos].isEmpty();
        ver_struct_list[1].pos = center_rect.top();
        ver_struct_list[1].size = center_rect.height();

        // bottom --------------------------------------------------

        ver_struct_list[2].stretch = 0;
        ver_struct_list[2].sizeHint = bottom_hint.height();
        ver_struct_list[2].minimumSize = bottom_min.height();
        ver_struct_list[2].maximumSize = bottom_max.height();
        ver_struct_list[2].expansive = false;
        ver_struct_list[2].empty = docks[BottomPos].isEmpty();
        ver_struct_list[2].pos = docks[BottomPos].rect.top();
        ver_struct_list[2].size = docks[BottomPos].rect.height();

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
        hor_struct_list[0].empty = docks[LeftPos].isEmpty();
        hor_struct_list[0].pos = docks[LeftPos].rect.left();
        hor_struct_list[0].size = docks[LeftPos].rect.width();

        // center --------------------------------------------------

        hor_struct_list[1].stretch = center_hint.width();
        int top = top_hint.width();
        if (corners[Qt::TopLeftCorner] == Qt::TopDockWidgetArea)
            top -= left_hint.width() + left_sep;
        if (corners[Qt::TopRightCorner] == Qt::TopDockWidgetArea)
            top -= right_hint.width() + right_sep;
        int bottom = bottom_hint.width();
        if (corners[Qt::BottomLeftCorner] == Qt::BottomDockWidgetArea)
            bottom -= left_hint.width() + left_sep;
        if (corners[Qt::BottomRightCorner] == Qt::BottomDockWidgetArea)
            bottom -= right_hint.width() + right_sep;
        hor_struct_list[1].sizeHint = qMax(top, center_hint.width(), bottom);

        top = top_min.width();
        if (corners[Qt::TopLeftCorner] == Qt::TopDockWidgetArea)
            top -= left_min.width() + left_sep;
        if (corners[Qt::TopRightCorner] == Qt::TopDockWidgetArea)
            top -= right_min.width() + right_sep;
        bottom = bottom_min.width();
        if (corners[Qt::BottomLeftCorner] == Qt::BottomDockWidgetArea)
            bottom -= left_min.width() + left_sep;
        if (corners[Qt::BottomRightCorner] == Qt::BottomDockWidgetArea)
            bottom -= right_min.width() + right_sep;
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
        hor_struct_list[2].empty = docks[RightPos].isEmpty();
        hor_struct_list[2].pos = docks[RightPos].rect.left();
        hor_struct_list[2].size = docks[RightPos].rect.width();

        for (int i = 0; i < 3; ++i) {
            hor_struct_list[i].sizeHint
                = qMax(hor_struct_list[i].sizeHint, hor_struct_list[i].minimumSize);
        }
    }
}

void QDockWidgetLayout::setGrid(QVector<QLayoutStruct> *ver_struct_list,
                                QVector<QLayoutStruct> *hor_struct_list)
{
    bool have_central = centralWidgetItem != 0 && !centralWidgetItem->isEmpty();

    // top ---------------------------------------------------

    QRect r = docks[TopPos].rect;
    if (hor_struct_list != 0) {
        r.setLeft(corners[Qt::TopLeftCorner] == Qt::TopDockWidgetArea
                    || docks[LeftPos].isEmpty()
                        ? rect.left() : hor_struct_list->at(1).pos);
        r.setRight(corners[Qt::TopRightCorner] == Qt::TopDockWidgetArea
                    || docks[RightPos].isEmpty()
                        ? rect.right() : hor_struct_list->at(2).pos - sep - 1);
    }
    if (ver_struct_list != 0) {
        r.setTop(rect.top());
        r.setBottom(ver_struct_list->at(1).pos - sep - 1);
    }
    docks[TopPos].rect = r;
    docks[TopPos].fitItems();

    // bottom ---------------------------------------------------

    r = docks[BottomPos].rect;
    if (hor_struct_list != 0) {
        r.setLeft(corners[Qt::BottomLeftCorner] == Qt::BottomDockWidgetArea
                    || docks[LeftPos].isEmpty()
                        ? rect.left() : hor_struct_list->at(1).pos);
        r.setRight(corners[Qt::BottomRightCorner] == Qt::BottomDockWidgetArea
                    || docks[RightPos].isEmpty()
                        ? rect.right() : hor_struct_list->at(2).pos - sep - 1);
    }
    if (ver_struct_list != 0) {
        r.setTop(ver_struct_list->at(2).pos);
        r.setBottom(rect.bottom());
    }
    docks[BottomPos].rect = r;
    docks[BottomPos].fitItems();

    // left ---------------------------------------------------

    r = docks[LeftPos].rect;
    if (hor_struct_list != 0) {
        r.setLeft(rect.left());
        r.setRight(hor_struct_list->at(1).pos - sep - 1);
    }
    if (ver_struct_list != 0) {
        r.setTop(corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea
                    || docks[TopPos].isEmpty()
                        ? rect.top() : ver_struct_list->at(1).pos);
        r.setBottom(corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea
                    || docks[BottomPos].isEmpty()
                        ? rect.bottom() : ver_struct_list->at(2).pos - sep - 1);
    }
    docks[LeftPos].rect = r;
    docks[LeftPos].fitItems();

    // right ---------------------------------------------------

    r = docks[RightPos].rect;
    if (hor_struct_list != 0) {
        r.setLeft(hor_struct_list->at(2).pos);
        r.setRight(rect.right());
    }
    if (ver_struct_list != 0) {
        r.setTop(corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea
                    || docks[TopPos].isEmpty()
                        ? rect.top() : ver_struct_list->at(1).pos);
        r.setBottom(corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea
                    || docks[BottomPos].isEmpty()
                        ? rect.bottom() : ver_struct_list->at(2).pos - sep - 1);
    }
    docks[RightPos].rect = r;
    docks[RightPos].fitItems();

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

void QDockWidgetLayout::fitLayout()
{
    QVector<QLayoutStruct> ver_struct_list(3);
    QVector<QLayoutStruct> hor_struct_list(3);
    getGrid(&ver_struct_list, &hor_struct_list);

    qGeomCalc(ver_struct_list, 0, 3, rect.top(), rect.height(), sep);
    qGeomCalc(hor_struct_list, 0, 3, rect.left(), rect.width(), sep);

    setGrid(&ver_struct_list, &hor_struct_list);
}

void QDockWidgetLayout::clear()
{
    for (int i = 0; i < PosCount; ++i)
        docks[i].clear();

    rect = QRect(0, 0, -1, -1);
    centralWidgetRect = QRect(0, 0, -1, -1);
}

QSize QDockWidgetLayout::sizeHint() const
{
    int left_sep = docks[LeftPos].isEmpty() ? 0 : sep;
    int right_sep = docks[RightPos].isEmpty() ? 0 : sep;
    int top_sep = docks[TopPos].isEmpty() ? 0 : sep;
    int bottom_sep = docks[BottomPos].isEmpty() ? 0 : sep;

    QSize left = docks[LeftPos].sizeHint() + QSize(left_sep, 0);
    QSize right = docks[RightPos].sizeHint() + QSize(right_sep, 0);
    QSize top = docks[TopPos].sizeHint() + QSize(0, top_sep);
    QSize bottom = docks[BottomPos].sizeHint() + QSize(0, bottom_sep);
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

QSize QDockWidgetLayout::minimumSize() const
{
    int left_sep = docks[LeftPos].isEmpty() ? 0 : sep;
    int right_sep = docks[RightPos].isEmpty() ? 0 : sep;
    int top_sep = docks[TopPos].isEmpty() ? 0 : sep;
    int bottom_sep = docks[BottomPos].isEmpty() ? 0 : sep;

    QSize left = docks[LeftPos].minimumSize() + QSize(left_sep, 0);
    QSize right = docks[RightPos].minimumSize() + QSize(right_sep, 0);
    QSize top = docks[TopPos].minimumSize() + QSize(0, top_sep);
    QSize bottom = docks[BottomPos].minimumSize() + QSize(0, bottom_sep);
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

void QDockWidgetLayout::addDockWidget(DockPos pos, QDockWidget *dockWidget,
                                             Qt::Orientation orientation)
{
    QWidgetItem *dockWidgetItem = new QWidgetItem(dockWidget);
    QDockAreaLayoutInfo &info = docks[pos];
    if (orientation == info.o || info.isEmpty()) {
        info.item_list.append(QDockAreaLayoutItem(dockWidgetItem));
    } else {
        QDockAreaLayoutInfo new_info(sep, orientation, widgetAnimator);
        new_info.item_list.append(new QDockAreaLayoutInfo(info));
        new_info.item_list.append(dockWidgetItem);
        info = new_info;
    }
}

void QDockWidgetLayout::splitDockWidget(QDockWidget *after,
                                               QDockWidget *dockWidget,
                                               Qt::Orientation orientation)
{
    QList<int> path = indexOf(after);
    if (path.isEmpty())
        return;

    QDockAreaLayoutInfo *info = this->info(path);
    Q_ASSERT(info != 0);
    info->split(path.last(), orientation, new QWidgetItem(dockWidget));
}

void QDockWidgetLayout::apply(bool animate)
{
    for (int i = 0; i < PosCount; ++i)
        docks[i].apply(animate);
    if (centralWidgetItem != 0 && !centralWidgetItem->isEmpty()) {
        widgetAnimator->animate(centralWidgetItem->widget(), centralWidgetRect,
                                animate);
    }
}

void QDockWidgetLayout::paintSeparators(QPainter *p, QWidget *widget,
                                                const QRegion &clip,
                                                const QPoint &mouse) const
{
    for (int i = 0; i < PosCount; ++i) {
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

QRegion QDockWidgetLayout::separatorRegion() const
{
    QRegion result;

    for (int i = 0; i < PosCount; ++i) {
        const QDockAreaLayoutInfo &dock = docks[i];
        if (dock.isEmpty())
            continue;
        result |= separatorRect(i);
        result |= dock.separatorRegion();
    }

    return result;
}

int QDockWidgetLayout::separatorMove(QList<int> separator, const QPoint &origin,
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

        if (index == LeftPos || index == RightPos)
            getGrid(0, &list);
        else
            getGrid(&list, 0);
    }

    QVector<QLayoutStruct> list = *cache;
    int sep_index = index == LeftPos || index == TopPos
                        ? 0 : 1;
    Qt::Orientation o = index == LeftPos || index == RightPos
                        ? Qt::Horizontal
                        : Qt::Vertical;

    delta = pick(o, dest - origin);
    delta = ::separatorMove(list, sep_index, delta, sep);

    if (index == LeftPos || index == RightPos)
        setGrid(0, &list);
    else
        setGrid(&list, 0);

    apply(false);

    return delta;
}

QLayoutItem *QDockWidgetLayout::itemAt(int *x, int index) const
{
    Q_ASSERT(x != 0);

    for (int i = 0; i < PosCount; ++i) {
        const QDockAreaLayoutInfo &dock = docks[i];
        if (dock.isEmpty())
            continue;
        if (QLayoutItem *ret = dock.itemAt(x, index))
            return ret;
    }

    if (centralWidgetItem && (*x)++ == index)
        return centralWidgetItem;

    return 0;
}

QLayoutItem *QDockWidgetLayout::takeAt(int *x, int index)
{
    Q_ASSERT(x != 0);

    for (int i = 0; i < PosCount; ++i) {
        QDockAreaLayoutInfo &dock = docks[i];
        if (dock.isEmpty())
            continue;
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

void QDockWidgetLayout::deleteAllLayoutItems()
{
    for (int i = 0; i < PosCount; ++i)
        docks[i].deleteAllLayoutItems();
}

#endif // QT_NO_DOCKWIDGET

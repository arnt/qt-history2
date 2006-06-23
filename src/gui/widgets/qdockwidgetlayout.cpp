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

static void dump(QDebug debug, const QDockAreaLayoutInfo &info, QString indent);
static void dump(QDebug debug, const QDockWidgetLayout &layout);

static void dump(QDebug debug, const QDockAreaLayoutItem &item, QString indent)
{
    debug << (const char*) indent.toLocal8Bit();
    if (item.skip())
        debug << "skip";
    if (item.gap)
        debug << "gap";
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

static void dump(QDebug debug, const QDockWidgetLayout &layout)
{
    debug << "Top\n";
    dump(debug, layout.docks[QDockWidgetLayout::TopPos], QString());
    debug << "Left\n";
    dump(debug, layout.docks[QDockWidgetLayout::LeftPos], QString());
    debug << "Bottom\n";
    dump(debug, layout.docks[QDockWidgetLayout::BottomPos], QString());
    debug << "Right\n";
    dump(debug, layout.docks[QDockWidgetLayout::RightPos], QString());
}

/******************************************************************************
** QDockAreaLayoutItem
*/

QDockAreaLayoutItem::QDockAreaLayoutItem(QWidgetItem *_widgetItem)
    : widgetItem(_widgetItem), subinfo(0), pos(0), size(0), gap(false)
{
}

QDockAreaLayoutItem::QDockAreaLayoutItem(QDockAreaLayoutInfo *_subinfo)
    : widgetItem(0), subinfo(_subinfo), pos(0), size(0), gap(false)
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
    if (widgetItem != 0)
        return widgetItem->sizeHint() + adjustForFrame(widgetItem->widget());
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
    : sep(0), o(Qt::Horizontal), widgetAnimator(0)
{
}

QDockAreaLayoutInfo::QDockAreaLayoutInfo(int _sep, Qt::Orientation _o,
                                                        QWidgetAnimator *animator)
    : sep(_sep), o(_o), widgetAnimator(animator)
{
}

void QDockAreaLayoutInfo::clear()
{
    item_list.clear();
    rect = QRect();
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
        return QSize(-1, -1);

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
                ls.stretch = item.size == 0 ? pick(o, item.sizeHint()) : item.size;
            } else {
                ls.sizeHint = item.size == 0 ? pick(o, item.sizeHint()) : item.size;
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
    return QRect(pos, s);
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
            if (QLayoutItem *ret = item.subinfo->itemAt(x, index))
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
                qWarning() << "QMainWindow::saveState(): 'objectName' not set for QDockWidget"
                            << w << w->windowTitle();
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
                qWarning() << "QMainWindow::restoreState: Cannot restore "
                                "a QDockWidget with an empty 'objectName'";
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
                qWarning() << "QMainWindow::restoreState(): cannot find a QDockWidget with "
                                "matching 'objectName' (looking for " << name << ").";
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

    return true;
}

QSize QDockWidgetLayout::calculateSize() const
{
    bool have_central = centralWidgetItem != 0 && !centralWidgetItem->isEmpty();

    int w = 0;
    int h = 0;

    if (have_central || !docks[LeftPos].isEmpty() || !docks[RightPos].isEmpty()) {
        if (!docks[LeftPos].isEmpty()) {
            w += docks[LeftPos].rect.width() + sep;
            h = qMax(h, docks[LeftPos].rect.height());
        }
        if (have_central) {
            w += centralWidgetRect.width();
            h = qMax(h, centralWidgetRect.height());
        }
        if (!docks[RightPos].isEmpty()) {
            w += docks[RightPos].rect.width() + sep;
            h = qMax(h, docks[RightPos].rect.height());
        }
    }

    if (!docks[LeftPos].isEmpty()) {
        w = qMax(w, docks[LeftPos].rect.width());
        h += docks[LeftPos].rect.height() + sep;
    }

    if (!docks[RightPos].isEmpty()) {
        w = qMax(w, docks[RightPos].rect.width());
        h += docks[RightPos].rect.height() + sep;
    }

    return QSize(w, h);
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

void QDockWidgetLayout::fitLayout()
{
    QSize central_hint(0, 0);
    QSize central_size(0, 0);
    QSize central_min(0, 0);
    bool have_central = centralWidgetItem != 0 && !centralWidgetItem->isEmpty();
    if (have_central) {
        central_size = centralWidgetItem->geometry().size();
        central_hint = centralWidgetItem->sizeHint();
        central_min = centralWidgetItem->minimumSize();
    }

    // vertical
    QVector<QLayoutStruct> ver_struct_list(3);

    ver_struct_list[0].stretch = 0;
    ver_struct_list[0].sizeHint
        = !docks[TopPos].isEmpty() && docks[TopPos].rect.isValid()
            ? docks[TopPos].rect.height()
            : docks[TopPos].sizeHint().height();
    ver_struct_list[0].minimumSize = docks[TopPos].minimumSize().height();
    ver_struct_list[0].maximumSize = docks[TopPos].maximumSize().height();
    ver_struct_list[0].expansive = false;
    ver_struct_list[0].empty = docks[TopPos].isEmpty();

    ver_struct_list[1].stretch = central_size.height();
    int h = central_size.height();
    if (h == 0 && !docks[LeftPos].isEmpty())
        h = docks[LeftPos].rect.height();
    if (h == 0 && !docks[RightPos].isEmpty())
        h = docks[RightPos].rect.height();
    ver_struct_list[1].sizeHint = h == 0
                                    ? qMax(docks[LeftPos].sizeHint().height(),
                                            central_hint.height(),
                                            docks[RightPos].sizeHint().height())
                                    : h;
    ver_struct_list[1].minimumSize = qMax(docks[LeftPos].minimumSize().height(),
                                            central_min.height(),
                                            docks[RightPos].minimumSize().height());
    ver_struct_list[1].maximumSize = have_central ? QWIDGETSIZE_MAX : 0;
    ver_struct_list[1].expansive = have_central;
    ver_struct_list[1].empty = docks[LeftPos].isEmpty()
                                    && !have_central
                                    && docks[RightPos].isEmpty();

    ver_struct_list[2].stretch = 0;
    ver_struct_list[2].sizeHint
        = !docks[BottomPos].isEmpty() && docks[BottomPos].rect.isValid()
            ? docks[BottomPos].rect.height()
            : docks[BottomPos].sizeHint().height();
    ver_struct_list[2].minimumSize = docks[BottomPos].minimumSize().height();
    ver_struct_list[2].maximumSize = docks[BottomPos].maximumSize().height();
    ver_struct_list[2].expansive = false;
    ver_struct_list[2].empty = docks[BottomPos].isEmpty();

    for (int i = 0; i < 3; ++i) {
        ver_struct_list[i].sizeHint
            = qMax(ver_struct_list[i].sizeHint, ver_struct_list[i].minimumSize);
    }

    qGeomCalc(ver_struct_list, 0, 3, rect.top(), rect.height(), sep);

    // horizontal
    QVector<QLayoutStruct> hor_struct_list(3);

    hor_struct_list[0].stretch = 0;
    hor_struct_list[0].sizeHint
        = !docks[LeftPos].isEmpty() && docks[LeftPos].rect.isValid()
            ? docks[LeftPos].rect.width()
            : docks[LeftPos].sizeHint().width();
    hor_struct_list[0].minimumSize = docks[LeftPos].minimumSize().width();
    hor_struct_list[0].maximumSize = docks[LeftPos].maximumSize().width();
    hor_struct_list[0].expansive = false;
    hor_struct_list[0].empty = docks[LeftPos].isEmpty();

    hor_struct_list[1].stretch = central_size.width();
    hor_struct_list[1].sizeHint
        = central_size.width() == 0 ? central_hint.width() : central_size.width();
    hor_struct_list[1].minimumSize = central_min.width();
    hor_struct_list[1].maximumSize = have_central ? QWIDGETSIZE_MAX : 0;
    hor_struct_list[1].expansive = have_central;
    hor_struct_list[1].empty = !have_central;

    hor_struct_list[2].stretch = 0;
    hor_struct_list[2].sizeHint
        = !docks[RightPos].isEmpty() && docks[RightPos].rect.isValid()
            ? docks[RightPos].rect.width()
            : docks[RightPos].sizeHint().width();
    hor_struct_list[2].minimumSize = docks[RightPos].minimumSize().width();
    hor_struct_list[2].maximumSize = docks[RightPos].maximumSize().width();
    hor_struct_list[2].expansive = false;
    hor_struct_list[2].empty = docks[RightPos].isEmpty();

    for (int i = 0; i < 3; ++i) {
        hor_struct_list[i].sizeHint
            = qMax(hor_struct_list[i].sizeHint, hor_struct_list[i].minimumSize);
    }

    qGeomCalc(hor_struct_list, 0, 3, rect.left(), rect.width(), sep);

    if (!docks[TopPos].isEmpty()) {
        docks[TopPos].rect = QRect(rect.left(), rect.top(), rect.width(), ver_struct_list[0].size);
        docks[TopPos].fitItems();
    }

    if (!docks[BottomPos].isEmpty()) {
        docks[BottomPos].rect = QRect(rect.left(), ver_struct_list[2].pos,
                                        rect.width(), ver_struct_list[2].size);
        docks[BottomPos].fitItems();
    }

    if (!docks[LeftPos].isEmpty()) {
        docks[LeftPos].rect = QRect(rect.left(), ver_struct_list[1].pos,
                                    hor_struct_list[0].size, ver_struct_list[1].size);
        docks[LeftPos].fitItems();
    }

    if (!docks[RightPos].isEmpty()) {
        docks[RightPos].rect = QRect(hor_struct_list[2].pos, ver_struct_list[1].pos,
                                    hor_struct_list[2].size, ver_struct_list[1].size);
        docks[RightPos].fitItems();
    }

    if (have_central) {
        centralWidgetRect = QRect(hor_struct_list[1].pos, ver_struct_list[1].pos,
                                    hor_struct_list[1].size, ver_struct_list[1].size);
    }
}

void QDockWidgetLayout::clear()
{
    for (int i = 0; i < PosCount; ++i)
        docks[i].clear();

    rect = QRect();
}

QSize QDockWidgetLayout::calculateSize(const QSize &szC,
                                              const QSize &szL,
                                              const QSize &szR,
                                              const QSize &szT,
                                              const QSize &szB) const
{
    int h1, h2, h3, w1, w2, w3;

    w1 = (corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea ? szL.width() : 0)
         + szT.width()
         + (corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea ? szR.width() : 0);
    w2 = szL.width() + szR.width() + szC.width();
    w3 = (corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea ? szL.width() : 0)
         + szB.width()
         + (corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea ? szR.width() : 0);

    h1 = (corners[Qt::TopLeftCorner] == Qt::TopDockWidgetArea ? szT.height() : 0)
         + szL.height()
         + (corners[Qt::BottomLeftCorner] == Qt::BottomDockWidgetArea ? szB.height() : 0);
    h2 = szT.height() + szB.height() + szC.height();
    h3 = (corners[Qt::TopRightCorner] == Qt::TopDockWidgetArea ? szT.height() : 0)
         + szR.height()
         + (corners[Qt::BottomRightCorner] == Qt::BottomDockWidgetArea ? szB.height() : 0);

    int left = 0, right = 0, top = 0, bottom = 0;
    if (!docks[LeftPos].isEmpty() && !szL.isEmpty())
        left += sep;
    if (!docks[RightPos].isEmpty() && !szR.isEmpty())
        right += sep;
    if (!docks[TopPos].isEmpty() && !szT.isEmpty())
        top += sep;
    if (!docks[BottomPos].isEmpty() && !szB.isEmpty())
        bottom += sep;

    return QSize(qMax(qMax(w1, w2), w3) + left + right, qMax(qMax(h1, h2), h3) + top + bottom);
}

QSize QDockWidgetLayout::sizeHint() const
{
/*
    return calculateSize((centralWidgetItem
                          ? centralWidgetItem->sizeHint()
                          : QSize(0, 0)),
                         docks[LeftPos].sizeHint(),
                         docks[RightPos].sizeHint(),
                         docks[TopPos].sizeHint(),
                         docks[BottomPos].sizeHint()); */

    QSize ls = docks[LeftPos].sizeHint();
    QSize rs = docks[RightPos].sizeHint();
    QSize ts = docks[TopPos].sizeHint();
    QSize bs = docks[BottomPos].sizeHint();
    QSize cs = centralWidgetItem == 0 ? QSize(0, 0) : centralWidgetItem->sizeHint();

    int sep_w = 0, sep_h = 0;
    if (docks[LeftPos].isEmpty())
        sep_w += sep;
    if (!docks[RightPos].isEmpty())
        sep_w += sep;
    if (!docks[TopPos].isEmpty())
        sep_h += sep;
    if (!docks[BottomPos].isEmpty())
        sep_h += sep;

    int w = qMax(ts.width(), ls.width() + cs.width() + rs.width() + sep_w, bs.width());
    int h = ts.height() + qMax(ls.height(), cs.height(), rs.height()) + bs.height() + sep_h;

    return QSize(w, h);
}

QSize QDockWidgetLayout::minimumSize() const
{
/*
    return calculateSize((centralWidgetItem
                          ? centralWidgetItem->minimumSize()
                          : QSize(0, 0)),
                         docks[LeftPos].minimumSize(),
                         docks[RightPos].minimumSize(),
                         docks[TopPos].minimumSize(),
                         docks[BottomPos].minimumSize()); */
    QSize ls = docks[LeftPos].minimumSize();
    QSize rs = docks[RightPos].minimumSize();
    QSize ts = docks[TopPos].minimumSize();
    QSize bs = docks[BottomPos].minimumSize();
    QSize cs = centralWidgetItem == 0 ? QSize(0, 0) : centralWidgetItem->minimumSize();

    int sep_w = 0, sep_h = 0;
    if (!docks[LeftPos].isEmpty())
        sep_w += sep;
    if (!docks[RightPos].isEmpty())
        sep_w += sep;
    if (!docks[TopPos].isEmpty())
        sep_h += sep;
    if (!docks[BottomPos].isEmpty())
        sep_h += sep;

    int w = qMax(ts.width(), ls.width() + cs.width() + rs.width() + sep_w, bs.width());
    int h = ts.height() + qMax(ls.height(), cs.height(), rs.height()) + bs.height() + sep_h;

    return QSize(w, h);
}

void QDockWidgetLayout::addDockWidget(DockPos pos, QDockWidget *dockWidget,
                                             Qt::Orientation orientation)
{
    QWidgetItem *dockWidgetItem = new QWidgetItem(dockWidget);
    QDockAreaLayoutInfo &info = docks[pos];
    if (orientation == info.o) {
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

    int index = path.takeLast();
    QDockAreaLayoutItem &item = this->item(path);
    Q_ASSERT(item.subinfo != 0);

    item.subinfo->split(index, orientation, new QWidgetItem(dockWidget));
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

    bool have_central = centralWidgetItem != 0 && !centralWidgetItem->isEmpty();
    QSize central_min(0, 0);
    QSize central_max(0, 0);
    if (have_central) {
        central_min = centralWidgetItem->minimumSize();
        central_max = QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    }

    if (cache->isEmpty()) {
        QVector<QLayoutStruct> &list = *cache;

        list.resize(3);
        if (index == LeftPos || index == RightPos) {
            if (docks[LeftPos].isEmpty()) {
                list[0].empty = true;
            } else {
                list[0].empty = false;
                list[0].pos = docks[LeftPos].rect.left();
                list[0].size = docks[LeftPos].rect.width();
                list[0].minimumSize = docks[LeftPos].minimumSize().width();
                list[0].maximumSize = docks[LeftPos].maximumSize().width();
            }
            if (!have_central) {
                list[1].empty = true;
            } else {
                list[1].empty = false;
                list[1].pos = centralWidgetRect.left();
                list[1].size = centralWidgetRect.width();
                list[1].minimumSize = central_min.width();
                list[1].maximumSize = central_max.width();
            }
            if (docks[RightPos].isEmpty()) {
                list[2].empty = true;
            } else {
                list[2].empty = false;
                list[2].pos = docks[RightPos].rect.left();
                list[2].size = docks[RightPos].rect.width();
                list[2].minimumSize = docks[RightPos].minimumSize().width();
                list[2].maximumSize = docks[RightPos].maximumSize().width();
            }
        } else {
            if (docks[TopPos].isEmpty()) {
                list[0].empty = true;
            } else {
                list[0].empty = false;
                list[0].pos = docks[TopPos].rect.top();
                list[0].size = docks[TopPos].rect.height();
                list[0].minimumSize = docks[TopPos].minimumSize().height();
                list[0].maximumSize = docks[TopPos].maximumSize().height();
            }
            if (!have_central
                    && docks[LeftPos].isEmpty()
                    && docks[RightPos].isEmpty()) {
                list[1].empty = true;
            } else {
                list[1].empty = false;
                QRect r;
                if (!docks[LeftPos].isEmpty())
                    r = docks[LeftPos].rect;
                if (!r.isValid() && have_central)
                    r = centralWidgetRect;
                if (!r.isValid() && !docks[RightPos].isEmpty())
                    r = docks[RightPos].rect;
                list[1].pos = r.top();
                list[1].size = r.height();
                list[1].minimumSize
                    = qMax(docks[LeftPos].minimumSize().height(),
                            central_min.height(),
                            docks[RightPos].minimumSize().height());
                list[1].maximumSize
                    = qMin(docks[LeftPos].maximumSize().height(),
                            central_max.height(),
                            docks[RightPos].maximumSize().height());
            }
            if (docks[BottomPos].isEmpty()) {
                list[2].empty = true;
            } else {
                list[2].empty = false;
                list[2].pos = docks[BottomPos].rect.top();
                list[2].size = docks[BottomPos].rect.height();
                list[2].minimumSize = docks[BottomPos].minimumSize().height();
                list[2].maximumSize = docks[BottomPos].maximumSize().height();
            }
        }
    }

    QVector<QLayoutStruct> list = *cache;
    int sep_index = index == LeftPos || index == TopPos
                        ? 0 : 1;
    Qt::Orientation o = index == LeftPos || index == RightPos
                        ? Qt::Horizontal
                        : Qt::Vertical;

    delta = pick(o, dest - origin);
    delta = ::separatorMove(list, sep_index, delta, sep);

    if (index == LeftPos || index == RightPos) {
        if (!docks[LeftPos].isEmpty()) {
            QRect r(list[0].pos, docks[LeftPos].rect.top(),
                    list[0].size, docks[LeftPos].rect.height());
            if (r != docks[LeftPos].rect) {
                docks[LeftPos].rect = r;
                docks[LeftPos].fitItems();
            }
        }
        if (have_central) {
            centralWidgetRect
                = QRect(list[1].pos, centralWidgetRect.top(),
                        list[1].size, centralWidgetRect.height());
        }
        if (!docks[RightPos].isEmpty()) {
            QRect r(list[2].pos, docks[RightPos].rect.top(),
                    list[2].size, docks[RightPos].rect.height());
            if (r != docks[RightPos].rect) {
                docks[RightPos].rect = r;
                docks[RightPos].fitItems();
            }
        }
    } else {
        if (!docks[TopPos].isEmpty()) {
            QRect r(docks[TopPos].rect.left(), list[0].pos,
                    docks[TopPos].rect.width(), list[0].size);
            if (r != docks[TopPos].rect) {
                docks[TopPos].rect = r;
                docks[TopPos].fitItems();
            }
        }
        if (have_central
                || !docks[LeftPos].isEmpty()
                || !docks[RightPos].isEmpty()) {
            if (!docks[LeftPos].isEmpty()) {
                QRect r(docks[LeftPos].rect.left(), list[1].pos,
                        docks[LeftPos].rect.width(), list[1].size);
                if (r != docks[LeftPos].rect) {
                    docks[LeftPos].rect = r;
                    docks[LeftPos].fitItems();
                }
            }
            if (have_central) {
                centralWidgetRect
                    = QRect(centralWidgetRect.left(), list[1].pos,
                            centralWidgetRect.width(), list[1].size);
            }
            if (!docks[RightPos].isEmpty()) {
                QRect r(docks[RightPos].rect.left(), list[1].pos,
                        docks[RightPos].rect.width(), list[1].size);
                if (r != docks[RightPos].rect) {
                    docks[RightPos].rect = r;
                    docks[RightPos].fitItems();
                }
            }
        }
        if (!docks[BottomPos].isEmpty()) {
            QRect r(docks[BottomPos].rect.left(), list[2].pos,
                    docks[BottomPos].rect.width(), list[2].size);
            if (r != docks[BottomPos].rect) {
                docks[BottomPos].rect = r;
                docks[BottomPos].fitItems();
            }
        }
    }
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

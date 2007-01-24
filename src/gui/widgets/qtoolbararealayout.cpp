#include <QWidgetItem>
#include <QToolBar>
#include <QStyleOption>
#include <qdebug.h>

#include "qtoolbararealayout_p.h"
#include "qmainwindowlayout_p.h"
#include "qwidgetanimator_p.h"
#include "qtoolbarhandle_p.h"

/******************************************************************************
** QToolBarAreaLayoutItem
*/

QSize QToolBarAreaLayoutItem::minimumSize() const
{
    if (skip())
        return QSize(0, 0);
    return widgetItem->minimumSize();
}

QSize QToolBarAreaLayoutItem::sizeHint() const
{
    if (skip())
        return QSize(0, 0);
    return widgetItem->sizeHint();
}

bool QToolBarAreaLayoutItem::skip() const
{
    if (gap)
        return false;
    return widgetItem == 0 || widgetItem->isEmpty();
}

/******************************************************************************
** QToolBarAreaLayoutLine
*/

QToolBarAreaLayoutLine::QToolBarAreaLayoutLine(Qt::Orientation orientation)
    : o(orientation)
{
}

QSize QToolBarAreaLayoutLine::sizeHint() const
{
    int a = 0, b = 0;
    for (int i = 0; i < toolBarItems.count(); ++i) {
        const QToolBarAreaLayoutItem &item = toolBarItems.at(i);
        if (item.skip())
            continue;

        QSize sh = item.sizeHint();
        a += pick(o, sh);
        b = qMax(b, perp(o, sh));
    }

    QSize result;
    rpick(o, result) = a;
    rperp(o, result) = b;

    return result;
}

QSize QToolBarAreaLayoutLine::minimumSize() const
{
    int a = 0, b = 0;
    for (int i = 0; i < toolBarItems.count(); ++i) {
        const QToolBarAreaLayoutItem &item = toolBarItems[i];
        if (item.skip())
            continue;

        QSize ms = item.minimumSize();
        a += pick(o, ms);
        b = qMax(b, perp(o, ms));
    }

    QSize result;
    rpick(o, result) = a;
    rperp(o, result) = b;

    return result;
}

void QToolBarAreaLayoutLine::fitLayout()
{
    // find out how much space our items are using
    int used = 0;
    int last = -1;
    for (int i = 0; i < toolBarItems.count(); ++i) {
        QToolBarAreaLayoutItem &item = toolBarItems[i];
        if (item.skip())
            continue;

        if (item.size == -1) {
            if (item.gap)
                item.size = pick(o, item.sizeHint());
            else
                item.size = pick(o, item.minimumSize());
        } else {
            item.size = qMin(item.size, pick(o, item.sizeHint()));
        }

        used += item.size;
        last = i;
    }

    int delta = pick(o, rect.size()) - used;

    if (delta > 0) { // we have space to grow - starting from the left
        for (int i = 0; i < toolBarItems.count(); ++i) {
            QToolBarAreaLayoutItem &item = toolBarItems[i];
            if (item.skip())
                continue;

            int hint = pick(o, item.sizeHint());
            if (hint > item.size) {
                int grow = qMin(hint - item.size, delta);
                item.size += grow;
                delta -= grow;
            }

            if (delta == 0)
                break;
        }
    } else if (delta < 0) { // we need to shrink - starting from the right
        delta = -delta;
        for (int i = toolBarItems.count() - 1; i >= 0; --i) {
            QToolBarAreaLayoutItem &item = toolBarItems[i];
            if (item.skip())
                continue;

            int m = pick(o, item.minimumSize());
            if (m < item.size) {
                int shrink = qMin(item.size - m, delta);
                item.size -= shrink;
                delta -= shrink;
            }

            if (delta == 0)
                break;
        }
    }

    // calculate the positions from the sizes
    int pos = 0;
    for (int i = 0; i < toolBarItems.count(); ++i) {
        QToolBarAreaLayoutItem &item = toolBarItems[i];
        if (item.skip())
            continue;

        item.pos = pos;
        if (i == last) // stretch the last item to the end of the line
            item.size = qMax(0, pick(o, rect.size()) - item.pos);
        pos += item.size;
    }
}

bool QToolBarAreaLayoutLine::skip() const
{
    for (int i = 0; i < toolBarItems.count(); ++i) {
        if (!toolBarItems.at(i).skip())
            return false;
    }
    return true;
}

/******************************************************************************
** QToolBarAreaLayoutInfo
*/

QToolBarAreaLayoutInfo::QToolBarAreaLayoutInfo(QInternal::DockPosition pos)
    : dockPos(pos)
{
    switch (pos) {
        case QInternal::LeftDock:
        case QInternal::RightDock:
            o = Qt::Vertical;
            break;
        case QInternal::TopDock:
        case QInternal::BottomDock:
            o = Qt::Horizontal;
            break;
        default:
            o = Qt::Horizontal;
            break;
    }
}

QSize QToolBarAreaLayoutInfo::sizeHint() const
{
    int a = 0, b = 0;
    for (int i = 0; i < lines.count(); ++i) {
        const QToolBarAreaLayoutLine &l = lines.at(i);
        if (l.skip())
            continue;

        QSize hint = l.sizeHint();
        a = qMax(a, pick(o, hint));
        b += perp(o, hint);
    }

    QSize result;
    rpick(o, result) = a;
    rperp(o, result) = b;

    return result;
}

QSize QToolBarAreaLayoutInfo::minimumSize() const
{
    int a = 0, b = 0;
    for (int i = 0; i < lines.count(); ++i) {
        const QToolBarAreaLayoutLine &l = lines.at(i);
        if (l.skip())
            continue;

        QSize m = l.minimumSize();
        a = qMax(a, pick(o, m));
        b += perp(o, m);
    }

    QSize result;
    rpick(o, result) = a;
    rperp(o, result) = b;

    return result;
}

void QToolBarAreaLayoutInfo::fitLayout()
{
    int b = 0;

    bool reverse = dockPos == QInternal::RightDock || dockPos == QInternal::BottomDock;

    int i = reverse ? lines.count() - 1 : 0;
    for (;;) {
        if (reverse && i < 0 || !reverse && i == lines.count())
            break;

        QToolBarAreaLayoutLine &l = lines[i];
        if (!l.skip()) {
            if (o == Qt::Horizontal) {
                l.rect.setLeft(rect.left());
                l.rect.setRight(rect.right());
                l.rect.setTop(b + rect.top());
                b += l.sizeHint().height();
                l.rect.setBottom(b - 1 + rect.top());
            } else {
                l.rect.setTop(rect.top());
                l.rect.setBottom(rect.bottom());
                l.rect.setLeft(b + rect.left());
                b += l.sizeHint().width();
                l.rect.setRight(b - 1 + rect.left());
            }

            l.fitLayout();
        }

        i += reverse ? -1 : 1;
    }
}

void QToolBarAreaLayoutInfo::insertToolBar(QToolBar *before, QToolBar *toolBar)
{
    toolBar->setOrientation(o);

    if (before == 0) {
        if (lines.isEmpty())
            lines.append(QToolBarAreaLayoutLine(o));
        lines.last().toolBarItems.append(new QToolBarWidgetItem(toolBar));
        return;
    }

    for (int j = 0; j < lines.count(); ++j) {
        QToolBarAreaLayoutLine &line = lines[j];

        for (int k = 0; k < line.toolBarItems.count(); ++k) {
            if (line.toolBarItems.at(k).widgetItem->widget() == before) {
                line.toolBarItems.insert(k, new QToolBarWidgetItem(toolBar));
                return;
            }
        }
    }
}

void QToolBarAreaLayoutInfo::removeToolBar(QToolBar *toolBar)
{
    for (int j = 0; j < lines.count(); ++j) {
        QToolBarAreaLayoutLine &line = lines[j];

        for (int k = 0; k < line.toolBarItems.count(); ++k) {
            QToolBarAreaLayoutItem &item = line.toolBarItems[k];
            if (item.widgetItem->widget() == toolBar) {
                delete item.widgetItem;
                item.widgetItem = 0;
                line.toolBarItems.removeAt(k);

                if (line.toolBarItems.isEmpty() && j < lines.count() - 1)
                    lines.removeAt(j);

                return;
            }
        }
    }
}

void QToolBarAreaLayoutInfo::insertToolBarBreak(QToolBar *before)
{
    if (before == 0) {
        if (!lines.isEmpty() && lines.last().toolBarItems.isEmpty())
            return;
        lines.append(QToolBarAreaLayoutLine(o));
        return;
    }

    for (int j = 0; j < lines.count(); ++j) {
        QToolBarAreaLayoutLine &line = lines[j];

        for (int k = 0; k < line.toolBarItems.count(); ++k) {
            if (line.toolBarItems.at(k).widgetItem->widget() == before) {
                if (k == 0)
                    return;

                QToolBarAreaLayoutLine newLine(o);
                newLine.toolBarItems = line.toolBarItems.mid(k);
                line.toolBarItems = line.toolBarItems.mid(0, k);
                lines.insert(j + 1, newLine);

                return;
            }
        }
    }
}

void QToolBarAreaLayoutInfo::removeToolBarBreak(QToolBar *before)
{
    for (int j = 0; j < lines.count(); ++j) {
        const QToolBarAreaLayoutLine &line = lines.at(j);

        for (int k = 0; k < line.toolBarItems.count(); ++k) {
            if (line.toolBarItems.at(k).widgetItem->widget() == before) {
                if (k != 0)
                    return;
                if (j == 0)
                    return;

                lines[j - 1].toolBarItems += lines[j].toolBarItems;
                lines.removeAt(j);

                return;
            }
        }
    }
}

QList<int> QToolBarAreaLayoutInfo::gapIndex(const QPoint &pos) const
{
    int p = pick(o, pos);

    if (rect.contains(pos)) {
        for (int j = 0; j < lines.count(); ++j) {
            const QToolBarAreaLayoutLine &line = lines.at(j);
            if (line.skip())
                continue;
            if (!line.rect.contains(pos))
                continue;

            int k = 0;
            for (; k < line.toolBarItems.count(); ++k) {
                const QToolBarAreaLayoutItem &item = line.toolBarItems.at(k);
                if (item.skip())
                    continue;
                if (p > item.pos + item.size)
                    continue;
                if (p > item.pos + item.size/2)
                    ++k;
                break;
            }

            QList<int> result;
            result << j << k;
            return result;
        }
    } else if (appendLineDropRect().contains(pos)) {
        QList<int> result;
        result << lines.count() << 0;
        return result;
    }

    return QList<int>();
}

bool QToolBarAreaLayoutInfo::insertGap(QList<int> path, QLayoutItem *item)
{
    int j = path.at(0);
    if (j == lines.count())
        lines.append(QToolBarAreaLayoutLine(o));

    QToolBarAreaLayoutLine &line = lines[j];
    int k = path.at(1);

    QToolBarAreaLayoutItem gap_item;
    gap_item.gap = true;
    gap_item.widgetItem = item;
    line.toolBarItems.insert(k, gap_item);

    return true;

}

void QToolBarAreaLayoutInfo::clear()
{
    lines.clear();
    rect = QRect(0, 0, -1, -1);
}

QRect QToolBarAreaLayoutInfo::itemRect(QList<int> path) const
{
    int j = path.at(0);
    int k = path.at(1);

    const QToolBarAreaLayoutLine &line = lines.at(j);
    const QToolBarAreaLayoutItem &item = line.toolBarItems.at(k);

    QRect result = line.rect;

    if (o == Qt::Horizontal) {
        result.setLeft(item.pos + line.rect.left());
        result.setWidth(item.size);
    } else {
        result.setTop(item.pos + line.rect.top());
        result.setHeight(item.size);
    }

    return result;
}

QRect QToolBarAreaLayoutInfo::appendLineDropRect() const
{
    QRect result;

    switch (dockPos) {
        case QInternal::LeftDock:
            result = QRect(rect.right(), rect.top(),
                            EmptyDockAreaSize, rect.height());
            break;
        case QInternal::RightDock:
            result = QRect(rect.left() - EmptyDockAreaSize, rect.top(),
                            EmptyDockAreaSize, rect.height());
            break;
        case QInternal::TopDock:
            result = QRect(rect.left(), rect.bottom() + 1,
                            rect.width(), EmptyDockAreaSize);
            break;
        case QInternal::BottomDock:
            result = QRect(rect.left(), rect.top() - EmptyDockAreaSize,
                            rect.width(), EmptyDockAreaSize);
            break;
        default:
            break;
    }

    return result;
}

/******************************************************************************
** QToolBarAreaLayout
*/

QToolBarAreaLayout::QToolBarAreaLayout(QMainWindow *win)
{
    mainWindow = win;
    for (int i = 0; i < QInternal::DockCount; ++i) {
        QInternal::DockPosition pos = static_cast<QInternal::DockPosition>(i);
        docks[i] = QToolBarAreaLayoutInfo(pos);
    }
}

QRect QToolBarAreaLayout::fitLayout()
{
    QSize left_hint = docks[QInternal::LeftDock].sizeHint();
    QSize right_hint = docks[QInternal::RightDock].sizeHint();
    QSize top_hint = docks[QInternal::TopDock].sizeHint();
    QSize bottom_hint = docks[QInternal::BottomDock].sizeHint();

    QRect center = rect.adjusted(left_hint.width(), top_hint.height(),
                                    -right_hint.width(), -bottom_hint.height());

    docks[QInternal::TopDock].rect = QRect(rect.left(), rect.top(),
                                rect.width(), top_hint.height());
    docks[QInternal::LeftDock].rect = QRect(rect.left(), center.top(),
                                left_hint.width(), center.height());
    docks[QInternal::RightDock].rect = QRect(center.right() + 1, center.top(),
                                    right_hint.width(), center.height());
    docks[QInternal::BottomDock].rect = QRect(rect.left(), center.bottom() + 1,
                                    rect.width(), bottom_hint.height());

    docks[QInternal::TopDock].fitLayout();
    docks[QInternal::LeftDock].fitLayout();
    docks[QInternal::RightDock].fitLayout();
    docks[QInternal::BottomDock].fitLayout();

    return center;
}

QSize QToolBarAreaLayout::minimumSize(const QSize &centerMin) const
{
    QSize result = centerMin;

    QSize left_min = docks[QInternal::LeftDock].minimumSize();
    QSize right_min = docks[QInternal::RightDock].minimumSize();
    QSize top_min = docks[QInternal::TopDock].minimumSize();
    QSize bottom_min = docks[QInternal::BottomDock].minimumSize();

    result.setWidth(qMax(top_min.width(), result.width()));
    result.setWidth(qMax(bottom_min.width(), result.width()));
    result.setHeight(qMax(left_min.height(), result.height()));
    result.setHeight(qMax(right_min.height(), result.height()));

    result.rwidth() += left_min.width() + right_min.width();
    result.rheight() += top_min.height() + bottom_min.height();

    return result;
}

QSize QToolBarAreaLayout::sizeHint(const QSize &centerHint) const
{
    QSize result = centerHint;

    QSize left_hint = docks[QInternal::LeftDock].sizeHint();
    QSize right_hint = docks[QInternal::RightDock].sizeHint();
    QSize top_hint = docks[QInternal::TopDock].sizeHint();
    QSize bottom_hint = docks[QInternal::BottomDock].sizeHint();

    result.setWidth(qMax(top_hint.width(), result.width()));
    result.setWidth(qMax(bottom_hint.width(), result.width()));
    result.setHeight(qMax(left_hint.height(), result.height()));
    result.setHeight(qMax(right_hint.height(), result.height()));

    result.rwidth() += left_hint.width() + right_hint.width();
    result.rheight() += top_hint.height() + bottom_hint.height();

    return result;
}

QLayoutItem *QToolBarAreaLayout::itemAt(int *x, int index) const
{
    Q_ASSERT(x != 0);

    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QToolBarAreaLayoutInfo &dock = docks[i];

        for (int j = 0; j < dock.lines.count(); ++j) {
            const QToolBarAreaLayoutLine &line = dock.lines.at(j);

            for (int k = 0; k < line.toolBarItems.count(); ++k) {
                if ((*x)++ == index)
                    return line.toolBarItems.at(k).widgetItem;
            }
        }
    }

    return 0;
}

QLayoutItem *QToolBarAreaLayout::takeAt(int *x, int index)
{
    Q_ASSERT(x != 0);

    for (int i = 0; i < QInternal::DockCount; ++i) {
        QToolBarAreaLayoutInfo &dock = docks[i];

        for (int j = 0; j < dock.lines.count(); ++j) {
            QToolBarAreaLayoutLine &line = dock.lines[j];

            for (int k = 0; k < line.toolBarItems.count(); ++k) {
                if ((*x)++ == index) {
                    QLayoutItem *result = line.toolBarItems.takeAt(k).widgetItem;
                    if (line.toolBarItems.isEmpty())
                        dock.lines.removeAt(j);
                    return result;
                }
            }
        }
    }

    return 0;
}

void QToolBarAreaLayout::deleteAllLayoutItems()
{
    for (int i = 0; i < QInternal::DockCount; ++i) {
        QToolBarAreaLayoutInfo &dock = docks[i];

        for (int j = 0; j < dock.lines.count(); ++j) {
            QToolBarAreaLayoutLine &line = dock.lines[j];

            for (int k = 0; k < line.toolBarItems.count(); ++k) {
                QToolBarAreaLayoutItem &item = line.toolBarItems[k];
                delete item.widgetItem;
                item.widgetItem = 0;
            }
        }
    }
}

QInternal::DockPosition QToolBarAreaLayout::findToolBar(QToolBar *toolBar) const
{
    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QToolBarAreaLayoutInfo &dock = docks[i];

        for (int j = 0; j < dock.lines.count(); ++j) {
            const QToolBarAreaLayoutLine &line = dock.lines.at(j);

            for (int k = 0; k < line.toolBarItems.count(); ++k) {
                if (line.toolBarItems.at(k).widgetItem->widget() == toolBar)
                    return static_cast<QInternal::DockPosition>(i);
            }
        }
    }

    return QInternal::DockCount;
}

void QToolBarAreaLayout::insertToolBar(QToolBar *before, QToolBar *toolBar)
{
    QInternal::DockPosition pos = findToolBar(before);
    if (pos == QInternal::DockCount)
        return;
    docks[pos].insertToolBar(before, toolBar);
}

void QToolBarAreaLayout::removeToolBar(QToolBar *toolBar)
{
    QInternal::DockPosition pos = findToolBar(toolBar);
    if (pos == QInternal::DockCount)
        return;
    docks[pos].removeToolBar(toolBar);
}

void QToolBarAreaLayout::addToolBar(QInternal::DockPosition pos, QToolBar *toolBar)
{
    docks[pos].insertToolBar(0, toolBar);
}

void QToolBarAreaLayout::insertToolBarBreak(QToolBar *before)
{
    QInternal::DockPosition pos = findToolBar(before);
    if (pos == QInternal::DockCount)
        return;
    docks[pos].insertToolBarBreak(before);
}

void QToolBarAreaLayout::removeToolBarBreak(QToolBar *before)
{
    QInternal::DockPosition pos = findToolBar(before);
    if (pos == QInternal::DockCount)
        return;
    docks[pos].removeToolBarBreak(before);
}

void QToolBarAreaLayout::addToolBarBreak(QInternal::DockPosition pos)
{
    docks[pos].insertToolBarBreak(0);
}

void QToolBarAreaLayout::apply(bool animate)
{
    QMainWindowLayout *layout = qobject_cast<QMainWindowLayout*>(mainWindow->layout());
    Q_ASSERT(layout != 0);

    Qt::LayoutDirection dir = mainWindow->layoutDirection();

    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QToolBarAreaLayoutInfo &dock = docks[i];

        for (int j = 0; j < dock.lines.count(); ++j) {
            const QToolBarAreaLayoutLine &line = dock.lines.at(j);
            if (line.skip())
                continue;

            for (int k = 0; k < line.toolBarItems.count(); ++k) {
                const QToolBarAreaLayoutItem &item = line.toolBarItems.at(k);
                if (item.skip() || item.gap)
                    continue;

                QRect r;
                if (line.o == Qt::Horizontal) {
                    r.setTop(line.rect.top());
                    r.setBottom(line.rect.bottom());
                    r.setLeft(line.rect.left() + item.pos);
                    r.setRight(line.rect.left() + item.pos + item.size - 1);
                } else {
                    r.setLeft(line.rect.left());
                    r.setRight(line.rect.right());
                    r.setTop(line.rect.top() + item.pos);
                    r.setBottom(line.rect.top() + item.pos + item.size - 1);
                }

                QRect geo = r;
                if (dock.o == Qt::Horizontal)
                    geo = QStyle::visualRect(dir, line.rect, geo);
                QWidget *widget = item.widgetItem->widget();
                layout->widgetAnimator->animate(widget, geo, animate);
            }
        }
    }
}

bool QToolBarAreaLayout::toolBarBreak(QToolBar *toolBar) const
{
    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QToolBarAreaLayoutInfo &dock = docks[i];

        for (int j = 0; j < dock.lines.count(); ++j) {
            const QToolBarAreaLayoutLine &line = dock.lines.at(j);

            for (int k = 0; k < line.toolBarItems.count(); ++k) {
                if (line.toolBarItems.at(k).widgetItem->widget() == toolBar)
                    return k == 0;
            }
        }
    }

    return false;
}

void QToolBarAreaLayout::getStyleOptionInfo(QStyleOptionToolBar *option, QToolBar *toolBar) const
{
    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QToolBarAreaLayoutInfo &dock = docks[i];

        for (int j = 0; j < dock.lines.count(); ++j) {
            const QToolBarAreaLayoutLine &line = dock.lines.at(j);

            for (int k = 0; k < line.toolBarItems.count(); ++k) {
                if (line.toolBarItems.at(k).widgetItem->widget() == toolBar) {
                    if (line.toolBarItems.count() == 1)
                        option->positionWithinLine = QStyleOptionToolBar::OnlyOne;
                    else if (k == 0)
                        option->positionWithinLine = QStyleOptionToolBar::Beginning;
                    else if (k == line.toolBarItems.count() - 1)
                        option->positionWithinLine = QStyleOptionToolBar::End;
                    else
                        option->positionWithinLine = QStyleOptionToolBar::Middle;

                    if (dock.lines.count() == 1)
                        option->positionOfLine = QStyleOptionToolBar::OnlyOne;
                    else if (j == 0)
                        option->positionOfLine = QStyleOptionToolBar::Beginning;
                    else if (j == dock.lines.count() - 1)
                        option->positionOfLine = QStyleOptionToolBar::End;
                    else
                        option->positionOfLine = QStyleOptionToolBar::Middle;

                    return;
                }
            }
        }
    }
}

QList<int> QToolBarAreaLayout::indexOf(QToolBar *toolBar) const
{
    QList<int> result;

    bool found = false;

    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QToolBarAreaLayoutInfo &dock = docks[i];

        for (int j = 0; j < dock.lines.count(); ++j) {
            const QToolBarAreaLayoutLine &line = dock.lines.at(j);

            for (int k = 0; k < line.toolBarItems.count(); ++k) {
                const QToolBarAreaLayoutItem &item = line.toolBarItems.at(k);
                if (!item.gap && item.widgetItem->widget() == toolBar) {
                    found = true;
                    result.prepend(k);
                    break;
                }
            }

            if (found) {
                result.prepend(j);
                break;
            }
        }

        if (found) {
            result.prepend(i);
            break;
        }
    }

    return result;
}

QList<int> QToolBarAreaLayout::gapIndex(const QPoint &pos) const
{
    Qt::LayoutDirection dir = mainWindow->layoutDirection();
    for (int i = 0; i < QInternal::DockCount; ++i) {
        QPoint p = pos;
        if (docks[i].o == Qt::Horizontal)
            p = QStyle::visualPos(dir, docks[i].rect, p);
        QList<int> result = docks[i].gapIndex(p);
        if (!result.isEmpty()) {
            result.prepend(i);
            return result;
        }
    }

    return QList<int>();
}

bool QToolBarAreaLayout::insertGap(QList<int> path, QLayoutItem *item)
{
    Q_ASSERT(!path.isEmpty());
    int i = path.takeFirst();
    Q_ASSERT(i >= 0 && i < QInternal::DockCount);
    return docks[i].insertGap(path, item);
}

void QToolBarAreaLayout::remove(QList<int> path)
{
    docks[path.at(0)].lines[path.at(1)].toolBarItems.removeAt(path.at(2));
}

void QToolBarAreaLayout::clear()
{
    for (int i = 0; i < QInternal::DockCount; ++i)
        docks[i].clear();
    rect = QRect(0, 0, -1, -1);
}

QToolBarAreaLayoutItem &QToolBarAreaLayout::item(QList<int> path)
{
    Q_ASSERT(path.count() == 3);

    Q_ASSERT(path.at(0) >= 0 && path.at(0) < QInternal::DockCount);
    QToolBarAreaLayoutInfo &info = docks[path.at(0)];
    Q_ASSERT(path.at(1) >= 0 && path.at(1) < info.lines.count());
    QToolBarAreaLayoutLine &line = info.lines[path.at(1)];
    Q_ASSERT(path.at(2) >= 0 && path.at(2) < line.toolBarItems.count());
    return line.toolBarItems[path.at(2)];
}

QRect QToolBarAreaLayout::itemRect(QList<int> path) const
{
    int i = path.takeFirst();

    QRect r = docks[i].itemRect(path);
    if (docks[i].o == Qt::Horizontal)
        r = QStyle::visualRect(mainWindow->layoutDirection(),
                                docks[i].rect, r);
    return r;
}

QLayoutItem *QToolBarAreaLayout::plug(QList<int> path)
{
    QToolBarAreaLayoutItem &item = this->item(path);
    Q_ASSERT(item.gap);
    Q_ASSERT(item.widgetItem != 0);
    item.gap = false;
    return item.widgetItem;
}

QLayoutItem *QToolBarAreaLayout::unplug(QList<int> path)
{
    QToolBarAreaLayoutItem &item = this->item(path);
    Q_ASSERT(!item.gap);
    item.gap = true;
    return item.widgetItem;
}

void QToolBarAreaLayout::saveState(QDataStream &stream) const
{
    // save toolbar state
    stream << (uchar) ToolBarStateMarkerEx;

    int lineCount = 0;
    for (int i = 0; i < QInternal::DockCount; ++i)
        lineCount += docks[i].lines.count();

    stream << lineCount;

    for (int i = 0; i < QInternal::DockCount; ++i) {
        const QToolBarAreaLayoutInfo &dock = docks[i];

        for (int j = 0; j < dock.lines.count(); ++j) {
            const QToolBarAreaLayoutLine &line = dock.lines.at(j);

            stream << i << line.toolBarItems.count();

            for (int k = 0; k < line.toolBarItems.count(); ++k) {
                const QToolBarAreaLayoutItem &item = line.toolBarItems.at(k);
                QWidget *widget = const_cast<QLayoutItem*>(item.widgetItem)->widget();
                QString objectName = widget->objectName();
                if (objectName.isEmpty()) {
                    qWarning("QMainWindow::saveState(): 'objectName' not set for QToolBar %p '%s'",
                                widget, widget->windowTitle().toLocal8Bit().constData());
                }
                stream << objectName;
                stream << (uchar) !widget->isHidden();
                stream << item.pos;
                stream << item.size;
                int dummy = 0;
                stream << dummy << dummy;
            }
        }
    }
}

bool QToolBarAreaLayout::restoreState(QDataStream &stream, const QList<QToolBar*> &toolBars)
{
    uchar tmarker;
    stream >> tmarker;
    if (tmarker != ToolBarStateMarker && tmarker != ToolBarStateMarkerEx)
        return false;

    int lines;
    stream >> lines;

    for (int j = 0; j < lines; ++j) {
        int pos;
        stream >> pos;
        if (pos < 0 || pos >= QInternal::DockCount)
            return false;
        int cnt;
        stream >> cnt;

        QToolBarAreaLayoutInfo &dock = docks[pos];
        QToolBarAreaLayoutLine line(dock.o);

        for (int k = 0; k < cnt; ++k) {
            QToolBarAreaLayoutItem item;

            QString objectName;
            stream >> objectName;
            uchar shown;
            stream >> shown;
            stream >> item.pos;
            stream >> item.size;
            int dummy;
            stream >> dummy;
            if (tmarker == ToolBarStateMarkerEx)
                stream >> dummy;

            QToolBar *toolBar = 0;
            for (int x = 0; x < toolBars.count(); ++x) {
                if (toolBars.at(x)->objectName() == objectName) {
                    toolBar = toolBars.at(x);
                    break;
                }
            }
            if (toolBar == 0) {
                qWarning("QMainWindow::restoreState(): cannot find a QToolBar with "
                         "matching 'objectName' (looking for \"%s\").",
                         qPrintable(objectName));
                continue;
            }

            item.widgetItem = new QToolBarWidgetItem(toolBar);
            toolBar->setOrientation(dock.o);
            toolBar->setVisible(shown);

            line.toolBarItems.append(item);
        }

        dock.lines.append(line);
    }


    return stream.status() == QDataStream::Ok;
}

#include "qgenerictreeview.h"
#include "qgenericheader.h"
#include <qitemdelegate.h>
#include <qapplication.h>
#include <qscrollbar.h>
#include <qpainter.h>
#include <qvector.h>
#include <qstyle.h>
#include <qevent.h>
#include <qpen.h>

#include <private/qgenerictreeview_p.h>
#define d d_func()
#define q q_func()

/*!
  \class QGenericTreeView qgenerictreeview.h

  \brief Tree view implementation

  This class implements a tree representation of a QGenericItemView working
  on a QAbstractItemModel.
*/

template <typename T>
inline void expand(QVector<T> &vec, int after, size_t n)
{
    size_t m = vec.size() - after - 1;
    vec.resize(vec.size() + n);
    T *b = static_cast<T *>(vec.data());
    T *src = b + after + 1;
    T *dst = src + n;
    memmove(dst, src, m * sizeof(T));
}

template <typename T>
inline void collapse(QVector<T> &vec, int after, size_t n)
{
    if (after + 1 + n < (size_t)vec.size()) {
        T *b = vec.data();
        T *dst = b + after + 1;
        T *src = dst + n;
        size_t m = vec.size() - n - after - 1;
        memmove(dst, src, m * sizeof(T));
    }
    vec.resize(vec.size() - n);
}

/*!
  \class QGenericTreeViewItem qgenerictreeview.cpp

  This class implements a QViewItem working on a QGenericTreeView.
*/

QGenericTreeView::QGenericTreeView(QAbstractItemModel *model, QWidget *parent)
    : QAbstractItemView(*new QGenericTreeViewPrivate, model, parent)
{
    setHeader(new QGenericHeader(model, Horizontal, this));
    d->header->setMovable(true);
    d->layout_parent_index = -1;
    d->layout_from_index = -1;
    d->layout_count = model->rowCount(root());
    d->rootDecoration = true;
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

QGenericTreeView::~QGenericTreeView()
{
}

QGenericHeader *QGenericTreeView::header() const
{
    return d->header;
}

void QGenericTreeView::setHeader(QGenericHeader *header)
{
    if (d->header) {
        QObject::disconnect(d->header, SIGNAL(sectionSizeChanged(int,int,int)),
                            this, SLOT(columnWidthChanged(int,int,int)));
        QObject::disconnect(d->header, SIGNAL(sectionIndexChanged(int,int,int)),
                            this, SLOT(contentsChanged()));
        QObject::disconnect(d->header, SIGNAL(sectionCountChanged(int,int)),
                            this, SLOT(columnCountChanged(int,int)));
        QObject::disconnect(d->header, SIGNAL(sectionHandleDoubleClicked(int,ButtonState)),
                            this, SLOT(resizeColumnToContents(int)));
        delete d->header;
    }
    d->header = header;
    setViewportMargins(0, d->header->sizeHint().height(), 0, 0);
    QObject::connect(d->header, SIGNAL(sectionSizeChanged(int,int,int)),
                     this, SLOT(columnWidthChanged(int,int,int)));
    QObject::connect(d->header, SIGNAL(sectionIndexChanged(int,int,int)),
                     this, SLOT(contentsChanged()));
    QObject::connect(d->header, SIGNAL(sectionCountChanged(int,int)),
                     this, SLOT(columnCountChanged(int,int)));
    QObject::connect(d->header, SIGNAL(sectionHandleDoubleClicked(int,ButtonState)),
                     this, SLOT(resizeColumnToContents(int)));
    d->header->setSelectionModel(selectionModel());

    updateGeometries();
}

int QGenericTreeView::indentation() const
{
    return d->indent;
}

void QGenericTreeView::setIndentation(int i)
{
    d->indent = i;
}

int QGenericTreeView::editColumn() const
{
    return d->editColumn;
}

void QGenericTreeView::setEditColumn(int column)
{
    d->editColumn = column;
}

bool QGenericTreeView::showRootDecoration() const
{
    return d->rootDecoration;
}

void QGenericTreeView::setShowRootDecoration(bool show)
{
    d->rootDecoration = show;
    d->viewport->update();
}

int QGenericTreeView::columnViewportPosition(int column) const
{
    int colp = d->header->sectionPosition(column) - d->header->offset();
    if (!QApplication::reverseLayout())
        return colp;
    return colp + (d->header->x() - d->viewport->x());
}

int QGenericTreeView::columnWidth(int column) const
{
    return d->header->sectionSize(column);
}

int QGenericTreeView::columnAt(int x) const
{
    int p = x + d->header->offset();
    if (!QApplication::reverseLayout())
        return d->header->sectionAt(p);
    return d->header->sectionAt(p - (d->header->x() - d->viewport->x()));
}

bool QGenericTreeView::isColumnHidden(int column) const
{
    return d->header->isSectionHidden(column);
}

void QGenericTreeView::hideColumn(int column)
{
    d->header->hideSection(column);
}

void QGenericTreeView::open(const QModelIndex &item)
{
    int v = verticalScrollBar()->value();
    int idx = d->viewIndex(item, v);
    if (idx > -1)
        d->open(idx);
}

void QGenericTreeView::close(const QModelIndex &item)
{
    int v = verticalScrollBar()->value();
    int idx = d->viewIndex(item, v);
    if (idx > -1)
        d->close(idx);
}

bool QGenericTreeView::isOpen(const QModelIndex &item) const
{
    int v = verticalScrollBar()->value();
    int idx = d->viewIndex(item, v);
    if (idx > -1)
        return d->isOpen(idx);
    return false;
}

void QGenericTreeView::paintEvent(QPaintEvent *e)
{
    QPainter painter(d->viewport);
    QRect area = e->rect();

    if (d->items.isEmpty())
        return;

    d->left = d->header->indexAt(d->header->offset() + area.left());
    d->right = d->header->indexAt(d->header->offset() + area.right());
    if (d->left > d->right) {
        int tmp = d->left;
        d->left = d->right;
        d->right = tmp;
    }

    const QGenericTreeViewItem *items = d->items.constData();

    QItemOptions options;
    getViewOptions(&options);
    QFontMetrics fontMetrics(this->fontMetrics());
    QAbstractItemDelegate *delegate = itemDelegate();
    QModelIndex index;
    QModelIndex current = selectionModel()->currentItem();

    int v = verticalScrollBar()->value();
    int h = d->viewport->height();
    int c = d->items.count();
    int i = d->itemAt(v);
    int y = d->coordinateAt(v, delegate->sizeHint(fontMetrics, options, items[i].index).height());
    while (y < h && i < c) {
        // prepare
        index = items[i].index;
        options.itemRect.setRect(0, y, 0, delegate->sizeHint(fontMetrics, options, index).height());
        options.focus = (index == current);
        d->current = i;
        // draw row
        drawRow(&painter, &options, index);
        // next row
        y += options.itemRect.height();
        ++i;
    }
}

void QGenericTreeView::drawRow(QPainter *painter, QItemOptions *options, const QModelIndex &index) const
{
    int y = options->itemRect.y();
    int width, height = options->itemRect.height();
    QColor base = options->palette.base();

    QModelIndex parent = model()->parent(index);
    QGenericHeader *header = d->header;
    QModelIndex current = selectionModel()->currentItem();
    bool focus = hasFocus() && current.isValid();
    bool reverse = QApplication::reverseLayout();

    int position;
    int headerSection;
    QModelIndex modelIndex;
    for (int headerIndex = d->left; headerIndex <= d->right; ++headerIndex) {
        headerSection = d->header->section(headerIndex);
        if (header->isSectionHidden(headerSection))
            continue;
        position = columnViewportPosition(headerSection);
        width = header->sectionSize(headerSection);
        modelIndex = model()->index(index.row(), headerSection, parent);
        options->focus = (focus && current == modelIndex);
        options->selected = selectionModel()->isSelected(modelIndex);
        if (headerSection == 0) {
            int i = d->indentation(d->current);
            options->itemRect.setRect(reverse ? position : i + position, y, width - i, height);
            painter->fillRect(position, y, width, height, base);
            drawBranches(painter, QRect(reverse ? position + width - i :
                                        position, y, i, options->itemRect.height()), index);
        } else {
            options->itemRect.setRect(position, y, width, height);
            painter->fillRect(position, y, width, height, base);
        }
        itemDelegate()->paint(painter, *options, modelIndex);
    }
}

void QGenericTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
//    painter->drawRect(rect);

    QModelIndex parent = model()->parent(index);
    QModelIndex current = parent;
    QModelIndex ancestor = model()->parent(current);
    bool reverse = QApplication::reverseLayout();
    int indent = d->indent;
    int level = d->items.at(d->current).level;
    int outer = d->rootDecoration ? 0 : 1;
    QRect primitive(reverse ? rect.left() : rect.right(), rect.top(), indent, rect.height());

    if (level >= outer) {
        // start with the innermost branch
        primitive.moveLeft(reverse ? primitive.left() : primitive.left() - indent);
        QStyle::SFlags flags = QStyle::Style_Item
                               | (model()->rowCount(parent) - 1 > index.row() ? QStyle::Style_Sibling : 0)
                               | (model()->hasChildren(index) ? QStyle::Style_Children : 0)
                               | (d->isOpen(d->current) ? QStyle::Style_Open : 0);
        style().drawPrimitive(QStyle::PE_TreeBranch, painter, primitive, palette(), flags);
    }
    // then go out level by level
    for (--level; level >= outer; --level) { // we have already drawn the innermost branch
        primitive.moveLeft(reverse ? primitive.left() + indent : primitive.left() - indent);
        style().drawPrimitive(QStyle::PE_TreeBranch, painter, primitive, palette(),
                              model()->rowCount(ancestor) - 1 > current.row() ? QStyle::Style_Sibling : 0);
        current = ancestor;
        ancestor = model()->parent(current);
    }
}

void QGenericTreeView::mousePressEvent(QMouseEvent *e)
{
    bool reverse = QApplication::reverseLayout();
    int scrollbar = reverse && verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
    int x = e->x() - d->header->x() + d->header->offset() + scrollbar;
    int column = d->header->sectionAt(x);
    int position = d->header->sectionPosition(column);
    int cx = reverse ? position + d->header->sectionSize(column) - x : x - position;
    int vi = d->item(e->y(), verticalScrollBar()->value());
    QModelIndex mi = d->modelIndex(vi);

    if (mi.isValid()) {
        int indent = d->indentation(vi) - d->header->offset();
        if (column == 0 && cx < (indent - d->indent))
            return; // we are in the empty area in front of the tree - do nothing
        if (column != 0 || cx > indent) {
            QAbstractItemView::mousePressEvent(e);
            return; // we are on an item - select it
        }
        if (d->isOpen(vi))
            d->close(vi);
        else
            d->open(vi);
    }
}

QModelIndex QGenericTreeView::itemAt(int, int y) const
{
    int vi = d->item(y, verticalScrollBar()->value());
    QModelIndex mi = d->modelIndex(vi);
    return model()->sibling(mi.row(), d->editColumn, mi);
}

int QGenericTreeView::horizontalOffset() const
{
    return d->header->offset();
}

int QGenericTreeView::verticalOffset() const
{
    // gives an estimate
    QItemOptions options;
    getViewOptions(&options);
    int iheight = d->delegate->sizeHint(fontMetrics(), options, model()->index(0, 0, 0)).height();
    int item = verticalScrollBar()->value() / d->verticalFactor;
    return item * iheight;
}

QRect QGenericTreeView::itemViewportRect(const QModelIndex &item) const
{
    if (!item.isValid())
        return QRect();

    int x = columnViewportPosition(item.column());
    int w = columnWidth(item.column());
    int v = verticalScrollBar()->value();
    int vi = d->viewIndex(item, v);
    if (vi < 0)
        return QRect();
    if (item.column() == 0) {
        int i = d->indentation(vi);
        x += i;
        w -= i;
    }
    int y = d->coordinate(vi, v);
    QItemOptions options;
    getViewOptions(&options);
    int h = itemDelegate()->sizeHint(fontMetrics(), options, d->modelIndex(vi)).height();
    return QRect(x, y, w, h);
}

void QGenericTreeView::ensureItemVisible(const QModelIndex &item)
{
    QRect area = viewport()->geometry();
    QRect rect = itemViewportRect(item);

    if (area.contains(rect))
        return;

    // vertical
    if (rect.top() < area.top()) { // above
        int v = verticalScrollBar()->value();
        int i = d->viewIndex(item, v);
        verticalScrollBar()->setValue(i * verticalFactor());
    } else if (rect.bottom() > area.bottom()) { // below
        QItemOptions options;
        getViewOptions(&options);
        QFontMetrics fontMetrics(this->fontMetrics());
        QAbstractItemDelegate *delegate = itemDelegate();
        int v = verticalScrollBar()->value();
        int i = d->viewIndex(item, v);
        int y = area.height();
        while (y > 0 && i > 0)
            y -= delegate->sizeHint(fontMetrics, options, d->items.at(i--).index).height();
        int a = (-y * verticalFactor()) / delegate->sizeHint(fontMetrics, options, d->items.at(i).index).height();
        verticalScrollBar()->setValue(++i * verticalFactor() + a);
    }

    // horizontal
    if (rect.left() < area.left()) { // left of
        horizontalScrollBar()->setValue(item.column() * horizontalFactor());
    } else if (rect.right() > area.right()) { // right of
        int c = item.column();
        int x = area.width();
        while (x > 0 && c > 0)
            x -= columnWidth(c--);
        int a = (-x * horizontalFactor()) / columnWidth(c);
        horizontalScrollBar()->setValue(++c * horizontalFactor() + a);
    }
}

QModelIndex QGenericTreeView::moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState)
{
    QModelIndex current = currentItem();
    int v = verticalScrollBar()->value();
    int vi = d->viewIndex(current, v); // FIXME: slow

    switch (cursorAction) {
    case QAbstractItemView::MoveDown:
        return d->modelIndex(d->below(vi));
    case QAbstractItemView::MoveUp:
        return d->modelIndex(d->above(vi));
    case QAbstractItemView::MoveLeft:
        if (d->isOpen(vi))
            d->close(vi);
        break;
    case QAbstractItemView::MoveRight:
        if (!d->isOpen(vi))
            d->open(vi);
        break;
    case QAbstractItemView::MovePageUp:
        return d->modelIndex(d->pageUp(vi));
    case QAbstractItemView::MovePageDown:
        return d->modelIndex(d->pageDown(vi));
    case QAbstractItemView::MoveHome:
        return model()->index(0, 0, 0);
    case QAbstractItemView::MoveEnd:
        return d->modelIndex(d->last());
    }
    return current;
}

void QGenericTreeView::setSelection(const QRect &rect, int command)
{
    QModelIndex tl = itemAt(rect.left(), rect.top());
    QModelIndex br = itemAt(rect.right(), rect.bottom());
    selectionModel()->select(QItemSelection(tl, br, model()), command);
}

QRect QGenericTreeView::selectionViewportRect(const QItemSelection &selection) const
{
    if (!selection.count())
        return QRect();
        
    QModelIndex bottomRight = model()->bottomRight(root());
    int top = d->items.count();
    int bottom = 0;
    int v = verticalScrollBar()->value();
    QItemSelectionRange r;
    QModelIndex topIndex, bottomIndex;
    for (int i = 0; i < selection.count(); ++i) {
        r = selection.at(i);
        topIndex = model()->index(r.top(), r.left(), r.parent());
        top = qMin(d->viewIndex(topIndex, v), top);
        bottomIndex = model()->index(r.bottom(), r.left(), r.parent());
        bottom = qMax(d->viewIndex(bottomIndex, v), bottom);
    }

    QItemOptions options;
    getViewOptions(&options);
    int bottomHeight = itemDelegate()->sizeHint(fontMetrics(), options, bottomIndex).height();
    int bottomPos = d->coordinate(bottom, v) + bottomHeight;
    int topPos = d->coordinate(top, v);
    
    return QRect(0, topPos, d->viewport->width(), bottomPos - topPos); // always the width of a row
}

void QGenericTreeView::scrollContentsBy(int dx, int dy)
{
    int hscroll = 0;
    int vscroll = 0;
    bool reverse = QApplication::reverseLayout();
    if (dx) {
        int value = horizontalScrollBar()->value();
        int section = d->header->section(value / d->horizontalFactor);
        int left = (value % d->horizontalFactor) * d->header->sectionSize(section);
        int offset = (left / d->horizontalFactor) + d->header->sectionPosition(section);
        hscroll = d->header->offset() - offset;
        d->header->setOffset(offset);
        //d->viewport->update();
        //return;
    } else
    if (dy) {
        /*
        int factor = d->verticalFactor;
        int value = verticalScrollBar()->value();
        int nw = value / factor;
        int old = (value + dy) / factor;
        QItemOptions options;
        getViewOptions(&options);
        int i = nw;
        if (i > old)
            while (i > old)
                vscroll += d->delegate->sizeHint(fontMetrics(), options, d->modelIndex(i--)).height();
        else
            while (i < old)
                vscroll -= d->delegate->sizeHint(fontMetrics(), options, d->modelIndex(i++)).height();
        int da = ((value % factor) * d->delegate->sizeHint(fontMetrics(), options, d->modelIndex(nw)).height())
                 -((value % factor) * d->delegate->sizeHint(fontMetrics(), options, d->modelIndex(old)).height());
        vscroll -= (da / factor);
        */
        d->viewport->update();
        return;
    }
    d->viewport->scroll(hscroll, vscroll);
}

void QGenericTreeView::contentsChanged()
{
    contentsChanged(QModelIndex(), QModelIndex());
}

void QGenericTreeView::contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // FIXME: items height may have changed: relayout ?
    QAbstractItemView::contentsChanged(topLeft, bottomRight);
}

void QGenericTreeView::contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!(topLeft.isValid() && bottomRight.isValid()))
        return;

    QModelIndex parent = model()->parent(topLeft);
    // do a local relayout of the items
    if (parent.isValid()) {
        int v = verticalScrollBar()->value();
        int pi = d->viewIndex(parent, v);
        if (d->isOpen(pi)) {
            d->close(pi);
            d->open(pi);
        }
    } else {
        updateGeometries();
        d->items.resize(0);
        d->layout_parent_index = -1;
        d->layout_from_index = -1;
        d->layout_count = model()->rowCount(root());
        startItemsLayout();
    }
}

void QGenericTreeView::contentsRemoved(const QModelIndex &parent,
                                       const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // do a local relayout of the items
    if (parent.isValid()) {
        int v = verticalScrollBar()->value();
        int pi = d->viewIndex(parent, v);
        if (d->isOpen(pi)) {
            d->close(pi);
            d->open(pi);
        }
    } else {
        int count = bottomRight.row() - topLeft.row();
        updateGeometries();
        // FIXME: this won't work if there are open branches
        collapse<QGenericTreeViewItem>(d->items, bottomRight.row() - 1, count);
        d->viewport->update();
    }
}

void QGenericTreeView::columnCountChanged(int, int)
{
    updateGeometries();
}

void QGenericTreeView::startItemsLayout()
{
    QItemOptions options;
    getViewOptions(&options);
    QModelIndex index = model()->index(0, 0, root());
    d->itemHeight = itemDelegate()->sizeHint(fontMetrics(), options, index).height();
    doItemsLayout(d->layout_count);
}

bool QGenericTreeView::doItemsLayout(int num)
{
    QModelIndex parent = d->modelIndex(d->layout_parent_index);
    QModelIndex current;

    int count = qMin(num, d->layout_count - (d->layout_from_index - d->layout_parent_index));

    if (d->layout_from_index == -1)
        d->items.resize(count);
    else
        expand<QGenericTreeViewItem>(d->items, d->layout_from_index, count);
    QGenericTreeViewItem *items = d->items.data();
    int level = d->layout_parent_index >= 0 ? items[d->layout_parent_index].level + 1 : 0;
    int first = d->layout_from_index + 1;
    for (int i = first; i < first + count; ++i) {
        current = model()->index(i - first, 0, parent);
        items[i].index = current;
//         items[i].open = false;
//         items[i].hidden = false;
//         items[i].total = 0;
        items[i].level = level;
    }
    int idx = d->layout_parent_index;
    int v = verticalScrollBar()->value();
    while (parent.isValid()) {
        items[idx].total += count;
        parent = model()->parent(parent);
        idx = d->viewIndex(parent, v); // FIXME: slow
    }
    updateGeometries();
    d->viewport->update();
    d->layout_from_index += count;
    return (d->layout_from_index >= (d->layout_parent_index + d->layout_count));
}

void QGenericTreeView::resizeColumnToContents(int column)
{
    int size = columnSizeHint(column);
    d->header->resizeSection(column, size);
}

void QGenericTreeView::columnWidthChanged(int column, int, int)
{
    bool reverse = QApplication::reverseLayout();
    int x = d->header->sectionPosition(column) + d->header->offset()
            - (reverse ? d->header->sectionSize(column) : 0);
    QRect rect(x, 0, d->viewport->width() - x, d->viewport->height());
    d->viewport->update(rect.normalize());
    updateGeometries();
    updateCurrentEditor();
}

void QGenericTreeView::updateGeometries()
{
    QSize hint = d->header->sizeHint();
    setViewportMargins(0, hint.height(), 0, 0);

    QRect vg = d->viewport->geometry();
    d->header->setGeometry(vg.left(), vg.top() - hint.height(), vg.width(), hint.height());
    if (QApplication::reverseLayout())
        d->header->setOffset(vg.right() - hint.width() + vg.left()/* + d->header->offset()*/);

    // update sliders
    QItemOptions options;
    getViewOptions(&options);
    QSize def = itemDelegate()->sizeHint(fontMetrics(), options, model()->index(0, 0, 0));

    // vertical
    int h = viewport()->height();
    int item = d->items.count();
    verticalScrollBar()->setPageStep(h / def.height() * verticalFactor());
    while (h > 0 && item > 0)
        h -= itemDelegate()->sizeHint(fontMetrics(), options, d->modelIndex(--item)).height();
    int max = item * verticalFactor();
    if (h < 0)
         max += 1 + (verticalFactor() * -h /
                     itemDelegate()->sizeHint(fontMetrics(), options, d->modelIndex(item)).height());
    verticalScrollBar()->setRange(0, max);
    
    int w = viewport()->width();
    int col = model()->columnCount(0);
    horizontalScrollBar()->setPageStep(w / def.width() * horizontalFactor());
    while (w > 0 && col > 0)
        w -= d->header->sectionSize(--col);
    max = col * horizontalFactor();
    if (w < 0)
        max += (horizontalFactor() * -w / d->header->sectionSize(col));
    horizontalScrollBar()->setRange(0, max);
}

void QGenericTreeView::verticalScrollbarAction(int action)
{
    QItemOptions options;
    getViewOptions(&options);

    int factor = d->verticalFactor;
    int value = verticalScrollBar()->value();
    int item = value / factor;
    int above = (value % factor) * d->delegate->sizeHint(fontMetrics(), options,
                                                         d->modelIndex(item)).height();
    int y = -(above / factor); // above the page

    if (action == QScrollBar::SliderPageStepAdd) {

        // go down to the bottom of the page
        int h = d->viewport->height();
        while (y < h && item < d->items.count())
            y += d->delegate->sizeHint(fontMetrics(), options, d->modelIndex(item++)).height();
        value = item * factor; // i is now the last item on the page
        if (y > h && item)
            value -= factor * (y - h) / d->delegate->sizeHint(fontMetrics(), options,
                                                              d->modelIndex(item - 1)).height();
        verticalScrollBar()->setSliderPosition(value);

    } else if (action == QScrollBar::SliderPageStepSub) {

        y += d->viewport->height();

        // go up to the top of the page
        while (y > 0 && item > 0)
            y -= d->delegate->sizeHint(fontMetrics(),
                                       options,
                                       d->modelIndex(--item)).height();
        value = item * factor; // i is now the first item in the page

        if (y < 0)
            value += factor * -y / d->delegate->sizeHint(fontMetrics(),
                                                         options,
                                                         d->modelIndex(item)).height();
        verticalScrollBar()->setSliderPosition(value);
    }
}

void QGenericTreeView::horizontalScrollbarAction(int action)
{
    // horizontal
    int factor = d->horizontalFactor;
    int value = horizontalScrollBar()->value();
    int column = value / factor;
    int above = (value % factor) * d->header->sectionSize(column); // what's left; in "item units"
    int x = -(above / factor); // left of the page

    if (action == QScrollBar::SliderPageStepAdd) {

        // go down to the right of the page
        int w = d->viewport->width();
        while (x < w && column < d->model->columnCount(0))
            x += d->header->sectionSize(column++);
        value = column * factor; // i is now the last item on the page
        if (x > w && column)
            value -= factor * (x - w) / d->header->sectionSize(column - 1);
        horizontalScrollBar()->setSliderPosition(value);

    } else if (action == QScrollBar::SliderPageStepSub) {

        x += d->viewport->width();

        // go up to the left of the page
        while (x > 0 && column > 0)
            x -= d->header->sectionSize(--column);
        value = column * factor; // i is now the first item in the page

        if (x < 0)
            value += factor * -x / d->header->sectionSize(column);
        horizontalScrollBar()->setSliderPosition(value);
    }
}

int QGenericTreeView::columnSizeHint(int column) const
{
    QItemOptions options;
    getViewOptions(&options);
    QFontMetrics fontMetrics(this->fontMetrics());
    QAbstractItemDelegate *delegate = itemDelegate();
    QModelIndex index;

    const QGenericTreeViewItem *items = d->items.constData();
    int v = verticalScrollBar()->value();
    int h = d->viewport->height();
    int c = d->items.count();
    int i = d->itemAt(v);
    int y = d->coordinateAt(v, delegate->sizeHint(fontMetrics, options, items[i].index).height());
    int w = 0;
    QSize size;

    while (y < h && i < c) {
        index = items[i].index;
        index = model()->sibling(index.row(), column, index);
        size = delegate->sizeHint(fontMetrics, options, index);
        w = qMax(w, size.width() + (column == 0 ? d->indentation(i) : 0));
        y += size.height();
        ++i;
    }

    return w;
}

bool QGenericTreeViewPrivate::isOpen(int i) const
{
    if (i < 0 || i >= items.count())
        return false;
    return items.at(i).open;
}

void QGenericTreeViewPrivate::open(int i)
{
    int c = q->model()->rowCount(modelIndex(i));
    items[i].open = true;
    if (c <= 0)
        return;
    layout_count = c;
    layout_parent_index = i;
    layout_from_index = i;
    q->startItemsLayout();
    q->updateGeometries();
}

void QGenericTreeViewPrivate::close(int i)
{
    QAbstractItemModel *model = q->model();
    int total = items.at(i).total;
    items[i].open = false;
    QModelIndex parent = modelIndex(i);
    int idx = i;
    int v = q->verticalScrollBar()->value();
    while (parent.isValid()) { // FIXME: *really slow*
        items[idx].total -= total;
        parent = model->parent(parent);
        idx = viewIndex(parent, v);
    }
    collapse<QGenericTreeViewItem>(items, i, total);
    q->updateGeometries();
    q->d->viewport->update();
}

int QGenericTreeViewPrivate::pageUp(int i) const
{
    int v = q->verticalScrollBar()->value();
    int y = coordinate(i, v) - viewport->height();
    int idx = item(y, v);
    return idx == -1 ? first() : idx;
}

int QGenericTreeViewPrivate::pageDown(int i) const
{
    int v = q->verticalScrollBar()->value();
    int y = coordinate(i, v) + viewport->height();
    int idx = item(y, v);
    return idx == -1 ? last() : idx;
}

int QGenericTreeViewPrivate::above(int i) const
{
    int idx = i;
    while (--idx >= 0 && items.at(idx).hidden);
    return idx >= 0 ? idx : i;
}

int QGenericTreeViewPrivate::below(int i) const
{
    int idx = i;
    while (++idx < items.count() && items.at(idx).hidden);
    return idx < items.count() ? idx : i;
}

int QGenericTreeViewPrivate::first() const
{
    int i = -1;
    while (++i < items.count() && items.at(i).hidden);
    return i < items.count() ? i : -1;
}

int QGenericTreeViewPrivate::last() const
{
    int i = items.count();
    while (--i >= 0 && items.at(i).hidden);
    return i >= 0 ? i : -1;
}

int QGenericTreeViewPrivate::indentation(int i) const
{
    if (i < 0 || i >= items.count())
        return 0;
    int level = items.at(i).level;
    if (rootDecoration)
        level++;
    return level * indent;
}

int QGenericTreeViewPrivate::coordinate(int item, int value) const
{
    QItemOptions options;
    q->getViewOptions(&options);
    QFontMetrics fontMetrics(q->fontMetrics());
    QAbstractItemDelegate *delegate = q->itemDelegate();
    int i = itemAt(value); // first item (may start above the page)
    int ih = delegate->sizeHint(fontMetrics, options, items.at(i).index).height();
    int y = coordinateAt(value, ih); // the part of the item above the page
    int h = q->viewport()->height();
    if (i <= item) {
        while (y < h && i < items.count()) {
            if (i == item)
                return y; // item is visible - actual y in viewport
            y += delegate->sizeHint(fontMetrics, options, items.at(i).index).height();
            ++i;
        }
        // item is below the viewport - estimated y
        return y + (itemHeight * (item - itemAt(value)));
    }
    // item is above the viewport - estimated y
    return y - (itemHeight * item);
}

int QGenericTreeViewPrivate::item(int coordinate, int value) const
{
    QItemOptions options;
    q->getViewOptions(&options);
    QFontMetrics fontMetrics(q->fontMetrics());
    QAbstractItemDelegate *delegate = q->itemDelegate();

    int i = itemAt(value);
    int y = coordinateAt(value, delegate->sizeHint(fontMetrics, options, items[i].index).height());
    int h = q->viewport()->height();
    if (coordinate >= y) {
        // search for item in viewport
        while (y < h && i < items.count()) {
            y += delegate->sizeHint(fontMetrics, options, items.at(i).index).height();
            if (coordinate < y)
                return i;
            ++i;
        }
        // item is below viewport - give estimated coordinates
    }
    // item is above the viewport - give estimated coordinates
    int idx = i + ((coordinate - y) / itemHeight);
    return idx < 0 || idx >= items.count() ? -1 : idx;
}

int QGenericTreeViewPrivate::viewIndex(const QModelIndex &index, int value) const
{
    // NOTE: this function is slow if the item is outside the visible area
    // search in visible items first, then below
    int t = itemAt(value);
    t = t > 100 ? t - 100 : 0; // start 100 items above the visible area
    for (int i = t; i < items.count(); ++i)
        if (items.at(i).index.row() == index.row() &&
            items.at(i).index.data() == index.data()) // ignore column
            return i;
    // search above
    for (int j = 0; j < t; ++j)
        if (items.at(j).index.row() == index.row() &&
            items.at(j).index.data() == index.data()) // ignore column
            return j;
    return -1;
}

QModelIndex QGenericTreeViewPrivate::modelIndex(int i) const
{
    if (i < 0 || i >= items.count())
        return QModelIndex();
    QModelIndex index = items.at(i).index;
    if (index.column() != editColumn)
        return index = q->model()->sibling(index.row(), editColumn, index);
    return items.at(i).index;
}

int QGenericTreeViewPrivate::itemAt(int value) const
{
    return value / q->verticalFactor();
}

int QGenericTreeViewPrivate::coordinateAt(int value, int iheight) const
{
    int factor = q->verticalFactor();
    int above = (value % factor) * iheight; // what's left; in "item units"
    return -(above / factor); // above the page
}

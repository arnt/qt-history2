#include "qgenerictreeview.h"
#include <qgenericheader.h>
#include <qitemdelegate.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qvector.h>
#include <qstyle.h>
#include <qevent.h>
#include <qpen.h>

#include <private/qabstractitemview_p.h>

//#include "cpu_time.h"


/*!
  \class QGenericTreeView qgenerictreeview.h

  \brief Tree view implementation

  This class implements a tree representation of a QGenericItemView working
  on a QGenericItemModel.
*/

template <typename T>
inline void expand(QVector<T> &vec, int after, size_t n)
{
    size_t m = vec.size() - after - 1;
    vec.resize(vec.size() + n);
    T *b = (T*)vec.data();
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

/*
struct Expanded {
    QModelIndex index;
    uint open : 1;
    uint total : 31;
    QVector<Expanded> expanded;
};
*/
struct QGenericTreeViewItem { // 20 bytes
    QGenericTreeViewItem() : open(false), hidden(false), total(0), level(0) {}
    QModelIndex index;
    uint open : 1;
    uint hidden : 1;
    uint total : 30; // total number of children visible
    uint level : 16; // indentation
};

/*!
  \class QGenericTreeViewItem qgenerictreeview.cpp

  This class implements a QViewItem working on a QGenericTreeView.
*/

class QGenericTreeViewPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QGenericTreeView);
public:

    QGenericTreeViewPrivate()
        : QAbstractItemViewPrivate(), header(0), indent(20), itemHeight(-1) { }

    ~QGenericTreeViewPrivate() {}

    bool isOpen(int i) const;
    void open(int i);
    void close(int i);

    int pageUp(int i) const;
    int pageDown(int i) const;
    int above(int i) const;
    int below(int i) const;
    int first() const;
    int last() const;

    int indentation(int i) const;
    int coordinate(int i) const;
    int viewIndex(int y) const;
    int viewIndex(const QModelIndex &index) const;
    QModelIndex modelIndex(int i) const;

    QGenericHeader *header;
    int indent;

    //Expanded expanded;
    QVector<QGenericTreeViewItem> items;
    int itemHeight; // this is just a number; contentsHeight() / numItems

    int layout_parent_index;
    int layout_from_index;
    int layout_count;

    // used for drawing
    int from;
    int to;
    int current;
};

#define d d_func()
#define q q_func()


QGenericTreeView::QGenericTreeView(QGenericItemModel *model, QWidget *parent, const char *name)
    : QAbstractItemView(*new QGenericTreeViewPrivate, model, parent, name)
{
    setHeader(new QGenericHeader(model, Horizontal, this, "treeview_header"));
    d->header->setMovable(true);
    d->layout_parent_index = -1;
    d->layout_from_index = -1;
    d->layout_count = model->rowCount(root());
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
	QObject::disconnect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
			    d->header, SLOT(setOffset(int)));
	QObject::disconnect(d->header, SIGNAL(sectionSizeChanged(int, int, int)),
			    this, SLOT(columnWidthChanged(int, int, int)));
	QObject::disconnect(d->header, SIGNAL(sectionIndexChanged(int, int, int)),
			    this, SLOT(contentsChanged()));
	QObject::disconnect(d->header, SIGNAL(sectionCountChanged(int, int)),
			    this, SLOT(columnCountChanged(int, int)));
	delete d->header; // FIXME ???
    }
    // FIXME: reparent header ???
    d->header = header;
    setMargins(0, d->header->sizeHint().height(), 0, 0);
    QObject::connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
		     d->header, SLOT(setOffset(int)));
    QObject::connect(d->header, SIGNAL(sectionSizeChanged(int, int, int)),
		     this, SLOT(columnWidthChanged(int, int, int)));
    QObject::connect(d->header, SIGNAL(sectionIndexChanged(int, int, int)),
		     this, SLOT(contentsChanged()));
    QObject::connect(d->header, SIGNAL(sectionCountChanged(int, int)),
    		     this, SLOT(columnCountChanged(int, int)));
    d->header->setSelectionModel(selectionModel());
    QSize size = d->header->sizeHint();
    resizeContents(size.width(), 0);
}

int QGenericTreeView::indentation() const
{
    return d->indent;
}

void QGenericTreeView::setIndentation(int i)
{
    d->indent = i;
}

bool QGenericTreeView::isColumnHidden(int column) const
{
    return d->header->isSectionHidden(column);
}

void QGenericTreeView::hideColumn(int column)
{
    d->header->hideSection(column);
}

void QGenericTreeView::drawContents(QPainter *painter, int cx, int cy, int cw, int ch)
{
    int from = d->header->sectionAt(cx);
    int to = d->header->sectionAt(cx + cw);
    d->from = 0;//from > -1 ? from : 0;
    d->to = d->header->count();//to > -1 ? to + 1 : d->header->count();

    if (d->items.isEmpty())
	return;

    QGenericTreeViewItem *items = d->items.data();
    QItemOptions options;
    getViewOptions(&options);
    QFontMetrics fontMetrics(this->fontMetrics());
    QItemDelegate *delegate = itemDelegate();

    int view_index = d->viewIndex(cy);
    QModelIndex model_index = d->modelIndex(view_index);
    static QPoint pt(0, 0);
    options.itemRect = itemRect(model_index);
    options.itemRect.moveTopLeft(pt);
    int height = options.itemRect.height();
    int bottom = cy + ch;
    int x = d->indentation(view_index);
    int y = d->coordinate(view_index);
    while (y < bottom && model_index.isValid()) {
	d->current = view_index;
        // draw row
	//painter->translate(0, y);
	options.itemRect.moveTop(y);
	drawRow(painter, &options, d->modelIndex(view_index));
	//painter->translate(0, -y);
	// next row
	++view_index;
	model_index = d->modelIndex(view_index);
	x = d->indentation(view_index);
	height = delegate->sizeHint(fontMetrics, options, model_index).height();
	y += height;
	options.itemRect.setHeight(height);
    }
}

void QGenericTreeView::drawRow(QPainter *painter, QItemOptions *options, const QModelIndex &index) const
{
    // FIXME: clean this up!
    int pos;
    int column = d->from;
    int width, height = options->itemRect.height();
    QColor base = options->palette.base();

    int x = d->indentation(d->current);
    int y = options->itemRect.y();
    QModelIndex parent = model()->parent(index);
    QGenericHeader *header = d->header;
    
    if (column == 0 && !header->isSectionHidden(column)) {
	pos = header->sectionPosition(column);
	width = header->sectionSize(column);
	options->selected = selectionModel()->isSelected(index);
	options->itemRect.moveLeft(x + pos);
	options->itemRect.setWidth(width - x);
	options->focus = (q->viewport()->hasFocus() && selectionModel()->currentItem() == index);
	painter->fillRect(pos, y, width - pos, height, base);
	drawBranches(painter, QRect(pos, y, d->indent, options->itemRect.height()), index);
	itemDelegate()->paint(painter, *options, index);
	++column;
    }
    
    options->focus = false;
    QModelIndex i = index;
    for (; column < model()->columnCount(root()); ++column) {
	if (header->isSectionHidden(column))
	    continue;
	pos = header->sectionPosition(column);
	i = model()->index(i.row(), column, parent);
	width = header->sectionSize(column);
	options->itemRect.setRect(pos, y, width, height);
	options->selected = selectionModel()->isSelected(i);
	painter->fillRect(pos, y, width, height, base);
	itemDelegate()->paint(painter, *options, i);
    }
}

void QGenericTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QModelIndex parent = model()->parent(index);
    QModelIndex current = parent;
    QModelIndex ancestor = model()->parent(current);
    int x = d->indentation(d->current);
    int indent = d->indent;
    painter->translate(x - indent * 2, 0);
    for (int level = d->items.at(d->current).level - 1; level >= 0; --level) {
	style().drawPrimitive(QStyle::PE_TreeBranch, painter, rect, palette(),
			      model()->rowCount(ancestor) - 1 > current.row() ? QStyle::Style_Sibling : 0);
	current = ancestor;
	ancestor = model()->parent(current);
	painter->translate(-indent, 0);
    }
    painter->translate(x, 0);
    QStyle::SFlags flags = QStyle::Style_Item |
			   (model()->rowCount(parent) - 1 > index.row() ? QStyle::Style_Sibling : 0) |
			   (model()->hasChildren(index) ? QStyle::Style_Children : 0) |
			   (d->isOpen(d->current) ? QStyle::Style_Open : 0);
    style().drawPrimitive(QStyle::PE_TreeBranch, painter, rect, palette(), flags);
    painter->translate(d->indent - x, 0);
}

void QGenericTreeView::contentsMousePressEvent(QMouseEvent *e)
{
    int column = d->header->sectionAt(e->x());
    int position = d->header->sectionPosition(column);
    int cx = e->x() - position;
    int vi = d->viewIndex(e->y());
    QModelIndex mi = d->modelIndex(vi);

    if (mi.isValid()) {
	int indent = d->indentation(vi);
   	if (column == 0 && cx < (indent - d->indent))
   	    return; // we are in the empty area in front of the tree - do nothing
	if (column > 0 || cx > indent) {
	    QAbstractItemView::contentsMousePressEvent(e);
	    return; // we are on an item - select it
	}
	if (d->isOpen(vi))
	    d->close(vi);
	else
	    d->open(vi);
    }
}

QModelIndex QGenericTreeView::itemAt(int x, int y) const
{
    int vi = d->viewIndex(y);
    QModelIndex mi = d->modelIndex(vi);
    int column = d->header->sectionAt(x);
    QModelIndex parent = model()->parent(mi);
    return model()->index(mi.row(), column, parent);
}

QRect QGenericTreeView::itemRect(const QModelIndex &item) const
{
    if (!item.isValid())
	return QRect();

    QGenericHeader *header = d->header;
    int x = header->sectionPosition(item.column());
    int w = header->sectionSize(item.column());
    int vi = d->viewIndex(item); // FIXME: slow
    if (item.column() == 0) {
	int i = d->indentation(vi);
	x += i;
	w -= i;
    }
    QItemOptions options;
    getViewOptions(&options);
    QModelIndex parent = model()->parent(item);
    QModelIndex sibling = model()->index(item.row(), 0, parent); // FIXME: find the tallest item in this row
    int h = itemDelegate()->sizeHint(fontMetrics(), options, sibling).height(); // FIXME: *really slow*
    return QRect(x, d->coordinate(vi), w, h);
}

QModelIndex QGenericTreeView::moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState)
{
    QModelIndex current = currentItem();
    int vi = d->viewIndex(current); // FIXME: slow

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

QItemSelectionModel::SelectionBehavior QGenericTreeView::selectionBehavior() const
{
    return QItemSelectionModel::SelectRows;
}

void QGenericTreeView::setSelection(const QRect &rect, QItemSelectionModel::SelectionUpdateMode mode)
{
    // FIXME: select rows
    QModelIndex tl = itemAt(rect.left(), rect.top());
    QModelIndex br = itemAt(rect.right(), rect.bottom());
    if (!tl.isValid() || !br.isValid())
	return;

//     if (tl == d->topLeft && br == d->bottomRight)
// 	return;
//     d->topLeft = tl;
//     d->bottomRight = br;

    selectionModel()->select(new QItemSelection(tl, br, model()), mode, selectionBehavior());
}

QRect QGenericTreeView::selectionRect(const QItemSelection *selection) const
{
    int section = d->header->count() - 1;
    int leftPos = d->header->sectionPosition(section);
    int rightPos = 0;
    int topPos = contentsHeight();
    int bottomPos = 0;
    for (int i = 0; i < selection->ranges.count(); ++i) {
	QItemSelectionRange r = selection->ranges.at(i);
	leftPos = qMin(d->header->sectionPosition(r.left()), leftPos);
	rightPos = qMax(d->header->sectionPosition(r.right()) + d->header->sectionSize(r.right()), rightPos);
	topPos = qMin(itemRect(model()->index(r.top(), r.left(), r.parent())).top(), topPos);
	bottomPos = qMax(itemRect(model()->index(r.bottom(), r.left(), r.parent())).bottom() + 1, bottomPos);
    }
    return QRect(leftPos, topPos, rightPos - leftPos, bottomPos - topPos);
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
	int pi = d->viewIndex(parent);
	if (d->isOpen(pi)) {
	    d->close(pi);
	    d->open(pi);
	}
    } else {
#if 0
	qDebug("contentsInserted top %d bottom %d ", topLeft.row(), bottomRight.row());
	// FIXME: this won't work if there are open branches
	int count = bottomRight.row() - topLeft.row() + 1;
	qDebug("count %d", count);
	expand<QGenericTreeViewItem>(d->items, topLeft.row() - 1, count);
	QGenericTreeViewItem *items = d->items.data();
	for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
	    items[i].index = model()->index(i, 0, 0);
	}
	resizeContents(contentsHeight() + d->itemHeight * count, contentsWidth());
	updateContents();
#else
	resizeContents(contentsWidth(), 0);
	d->items.resize(0);
	d->layout_parent_index = -1;
	d->layout_from_index = -1;
	d->layout_count = model()->rowCount(root());
	startItemsLayout();
#endif
    }
}

void QGenericTreeView::contentsRemoved(const QModelIndex &parent,
				       const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // do a local relayout of the items
    if (parent.isValid()) {
	int pi = d->viewIndex(parent);
	if (d->isOpen(pi)) {
	    d->close(pi);
	    d->open(pi);
	}
    } else {
	qDebug("contentsRemoved");
	int count = bottomRight.row() - topLeft.row();
	//resizeContents(contentsHeight() - d->itemHeight * count, contentsWidth());
	// FIXME: this won't work if there are open branches
	collapse<QGenericTreeViewItem>(d->items, bottomRight.row() - 1, count);
	updateContents();
    }
}

void QGenericTreeView::columnCountChanged(int, int)
{
    resizeContents(d->header->sizeHint().width(), contentsHeight());
}

void QGenericTreeView::startItemsLayout()
{
    QItemOptions options;
    getViewOptions(&options);
    QModelIndex index = model()->index(0, 0, root());
    d->itemHeight = itemDelegate()->sizeHint(fontMetrics(), options, index).height();
//    verticalScrollBar()->setLineStep(d->itemHeight);
    doItemsLayout(d->layout_count);
}

bool QGenericTreeView::doItemsLayout(int num)
{
    QModelIndex parent = d->modelIndex(d->layout_parent_index);
    QModelIndex current;

    int count = qMin(num, d->layout_count - (d->layout_from_index - d->layout_parent_index));

    // FIXME: problem here!!
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
// 	items[i].open = false;
// 	items[i].hidden = false;
// 	items[i].total = 0;
	items[i].level = level;
    }
    int idx = d->layout_parent_index;
    while (parent.isValid()) {
	items[idx].total += count;
	parent = model()->parent(parent);
	idx = d->viewIndex(parent); // FIXME: slow
    }
    int height = contentsHeight() + d->itemHeight * count;
    resizeContents(contentsWidth(), height);
    updateContents();
    d->layout_from_index += count;
    return (d->layout_from_index >= (d->layout_parent_index + d->layout_count));
}

void QGenericTreeView::columnWidthChanged(int column, int oldSize, int newSize)
{
    int columnPos = d->header->sectionPosition(column);
    updateContents(columnPos, contentsY(),
		   visibleWidth() - columnPos + contentsX(), visibleHeight());
    resizeContents(contentsWidth() + (newSize - oldSize), contentsHeight());
    updateCurrentEditor();
}

void QGenericTreeView::updateGeometries()
{
    int margin = QApplication::reverseLayout() ? rightMargin() : leftMargin();
    QRect r(margin + frameWidth(), frameWidth(), visibleWidth(), topMargin());
    d->header->setGeometry(QStyle::visualRect(r, rect()));
    QSize hint = d->header->sizeHint();
    setMargins(0, hint.height(), 0, 0);
    resizeContents(hint.width(), contentsHeight());
    horizontalScrollBar()->raise();
    verticalScrollBar()->raise();
}

bool QGenericTreeViewPrivate::isOpen(int i) const
{
    if (i < 0 || i >= items.count())
	return false;
    return items.at(i).open;
}

void QGenericTreeViewPrivate::open(int i)
{
    items[i].open = true;
    layout_parent_index = i;
    layout_from_index = i;
    layout_count = q->model()->rowCount(modelIndex(i));
    q->startItemsLayout();
}

void QGenericTreeViewPrivate::close(int i)
{
    QGenericItemModel *model = q->model();
    int total = items.at(i).total;
    items[i].open = false;
    QModelIndex parent = modelIndex(i);
    int idx = i;
    while (parent.isValid()) { // FIXME: *really slow*
	items[idx].total -= total;
	parent = model->parent(parent);
	idx = viewIndex(parent); // FIXME: slow
    }
    collapse<QGenericTreeViewItem>(items, i, total);
    int height = total * itemHeight;
    q->resizeContents(q->contentsWidth(), q->contentsHeight() - height);
    q->updateContents();
}

int QGenericTreeViewPrivate::pageUp(int i) const
{
    int y = coordinate(i) - q->visibleHeight();
    int idx = viewIndex(y);
    return idx == -1 ? first() : idx;
}

int QGenericTreeViewPrivate::pageDown(int i) const
{
    int y = coordinate(i) + q->visibleHeight();
    int idx = viewIndex(y);
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
    return (items.at(i).level + 1) * indent;
}

int QGenericTreeViewPrivate::coordinate(int i) const
{
    return itemHeight * i;
    // FIXME: check if item is visible, if it is, get a coordinate on screen
}

int QGenericTreeViewPrivate::viewIndex(int y) const
{
#if 0
    int visible_top = q->contentsY();
    int visible_bottom = visible_top + q->visibleHeight();
    if (y > visible_top && y < visible_bottom) { // y is in the visible area
	QItemOptions options;
	q->getViewOptions(&options);
	QFontMetrics fontMetrics(q->fontMetrics());
	QItemDelegate *delegate = q->itemDelegate();
 	int item_y = visible_top;
	int item_index = visible_top / itemHeight;
 	while (item_y < y && item_index < items.count() - 1)
	    item_y += delegate->sizeHint(fontMetrics, options, modelIndex(++item_index)).height();
 	return item_index;
    }
#endif
    int item_index = y / itemHeight;
    return item_index >= items.count() ? -1 : item_index;
}

int QGenericTreeViewPrivate::viewIndex(const QModelIndex &index) const
{
    // FIXME: use map or something similar ?
    for (int i = 0; i < items.count(); ++i)
	if (items.at(i).index.row() == index.row() &&
	    items.at(i).index.data() == index.data()) // ignore column
	    return i;
    return -1;
}

QModelIndex QGenericTreeViewPrivate::modelIndex(int i) const
{
    if (i < 0 || i >= items.count())
	return QModelIndex();
    return items.at(i).index;
}

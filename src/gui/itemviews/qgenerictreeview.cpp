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

#include <private/qabstractitemview_p.h>

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

    bool isOpen(int item) const;
    void open(int item);
    void close(int item);

    int pageUp(int item) const;
    int pageDown(int item) const;
    int above(int item) const;
    int below(int item) const;
    int first() const;
    int last() const;

    int indentation(int item) const;
    int coordinate(int item, int value) const;
    int item(int coordinate, int value) const;

    int viewIndex(const QModelIndex &index, int value) const;
    QModelIndex modelIndex(int i) const;

    int itemAt(int value) const;
    int coordinateAt(int value, int iheight) const;

    QGenericHeader *header;
    int indent;

    //Expanded expanded;
    QVector<QGenericTreeViewItem> items;
    int itemHeight; // this is just a number; contentsHeight() / numItems

    int layout_parent_index;
    int layout_from_index;
    int layout_count;

    // used for drawing
    int left;
    int right;
    int current;
};

#define d d_func()
#define q q_func()

QGenericTreeView::QGenericTreeView(QGenericItemModel *model, QWidget *parent)
    : QAbstractItemView(*new QGenericTreeViewPrivate, model, parent)
{
    setHeader(new QGenericHeader(model, Horizontal, this));
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
	QObject::disconnect(d->header, SIGNAL(sectionSizeChanged(int,int,int)),
			    this, SLOT(columnWidthChanged(int,int,int)));
	QObject::disconnect(d->header, SIGNAL(sectionIndexChanged(int,int,int)),
			    this, SLOT(contentsChanged()));
	QObject::disconnect(d->header, SIGNAL(sectionCountChanged(int,int)),
			    this, SLOT(columnCountChanged(int,int)));
	delete d->header; // FIXME ???
    }
    // FIXME: reparent header ???
    d->header = header;
    setViewportMargins(0, d->header->sizeHint().height(), 0, 0);
    QObject::connect(d->header, SIGNAL(sectionSizeChanged(int,int,int)),
		     this, SLOT(columnWidthChanged(int,int,int)));
    QObject::connect(d->header, SIGNAL(sectionIndexChanged(int,int,int)),
		     this, SLOT(contentsChanged()));
    QObject::connect(d->header, SIGNAL(sectionCountChanged(int,int)),
    		     this, SLOT(columnCountChanged(int,int)));
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

int QGenericTreeView::columnViewportPosition(int column) const
{
    return d->header->sectionPosition(column) - d->header->offset();
}

int QGenericTreeView::columnWidth(int column) const
{
    return d->header->sectionSize(column);
}

int QGenericTreeView::columnAt(int x) const
{
    return d->header->sectionAt(x + d->header->offset());
}

bool QGenericTreeView::isColumnHidden(int column) const
{
    return d->header->isSectionHidden(column);
}

void QGenericTreeView::hideColumn(int column)
{
    d->header->hideSection(column);
}

int QGenericTreeView::contentsX() const
{
    return d->header->offset();
}

int QGenericTreeView::contentsY() const
{
    return -1; // FIXME: invalid
}

int QGenericTreeView::contentsWidth() const
{
    return d->header->size();
}

int QGenericTreeView::contentsHeight() const
{
    return -1; // FIXME: invalid
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
//     static int fr = 0;
//     qDebug("paint %d", fr++);
    
//    QRect area = e->rect();
    QPainter painter(viewport());

    d->left = 0;//qMax(d->header->sectionAt(contentsX()), 0);
    d->right = -1;//d->header->sectionAt(contentsX() + visibleWidth());
    if (d->right < 0)
	d->right = d->header->count() - 1;

    if (d->items.isEmpty())
	return;

    QGenericTreeViewItem *items = d->items.data();

    QItemOptions options;
    getViewOptions(&options);
    QFontMetrics fontMetrics(this->fontMetrics());
    QAbstractItemDelegate *delegate = itemDelegate();
    QModelIndex index;
    QModelIndex current = selectionModel()->currentItem();

    int h = viewport()->height();
    int v = verticalScrollBar()->value();
    int c = d->items.count();
    int i = d->itemAt(v);
    int y = d->coordinateAt(v, delegate->sizeHint(fontMetrics, options, items[i].index).height());
    int x = -d->header->offset();
    painter.translate(x, 0);
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
    painter.translate(-x, 0);
}

void QGenericTreeView::drawRow(QPainter *painter, QItemOptions *options, const QModelIndex &index) const
{
    // FIXME: clean this up!
    int pos;
    int column = d->left;
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
    for (; column <= d->right; ++column) {
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

void QGenericTreeView::mousePressEvent(QMouseEvent *e)
{
    int column = d->header->sectionAt(e->x());
    int position = d->header->sectionPosition(column);
    int cx = e->x() - position;
    int vi = d->item(e->y(), verticalScrollBar()->value());
    QModelIndex mi = d->modelIndex(vi);

    if (mi.isValid()) {
	int indent = d->indentation(vi) - d->header->offset();
   	if (column == 0 && cx < (indent - d->indent))
   	    return; // we are in the empty area in front of the tree - do nothing
	if (column > 0 || cx > indent) {
	    QAbstractItemView::mousePressEvent(e);
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
    int vi = d->item(y, verticalScrollBar()->value());
    QModelIndex mi = d->modelIndex(vi);
    int column = d->header->sectionAt(x);
    QModelIndex parent = model()->parent(mi);
    return model()->index(mi.row(), column, parent);
}

QRect QGenericTreeView::itemViewportRect(const QModelIndex &item) const
{
    if (!item.isValid())
	return QRect();

//     QGenericHeader *header = d->header;
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
    selectionModel()->select(QItemSelection(tl, br, model()), mode, selectionBehavior());
}

QRect QGenericTreeView::selectionRect(const QItemSelection &selection) const
{
    int section = d->header->count() - 1;
    int leftPos = d->header->sectionPosition(section);
    int rightPos = 0;
    int topPos = contentsHeight();
    int bottomPos = 0;
    for (int i = 0; i < selection.count(); ++i) {
	QItemSelectionRange r = selection.at(i);
	leftPos = qMin(columnViewportPosition(r.left()), leftPos);
	rightPos = qMax(columnViewportPosition(r.right()) + columnWidth(r.right()), rightPos);
	topPos = qMin(itemViewportRect(model()->index(r.top(), r.left(), r.parent())).top(), topPos);
	bottomPos = qMax(itemViewportRect(model()->index(r.bottom(), r.left(), r.parent())).bottom() + 1, bottomPos);
    }
    return QRect(leftPos, topPos, rightPos - leftPos, bottomPos - topPos);
}

void QGenericTreeView::scrollContentsBy(int dx, int dy)
{
    if (dx) {
	int value = horizontalScrollBar()->value();
        int column = value / horizontalFactor();
	int left = (value % horizontalFactor()) * columnWidth(column);
	d->header->setOffset((left / horizontalFactor()) + d->header->sectionPosition(column));
    }
    QViewport::scrollContentsBy(dx, dy);
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
//	resizeContents(contentsWidth(), 0);
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
	qDebug("contentsRemoved");
	int count = bottomRight.row() - topLeft.row();
	//resizeContents(contentsHeight() - d->itemHeight * count, contentsWidth());
	updateGeometries();
	// FIXME: this won't work if there are open branches
	collapse<QGenericTreeViewItem>(d->items, bottomRight.row() - 1, count);
	d->viewport->update();
    }
}

void QGenericTreeView::columnCountChanged(int, int)
{
//    resizeContents(d->header->sizeHint().width(), contentsHeight());
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
    int v = verticalScrollBar()->value();
    while (parent.isValid()) {
	items[idx].total += count;
	parent = model()->parent(parent);
	idx = d->viewIndex(parent, v); // FIXME: slow
    }
//     int height = contentsHeight() + d->itemHeight * count;
//    resizeContents(contentsWidth(), height);
    updateGeometries();
    d->viewport->update();
    d->layout_from_index += count;
    return (d->layout_from_index >= (d->layout_parent_index + d->layout_count));
}

void QGenericTreeView::columnWidthChanged(int column, int, int)
{
    int columnPos = d->header->sectionPosition(column);
    d->viewport->update(QRect(columnPos, contentsY(),
 		   visibleWidth() - columnPos + contentsX(), visibleHeight()));
//     resizeContents(contentsWidth() + (newSize - oldSize), contentsHeight());
    updateGeometries();
    updateCurrentEditor();
}

void QGenericTreeView::updateGeometries()
{
    QSize hint = d->header->sizeHint();
    setViewportMargins(0, hint.height(), 0, 0);

    bool reverse = QApplication::reverseLayout();
    QRect vg = viewport()->geometry();
    d->header->setGeometry(reverse ? vg.right() - hint.width() : vg.left(),
			   vg.top() - hint.height() - 1, vg.width(), hint.height());

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

    // horizontal
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
    int above = (value % factor) * d->delegate->sizeHint(fontMetrics(),
							 options,
							 d->modelIndex(item)).height();
    int y = -(above / factor); // above the page
	
    if (action == QScrollBar::SliderPageStepAdd) {
	    
	// go down to the bottom of the page
	int h = d->viewport->height();
	while (y < h && item < d->items.count())
	    y += d->delegate->sizeHint(fontMetrics(), options, d->modelIndex(item++)).height();
	value = item * factor; // i is now the last item on the page
	if (y > h && item)
	    value -= factor * (y - h) / d->delegate->sizeHint(fontMetrics(),
							      options,
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
    int factor = d->horizontalFactor;
    int value = horizontalScrollBar()->value();
    int column = value / factor;
    int above = (value % factor) * d->header->sectionSize(column); // what's left; in "item units"
    int x = -(above / factor); // above the page
	
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
    q->updateGeometries();
}

void QGenericTreeViewPrivate::close(int i)
{
    QGenericItemModel *model = q->model();
    int total = items.at(i).total;
    items[i].open = false;
    QModelIndex parent = modelIndex(i);
    int idx = i;
    int v = q->verticalScrollBar()->value();
    while (parent.isValid()) { // FIXME: *really slow*
	items[idx].total -= total;
	parent = model->parent(parent);
	idx = viewIndex(parent, v); // FIXME: slow
    }
    collapse<QGenericTreeViewItem>(items, i, total);
//     int height = total * itemHeight;
//    q->resizeContents(q->contentsWidth(), q->contentsHeight() - height);
    q->updateGeometries();
    q->d->viewport->update();
}

int QGenericTreeViewPrivate::pageUp(int i) const
{
    int v = q->verticalScrollBar()->value();
    int y = coordinate(i, v) - q->visibleHeight();
    int idx = item(y, v);
    return idx == -1 ? first() : idx;
}

int QGenericTreeViewPrivate::pageDown(int i) const
{
    int v = q->verticalScrollBar()->value();
    int y = coordinate(i, v) + q->visibleHeight();
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
    return (items.at(i).level + 1) * indent;// - header->offset();
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
    t = t > 100 ? t - 100 : 0; // start a few items above the visible area
    for (int i = t; i < items.count(); ++i) {
	if (items.at(i).index.row() == index.row() &&
	    items.at(i).index.data() == index.data()) // ignore column
	    return i;
    }
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
    return items.at(i).index;
}

int QGenericTreeViewPrivate::itemAt(int value) const
{
    return value / q->verticalFactor();
}

int QGenericTreeViewPrivate::coordinateAt(int value, int iheight) const
{
    int factor = q->verticalFactor();
//     int item = value / factor;
    int above = (value % factor) * iheight; // what's left; in "item units"
    return -(above / factor); // above the page
}

#include "qgenerictableview.h"
#include "qgenericheader.h"
#include <qitemdelegate.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qsize.h>
#include <qscrollbar.h>
#include <private/qobject_p.h>

#include <private/qabstractitemview_p.h>

/*!
  \class QGenericTableView qgenerictableview.h

  \brief Table view implementation

  This class implements a table representation of a QGenericItemView working
  on a QGenericItemModel.
*/

class QGenericTableViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QGenericTableView);
public:
    QGenericTableViewPrivate()
	: showGrid(true), topHeader(0), leftHeader(0) {}

    bool showGrid;
    QGenericHeader *topHeader, *leftHeader;
    QModelIndex topLeft, bottomRight; // Used for optimization in setSelection
};

#define d d_func()
#define q q_func()

QGenericTableView::QGenericTableView(QGenericItemModel *model, QWidget *parent)
    : QAbstractItemView(*new QGenericTableViewPrivate, model, parent)
{
    setLeftHeader(new QGenericHeader(model, Vertical, this));
    d->leftHeader->setClickable(true);
    setTopHeader(new QGenericHeader(model, Horizontal, this));
    d->topHeader->setClickable(true);
    //model->fetchMore(); // FIXME: can we move this to qabstractitemview?
}

QGenericTableView::~QGenericTableView()
{
}

QGenericHeader *QGenericTableView::topHeader() const
{
    return d->topHeader;
}

QGenericHeader *QGenericTableView::leftHeader() const
{
    return d->leftHeader;
}

void QGenericTableView::setTopHeader(QGenericHeader *header)
{
    if (d->topHeader) {
	QObject::disconnect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
			    this, SLOT(setHorizontalOffset(int)));
	QObject::disconnect(d->topHeader,SIGNAL(sectionSizeChanged(int,int,int)),
			    this, SLOT(columnWidthChanged(int,int,int)));
	QObject::disconnect(d->topHeader, SIGNAL(sectionIndexChanged(int,int,int)),
			    this, SLOT(columnIndexChanged(int,int,int)));
	QObject::disconnect(d->topHeader, SIGNAL(sectionClicked(int, ButtonState)),
			    this, SLOT(selectColumn(int, ButtonState)));
	QObject::disconnect(d->topHeader, SIGNAL(sectionCountChanged(int, int)),
			    this, SLOT(columnCountChanged(int, int)));
    }

    d->topHeader = header;

    QObject::connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
		     this, SLOT(setHorizontalOffset(int)));
    QObject::connect(d->topHeader,SIGNAL(sectionSizeChanged(int,int,int)),
		     this, SLOT(columnWidthChanged(int,int,int)));
    QObject::connect(d->topHeader, SIGNAL(sectionIndexChanged(int,int,int)),
		     this, SLOT(columnIndexChanged(int,int,int)));
    QObject::connect(d->topHeader, SIGNAL(sectionClicked(int, ButtonState)),
		     this, SLOT(selectColumn(int, ButtonState)));
    QObject::connect(d->topHeader, SIGNAL(sectionCountChanged(int, int)),
		     this, SLOT(columnCountChanged(int, int)));

    // FIXME: this needs to be set in setSelectionModel too
    d->topHeader->setSelectionModel(selectionModel());
    //columnCountChanged(0, d->topHeader->count()); // FIXME
}

void QGenericTableView::setLeftHeader(QGenericHeader *header)
{
    if (d->leftHeader) {
	disconnect(verticalScrollBar(), SIGNAL(valueChanged(int)),
		   this, SLOT(setVerticalOffset(int)));
	disconnect(d->leftHeader, SIGNAL(sectionSizeChanged(int,int,int)),
		   this, SLOT(rowHeightChanged(int,int,int)));
	disconnect(d->leftHeader, SIGNAL(sectionIndexChanged(int,int,int)),
		   this, SLOT(rowIndexChanged(int,int,int)));
	disconnect(d->leftHeader, SIGNAL(sectionClicked(int, ButtonState)),
		   this, SLOT(selectRow(int,ButtonState)));
	disconnect(d->leftHeader, SIGNAL(sectionCountChanged(int, int)),
		   this, SLOT(rowCountChanged(int, int)));
    }

    d->leftHeader = header;

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
	    this, SLOT(setVerticalOffset(int)));
    connect(d->leftHeader, SIGNAL(sectionSizeChanged(int,int,int)),
	    this, SLOT(rowHeightChanged(int,int,int)));
    connect(d->leftHeader, SIGNAL(sectionIndexChanged(int,int,int)),
	    this, SLOT(rowIndexChanged(int,int,int)));
    connect(d->leftHeader, SIGNAL(sectionClicked(int, ButtonState)),
	    this, SLOT(selectRow(int,ButtonState)));
    connect(d->leftHeader, SIGNAL(sectionCountChanged(int, int)),
	    this, SLOT(rowCountChanged(int, int)));

    // FIXME: this needs to be set in setSelectionModel too
    d->leftHeader->setSelectionModel(selectionModel());
    //rowCountChanged(0, d->leftHeader->count());
}

void QGenericTableView::drawGrid(QPainter *p, int x, int y, int w, int h) const
{
    QPen old = p->pen();
    p->setPen(lightGray);
    p->drawLine(x, y + h, x + w, y + h);
    p->drawLine(x + w, y, x + w, y + h);
    p->setPen(old);
}

void QGenericTableView::paintEvent(QPaintEvent *e)
{
    QPainter painter(viewport());
    
    int colfirst = columnAt(0);
    int collast = columnAt(viewport()->width());
    int rowfirst = rowAt(0);
    int rowlast = rowAt(viewport()->height());
    bool showGrid = d->showGrid;

    QModelIndex bottomRight = model()->bottomRight(root());

    if (rowfirst == -1 || colfirst == -1)
	return;
    if (rowlast == -1)
	rowlast = bottomRight.row();
    if (collast == -1)
	collast = bottomRight.column();

    QItemOptions options;
    getViewOptions(&options);
    QItemSelectionModel *sels = selectionModel();
    QGenericHeader *leftHeader = d->leftHeader;
    QGenericHeader *topHeader = d->topHeader;
    QModelIndex cvi = currentItem();

    for (int r = rowfirst; r <= rowlast; ++r) {
	if (leftHeader->isSectionHidden(r))
	    continue;
	int rowp = rowViewportPosition(r);
	int rowh = rowHeight(r);
	for (int c = colfirst; c <= collast; ++c) {
	    if (topHeader->isSectionHidden(c))
		continue;
	    int colp = columnViewportPosition(c);
	    int colw = columnWidth(c);
	    QModelIndex item = model()->index(r, c, root());
	    if (item.isValid()) {
		options.itemRect = QRect(colp, rowp, colw - 1, rowh - 1);
		options.selected = sels ? sels->isSelected(item) : 0;
		options.focus = (viewport()->hasFocus() && item == cvi);
		painter.fillRect(colp, rowp, colw, rowh,
				 (options.selected ? options.palette.highlight() :
				  options.palette.base()));
		itemDelegate()->paint(&painter, options, item);
	    }
	    if (showGrid)
		drawGrid(&painter, colp, rowp, colw - 1, rowh - 1);
	}
    }
}

QModelIndex QGenericTableView::itemAt(int x, int y) const
{
    return model()->index(rowAt(y), columnAt(x), root());
}

QRect QGenericTableView::itemViewportRect(const QModelIndex &item) const
{
    if (!item.isValid() || model()->parent(item) != root())
	return QRect();
    return QRect(columnViewportPosition(item.column()), rowViewportPosition(item.row()),
		 columnWidth(item.column()), rowHeight(item.row()));
}

void QGenericTableView::ensureItemVisible(const QModelIndex &item)
{
    QRect area = viewport()->geometry();
    QRect rect = itemViewportRect(item);

    if (area.contains(rect) || model()->parent(item) != root())
	return;
    
    // vertical
    if (rect.top() < area.top()) { // above
	verticalScrollBar()->setValue(item.row() * verticalFactor());
    } else if (rect.bottom() > area.bottom()) { // below
	int r = item.row();
 	int y = area.height();
 	while (y > 0 && r > 0)
 	    y -= rowHeight(r--);
	int a = (-y * verticalFactor()) / rowHeight(r);
	verticalScrollBar()->setValue(++r * verticalFactor() + a);
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

QModelIndex QGenericTableView::moveCursor(QAbstractItemView::CursorAction cursorAction,
					  ButtonState /*state*/)
{
    QModelIndex current = currentItem();
    QModelIndex bottomRight = model()->bottomRight(root());
    switch (cursorAction) {
        case QAbstractItemView::MoveUp:
	    return model()->index(current.row() - 1, current.column(), root());
        case QAbstractItemView::MoveDown:
	    return model()->index(current.row() + 1, current.column(), root());
        case QAbstractItemView::MoveLeft:
	    return model()->index(current.row(), current.column() - 1, root());
        case QAbstractItemView::MoveRight:
	    return model()->index(current.row(), current.column() + 1, root());
        case QAbstractItemView::MoveHome:
	    return model()->index(0, current.column(), root());
        case QAbstractItemView::MoveEnd:
	    return model()->index(bottomRight.row(), current.column(), root());
        case QAbstractItemView::MovePageUp: {
	    int newRow = rowAt(itemViewportRect(current).top() - viewport()->height());
	    return model()->index(newRow <= bottomRight.row() ? newRow : 0, current.column(), root());
	}
        case QAbstractItemView::MovePageDown: {
	    int newRow = rowAt(itemViewportRect(current).bottom() + viewport()->height());
	    return model()->index(newRow >= 0 ? newRow : bottomRight.row(), current.column(), root());
	}
    }
    return QModelIndex();
}

void QGenericTableView::setSelection(const QRect &rect, QItemSelectionModel::SelectionUpdateMode mode)
{
    QModelIndex tl = itemAt(rect.left(), rect.top());
    QModelIndex br = itemAt(rect.right(), rect.bottom());
    if (!tl.isValid() || !br.isValid())
	return;

    if (d->topLeft == tl && d->bottomRight == br)
	return;

    d->topLeft = tl;
    d->bottomRight = br;

    // FIXME: only change selection when the tl and br items have changed
    selectionModel()->select(new QItemSelection(tl, br, model()), mode, selectionBehavior());
}

QRect QGenericTableView::selectionRect(const QItemSelection *selection) const
{
    // We only care about the root level in the model.
    // Also, as the table displays the items as they are stored in the model,
    // we only need the top ledf and bottom right items.
    QModelIndex bottomRight = model()->bottomRight(root());
    int top = bottomRight.row();
    int left = bottomRight.column();
    int bottom = 0;
    int right = 0;
    int rangeTop, rangeLeft, rangeBottom, rangeRight;
    int i;
    for (i = 0; i < selection->ranges.count(); ++i) {
        QItemSelectionRange r = selection->ranges.at(i);
	if (r.parent().isValid())
	    continue; // FIXME: table don't know about anything but toplevel items
	rangeTop = d->leftHeader->index(r.top());
	rangeLeft = d->topHeader->index(r.left());
	rangeBottom = d->leftHeader->index(r.bottom());
	rangeRight = d->topHeader->index(r.right());
	if (rangeTop < top)
	    top = rangeTop;
	if (rangeLeft < left)
	    left = rangeLeft;
	if (rangeBottom > bottom)
	    bottom = rangeBottom;
	if (rangeRight > right)
	    right = rangeRight;
    }
    int leftPos = columnViewportPosition(left);
    int topPos = rowViewportPosition(top);
    int rightPos = columnViewportPosition(right) + columnWidth(right);
    int bottomPos = rowViewportPosition(bottom) + rowHeight(bottom);
    return QRect(leftPos, topPos, rightPos - leftPos, bottomPos - topPos);
}

void QGenericTableView::rowCountChanged(int, int)
{
    updateGeometries();
    updateViewport();
//    if (viewport()->height() >= contentsHeight())
//         QApplication::postEvent(model(), new QMetaCallEvent(QEvent::InvokeSlot,
// 				model()->metaObject()->indexOfSlot("fetchMore()"), this));
//    if (viewport()->height() >= contentsHeight())
//	emit needMore();
}

void QGenericTableView::columnCountChanged(int, int)
{
    updateGeometries();
    updateViewport();
}

void QGenericTableView::updateGeometries()
{
    int width = d->leftHeader->sizeHint().width();
    QSize topHint = d->topHeader->sizeHint();

    bool reverse = QApplication::reverseLayout();
    setViewportMargins(reverse ? 0 : width, topHint.height(), reverse ? width : 0, 0);

    QRect vg = viewport()->geometry();
    d->leftHeader->setGeometry(reverse ? vg.right() + 1 : (vg.left() - 1 - width), vg.top(), width, vg.height());
    d->topHeader->setGeometry(reverse ? vg.right() - topHint.width() : vg.left(),
			      vg.top() - topHint.height() - 1, vg.width(), topHint.height());

    // update sliders
    QItemOptions options;
    getViewOptions(&options);
    QSize def = itemDelegate()->sizeHint(fontMetrics(), options, model()->index(0, 0, 0));
    
    int h = viewport()->height();
    int row = model()->rowCount(0);
    verticalScrollBar()->setPageStep(h / def.height() * verticalFactor());
    while (h > 0 && row > 0)
 	h -= d->leftHeader->sectionSize(--row);
    int max = row * verticalFactor();
    if (h < 0)
	 max += 1 + (verticalFactor() * -h / d->leftHeader->sectionSize(row));
    verticalScrollBar()->setRange(0, max);

    int w = viewport()->width();
    int col = model()->columnCount(0);
    horizontalScrollBar()->setPageStep(w / def.width() * horizontalFactor());
    while (w > 0 && col > 0)
 	w -= d->topHeader->sectionSize(--col);
    max = col * horizontalFactor();
    if (w < 0)
 	max += (horizontalFactor() * -w / d->topHeader->sectionSize(col));
    horizontalScrollBar()->setRange(0, max);
}

void QGenericTableView::setHorizontalOffset(int value)
{
    int column = value / horizontalFactor();
    int left = (value % horizontalFactor()) * columnWidth(column);
    d->topHeader->setOffset((left / horizontalFactor()) +
			    d->topHeader->sectionPosition(column));
}

void QGenericTableView::setVerticalOffset(int value)
{
    int row = value / verticalFactor();
    int above = (value % verticalFactor()) * rowHeight(row);
    d->leftHeader->setOffset((above / verticalFactor()) +
			     d->leftHeader->sectionPosition(row));
}

int QGenericTableView::rowSizeHint(int row) const
{
    int columnfirst = columnAt(0);
    int columnlast = columnAt(viewport()->width());

    QItemOptions options;
    getViewOptions(&options);

    int hint = 0;
    QModelIndex index;
    for (int column = columnfirst; column < columnlast; ++column) {
	index = model()->index(row, column, root());
	hint = qMax(hint, itemDelegate()->sizeHint(fontMetrics(), options, index).height());
    }
    return hint;
}

int QGenericTableView::columnSizeHint(int column) const
{
    int rowfirst = rowAt(0);
    int rowlast = rowAt(viewport()->height());

    QItemOptions options;
    getViewOptions(&options);

    int hint = 0;
    QModelIndex index;
    for (int row = rowfirst; row < rowlast; ++row) {
	index = model()->index(row, column, root());
	hint = qMax(hint, itemDelegate()->sizeHint(fontMetrics(), options, index).width());
    }
    return hint;
}

int QGenericTableView::rowViewportPosition(int row) const
{
    return d->leftHeader->sectionPosition(row) - d->leftHeader->offset();
}

int QGenericTableView::rowHeight(int row) const
{
    return d->leftHeader->sectionSize(row);
}

int QGenericTableView::rowAt(int y) const
{
    return d->leftHeader->sectionAt(y + d->leftHeader->offset());
}

int QGenericTableView::columnViewportPosition(int column) const
{
    return d->topHeader->sectionPosition(column) - d->topHeader->offset();
}

int QGenericTableView::columnWidth(int column) const
{
    return d->topHeader->sectionSize(column);
}

int QGenericTableView::columnAt(int x) const
{
    return d->topHeader->sectionAt(x + d->topHeader->offset());
}

bool QGenericTableView::isRowHidden(int row) const
{
    return d->leftHeader->isSectionHidden(row);
}

bool QGenericTableView::isColumnHidden(int column) const
{
    return d->topHeader->isSectionHidden(column);
}

void QGenericTableView::setShowGrid(bool show)
{
    d->showGrid = show;
}

bool QGenericTableView::showGrid() const
{
    return d->showGrid;
}

int QGenericTableView::contentsX() const
{
    return d->topHeader->offset();
}

int QGenericTableView::contentsY() const
{
    return d->leftHeader->offset();
}

int QGenericTableView::contentsWidth() const
{
    return d->topHeader->size();
}

int QGenericTableView::contentsHeight() const
{
    return d->leftHeader->size();
}

void QGenericTableView::rowHeightChanged(int row, int /*oldSize*/, int /*newSize*/)
{
//     updateContents(contentsX(), rowPosition(row),
// 		    visibleWidth(), visibleHeight() - rowPosition(row) + contentsY());
    updateGeometries();
    int rowp = rowViewportPosition(row);
    updateViewport(QRect(0, rowp, viewport()->width(), viewport()->height() - rowp));
    updateCurrentEditor();
}

void QGenericTableView::columnWidthChanged(int column, int /*oldSize*/, int /*newSize*/)
{
//     updateContents(columnPosition(column), contentsY(),
// 		    visibleWidth() - columnPosition(column) + contentsX(), visibleHeight());
    updateGeometries();
    int colp = columnViewportPosition(column);
    updateViewport(QRect(colp, 0, viewport()->height(), viewport()->width() - colp));
    updateCurrentEditor();
}

void QGenericTableView::rowIndexChanged(int, int /*oldIndex*/, int /*newIndex*/)
{
//     int o = d->leftHeader->sectionPosition(d->leftHeader->section(oldIndex));
//     int n = d->leftHeader->sectionPosition(d->leftHeader->section(newIndex));
//     int top = (o < n ? o : n);
//     int height = visibleHeight() - (o > n ? o : n ) + contentsY();
//     updateContents(contentsX(), top, visibleWidth(), height);
    updateGeometries();
    updateViewport();
}

void QGenericTableView::columnIndexChanged(int, int /*oldIndex*/, int /*newIndex*/)
{
//     int o = d->topHeader->sectionPosition(d->topHeader->section(oldIndex));
//     int n = d->topHeader->sectionPosition(d->topHeader->section(newIndex));
//     int left = (o < n ? o : n);
//     int width = visibleWidth() - (o > n ? o : n) + contentsX();
//     updateContents(left, contentsY(), width, visibleHeight());
    updateGeometries();
    updateViewport();
}

void QGenericTableView::selectRow(int row, ButtonState state)
{
    QModelIndex bottomRight = model()->bottomRight(root());
    if (row >= 0 && row <= bottomRight.row()) {
	QModelIndex tl = model()->index(row, 0, root());
	QModelIndex br = model()->index(row, bottomRight.column(), root());
	selectionModel()->setCurrentItem(tl, QItemSelectionModel::NoUpdate,
					 QItemSelectionModel::SelectItems);
	selectionModel()->select(new QItemSelection(tl, br, model()),
				 selectionUpdateMode(state),
				 QItemSelectionModel::SelectItems);
    }
}

void QGenericTableView::selectColumn(int column, ButtonState state)
{
    QModelIndex bottomRight = model()->bottomRight(root());
    if (column >= 0 && column <= bottomRight.column()) {
	QModelIndex tl = model()->index(0, column, root());
	QModelIndex br = model()->index(bottomRight.row(), column, root());
	selectionModel()->setCurrentItem(tl, QItemSelectionModel::NoUpdate,
					 QItemSelectionModel::SelectItems);
	selectionModel()->select(new QItemSelection(tl, br, model()),
				 selectionUpdateMode(state),
				 QItemSelectionModel::SelectItems);
    }
}

void QGenericTableView::hideRow(int row)
{
    d->leftHeader->hideSection(row);
}

void QGenericTableView::hideColumn(int column)
{
    d->topHeader->hideSection(column);
}

void QGenericTableView::showRow(int row)
{
    d->leftHeader->showSection(row);
}

void QGenericTableView::showColumn(int column)
{
    d->topHeader->showSection(column);
}

#include "qgenerictableview.h"
#include <qgenericheader.h>
#include <qitemdelegate.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <private/qobject_p.h>

/*!
  \class QGenericTableView qgenerictableview.h

  \brief Table view implementation

  This class implements a table representation of a QGenericItemView working
  on a QGenericItemModel.
*/

class QGenericTableViewPrivate
{
public:
    QGenericTableViewPrivate(QGenericTableView *owner)
	: drawGrid(true), topHeader(0), leftHeader(0), q(owner) {}

    bool drawGrid;
    QGenericHeader *topHeader, *leftHeader;
    QModelIndex topLeft, bottomRight; // Used for optimization in setSelection
    QGenericTableView *q;
};

QGenericTableView::QGenericTableView(QGenericItemModel *model, QWidget *parent, const char *name)
    : QAbstractItemView(model, parent, name),
      d(new QGenericTableViewPrivate(this))
{
    setLeftHeader(new QGenericHeader(model, Vertical, this, "leftHeader"));
    d->leftHeader->setClickable(true);
    setTopHeader(new QGenericHeader(model, Horizontal, this, "topHeader"));
    d->topHeader->setClickable(true);

    model->fetchMore(); // FIXME: can we move this to qabstractitemview?
}

QGenericTableView::~QGenericTableView()
{
    delete d;
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
	disconnect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
		   d->topHeader, SLOT(setOffset(int)));
	disconnect(d->topHeader,SIGNAL(sectionSizeChanged(int,int,int)),
		   this, SLOT(columnWidthChanged(int,int,int)));
	disconnect(d->topHeader, SIGNAL(sectionIndexChanged(int,int,int)),
		   this, SLOT(columnIndexChanged(int,int,int)));
	disconnect(d->topHeader, SIGNAL(sectionClicked(int, ButtonState)),
		   this, SLOT(selectColumn(int, ButtonState)));
	disconnect(d->topHeader, SIGNAL(sectionCountChanged(int, int)),
		   this, SLOT(columnCountChanged(int, int)));	
    }

    d->topHeader = header;

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
	    d->topHeader, SLOT(setOffset(int)));
    connect(d->topHeader,SIGNAL(sectionSizeChanged(int,int,int)),
	    this, SLOT(columnWidthChanged(int,int,int)));
    connect(d->topHeader, SIGNAL(sectionIndexChanged(int,int,int)),
	    this, SLOT(columnIndexChanged(int,int,int)));
    connect(d->topHeader, SIGNAL(sectionClicked(int, ButtonState)),
	    this, SLOT(selectColumn(int, ButtonState)));
    connect(d->topHeader, SIGNAL(sectionCountChanged(int, int)),
	    this, SLOT(columnCountChanged(int, int)));
    
    // FIXME: this needs to be set in setSelectionModel too
    d->topHeader->setSelectionModel(selectionModel());
    columnCountChanged(0, d->topHeader->count());
}

void QGenericTableView::setLeftHeader(QGenericHeader *header)
{
    if (d->leftHeader) {
	disconnect(verticalScrollBar(), SIGNAL(valueChanged(int)),
		   d->leftHeader, SLOT(setOffset(int)));
	disconnect(d->leftHeader, SIGNAL(sectionSizeChanged(int,int,int)),
		   this, SLOT(rowHeightChanged(int,int,int)));
	disconnect(d->leftHeader, SIGNAL(sectionIndexChanged(int,int,int)),
		   this, SLOT(rowIndexChanged(int,int,int)));
	disconnect(d->leftHeader, SIGNAL(sectionClicked(int, ButtonState)),
		   this, SLOT(selectRow(int,ButtonState)));
	disconnect(d->leftHeader, SIGNAL(sectionCountChanged(int, int)),
		   this, SLOT(rowCountChanged(int, int)));
    }
#include <qgenericitemmodel.h>
    d->leftHeader = header;

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
	    d->leftHeader, SLOT(setOffset(int)));
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
    rowCountChanged(0, d->leftHeader->count());
}

void QGenericTableView::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
    int colfirst = columnAt(cx);
    int collast = columnAt(cx + cw);
    int rowfirst = rowAt(cy);
    int rowlast = rowAt(cy + ch);
    bool drawGrid = d->drawGrid;

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
	int rowp = rowPosition(r);
	int rowh = rowHeight(r);
	for (int c = colfirst; c <= collast; ++c) {
	    if (topHeader->isSectionHidden(c))
		continue;
	    int colp = columnPosition(c);
	    int colw = columnWidth(c);
	    int oldrp = rowp;
	    int oldrh = rowh;

	    p->translate(colp, rowp);

	    QModelIndex item = model()->index(r, c, root());
	    if (item.isValid()) {
		options.itemRect = QRect(colp, rowp, colw, rowh);
		options.selected = sels ? sels->isSelected(item) : 0;
		options.focus = (viewport()->hasFocus() && item == cvi);
		p->fillRect(0, 0, colw, rowh,
			    (options.selected ?
			     options.palette.highlight() :
			     options.palette.base()));
		itemDelegate()->paint(p, options, item);
	    }
	    if (drawGrid) {
		p->setPen(lightGray);
		p->drawLine(0, rowh-1, colw-1, rowh-1);
		p->drawLine(colw-1, 0, colw-1, rowh-1);
		p->setPen(palette().text());
	    }
	    p->translate(-colp, -rowp);

	    rowp = oldrp;
	    rowh = oldrh;
	}
    }
}

QModelIndex QGenericTableView::itemAt(int x, int y) const
{
    return model()->index(rowAt(y), columnAt(x), root());
}

QRect QGenericTableView::itemRect(const QModelIndex &item) const
{
    if (!item.isValid() || model()->parent(item) != root())
	return QRect();
    return QRect(columnPosition(item.column()), rowPosition(item.row()),
		 columnWidth(item.column()), rowHeight(item.row()));
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
	    int newRow = rowAt(itemRect(current).top() - viewport()->height());
	    return model()->index(newRow <= bottomRight.row() ? newRow : 0, current.column(), root());
	}
        case QAbstractItemView::MovePageDown: {
	    int newRow = rowAt(itemRect(current).bottom() + viewport()->height());
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
    int leftPos = columnPosition(left);
    int topPos = rowPosition(top);
    int rightPos = columnPosition(right) + columnWidth(right);
    int bottomPos = rowPosition(bottom) + rowHeight(bottom);
    return QRect(leftPos, topPos, rightPos - leftPos, bottomPos - topPos);
}

void QGenericTableView::rowCountChanged(int, int)
{
    QSize left = d->leftHeader->sizeHint(); // FIXME
    resizeContents(contentsWidth(), left.height());
    updateContents();
    if (viewport()->height() >= contentsHeight())
        QApplication::postEvent(model(), new QMetaCallEvent(QEvent::InvokeSlot,
				model()->metaObject()->indexOfSlot("fetchMore()"), this));
}

void QGenericTableView::columnCountChanged(int, int)
{
    QSize top = d->topHeader->sizeHint();
    resizeContents(top.width(), contentsHeight());
    updateContents();
}

void QGenericTableView::updateGeometries()
{
    int right = QApplication::reverseLayout() ? d->leftHeader->sizeHint().width() : 0;
    int left = QApplication::reverseLayout() ? 0 : d->leftHeader->sizeHint().width();
    int top = d->topHeader->sizeHint().height();
    setMargins(left, top, right, 0);
    int verticalMargin = QApplication::reverseLayout() ? rightMargin() : leftMargin();

    QRect lr(frameWidth(), topMargin() + frameWidth(), verticalMargin, visibleHeight() + topMargin());
    d->leftHeader->setGeometry(QStyle::visualRect(lr, rect()));

    QRect tr(verticalMargin + frameWidth(), frameWidth(), visibleWidth() + verticalMargin, topMargin());
    d->topHeader->setGeometry(QStyle::visualRect(tr, rect()));

    horizontalScrollBar()->raise();
    verticalScrollBar()->raise();
}

int QGenericTableView::rowSizeHint(int row) const
{
    int columnfirst = columnAt(contentsX());
    int columnlast = columnAt(contentsX() + contentsWidth());

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
    int rowfirst = rowAt(contentsY());
    int rowlast = rowAt(contentsY() + contentsHeight());

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

int QGenericTableView::rowPosition(int row) const
{
    return d->leftHeader->sectionPosition(row);
}

int QGenericTableView::rowHeight(int row) const
{
    return d->leftHeader->sectionSize(row);
}

int QGenericTableView::rowAt(int pos) const
{
    return d->leftHeader->sectionAt(pos);
}

int QGenericTableView::columnPosition(int col) const
{
    return d->topHeader->sectionPosition(col);
}

int QGenericTableView::columnWidth(int col) const
{
    return d->topHeader->sectionSize(col);
}

int QGenericTableView::columnAt(int pos) const
{
    return d->topHeader->sectionAt(pos);
}

bool QGenericTableView::isRowHidden(int row) const
{
    return d->leftHeader->isSectionHidden(row);
}

bool QGenericTableView::isColumnHidden(int column) const
{
    return d->topHeader->isSectionHidden(column);
}

void QGenericTableView::setDrawGrid(bool draw)
{
    d->drawGrid = draw;
}

bool QGenericTableView::drawGrid() const
{
    return d->drawGrid;
}

void QGenericTableView::rowHeightChanged(int row, int oldSize, int newSize)
{
    updateContents(contentsX(), rowPosition(row),
		    visibleWidth(), visibleHeight() - rowPosition(row) + contentsY());
    resizeContents(contentsWidth(), contentsHeight() + newSize - oldSize);
    updateCurrentEditor();
}

void QGenericTableView::columnWidthChanged(int column, int oldSize, int newSize)
{
    updateContents(columnPosition(column), contentsY(),
		    visibleWidth() - columnPosition(column) + contentsX(), visibleHeight());
    resizeContents(contentsWidth() + newSize - oldSize, contentsHeight());
    updateCurrentEditor();
}

void QGenericTableView::rowIndexChanged(int, int oldIndex, int newIndex)
{
    int o = d->leftHeader->sectionPosition(d->leftHeader->section(oldIndex));
    int n = d->leftHeader->sectionPosition(d->leftHeader->section(newIndex));
    int top = (o < n ? o : n);
    int height = visibleHeight() - (o > n ? o : n ) + contentsY();
    updateContents(contentsX(), top, visibleWidth(), height);
}

void QGenericTableView::columnIndexChanged(int, int oldIndex, int newIndex)
{
    int o = d->topHeader->sectionPosition(d->topHeader->section(oldIndex));
    int n = d->topHeader->sectionPosition(d->topHeader->section(newIndex));
    int left = (o < n ? o : n);
    int width = visibleWidth() - (o > n ? o : n) + contentsX();
    updateContents(left, contentsY(), width, visibleHeight());
    // FIXME: doesn't work
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

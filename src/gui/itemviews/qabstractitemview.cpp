#include "qabstractitemview.h"
#include <qitemdelegate.h>
#include <qguardedptr.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qbitmap.h>
#include <qpair.h>
#include <qevent.h>
#include <private/qobject_p.h>

class QItemViewDragObjectPrivate
{
public:
//    static bool decode(QMimeSource *src, QModelIndexList &items);
    QModelIndexList items;
    QGenericItemModel *model;
};

QItemViewDragObject::QItemViewDragObject(QAbstractItemView *dragSource, const char *name)
    : QDragObject(dragSource, name)
{
    d = new QItemViewDragObjectPrivate;
    d->model = dragSource->model();
}

QItemViewDragObject::~QItemViewDragObject()
{
    delete d;
}

void QItemViewDragObject::append(QModelIndex &item)
{
    d->items.append(item);
}

void QItemViewDragObject::set(QModelIndexList &items)
{
    d->items = items;
}

const char *QItemViewDragObject::format(int i) const
{
    return d->model->format(i);
}

bool QItemViewDragObject::canDecode(QMimeSource *src) const
{
    return d->model->canDecode(src);
}

QByteArray QItemViewDragObject::encodedData(const char *mime) const
{
    return d->model->encodedData(mime, d->items);
}

bool QItemViewDragObject::decode(QMimeSource *src) const
{
    return d->model->decode(src);
}

class QAbstractItemViewPrivate
{
public:
    QAbstractItemViewPrivate(QAbstractItemView *owner)
	: q(owner),
	  model(0),
	  delegate(0),
//	  sortColumn(-1),
	  layoutLock(false),
	  state(QAbstractItemView::NoState),
	  startEditActions(QItemDelegate::SelectedClicked|
			   QItemDelegate::RenameKeyPressed|
			   QItemDelegate::AnyKeyPressed) {}

    ~QAbstractItemViewPrivate() {}

    inline bool shouldEdit(const QModelIndex &item, QItemDelegate::StartEditAction action)
    {
	return q->model()->isEditable(item) && (action & startEditActions);
    }

    bool createEditor(const QModelIndex &item, QItemDelegate::StartEditAction action, QEvent *event);
//     bool sendItemEvent(const QModelIndex &data, QEvent *event);
//     QWidget *findPersistentEditor( const QModelIndexPtr &item ) const;
//     void insertPersistentEditor( const QModelIndexPtr &item, QWidget *editor );

    QAbstractItemView *q;
    QGenericItemModel *model;
    QGuardedPtr<QWidget> currentEditor;
    QModelIndex editItem;
    QItemDelegate *delegate;
    QItemSelectionModelPointer selectionModel;
 //    QVector<int> sorting;
//     int sortColumn;

    // #### this datastructur is far to inefficient. We need a faster
    // #### way to associate data with an item and look it up.
    // use QHash<QGenericModelItenPtr, QWidget*>
    //QList<QPair<QModelIndex, QWidget*> > persistentEditors;

    bool layoutLock; // FIXME: this is because the layout will trigger resize events
    QRect dragRect;
    QAbstractItemView::State state;
    QPoint cursorIndex;
    int startEditActions;

    QModelIndex root;
};

/*!
  \class QViewItem qgenericitemview.h

  \brief This is an abstract class which is used by QAbstractItemView and
  QViewSelection to work on generic items. A view subclass also
  has to reimplement a QViewItem subclass, which works on the
  internal datastructure.

  QViewItems are short living and reference counted. To ease
  dealing with them, only QViewItemPtr objects are used in the
  API. This class deals with the reference counting.
*/

/*!
  \class QViewSelection qgenericitemview.h

  \brief A selection in a view

  This class represents a selection range in a view. It spans from
  topLeft() to bottomRight(), having the anchor item anchor().

  A QAbstractItemView has a list of QViewSelections.
*/

/*!
  \class QAbstractItemView qgenericitemview.h

  \brief Abstract baseclass for every view working on a QGenericItemModel

  This subclass of QScrollView provides the base functionality needed
  by every item view which works on a QGenericItemModel. It handles common
  functionality of editing of items, keyboard and mouse handling, etc.
 Current item and  selections are handled by the QItemSelectionModel.

  A specific view only implements the specific functionality of that
  view, like drawing an item, returning the geometry of an item,
  finding items, etc.

  To work in an abstract way, items are wrapped in QViewItem
  objects. Every view also has to implement a QViewItem subclass,
  which points to the internal data structure, and two functions to
  convert a QViewItem to a QModelIndex and vica versa. a
  QAbstractItemView does <b>not</b> hold a QViewItem for every displayed
  item. QViewItems are created on demand to point to a certain
  item in the view when passed around in the API.
*/

QAbstractItemView::QAbstractItemView(QGenericItemModel *model, QWidget *parent, const char *name)
    : QScrollView(parent, name),
      d(new QAbstractItemViewPrivate(this))
{
    Q_ASSERT(model)
    setAttribute(WA_NoBackground);
    setAttribute(WA_StaticContents);
    d->model = model;
    setSelectionModel(new QItemSelectionModel(model));

    QObject::connect(model, SIGNAL(contentsChanged(const QModelIndex &, const QModelIndex &)),
		     this, SLOT(contentsChanged(const QModelIndex &, const QModelIndex &)));
    QObject::connect(model, SIGNAL(contentsInserted(const QModelIndex &, const QModelIndex &)),
		     this, SLOT(contentsInserted(const QModelIndex &, const QModelIndex &)));
    QObject::connect(model, SIGNAL(contentsRemoved(const QModelIndex &, const QModelIndex &, const QModelIndex &)),
		     this, SLOT(contentsRemoved(const QModelIndex &, const QModelIndex &, const QModelIndex &)));

    viewport()->setFocusProxy(this);
    viewport()->setFocusPolicy(WheelFocus);

    // FIXME: this is only true when we have a view that is layed out TopToBottom
    connect(verticalScrollBar(), SIGNAL(sliderReleased()), this, SLOT(fetchMore()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(fetchMore()));

    viewport()->setBackgroundRole(QPalette::Base);

    //emit model->contentsChanged(); // initial emit to start layout
    QApplication::postEvent(this, new QMetaCallEvent(QEvent::InvokeSlot,
                            metaObject()->indexOfSlot("startItemsLayout()"), this));
}

QAbstractItemView::~QAbstractItemView()
{
    delete d;
}

QGenericItemModel *QAbstractItemView::model() const
{
    return d->model;
}
/*
void QAbstractItemView::sort(int column, Qt::SortOrder order)
{
    // FIXME: this code is not active yet
    int rows = model()->rowCount(root());
    d->sorting.resize(rows);
    d->sortColumn = column;
    int count = 0;
    // FIXME N^2 rank sorting
    QModelIndex a;
    QModelIndex b;
    for (int i = 0; i < rows; ++i) {
	a = model()->index(i, column, root());
	for (int j = 0; j < rows; ++j) {
	    b = model()->index(j, column, root());
	    if (model()->greater(a, b) || model()->equal(a, b))
		++count;
	}
	d->sorting[count] = i;
    }
}

int QAbstractItemView::sorted(int row) const
{
    return d->sorting.count() ? d->sorting.at(row) : row;
}
*/
void QAbstractItemView::clearSelections()
{
    selectionModel()->clear();
}

void QAbstractItemView::setCurrentItem(const QModelIndex &data)
{
    selectionModel()->setCurrentItem(data, selectionUpdateMode(NoButton),
				     selectionBehavior());
}

QModelIndex QAbstractItemView::currentItem() const
{
    return selectionModel()->currentItem();
}

void QAbstractItemView::setSelectionMode(QItemSelectionModel::SelectionMode mode)
{
    d->selectionModel->setSelectionMode(mode);
}

QItemSelectionModel::SelectionMode QAbstractItemView::selectionMode() const
{
    return d->selectionModel->selectionMode();
}

void QAbstractItemView::setStartEditActions(int actions)
{
    d->startEditActions = actions;
}

int QAbstractItemView::startEditActions() const
{
    return d->startEditActions;
}

void QAbstractItemView::contentsMousePressEvent(QMouseEvent *e)
{
    QPoint pos = e->pos();
    QModelIndex item  = itemAt(pos);
    if (item.isValid() && e->state() & ShiftButton)
	d->dragRect.setBottomRight(pos); // do not normalize
    else
	d->dragRect = QRect(pos, pos);
    if (item.isValid()) {
	if (e->state() & ShiftButton) {
	    selectionModel()->setCurrentItem(item, QItemSelectionModel::NoUpdate, selectionBehavior());
	    setSelection(d->dragRect.normalize(), selectionUpdateMode(e->state()));
	} else {
	    bool isSelected = selectionModel()->isSelected(item);
	    bool isToggle = selectionUpdateMode(e->state()) == QItemSelectionModel::Toggle;
	    bool noUpdate = isSelected && !isToggle;
	    selectionModel()->setCurrentItem(item, (noUpdate ? QItemSelectionModel::NoUpdate :
						    selectionUpdateMode(e->state())), selectionBehavior());
	}
    } else {
 	clearSelections();
    }

    handleEdit(item, QItemDelegate::NoAction, e); // FIXME: ???
}

void QAbstractItemView::contentsMouseMoveEvent(QMouseEvent *e)
{
    if (!(e->state() & Qt::LeftButton))
	return;
    QPoint pos = e->pos();

    QRect oldRect = d->dragRect;
    d->dragRect.setBottomRight(pos); // do not normalize

    ensureVisible(pos.x(), pos.y());

    if (state() == Dragging &&
	(d->dragRect.topLeft() - pos).manhattanLength() > QApplication::startDragDistance()) {
	startDrag();
	setState(NoState);
   	return;
    }

    QModelIndex item = itemAt(pos);
    if (currentItem() == item && state() == Selecting) {
	updateContents(d->dragRect.normalize() | oldRect.normalize()); // draw selection rect
	return;
    }

    if (item.isValid()) {
	if (state() != Selecting) {
	    bool dnd = model()->isDragEnabled(item) && supportsDragAndDrop();
	    bool selected = selectionModel()->isSelected(item);
	    if (dnd && selected) {
		setState(Dragging);
		return;
	    }
	}
	selectionModel()->setCurrentItem(item, QItemSelectionModel::NoUpdate, selectionBehavior());
    }
    setState(Selecting);
    setSelection(d->dragRect.normalize(), selectionUpdateMode(e->state()));
    updateContents(d->dragRect.normalize() | oldRect.normalize()); // draw selection rect

    handleEdit(item, QItemDelegate::NoAction, e); // FIXME: ???
}

void QAbstractItemView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    updateContents(d->dragRect.normalize());
    setState(NoState);
    handleEdit(currentItem(), QItemDelegate::NoAction, e); // FIXME: ???
}

void QAbstractItemView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
    QModelIndex item = itemAt(e->pos());
    if (!item.isValid())
	return;
    handleEdit(item, QItemDelegate::DoubleClicked, e);
}

void QAbstractItemView::contentsDragEnterEvent(QDragEnterEvent *e)
{
    if (model()->canDecode(e))
	e->accept();
}

void QAbstractItemView::contentsDropEvent(QDropEvent *e)
{
    if (!model()->decode(e)) {
	// something went wrong
	return;
    }
}

void QAbstractItemView::focusInEvent(QFocusEvent *e)
{
    QScrollView::focusInEvent(e);
    QModelIndex item = currentItem();
    if (item.isValid())
	updateItem(item);
}

void QAbstractItemView::focusOutEvent(QFocusEvent *e)
{
    QScrollView::focusOutEvent(e);
    QModelIndex item = currentItem();
    if (item.isValid())
	updateItem(item);
}

void QAbstractItemView::keyPressEvent(QKeyEvent *e)
{
    bool hadCurrent = true;
    QModelIndex current = currentItem();
    if (!current.isValid()) {
	hadCurrent = false;
	setCurrentItem(model()->index(0, 0, 0));
    }
    QModelIndex newCurrent = current;
    bool navigationKey = false;
    if (hadCurrent) {
	switch (e->key()) {
	case Key_Down:
	    newCurrent = moveCursor(MoveDown, e->state());
	    navigationKey = true;
	    break;
	case Key_Up:
	    newCurrent = moveCursor(MoveUp, e->state());
	    navigationKey = true;
	    break;
	case Key_Left:
	    newCurrent = moveCursor(MoveLeft, e->state());
	    navigationKey = true;
	    break;
	case Key_Right:
	    newCurrent = moveCursor(MoveRight, e->state());
	    navigationKey = true;
	    break;
	case Key_Home:
	    newCurrent = moveCursor(MoveHome, e->state());
	    navigationKey = true;
	    break;
	case Key_End:
	    newCurrent = moveCursor(MoveEnd, e->state());
	    navigationKey = true;
	    break;
	case Key_PageUp:
	    newCurrent = moveCursor(MovePageUp, e->state());
	    navigationKey = true;
	    break;
	case Key_PageDown:
	    newCurrent = moveCursor(MovePageDown, e->state());
	    navigationKey = true;
	    break;
	}

	if (newCurrent != current && newCurrent.isValid()) {
	    if (e->state() & ShiftButton) {
		d->dragRect.setBottomRight(itemRect(newCurrent).bottomRight());
		selectionModel()->setCurrentItem(newCurrent, QItemSelectionModel::NoUpdate, selectionBehavior());
		setSelection(d->dragRect.normalize(), selectionUpdateMode(e->state()));
	    } else if (e->state() & ControlButton) {
		d->dragRect = itemRect(newCurrent);
		selectionModel()->setCurrentItem(newCurrent, QItemSelectionModel::NoUpdate, selectionBehavior());
	    } else {
		d->dragRect = itemRect(newCurrent);
		selectionModel()->setCurrentItem(newCurrent, selectionUpdateMode(e->state()), selectionBehavior());
	    }
	}
    }

    switch (e->key()) {
    case Key_F2:
	if (handleEdit(currentItem(), QItemDelegate::RenameKeyPressed, e))
	    return;
	break;
    case Key_Escape: // keys to ignore
    case Key_Enter:
    case Key_Return:
	break;
    case Key_Space:
	if (!navigationKey)
	    if (handleEdit(currentItem(), QItemDelegate::NoAction, e))
		return;
	break;
    default:
	if (!navigationKey && e->text()[0].isPrint())
	    if (handleEdit(currentItem(), QItemDelegate::AnyKeyPressed, e))
		return;
	break;
    }

    switch (e->key()) {
	case Key_Space:
	    selectionModel()->select(currentItem(), QItemSelectionModel::Toggle, selectionBehavior());
	    break;
    }
}

void QAbstractItemView::keyReleaseEvent(QKeyEvent *e)
{
    handleEdit(currentItem(), QItemDelegate::NoAction, e);
}

void QAbstractItemView::resizeEvent(QResizeEvent *e)
{
    QScrollView::resizeEvent(e);
    updateGeometries();
}

void QAbstractItemView::showEvent(QShowEvent *e)
{
    QScrollView::showEvent(e);
    updateGeometries();
}

bool QAbstractItemView::handleEdit(const QModelIndex &item,
				   QItemDelegate::StartEditAction action,
				   QEvent *event)
{
   if (!model()->isEditable(item))//|| data->editType() == QItemDElegate::PersistentWidget)
	return false;

   QItemDelegate::EditType editType = itemDelegate()->editType(item);
   bool type = (editType == QItemDelegate::PersistentWidget || editType == QItemDelegate::WidgetOnTyping);
   if (type && (action & d->startEditActions)) {
       setState(Editing);
       return d->createEditor(item, action, event);
   }

//     if (event && delegate->editType() == QItemDelegate::NoWidget)
// 	return d->sendItemEvent(data, event);

    return false;
}

void QAbstractItemView::endEdit(const QModelIndex &item, bool accept)
{
    setState(NoState);

    if (!d->currentEditor)
	return;

    if (!item.isValid())
	return ;

    if (!model()->isEditable(item)) {
	delete d->currentEditor;
	setFocus();
	return;
    }

    QItemDelegate::EditType editType = itemDelegate()->editType(item);
    if (editType == QItemDelegate::PersistentWidget || editType == QItemDelegate::NoWidget) {
	delete d->currentEditor;
	setFocus();
	return;
    }

    if (accept)
	itemDelegate()->setContentFromEditor((QWidget*)d->currentEditor, item);
    delete d->currentEditor;
    setFocus();
}

void QAbstractItemView::updateCurrentEditor()
{
    //  this presumes that only one editor is open at one time
    QModelIndex item = currentItem(); // the edited item
    if (!d->currentEditor)//|| !item->isEditable() || item->editType() != QItemDelegate::PersistentWidget )
 	return;
    QItemOptions options;
    getViewOptions(&options);
    options.itemRect = itemRect(currentItem());
    options.itemRect.moveTop(options.itemRect.top() - contentsY());
    options.itemRect.moveLeft(options.itemRect.left() - contentsX());
    itemDelegate()->updateEditorGeometry(d->currentEditor, options, item);
}

void QAbstractItemView::updateGeometries()
{
    // do nothing
}

bool QAbstractItemView::eventFilter(QObject *o, QEvent *e)
{
    // FIXME: try to remove this
    if (o == d->currentEditor) {
	if (e->type() == QEvent::KeyPress) {
	    switch (((QKeyEvent*)e)->key()) {
	    case Key_Escape:
		endEdit(d->editItem, false);
		return true;
	    case Key_Enter:
	    case Key_Return:
		endEdit(d->editItem, true);
		return true;
	    default:
		break;
	    }
	}
    }
    return QScrollView::eventFilter(o, e);
}

void QAbstractItemView::contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // Single item changed
    if (topLeft == bottomRight && topLeft.isValid()) {
	if (d->currentEditor && topLeft == currentItem())
	    itemDelegate()->updateEditorContents(d->currentEditor, topLeft);
	else
	    updateItem(topLeft);
	return;
    }

    // FIXME: narrow down to the changed items
    updateContents();
    startItemsLayout();
}

void QAbstractItemView::contentsInserted(const QModelIndex &, const QModelIndex &)
{
    // do nothing
}

void QAbstractItemView::contentsRemoved(const QModelIndex &parent, const QModelIndex &, const QModelIndex &)
{
    // do nothing
}

QItemDelegate *QAbstractItemView::itemDelegate() const
{
    if (!d->delegate)
	d->delegate = new QItemDelegate(model()); // FIXME: memleak
    return d->delegate;
}

void QAbstractItemView::setItemDelegate(QItemDelegate *delegate)
{
//     if (delegate->model() != model()) {
// 	qWarning( "QAbstractItemView::setDelegate() failed: Trying to set a delegate, "
// 		  "which works on a different model than the view." );
// 	return;
//     }
    d->delegate = delegate;
}

void QAbstractItemView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    if (selectionModel->model() != model()) {
	qWarning( "QAbstractItemView::setSelectionModel() failed: Trying to set a selection model, "
		  "which works on a different model than the view." );
	return;
    }

    if (!!d->selectionModel) {
	disconnect(d->selectionModel,
		   SIGNAL(selectionChanged(const QItemSelectionPointer&, const QItemSelectionPointer&)),
		   this,
		   SLOT(selectionChanged(const QItemSelectionPointer&, const QItemSelectionPointer&)));
	disconnect(d->selectionModel,
		   SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
		   this,
		   SLOT(currentChanged(const QModelIndex&, const QModelIndex&)));
    }

    d->selectionModel = selectionModel;

    connect(d->selectionModel,
	    SIGNAL(selectionChanged(const QItemSelectionPointer&, const QItemSelectionPointer&)),
	    this, SLOT(selectionChanged(const QItemSelectionPointer&, const QItemSelectionPointer&)));
    connect(d->selectionModel,
	    SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
	    this, SLOT(currentChanged(const QModelIndex&, const QModelIndex&)));
}

QItemSelectionModel* QAbstractItemView::selectionModel() const
{
    return d->selectionModel;
}

void QAbstractItemView::selectionChanged(const QItemSelectionPointer &deselected,
					 const QItemSelectionPointer &selected)
{
    QRect rect;
    if (!!deselected) {
 	rect = selectionRect(deselected);
 	updateContents(rect);
    }
    if (!!selected) {
 	rect = selectionRect(selected);
 	updateContents(rect);
    }
}

void QAbstractItemView::currentChanged(const QModelIndex &old, const QModelIndex &current)
{
    if (d->currentEditor)
	endEdit(old, true);

    // FIXME: if the mode is SelectCurrent, there is no need to repaint current
    // this will be done in selectionChanged

    // FIXME: calling ensureItemVisible first before redrawing oldRect
    // make the view look slow, but since repaintItem does not update the rect
    // immediately we get drawing errors because the contentview might be shifted
    if (current.isValid())
	ensureItemVisible(current);

    if (old.isValid())
// 	if (selectionBehavior() == QItemSelectionModel::SelectRows)
// 	    updateRow(old);
// 	else
	updateItem(old);
    if (current.isValid())
// 	if (selectionBehavior() == QItemSelectionModel::SelectRows)
// 	    updateRow(current);
// 	else
	updateItem(current);

    handleEdit(current, QItemDelegate::CurrentChanged, 0); // FIXME: ???
}

void QAbstractItemView::startItemsLayout()
{
    if (!d->layoutLock) {
	d->layoutLock = true;
	while (!doItemsLayout(100))
	    qApp->processEvents();
	d->layoutLock = false;
    }
}

bool QAbstractItemView::doItemsLayout(int)
{
    return true; // Do nothing
}

void QAbstractItemView::fetchMore()
{
    if (!verticalScrollBar()->isSliderDown() &&
	verticalScrollBar()->value() == verticalScrollBar()->maximum())
	model()->fetchMore();
}

void QAbstractItemView::updateItem(const QModelIndex &item)
{
    updateContents(itemRect(item));
}

void QAbstractItemView::updateRow(const QModelIndex &item)
{
    QModelIndex parent = model()->parent(item);
    int row = item.row();
    int columns = model()->columnCount(parent);
    QModelIndex left = model()->index(row, 0, parent);
    QModelIndex right = model()->index(row, columns - 1, parent);
    QRect rect = itemRect(left) | itemRect(right);
    updateContents(rect);
}

void QAbstractItemView::ensureItemVisible(const QModelIndex &item)
{
    QRect rect = itemRect(item);
    if (!rect.isValid())
	return;
    int xmargin = rect.width() >> 1;
    int ymargin = rect.height() >> 1;
    ensureVisible(rect.x() + xmargin, rect.y() + ymargin, xmargin, ymargin);
}

void QAbstractItemView::clearArea(QPainter *painter, const QRect &rect) const
{
    painter->fillRect(rect, palette().brush(QPalette::Base));
}

void QAbstractItemView::drawSelectionRect(QPainter *painter, const QRect &rect) const
{
    style().drawPrimitive(QStyle::PE_FocusRect, painter, rect, palette(),
			  QStyle::Style_Default, QStyleOption(palette().base()));
}

bool QAbstractItemView::supportsDragAndDrop() const
{
    return false;
}

QDragObject *QAbstractItemView::dragObject()
{
    QItemViewDragObject *dragObject = new QItemViewDragObject(this, "DragObject");
    QModelIndexList items = selectionModel()->selectedItems();
     dragObject->set(items);
    return dragObject;
}

void QAbstractItemView::startDrag()
{
    QDragObject *obj = dragObject();
    if (!obj)
	return;
//     if ( obj->drag() && obj->target() != viewport() )
// 	emit moved();
    obj->drag();
}

QRect QAbstractItemView::dragRect() const
{
    return d->dragRect;
}

void QAbstractItemView::setRoot(const QModelIndex &item)
{
    d->root = item;
    int r = model()->rowCount(item) - 1;
    int c = model()->columnCount(item) - 1;
    contentsChanged(model()->index(0, 0, item), model()->index(r, c, item));
}

QModelIndex QAbstractItemView::root() const
{
    return d->root;
}

// FIXME: find another way of getting this info

void QAbstractItemView::getViewOptions(QItemOptions *options) const
{
    options->palette = palette();
    options->editing = state() == Editing;
}

// FIXME: find better names

QAbstractItemView::State QAbstractItemView::state() const
{
    return d->state;
}

void QAbstractItemView::setState(State state)
{
    d->state = state;
}

QItemSelectionModel::SelectionBehavior QAbstractItemView::selectionBehavior() const
{
    return QItemSelectionModel::SelectItems;
}

QItemSelectionModel::SelectionUpdateMode QAbstractItemView::selectionUpdateMode(ButtonState state) const
{
    if (selectionMode() == QItemSelectionModel::Single)
	return QItemSelectionModel::ClearAndSelect;
    if (state & ShiftButton)
	return QItemSelectionModel::Select;
    if (state & ControlButton)
	return QItemSelectionModel::Toggle;
    if (QAbstractItemView::state() == Selecting)
	return QItemSelectionModel::Select;
    return QItemSelectionModel::ClearAndSelect;
}

bool QAbstractItemViewPrivate::createEditor(const QModelIndex &item,
					    QItemDelegate::StartEditAction action,
					    QEvent *event)
{
    static bool recursive_lock = false; // FIXME: hack around design flaw
    if ( recursive_lock )
	return false;
    recursive_lock = true;

    QItemDelegate *delegate = q->itemDelegate();

    QItemOptions options;
    q->getViewOptions(&options);
    options.itemRect = q->itemRect(item);
    options.itemRect.moveTop(options.itemRect.top() - q->contentsY());
    options.itemRect.moveLeft(options.itemRect.left() - q->contentsX());

    QWidget *w = delegate->createEditor(action, q->viewport(), options, item);
    w->show();
    w->setFocus();
    if (event &&
	(action == QItemDelegate::AnyKeyPressed || event->type() == QEvent::MouseButtonPress))
	QApplication::sendEvent(w, event);

    recursive_lock = false;

    if (delegate->editType(item) != QItemDelegate::PersistentWidget)
	currentEditor = w;
    currentEditor->installEventFilter(q);
    editItem = item;
    return true;
}

// FIXME: sendeing events is part of the item interface and should not be done for model items
/*
bool QAbstractItemViewPrivate::sendItemEvent(const QModelIndex &data, QEvent *event)
{
    QItemOptions options( q );
    q->getViewOptions( &options );
    options.itemRect = q->itemRect( q->itemToViewItem( data ) );
    options.itemRect.moveTop( options.itemRect.top() - q->contentsY() );
    options.itemRect.moveLeft( options.itemRect.left() - q->contentsX() );

    switch ( event->type() ) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseMove:
	case QEvent::MouseButtonDblClick: {
	    QPoint pt( options.itemRect.x(), options.itemRect.y() );
	    QMouseEvent *me = (QMouseEvent*)event;
	    QMouseEvent ce( me->type(), me->pos() - pt,
			    me->globalPos(), me->button(), me->state() );
	    //data->event( &ce, &options ); // FIXME: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	    if ( ce.isAccepted() ) {
		me->accept();
		return true;
	    }
	} break;
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	    //data->event( event, &options ); // FIXME: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	    if ( ( (QKeyEvent*)event )->isAccepted() ) {
		return true;
	    }
	    break;
	default:
	    break;
    }
    return false;
}

// FIXME: these are not used - implement persistent editor infrastructiure

QWidget *QAbstractItemViewPrivate::findPersistentEditor( const QModelIndex &item ) const
{
    for ( QList<QPair<QModelIndex, QWidget*> >::ConstIterator it =
	      persistentEditors.begin(); it != persistentEditors.end(); ++it ) {
	if ( (*it).first == item )
	    return (*it).second;
    }
    return 0;
}

void QAbstractItemViewPrivate::insertPersistentEditor( const QModelIndex &item,
						       QWidget *editor )
{
    persistentEditors.append( QPair<QModelIndex, QWidget*>( item, editor ) );
}
*/

#include "qabstractitemview.h"
#include <qitemdelegate.h>
#include <qguardedptr.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qbitmap.h>
#include <qpair.h>
#include <qevent.h>
#include <qscrollbar.h>

#include <private/qabstractitemview_p.h>
#define d d_func()
#define q q_func()


QItemViewDragObject::QItemViewDragObject(QAbstractItemView *dragSource)
    : QDragObject(*(new QItemViewDragObjectPrivate), dragSource)
{
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

QAbstractItemViewPrivate::QAbstractItemViewPrivate()
    :   model(0),
        delegate(0),
	selectionModel(0),
//        sortColumn(-1),
        layoutLock(false),
        state(QAbstractItemView::NoState),
        startEditActions(QAbstractItemDelegate::DoubleClicked|
			 QAbstractItemDelegate::EditKeyPressed|
			 QAbstractItemDelegate::AnyKeyPressed|
			 QAbstractItemDelegate::CurrentChanged)
{
}

QAbstractItemViewPrivate::~QAbstractItemViewPrivate()
{
}

void QAbstractItemViewPrivate::init()
{
    q->setSelectionModel(new QItemSelectionModel(model));

    QObject::connect(model, SIGNAL(contentsChanged(QModelIndex,QModelIndex)),
                     q, SLOT(contentsChanged(QModelIndex,QModelIndex)));
    QObject::connect(model, SIGNAL(contentsInserted(QModelIndex,QModelIndex)),
                     q, SLOT(contentsInserted(QModelIndex,QModelIndex)));
    QObject::connect(model, SIGNAL(contentsRemoved(QModelIndex,QModelIndex,QModelIndex)),
                     q, SLOT(contentsRemoved(QModelIndex,QModelIndex,QModelIndex)));

    q->setHorizontalFactor(1024);
    q->setVerticalFactor(1024);

    q->viewport()->installEventFilter(q);
    q->viewport()->setFocusProxy(q);
    q->viewport()->setFocusPolicy(QWidget::WheelFocus);

    // FIXME: this is only true when we have a view that is layed out TopToBottom
    QObject::connect(q->verticalScrollBar(), SIGNAL(sliderReleased()), q, SLOT(fetchMore()));
    QObject::connect(q->verticalScrollBar(), SIGNAL(valueChanged(int)), q, SLOT(fetchMore()));

    QObject::connect(q->verticalScrollBar(), SIGNAL(valueChanged(int)), q, SLOT(updateCurrentEditor()),
		     QueuedConnection);
    QObject::connect(q->horizontalScrollBar(), SIGNAL(valueChanged(int)),q, SLOT(updateCurrentEditor()),
		     QueuedConnection);
    QObject::connect(q, SIGNAL(needMore()), model, SLOT(fetchMore()), QueuedConnection);

    q->viewport()->setBackgroundRole(QPalette::Base);

    //emit model->contentsChanged(); // initial emit to start layout
    QApplication::postEvent(q, new QMetaCallEvent(QEvent::InvokeSlot,
                            q->metaObject()->indexOfSlot("startItemsLayout()"), q));
}

/*!
  \class QAbstractItemView qgenericitemview.h

  \brief Abstract baseclass for every view working on a QGenericItemModel

  This subclass of QViewprt provides the base functionality needed
  by every item view which works on a QGenericItemModel. It handles common
  functionality of editing of items, keyboard and mouse handling, etc.
 Current item and  selections are handled by the QItemSelectionModel.

  A specific view only implements the specific functionality of that
  view, like drawing an item, returning the geometry of an item,
  finding items, etc.
*/

QAbstractItemView::QAbstractItemView(QGenericItemModel *model, QWidget *parent)
    : QViewport(*(new QAbstractItemViewPrivate), parent)
{
    Q_ASSERT(model)
    d->model = model;
    d->init();
}

QAbstractItemView::QAbstractItemView(QAbstractItemViewPrivate &dd, QGenericItemModel *model, QWidget *parent)
    : QViewport(dd, parent)
{
    Q_ASSERT(model)
    d->model = model;
    d->init();
}

QAbstractItemView::~QAbstractItemView()
{
}

QGenericItemModel *QAbstractItemView::model() const
{
    return d->model;
}

void QAbstractItemView::setHorizontalFactor(int factor)
{
    d->horizontalFactor = factor;
    horizontalScrollBar()->setSingleStep(factor);
}

int QAbstractItemView::horizontalFactor() const
{
    return d->horizontalFactor;
}

void QAbstractItemView::setVerticalFactor(int factor)
{
    d->verticalFactor = factor;
    verticalScrollBar()->setSingleStep(factor);
}

int QAbstractItemView::verticalFactor() const
{
    return d->verticalFactor;
}

void QAbstractItemView::clearSelections()
{
    selectionModel()->clear();
}

void QAbstractItemView::setCurrentItem(const QModelIndex &data)
{
    selectionModel()->setCurrentItem(data, selectionUpdateMode(NoButton), selectionBehavior());
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

void QAbstractItemView::viewportMousePressEvent(QMouseEvent *e)
{
    QPoint pos = e->pos();
    QModelIndex item = itemAt(pos);
    if (item.isValid()) {
	d->pressedItem = item;
	d->pressedState = e->state();
	selectionModel()->setCurrentItem(item, QItemSelectionModel::NoUpdate, selectionBehavior());
    }

    if (e->state() & ShiftButton)
	d->dragRect.setBottomRight(pos); // do not normalize
    else
	d->dragRect = QRect(pos, pos);

    if (item.isValid())
	setSelection(d->dragRect.normalize(), selectionUpdateMode(e->state(), item, e->type()));
    else
	clearSelections();
}

void QAbstractItemView::viewportMouseMoveEvent(QMouseEvent *e)
{
    if (!(e->state() & LeftButton))
	return;
    QPoint pos = e->pos();

    QRect oldRect = d->dragRect;
    d->dragRect.setBottomRight(pos); // do not normalize

//    ensureVisible(pos.x(), pos.y());

    if (state() == Dragging &&
	(d->dragRect.topLeft() - pos).manhattanLength() > QApplication::startDragDistance()) {
	startDrag();
	setState(NoState);
   	return;
    }

    QModelIndex item = itemAt(pos);
    if (currentItem() == item && state() == Selecting) {
	updateViewport(d->dragRect.normalize() | oldRect.normalize()); // draw selection rect
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
    setSelection(d->dragRect.normalize(), selectionUpdateMode(e->state(), item, e->type()));
    updateViewport(d->dragRect.normalize() | oldRect.normalize()); // draw selection rect
}

void QAbstractItemView::viewportMouseReleaseEvent(QMouseEvent *e)
{
    QPoint pos = e->pos();
    QModelIndex item  = itemAt(pos);
    selectionModel()->select(item,
			     selectionUpdateMode(e->state(),
						 item,
						 e->type()),
			     selectionBehavior());
    d->pressedItem = QModelIndex();
    d->pressedState = NoButton;
    updateViewport(d->dragRect.normalize());
    setState(NoState);
}

void QAbstractItemView::viewportMouseDoubleClickEvent(QMouseEvent *e)
{
    QModelIndex item = itemAt(e->pos());
    if (!item.isValid())
	return;
    startEdit(item, QAbstractItemDelegate::DoubleClicked, e);
}

void QAbstractItemView::viewportContextMenuEvent(QContextMenuEvent *)
{
    // do nothing
}

void QAbstractItemView::viewportDragEnterEvent(QDragEnterEvent *e)
{
    if (model()->canDecode(e))
	e->accept();
}

void QAbstractItemView::viewportDragMoveEvent(QDragMoveEvent *)
{
    // do nothing
}

void QAbstractItemView::viewportDragLeaveEvent(QDragLeaveEvent *)
{
    // do nothing
}

void QAbstractItemView::viewportDropEvent(QDropEvent *e)
{
    if (model()->decode(e))
	e->accept();
}

void QAbstractItemView::focusInEvent(QFocusEvent *e)
{
    QViewport::focusInEvent(e);
    QModelIndex item = currentItem();
    if (item.isValid())
	updateItem(item);
}

void QAbstractItemView::focusOutEvent(QFocusEvent *e)
{
    QViewport::focusOutEvent(e);
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
    if (hadCurrent) {
	switch (e->key()) {
	case Key_Down:
	    newCurrent = moveCursor(MoveDown, e->state());
	    break;
	case Key_Up:
	    newCurrent = moveCursor(MoveUp, e->state());
	    break;
	case Key_Left:
	    newCurrent = moveCursor(MoveLeft, e->state());
	    break;
	case Key_Right:
	    newCurrent = moveCursor(MoveRight, e->state());
	    break;
	case Key_Home:
	    newCurrent = moveCursor(MoveHome, e->state());
	    break;
	case Key_End:
	    newCurrent = moveCursor(MoveEnd, e->state());
	    break;
	case Key_PageUp:
	    newCurrent = moveCursor(MovePageUp, e->state());
	    break;
	case Key_PageDown:
	    newCurrent = moveCursor(MovePageDown, e->state());
	    break;
	}

	if (newCurrent != current && newCurrent.isValid()) {
	    if (e->state() & ShiftButton) {
		d->dragRect.setBottomRight(itemViewportRect(newCurrent).bottomRight());
		selectionModel()->setCurrentItem(newCurrent,
						 QItemSelectionModel::NoUpdate,
						 selectionBehavior());
		setSelection(d->dragRect.normalize(),
			     selectionUpdateMode(e->state(), newCurrent, e->type(), (Key)e->key()));
	    } else if (e->state() & ControlButton) {
		d->dragRect = itemViewportRect(newCurrent);
		selectionModel()->setCurrentItem(newCurrent,
						 QItemSelectionModel::NoUpdate,
						 selectionBehavior());
	    } else {
		d->dragRect = itemViewportRect(newCurrent);
		selectionModel()->setCurrentItem(newCurrent,
						 selectionUpdateMode(e->state(),
								     newCurrent,
								     e->type(),
								     (Key)e->key()),
						 selectionBehavior());
	    }
	    return;
	}
    }

    switch (e->key()) {
    case Key_F2:
	if (startEdit(currentItem(), QAbstractItemDelegate::EditKeyPressed, e))
	    return;
	break;
    case Key_Escape: // keys to ignore
    case Key_Enter:
    case Key_Return:
	break;
    case Key_Space:
	// if (startEdit(currentItem(), QAbstractItemDelegate::NoAction, e)) {
// 		return;
// 	} else {
    {
	QItemSelectionModel::SelectionUpdateMode updateMode =
	    selectionUpdateMode(e->state(), currentItem(), e->type(), (Key)e->key());
	    selectionModel()->select(currentItem(), updateMode, selectionBehavior());
	    return;
	}
	break;
    default:
	if (e->text()[0].isPrint() && // FIXME: ???
	    startEdit(currentItem(), QAbstractItemDelegate::AnyKeyPressed, e))
	    return;
	break;
    }
}

void QAbstractItemView::resizeEvent(QResizeEvent *e)
{
    QViewport::resizeEvent(e);
    updateGeometries();
}

void QAbstractItemView::showEvent(QShowEvent *e)
{
    QViewport::showEvent(e);
    updateGeometries();
}

bool QAbstractItemView::startEdit(const QModelIndex &item,
				  QAbstractItemDelegate::StartEditAction action,
				  QEvent *event)
{
//    QAbstractItemDelegate::EditType editType = itemDelegate()->editType(item);
    if (d->shouldEdit(item, action) && d->createEditor(item, action, event))
	setState(Editing);
//     if (event && delegate->editType() == QAbstractItemDelegate::NoWidget)
// 	return d->sendItemEvent(data, event);
    return state() == Editing;
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

    QAbstractItemDelegate::EditType editType = itemDelegate()->editType(item);
    if (editType == QAbstractItemDelegate::PersistentWidget || editType == QAbstractItemDelegate::NoWidget) {
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
    if (!d->currentEditor)//|| !item->isEditable() || item->editType() != QAbstractItemDelegate::PersistentWidget)
 	return;
    QItemOptions options;
    getViewOptions(&options);
    options.itemRect = itemViewportRect(currentItem());
    itemDelegate()->updateEditorGeometry(d->currentEditor, options, item);
}

void QAbstractItemView::updateGeometries()
{
    //do nothing
}

bool QAbstractItemView::eventFilter(QObject *object, QEvent *event)
{
    if (object == viewport()) {
  	switch (event->type()) {
  	case QEvent::MouseButtonPress:
	    viewportMousePressEvent((QMouseEvent*)event);
	    if (((QMouseEvent*)event)->isAccepted())
		return true;
	    break;
  	case QEvent::MouseMove:
  	    viewportMouseMoveEvent((QMouseEvent*)event);
	    if (((QMouseEvent*)event)->isAccepted())
		return true;
	    break;
  	case QEvent::MouseButtonRelease:
 	    viewportMouseReleaseEvent((QMouseEvent*)event);
	    if (((QMouseEvent*)event)->isAccepted())
		return true;
	    break;
  	case QEvent::MouseButtonDblClick:
  	    viewportMouseDoubleClickEvent((QMouseEvent*)event);
	    if (((QMouseEvent*)event)->isAccepted())
		return true;
	case QEvent::ContextMenu:
	    viewportContextMenuEvent((QContextMenuEvent*)event);
	    if (((QContextMenuEvent*)event)->isAccepted())
		return true;
	    break;
  	case QEvent::DragEnter:
  	    viewportDragEnterEvent((QDragEnterEvent*)event);
	    break;
	case QEvent::DragMove:
	    viewportDragMoveEvent((QDragMoveEvent*)event);
	    break;
	case QEvent::DragLeave:
	    viewportDragLeaveEvent((QDragLeaveEvent*)event);
	    break;
  	case QEvent::Drop:
 	    viewportDropEvent((QDropEvent*)event);
	    break;
  	default:
  	    break;
 	}
    } else if (object == d->currentEditor) {
	if (event->type() == QEvent::KeyPress) {
	    switch (((QKeyEvent*)event)->key()) {
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
    return QViewport::eventFilter(object, event);
}

int QAbstractItemView::visibleWidth() const
{
    return viewport()->width();
}

int QAbstractItemView::visibleHeight() const
{
    return viewport()->height();
}

QRect QAbstractItemView::visibleRect() const
{
    return QRect(contentsX(), contentsY(), viewport()->width(), viewport()->height());
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

    update();
    startItemsLayout();
}

void QAbstractItemView::contentsInserted(const QModelIndex &, const QModelIndex &)
{
    // do nothing
}

void QAbstractItemView::contentsRemoved(const QModelIndex &, const QModelIndex &, const QModelIndex &)
{
    // do nothing
}

QAbstractItemDelegate *QAbstractItemView::itemDelegate() const
{
    if (!d->delegate)
	d->delegate = new QItemDelegate(model()); // FIXME: memleak
    return d->delegate;
}

void QAbstractItemView::setItemDelegate(QAbstractItemDelegate *delegate)
{
//     if (delegate->model() != model()) {
//  	qWarning( "QAbstractItemView::setDelegate() failed: Trying to set a delegate, "
//  		  "which works on a different model than the view." );
//  	return;
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
		   SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
		   this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
	disconnect(d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
		   this, SLOT(currentChanged(QModelIndex,QModelIndex)));
    }

    d->selectionModel = selectionModel;

    connect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
	    this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
    connect(d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
	    this, SLOT(currentChanged(QModelIndex,QModelIndex)));
}

QItemSelectionModel* QAbstractItemView::selectionModel() const
{
    return d->selectionModel;
}

void QAbstractItemView::selectionChanged(const QItemSelection &deselected, const QItemSelection &selected)
{
    update();
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

    startEdit(current, QAbstractItemDelegate::CurrentChanged, 0);
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
    // FIXME
//     if (!verticalScrollBar()->isSliderDown() &&
// 	verticalScrollBar()->value() == verticalScrollBar()->maximum())
// 	model()->fetchMore();
}

void QAbstractItemView::updateItem(const QModelIndex &item)
{
    updateViewport(itemViewportRect(item));
}

void QAbstractItemView::updateRow(const QModelIndex &item)
{
    QModelIndex parent = model()->parent(item);
    int row = item.row();
    int columns = model()->columnCount(parent);
    QModelIndex left = model()->index(row, 0, parent);
    QModelIndex right = model()->index(row, columns - 1, parent);
    QRect rect = itemViewportRect(left) | itemViewportRect(right);
    updateViewport(rect);
}

void QAbstractItemView::updateViewport(const QRect &)
{
    // will only update if rect is visible
//     if (viewport()->rect().intersects(rect))
// 	viewport()->update(rect);
    viewport()->update(); // FIXME: use rect
}

void QAbstractItemView::clearArea(QPainter *painter, const QRect &rect) const
{
    painter->fillRect(rect, palette().brush(QPalette::Base));
}

bool QAbstractItemView::supportsDragAndDrop() const
{
    return false;
}

QDragObject *QAbstractItemView::dragObject()
{
    QItemViewDragObject *dragObject = new QItemViewDragObject(this);
    QModelIndexList items = selectionModel()->selectedItems();
    dragObject->set(items);
    return dragObject;
}

void QAbstractItemView::startDrag()
{
    QDragObject *obj = dragObject();
    if (!obj)
	return;
//     if (obj->drag() && obj->target() != viewport())
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

/*!
  Returns the SelectionUpdateMode to be used when
  updating selections. Reimplement this function to add your own
  selection update behavior.

  This function is usually called on user input events like mouse and
  keyboard events.
*/

QItemSelectionModel::SelectionUpdateMode QAbstractItemView::selectionUpdateMode(
    ButtonState state,
    const QModelIndex &item,
    QEvent::Type type,
    Key key) const
{
    // ClearAndSelect always on Single selectionmode
    if (selectionMode() == QItemSelectionModel::Single)
	return QItemSelectionModel::ClearAndSelect;

    // Toggle on MouseMove
    if (type == QEvent::MouseMove && state & ControlButton)
	return QItemSelectionModel::ToggleCurrent;

    // NoUpdate when pressing without modifiers on a selected item
    if (type == QEvent::MouseButtonPress &&
	!(d->pressedState & ShiftButton) &&
	!(d->pressedState & ControlButton) &&
	item.isValid() &&
	selectionModel()->isSelected(item))
	return QItemSelectionModel::NoUpdate;

    // ClearAndSelect on MouseButtonRelease if MouseButtonPress on selected item
    if (type == QEvent::MouseButtonRelease &&
	item.isValid() &&
	item == d->pressedItem &&
	!(d->pressedState & ShiftButton) &&
	!(d->pressedState & ControlButton) &&
	selectionModel()->isSelected(item))
	return QItemSelectionModel::ClearAndSelect;
    else if (type == QEvent::MouseButtonRelease)
	return QItemSelectionModel::NoUpdate;

    // NoUpdate on Key movement and Ctrl
    if (type == QEvent::KeyPress &&
	state & ControlButton &&
	(key == Key_Down ||
	 key == Key_Up ||
	 key == Key_Left ||
	 key == Key_Right ||
	 key == Key_Home ||
	 key == Key_End ||
	 key == Key_PageUp ||
	 key == Key_PageDown))
	return QItemSelectionModel::NoUpdate;

    // Toggle on Ctrl-Key_Space, Select on Space
    if (type == QEvent::KeyPress && key == Key_Space) {
	if (state & ControlButton)
	    return QItemSelectionModel::Toggle;
	else
	    return QItemSelectionModel::Select;
    }

    if (state & ShiftButton)
	return QItemSelectionModel::SelectCurrent;
    if (state & ControlButton)
	return QItemSelectionModel::Toggle;
    if (QAbstractItemView::state() == Selecting)
	return QItemSelectionModel::SelectCurrent;
    return QItemSelectionModel::ClearAndSelect;
}

bool QAbstractItemViewPrivate::createEditor(const QModelIndex &item,
					    QAbstractItemDelegate::StartEditAction action,
					    QEvent *event)
{
    QAbstractItemDelegate *delegate = q->itemDelegate();
    QItemOptions options;
    q->getViewOptions(&options);
    options.itemRect = q->itemViewportRect(item);
    QWidget *editor = delegate->createEditor(action, q->viewport(), options, item);
    if (!editor)
	return false;
    editor->show();
    editor->setFocus();
    if (event && (action == QAbstractItemDelegate::AnyKeyPressed || event->type() == QEvent::MouseButtonPress))
	QApplication::sendEvent(editor, event);
    if (delegate->editType(item) != QAbstractItemDelegate::PersistentWidget)
	currentEditor = editor;
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
    options.itemRect = q->itemViewportRect( q->itemToViewItem( data ) );

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

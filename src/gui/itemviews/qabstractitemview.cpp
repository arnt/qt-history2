/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qabstractitemview.h"
#include <qitemdelegate.h>
#include <qpointer.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qbitmap.h>
#include <qpair.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

#include <private/qabstractitemview_p.h>
#define d d_func()
#define q q_func()

QAbstractItemViewPrivate::QAbstractItemViewPrivate()
    :   model(0),
        delegate(0),
        selectionModel(0),
        selectionMode(QAbstractItemView::Extended),
        selectionBehavior(QAbstractItemView::SelectItems),
        state(QAbstractItemView::NoState),
        startEditActions(QAbstractItemDelegate::DoubleClicked
                         |QAbstractItemDelegate::EditKeyPressed),
        inputInterval(400)
{
}

QAbstractItemViewPrivate::~QAbstractItemViewPrivate()
{
}

void QAbstractItemViewPrivate::init()
{
    q->setSelectionModel(new QItemSelectionModel(model, q));

    QObject::connect(model, SIGNAL(contentsChanged(QModelIndex,QModelIndex)),
                     q, SLOT(contentsChanged(QModelIndex,QModelIndex)));
    QObject::connect(model, SIGNAL(contentsInserted(QModelIndex,QModelIndex)),
                     q, SLOT(contentsInserted(QModelIndex,QModelIndex)));
    QObject::connect(model, SIGNAL(contentsRemoved(QModelIndex,QModelIndex,QModelIndex)),
                     q, SLOT(contentsRemoved(QModelIndex,QModelIndex,QModelIndex)));

    q->setHorizontalFactor(256);
    q->setVerticalFactor(256);

    viewport->installEventFilter(q);
    rubberBand = new QRubberBand(QRubberBand::Rectangle, viewport);
    rubberBand->hide();

    // FIXME: this is only true when we have a view that is layed out TopToBottom
    QObject::connect(q->verticalScrollBar(), SIGNAL(sliderReleased()), q, SLOT(fetchMore()));
    QObject::connect(q->verticalScrollBar(), SIGNAL(valueChanged(int)), q, SLOT(fetchMore()));

    QObject::connect(q->verticalScrollBar(), SIGNAL(actionTriggered(int)), q, SLOT(verticalScrollbarAction(int)));
    QObject::connect(q->horizontalScrollBar(), SIGNAL(actionTriggered(int)), q, SLOT(horizontalScrollbarAction(int)));

    QObject::connect(q->verticalScrollBar(), SIGNAL(valueChanged(int)), q, SLOT(updateCurrentEditor()),
                     QueuedConnection);
    QObject::connect(q->horizontalScrollBar(), SIGNAL(valueChanged(int)),q, SLOT(updateCurrentEditor()),
                     QueuedConnection);
    QObject::connect(q, SIGNAL(needMore()), model, SLOT(fetchMore()), QueuedConnection);

    QApplication::postEvent(q, new QMetaCallEvent(QEvent::InvokeSlot,
                               q->metaObject()->indexOfSlot("startItemsLayout()"), q));
}

/*!
  \class QAbstractItemView qgenericitemview.h

  \brief Abstract baseclass for every view working on a QAbstractItemModel

  This subclass of QViewprt provides the base functionality needed
  by every item view which works on a QAbstractItemModel. It handles common
  functionality of editing of items, keyboard and mouse handling, etc.
 Current item and  selections are handled by the QItemSelectionModel.

  A specific view only implements the specific functionality of that
  view, like drawing an item, returning the geometry of an item,
  finding items, etc.
*/

QAbstractItemView::QAbstractItemView(QAbstractItemModel *model, QWidget *parent)
    : QViewport(*(new QAbstractItemViewPrivate), parent)
{
    Q_ASSERT(model)
    d->model = model;
    d->init();
}

QAbstractItemView::QAbstractItemView(QAbstractItemViewPrivate &dd, QAbstractItemModel *model, QWidget *parent)
    : QViewport(dd, parent)
{
    Q_ASSERT(model)
    d->model = model;
    d->init();
}

QAbstractItemView::~QAbstractItemView()
{
}

QAbstractItemModel *QAbstractItemView::model() const
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
    d->selectionModel->clear();
}

void QAbstractItemView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    if (!selectionModel)
        return;

    if (selectionModel->model() != model()) {
        qWarning("QAbstractItemView::setSelectionModel() failed: Trying to set a selection model, "
                  "which works on a different model than the view.");
        return;
    }

    if (d->selectionModel) {
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

    bool block = selectionModel->blockSignals(true);
    if (!selectionModel->currentItem().isValid())
        selectionModel->setCurrentItem(model()->index(0, 0, root()), QItemSelectionModel::NoUpdate);
    selectionModel->blockSignals(block);
}

QItemSelectionModel* QAbstractItemView::selectionModel() const
{
    return d->selectionModel;
}

void QAbstractItemView::setSelectionMode(int mode)
{
    d->selectionMode = mode;
}

int QAbstractItemView::selectionMode() const
{
    return d->selectionMode;
}

void QAbstractItemView::setSelectionBehavior(int behavior)
{
    d->selectionBehavior = behavior;
}

int QAbstractItemView::selectionBehavior() const
{
    return d->selectionBehavior;
}

void QAbstractItemView::setCurrentItem(const QModelIndex &data)
{
    d->selectionModel->setCurrentItem(data, selectionCommand(NoButton));
}

QModelIndex QAbstractItemView::currentItem() const
{
    return d->selectionModel->currentItem();
}

void QAbstractItemView::setRoot(const QModelIndex &index)
{
    d->root = index;
//    if (isVisible())
    startItemsLayout();
    update();
}

QModelIndex QAbstractItemView::root() const
{
    return d->root;
}

void QAbstractItemView::edit(const QModelIndex &index)
{
    if (!index.isValid())
        qWarning("edit: index was invalid");
    if (!startEdit(index, QAbstractItemDelegate::AlwaysEdit, 0))
        qWarning("edit: editing failed");
}

void QAbstractItemView::setStartEditActions(int actions)
{
    d->startEditActions = actions;
}

int QAbstractItemView::startEditActions() const
{
    return d->startEditActions;
}

bool QAbstractItemView::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::ToolTip: {
        if (!isActiveWindow())
            break;
        QPoint pos = static_cast<QHelpEvent*>(e)->globalPos();
        QModelIndex index = itemAt(pos);
        if (!index.isValid())
            break;
        QString tooltip = model()->data(index, QAbstractItemModel::ToolTip).toString();
        QToolTip::showText(pos, tooltip, this);
        return true; }
    case QEvent::WhatsThis: {
        QPoint pos = static_cast<QHelpEvent*>(e)->globalPos();
        QModelIndex index = itemAt(pos);
        if (!index.isValid())
            break;
        QString whatsthis = model()->data(index, QAbstractItemModel::ToolTip).toString();
        QWhatsThis::showText(pos, whatsthis, this);
        return true; }
    case QEvent::StatusTip: {
        QPoint pos = static_cast<QHelpEvent*>(e)->globalPos();
        QModelIndex index = itemAt(pos);
        if (!index.isValid())
            break;
        QString statustip = model()->data(index, QAbstractItemModel::ToolTip).toString();
        if (!statustip.isEmpty())
            setStatusTip(statustip);
        return true; }
    default:
        break;
    }
    return QViewport::event(e);
}

void QAbstractItemView::mousePressEvent(QMouseEvent *e)
{
    QPoint pos = e->pos();
    QModelIndex item = itemAt(pos);

    d->pressedItem = item;
    d->pressedState = e->state();
    d->pressedPosition = pos;
    d->pressedPosition += QPoint(horizontalOffset(), verticalOffset());

    if (item.isValid())
        d->selectionModel->setCurrentItem(item, QItemSelectionModel::NoUpdate);

    QRect rect = d->rubberBand->geometry();
    rect.moveTopLeft(d->viewport->mapFromGlobal(rect.topLeft()));

    if (e->state() & ShiftButton && d->selectionMode != Single)
        rect.setBottomRight(pos); // do not normalize
    else
        rect.setCoords(pos.x(), pos.y(), pos.x(), pos.y());

    setSelection(rect.normalize(), selectionCommand(e->state(), item, e->type()));

    rect.moveTopLeft(d->viewport->mapToGlobal(rect.topLeft()));
    d->rubberBand->setGeometry(rect);

    emit pressed(item, e->button());
}

void QAbstractItemView::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->state() & LeftButton))
        return;
    QPoint bottomRight = e->pos();
    QPoint topLeft;
    if (d->selectionMode != Single) {
        topLeft = d->pressedPosition;
        topLeft -= QPoint(horizontalOffset(), verticalOffset());
    } else {
        topLeft = bottomRight;
    }
    QRect rect = QRect(topLeft, bottomRight).normalize();

    d->rubberBand->setGeometry(QRect(d->viewport->mapToGlobal(topLeft),
                                     d->viewport->mapToGlobal(bottomRight)).normalize());

    if (state() == Dragging && // the user has already started moving the mouse
        (topLeft - bottomRight).manhattanLength() > QApplication::startDragDistance()) {
        startDrag();
        setState(NoState); // the startDrag will return when the dnd operation is done
        return;
    }

    QModelIndex item = itemAt(bottomRight);
    if (currentItem() == item && state() == Selecting)
        return; // we haven't moved over another item yet

    if (item.isValid()) {
        if (state() != Selecting) {
            bool dnd = model()->isDragEnabled(item) && supportsDragAndDrop();
            bool selected = d->selectionModel->isSelected(item);
            if (dnd && selected) {
                setState(Dragging);
                return;
            }
        }
        d->selectionModel->setCurrentItem(item, QItemSelectionModel::NoUpdate);
    }
    setState(Selecting);
    setSelection(rect, selectionCommand(e->state(), item, e->type()));
    d->rubberBand->show();
}

void QAbstractItemView::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint pos = e->pos();
    QModelIndex item = itemAt(pos);
    d->selectionModel->select(item, selectionCommand(e->state(), item, e->type()));
    d->rubberBand->hide();
    setState(NoState);
    if (item == d->pressedItem)
        emit clicked(item, e->button());
}

void QAbstractItemView::mouseDoubleClickEvent(QMouseEvent *e)
{
    QModelIndex item = itemAt(e->pos());
    if (!item.isValid())
        return;
    emit doubleClicked(item, e->button());
    startEdit(item, QAbstractItemDelegate::DoubleClicked, e);
}

void QAbstractItemView::dragEnterEvent(QDragEnterEvent *e)
{
    if (model()->canDecode(e))
        e->accept();
}

void QAbstractItemView::dropEvent(QDropEvent *e)
{
    QModelIndex index = itemAt(e->pos());
    if (model()->decode(e, (index.isValid() ? index : root())))
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
            QRect rect = d->rubberBand->geometry();
            rect.moveTopLeft(d->viewport->mapFromGlobal(rect.topLeft()));
            int command = selectionCommand(e->state(), newCurrent, e->type(), (Key)e->key());
            if (e->state() & ShiftButton && d->selectionMode != Single) {
                rect.setBottomRight(itemViewportRect(newCurrent).bottomRight());
                d->selectionModel->setCurrentItem(newCurrent, QItemSelectionModel::NoUpdate);
                setSelection(rect.normalize(), command);
            } else if (e->state() & ControlButton && d->selectionMode != Single) {
                rect = itemViewportRect(newCurrent);
                d->selectionModel->setCurrentItem(newCurrent, QItemSelectionModel::NoUpdate);
            } else {
                rect = itemViewportRect(newCurrent);
                d->selectionModel->setCurrentItem(newCurrent, command);
            }
            rect.moveTopLeft(d->viewport->mapToGlobal(rect.topLeft()));
            d->rubberBand->setGeometry(rect);
            return;
        }
    }

    switch (e->key()) {
        // keys to ignore
    case Key_Down:
    case Key_Up:
    case Key_Left:
    case Key_Right:
    case Key_Home:
    case Key_End:
    case Key_PageUp:
    case Key_PageDown:
    case Key_Escape:
    case Key_Enter:
    case Key_Shift:
    case Key_Control:
        break;
    case Key_Return:
        emit returnPressed(currentItem());
        e->accept();
        return;
    case Key_Space:
        d->selectionModel->select(currentItem(), selectionCommand(e->state(), currentItem(), e->type(),(Key)e->key()));
        emit spacePressed(currentItem());
        e->accept();
        return;
    case Key_Delete:
        emit deletePressed(currentItem());
        e->accept();
        return;
    case Key_F2:
        if (startEdit(currentItem(), QAbstractItemDelegate::EditKeyPressed, e))
            return;
        break;
    default:
        if (!e->text().isEmpty()) {
            if (startEdit(currentItem(), QAbstractItemDelegate::AnyKeyPressed, e)) {
                return;
            } else {
                keyboardSearch(e->text());
                return;
            }
        }
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

bool QAbstractItemView::startEdit(const QModelIndex &index,
                                  QAbstractItemDelegate::StartEditAction action,
                                  QEvent *event)
{
    QAbstractItemDelegate::EditType editType = d->delegate->editType(index);
    if (d->shouldEdit(action, index) || editType == QAbstractItemDelegate::PersistentWidget) {
        if (editType == QAbstractItemDelegate::NoWidget && d->delegate->event(event, index))
            d->state = Editing;
        else if (d->createEditor(action, event, index))
            d->state = Editing;
    }
    return d->state == Editing;
}

void QAbstractItemView::endEdit(const QModelIndex &index, bool accept)
{
    if (!index.isValid())
        return ;

    setState(NoState);

    QAbstractItemDelegate::EditType editType = d->delegate->editType(index);
    if (editType == QAbstractItemDelegate::PersistentWidget
        || editType == QAbstractItemDelegate::NoWidget) {
        // No editor to remove
        setFocus();
        return;
    }

    if (accept) {
        d->delegate->setContentFromEditor(d->currentEditor, index);
        d->delegate->removeEditor(QAbstractItemDelegate::Accepted,
                                  d->currentEditor, index);
    } else {
        d->delegate->removeEditor(QAbstractItemDelegate::Cancelled,
                                  d->currentEditor, index);
    }
    setFocus();
}

QWidget *QAbstractItemView::currentEditor() const
{
    if (d->state == Editing)
        return d->currentEditor;
    return 0;
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

void QAbstractItemView::verticalScrollbarAction(int)
{
    //do nothing
}

void QAbstractItemView::horizontalScrollbarAction(int)
{
    //do nothing
}

bool QAbstractItemView::eventFilter(QObject *object, QEvent *event)
{
    if (object == d->currentEditor && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
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
    return false;
}

/*!
  Moves to and selects the item best matching the string \a search. If no item is found nothing happens.
*/
void QAbstractItemView::keyboardSearch(const QString &search) {
    QModelIndex start = currentItem().isValid() ? currentItem() : model()->index(0, 0);
    QTime now(QTime::currentTime());
    bool skipRow = false;
    if (d->keyboardInputTime.msecsTo(now) > keyboardInputInterval()) {
        d->keyboardInput = search;
        skipRow = true;
    } else {
        d->keyboardInput += search;
    }
    d->keyboardInputTime = now;

    // special case for searches with same key like 'aaaaa'
    bool sameKey = false;
    if (d->keyboardInput.length() > 1) {
        sameKey = d->keyboardInput.count(d->keyboardInput.at(d->keyboardInput.length() - 1)) ==
                  d->keyboardInput.length();
        if (sameKey)
            skipRow = true;
    }

    // skip if we are searching for the same key or a new search started
    if (skipRow) {
        int newRow = (start.row() < model()->rowCount(model()->parent(start)) - 1) ?
                     start.row() + 1 : 0;
        start = model()->index(newRow,
                               start.column(),
                               model()->parent(start),
                               start.type());
    }

    // search from start, if that fails search from row 0
    QString searchString = sameKey ? QString(d->keyboardInput.at(0)) : d->keyboardInput;
    QModelIndexList match;
    match = model()->match(start, QAbstractItemModel::Display, searchString, 1);
    if (start.row() > 0 && (match.isEmpty() || !match.at(0).isValid()))
        match = model()->match(model()->index(0,
                                              start.column(),
                                              model()->parent(start),
                                              start.type()),
                          QAbstractItemModel::Display, searchString, 1);

    if (!match.isEmpty() && match.at(0).isValid()) {
        setCurrentItem(match.at(0));
    }
}

void QAbstractItemView::updateItem(const QModelIndex &item)
{
    if (!isVisible())
        return;
    QRect rect = itemViewportRect(item);
    if (rect.isValid())
        d->viewport->update(itemViewportRect(item));
}

void QAbstractItemView::updateRow(const QModelIndex &item)
{
    QModelIndex parent = model()->parent(item);
    int row = item.row();
    int columns = model()->columnCount(parent);
    QModelIndex left = model()->index(row, 0, parent);
    QModelIndex right = model()->index(row, columns - 1, parent);
    QRect rect = itemViewportRect(left) | itemViewportRect(right);
    d->viewport->update(rect);
}

/*!
    \property QAbstractItemView::keyboardInputInterval
    \brief the interval threshold for doing keyboard searches.
*/
void QAbstractItemView::setKeyboardInputInterval(int msec)
{
    if (msec >= 0)
        d->inputInterval = msec;
}

int QAbstractItemView::keyboardInputInterval() const
{
    return d->inputInterval;
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
    // NOTE: if root() was a valid QModelIndex, it may have been invalidated
}

void QAbstractItemView::contentsRemoved(const QModelIndex &, const QModelIndex &, const QModelIndex &)
{
    // do nothing
    // NOTE: if root() was a valid QModelIndex, it may have been invalidated
}

QAbstractItemDelegate *QAbstractItemView::itemDelegate() const
{
    if (!d->delegate)
        d->delegate = new QItemDelegate(d->model, const_cast<QAbstractItemView *>(this));
    return d->delegate;
}

void QAbstractItemView::setItemDelegate(QAbstractItemDelegate *delegate)
{
    if (delegate->model() != model()) {
         qWarning("QAbstractItemView::setDelegate() failed: Trying to set a delegate, "
                   "which works on a different model than the view.");
         return;
    }
    if (d->delegate && d->delegate->parent() == this)
        delete d->delegate;
    d->delegate = delegate;
}

void QAbstractItemView::selectionChanged(const QItemSelection &deselected, const QItemSelection &selected)
{
    QRect deselectedRect = selectionViewportRect(deselected);
    QRect selectedRect = selectionViewportRect(selected);
    d->viewport->update(deselectedRect.unite(selectedRect));
}

void QAbstractItemView::currentChanged(const QModelIndex &old, const QModelIndex &current)
{
    if (d->currentEditor)
        endEdit(old, true);

    // FIXME: calling ensureItemVisible first before redrawing oldRect
    // make the view look slow, but since repaintItem does not update the rect
    // immediately we get drawing errors because the contentview might be shifted

    if (current.isValid() && isVisible())
        ensureItemVisible(current);

    if (old.isValid())
        updateItem(old);
    if (current.isValid()) {
        updateItem(current);
        // FIXME: the QWidget::scroll() will sometimes blit before we get the chance to repaint the old item
        //qApp->processEvents(); // force paint events to be processed
        startEdit(current, QAbstractItemDelegate::CurrentChanged, 0);
    }
}

void QAbstractItemView::startItemsLayout()
{
    while (!doItemsLayout(100))
            qApp->processEvents();
}

bool QAbstractItemView::doItemsLayout(int)
{
    return true; // Do nothing
}

void QAbstractItemView::fetchMore()
{
    // FIXME
    if (!verticalScrollBar()->isSliderDown() &&
        verticalScrollBar()->value() == verticalScrollBar()->maximum())
        model()->fetchMore();
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
    QModelIndexList items = d->selectionModel->selectedItems();
    return model()->dragObject(items, this);
}

void QAbstractItemView::startDrag()
{
    QDragObject *obj = dragObject();
    if (!obj)
        return;
//     if (obj->drag() && obj->target() != viewport())
//         emit moved();
    obj->drag();
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

/*!
  Returns the SelectionCommand to be used when
  updating selections. Reimplement this function to add your own
  selection behavior.

  This function is called on user input events like mouse and
  keyboard events.
*/
int QAbstractItemView::selectionCommand(ButtonState state,
                                        const QModelIndex &item,
                                        QEvent::Type type,
                                        Key key) const
{
    int behavior = 0;
    switch (d->selectionBehavior) {
    case SelectRows:
        behavior = QItemSelectionModel::Rows;
        break;
    case SelectColumns:
        behavior = QItemSelectionModel::Columns;
        break;
    }

    // Single: ClearAndSelect on valid index otherwise NoUpdate
    if (selectionMode() == Single) {
        if (item.isValid()) {
            return QItemSelectionModel::ClearAndSelect | behavior;
        } else {
            return QItemSelectionModel::NoUpdate;
        }
    }

    if (selectionMode() == Multi) {
        // NoUpdate on Key movement and Ctrl
        if (type == QEvent::KeyPress &&
            (key == Key_Down ||
             key == Key_Up ||
             key == Key_Left ||
             key == Key_Right ||
             key == Key_Home ||
             key == Key_End ||
             key == Key_PageUp ||
             key == Key_PageDown))
            return QItemSelectionModel::NoUpdate;

        // Select/Deselect on Space
        if (type == QEvent::KeyPress && item.isValid() && key == Key_Space) {
            if (d->selectionModel->isSelected(item))
                return QItemSelectionModel::Deselect | behavior;
            else
                return QItemSelectionModel::Select | behavior;
        }

        // Select/Deselect on MouseButtonPress
        if (type == QEvent::MouseButtonPress && item.isValid()) {
            if (d->selectionModel->isSelected(item))
                return QItemSelectionModel::Deselect | behavior;
            else
                return QItemSelectionModel::Select | behavior;
        }

        // Select/Deselect on MouseMove
        if (type == QEvent::MouseMove && item.isValid()) {
            if (d->selectionModel->isSelected(item))
                return QItemSelectionModel::Deselect | behavior;
            else
                return QItemSelectionModel::Select | behavior;
        }
        return QItemSelectionModel::NoUpdate;
    }

    // Toggle on MouseMove
    if (type == QEvent::MouseMove && state & ControlButton)
        return QItemSelectionModel::ToggleCurrent | behavior;

    // NoUpdate when pressing without modifiers on a selected item
    if (type == QEvent::MouseButtonPress &&
        !(d->pressedState & ShiftButton) &&
        !(d->pressedState & ControlButton) &&
        item.isValid() &&
        d->selectionModel->isSelected(item))
        return QItemSelectionModel::NoUpdate;

    // Clear on MouseButtonPress on non-valid item with no modifiers and not RightButton
    if (type == QEvent::MouseButtonPress &&
        !item.isValid() &&
        !(state & RightButton) &&
        !(state & ShiftButton) &&
        !(state & ControlButton))
        return QItemSelectionModel::Clear;

    // ClearAndSelect on MouseButtonRelease if MouseButtonPress on selected item
    if (type == QEvent::MouseButtonRelease &&
        item.isValid() &&
        item == d->pressedItem &&
        !(d->pressedState & ShiftButton) &&
        !(d->pressedState & ControlButton) &&
        d->selectionModel->isSelected(item))
        return QItemSelectionModel::ClearAndSelect | behavior;
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
            return QItemSelectionModel::Toggle | behavior;
        else
            return QItemSelectionModel::Select | behavior;
    }

    if (state & ShiftButton)
        return QItemSelectionModel::SelectCurrent | behavior;
    if (state & ControlButton)
        return QItemSelectionModel::Toggle | behavior;
    if (QAbstractItemView::state() == Selecting)
        return QItemSelectionModel::SelectCurrent | behavior;
    return QItemSelectionModel::ClearAndSelect | behavior;
}

QWidget *QAbstractItemViewPrivate::createEditor(QAbstractItemDelegate::StartEditAction action,
                                                QEvent *event, const QModelIndex &index)
{
    QWidget *editor = 0;
    bool persistent = delegate->editType(index) == QAbstractItemDelegate::PersistentWidget;
    if (persistent)
        editor = persistentEditor(index);
    if (!editor) {
        QItemOptions options;
        q->getViewOptions(&options);
        options.itemRect = q->itemViewportRect(index);
        options.focus = (index == q->currentItem());
        editor = delegate->createEditor(action, viewport, options, index);
    }
    if (editor) {
        editor->show();
        editor->setFocus();
        if (event && (action == QAbstractItemDelegate::AnyKeyPressed || event->type() == QEvent::MouseButtonPress))
            QApplication::sendEvent(editor, event);
        if (persistent) {
            setPersistentEditor(editor, index);
        } else {
            currentEditor = editor;
            currentEditor->installEventFilter(q);
            editItem = index;
        }
    }
    return editor;
}

QWidget *QAbstractItemViewPrivate::persistentEditor(const QModelIndex &index) const
{
    QList<QPair<QModelIndex, QWidget*> >::ConstIterator it = persistentEditors.begin();
    for (; it != persistentEditors.end(); ++it) {
        if ((*it).first == index)
            return (*it).second;
    }
    return 0;
}

void QAbstractItemViewPrivate::setPersistentEditor(QWidget *editor, const QModelIndex &index)
{
    persistentEditors.append(QPair<QModelIndex, QWidget*>(index, editor));
}

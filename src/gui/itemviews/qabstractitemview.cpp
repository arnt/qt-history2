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
#include <qmenu.h>
#include <qevent.h>
#include <qeventloop.h>
#include <qscrollbar.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qrubberband.h>

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
        inputInterval(400),
        autoScroll(true),
        autoScrollTimer(0),
        autoScrollMargin(16),
        autoScrollInterval(50),
        autoScrollCount(0)
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
    QObject::connect(model, SIGNAL(contentsRemoved(QModelIndex,QModelIndex)),
                     q, SLOT(contentsRemoved(QModelIndex,QModelIndex)));

    q->setHorizontalFactor(256);
    q->setVerticalFactor(256);

    viewport->installEventFilter(q);
    QObject::connect(q->verticalScrollBar(), SIGNAL(sliderReleased()), q, SLOT(fetchMore()));
    QObject::connect(q->verticalScrollBar(), SIGNAL(valueChanged(int)), q, SLOT(fetchMore()));
    QObject::connect(q->verticalScrollBar(), SIGNAL(actionTriggered(int)),
                     q, SLOT(verticalScrollbarAction(int)));
    QObject::connect(q->horizontalScrollBar(), SIGNAL(actionTriggered(int)),
                     q, SLOT(horizontalScrollbarAction(int)));
    QObject::connect(q->verticalScrollBar(), SIGNAL(valueChanged(int)),
                     q, SLOT(updateCurrentEditor()), QueuedConnection);
    QObject::connect(q->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                     q, SLOT(updateCurrentEditor()), QueuedConnection);
    QObject::connect(q, SIGNAL(needMore()), model, SLOT(fetchMore()), QueuedConnection);

    QApplication::postEvent(q, new QMetaCallEvent(QEvent::InvokeSlot,
                               q->metaObject()->indexOfSlot("doItemsLayout()"), q));

    q->setAttribute(WA_PaintOnScreen);
    q->setAttribute(WA_NoBackground);

    viewport->setAttribute(WA_PaintOnScreen);
    viewport->setAttribute(WA_NoBackground);
    viewport->setAttribute(WA_NoSystemBackground);
}

/*!
    \class QAbstractItemView qabstractitemview.h

    \brief The QAbstractItemView class is the base class for every
    view that uses a QAbstractItemModel.

    This class is a QViewport subclass that provides all the
    functionality common to all views, such as keyboard and mouse
    support for editing items, scrolling, and selection control; (but
    note that selections are handled separately by the
    QItemSelectionModel class).

    The view classes that inherit QAbstractItemView only need
    to implement their own view-specific functionality, such as
    drawing items, returning the geometry of items, finding items,
    etc.

    QAbstractItemView provides common slots such as edit() and
    setCurrentItem(), and common signals such as clicked(),
    doubleClicked(), returnPressed(), spacePressed(), and
    deletePressed(). Many protected slots are also provided, including
    contentsChanged(), contentsInserted(), contentsRemoved(),
    selectionChanged(), and currentChanged().

    The ``root'' item is returned by root(), and the current item by
    currentItem(). To make sure that an item is visible use
    ensureItemVisible().

    Some of QAbstractItemView's functions are concerned with
    scrolling, for example setHorizontalFactor() and
    setVerticalFactor(). Several other functions are concerned with
    selection control, for example, setSelectionModel(),
    setSelectionMode(), and setSelectionBehavior().

    For complete control over the display and editing of items you can
    specify a delegate with setItemDelegate().

    QAbstractItemView provides a lot of protected functions. Some are
    concerned with editing, for example, startEdit(), endEdit(), and
    currentEditor(), whilst others are keyboard and mouse event
    handlers.

*/

/*!
    \enum QAbstractItemView::SelectionMode

    \value Single
    \value Multi
    \value Extended
*/

/*!
    \enum QAbstractItemView::SelectionBehavior

    \value SelectItems
    \value SelectRows
    \value SelectColumns
*/

/*!
    \enum QAbstractItemView::CursorAction

    \value MoveUp
    \value MoveDown
    \value MoveLeft
    \value MoveRight
    \value MoveHome
    \value MoveEnd
    \value MovePageUp
    \value MovePageDown
*/

/*!
    \enum QAbstractItemView::State

    \value NoState
    \value Dragging
    \value Selecting
    \value Editing
    \value Opening
    \value Closing
*/


/*!
    \fn QRect QAbstractItemView::itemViewportRect(const QModelIndex &index) const = 0
    Returns the rectangle on the viewport occupied by the item at \a
    index.

    In the base class this is a pure virtual function.
*/


/*!
    \fn void QAbstractItemView::ensureItemVisible(const QModelIndex &index) = 0

    Scrolls the view if necessary to ensure that the item at \a index
    is visible.

    In the base class this is a pure virtual function.
*/


/*!
    \fn QModelIndex QAbstractItemView::itemAt(const QPoint &p) const

    \overload

    Returns the model index of the item at point \a p.

    In the base class this is built on the other itemAt() function,
    which is pure virtual.
*/


/*!
    \fn QModelIndex QAbstractItemView::itemAt(int x, int y) const = 0

    Returns the model index of the item at point \a(x, y).

    In the base class this is a pure virtual function.
*/

/*!
    \fn void QAbstractItemView::needMore()

    \internal
*/

/*!
    \fn void QAbstractItemView::pressed(const QModelIndex &index, int button)

    This signal is emitted when a mouse button is pressed. The button
    is given by \a button (see \l{Qt::ButtonState}), and the item the
    mouse was pressed on is given by \a index (which may be invalid if
    the mouse was not pressed on an item).
*/

/*!
    \fn void QAbstractItemView::clicked(const QModelIndex &index, int button)

    This signal is emitted when a mouse button is clicked. The button
    is given by \a button (see \l{Qt::ButtonState}), and the item the
    mouse was clicked on is given by \a index (which may be invalid if
    the mouse was not clicked on an item).
*/

/*!
    \fn void QAbstractItemView::doubleClicked(const QModelIndex &index, int button)

    This signal is emitted when a mouse button is double-clicked. The
    button is given by \a button (see \l{Qt::ButtonState}), and the
    item the mouse was double-clicked on is given by \a index (which
    may be invalid if the mouse was not double-clicked on an item).
*/

/*!
    \fn void QAbstractItemView::returnPressed(const QModelIndex &index)

    This signal is emitted when Return (or Enter) is pressed. The item
    that the key press was made on is given by \a index.
*/

/*!
    \fn void QAbstractItemView::spacePressed(const QModelIndex &index)

    This signal is emitted when Space is pressed. The item that the
    key press was made on is given by \a index.
*/

/*!
    \fn void QAbstractItemView::deletePressed(const QModelIndex &index)

    This signal is emitted when the Delete key is pressed. The item
    that the key press was made on is given by \a index.
*/

/*!
    \fn void QAbstractItemView::aboutToShowContextMenu(QMenu *menu, const QModelIndex &index)

    This signal is emitted when the context menu is invoked. The \a
    menu is an empty menu; if you populate it with actions it will be
    popped up for the user. The current item when the contex menu
    event occurred is given by \a index.
*/

/*!
    \fn QModelIndex QAbstractItemView::moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state) = 0

    Moves the cursor in the view in accordance with the given \a
    cursorAction and button \a state.
*/


/*!
    \fn int QAbstractItemView::horizontalOffset() const = 0

    Returns the horizontal offset of the view.

    In the base class this is a pure virtual function.
*/

/*!
    \fn int QAbstractItemView::verticalOffset() const = 0

    Returns the vertical offset of the view.

    In the base class this is a pure virtual function.
*/


/*!
    \fn void QAbstractItemView::setSelection(const QRect &rect, int command) = 0

    Applies the selection \a command to the items in or touched by the
    rectangle, \a rect.

    \sa selectionCommand()
*/

/*!
    \fn QRect QAbstractItemView::selectionViewportRect(const QItemSelection &selection) const = 0

    Returns the rectangle from the viewport of the items in the given
    \a selection.
*/

/*!
    \fn int QAbstractItemView::selectionCommand(ButtonState state, const QModelIndex &index, QEvent::Type type, Qt::Key key) const

    Returns a ``selection command'' based on the given button \a
    state, model item \a index, event \a type, and \a key.
*/


/*!
    Creates a new QAbstractItemView to view the \a model, and with
    parent \a parent.
*/
QAbstractItemView::QAbstractItemView(QAbstractItemModel *model, QWidget *parent)
    : QViewport(*(new QAbstractItemViewPrivate), parent)
{
    Q_ASSERT(model);
    d->model = model;
    d->init();
}

/*!
    \internal
*/
QAbstractItemView::QAbstractItemView(QAbstractItemViewPrivate &dd, QAbstractItemModel *model, QWidget *parent)
    : QViewport(dd, parent)
{
    Q_ASSERT(model);
    d->model = model;
    d->init();
}

/*!
    Destroys the view.
*/
QAbstractItemView::~QAbstractItemView()
{
}

/*!
    Returns the model that this view is presenting. The model is set
    in the constructor.
*/
QAbstractItemModel *QAbstractItemView::model() const
{
    return d->model;
}

/*!
    Sets the horizontal scrollbar's stepping factor to \a factor.

    \sa horizontalFactor() setVerticalFactor()
*/
void QAbstractItemView::setHorizontalFactor(int factor)
{
    d->horizontalFactor = factor;
    horizontalScrollBar()->setSingleStep(factor);
}

/*!
    Returns the horizontal scrollbar's stepping factor.

    \sa setHorizontalFactor() verticalFactor()
*/
int QAbstractItemView::horizontalFactor() const
{
    return d->horizontalFactor;
}

/*!
    Sets the vertical scrollbar's stepping factor to \a factor.

    \sa verticalFactor() setHorizontalFactor()
*/
void QAbstractItemView::setVerticalFactor(int factor)
{
    d->verticalFactor = factor;
    verticalScrollBar()->setSingleStep(factor);
}

/*!
    Returns the vertical scrollbar's stepping factor.

    \sa setVerticalFactor() horizontalFactor()
*/
int QAbstractItemView::verticalFactor() const
{
    return d->verticalFactor;
}

/*!
    Clears the selection.

    \sa setSelectionModel()
*/
void QAbstractItemView::clearSelections()
{
    d->selectionModel->clear();
}

/*!
    Sets the current selection to be \a selectionModel.

    \sa selectionModel() clearSelections()
*/
void QAbstractItemView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    if (!selectionModel)
        return;

    if (selectionModel->model() != model()) {
        qWarning("QAbstractItemView::setSelectionModel() failed: Trying to set a selection model,"
                  " which works on a different model than the view.");
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

// ### DOC: Couldn't we call this selection() (and setSelection() and clearSelection()) ?
/*!
    Returns the current selection.

    \sa setSelectionModel() clearSelections()
*/
QItemSelectionModel* QAbstractItemView::selectionModel() const
{
    return d->selectionModel;
}

/*!
    Sets the \l{SelectionMode} flags to \a mode.

    \sa selectionMode() SelectionBehavior
*/
void QAbstractItemView::setSelectionMode(int mode)
{
    d->selectionMode = mode;
}

/*!
    Returns the \l{SelectionMode} flags.

    \sa setSelectionMode() SelectionBehavior
*/
int QAbstractItemView::selectionMode() const
{
    return d->selectionMode;
}

/*!
    Sets the \l{SelectionBehavior} flags to \a behavior.

    \sa selectionBehavior() SelectionMode
*/
void QAbstractItemView::setSelectionBehavior(int behavior)
{
    d->selectionBehavior = behavior;
}

/*!
    Returns the \l{SelectionBehavior} flags.

    \sa setSelectionBehavior() SelectionMode
*/
int QAbstractItemView::selectionBehavior() const
{
    return d->selectionBehavior;
}

/*!
    Sets the current item to be the item at \a index.

    \sa currentItem()
*/
void QAbstractItemView::setCurrentItem(const QModelIndex &index)
{
    d->selectionModel->setCurrentItem(index, selectionCommand(NoButton, index));
}

/*!
    Returns the model index of the current item.

    \sa setCurrentItem()
*/
QModelIndex QAbstractItemView::currentItem() const
{
    return d->selectionModel->currentItem();
}

/*!
    Sets the ``root'' item to the item at \a index.

    \sa root()
*/
void QAbstractItemView::setRoot(const QModelIndex &index)
{
    d->root = index;
    doItemsLayout();
}

/*!
    Returns the model index of the model's ``root'' item.

    \sa setRoot()
*/
QModelIndex QAbstractItemView::root() const
{
    return d->root;
}

/*!
    Calls startEdit() for the item at \a index.
*/
void QAbstractItemView::edit(const QModelIndex &index)
{
    if (!index.isValid())
        qWarning("edit: index was invalid");
    if (!startEdit(index, QAbstractItemDelegate::AlwaysEdit, 0))
        qWarning("edit: editing failed");
}

// ### DOC: Guessed
/*!
    \internal
*/
void QAbstractItemView::doItemsLayout()
{
    d->viewport->update();
    // do nothing
}

// ### DOC: Guessed
/*!
    \internal
*/
void QAbstractItemView::setStartEditActions(int actions)
{
    d->startEditActions = actions;
}

// ### DOC: Guessed
/*!
    \internal
*/
int QAbstractItemView::startEditActions() const
{
    return d->startEditActions;
}

/*!
    \property QAbstractItemView::autoScroll
    \brief whether autoscrolling in drag move events is enabled

    If this property is set to true (the default), the
    QAbstractItemView automatically scrolls the contents of the view
    if the user drags past the left or bottom edge. This only works if
    the viewport accepts drops. Autoscroll is switched off by setting
    the property to false.
*/
void QAbstractItemView::setAutoScroll(bool b)
{
    d->autoScroll = b;
}

bool QAbstractItemView::autoScroll() const
{
    return d->autoScroll;
}

/*!
    \fn bool QAbstractItemView::event(QEvent *event)

    This function is used to handle tool tips, status tips, and What's
    This? mode, if the given \a event is a QEvent::ToolTip, a
    QEvent::WhatsThis, or a QEvent::StatusTip. It passes all other
    events on to its base class event() handler.
*/
bool QAbstractItemView::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::ToolTip: {
        if (!isActiveWindow())
            break;
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        if (!he)
            break;
        QModelIndex index = itemAt(he->pos());
        if (!index.isValid())
            break;
        QString tooltip = model()->data(index, QAbstractItemModel::ToolTip).toString();
        QToolTip::showText(he->globalPos(), tooltip, this);
        return true; }
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        QModelIndex index = itemAt(he->pos());
        if (!index.isValid())
            break;
        QString whatsthis = model()->data(index, QAbstractItemModel::ToolTip).toString();
        QWhatsThis::showText(he->globalPos(), whatsthis, this);
        return true; }
    case QEvent::StatusTip: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        QModelIndex index = itemAt(he->pos());
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

/*!
    This function is called when a mouse event \a e occurs. If a valid
    item is pressed on it is made into the current item. This function
    emits the pressed() signal.
*/
void QAbstractItemView::mousePressEvent(QMouseEvent *e)
{
    QPoint pos = e->pos();
    QModelIndex index = itemAt(pos);

    d->pressedItem = index;
    d->pressedState = e->state();
    d->pressedPosition = pos + QPoint(horizontalOffset(), verticalOffset());

    if (index.isValid())
        d->selectionModel->setCurrentItem(index, QItemSelectionModel::NoUpdate);

    QRect rect(pos, pos);
    setSelection(rect.normalize(), selectionCommand(e->state(), index, e->type()));
    emit pressed(index, e->button());
}

/*!
    This function is called when a mouse move event \a e occurs. If a
    selection is in progress and new items are moved over the
    selection is extended; if a drag is in progress it is continued.
*/
void QAbstractItemView::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->state() & LeftButton))
        return;
    QPoint topLeft;
    QPoint bottomRight = e->pos();
    if (d->selectionMode != Single)
        topLeft = d->pressedPosition - QPoint(horizontalOffset(), verticalOffset());
    else
        topLeft = bottomRight;
    if (state() == Dragging && // the user has already started moving the mouse
        (topLeft - bottomRight).manhattanLength() > QApplication::startDragDistance()) {
        startDrag();
        setState(NoState); // the startDrag will return when the dnd operation is done
        stopAutoScroll();
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
    setSelection(QRect(topLeft, bottomRight).normalize(),
                 selectionCommand(e->state(), item, e->type()));
}

/*!
    This function is called when a mouse release event \a e
    occurs. It will emit the clicked() signal if an item was being
    pressed and will send a context menu event if it is a right-mouse
    button release.

    \sa contextMenuEvent()
*/
void QAbstractItemView::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint pos = e->pos();
    QModelIndex index = itemAt(pos);
    d->selectionModel->select(index, selectionCommand(e->state(), index, e->type()));
    setState(NoState);
    if (index == d->pressedItem)
        emit clicked(index, e->button());
    if (e->button() == RightButton) {
        QContextMenuEvent me(QContextMenuEvent::Mouse, pos, e->state());
        QApplication::sendEvent(this, &me);
    }
}

/*!
    This function is called when a mouse double-click event \a e
    occurs. If the double-click is on a valid item it emits the
    doubleClicked() signal and calls startEdit() on the item.
*/
void QAbstractItemView::mouseDoubleClickEvent(QMouseEvent *e)
{
    QModelIndex item = itemAt(e->pos());
    if (!item.isValid())
        return;
    emit doubleClicked(item, e->button());
    startEdit(item, QAbstractItemDelegate::DoubleClicked, e);
}

/*!
    This function is called when context menu event \a e occurs. It
    emits the aboutToShowContextMenu() signal with a pointer to a
    QMenu object; if this QMenu object is populated with actions (e.g.
    in a slot you've connected to the aboutToShowContextMenu()
    signal), then the menu is shown.
*/
void QAbstractItemView::contextMenuEvent(QContextMenuEvent *e)
{
    QPoint position = e->pos();
    QModelIndex index = itemAt(position);
    QMenu contextMenu(this);
    emit aboutToShowContextMenu(&contextMenu, index);
    if (contextMenu.actions().count() > 0)
        contextMenu.exec(mapToGlobal(position));
}

/*!
    This function is called when drag enter event \a e occurs. If the
    drag is over a valid dropping place (e.g. over an item that
    accepts drops), the event is accepted.

    \sa dropEvent() startDrag()
*/
void QAbstractItemView::dragEnterEvent(QDragEnterEvent *e)
{
    if (model()->canDecode(e))
        e->accept();
}

/*!
    This function is called when drag move event \a e occurs. It can
    cause the view to scroll, for example if the user drags a
    selection to view's right or bottom edge.

    \sa dropEvent() startDrag()
*/
void QAbstractItemView::dragMoveEvent(QDragMoveEvent *e)
{
    if (!model()->canDecode(e)) {
        e->ignore();
        return;
    }
    e->accept();

    if (d->shouldAutoScroll(e->pos()))
        startAutoScroll();
}

/*!
    This function is called when drop event \a e occurs. If there's a
    valid item under the mouse pointer when the drop occurs, the drop
    is accepted.

    \sa startDrag() itemAt()
*/
void QAbstractItemView::dropEvent(QDropEvent *e)
{
    QModelIndex index = itemAt(e->pos());
    if (model()->decode(e, (index.isValid() ? index : root())))
        e->accept();
}

/*!
    This function is called when focus event \a e occurs and is a
    focus in event.
*/
void QAbstractItemView::focusInEvent(QFocusEvent *e)
{
    QViewport::focusInEvent(e);
    QModelIndex item = currentItem();
    if (item.isValid())
        d->viewport->update(itemViewportRect(item));
}

/*!
    This function is called when focus event \a e occurs and is a
    focus out event.
*/
void QAbstractItemView::focusOutEvent(QFocusEvent *e)
{
    QViewport::focusOutEvent(e);
    QModelIndex item = currentItem();
    if (item.isValid())
        d->viewport->update(itemViewportRect(item));
}

/*!
    This function is called when a key event \a e occurs. It handles
    basic cursor movement, e.g. Up, Down, Left, Right, Home, PageUp,
    and PageDown, and emits the returnPressed(), spacePressed(), and
    deletePressed() signals is the associated key is pressed. This
    function is where editing is initiated by key press, e.g. if F2 is
    pressed.

    \sa startEdit()
*/
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
            QPoint offset(horizontalOffset(), verticalOffset());
            int command = selectionCommand(e->state(), newCurrent, e->type(), (Key)e->key());
            if (e->state() & ShiftButton && d->selectionMode != Single) {
                d->selectionModel->setCurrentItem(newCurrent, QItemSelectionModel::NoUpdate);
                QRect rect(d->pressedPosition - offset, itemViewportRect(newCurrent).center());
                setSelection(rect.normalize(), command);
            } else if (e->state() & ControlButton && d->selectionMode != Single) {
                d->selectionModel->setCurrentItem(newCurrent, QItemSelectionModel::NoUpdate);
            } else {
                d->selectionModel->setCurrentItem(newCurrent, command);
                d->pressedPosition = itemViewportRect(newCurrent).center() + offset;
            }
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
    case Key_Shift:
    case Key_Control:
        break;
    case Key_Enter:
    case Key_Return:
        emit returnPressed(currentItem());
        e->accept();
        return;
    case Key_Space:
        d->selectionModel->select(currentItem(),
                                  selectionCommand(e->state(),
                                                   currentItem(),
                                                   e->type(),
                                                   (Key)e->key()));
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

// ### DOC: Guessed
/*!
    This function is called when a resize event \a e occurs.
*/
void QAbstractItemView::resizeEvent(QResizeEvent *e)
{
    d->backBuffer.resize(e->size());
    QViewport::resizeEvent(e);
    updateGeometries();
}

/*!
    This function is called when a show event \a e occurs.
*/
void QAbstractItemView::showEvent(QShowEvent *e)
{
    QViewport::showEvent(e);
    updateGeometries();
}

// ### DOC: Guessed
/*!
    \internal
*/
void QAbstractItemView::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->autoScrollTimer)
        doAutoScroll();
}

/*!
    Starts editing the item at \a index, creating an editor if
    necessary, and returns true if the view's \l{State} is now \c
    Editing; otherwise returns false. The action that initiated the
    editing is given by \a action, and the event that was behind this
    is given by \a event.

    \sa endEdit() QAbstractItemDelegate::releaseEditor()
*/
bool QAbstractItemView::startEdit(const QModelIndex &index,
                                  QAbstractItemDelegate::StartEditAction action,
                                  QEvent *event)
{
    QAbstractItemDelegate::EditType editType = itemDelegate()->editType(index);
    if (d->shouldEdit(action, index) || editType == QAbstractItemDelegate::PersistentWidget) {
        if (editType == QAbstractItemDelegate::NoWidget && itemDelegate()->event(event, index))
            d->state = Editing;
        else if (d->createEditor(action, event, index))
            d->state = Editing;
    }
    return d->state == Editing;
}

/*!
    If \a accept is true the edit of the item at \a index is accepted
    and the model is updated with the editor's value. If \a accept is
    false, the edited value is ignored. In both cases, if there was an
    editor widget it is released.

    \sa startEdit() QAbstractItemDelegate::releaseEditor()
*/
void QAbstractItemView::endEdit(const QModelIndex &index, bool accept)
{
    if (!index.isValid())
        return;

    setState(NoState);

    QAbstractItemDelegate::EditType editType = itemDelegate()->editType(index);
    if (editType == QAbstractItemDelegate::PersistentWidget
        || editType == QAbstractItemDelegate::NoWidget) {
        // No editor to remove
        setFocus();
        return;
    }

    if (accept) {
        itemDelegate()->setModelData(d->currentEditor, index);
        itemDelegate()->releaseEditor(QAbstractItemDelegate::Accepted,
                                      d->currentEditor, index);
    } else {
        itemDelegate()->releaseEditor(QAbstractItemDelegate::Cancelled,
                                      d->currentEditor, index);
    }
    setFocus();
}

/*!
    Returns the editor if the view is in the \c Editing \l{State};
    otherwise returns 0.
*/
QWidget *QAbstractItemView::currentEditor() const
{
    if (d->state == Editing)
        return d->currentEditor;
    return 0;
}

// ### DOC: Guessed
/*!
    \internal
*/
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

// ### DOC: No idea!
void QAbstractItemView::updateGeometries()
{
    //do nothing
}

// ### DOC: No idea!
void QAbstractItemView::verticalScrollbarAction(int)
{
    //do nothing
}

// ### DOC: No idea!
void QAbstractItemView::horizontalScrollbarAction(int)
{
    //do nothing
}

/*!
    If the \a object is the current editor: if the \a event is an Esc
    key press the current edit is cancelled and ended, or if the \a
    event is an Enter or Return key press the current edit is accepted
    and ended. If editing is ended the event filter returns true to
    signify that it has handled the event; in all other cases it does
    nothing and returns false to signify that the event hasn't been
    handled.

    \sa endEdit()
*/
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

    // search from start with wraparound
    QString searchString = sameKey ? QString(d->keyboardInput.at(0)) : d->keyboardInput;
    QModelIndexList match;
    match = model()->match(start, QAbstractItemModel::Display, searchString, 1, true);
    if (!match.isEmpty() && match.at(0).isValid()) {
        setCurrentItem(match.at(0));
    }
}

/*!
    Returns the size hint for the specified \a item.
*/
QSize QAbstractItemView::itemSizeHint(const QModelIndex &item) const
{
    QItemOptions options;
    getViewOptions(&options);
    return itemDelegate()->sizeHint(fontMetrics(), options, item);
}

/*!
    Returns the height size hint for the specified \a row.
*/
int QAbstractItemView::rowSizeHint(int row) const
{
    QItemOptions options;
    getViewOptions(&options);
    QAbstractItemDelegate *delegate = itemDelegate();
    int height = 0;
    int colCount = d->model->columnCount(root());
    QModelIndex idx;
    for (int c = 0; c < colCount; ++c) {
        idx = d->model->index(row, c, root());
        height = qMax(height, delegate->sizeHint(fontMetrics(), options, idx).height());
    }
    return height;
}

/*!
    Returns the width size hint for the specified \a column.
*/
int QAbstractItemView::columnSizeHint(int column) const
{
    QItemOptions options;
    getViewOptions(&options);
    QAbstractItemDelegate *delegate = itemDelegate();
    int width = 0;
    int rowCount = d->model->rowCount(root());
    QModelIndex idx;
    for (int r = 0; r < rowCount; ++r) {
        idx = d->model->index(r, column, root());
        width = qMax(width, delegate->sizeHint(fontMetrics(), options, idx).width());
    }
    return width;
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

/*!
    This signal is emitted if items are changed in the model. The
    changed items are those from \a topLeft to \a bottomRight
    inclusive. If just one item is changed \a topLeft == \a
    bottomRight.
*/
void QAbstractItemView::contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // Single item changed
    if (topLeft == bottomRight && topLeft.isValid()) {
        if (d->currentEditor && topLeft == currentItem())
            itemDelegate()->setEditorData(d->currentEditor, topLeft);
        else
            d->viewport->update(itemViewportRect(topLeft));
        return;
    }
    doItemsLayout();
}

/*!
    This signal is emitted if new items are inserted into the model.
    The new items are those from \a topLeft to \a bottomRight
    inclusive.

    The base class implementation does nothing.
*/
void QAbstractItemView::contentsInserted(const QModelIndex &, const QModelIndex &)
{
    // do nothing
    // NOTE: if root() was a valid QModelIndex, it may have been invalidated
}

/*!
    This signal is emitted if items are removed from the model. The
    removed items are those from \a topLeft to \a bottomRight
    inclusive.

    The base class implementation does nothing.
*/
void QAbstractItemView::contentsRemoved(const QModelIndex &, const QModelIndex &)
{
    // do nothing
    // NOTE: if root() was a valid QModelIndex, it may have been invalidated
}

/*!
    Returns the item delegate used by this view and model. This is
    either one set with setItemDelegate(), or the default one.

    \sa setItemDelegate()
*/
QAbstractItemDelegate *QAbstractItemView::itemDelegate() const
{
    if (!d->delegate)
        d->delegate = new QItemDelegate(d->model, const_cast<QAbstractItemView *>(this));
    return d->delegate;
}

/*!
    Sets the item delegate for this view and its model to \a delegate.
    This is useful if you want complete control over the editing and
    display of items.

    \sa itemDelegate()
*/
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

/*!
    This signal is emitted when the selection is changed. The previous
    selection (which may be empty), is given by \a deselected, and the
    new selection by \a selected.
*/
void QAbstractItemView::selectionChanged(const QItemSelection &deselected,
                                         const QItemSelection &selected)
{
    QRect deselectedRect = selectionViewportRect(deselected);
    QRect selectedRect = selectionViewportRect(selected);
    d->viewport->repaint(deselectedRect.unite(selectedRect));
}

/*!
    This signal is emitted when a new item becomes the current item.
    The previous current item is given by \a old, and the new current
    item by \a current.

    If you want to know about changes to items see the
    contentsChanged() signal.
*/
void QAbstractItemView::currentChanged(const QModelIndex &old, const QModelIndex &current)
{
    if (d->currentEditor)
        endEdit(old, true);

    if (old.isValid())
        d->viewport->repaint(itemViewportRect(old));

    if (current.isValid())
        ensureItemVisible(current);

    startEdit(current, QAbstractItemDelegate::CurrentChanged, 0);
}

void QAbstractItemView::fetchMore()
{
    // FIXME: this may not be the right way of doing this
    if (!verticalScrollBar()->isSliderDown() &&
        verticalScrollBar()->value() == verticalScrollBar()->maximum())
        model()->fetchMore();
}

/*!
    Clears the area specified by \a rect with the palette's base color
    using the given \a painter.
*/
void QAbstractItemView::clearArea(QPainter *painter, const QRect &rect) const
{
    painter->fillRect(rect, palette().brush(QPalette::Base));
}

/*!
    Returns true if this view supports drag and drop; otherwise
    returns false.

    The base class implementation returns false.
*/
bool QAbstractItemView::supportsDragAndDrop() const
{
    return false;
}

/*!
    Returns a new drag object that contains the model indexes of all
    the model's selected items.

    \sa startDrag()
*/
QDragObject *QAbstractItemView::dragObject()
{
    QModelIndexList items = d->selectionModel->selectedItems();
    return model()->dragObject(items, this);
}

/*!
    Starts a drag by calling drag() on a new dragObject().
*/
void QAbstractItemView::startDrag()
{
    QDragObject *obj = dragObject();
    if (!obj)
        return;
    obj->drag();
}

/*!
    Populates the \a options with the view's palette and whether or
    not the view is in the \c Editing state.
*/
void QAbstractItemView::getViewOptions(QItemOptions *options) const
{
    options->palette = palette();
    options->editing = state() == Editing;
}

/*!
    Returns the item view's state.

    \sa setState()
*/
QAbstractItemView::State QAbstractItemView::state() const
{
    return d->state;
}

/*!
    Sets the item view's state to \a state

    \sa state()
*/
void QAbstractItemView::setState(State state)
{
    d->state = state;
}

/*!
  internal
*/
void QAbstractItemView::startAutoScroll()
{
    if (d->autoScrollTimer)
        killTimer(d->autoScrollTimer);
    d->autoScrollTimer = startTimer(d->autoScrollInterval);
    d->autoScrollCount = 0;
}

/*!
  internal
*/
void QAbstractItemView::stopAutoScroll()
{
    killTimer(d->autoScrollTimer);
    d->autoScrollTimer = 0;
    d->autoScrollCount = 0;
}

/*!
  \internal
*/
void QAbstractItemView::doAutoScroll()
{
    if (d->autoScrollCount < qMax(verticalScrollBar()->pageStep(),
                                  horizontalScrollBar()->pageStep()))
        ++d->autoScrollCount;
    int margin = d->autoScrollMargin;
    int verticalValue = verticalScrollBar()->value();
    int horizontalValue = horizontalScrollBar()->value();

    QPoint pos = d->viewport->mapFromGlobal(QCursor::pos());
    QRect area = d->viewport->clipRegion().boundingRect();

    int delta = 0;
    if (pos.y() - area.top() < margin)
        delta = -d->autoScrollCount;
    else if (area.bottom() - pos.y() < margin)
        delta = d->autoScrollCount;
    verticalScrollBar()->setValue(verticalValue + delta);

    if (pos.x() - area.left() < margin)
        delta = -d->autoScrollCount;
    else if (area.right() - pos.x() < margin)
        delta = d->autoScrollCount;
    horizontalScrollBar()->setValue(horizontalValue + delta);

    if (verticalValue == verticalScrollBar()->value()
        && horizontalValue == horizontalScrollBar()->value()
        || state() != Dragging)
        stopAutoScroll();
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

bool QAbstractItemViewPrivate::shouldEdit(QAbstractItemDelegate::StartEditAction action,
                                          const QModelIndex &index)
{
    if (!model->isEditable(index))
        return false;
    if (state == QAbstractItemView::Editing)
        return false;
    return ((action == QAbstractItemDelegate::AlwaysEdit) || (action & startEditActions));
}

bool QAbstractItemViewPrivate::shouldAutoScroll(const QPoint &pos)
{
    if (!autoScroll)
        return false;
    QRect area = viewport->clipRegion().boundingRect();
    return (pos.y() - area.top() < autoScrollMargin)
        || (area.bottom() - pos.y() < autoScrollMargin)
        || (pos.x() - area.left() < autoScrollMargin)
        || (area.right() - pos.x() < autoScrollMargin);
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
        editor = delegate->editor(action, viewport, options, index);
    }
    if (editor) {
        editor->show();
        editor->setFocus();
        if (event && (action == QAbstractItemDelegate::AnyKeyPressed
                      || event->type() == QEvent::MouseButtonPress))
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

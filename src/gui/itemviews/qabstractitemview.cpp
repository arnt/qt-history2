/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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
#include <qdebug.h>

#include <private/qabstractitemview_p.h>
#define d d_func()
#define q q_func()

class QDefaultModel : public QAbstractListModel
{
public:
    QDefaultModel(QObject *parent) : QAbstractListModel(parent) {}
    ~QDefaultModel() {}

    int rowCount() const  { return 0; }
    QVariant data(const QModelIndex &, int) const { return QVariant(); }
};

QAbstractItemViewPrivate::QAbstractItemViewPrivate()
    :   model(0),
        delegate(0),
        selectionModel(0),
        selectionMode(QAbstractItemView::ExtendedSelection),
        selectionBehavior(QAbstractItemView::SelectItems),
        state(QAbstractItemView::NoState),
        beginEditActions(QAbstractItemDelegate::DoubleClicked
                         |QAbstractItemDelegate::EditKeyPressed),
        inputInterval(400),
        autoScroll(true),
        autoScrollTimer(0),
        autoScrollMargin(16),
        autoScrollInterval(50),
        autoScrollCount(0),
        dragEnabled(false),
        layoutPosted(false)
{
}

QAbstractItemViewPrivate::~QAbstractItemViewPrivate()
{
}

void QAbstractItemViewPrivate::init()
{
    viewport->installEventFilter(q);

    q->setModel(new QDefaultModel(q));

    q->verticalScrollBar()->setRange(0, 0);
    q->horizontalScrollBar()->setRange(0, 0);

    QObject::connect(q->verticalScrollBar(), SIGNAL(actionTriggered(int)),
                     q, SLOT(verticalScrollbarAction(int)));
    QObject::connect(q->horizontalScrollBar(), SIGNAL(actionTriggered(int)),
                     q, SLOT(horizontalScrollbarAction(int)));

    viewport->setBackgroundRole(QPalette::Base);
    viewport->setAttribute(Qt::WA_NoBackground);

    q->setHorizontalFactor(256);
    q->setVerticalFactor(256);

    doDelayedItemsLayout();
}

/*!
    \class QAbstractItemView qabstractitemview.h

    \brief The QAbstractItemView class is the base class for every
    view that uses a QAbstractItemModel.

    \ingroup model-view

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
    dataChanged(), rowsInserted(), rowsRemoved(),
    columnsInserted(), columnsRemoved(),
    selectionChanged(), and currentChanged().

    The root item is returned by root(), and the current item by
    currentItem(). To make sure that an item is visible use
    ensureItemVisible().

    Some of QAbstractItemView's functions are concerned with
    scrolling, for example setHorizontalFactor() and
    setVerticalFactor(). Several other functions are concerned with
    selection control; for example setSelectionMode(), and
    setSelectionBehavior(). This class provides a default selection
    model to work with (selectionModel()), but this can be replaced
    by using setSelectionModel() with an instance of
    QItemSelectionModel.

    For complete control over the display and editing of items you can
    specify a delegate with setItemDelegate().

    QAbstractItemView provides a lot of protected functions. Some are
    concerned with editing, for example, beginEdit(), endEdit(), and
    currentEditor(), whilst others are keyboard and mouse event
    handlers.

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemModel

*/

/*!
    \enum QAbstractItemView::SelectionMode

    \value SingleSelection
    \value MultiSelection
    \value ExtendedSelection
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

    Returns the model index of the item at point (\a x, \a y).

    In the base class this is a pure virtual function.
*/

/*!
    \fn void QAbstractItemView::rootChanged(const QModelIndex &old, const QModelIndex &root)

    This signal is emitted when the model's root index changes. The
    previous index is specified by \a old, and the new root index is
    specified by \a root.
*/

/*!
  \fn void QAbstractItemView::onItem(const QModelIndex &index, int button)

  This signal is emitted when the cursor is positioned on the item
  specified by \a index.
  The button state is specified by \a button (see \l{Qt::ButtonState}).
*/

/*!
    \fn void QAbstractItemView::pressed(const QModelIndex &index, int button)

    This signal is emitted when a mouse button is pressed. The button
    is specified by \a button (see \l{Qt::ButtonState}), and the item the
    mouse was pressed on is specified by \a index (which may be invalid if
    the mouse was not pressed on an item).
*/

/*!
    \fn void QAbstractItemView::clicked(const QModelIndex &index, int button)

    This signal is emitted when a mouse button is clicked. The button
    is specified by \a button (see \l{Qt::ButtonState}), and the item the
    mouse was clicked on is specified by \a index (which may be invalid if
    the mouse was not clicked on an item).
*/

/*!
    \fn void QAbstractItemView::doubleClicked(const QModelIndex &index, int button)

    This signal is emitted when a mouse button is double-clicked. The
    button is specified by \a button (see \l{Qt::ButtonState}), and the
    item the mouse was double-clicked on is specified by \a index (which
    may be invalid if the mouse was not double-clicked on an item).
*/

/*!
    \fn void QAbstractItemView::returnPressed(const QModelIndex &index)

    This signal is emitted when Return (or Enter) is pressed. The item
    to be acted on by the key press is specified by \a index.
*/

/*!
    \fn void QAbstractItemView::spacePressed(const QModelIndex &index)

    This signal is emitted when Space is pressed. The item to be acted on
    by the key press is specified by \a index.
*/

/*!
    \fn void QAbstractItemView::deletePressed(const QModelIndex &index)

    This signal is emitted when the Delete key is pressed. The item
    to be acted on by the key press is specified by \a index.
*/

/*!
    \fn void QAbstractItemView::aboutToShowContextMenu(QMenu *menu, const QModelIndex &index)

    This signal is emitted when the context menu is invoked. The \a
    menu is an empty menu; if you populate it with actions it will be
    popped up for the user. The current item when the contex menu
    event occurred is specified by \a index.
*/

/*!
    \fn QModelIndex QAbstractItemView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state) = 0

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
    \fn void QAbstractItemView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)

    Applies the selection \a flags to the items in or touched by the
    rectangle, \a rect.

    \sa selectionCommand()
*/

/*!
    \fn QRect QAbstractItemView::selectionViewportRect(const QItemSelection &selection) const = 0

    Returns the rectangle from the viewport of the items in the given
    \a selection.
*/

/*!
    Creates a new QAbstractItemView with parent \a parent.
*/
QAbstractItemView::QAbstractItemView(QWidget *parent)
    : QViewport(*(new QAbstractItemViewPrivate), parent)
{
    d->init();
}

/*!
    \internal
*/
QAbstractItemView::QAbstractItemView(QAbstractItemViewPrivate &dd, QWidget *parent)
    : QViewport(dd, parent)
{
    d->init();
}

/*!
    Destroys the view.
*/
QAbstractItemView::~QAbstractItemView()
{
    if (d->selectionModel)
        disconnect(d->selectionModel, SIGNAL(destroyed(QObject *)),
                   this, SLOT(selectionModelDestroyed()));
}

/*!
  Sets the \a model for the view to present.
  \omit
  This function will also create and set a new selection model.
  \endomit
*/
void QAbstractItemView::setModel(QAbstractItemModel *model)
{
    if (d->model) {
        QObject::disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                            this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        QObject::disconnect(d->model, SIGNAL(rowsInserted(const QModelIndex&,int,int)),
                            this, SLOT(rowsInserted(const QModelIndex&,int,int)));
        QObject::disconnect(d->model, SIGNAL(rowsRemoved(const QModelIndex&,int,int)),
                            this, SLOT(rowsRemoved(const QModelIndex&,int,int)));
        QObject::disconnect(d->model, SIGNAL(reset()), this, SLOT(reset()));
    }

    d->model = model;

    if (d->model) {
        QObject::connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                         this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        QObject::connect(d->model, SIGNAL(rowsInserted(const QModelIndex&,int,int)),
                         this, SLOT(rowsInserted(const QModelIndex&,int,int)));
        QObject::connect(d->model, SIGNAL(rowsRemoved(const QModelIndex&,int,int)),
                         this, SLOT(rowsRemoved(const QModelIndex&,int,int)));
        QObject::connect(d->model, SIGNAL(reset()), this, SLOT(reset()));
    }

    setRoot(QModelIndex::Null);// triggers layout

    d->selectionModel = 0;
    setSelectionModel(new QItemSelectionModel(d->model, this));
}

/*!
    Returns the model that this view is presenting.
*/
QAbstractItemModel *QAbstractItemView::model() const
{
    return d->model;
}

/*!
    Sets the current selection to be \a selectionModel.

    \sa selectionModel() clearSelections()
*/
void QAbstractItemView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    Q_ASSERT(selectionModel);

    if (selectionModel && selectionModel->model() != d->model) {
        qWarning("QAbstractItemView::setSelectionModel() failed: Trying to set a selection model,"
                 " which works on a different model than the view.");
        return;
    }

    if (d->selectionModel) {
        disconnect(d->selectionModel, SIGNAL(destroyed(QObject*)),
                   this, SLOT(selectionModelDestroyed()));
        disconnect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                   this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
        disconnect(d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                   this, SLOT(currentChanged(QModelIndex,QModelIndex)));
    }

    d->selectionModel = selectionModel;

    if (d->selectionModel) {
        connect(d->selectionModel, SIGNAL(destroyed(QObject*)),
                this, SLOT(selectionModelDestroyed()));
        connect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
        connect(d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentChanged(QModelIndex,QModelIndex)));

        bool block = d->selectionModel->blockSignals(true);
        if (!d->selectionModel->currentItem().isValid())
            d->selectionModel->setCurrentItem(d->model->index(0, 0, root()),
                                              QItemSelectionModel::NoUpdate);
        d->selectionModel->blockSignals(block);
    }
}

// ### DOC: Couldn't we call this selection() (and setSelection() and clearSelection()) ?
// This can be confused with actually selecting items.
/*!
    Returns the current selection.

    \sa setSelectionModel() clearSelections()
*/
QItemSelectionModel* QAbstractItemView::selectionModel() const
{
//     if (!d->selectionModel)
//         setSelectionModel(new QItemSelectionModel(model(), const_cast<QAbstractItemView*>(this));
    return d->selectionModel;
}

/*!
    Sets the item delegate for this view and its model to \a delegate.
    This is useful if you want complete control over the editing and
    display of items.

    \sa itemDelegate()
*/
void QAbstractItemView::setItemDelegate(QAbstractItemDelegate *delegate)
{
    Q_ASSERT(delegate);
    d->delegate = delegate;
    QObject::connect(delegate, SIGNAL(doneEditing(QWidget*, QAbstractItemDelegate::EndEditAction)),
                     this, SLOT(doneEditing(QWidget*, QAbstractItemDelegate::EndEditAction)));
}

/*!
    Returns the item delegate used by this view and model. This is
    either one set with setItemDelegate(), or the default one.

    \sa setItemDelegate()
*/
QAbstractItemDelegate *QAbstractItemView::itemDelegate() const
{
    if (!d->delegate) {
        QAbstractItemView *that = const_cast<QAbstractItemView*>(this);
        that->setItemDelegate(new QItemDelegate(that));
    }
    return d->delegate;
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
    selectionModel()->setCurrentItem(index, selectionCommand(Qt::NoButton, index));
}

/*!
    Returns the model index of the current item.

    \sa setCurrentItem()
*/
QModelIndex QAbstractItemView::currentItem() const
{
    return selectionModel()->currentItem();
}


/*!
  Reset the internal state of the view.
*/
void QAbstractItemView::reset()
{
    bool block = blockSignals(true); // no rootChanged signal
    setRoot(QModelIndex::Null); // does a relayout
    blockSignals(block);
     // the views will be updated later
}

/*!
    Sets the ``root'' item to the item at \a index.

    \sa root()
*/
void QAbstractItemView::setRoot(const QModelIndex &index)
{
    QModelIndex old = d->root;
    d->root = QPersistentModelIndex(index, d->model);
    emit rootChanged(old, index);
    if (isVisible())
        doItemsLayout();
}

/*!
    Returns the model index of the model's ``root'' item.

    \sa setRoot()
*/
QModelIndex QAbstractItemView::root() const
{
    return QModelIndex(d->root);
}

/*!
    Calls beginEdit() for the item at \a index.
*/
void QAbstractItemView::edit(const QModelIndex &index)
{
    if (!index.isValid())
        qWarning("edit: index was invalid");
    if (!beginEdit(index, QAbstractItemDelegate::AlwaysEdit, 0))
        qWarning("edit: editing failed");
}

/*!
    Clears the selection.
*/
void QAbstractItemView::clearSelections()
{
    selectionModel()->clear();
}

/*!
    \internal

    This function is intended to lay out the items in the view.
    The default implementation just call updateGeometries() on the viewport.
*/
void QAbstractItemView::doItemsLayout()
{
    d->layoutPosted = false;
    updateGeometries();
    d->viewport->update();
}

/*!
    \property QAbstractItemView::beginEditActions
    \brief which actions will initiate item editing

    This property is a selection of flags defined by
    \l{QAbstractItemDelegate::BeginEditAction}, combined using the OR
    operator. The view will only initiate the editing of an item if the
    action performed is set in this property.
*/
void QAbstractItemView::setBeginEditActions(QAbstractItemDelegate::BeginEditActions actions)
{
    d->beginEditActions = actions;
}

QAbstractItemDelegate::BeginEditActions QAbstractItemView::beginEditActions() const
{
    return d->beginEditActions;
}

/*!
    \property QAbstractItemView::autoScroll
    \brief whether autoscrolling in drag move events is enabled

    If this property is set to true (the default), the
    QAbstractItemView automatically scrolls the contents of the view
    if the user drags within 16 pixels of the viewport edge. This only works if
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
        QPoint margins = d->viewport->geometry().topLeft();
        QModelIndex index = itemAt(he->pos() - margins);
        if (!index.isValid())
            break;
        QString tooltip = model()->data(index, QAbstractItemModel::ToolTipRole).toString();
        QToolTip::showText(he->globalPos(), tooltip, this);
        return true; }
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        QPoint margins = d->viewport->geometry().topLeft();
        QModelIndex index = itemAt(he->pos() - margins);
        if (!index.isValid())
            break;
        QString whatsthis = model()->data(index, QAbstractItemModel::WhatsThisRole).toString();
        QWhatsThis::showText(he->globalPos(), whatsthis, this);
        return true; }
    case QEvent::StatusTip: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        QPoint margins = d->viewport->geometry().topLeft();
        QModelIndex index = itemAt(he->pos() - margins);
        if (!index.isValid())
            break;
        QString statustip = model()->data(index, QAbstractItemModel::StatusTipRole).toString();
        if (!statustip.isEmpty())
            setStatusTip(statustip);
        return true; }
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        d->viewport->update();
        break;
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

    QPersistentModelIndex persistent(index, model());
    if (d->state == Editing && d->editors.contains(persistent))
        return;

    QPoint offset(horizontalOffset(), verticalOffset());
    QItemSelectionModel::SelectionFlags command =
        selectionCommand(e->state(), index, e->type());
    d->pressedItem = index;
    d->pressedState = e->state();
    if ((command & QItemSelectionModel::Current) == 0)
        d->pressedPosition = pos + offset;

    if (index.isValid())
        selectionModel()->setCurrentItem(index, QItemSelectionModel::NoUpdate);

    QRect rect(d->pressedPosition - offset, pos);
    setSelection(rect.normalize(), command);

    emit pressed(index, e->button());
    beginEdit(index, QAbstractItemDelegate::SelectedClicked, e);
}

/*!
    This function is called when a mouse move event \a e occurs. If a
    selection is in progress and new items are moved over the
    selection is extended; if a drag is in progress it is continued.
*/
void QAbstractItemView::mouseMoveEvent(QMouseEvent *e)
{
    QPoint topLeft;
    QPoint bottomRight = e->pos();
    if (d->selectionMode != SingleSelection)
        topLeft = d->pressedPosition - QPoint(horizontalOffset(), verticalOffset());
    else
        topLeft = bottomRight;

    if (state() == Dragging) {
        if ((topLeft - bottomRight).manhattanLength() > QApplication::startDragDistance()) {
            startDrag();
            setState(NoState); // the startDrag will return when the dnd operation is done
            stopAutoScroll();
        }
        return;
    }

    QModelIndex index = itemAt(bottomRight);
    QPersistentModelIndex persistent(index, model());
    if (state() == Editing && d->editors.contains(persistent))
        return;

    if (index != currentItem()) {
        if (index.isValid())
            emit onItem(index, e->state());
    } else if (state() == Selecting) {
        return; // we haven't moved over another item yet
    }

    if (!(e->state() & Qt::LeftButton))
        return;

    if (index.isValid()) {
        if (state() != Selecting) {
            bool dnd = model()->isDragEnabled(index) && isDragEnabled(index);
            bool selected = selectionModel()->isSelected(index);
            if (dnd && selected) {
                setState(Dragging);
                return;
            }
        }
        selectionModel()->setCurrentItem(index, QItemSelectionModel::NoUpdate);
    }
    setState(Selecting);
    setSelection(QRect(topLeft, bottomRight).normalize(),
                 selectionCommand(e->state(), index, e->type()));
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

    QPersistentModelIndex persistent(index, model());
    if (state() == Editing && d->editors.contains(persistent))
        return;

    if (state() == Selecting) {
        selectionModel()->select(index, selectionCommand(e->state(), index, e->type()));
        setState(NoState);
    }

    if (index == d->pressedItem)
        emit clicked(index, e->button());
    if (e->button() == Qt::RightButton) {
        QContextMenuEvent me(QContextMenuEvent::Mouse, pos, e->state());
        QApplication::sendEvent(this, &me);
    }
}

/*!
    This function is called when a mouse double-click event \a e
    occurs. If the double-click is on a valid item it emits the
    doubleClicked() signal and calls beginEdit() on the item.
*/
void QAbstractItemView::mouseDoubleClickEvent(QMouseEvent *e)
{
    QModelIndex index = itemAt(e->pos());
    if (!index.isValid())
        return;
    emit doubleClicked(index, e->button());
    beginEdit(index, QAbstractItemDelegate::DoubleClicked, e);
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

    \sa startDrag()
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
    QModelIndex index = currentItem();
    if (index.isValid())
        d->viewport->update(itemViewportRect(index));
}

/*!
    This function is called when focus event \a e occurs and is a
    focus out event.
*/
void QAbstractItemView::focusOutEvent(QFocusEvent *e)
{
    QViewport::focusOutEvent(e);
    QModelIndex index = currentItem();
    if (index.isValid())
        d->viewport->update(itemViewportRect(index));
}

/*!
    This function is called when a key event \a e occurs. It handles
    basic cursor movement, e.g. Up, Down, Left, Right, Home, PageUp,
    and PageDown, and emits the returnPressed(), spacePressed(), and
    deletePressed() signals is the associated key is pressed. This
    function is where editing is initiated by key press, e.g. if F2 is
    pressed.

    \sa beginEdit()
*/
void QAbstractItemView::keyPressEvent(QKeyEvent *e)
{
    bool hadCurrent = true;
    QModelIndex current = currentItem();
    if (!current.isValid()) {
        hadCurrent = false;
        setCurrentItem(model()->index(0, 0, QModelIndex::Null));
    }
    QModelIndex newCurrent = current;
    if (hadCurrent) {
        switch (e->key()) {
        case Qt::Key_Down:
            newCurrent = moveCursor(MoveDown, e->state());
            break;
        case Qt::Key_Up:
            newCurrent = moveCursor(MoveUp, e->state());
            break;
        case Qt::Key_Left:
            newCurrent = moveCursor(MoveLeft, e->state());
            break;
        case Qt::Key_Right:
            newCurrent = moveCursor(MoveRight, e->state());
            break;
        case Qt::Key_Home:
            newCurrent = moveCursor(MoveHome, e->state());
            break;
        case Qt::Key_End:
            newCurrent = moveCursor(MoveEnd, e->state());
            break;
        case Qt::Key_PageUp:
            newCurrent = moveCursor(MovePageUp, e->state());
            break;
        case Qt::Key_PageDown:
            newCurrent = moveCursor(MovePageDown, e->state());
            break;
        }

        if (newCurrent != current && newCurrent.isValid()) {
            QItemSelectionModel::SelectionFlags command = selectionCommand(e->state(),
                                                                           newCurrent,
                                                                           e->type(),
                                                                           (Qt::Key)e->key());
            if (command & QItemSelectionModel::Current) {
                selectionModel()->setCurrentItem(newCurrent, QItemSelectionModel::NoUpdate);
                QPoint offset(horizontalOffset(), verticalOffset());
                QRect rect(d->pressedPosition - offset, itemViewportRect(newCurrent).center());
                setSelection(rect.normalize(), command);
            } else {
                selectionModel()->setCurrentItem(newCurrent, command);
                QPoint offset(horizontalOffset(), verticalOffset());
                d->pressedPosition = itemViewportRect(newCurrent).center() + offset;
            }
            return;
        }
    }

    switch (e->key()) {
    // keys to ignore
    case Qt::Key_Down:
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Escape:
    case Qt::Key_Shift:
    case Qt::Key_Control:
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        emit returnPressed(currentItem());
        return;
    case Qt::Key_Space:
        selectionModel()->select(currentItem(),
                                 selectionCommand(e->state(),
                                                  currentItem(),
                                                  e->type(),
                                                  (Qt::Key)e->key()));
        emit spacePressed(currentItem());
        return;
    case Qt::Key_Delete:
        emit deletePressed(currentItem());
        return;
    case Qt::Key_F2:
        if (beginEdit(currentItem(), QAbstractItemDelegate::EditKeyPressed, e))
            return;
        break;
    default:
        if (!e->text().isEmpty()) {
            if (beginEdit(currentItem(), QAbstractItemDelegate::AnyKeyPressed, e)) {
                return;
            } else {
                keyboardSearch(e->text());
                return;
            }
        }
        break;
    }

    e->ignore();
}

/*!
    This function is called when a resize event \a e occurs.
*/
void QAbstractItemView::resizeEvent(QResizeEvent *e)
{
    QViewport::resizeEvent(e);
    updateGeometries();
}

/*!
  This function is called when a timer event \a e occurs.
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
    editing is specified by \a action, and the event that was behind this
    is specified by \a event.

    \sa endEdit() QAbstractItemDelegate::releaseEditor()
*/
bool QAbstractItemView::beginEdit(const QModelIndex &index,
                                  QAbstractItemDelegate::BeginEditAction action,
                                  QEvent *event)
{
    if (itemDelegate()->editorType(model(), index) == QAbstractItemDelegate::Events) {
        itemDelegate()->event(event, model(), index);
        return true; // event was consumed
    }

    QModelIndex buddy = model()->buddy(index);
    QModelIndex edit = buddy.isValid() ? buddy : index;
    if (edit.isValid() && d->shouldEdit(action, edit)) {
        QWidget *editor = d->requestEditor(action, event, edit);
        if (editor) {
            d->state = Editing;
            editor->show();
            editor->setFocus();
        }
    }

    return d->state == Editing;
}

/*!
    If the end \a action is \c Accepted the edit of the item at \a
    index is accepted and the model is updated with the editor's
    value. If the \a action is \c Cancelled, the edited value is
    ignored. In both cases, if there was an editor widget it is
    released.

    \sa beginEdit() QAbstractItemDelegate::releaseEditor()
*/
void QAbstractItemView::endEdit(const QModelIndex &index,
                                QAbstractItemDelegate::EndEditAction action)
{
    if (d->state != Editing)
        return;

    QModelIndex buddy = model()->buddy(index);
    QPersistentModelIndex persistent(buddy.isValid() ? buddy : index, model());

    if (!persistent.isValid())
        return;

    if (itemDelegate()->editorType(model(), persistent) == QAbstractItemDelegate::Events) {
        d->state = NoState;
        return;
    }

    QWidget *editor = d->editors.value(persistent);
    if (editor) {
        itemDelegate()->releaseEditor(action, editor, model(), index);
        d->editors.remove(persistent);
        d->state = NoState;
        setFocus();
    }
}

/*!
    \internal
*/
void QAbstractItemView::updateEditors()
{
    QStyleOptionViewItem option = viewOptions();
    QMap<QPersistentModelIndex, QPointer<QWidget> >::iterator it = d->editors.begin();
    for (; it != d->editors.end(); ++it) {
        option.rect = itemViewportRect(it.key());
        itemDelegate()->updateEditorGeometry(it.value(), option, d->model, it.key());
    }
}

/*!
    \internal
*/
void QAbstractItemView::updateGeometries()
{
    updateEditors();
}

/*!
    \internal
*/
void QAbstractItemView::verticalScrollbarAction(int)
{
    //do nothing
}

/*!
    \internal
*/
void QAbstractItemView::horizontalScrollbarAction(int)
{
    //do nothing
}

/*!
  \internal
*/
void QAbstractItemView::selectionModelDestroyed()
{
    d->selectionModel = 0;
}

/*!
  \internal
*/
void QAbstractItemView::doneEditing(QWidget *editor, QAbstractItemDelegate::EndEditAction action)
{
    endEdit(d->editors.key(editor), action);
}

// ###DOC: this value is also used by the "scroll in item units" algorithm to
// enable the scrolling in fractions of item units (one step == itemHeight / verticalFraction)
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
  Moves to and selects the item best matching the string \a search.
  If no item is found nothing happens.
*/
void QAbstractItemView::keyboardSearch(const QString &search)
{
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
        QModelIndex parent = model()->parent(start);
        int newRow = (start.row() < model()->rowCount(parent) - 1)
                     ? start.row() + 1 : 0;
        start = model()->index(newRow,
                               start.column(),
                               model()->parent(start),
                               start.type());
    }

    // search from start with wraparound
    QString searchString = sameKey ? QString(d->keyboardInput.at(0)) : d->keyboardInput;
    QModelIndexList match;
    match = model()->match(start, QAbstractItemModel::DisplayRole, searchString);
    if (!match.isEmpty() && match.at(0).isValid()) {
        setCurrentItem(match.at(0));
    }
}

/*!
    Returns the size hint for the item with the specified \a index.
*/
QSize QAbstractItemView::itemSizeHint(const QModelIndex &index) const
{
    return itemDelegate()->sizeHint(fontMetrics(), viewOptions(), model(), index);
}

/*!
    Returns the height size hint for the specified \a row.
*/
int QAbstractItemView::rowSizeHint(int row) const
{
    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    int height = 0;
    int colCount = d->model->columnCount(root());
    QModelIndex idx;
    for (int c = 0; c < colCount; ++c) {
        idx = d->model->index(row, c, root());
        height = qMax(height, delegate->sizeHint(fontMetrics(), option, d->model, idx).height());
    }
    return height;
}

/*!
    Returns the width size hint for the specified \a column.
*/
int QAbstractItemView::columnSizeHint(int column) const
{
    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    int width = 0;
    int rows = d->model->rowCount(root());
    QModelIndex idx;
    for (int r = 0; r < rows; ++r) {
        idx = d->model->index(r, column, root());
        width = qMax(width, delegate->sizeHint(fontMetrics(), option, d->model, idx).width());
    }
    return width;
}

/*!
    Sets \a editor as the persistent editor for the item at the given
    \a index. If \a editor is 0 and no previous persistent editor has
    been set for the \a index, the editor will be created by the
    delegate.
*/
void QAbstractItemView::setPersistentEditor(const QModelIndex &index, QWidget *editor)
{
    QPersistentModelIndex persistent(index, model());
    if (d->editors.contains(persistent))
        return;
    if (!editor) {
        QStyleOptionViewItem option = viewOptions();
        option.rect = itemViewportRect(index);
        option.state = (index == currentItem() ? QStyle::Style_HasFocus|option.state : option.state);
        editor = itemDelegate()->editor(QAbstractItemDelegate::AlwaysEdit,
                                        d->viewport, option, model(), index);
        itemDelegate()->setModelData(editor, model(), index);
        itemDelegate()->updateEditorGeometry(editor, option, model(), index);
    }
    d->editors.insert(persistent, editor);
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
    This slot is called when items are changed in the model. The
    changed items are those from \a topLeft to \a bottomRight
    inclusive. If just one item is changed \a topLeft == \a
    bottomRight.
*/
void QAbstractItemView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // Single item changed
    if (topLeft == bottomRight && topLeft.isValid()) {
        QPersistentModelIndex persistent(topLeft, model());
        if (d->editors.contains(persistent)) {
            itemDelegate()->setEditorData(d->editors.value(persistent), d->model, topLeft);
        } else {
            d->viewport->update(itemViewportRect(topLeft));
        }
        return;
    }
    // single row changed
    if (topLeft.row() == bottomRight.row() && topLeft.isValid()) {
        QModelIndex current = currentItem();
        QPersistentModelIndex persistent(topLeft, model());
        if (d->editors.contains(persistent)) { // FIXME: update all persistent editors
            itemDelegate()->setEditorData(d->editors.value(persistent), d->model, current);
        } else {
            QRect tl = itemViewportRect(topLeft);
            QRect br = itemViewportRect(bottomRight);
            d->viewport->update(tl.unite(br));
        }
    }
    // more changed
    d->viewport->update();
}

/*!
    This slot is called when rows are inserted. The new rows are those
    under the given \a parent from \a start to \a end inclusive. The
    base class implementation does nothing.

    \sa rowsRemoved()
*/
void QAbstractItemView::rowsInserted(const QModelIndex &, int, int)
{
    // do nothing
}

/*!
    This slot is called when rows are removed. The deleted rows are
    those under the given \a parent from \a start to \a end inclusive.
    The base class implementation does nothing.

    \sa rowsInserted()
*/
void QAbstractItemView::rowsRemoved(const QModelIndex &, int, int)
{
    // do nothing
}

/*!
    This slot  is called when the selection is changed. The previous
    selection (which may be empty), is specified by \a deselected, and the
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
    This slot is called when a new item becomes the current item.
    The previous current item is specified by the \a old index, and the new
    item by the \a current index.

    If you want to know about changes to items see the
    dataChanged() signal.
*/
void QAbstractItemView::currentChanged(const QModelIndex &old, const QModelIndex &current)
{
    if (old.isValid()) {
        endEdit(old, QAbstractItemDelegate::Accepted);
        int behavior = selectionBehavior();
        QRect rect = itemViewportRect(old);
        if (behavior & SelectRows) {
            rect.setLeft(0);
            rect.setRight(d->viewport->width());
        }
        if (behavior & SelectColumns) {
            rect.setTop(0);
            rect.setBottom(d->viewport->height());
        }
        d->viewport->repaint(rect);
    }

    if (current.isValid()) {
        ensureItemVisible(current);
        beginEdit(current, QAbstractItemDelegate::CurrentChanged, 0);
    }
}

/*!
    Returns a new drag object that contains the model indexes of all
    the model's selected items.

    \sa startDrag()
*/
QDragObject *QAbstractItemView::dragObject()
{
    QModelIndexList items = selectionModel()->selectedItems();
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
    Returns true if the item view allows the item at position \a index
    to be dragged; otherwise returns false.
*/
bool QAbstractItemView::isDragEnabled(const QModelIndex &) const
{
    return false;
}

/*!
    Returns QStyleOptionViewItem structure populated with the view's palette and whether or
    not the view is in the \c Editing state.
*/
QStyleOptionViewItem QAbstractItemView::viewOptions() const
{
    QStyleOptionViewItem option(0);
    option.palette = palette();
    option.font = font();
    option.state = QStyle::Style_Enabled
                   |(state() == Editing ? QStyle::Style_Editing : QStyle::Style_Default);
    option.decorationSize = QStyleOptionViewItem::Small;
    option.decorationPosition = QStyleOptionViewItem::Left;
    option.decorationAlignment = Qt::AlignCenter;
    option.displayAlignment = Qt::AlignAuto|Qt::AlignVCenter;
    return option;
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
    Returns the SelectionFlags to be used when updating selections.
    Reimplement this function to add your own selection behavior.

    This function is called on user input events like mouse and
    keyboard events; the mouse button state is specified by \a state, the
    index of the relevant item by \a index, the even type by \a type,
    and the key (if a key was pressed) by \a key.
*/
QItemSelectionModel::SelectionFlags QAbstractItemView::selectionCommand(Qt::ButtonState state,
                                                                        const QModelIndex &index,
                                                                        QEvent::Type type,
                                                                        Qt::Key key) const
{
    QItemSelectionModel::SelectionFlags behavior = QItemSelectionModel::NoUpdate;
    switch (d->selectionBehavior) {
    case SelectRows:
        behavior = QItemSelectionModel::Rows;
        break;
    case SelectColumns:
        behavior = QItemSelectionModel::Columns;
        break;
    }

    // SingleSelection: ClearAndSelect on valid index otherwise NoUpdate
    if (selectionMode() == SingleSelection)
        if (index.isValid())
            return QItemSelectionModel::ClearAndSelect | behavior;
        else
            return QItemSelectionModel::NoUpdate;

    if (selectionMode() == MultiSelection) {
        // NoUpdate on Key movement and Ctrl
        if (type == QEvent::KeyPress
            && (key == Qt::Key_Down
                || key == Qt::Key_Up
                || key == Qt::Key_Left
                || key == Qt::Key_Right
                || key == Qt::Key_Home
                || key == Qt::Key_End
                || key == Qt::Key_PageUp
                || key == Qt::Key_PageDown))
            return QItemSelectionModel::NoUpdate;

        // Select/Deselect on Space
        if (type == QEvent::KeyPress && index.isValid() && key == Qt::Key_Space)
            if (selectionModel()->isSelected(index))
                return QItemSelectionModel::Deselect | behavior;
            else
                return QItemSelectionModel::Select | behavior;

        // Select/Deselect on MouseButtonPress
        if (type == QEvent::MouseButtonPress && index.isValid())
            if (selectionModel()->isSelected(index))
                return QItemSelectionModel::Deselect | behavior;
            else
                return QItemSelectionModel::Select | behavior;

        // Select/Deselect on MouseMove
        if (type == QEvent::MouseMove && index.isValid())
            if (selectionModel()->isSelected(index))
                return QItemSelectionModel::Deselect | behavior;
            else
                return QItemSelectionModel::Select | behavior;

        return QItemSelectionModel::NoUpdate;
    }

    // Toggle on MouseMove
    if (type == QEvent::MouseMove && state & Qt::ControlButton)
        return QItemSelectionModel::ToggleCurrent | behavior;

    // NoUpdate when pressing without modifiers on a selected item
    if (type == QEvent::MouseButtonPress
        && !(d->pressedState & Qt::ShiftButton)
        && !(d->pressedState & Qt::ControlButton)
        && index.isValid()
        && selectionModel()->isSelected(index))
        return QItemSelectionModel::NoUpdate;

    // Clear on MouseButtonPress on non-valid item with no modifiers and not Qt::RightButton
    if (type == QEvent::MouseButtonPress
        && !index.isValid()
        && !(state & Qt::RightButton)
        && !(state & Qt::ShiftButton)
        && !(state & Qt::ControlButton))
        return QItemSelectionModel::Clear;

    // ClearAndSelect on MouseButtonRelease if MouseButtonPress on selected item
    if (type == QEvent::MouseButtonRelease
        && index.isValid()
        && index == d->pressedItem
        && !(d->pressedState & Qt::ShiftButton)
        && !(d->pressedState & Qt::ControlButton)
        && selectionModel()->isSelected(index))
        return QItemSelectionModel::ClearAndSelect | behavior;
    else if (type == QEvent::MouseButtonRelease)
        return QItemSelectionModel::NoUpdate;

    // NoUpdate on Key movement and Ctrl
    if (type == QEvent::KeyPress
        && state & Qt::ControlButton
        && (key == Qt::Key_Down
            || key == Qt::Key_Up
            || key == Qt::Key_Left
            || key == Qt::Key_Right
            || key == Qt::Key_Home
            || key == Qt::Key_End
            || key == Qt::Key_PageUp
            || key == Qt::Key_PageDown))
        return QItemSelectionModel::NoUpdate;

    // Toggle on Ctrl-Qt::Key_Space, Select on Space
    if (type == QEvent::KeyPress && key == Qt::Key_Space)
        if (state & Qt::ControlButton)
            return QItemSelectionModel::Toggle | behavior;
        else
            return QItemSelectionModel::Select | behavior;

    if (state & Qt::ShiftButton)
        return QItemSelectionModel::SelectCurrent | behavior;
    if (state & Qt::ControlButton)
        return QItemSelectionModel::Toggle | behavior;
    if (QAbstractItemView::state() == Selecting)
        return QItemSelectionModel::SelectCurrent | behavior;
    return QItemSelectionModel::ClearAndSelect | behavior;
}

bool QAbstractItemViewPrivate::shouldEdit(QAbstractItemDelegate::BeginEditAction action,
                                          const QModelIndex &index)
{
    if (!model->isEditable(index))
        return false;
    if (state == QAbstractItemView::Editing)
        return false;
    if (action == QAbstractItemDelegate::AlwaysEdit)
        return true;
    if (action & beginEditActions)
        return true;
    if (delegate && delegate->editorType(model, index) == QAbstractItemDelegate::Events)
        return true;
    return d->editors.contains(QPersistentModelIndex(index, model)); // persistent editor
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

QWidget *QAbstractItemViewPrivate::requestEditor(QAbstractItemDelegate::BeginEditAction action,
                                                 QEvent *event, const QModelIndex &index)
{
    if (delegate->editorType(model, index) == QAbstractItemDelegate::Events)
        return 0;

    QPersistentModelIndex persistent = QPersistentModelIndex(index, model);
    QWidget *editor = editors.value(persistent);

    if (!editor) {
        QStyleOptionViewItem option = q->viewOptions();
        option.rect = q->itemViewportRect(index);
        option.state |= (index == q->currentItem() ? QStyle::Style_HasFocus : QStyle::Style_Default);
        editor = delegate->editor(action, viewport, option, model, index);
        delegate->setEditorData(editor, model, index);
        delegate->updateEditorGeometry(editor, option, model, index);
        d->editors.insert(persistent, editor);
    }

    if (editor && event && (action == QAbstractItemDelegate::AnyKeyPressed
                            || event->type() == QEvent::MouseButtonPress))
        QApplication::sendEvent(editor, event);

    return editor;
}

void QAbstractItemViewPrivate::doDelayedItemsLayout()
{
    if (!layoutPosted) {
        int slot = q->metaObject()->indexOfSlot("doItemsLayout()");
        QApplication::postEvent(q, new QMetaCallEvent(slot, q));
        layoutPosted = true;
    }
}

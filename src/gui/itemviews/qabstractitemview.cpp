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
#include <qpointer.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qbitmap.h>
#include <qpair.h>
#include <qmenu.h>
#include <qdrag.h>
#include <qevent.h>
#include <qeventloop.h>
#include <qscrollbar.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qitemdelegate.h>
#include <qdatetime.h>
#include <qrubberband.h>
#include <qdebug.h>

#include <private/qabstractitemview_p.h>
#define d d_func()
#define q q_func()

class QDefaultModel : public QAbstractTableModel
{
public:
    QDefaultModel(QObject *parent) : QAbstractTableModel(parent) {}
    ~QDefaultModel() {}

    int rowCount() const { return 0; }
    int columnCount() const { return 0; }
    QVariant data(const QModelIndex &, int) const { return QVariant(); }
};

QAbstractItemViewPrivate::QAbstractItemViewPrivate()
    :   model(0),
        delegate(0),
        selectionModel(0),
        selectionMode(QAbstractItemView::ExtendedSelection),
        selectionBehavior(QAbstractItemView::SelectItems),
        state(QAbstractItemView::NoState),
        editTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed),
        hasKeyTracking(false),
        inputInterval(400),
        autoScroll(true),
        autoScrollTimer(0),
        autoScrollMargin(16),
        autoScrollInterval(50),
        autoScrollCount(0),
        dragEnabled(false),
        layoutPosted(false),
        alternatingColors(false),
        oddColor(Qt::lightGray),
        evenColor(Qt::white),
        draggableItems(true),
        dropIndicator(0)
{
}

QAbstractItemViewPrivate::~QAbstractItemViewPrivate()
{
}

void QAbstractItemViewPrivate::init()
{
    q->setItemDelegate(new QItemDelegate(q));
    q->setModel(new QDefaultModel(q));

    q->verticalScrollBar()->setRange(0, 0);
    q->horizontalScrollBar()->setRange(0, 0);

    QObject::connect(q->verticalScrollBar(), SIGNAL(actionTriggered(int)),
                     q, SLOT(verticalScrollbarAction(int)));
    QObject::connect(q->horizontalScrollBar(), SIGNAL(actionTriggered(int)),
                     q, SLOT(horizontalScrollbarAction(int)));
    QObject::connect(q->verticalScrollBar(), SIGNAL(valueChanged(int)),
                     q, SLOT(verticalScrollbarValueChanged(int)));
    QObject::connect(q->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                     q, SLOT(horizontalScrollbarValueChanged(int)));

    viewport->setBackgroundRole(QPalette::Base);
    viewport->setAttribute(Qt::WA_NoBackground);

    q->setHorizontalFactor(64);
    q->setVerticalFactor(64);

    dropIndicator = new QRubberBand(QRubberBand::Line, viewport);

    doDelayedItemsLayout();
}

/*!
    \class QAbstractItemView qabstractitemview.h

    \brief The QAbstractItemView class provides the basic functionality for
    item view classes.

    \ingroup model-view

    QAbstractItemView class is the base class for every standard view
    that uses a QAbstractItemModel. QAbstractItemView is an abstract
    class and cannot itself be instantiated. It provides a standard
    interface for interoperating with models through the signals and
    slots mechanism, enabling subclasses to be kept up-to-date with
    changes to their models.  This class provides standard support for
    keyboard and mouse navigation, viewport scrolling, item editing,
    and selections.

    The view classes that inherit QAbstractItemView only need
    to implement their own view-specific functionality, such as
    drawing items, returning the geometry of items, finding items,
    etc.

    QAbstractItemView provides common slots such as edit() and
    setCurrentIndex(), and common signals such as clicked(),
    doubleClicked(), returnPressed(), spacePressed(), and
    deletePressed(). Many protected slots are also provided, including
    dataChanged(), rowsInserted(), rowsAboutToBeRemoved(),
    columnsInserted(), columnsRemoved(),
    selectionChanged(), and currentChanged().

    The root item is returned by root(), and the current item by
    currentIndex(). To make sure that an item is visible use
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
    concerned with editing, for example, edit(), endEdit(), and
    currentEditor(), whilst others are keyboard and mouse event
    handlers.

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemModel

*/

/*!
    \enum QAbstractItemView::SelectionMode

    This enum indicates how the view responds to user selections:

    \value Single  When the user selects an item, any already-selected
    item becomes unselected, and the user cannot unselect the selected
    item.

    \value Multi  When the user selects an item in the usual way, the
    selection status of that item is toggled and the other items are
    left alone.

    \value Extended When the user selects an item in the usual way,
    the selection is cleared and the new item selected. However, if
    the user presses the Ctrl key when clicking on an item, the
    clicked item gets toggled and all other items are left untouched.
    And if the user presses the Shift key while clicking on an item,
    all items between the current item and the clicked item get
    selected or unselected, depending on the state of the clicked
    item. Also, multiple items can be selected by dragging the mouse
    over them.

    \value NoSelection  Items cannot be selected.

    In other words, \c Single is a real single-selection list view, \c
    Multi a real multi-selection list view, \c Extended is a list view
    where users can select multiple items but usually want to select
    either just one or a range of contiguous items, and \c NoSelection
    is a list view where the user can look but not touch.
*/

/*!
    \enum QAbstractItemView::SelectionBehavior

    \value SelectItems   Selecting single items.
    \value SelectRows    Selecting only rows.
    \value SelectColumns Selecting only columns.
*/

/*!
  \enum QAbstractItemView::BeginEditAction

  This enum describes actions which will initiate item editing.

  \value NeverEdit       No editing possible.
  \value CurrentChanged  Editing start whenever current item changes.
  \value DoubleClicked   Editing starts when an item is double clicked.
  \value SelectedClicked Editing starts when clicking on an already selected item.
  \value EditKeyPressed  Editing starts when an edit key has been pressed over an item.
  \value AnyKeyPressed   Editing starts when any key is pressed over an item.
  \value AlwaysEdit      Editing starts for all above actions.
*/

/*!
  \enum QAbstractItemView::CursorAction

  This enum describes the different ways to navigate between items, \sa moveCursor()

  \value MoveUp       Move to the item above the current.
  \value MoveDown     Move to the item below the current.
  \value MoveLeft     Move to the item left of the current.
  \value MoveRight    Move to the item right of the current.
  \value MoveHome     Move to the top-left corner item.
  \value MoveEnd      Move to the bottom-right corner item.
  \value MovePageUp   Move one page up above the current.
  \value MovePageDown Move one page down below the current.
*/

/*!
    \enum QAbstractItemView::State

    Describes the different states the view can be in. This is usually
    only interesting when reimplementing your own view.

    \value NoState        The is the default state.
    \value DraggingState  The user is dragging items.
    \value SelectingState The user is selecting items.
    \value EditingState   The user is editing an item in a widget editor.
    \value OpeningState   The user is opening a branch of items.
    \value ClosingState   The user is closing a branch of items.
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
  \fn void QAbstractItemView::itemEntered(const QModelIndex &index, const QMouseEvent *event)

  This signal is emitted when the mouse cursor enters the item
  specified by \a index.
*/

/*!
  \fn void QAbstractItemView::viewportEntered(QMouseEvent *event)

  This signal is emitted when the mouse cursor enters the
  viewport.
*/

/*!
    \fn void QAbstractItemView::pressed(const QModelIndex &index, const QMouseEvent *event)

    This signal is emitted when a mouse button is pressed. The item the
    mouse was pressed on is specified by \a index (which may be invalid if
    the mouse was not pressed on an item).
*/

/*!
    \fn void QAbstractItemView::clicked(const QModelIndex &index, const QMouseEvent *event)

    This signal is emitted when a mouse button is clicked. The item the
    mouse was clicked on is specified by \a index (which may be invalid if
    the mouse was not clicked on an item).
*/

/*!
    \fn void QAbstractItemView::doubleClicked(const QModelIndex &index, const QMouseEvent *event)

    This signal is emitted when a mouse button is double-clicked. The
    item the mouse was double-clicked on is specified by \a index (which
    may be invalid if the mouse was not double-clicked on an item).
*/

/*!
    \fn void QAbstractItemView::keyPressed(const QModelIndex &index, const QKeyEvent *event)

    This signal is emitted if keyTracking is enabled and a key is
    pressed in the view.  The item \a index is the current item.
*/

/*!
    \fn void QAbstractItemView::returnPressed(const QModelIndex &index)

    This signal is emitted when Return (or Enter) is pressed. The item
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
  \fn bool QAbstractItemView::isIndexHidden(const QModelIndex &index) const

  Returns true if the item refered to by the given \a index is hidden,
  otherwise returns false.

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
    Constructs an abstract item view with the given \a parent.
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
        disconnect(d->selectionModel, SIGNAL(destroyed(QObject*)),
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
        // view
        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(rowsInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(reset()), this, SLOT(reset()));
    }

    d->model = model;

    if (d->model) {
        // view
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(rowsInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(reset()), this, SLOT(reset()));
        // will connect in setSelectionModel
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
    Sets the current selection to the given \a selectionModel.

    \sa selectionModel() clearSelection()
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
        // view
        disconnect(d->selectionModel, SIGNAL(destroyed(QObject*)),
                   this, SLOT(selectionModelDestroyed()));
        disconnect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                   this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
        disconnect(d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                   this, SLOT(currentChanged(QModelIndex,QModelIndex)));
    }

    d->selectionModel = selectionModel;

    if (d->selectionModel) {
        // view
        connect(d->selectionModel, SIGNAL(destroyed(QObject*)),
                this, SLOT(selectionModelDestroyed()));
        connect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
        connect(d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentChanged(QModelIndex,QModelIndex)));
        bool block = d->selectionModel->blockSignals(true);
        if (!currentIndex().isValid())
            setCurrentIndex(d->model->index(0, 0, root()));
        d->selectionModel->blockSignals(block);
    }
}

// ### DOC: Couldn't we call this selection() (and setSelection() and clearSelection()) ?
// This can be confused with actually selecting items.
/*!
    Returns the current selection.

    \sa setSelectionModel() clearSelection()
*/
QItemSelectionModel* QAbstractItemView::selectionModel() const
{
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
    if (d->delegate) {
        // view
        disconnect(d->delegate, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)),
                   this, SLOT(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
        disconnect(d->delegate, SIGNAL(commitData(QWidget*)), this, SLOT(commitData(QWidget*)));
    }
    d->delegate = delegate;
    if (d->delegate) {
        // view
        connect(d->delegate, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)),
                this, SLOT(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
        connect(d->delegate, SIGNAL(commitData(QWidget*)), this, SLOT(commitData(QWidget*)));
    }
}

/*!
    Returns the item delegate used by this view and model. This is
    either one set with setItemDelegate(), or the default one.

    \sa setItemDelegate()
*/
QAbstractItemDelegate *QAbstractItemView::itemDelegate() const
{
    return d->delegate;
}

// ###DOC: this has to be explained in a better way
/*!
  \property QAbstractItemView::selectionMode
  \brief which selection mode the view operates in.

  This property holds whether the user can select
  only one item or several items.

  \sa SelectionMode SelectionBehavior
*/
void QAbstractItemView::setSelectionMode(SelectionMode mode)
{
    d->selectionMode = mode;
}

QAbstractItemView::SelectionMode QAbstractItemView::selectionMode() const
{
    return d->selectionMode;
}

/*!
  \property QAbstractItemView::selectionBehavior
  \brief which selection behavior the view uses.

  This property holds whether selections are done
  in terms of single items, rows or columns.

  \sa SelectionMode SelectionBehavior
*/

void QAbstractItemView::setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior)
{
    d->selectionBehavior = behavior;
}

QAbstractItemView::SelectionBehavior QAbstractItemView::selectionBehavior() const
{
    return d->selectionBehavior;
}

/*!
    Sets the current item to be the itm at \a index.

    \sa currentIndex()
*/
void QAbstractItemView::setCurrentIndex(const QModelIndex &index)
{
    selectionModel()->setCurrentIndex(index, selectionCommand(index, 0));
}

/*!
    Returns the model index of the current item.

    \sa setCurrentIndex()
*/
QModelIndex QAbstractItemView::currentIndex() const
{
    return selectionModel()->currentIndex();
}


/*!
  Reset the internal state of the view.
*/
void QAbstractItemView::reset()
{
    QMap<QPersistentModelIndex, QWidget*>::iterator it = d->editors.begin();
    for (; it != d->editors.end(); ++it) {
        QObject::disconnect(it.value(), SIGNAL(destroyed(QObject*)),
                            this, SLOT(editorDestroyed(QObject*)));
        itemDelegate()->releaseEditor(it.value());
    }
    d->editors.clear();
    d->persistent.clear();
    d->state = NoState;
    if (isVisible())
        doItemsLayout();
     // the views will be updated later
}

/*!
    Sets the root item to the item at \a index.

    \sa root()
*/
void QAbstractItemView::setRoot(const QModelIndex &index)
{
    QModelIndex old = d->root;
    d->root = index;
    emit rootChanged(old, index);
    if (isVisible())
        doItemsLayout();
}

/*!
    Returns the model index of the model's root item. The root item is
    the parent item to the views toplevel items. The root can be invalid.

    \sa setRoot()
*/
QModelIndex QAbstractItemView::root() const
{
    return QModelIndex(d->root);
}

/*!
  Selects all non-hidden items.
*/
void QAbstractItemView::selectAll()
{
    QItemSelection selection;
    QModelIndex tl = model()->index(0, 0, root());
    QModelIndex br = model()->index(model()->rowCount(root()) - 1,
                                    model()->columnCount(root()) - 1,
                                    root());
    selection.append(QItemSelectionRange(tl, br));
    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

/*!
    Starts editing the item item at \a index if it is editable.
*/
void QAbstractItemView::edit(const QModelIndex &index)
{
    if (!index.isValid())
        qWarning("edit: index was invalid");
    if (!edit(index, AllEditTriggers, 0))
        qWarning("edit: editing failed");
}

/*!
    Clears all selected items.
*/
void QAbstractItemView::clearSelection()
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
    \property QAbstractItemView::editTriggers
    \brief which actions will initiate item editing

    This property is a selection of flags defined by
    \l{BeginEditAction}, combined using the OR
    operator. The view will only initiate the editing of an item if the
    action performed is set in this property.
*/
void QAbstractItemView::setEditTriggers(EditTriggers actions)
{
    d->editTriggers = actions;
}

QAbstractItemView::EditTriggers QAbstractItemView::editTriggers() const
{
    return d->editTriggers;
}

/*!
    \property QAbstractItemView::autoScroll
    \brief whether autoscrolling in drag move events is enabled

    If this property is set to true (the default), the
    QAbstractItemView automatically scrolls the contents of the view
    if the user drags within 16 pixels of the viewport edge. This only works if
    the viewport accepts drops. Autoscroll is switched off by setting
    this property to false.
*/
void QAbstractItemView::setAutoScroll(bool enable)
{
    d->autoScroll = enable;
}

bool QAbstractItemView::hasAutoScroll() const
{
    return d->autoScroll;
}

/*!
  \property QAbstractItemView::keyTracking
  \brief whether the view emits a keyPressed signal for every key press.
*/

void QAbstractItemView::setKeyTracking(bool enable)
{
    d->hasKeyTracking = enable;
}

bool QAbstractItemView::hasKeyTracking() const
{
    return d->hasKeyTracking;
}

/*!
  \property QAbstractItemView::alternatingRowColors
  \brief whether to draw the background using alternating colors

  If this property is true, the item background will be drawn using
  alternating colors, otherwise the background will be drawn using the QPalette::Base color.
*/

bool QAbstractItemView::alternatingRowColors() const
{
    return d->alternatingColors;
}

void QAbstractItemView::setAlternatingRowColors(bool enable)
{
    d->alternatingColors = enable;
    if (isVisible())
        d->viewport->update();
}

/*!
  \property QAbstractItemView::oddRowColor
  \brief the color used to draw the background for odd rows
*/

QColor QAbstractItemView::oddRowColor() const
{
    return d->oddColor;
}

void QAbstractItemView::setOddRowColor(const QColor &odd)
{
    d->oddColor = odd;
}

/*!
  \property QAbstractItemView::evenRowColor
  \brief the color used to draw the background for even rows
*/

QColor QAbstractItemView::evenRowColor() const
{
    return d->evenColor;
}

void QAbstractItemView::setEvenRowColor(const QColor &even)
{
    d->evenColor = even;
}

/*!
  \property QAbstractItemView::draggableItems
  \brief whether the items in the view are draggable
*/
bool QAbstractItemView::draggableItems() const
{
    return d->draggableItems;
}

void QAbstractItemView::setDraggableItems(bool draggable)
{
    d->draggableItems = draggable;
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
        if (index.isValid()) {
            QString tooltip = model()->data(index, QAbstractItemModel::ToolTipRole).toString();
            QToolTip::showText(he->globalPos(), tooltip, this);
        }
        return true; }
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        QPoint margins = d->viewport->geometry().topLeft();
        QModelIndex index = itemAt(he->pos() - margins);
        if (index.isValid()) {
            QString whatsthis = model()->data(index, QAbstractItemModel::WhatsThisRole).toString();
            QWhatsThis::showText(he->globalPos(), whatsthis, this);
        }
        return true; }
    case QEvent::StatusTip: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        QPoint margins = d->viewport->geometry().topLeft();
        QModelIndex index = itemAt(he->pos() - margins);
        if (index.isValid()) {
            QString statustip = model()->data(index, QAbstractItemModel::StatusTipRole).toString();
            if (!statustip.isEmpty())
                setStatusTip(statustip);
        }
        return true; }
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        d->viewport->update();
        break;
    case QEvent::KeyPress: {
        // This is to avoid loosing focus on Tab and Backtab
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
            keyPressEvent(ke);
            return ke->isAccepted();
        }
        break;}
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

    if (d->state == EditingState && d->editors.contains(index))
        return;

    bool itemWasSelected = selectionModel()->isSelected(index);
    QPoint offset(horizontalOffset(), verticalOffset());
    d->pressedItem = index;
    d->pressedModifiers = e->modifiers();
    QItemSelectionModel::SelectionFlags command = selectionCommand(index, e);
    if ((command & QItemSelectionModel::Current) == 0)
        d->pressedPosition = pos + offset;

    if (index.isValid())
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);

    QRect rect(d->pressedPosition - offset, pos);
    setSelection(rect.normalize(), command);

    emit pressed(index, e->button(), e->modifiers());
    if (e->button() == Qt::LeftButton && itemWasSelected && selectionModel()->isSelected(index))
        edit(index, SelectedClicked, e);
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

    if (state() == DraggingState) {
        topLeft = d->pressedPosition - QPoint(horizontalOffset(), verticalOffset());
        if ((topLeft - bottomRight).manhattanLength() > QApplication::startDragDistance()) {
            startDrag();
            setState(NoState); // the startDrag will return when the dnd operation is done
            stopAutoScroll();
        }
        return;
    }

    if (d->selectionMode != SingleSelection)
        topLeft = d->pressedPosition - QPoint(horizontalOffset(), verticalOffset());
    else
        topLeft = bottomRight;


    QModelIndex index = itemAt(bottomRight);
    if (state() == EditingState && d->editors.contains(index))
        return;

    if (d->enteredItem != index) {
        if (index.isValid())
            emit itemEntered(index, e->button(), e->modifiers());
        else
            emit viewportEntered(e->button(), e->modifiers());
        d->enteredItem = index;
    } else if (state() == SelectingState) {
        return; // we haven't moved over another item yet
    }

    if (!(e->buttons() & Qt::LeftButton)) // FIXME: what is the logic here ?
        return;

    if (index.isValid()) {
        if (state() != SelectingState) {
            bool dragging = model()->flags(index) & QAbstractItemModel::ItemIsDragEnabled;
            bool selected = selectionModel()->isSelected(index);
            if (dragging && selected && d->draggableItems) {
                setState(DraggingState);
                return;
            }
        }
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    }
    setState(SelectingState);
    setSelection(QRect(topLeft, bottomRight).normalize(),
                 selectionCommand(index, e));
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

    if (state() == EditingState && d->editors.contains(index))
        return;

    selectionModel()->select(index, selectionCommand(index, e));
    if (state() == SelectingState)
        setState(NoState);

    if (index == d->pressedItem)
        emit clicked(index, e->button(), e->modifiers());
    if (e->button() == Qt::RightButton) {
        QContextMenuEvent me(QContextMenuEvent::Mouse, pos);
        QApplication::sendEvent(this, &me);
    }
}

/*!
    This function is called when a mouse double-click event \a e
    occurs. If the double-click is on a valid item it emits the
    doubleClicked() signal and calls edit() on the item.
*/
void QAbstractItemView::mouseDoubleClickEvent(QMouseEvent *e)
{
    QModelIndex index = itemAt(e->pos());
    if (!index.isValid())
        return;
    emit doubleClicked(index, e->button(), e->modifiers());
    edit(index, DoubleClicked, e);
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
        contextMenu.exec(e->globalPos());
}

/*!
    This function is called when drag enter event \a e occurs. If the
    drag is over a valid dropping place (e.g. over an item that
    accepts drops), the event is accepted.

    \sa dropEvent() startDrag()
*/
void QAbstractItemView::dragEnterEvent(QDragEnterEvent *e)
{
    if (d->canDecode(e))
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
    // the ignore by default
    e->ignore();

    QModelIndex index = itemAt(e->pos());
    index = model()->sibling(index.row(), 0, index);

    if (d->canDecode(e)) {
        if (index.isValid()) {
            // update the drag indicator geometry
            QRect rect = itemViewportRect(index);
            rect.setRight(d->viewport->width());
            QRect global(d->viewport->mapToGlobal(rect.topLeft()), rect.size());
            switch (d->position(e->pos(), rect, 2)) {
            case QAbstractItemViewPrivate::Above: {
                QRect geometry(global.left(), global.top() - 2, global.width(), 4);
                if (geometry != d->dropIndicator->geometry())
                    d->dropIndicator->setGeometry(geometry);
                break; }
            case QAbstractItemViewPrivate::Below: {
                QRect geometry(global.left(), global.bottom() - 1, global.width(), 4);
                if (geometry != d->dropIndicator->geometry())
                    d->dropIndicator->setGeometry(geometry);
                break; }
            case QAbstractItemViewPrivate::On: {
                if (model()->flags(index) & QAbstractItemModel::ItemIsDropEnabled) {
                    if (global != d->dropIndicator->geometry()) {
                        d->dropIndicator->setGeometry(global);
                        QRegion top(0, 0, rect.width(), 3);
                        QRegion left(0, 0, 3, rect.height());
                        QRegion bottom(0, rect.height() - 3, rect.width(), 3);
                        QRegion right(rect.width() - 3, 0, 3, rect.height());
                        d->dropIndicator->setMask(top + left + bottom + right);
                    }
                }
                break; }
            }
            if (!d->dropIndicator->isVisible()) {
                d->dropIndicator->show();
                d->dropIndicator->raise();
            }
            e->accept(); // allow dropping on dropenabled items
        } else {
            e->accept(); // allow dropping in empty areas
        }
    }

    if (d->shouldAutoScroll(e->pos()))
        startAutoScroll();

    QViewport::dragMoveEvent(e);
}

/*!

*/
void QAbstractItemView::dragLeaveEvent(QDragLeaveEvent *)
{
    stopAutoScroll();
    d->dropIndicator->hide();
}

/*!
    This function is called when drop event \a e occurs. If there's a
    valid item under the mouse pointer when the drop occurs, the drop
    is accepted.

    \sa startDrag()
*/
void QAbstractItemView::dropEvent(QDropEvent *e)
{
    QPoint pos;
    QModelIndex index;
    // if we drop on the viewport
    if (d->viewport->rect().contains(e->pos())) {
        QPoint center = d->dropIndicator->geometry().center();
        pos = d->viewport->mapFromGlobal(center);
        index = itemAt(pos);
        index = model()->sibling(index.row(), 0, index);
    }
    // if we are allowed to do the drop
    if (model()->supportedDropActions() & e->proposedAction()) {
        // find the parent index and the row to drop after
        int row = -1; // prepend
        if (model()->flags(index) & QAbstractItemModel::ItemIsDropEnabled
            || model()->flags(index.parent()) & QAbstractItemModel::ItemIsDropEnabled) {
            switch(d->position(pos, itemViewportRect(index), 2)) {
            case QAbstractItemViewPrivate::Above:
                row = index.row();
                index = index.parent();
                break;
            case QAbstractItemViewPrivate::Below:
                row = index.row() + 1;
                index = index.parent();
                break;
            case QAbstractItemViewPrivate::On:
                row = model()->rowCount(index); // append
                break;
            }
        }
        if (model()->dropMimeData(e->mimeData(), e->proposedAction(), row, index))
            e->acceptProposedAction();
    }
    stopAutoScroll();
    d->dropIndicator->hide();
}

/*!
    This function is called when focus event \a e occurs and is a
    focus in event.
*/
void QAbstractItemView::focusInEvent(QFocusEvent *e)
{
    QViewport::focusInEvent(e);
    QModelIndex index = currentIndex();
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
    QModelIndex index = currentIndex();
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

    \sa edit()
*/
void QAbstractItemView::keyPressEvent(QKeyEvent *e)
{
    bool hadCurrent = true;
    QModelIndex current = currentIndex();
    if (!current.isValid()) {
        hadCurrent = false;
        setCurrentIndex(model()->index(0, 0, QModelIndex::Null));
    }
    QModelIndex newCurrent = current;
    if (hadCurrent) {
        switch (e->key()) {
        case Qt::Key_Down:
            newCurrent = moveCursor(MoveDown, e->modifiers());
            break;
        case Qt::Key_Up:
            newCurrent = moveCursor(MoveUp, e->modifiers());
            break;
        case Qt::Key_Left:
            newCurrent = moveCursor(MoveLeft, e->modifiers());
            break;
        case Qt::Key_Right:
            newCurrent = moveCursor(MoveRight, e->modifiers());
            break;
        case Qt::Key_Home:
            newCurrent = moveCursor(MoveHome, e->modifiers());
            break;
        case Qt::Key_End:
            newCurrent = moveCursor(MoveEnd, e->modifiers());
            break;
        case Qt::Key_PageUp:
            newCurrent = moveCursor(MovePageUp, e->modifiers());
            break;
        case Qt::Key_PageDown:
            newCurrent = moveCursor(MovePageDown, e->modifiers());
            break;
        case Qt::Key_Tab:
            newCurrent = moveCursor(MoveNext, e->modifiers());
            break;
        case Qt::Key_Backtab:
            newCurrent = moveCursor(MovePrevious, e->modifiers());
            break;
        }

        if (newCurrent != current && newCurrent.isValid()) {
            QItemSelectionModel::SelectionFlags command =
                selectionCommand(newCurrent, e);
            if (command & QItemSelectionModel::Current) {
                setCurrentIndex(newCurrent);
                QPoint offset(horizontalOffset(), verticalOffset());
                QRect rect(d->pressedPosition - offset, itemViewportRect(newCurrent).center());
                setSelection(rect.normalize(), command);
            } else {
                selectionModel()->setCurrentIndex(newCurrent, command);
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
        e->ignore();
        break;
    case Qt::Key_Backtab:
    case Qt::Key_Tab:
        e->accept(); // don't change focus
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        emit returnPressed(currentIndex());
        break;
    case Qt::Key_Space:
        selectionModel()->select(currentIndex(),
                                 selectionCommand(currentIndex(), e));
    case Qt::Key_Delete:
        break;
    case Qt::Key_F2:
        if (!edit(currentIndex(), EditKeyPressed, e))
            e->ignore();
        break;
    case Qt::Key_A:
        if (e->modifiers() & Qt::ControlModifier) {
            selectAll();
            break;
        }
    default:
        if (!e->text().isEmpty()) {
            if (!edit(currentIndex(), AnyKeyPressed, e))
                keyboardSearch(e->text());
        }
        e->ignore();
        break;
    }

    if (d->hasKeyTracking)
        emit keyPressed(currentIndex(), static_cast<Qt::Key>(e->key()), e->modifiers());
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
  This convenience function returns a list of all selected and
  non-hidden item indexes in the view. The list contains no
  duplicates, and is not sorted.

  The default implementation does nothing.
*/
QModelIndexList QAbstractItemView::selectedIndexes() const
{
    return QModelIndexList();
}

/*!
    Starts editing the item at \a index, creating an editor if
    necessary, and returns true if the view's \l{State} is now \c
    EditingState; otherwise returns false. The action that initiated the
    editing is specified by \a action, and the event that was behind this
    is specified by \a event.

    \sa endEdit()
*/
bool QAbstractItemView::edit(const QModelIndex &index,
                             EditTrigger trigger,
                             QEvent *event)
{
    QModelIndex buddy = model()->buddy(index);

    QStyleOptionViewItem options = viewOptions();
    options.rect = itemViewportRect(buddy);
    options.state |= (buddy == currentIndex() ? QStyle::Style_HasFocus : QStyle::Style_None);

    if (event && itemDelegate()->editorEvent(event, options, buddy))
        return true; // the delegate handled the event

    if (!d->shouldEdit(trigger, buddy))
        return false;

    QWidget *editor = d->editor(buddy, options);
    if (!editor)
        return false;

    if (event && event->type() == QEvent::KeyPress
        && d->editTriggers & AnyKeyPressed
        && trigger & AnyKeyPressed)
        QApplication::sendEvent(editor->focusProxy() ? editor->focusProxy() : editor, event);
    d->state = EditingState;
    editor->show();
    editor->setFocus();

    return true;
}

/*!
  \internal
*/
void QAbstractItemView::updateEditorData()
{
    QMap<QPersistentModelIndex, QWidget*>::iterator it = d->editors.begin();
    for (; it != d->editors.end(); ++it)
        itemDelegate()->setEditorData(it.value(), it.key());
}

/*!
    \internal
*/
void QAbstractItemView::updateEditorGeometries()
{
    QStyleOptionViewItem option = viewOptions();
    QMap<QPersistentModelIndex, QWidget*>::iterator it = d->editors.begin();
    for (; it != d->editors.end(); ++it) {
        option.rect = itemViewportRect(it.key());
        if (option.rect.isValid())
            it.value()->show();
        else
            it.value()->hide();
        if (it.key().isValid()) {
            itemDelegate()->updateEditorGeometry(it.value(), option, it.key());
        } else {
            // remove editors in deleted indexes
            itemDelegate()->releaseEditor(it.value());
            d->editors.erase(it);
        }
    }
}

/*!
    \internal
*/
void QAbstractItemView::updateGeometries()
{
    updateEditorGeometries();
    d->fetchMore();
}

/*!
  \internal
*/
void QAbstractItemView::verticalScrollbarValueChanged(int value)
{
    if (verticalScrollBar()->maximum() == value)
        model()->fetchMore(root());
}

/*!
  \internal
*/
void QAbstractItemView::horizontalScrollbarValueChanged(int value)
{
    if (horizontalScrollBar()->maximum() == value)
        model()->fetchMore(root());
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
  Releases the editor \a editor from the view without commiting the
  editors data.

  \sa endEdit()
*/

/*!
    Ends the editing operation on the item represented by the given \a index.
    If there was an editor widget it is released.

    \sa edit() QAbstractItemDelegate::releaseEditor()
*/

void QAbstractItemView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    // close the editor
    if (editor && !d->persistent.contains(editor)) { // if the editor is not persistent, remove it
        d->state = NoState;
        QObject::disconnect(editor, SIGNAL(destroyed(QObject*)),
                            this, SLOT(editorDestroyed(QObject*)));
        d->editors.remove(d->editors.key(editor));
        itemDelegate()->releaseEditor(editor);
    }

    setFocus();

    // The EndEditHint part
    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect
                                                |d->selectionBehaviorFlags();
    switch (hint) {
    case QAbstractItemDelegate::EditNextItem: {
        QModelIndex index = moveCursor(MoveNext, Qt::NoModifier);
        if (index.isValid()) {
            selectionModel()->setCurrentIndex(index, flags);
            edit(index);
        } break; }
    case QAbstractItemDelegate::EditPreviousItem: {
        QModelIndex index = moveCursor(MovePrevious, Qt::NoModifier);
        if (index.isValid()) {
            selectionModel()->setCurrentIndex(index, flags);
            edit(index);
        } break; }
    case QAbstractItemDelegate::SubmitModelCache:
        model()->submit();
        break;
    case QAbstractItemDelegate::RevertModelCache:
        model()->revert();
        break;
    default:
        break;
    }
}

/*!
  Commit the data in the \a editor to the model.

  \sa closeEditor()
*/
void QAbstractItemView::commitData(QWidget *editor)
{
    QModelIndex index = d->editors.key(editor);
    itemDelegate()->setModelData(editor, model(), index);
}

/*!
  Remove the editor \a editor from the map.
*/
void QAbstractItemView::editorDestroyed(QObject *editor)
{
    QWidget *w = ::qt_cast<QWidget*>(editor);
    QPersistentModelIndex key = d->editors.key(w);
    d->editors.remove(key);
    d->persistent.removeAll(w);
    if (d->state == EditingState)
        d->state = NoState;
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
    Sets the vertical scrollbar's stepping \a factor.

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
    QModelIndex start = currentIndex().isValid() ? currentIndex()
                        : model()->index(0, 0, root());
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
        QModelIndex parent = start.parent();
        int newRow = (start.row() < model()->rowCount(parent) - 1) ? start.row() + 1 : 0;
        start = model()->index(newRow, start.column(), parent);
    }

    // search from start with wraparound
    QString searchString = sameKey ? QString(d->keyboardInput.at(0)) : d->keyboardInput;
    QModelIndexList match;
    match = model()->match(start, QAbstractItemModel::DisplayRole, searchString);
    if (!match.isEmpty() && match.at(0).isValid()) {
        selectionModel()->setCurrentIndex(match.at(0),
            (d->selectionMode == SingleSelection
             ? QItemSelectionModel::ClearAndSelect
             : QItemSelectionModel::NoUpdate));
    }
}

/*!
    Returns the size hint for the item with the specified \a index.
*/
QSize QAbstractItemView::itemSizeHint(const QModelIndex &index) const
{
    return itemDelegate()->sizeHint(viewOptions(), index);
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
    QModelIndex index;
    for (int c = 0; c < colCount; ++c) {
        index = d->model->index(row, c, root());
        height = qMax(height, delegate->sizeHint(option, index).height());
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
    QModelIndex index;
    for (int r = 0; r < rows; ++r) {
        index = d->model->index(r, column, root());
        width = qMax(width, delegate->sizeHint(option, index).width());
    }
    return width;
}

/*!
    Opens a persistent editor on the item at the given \a index.
    If no editor exists, the delegate will create a new editor.
*/
void QAbstractItemView::openPersistentEditor(const QModelIndex &index)
{
    QStyleOptionViewItem options = viewOptions();
    options.rect = itemViewportRect(index);
    options.state |= (index == currentIndex() ? QStyle::Style_HasFocus : QStyle::Style_None);

    QWidget *editor = d->editor(index, options);
    if (editor)
        d->persistent.append(editor);
}

/*!
  Closes the persistent editor for the item at the given \a index.
*/
void QAbstractItemView::closePersistentEditor(const QModelIndex &index)
{
    QWidget *editor = d->editors.value(index);
    if (editor) {
        d->persistent.removeAll(editor);
        itemDelegate()->releaseEditor(editor);
    }
    d->editors.remove(index);
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
        if (d->editors.contains(topLeft))
            itemDelegate()->setEditorData(d->editors.value(topLeft), topLeft);
        else
            d->viewport->update(itemViewportRect(topLeft));
        return;
    }
    updateEditorData(); // we are counting on having relatively few editors
    // single row or column changed
    bool sameRow = topLeft.row() == bottomRight.row() && topLeft.isValid();
    bool sameCol = topLeft.column() == bottomRight.column() && topLeft.isValid();
    if (sameRow || sameCol) {
        QRect tl = itemViewportRect(topLeft);
        QRect br = itemViewportRect(bottomRight);
        d->viewport->update(tl.unite(br));
        return;
    }
    // more changed
    d->viewport->update();
}

/*!
    This slot is called when rows are inserted. The new rows are those
    under the given \a parent from \a start to \a end inclusive. The
    base class implementation calls fetchMore() on the model to check
    for more data.

    \sa rowsRemoved()
*/
void QAbstractItemView::rowsInserted(const QModelIndex &, int, int)
{
    d->fetchMore();
}

/*!
    This slot is called when rows are about to be removed. The deleted rows are
    those under the given \a parent from \a start to \a end inclusive.
    The base class implementation does nothing.

    \sa rowsInserted()
*/
void QAbstractItemView::rowsAboutToBeRemoved(const QModelIndex &, int, int)
{
    // do nothing
}

/*!
    This slot is called when the selection is changed. The previous
    selection (which may be empty), is specified by \a deselected, and the
    new selection by \a selected.
*/
void QAbstractItemView::selectionChanged(const QItemSelection &selected,
                                         const QItemSelection &deselected)
{
    QRect deselectedRect = selectionViewportRect(deselected);
    QRect selectedRect = selectionViewportRect(selected);
    d->viewport->repaint(deselectedRect.unite(selectedRect));
}

/*!
    This slot is called when a new item becomes the current item.
    The previous current item is specified by the \a previous index, and the new
    item by the \a current index.

    If you want to know about changes to items see the
    dataChanged() signal.
*/
void QAbstractItemView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (previous.isValid()) {
        // repaint the previous item; if it is not selected, this is the only place to do this
        int behavior = selectionBehavior();
        QRect rect = itemViewportRect(previous);
        if (behavior & SelectRows) {
            rect.setLeft(0);
            rect.setRight(d->viewport->width());
        }
        if (behavior & SelectColumns) {
            rect.setTop(0);
            rect.setBottom(d->viewport->height());
        }
        // painting in the next paint event is too late (because of scrolling)
        d->viewport->repaint(rect);
        // if we are editing, commit the data and close the editor
        if (state() == EditingState) {
            QWidget *editor = d->editors.value(previous);
            if (editor) {
                commitData(editor);
                closeEditor(editor, QAbstractItemDelegate::NoHint);
            }
        }
    }

    if (current.isValid()) {
        ensureItemVisible(current);
        edit(current, CurrentChanged, 0);
    }
}

/*!
    Starts a drag by calling drag->start().
*/
void QAbstractItemView::startDrag()
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    if (indexes.count() > 0) {
        // setup pixmap
        QRect rect = itemViewportRect(indexes.at(0));
        QList<QRect> rects;
        for (int i = 0; i < indexes.count(); ++i) {
            rects.append(itemViewportRect(indexes.at(i)));
            rect |= rects.at(i);
        }
        rect = rect.intersect(d->viewport->rect());
        QPixmap pixmap(rect.size());
        pixmap.fill(palette().base());
        QPainter painter(&pixmap);
        QStyleOptionViewItem option = viewOptions();
        for (int j = 0; j < indexes.count(); ++j) {
            option.rect = QRect(rects.at(j).topLeft() - rect.topLeft(),
                                rects.at(j).size());
            itemDelegate()->paint(&painter, option, indexes.at(j));
        }
        painter.setPen(Qt::black);
        painter.drawRect(0, 0, rect.width() - 1, rect.height() - 1);
        painter.end();
        // create drag object
        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(model()->mimeData(indexes));
        if (drag->start() == QDrag::MoveAction || true)
            d->removeSelectedRows();
    }
}

/*!
    Returns QStyleOptionViewItem structure populated with the view's
    palette, font, state, alignments etc.
*/
QStyleOptionViewItem QAbstractItemView::viewOptions() const
{
    QStyleOptionViewItem option;
    option.palette = palette();
    option.font = font();
    option.state = (isEnabled() ? QStyle::Style_Enabled : QStyle::Style_None);
    option.state |= (state() == EditingState ? QStyle::Style_Editing : QStyle::Style_None);
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
    Sets the item view's state to the given \a state

    \sa state()
*/
void QAbstractItemView::setState(State state)
{
    d->state = state;
}

/*!
  \internal
*/
void QAbstractItemView::startAutoScroll()
{
    if (d->autoScrollTimer)
        killTimer(d->autoScrollTimer);
    d->autoScrollTimer = startTimer(d->autoScrollInterval);
    d->autoScrollCount = 0;
}

/*!
  \internal
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
    // find how much we should scroll with
    int verticalStep = verticalScrollBar()->pageStep();
    int horizontalStep = horizontalScrollBar()->pageStep();
    if (d->autoScrollCount < qMax(verticalStep, horizontalStep))
        ++d->autoScrollCount;

    int margin = d->autoScrollMargin;
    int verticalValue = verticalScrollBar()->value();
    int horizontalValue = horizontalScrollBar()->value();

    QPoint pos = d->viewport->mapFromGlobal(QCursor::pos());
    QRect area = d->viewport->clipRegion().boundingRect();

    // do the scrolling if we are in the scroll margins
    if (pos.y() - area.top() < margin)
        verticalScrollBar()->setValue(verticalValue - d->autoScrollCount);
    else if (area.bottom() - pos.y() < margin)
        verticalScrollBar()->setValue(verticalValue + d->autoScrollCount);
    if (pos.x() - area.left() < margin)
        horizontalScrollBar()->setValue(horizontalValue - d->autoScrollCount);
    else if (area.right() - pos.x() < margin)
        horizontalScrollBar()->setValue(horizontalValue + d->autoScrollCount);
    // if nothing changed, stop scrolling
    bool verticalUnchanged = (verticalValue == verticalScrollBar()->value());
    bool horizontalUnchanged = (horizontalValue == horizontalScrollBar()->value());
    if (verticalUnchanged && horizontalUnchanged)
        stopAutoScroll();
    else
        d->dropIndicator->hide();
}

/*!
    Returns the SelectionFlags to be used when updating selections.
    Reimplement this function to add your own selection behavior.

    This function is called on user input events like mouse and
    keyboard events; the mouse button state is specified by \a state, the
    index of the relevant item by \a index, the even type by \a type,
    and the key (if a key was pressed) by \a key.
*/
QItemSelectionModel::SelectionFlags QAbstractItemView::selectionCommand(const QModelIndex &index,
                                                                        const QEvent *event) const
{
    switch (selectionMode()) {
    case NoSelection: // Never update selection model
        return QItemSelectionModel::NoUpdate;
    case SingleSelection: // ClearAndSelect on valid index otherwise NoUpdate
        if (index.isValid())
            return QItemSelectionModel::ClearAndSelect|d->selectionBehaviorFlags();
        return QItemSelectionModel::NoUpdate;
    case MultiSelection:
        return d->multiSelectionCommand(index, event);
    case ExtendedSelection:
        return d->extendedSelectionCommand(index, event);
    }
    return QItemSelectionModel::NoUpdate;
}

QItemSelectionModel::SelectionFlags QAbstractItemViewPrivate::multiSelectionCommand(
    const QModelIndex &index, const QEvent *event) const
{
    if (!index.isValid())
        return QItemSelectionModel::NoUpdate;

    if (event) {
        switch (event->type()) {
        case QEvent::KeyPress: // NoUpdate on Key movement and Ctrl
            switch (static_cast<const QKeyEvent*>(event)->key()) {
            case Qt::Key_Down:
            case Qt::Key_Up:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                if (static_cast<const QKeyEvent*>(event)->modifiers() & Qt::ControlModifier)
                    return QItemSelectionModel::NoUpdate;
                break;
            case Qt::Key_Space: // Select/Deselect on Space
                if (selectionModel->isSelected(index))
                    return QItemSelectionModel::Deselect|selectionBehaviorFlags();
                return QItemSelectionModel::Select|selectionBehaviorFlags();
            default:
                return QItemSelectionModel::NoUpdate;
            } // switch
            return QItemSelectionModel::NoUpdate;
        case QEvent::MouseButtonPress: // Select/Deselect on MouseButtonPress
            if (selectionModel->isSelected(index))
                return QItemSelectionModel::Deselect|selectionBehaviorFlags();
            return QItemSelectionModel::Select|selectionBehaviorFlags();
        case QEvent::MouseMove: // Select/Deselect on MouseMove
            if (selectionModel->isSelected(index))
                return QItemSelectionModel::Deselect|selectionBehaviorFlags();
            return QItemSelectionModel::Select|selectionBehaviorFlags();
        default:
            break;
        } // switch
    }

    return QItemSelectionModel::ClearAndSelect|selectionBehaviorFlags();
}

QItemSelectionModel::SelectionFlags QAbstractItemViewPrivate::extendedSelectionCommand(
    const QModelIndex &index, const QEvent *event) const
{
    Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
    if (event) {
        switch (event->type()) {
        case QEvent::MouseMove: // Toggle on MouseMove
            modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
            if (modifiers & Qt::ControlModifier)
                return QItemSelectionModel::ToggleCurrent|selectionBehaviorFlags();
            break;
        case QEvent::MouseButtonPress: {// NoUpdate when pressing without modifiers on a selected item
            modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
            if (!(d->pressedModifiers & Qt::ShiftModifier)
                && !(d->pressedModifiers & Qt::ControlModifier)
                && index.isValid()
                && selectionModel->isSelected(index))
                return QItemSelectionModel::NoUpdate;
            // Clear on MouseButtonPress on non-valid item with no modifiers and not Qt::RightButton
            Qt::MouseButton button = static_cast<const QMouseEvent*>(event)->button();
            if (!index.isValid() && !(button & Qt::RightButton)
                && !(modifiers & Qt::ShiftModifier) && !(modifiers & Qt::ControlModifier))
                return QItemSelectionModel::Clear;
            break; }
        case QEvent::MouseButtonRelease: // ClearAndSelect on MouseButtonRelease if MouseButtonPress on selected item
            modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
            if (index.isValid()
                && index == d->pressedItem
                && !(d->pressedModifiers & Qt::ShiftModifier)
                && !(d->pressedModifiers & Qt::ControlModifier)
                && selectionModel->isSelected(index))
                return QItemSelectionModel::ClearAndSelect|selectionBehaviorFlags();
            return QItemSelectionModel::NoUpdate;
        case QEvent::KeyPress: // NoUpdate on Key movement and Ctrl
            modifiers = static_cast<const QKeyEvent*>(event)->modifiers();
            switch (static_cast<const QKeyEvent*>(event)->key()) {
            case Qt::Key_Backtab:
                modifiers ^= Qt::ShiftModifier; // special case for backtab
            case Qt::Key_Down:
            case Qt::Key_Up:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            case Qt::Key_Tab:
                if (modifiers & Qt::ControlModifier)
                    return QItemSelectionModel::NoUpdate;
                break;
            case Qt::Key_Space:// Toggle on Ctrl-Qt::Key_Space, Select on Space
                if (modifiers & Qt::ControlModifier)
                    return QItemSelectionModel::Toggle|selectionBehaviorFlags();
                return QItemSelectionModel::Select|selectionBehaviorFlags();
            default:
                break;
            }
        default:
            break;
        }
    }

    if (modifiers & Qt::ShiftModifier)
        return QItemSelectionModel::SelectCurrent|selectionBehaviorFlags();
    if (modifiers & Qt::ControlModifier)
        return QItemSelectionModel::Toggle|selectionBehaviorFlags();
    if (state == QAbstractItemView::SelectingState)
        return QItemSelectionModel::SelectCurrent|selectionBehaviorFlags();

    return QItemSelectionModel::ClearAndSelect|selectionBehaviorFlags();
}

void QAbstractItemViewPrivate::fetchMore()
{
    int last = model->rowCount(root) - 1;
    if (last < 0)
        return;
    QModelIndex index = model->index(last, 0, root);
    QRect rect = q->itemViewportRect(index);
    if (viewport->rect().contains(rect))
        model->fetchMore(root);
}

bool QAbstractItemViewPrivate::shouldEdit(QAbstractItemView::EditTrigger trigger,
                                          const QModelIndex &index)
{
    if (!index.isValid())
        return false;
    if ((model->flags(index) & QAbstractItemModel::ItemIsEditable) == 0)
        return false;
    if (state == QAbstractItemView::EditingState)
        return false;
    if (trigger == QAbstractItemView::AllEditTriggers)
        return true;
    if (trigger & editTriggers)
        return true;
    return d->editors.contains(index); // persistent editor
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

void QAbstractItemViewPrivate::doDelayedItemsLayout()
{
    if (!layoutPosted) {
        int slot = q->metaObject()->indexOfSlot("doItemsLayout()");
        QApplication::postEvent(q, new QMetaCallEvent(slot, q));
        layoutPosted = true;
    }
}

QWidget *QAbstractItemViewPrivate::editor(const QModelIndex &index,
                                          const QStyleOptionViewItem &options)
{
    QWidget *w = editors.value(index);
    if (!w) {
        w = q->itemDelegate()->editor(viewport, options, index);
        if (w) {
            QObject::connect(w, SIGNAL(destroyed(QObject*)), q, SLOT(editorDestroyed(QObject*)));
            q->itemDelegate()->setEditorData(w, index);
            q->itemDelegate()->updateEditorGeometry(w, options, index);
            editors.insert(index, w);
        }
    }
    return w;
}

void QAbstractItemViewPrivate::removeSelectedRows()
{
    const QItemSelection selection = selectionModel->selection();
    QList<QItemSelectionRange>::const_iterator it = selection.begin();
    for (; it != selection.end(); ++it) {
        QModelIndex parent = (*it).parent();
        if ((*it).left() != 0)
            continue;
        if ((*it).right() != (model->columnCount(parent) - 1))
            continue;
        int count = (*it).bottom() - (*it).top() + 1;
        model->removeRows((*it).top(), parent, count);
    }
}

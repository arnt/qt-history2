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

#ifndef QT_NO_ITEMVIEWS
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
#include <private/qabstractitemview_p.h>

QAbstractItemViewPrivate::QAbstractItemViewPrivate()
    :   model(0),
        delegate(0),
        selectionModel(0),
        selectionMode(QAbstractItemView::ExtendedSelection),
        selectionBehavior(QAbstractItemView::SelectItems),
        pressedModifiers(Qt::NoModifier),
        pressedPosition(QPoint(-1, -1)),
        state(QAbstractItemView::NoState),
        editTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed),
        tabKeyNavigation(false),
#ifndef QT_NO_DRAGANDDROP
        showDropIndicator(false),
        dragEnabled(false),
        dropIndicatorPosition(QAbstractItemView::OnItem),
#endif
        autoScroll(true),
        autoScrollMargin(16),
        autoScrollInterval(50),
        autoScrollCount(0),
        layoutPosted(false),
        alternatingColors(false),
        textElideMode(Qt::ElideRight)
{
}

QAbstractItemViewPrivate::~QAbstractItemViewPrivate()
{
}

void QAbstractItemViewPrivate::init()
{
    Q_Q(QAbstractItemView);
    q->setItemDelegate(new QItemDelegate(q));

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

    q->setHorizontalStepsPerItem(64);
    q->setVerticalStepsPerItem(64);

    doDelayedItemsLayout();
}

/*!
    \class QAbstractItemView qabstractitemview.h

    \brief The QAbstractItemView class provides the basic functionality for
    item view classes.

    \ingroup model-view
    \mainclass

    QAbstractItemView class is the base class for every standard view
    that uses a QAbstractItemModel. QAbstractItemView is an abstract
    class and cannot itself be instantiated. It provides a standard
    interface for interoperating with models through the signals and
    slots mechanism, enabling subclasses to be kept up-to-date with
    changes to their models.  This class provides standard support for
    keyboard and mouse navigation, viewport scrolling, item editing,
    and selections.

    The QAbstractItemView class is one of the \l{Model/View Classes}
    and is part of Qt's \l{Model/View Programming}{model/view framework}.

    The view classes that inherit QAbstractItemView only need
    to implement their own view-specific functionality, such as
    drawing items, returning the geometry of items, finding items,
    etc.

    QAbstractItemView provides common slots such as edit() and
    setCurrentIndex(). Many protected slots are also provided, including
    dataChanged(), rowsInserted(), rowsAboutToBeRemoved(),
    columnsInserted(), columnsRemoved(),
    selectionChanged(), and currentChanged().

    The root item is returned by rootIndex(), and the current item by
    currentIndex(). To make sure that an item is visible use
    scrollTo().

    Some of QAbstractItemView's functions are concerned with
    scrolling, for example setHorizontalFactor() and
    setVerticalFactor(). Several other functions are concerned with
    selection control; for example setSelectionMode(), and
    setSelectionBehavior(). This class provides a default selection
    model to work with (selectionModel()), but this can be replaced
    by using setSelectionModel() with an instance of
    QItemSelectionModel.

    When implimenting a view that will have scrollbars you want to overload
    resizeEvent to set the scrollbars range so they will turn on and off, for example:
    \code
        horizontalScrollBar()->setRange(0, realWidth - width());
    \endcode
    Note that QAbstractScrollView wont turn on/off the scroolbars based upon the
    ranges until the widget is shown.  They should be manually turned
    on and off in if other functions require that information.

    For complete control over the display and editing of items you can
    specify a delegate with setItemDelegate().

    QAbstractItemView provides a lot of protected functions. Some are
    concerned with editing, for example, edit(), and commitData(),
    whilst others are keyboard and mouse event handlers.

    \sa {Model/View Programming}, QAbstractItemModel

*/

/*!
    \enum QAbstractItemView::SelectionMode

    This enum indicates how the view responds to user selections:

    \value SingleSelection  When the user selects an item, any
    already-selected item becomes unselected, and the user cannot
    unselect the selected item.

    \value ContiguousSelection When the user selects an item in the
    usual way, the selection is cleared and the new item selected.
    However, if the user presses the Shift key while clicking on an item,
    all items between the current item and the clicked item are
    selected or unselected, depending on the state of the clicked
    item.

    \value ExtendedSelection When the user selects an item in the
    usual way, the selection is cleared and the new item selected.
    However, if the user presses the Ctrl key when clicking on an
    item, the clicked item gets toggled and all other items are left
    untouched.
    If the user presses the Shift key while clicking on an item,
    all items between the current item and the clicked item are
    selected or unselected, depending on the state of the clicked
    item. Multiple items can be selected by dragging the mouse
    over them.

    \value MultiSelection When the user selects an item in the usual
    way, the selection status of that item is toggled and the other
    items are left alone. Multiple items can be toggled by dragging
    the mouse over them.

    \value NoSelection  Items cannot be selected.

    The most commonly used modes are SingleSelection and
    ExtendedSelection.
*/

/*!
    \enum QAbstractItemView::SelectionBehavior

    \value SelectItems   Selecting single items.
    \value SelectRows    Selecting only rows.
    \value SelectColumns Selecting only columns.
*/

/*!
    \enum QAbstractItemView::ScrollHint

    \value EnsureVisible  Scroll to ensure that the item is visible.
    \value PositionAtTop  Scroll to position the item at the top of the viewport.
    \value PositionAtBottom  Scroll to position the item at the bottom of the viewport.
*/


/*!
  \enum QAbstractItemView::EditTrigger

  This enum describes actions which will initiate item editing.

  \value NoEditTriggers  No editing possible.
  \value CurrentChanged  Editing start whenever current item changes.
  \value DoubleClicked   Editing starts when an item is double clicked.
  \value SelectedClicked Editing starts when clicking on an already selected item.
  \value EditKeyPressed  Editing starts when an edit key has been pressed over an item.
  \value AnyKeyPressed   Editing starts when any key is pressed over an item.
  \value AllEditTriggers Editing starts for all above actions.
*/

/*!
  \enum QAbstractItemView::CursorAction

  This enum describes the different ways to navigate between items, \sa moveCursor()

  \value MoveUp       Move to the item above the current item.
  \value MoveDown     Move to the item below the current item.
  \value MoveLeft     Move to the item left of the current item.
  \value MoveRight    Move to the item right of the current item.
  \value MoveHome     Move to the top-left corner item.
  \value MoveEnd      Move to the bottom-right corner item.
  \value MovePageUp   Move one page up above the current item.
  \value MovePageDown Move one page down below the current item.
  \value MoveNext     Move to the item after the current item.
  \value MovePrevious Move to the item before the current item.
*/

/*!
    \enum QAbstractItemView::State

    Describes the different states the view can be in. This is usually
    only interesting when reimplementing your own view.

    \value NoState        The is the default state.
    \value DraggingState  The user is dragging items.
    \value DragSelectingState The user is selecting items.
    \value EditingState   The user is editing an item in a widget editor.
    \value ExpandingState   The user is opening a branch of items.
    \value CollapsingState   The user is closing a branch of items.
*/

/*!
    \fn QRect QAbstractItemView::visualRect(const QModelIndex &index) const = 0
    Returns the rectangle on the viewport occupied by the item at \a index.

    If your item is displayed in several areas then visualRect should return
    the primary area that contains index and not the complete area that index
    might encompasses, touch or cause drawing.

    In the base class this is a pure virtual function.
*/

/*!
    \fn void QAbstractItemView::scrollTo(const QModelIndex &index, ScrollHint hint) = 0

    Scrolls the view if necessary to ensure that the item at \a index
    is visible. The view will try to position the item according to the given \a hint.

    In the base class this is a pure virtual function.
*/

/*!
    \fn QModelIndex QAbstractItemView::indexAt(const QPoint &point) const

    \overload

    Returns the model index of the item at point \a point.

    In the base class this is a pure virtual function.
*/

/*!
    \fn void QAbstractItemView::activated(const QModelIndex &index)

    This signal is emitted when the item specified by \a index is
    activated by the user (e.g., by single- or double-clicking the
    item, depending on the platform).

    \sa clicked(), doubleClicked(), entered(), pressed()
*/

/*!
    \fn void QAbstractItemView::entered(const QModelIndex &index)

    This signal is emitted when the mouse cursor enters the item
    specified by \a index.
    Mouse tracking needs to be enabled for this feature to work.

    \sa viewportEntered(), activated(), clicked(), doubleClicked(), pressed()
*/

/*!
    \fn void QAbstractItemView::viewportEntered()

    This signal is emitted when the mouse cursor enters the viewport.
    Mouse tracking needs to be enabled for this feature to work.

    \sa entered()
*/

/*!
    \fn void QAbstractItemView::pressed(const QModelIndex &index)

    This signal is emitted when a mouse button is pressed. The item
    the mouse was pressed on is specified by \a index. The signal is
    only emitted when the index is valid.

    \sa activated(), clicked(), doubleClicked(), entered()
*/

/*!
    \fn void QAbstractItemView::clicked(const QModelIndex &index)

    This signal is emitted when a mouse button is clicked. The item
    the mouse was clicked on is specified by \a index. The signal is
    only emitted when the index is valid.

    \sa activated(), doubleClicked(), entered(), pressed()
*/

/*!
    \fn void QAbstractItemView::doubleClicked(const QModelIndex &index)

    This signal is emitted when a mouse button is double-clicked. The
    item the mouse was double-clicked on is specified by \a index.
    The signal is only emitted when the index is valid.
*/

/*!
    \fn QModelIndex QAbstractItemView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) = 0

    Moves the cursor in the view according to the given \a cursorAction and
    keyboard modifiers specified by \a modifiers.
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

  Returns true if the item refered to by the given \a index is hidden in the view,
  otherwise returns false.

  Hiding is a view specific feature.  For example in TableView a column can be marked
  as hidden or a row in the TreeView.

  In the base class this is a pure virtual function.
*/

/*!
    \fn void QAbstractItemView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)

    Applies the selection \a flags to the items in or touched by the
    rectangle, \a rect.

    When implementing your own itemview setSelection should call
    selectionModel()->select(selection, flags) where selection
    is either an empty QModelIndex or a QItemSelection that contains
    all items that are contained in \a rect.

    \sa selectionCommand()
*/

/*!
    \fn QRegion QAbstractItemView::visualRegionForSelection(const QItemSelection &selection) const = 0

    Returns the region from the viewport of the items in the given
    \a selection.
*/

/*!
    Constructs an abstract item view with the given \a parent.
*/
QAbstractItemView::QAbstractItemView(QWidget *parent)
    : QAbstractScrollArea(*(new QAbstractItemViewPrivate), parent)
{
    d_func()->init();
}

/*!
    \internal
*/
QAbstractItemView::QAbstractItemView(QAbstractItemViewPrivate &dd, QWidget *parent)
    : QAbstractScrollArea(dd, parent)
{
    d_func()->init();
}

/*!
    Destroys the view.
*/
QAbstractItemView::~QAbstractItemView()
{
}

/*!
  Sets the \a model for the view to present.
  \omit
  This function will also create and set a new selection model.
  \endomit
*/
void QAbstractItemView::setModel(QAbstractItemModel *model)
{
    Q_D(QAbstractItemView);
    if (d->model) {
        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(rowsInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(columnsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(modelReset()), this, SLOT(reset()));
        disconnect(d->model, SIGNAL(layoutChanged()), this, SLOT(doItemsLayout()));
    }

    d->model = model;

    if (d->model) {
        // These asserts do basic sanity checking of the model
        
        // A model should return the same index, including its internal id/pointer.
        Q_ASSERT(model->index(0,0) == model->index(0,0));
        // The parent of a top level index should be invalid.
        Q_ASSERT(model->index(0,0).parent() == QModelIndex());

        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(rowsInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(columnsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(modelReset()), this, SLOT(reset()));
        connect(d->model, SIGNAL(layoutChanged()), this, SLOT(doItemsLayout()));
    }


    setSelectionModel(new QItemSelectionModel(d->model, this));
    reset(); // kill editors, set new root and do layout
}

/*!
    Returns the model that this view is presenting.
*/
QAbstractItemModel *QAbstractItemView::model() const
{
    return d_func()->model;
}

/*!
    Sets the current selection to the given \a selectionModel.

    \sa selectionModel() clearSelection()
*/
void QAbstractItemView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    Q_ASSERT(selectionModel);
    Q_D(QAbstractItemView);

    if (selectionModel->model() != model()) {
        qWarning("QAbstractItemView::setSelectionModel() failed: "
                 "Trying to set a selection model, which works on "
                 "a different model than the view.");
        return;
    }

    if (d->selectionModel) {
        disconnect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                   this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
        disconnect(d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                   this, SLOT(currentChanged(QModelIndex,QModelIndex)));
    }

    d->selectionModel = selectionModel;

    if (d->selectionModel) {
        connect(d->selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
        connect(d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentChanged(QModelIndex,QModelIndex)));
    }
}

/*!
    Returns the current selection.

    \sa setSelectionModel() clearSelection()
*/
QItemSelectionModel* QAbstractItemView::selectionModel() const
{
    return d_func()->selectionModel;
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
    Q_D(QAbstractItemView);

    if (d->delegate) {
        disconnect(d->delegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                   this, SLOT(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
        disconnect(d->delegate, SIGNAL(commitData(QWidget*)), this, SLOT(commitData(QWidget*)));
    }

    d->delegate = delegate;

    if (d->delegate) {
        connect(d->delegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                this, SLOT(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
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
    return d_func()->delegate;
}

/*!
  \property QAbstractItemView::selectionMode
  \brief which selection mode the view operates in

  This property controls whether the user can select one or many items
  and, in many-item selections, whether the selection must be a
  continuous range of items.

  \sa SelectionMode SelectionBehavior
*/
void QAbstractItemView::setSelectionMode(SelectionMode mode)
{
    d_func()->selectionMode = mode;
}

QAbstractItemView::SelectionMode QAbstractItemView::selectionMode() const
{
    return d_func()->selectionMode;
}

/*!
  \property QAbstractItemView::selectionBehavior
  \brief which selection behavior the view uses

  This property holds whether selections are done
  in terms of single items, rows or columns.

  \sa SelectionMode SelectionBehavior
*/

void QAbstractItemView::setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior)
{
    d_func()->selectionBehavior = behavior;
}

QAbstractItemView::SelectionBehavior QAbstractItemView::selectionBehavior() const
{
    return d_func()->selectionBehavior;
}

/*!
    Sets the current item to be the item at \a index.
    Depending on the current selection mode, the item may also be selected.

    To set an item as the current item without selecting it, call

    \c{selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);}

    \sa currentIndex(), setSelectionMode(), selectionMode()
*/
void QAbstractItemView::setCurrentIndex(const QModelIndex &index)
{
    if (selectionModel())
        selectionModel()->setCurrentIndex(index, selectionCommand(index, 0));
}

/*!
    Returns the model index of the current item.

    \sa setCurrentIndex()
*/
QModelIndex QAbstractItemView::currentIndex() const
{
    return selectionModel() ? selectionModel()->currentIndex() : QModelIndex();
}


/*!
  Reset the internal state of the view.
*/
void QAbstractItemView::reset()
{
    Q_D(QAbstractItemView);
    QMap<QPersistentModelIndex, QPointer<QWidget> >::iterator it = d->editors.begin();
    for (; it != d->editors.end(); ++it) {
        QWidget *editor = it.value();
        editor->hide();
        d->releaseEditor(editor);
    }
    d->editors.clear();
    d->persistent.clear();
    setState(NoState);
    setRootIndex(QModelIndex());
}

/*!
    Sets the root item to the item at the given \a index.

    \sa rootIndex()
*/
void QAbstractItemView::setRootIndex(const QModelIndex &index)
{
    Q_D(QAbstractItemView);
    d->root = index;
    d->doDelayedItemsLayout();
}

/*!
    Returns the model index of the model's root item. The root item is
    the parent item to the views toplevel items. The root can be invalid.

    \sa setRootIndex()
*/
QModelIndex QAbstractItemView::rootIndex() const
{
    return QModelIndex(d_func()->root);
}

/*!
  Selects all non-hidden items.
*/
void QAbstractItemView::selectAll()
{
    if (!model() || !selectionModel())
        return;

    QItemSelection selection;
    QModelIndex tl = model()->index(0, 0, rootIndex());
    QModelIndex br = model()->index(model()->rowCount(rootIndex()) - 1,
                                    model()->columnCount(rootIndex()) - 1,
                                    rootIndex());
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
    if (selectionModel())
        selectionModel()->clear();
}

/*!
    \internal

    This function is intended to lay out the items in the view.
    The default implementatiidon just calls updateGeometries() and updates the viewport.
*/
void QAbstractItemView::doItemsLayout()
{
    d_func()->layoutPosted = false;
    updateGeometries();
    d_func()->viewport->update();
}

/*!
    \property QAbstractItemView::editTriggers
    \brief which actions will initiate item editing

    This property is a selection of flags defined by
    \l{EditTrigger}, combined using the OR
    operator. The view will only initiate the editing of an item if the
    action performed is set in this property.
*/
void QAbstractItemView::setEditTriggers(EditTriggers actions)
{
    d_func()->editTriggers = actions;
}

QAbstractItemView::EditTriggers QAbstractItemView::editTriggers() const
{
    return d_func()->editTriggers;
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
    d_func()->autoScroll = enable;
}

bool QAbstractItemView::hasAutoScroll() const
{
    return d_func()->autoScroll;
}

/*!
  \property QAbstractItemView::tabKeyNavigation
  \brief whether item navigation with tab and backtab is enabled.
*/

void QAbstractItemView::setTabKeyNavigation(bool enable)
{
    d_func()->tabKeyNavigation = enable;
}

bool QAbstractItemView::tabKeyNavigation() const
{
    return d_func()->tabKeyNavigation;
}

#ifndef QT_NO_DRAGANDDROP
/*!
  \property QAbstractItemView::showDropIndicator
  \brief whether the drop indicator is shown when dragging items and dropping.
*/

void QAbstractItemView::setDropIndicatorShown(bool enable)
{
    d_func()->showDropIndicator = enable;
}

bool QAbstractItemView::showDropIndicator() const
{
    return d_func()->showDropIndicator;
}

/*!
  \property QAbstractItemView::dragEnabled
  \brief whether the view supports dragging of its own items
*/

void QAbstractItemView::setDragEnabled(bool enable)
{
    d_func()->dragEnabled = enable;
}

bool QAbstractItemView::dragEnabled() const
{
    return d_func()->dragEnabled;
}
#endif // QT_NO_DRAGANDDROP

/*!
  \property QAbstractItemView::alternatingRowColors
  \brief whether to draw the background using alternating colors

  If this property is true, the item background will be drawn using
  QPalette::Base and QPalette::AlternateBase; otherwise the background
  will be drawn using the QPalette::Base color.

  By default, this property is false.
*/

void QAbstractItemView::setAlternatingRowColors(bool enable)
{
    d_func()->alternatingColors = enable;
    if (isVisible())
        d_func()->viewport->update();
}

bool QAbstractItemView::alternatingRowColors() const
{
    return d_func()->alternatingColors;
}

/*!
    \property QAbstractItemView::iconSize
    \brief the size of items

    Setting this property when the view is visible will cause the
    items to be laid out again.
*/
void QAbstractItemView::setIconSize(const QSize &size)
{
    d_func()->iconSize = size;
    d_func()->doDelayedItemsLayout();
}

QSize QAbstractItemView::iconSize() const
{
    return d_func()->iconSize;
}

/*!
    \property QAbstractItemView::textElideMode
    \brief the the position of the "..." in elided text.
*/
void QAbstractItemView::setTextElideMode(Qt::TextElideMode mode)
{
    d_func()->textElideMode = mode;
}

Qt::TextElideMode QAbstractItemView::textElideMode() const
{
    return d_func()->textElideMode;
}

/*!
  \reimp
*/
bool QAbstractItemView::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress && d_func()->tabKeyNavigation) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
            keyPressEvent(keyEvent);
            return keyEvent->isAccepted();
        }
    }
    return QAbstractScrollArea::event(event);
}

/*!
    \fn bool QAbstractItemView::viewportEvent(QEvent *event)

    This function is used to handle tool tips, status tips, and What's
    This? mode, if the given \a event is a QEvent::ToolTip, a
    QEvent::WhatsThis, or a QEvent::StatusTip. It passes all other
    events on to its base class viewportEvent() handler.
*/
bool QAbstractItemView::viewportEvent(QEvent *event)
{
    Q_D(QAbstractItemView);
    switch (event->type()) {
    case QEvent::HoverEnter: {
        QHoverEvent *he = static_cast<QHoverEvent*>(event);
        d->hover = indexAt(he->pos());
        d->viewport->update(visualRect(d->hover));
        break; }
    case QEvent::HoverLeave: {
        d->viewport->update(visualRect(d->hover)); // update old
        d->hover = QModelIndex();
        break; }
    case QEvent::HoverMove: {
        QHoverEvent *he = static_cast<QHoverEvent*>(event);
        QModelIndex old = d->hover;
        d->hover = indexAt(he->pos());
        if (d->hover != old)
            d->viewport->update(visualRect(old)|visualRect(d->hover));
        break; }
#ifndef QT_NO_TOOLTIP
    case QEvent::ToolTip: {
        if (!isActiveWindow())
            break;
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        QModelIndex index = indexAt(he->pos());
        if (index.isValid()) {
            QString tooltip = model()->data(index, Qt::ToolTipRole).toString();
            QToolTip::showText(he->globalPos(), tooltip, this);
            return true;
        }
        else {
            QString emptyString;
            QToolTip::showText(he->globalPos(), emptyString, this);
        }
        break;}
#endif
#ifndef QT_NO_WHATSTHIS
    case QEvent::QueryWhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        QModelIndex index = indexAt(he->pos());
        if (index.isValid() && model()->data(index, Qt::WhatsThisRole).isValid())
            return true;
        break ; }
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        QModelIndex index = indexAt(he->pos());
        if (index.isValid()) {
            QString whatsthis = model()->data(index, Qt::WhatsThisRole).toString();
            QWhatsThis::showText(he->globalPos(), whatsthis, this);
            return true;
        }
        break ; }
#endif
    case QEvent::FontChange:
        d->doDelayedItemsLayout(); // the size of the items will change
        break;
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        d->viewport->update();
        break;
    default:
        break;
    }
    return QAbstractScrollArea::viewportEvent(event);
}

/*!
    This function is called with the given \a event when a mouse button is pressed
    while the cursor is inside the widget. If a valid item is pressed on it is made
    into the current item. This function emits the pressed() signal.
*/
void QAbstractItemView::mousePressEvent(QMouseEvent *event)
{
    Q_D(QAbstractItemView);
    QPoint pos = event->pos();
    QModelIndex index = indexAt(pos);

    if (!selectionModel() || (d->state == EditingState && d->editors.contains(index)))
        return;

    bool alreadySelected = selectionModel()->isSelected(index);
    d->pressedIndex = index;
    d->pressedModifiers = event->modifiers();
    QItemSelectionModel::SelectionFlags command = selectionCommand(index, event);
    QPoint offset(horizontalOffset(), verticalOffset());
    if ((command & QItemSelectionModel::Current) == 0)
        d->pressedPosition = pos + offset;
    
    if (d->pressedPosition == QPoint(-1, -1))
        d->pressedPosition = visualRect(selectionModel()->currentIndex()).center() + offset;
    QRect rect(d->pressedPosition - offset, pos);
    setSelection(rect.normalized(), command);
    
    if (index.isValid())
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);

    // signal handlers may change the model
    if (index.isValid()) {
        QPersistentModelIndex persistent = index;
        emit pressed(index);
        index = persistent;
        if (alreadySelected)
            edit(index, SelectedClicked, event);
    }
}

/*!
    This function is called with the given \a event when a mouse move event is
    sent to the widget. If a selection is in progress and new items are moved
    over the selection is extended; if a drag is in progress it is continued.
*/
void QAbstractItemView::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QAbstractItemView);
    QPoint topLeft;
    QPoint bottomRight = event->pos();

    if (state() == ExpandingState || state() == CollapsingState)
        return;
#ifndef QT_NO_DRAGANDDROP
    if (state() == DraggingState) {
        topLeft = d->pressedPosition - QPoint(horizontalOffset(), verticalOffset());
        if ((topLeft - bottomRight).manhattanLength() > QApplication::startDragDistance()) {
            startDrag(model()->supportedDropActions());
            setState(NoState); // the startDrag will return when the dnd operation is done
            stopAutoScroll();
        }
        return;
    }
#endif // QT_NO_DRAGANDDROP

    if (d->selectionMode != SingleSelection)
        topLeft = d->pressedPosition - QPoint(horizontalOffset(), verticalOffset());
    else
        topLeft = bottomRight;

    QModelIndex index = indexAt(bottomRight);
    QModelIndex buddy = model() ? model()->buddy(d->pressedIndex) : QModelIndex();
    if (state() == EditingState && d->editors.contains(buddy))
        return;

    if (d->enteredIndex != index) {
        // signal handlers may change the model
        QPersistentModelIndex persistent = index;
        if (index.isValid()){
            emit entered(index);
#ifndef QT_NO_STATUSTIP
            QString statustip = model()->data(index, Qt::StatusTipRole).toString();
            if (parent() && !statustip.isEmpty()) {
                QStatusTipEvent tip(statustip);
                QApplication::sendEvent(parent(), &tip);
            }
#endif
        } else {
#ifndef QT_NO_STATUSTIP
            if (parent()) {
                QString emptyString;
                QStatusTipEvent tip(emptyString);
                QApplication::sendEvent(parent(), &tip);
            }
#endif
            emit viewportEntered();
        }
        d->enteredIndex = persistent;
        index = persistent;
    }

#ifndef QT_NO_DRAGANDDROP
    if (index.isValid() && d->dragEnabled && state() != DragSelectingState) {
        bool dragging = model()->flags(index) & Qt::ItemIsDragEnabled;
        bool selected = selectionModel()->isSelected(index);
        if (dragging && selected) {
            setState(DraggingState);
            return;
        }
    }
#endif

    if ((event->buttons() & Qt::LeftButton) && d->selectionAllowed(index) && selectionModel()) {
        setState(DragSelectingState);
        QItemSelectionModel::SelectionFlags command = selectionCommand(index, event);

        // Do the normalize ourselves, since QRect::normalized() is flawed
        if (topLeft.y() > bottomRight.y()) qSwap(topLeft.ry(), bottomRight.ry());
        if (topLeft.x() > bottomRight.x()) qSwap(topLeft.rx(), bottomRight.rx());
        QRect selectionRect = QRect(topLeft, bottomRight);
        setSelection(selectionRect, command);

        // set at the end because it might scroll the view
        if (index.isValid())
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    }
}

/*!
    This function is called with the given \a event when a mouse button is released
    while the cursor is inside the widget. It will emit the clicked() signal if an
    item was being pressed.
*/
void QAbstractItemView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QAbstractItemView);
    d->pressedPosition = QPoint(-1, -1);

    QPoint pos = event->pos();
    QModelIndex index = indexAt(pos);

    if (state() == EditingState)
        return;

    setState(NoState);

    if (selectionModel())
        selectionModel()->select(index, selectionCommand(index, event));
    
    if (index == d_func()->pressedIndex && index.isValid()) {
        // signal handlers may change the model
        QPersistentModelIndex persistent = index;
        emit clicked(persistent);
        if (edit(persistent, NoEditTriggers, event)) // send event to delegate
            return;
        if (style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
            emit activated(persistent);
    }
}

/*!
    This function is called with the given \a event when a mouse button is
    double clicked inside the widget. If the double-click is on a valid item it
    emits the doubleClicked() signal and calls edit() on the item.
*/
void QAbstractItemView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
        return;
    // signal handlers may change the model
    QPersistentModelIndex persistent = index;
    emit doubleClicked(persistent);
    if (!edit(persistent, DoubleClicked, event)
        && !style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
        emit activated(persistent);
}

#ifndef QT_NO_DRAGANDDROP

/*!
    This function is called with the given \a event when a drag and drop operation enters
    the widget. If the drag is over a valid dropping place (e.g. over an item that
    accepts drops), the event is accepted; otherwise it is ignored.

    \sa dropEvent() startDrag()
*/
void QAbstractItemView::dragEnterEvent(QDragEnterEvent *event)
{
    if (d_func()->canDecode(event)) {
        event->accept();
        setState(DraggingState);
    } else {
        event->ignore();
    }
}

/*!
    This function is called continuously with the given \a event during a drag and
    drop operation over the widget. It can cause the view to scroll if, for example,
    the user drags a selection to view's right or bottom edge. In this case, the
    event will be accepted; otherwise it will be ignored.

    \sa dropEvent() startDrag()
*/
void QAbstractItemView::dragMoveEvent(QDragMoveEvent *event)
{
    Q_D(QAbstractItemView);
    // the ignore by default
    event->ignore();

    if (!model())
        return;

    QModelIndex index = indexAt(event->pos());
    if (d->canDecode(event)) {
        if (index.isValid() && d->showDropIndicator) {
            QRect rect = visualRect(index);
            d->dropIndicatorPosition = d->position(event->pos(), rect, 2);
            switch (d->dropIndicatorPosition) {
            case AboveItem:
                d->dropIndicatorRect = QRect(rect.left(), rect.top(), rect.width(), 0);
                event->accept();
                break;
            case BelowItem:
                d->dropIndicatorRect = QRect(rect.left(), rect.bottom(), rect.width(), 0);
                event->accept();
                break;
            case OnItem:
                if (model()->flags(index) & Qt::ItemIsDropEnabled) {
                    d->dropIndicatorRect = rect;
                    event->accept();
                }
		break;
	    case OnViewport:
	        break;
            }
            update();
        } else {
            d->dropIndicatorRect = QRect();
            d->dropIndicatorPosition = QAbstractItemView::OnViewport;
            event->accept(); // allow dropping in empty areas
        }
    } // can decode

    if (d->shouldAutoScroll(event->pos()))
        startAutoScroll();
}

/*!
    \fn void QAbstractItemView::dragLeaveEvent(QDragLeaveEvent *event)

    This function is called when the item being dragged leaves the view.
    The \a event describes the state of the drag and drop operation.
*/
void QAbstractItemView::dragLeaveEvent(QDragLeaveEvent *)
{
    stopAutoScroll();
    setState(NoState);
    update();
}

/*!
    This function is called with the given \a event when a drop event occurs over
    the widget. If there's a valid item under the mouse pointer when the drop
    occurs, the drop event is accepted; otherwise it is ignored.

    \sa startDrag()
*/
void QAbstractItemView::dropEvent(QDropEvent *event)
{
    Q_D(QAbstractItemView);
    QModelIndex index;
    // if we drop on the viewport
    if (d->viewport->rect().contains(event->pos())) {
        index = indexAt(event->pos());
        if (!index.isValid())
            index = rootIndex(); // drop on viewport
    }
    // if we are allowed to do the drop
    if (model()->supportedDropActions() & event->proposedAction()) {
        int row = -1;
        int col = -1;
        if (index.isValid() &&
            (model()->flags(index) & Qt::ItemIsDropEnabled
             || model()->flags(index.parent()) & Qt::ItemIsDropEnabled)) {
            d->dropIndicatorPosition = d->position(event->pos(), visualRect(index), 2);
            switch (d->dropIndicatorPosition) {
            case AboveItem:
                row = index.row();
                col = index.column();
                index = index.parent();
                break;
            case BelowItem:
                row = index.row() + 1;
                col = index.column();
                index = index.parent();
                break;
            case OnItem:
            case OnViewport:
                break;
            }
        } else {
            d->dropIndicatorPosition = QAbstractItemView::OnViewport;
        }
        if (model()->dropMimeData(event->mimeData(), event->proposedAction(), row, col, index))
            event->acceptProposedAction();
    }
    stopAutoScroll();
    setState(NoState);
    update();
}

#endif // QT_NO_DRAGANDDROP

/*!
    This function is called with the given \a event when the widget obtains the focus.
    By default, the event is ignored.

    \sa setFocus(), focusOutEvent()
*/
void QAbstractItemView::focusInEvent(QFocusEvent *event)
{
    QAbstractScrollArea::focusInEvent(event);
    QModelIndex index = currentIndex();
    if (index.isValid())
        d_func()->viewport->update(visualRect(index));
}

/*!
    This function is called with the given \a event when the widget obtains the focus.
    By default, the event is ignored.

    \sa clearFocus(), focusInEvent()
*/
void QAbstractItemView::focusOutEvent(QFocusEvent *event)
{
    QAbstractScrollArea::focusOutEvent(event);
    QModelIndex index = currentIndex();
    if (index.isValid())
        d_func()->viewport->update(visualRect(index));
}

/*!
    This function is called with the given \a event when a key event is sent to
    the widget. The default implementation handles basic cursor movement, e.g. Up,
    Down, Left, Right, Home, PageUp, and PageDown, and emits the returnPressed(),
    spacePressed(), and deletePressed() signals if the associated key is pressed.
    This function is where editing is initiated by key press, e.g. if F2 is
    pressed.

    \sa edit()
*/
void QAbstractItemView::keyPressEvent(QKeyEvent *event)
{
    Q_D(QAbstractItemView);
    if (!model())
        return;

#ifdef QT_KEYPAD_NAVIGATION
    switch( event->key() ) {
    case Qt::Key_Select:
        if (QApplication::keypadNavigationEnabled()) {
            if (!hasEditFocus()) {
                setEditFocus(true);
                return;
            }
        }
        break;
    case Qt::Key_Back:
        if (QApplication::keypadNavigationEnabled() && hasEditFocus())
            setEditFocus(false);
        else
            event->ignore();
        return;
    default:
        if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
            event->ignore();
            return;
        }
    }
#endif

    QModelIndex current = currentIndex();
    QModelIndex newCurrent;
    switch (event->key()) {
    case Qt::Key_Down:
        newCurrent = moveCursor(MoveDown, event->modifiers());
        break;
    case Qt::Key_Up:
        newCurrent = moveCursor(MoveUp, event->modifiers());
        break;
    case Qt::Key_Left:
        newCurrent = moveCursor(MoveLeft, event->modifiers());
        break;
    case Qt::Key_Right:
        newCurrent = moveCursor(MoveRight, event->modifiers());
        break;
    case Qt::Key_Home:
        newCurrent = moveCursor(MoveHome, event->modifiers());
        break;
    case Qt::Key_End:
        newCurrent = moveCursor(MoveEnd, event->modifiers());
        break;
    case Qt::Key_PageUp:
        newCurrent = moveCursor(MovePageUp, event->modifiers());
        break;
    case Qt::Key_PageDown:
        newCurrent = moveCursor(MovePageDown, event->modifiers());
        break;
    case Qt::Key_Tab:
        newCurrent = moveCursor(MoveNext, event->modifiers());
        break;
    case Qt::Key_Backtab:
        newCurrent = moveCursor(MovePrevious, event->modifiers());
        break;
    }

    if (newCurrent != current && newCurrent.isValid()) {
        QItemSelectionModel::SelectionFlags command = selectionCommand(newCurrent, event);
        if (command & QItemSelectionModel::Current) {
            selectionModel()->setCurrentIndex(newCurrent, QItemSelectionModel::NoUpdate);
            QPoint offset(horizontalOffset(), verticalOffset());
            QRect rect(d->pressedPosition - offset, visualRect(newCurrent).center());
            setSelection(rect.normalized(), command);
        } else {
            selectionModel()->setCurrentIndex(newCurrent, command);
            QPoint offset(horizontalOffset(), verticalOffset());
            d->pressedPosition = visualRect(newCurrent).center() + offset;
        }
        return;
    }

    switch (event->key()) {
    // ignored keys
    case Qt::Key_Down:
    case Qt::Key_Up:
#ifdef QT_KEYPAD_NAVIGATION
        if (QApplication::keypadNavigationEnabled()) {
            event->accept(); // don't change focus
            break;
        }
#endif
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Escape:
    case Qt::Key_Shift:
    case Qt::Key_Control:
        event->ignore();
        break;
    case Qt::Key_Space:
        selectionModel()->select(currentIndex(), selectionCommand(currentIndex(), event));
        break;
#ifdef Q_WS_MAC
    case Qt::Key_Return:
        if (!edit(currentIndex(), EditKeyPressed, event))
            event->ignore();
        break;
    case Qt::Key_O:
        if (event->modifiers() & Qt::ControlModifier)
            emit activated(currentIndex());
        break;
#else
    case Qt::Key_F2:
        if (!edit(currentIndex(), EditKeyPressed, event))
            event->ignore();
        break;
    case Qt::Key_Return:
    case Qt::Key_Select:
        emit activated(currentIndex());
        break;
#endif
    case Qt::Key_A:
        if (event->modifiers() & Qt::ControlModifier) {
            SelectionMode mode = selectionMode();
            if (mode == MultiSelection || mode == ExtendedSelection)
                selectAll();
            break;
        }
    default: {
        bool modified = (event->modifiers() == Qt::ControlModifier)
                        || (event->modifiers() == Qt::AltModifier)
                        || (event->modifiers() == Qt::MetaModifier);
        if (!event->text().isEmpty() && !modified) {
            if (!edit(currentIndex(), AnyKeyPressed, event))
                keyboardSearch(event->text());
        }
        event->ignore();
        break; }
    }
}

/*!
    This function is called with the given \a event when a resize event is sent to
    the widget.

    \sa QWidget::resizeEvent()
*/
void QAbstractItemView::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateGeometries();
}

/*!
  This function is called with the given \a event when a timer event is sent
  to the widget.

  \sa QObject::timerEvent()
*/
void QAbstractItemView::timerEvent(QTimerEvent *event)
{
    Q_D(QAbstractItemView);
    if (event->timerId() == d->autoScrollTimer.timerId())
        doAutoScroll();
    else if (event->timerId() == d->updateTimer.timerId())
        d->updateDirtyRegion();
}

#ifndef QT_NO_DRAGANDDROP
/*!
    \since 4.1

    Returns the position of the drop indicator in relation to the closest item.
*/
QAbstractItemView::DropIndicatorPosition QAbstractItemView::dropIndicatorPosition() const
{
    Q_D(const QAbstractItemView);
    return d->dropIndicatorPosition;
}
#endif

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
    necessary, and returns true if the view's \l{State} is now
    EditingState; otherwise returns false.

    The action that caused the editing process is described by
    \a trigger, and the associated event is specified by \a event.

    \sa closeEditor()
*/
bool QAbstractItemView::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event)
{
    Q_D(QAbstractItemView);

    if (!model() || !index.isValid())
        return false;

    QModelIndex buddy = model()->buddy(index);
    QStyleOptionViewItem options = viewOptions();
    options.rect = visualRect(buddy);
    options.state |= (buddy == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
    if (event && itemDelegate()->editorEvent(event, model(), options, buddy))
        return true; // the delegate handled the event

    if (!d->shouldEdit(trigger, buddy))
        return false;

    QWidget *editor = d->editor(buddy, options);
    if (!editor)
        return false;

    if (event && event->type() == QEvent::KeyPress && (trigger & d->editTriggers) == AnyKeyPressed)
        QApplication::sendEvent(editor->focusProxy() ? editor->focusProxy() : editor, event);
    setState(EditingState);
    editor->show();
    editor->setFocus();

    return true;
}

/*!
  \internal
*/
void QAbstractItemView::updateEditorData()
{
    QMap<QPersistentModelIndex, QPointer<QWidget> >::iterator it = d_func()->editors.begin();
    for (; it != d_func()->editors.end(); ++it)
        if (it.value() && it.key().isValid())
            itemDelegate()->setEditorData(it.value(), it.key());
}

/*!
    \internal
*/
void QAbstractItemView::updateEditorGeometries()
{
    Q_D(QAbstractItemView);
    QStyleOptionViewItem option = viewOptions();
    const QMap<QPersistentModelIndex, QPointer<QWidget> > editors = d->editors;
    QMap<QPersistentModelIndex, QPointer<QWidget> >::const_iterator it = editors.begin();
    while (it != editors.end()) {
        if (it.key().isValid() && it.value()) {
            option.rect = visualRect(it.key());
            if (option.rect.isValid()) {
                it.value()->show();
                itemDelegate()->updateEditorGeometry(it.value(), option, it.key());
            } else {
                it.value()->hide();
            }
        } else {
            if (it.value())
                d->releaseEditor(it.value());
            d->editors.remove(it.key());
        }
        ++it;
    }
}

/*!
    \internal
*/
void QAbstractItemView::updateGeometries()
{
    updateEditorGeometries();
    d_func()->fetchMore();
}

/*!
  \internal
*/
void QAbstractItemView::verticalScrollbarValueChanged(int value)
{
    if (verticalScrollBar()->maximum() == value && model())
        model()->fetchMore(rootIndex());
}

/*!
  \internal
*/
void QAbstractItemView::horizontalScrollbarValueChanged(int value)
{
    if (horizontalScrollBar()->maximum() == value && model())
        model()->fetchMore(rootIndex());
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
    Closes the given \a editor, and releases it. The \a hint is
    used to specify how the view should respond to the end of the editing
    operation. For example, the hint may indicate that the next item in
    the view should be opened for editing.

    \sa edit()
*/

void QAbstractItemView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    // close the editor
    Q_D(QAbstractItemView);
    if (editor && !d->persistent.contains(editor)) { // if the editor is not persistent, remove it
        setState(NoState);
        QModelIndex index = d->editors.key(editor);
        d->editors.remove(index);
        d->releaseEditor(editor);
    }

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
    if (!model() || !editor)
        return;
    QModelIndex index = d_func()->editors.key(editor);
    itemDelegate()->setModelData(editor, model(), index);
}

/*!
  Remove the editor \a editor from the map.
*/
void QAbstractItemView::editorDestroyed(QObject *editor)
{
    Q_D(QAbstractItemView);
    QWidget *w = ::qobject_cast<QWidget*>(editor);
    QPersistentModelIndex key = d_func()->editors.key(w);
    d->editors.remove(key);
    d->persistent.removeAll(w);
    if (state() == EditingState)
        setState(NoState);
}

/*!
    Sets the horizontal scrollbar's steps per item to \a steps.

    This is the number of steps used by the horizontal scrollbar to
    represent the width of an item.

    Note that if the view has a horizontal header, the item steps
    will be ignored and the header section size will be used instead.

    \sa horizontalStepsPerItem() setVerticalStepsPerItem()
*/
void QAbstractItemView::setHorizontalStepsPerItem(int steps)
{
    d_func()->horizontalStepsPerItem = steps;
    horizontalScrollBar()->setSingleStep(steps);
}

/*!
    Returns the horizontal scrollbar's steps per item.

    \sa setHorizontalStepsPerItem() verticalStepsPerItem()
*/
int QAbstractItemView::horizontalStepsPerItem() const
{
    return d_func()->horizontalStepsPerItem;
}

/*!
    Sets the vertical scrollbar's steps per item to \a steps.

    This is the number of steps used by the vertical scrollbar to
    represent the height of an item.

    Note that if the view has a vertical header, the item steps
    will be ignored and the header section size will be used instead.

    \sa verticalStepsPerItem() setHorizontalStepsPerItem()
*/
void QAbstractItemView::setVerticalStepsPerItem(int steps)
{
    d_func()->verticalStepsPerItem = steps;
    verticalScrollBar()->setSingleStep(steps);
}

/*!
    Returns the vertical scrollbar's steps per item.

    \sa setVerticalStepsPerItem() horizontalStepsPerItem()
*/
int QAbstractItemView::verticalStepsPerItem() const
{
    return d_func()->verticalStepsPerItem;
}

/*!
  Moves to and selects the item best matching the string \a search.
  If no item is found nothing happens.
*/
void QAbstractItemView::keyboardSearch(const QString &search)
{
    Q_D(QAbstractItemView);
    if (!model() || !model()->rowCount(rootIndex()) || !model()->columnCount(rootIndex()))
        return;

    QModelIndex start = currentIndex().isValid() ? currentIndex()
                        : model()->index(0, 0, rootIndex());
    QTime now(QTime::currentTime());
    bool skipRow = false;
    if (d->keyboardInputTime.msecsTo(now) > QApplication::keyboardInputInterval()) {
        d->keyboardInput = search;
        skipRow = true;
    } else {
        d->keyboardInput += search;
    }
    d->keyboardInputTime = now;

    // special case for searches with same key like 'aaaaa'
    bool sameKey = false;
    if (d->keyboardInput.length() > 1) {
        int c = d->keyboardInput.count(d->keyboardInput.at(d->keyboardInput.length() - 1));
        sameKey = (c == d->keyboardInput.length());
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
    match = model()->match(start, Qt::DisplayRole, searchString);
    if (!match.isEmpty() && match.at(0).isValid()) {
        selectionModel()->setCurrentIndex(match.at(0),
            (d->selectionMode == SingleSelection
             ? QItemSelectionModel::SelectionFlags(QItemSelectionModel::ClearAndSelect
                                                   | d->selectionBehaviorFlags())
             : QItemSelectionModel::SelectionFlags(QItemSelectionModel::NoUpdate)));
    }
}

/*!
    Returns the size hint for the item with the specified \a index or
    an invalid size for invalid indexes.
*/
QSize QAbstractItemView::sizeHintForIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return QSize();
    return itemDelegate()->sizeHint(viewOptions(), index);
}

/*!
    Returns the height size hint for the specified \a row or -1 if
    there is no model.
*/
int QAbstractItemView::sizeHintForRow(int row) const
{
    Q_D(const QAbstractItemView);
        
    Q_ASSERT(row >= 0);
    if(!model() || row >= model()->rowCount())
        return -1;

    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    Q_ASSERT(delegate);
    int height = 0;
    int colCount = model()->columnCount(rootIndex());
    QModelIndex index;
    for (int c = 0; c < colCount; ++c) {
        index = model()->index(row, c, rootIndex());
        if (QWidget *editor = d->editors.value(index))
            height = qMax(height, editor->sizeHint().height());
        height = qMax(height, delegate->sizeHint(option, index).height());
    }
    return height;
}

/*!
    Returns the width size hint for the specified \a column or -1 if there is no model.
*/
int QAbstractItemView::sizeHintForColumn(int column) const
{
    Q_D(const QAbstractItemView);

    Q_ASSERT(column >= 0);
    if(!model() || column >= model()->columnCount())
        return -1;

    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    Q_ASSERT(delegate);
    int width = 0;
    int rows = model()->rowCount(rootIndex());
    QModelIndex index;
    for (int r = 0; r < rows; ++r) {
        index = model()->index(r, column, rootIndex());
        if (QWidget *editor = d->editors.value(index))
            width = qMax(width, editor->sizeHint().width());
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
    Q_D(QAbstractItemView);
    QStyleOptionViewItem options = viewOptions();
    options.rect = visualRect(index);
    options.state |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);

    QWidget *editor = d->editor(index, options);
    if (editor) {
        editor->show();
        d->persistent.append(editor);
    }
}

/*!
  Closes the persistent editor for the item at the given \a index.
*/
void QAbstractItemView::closePersistentEditor(const QModelIndex &index)
{
    Q_D(QAbstractItemView);
    QWidget *editor = d->editors.value(index);
    if (editor) {
        d->persistent.removeAll(editor);
        d->releaseEditor(editor);
    }
    d->editors.remove(index);
}

/*!
    \since 4.1

    Sets the given \a widget on the item at the given \a index.
*/
void QAbstractItemView::setIndexWidget(const QModelIndex &index, QWidget *widget)
{
    Q_D(QAbstractItemView);
    Q_ASSERT(widget);
    Q_ASSERT(index.isValid());
    widget->setParent(viewport());
    widget->setGeometry(visualRect(index));
    d->persistent.append(widget);
    d->editors.insert(index, widget);
    widget->show();
}

/*!
    \since 4.1

    Returns the widget for the item at the given \a index.
*/
QWidget* QAbstractItemView::indexWidget(const QModelIndex &index) const
{
    return d_func()->editors.value(index);
}

/*!
    \since 4.1

    Scrolls the view to the top.
*/
void QAbstractItemView::scrollToTop()
{
    verticalScrollBar()->setValue(verticalScrollBar()->minimum());
}

/*!
    \since 4.1

    Scrolls the view to the bottom.
*/
void QAbstractItemView::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
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
    Q_D(QAbstractItemView);
    if (topLeft == bottomRight && topLeft.isValid()) {
        if (d->editors.contains(topLeft))
            itemDelegate()->setEditorData(d->editors.value(topLeft), topLeft);
        else if (isVisible() && !d->layoutPosted) // otherwise the items will be update later anyway
            d->viewport->update(visualRect(topLeft));
        return;
    }
    updateEditorData(); // we are counting on having relatively few editors
    if (!isVisible() || d->layoutPosted)
        return; // no need to update
    // single row or column changed
    bool sameRow = topLeft.row() == bottomRight.row() && topLeft.isValid();
    bool sameCol = topLeft.column() == bottomRight.column() && topLeft.isValid();
    if (sameRow || sameCol) {
        QRect tl = visualRect(topLeft);
        QRect br = visualRect(bottomRight);
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

    \sa rowsAboutToBeRemoved()
*/
void QAbstractItemView::rowsInserted(const QModelIndex &, int, int)
{
    if (!isVisible())
        d_func()->fetchMore();
}

/*!
    This slot is called when rows are about to be removed. The deleted rows are
    those under the given \a parent from \a start to \a end inclusive.

    \sa rowsInserted()
*/
void QAbstractItemView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(QAbstractItemView);

    setState(CollapsingState);

    // Ensure one selected item in single selection mode.
    QModelIndex current = currentIndex();
    if (selectionMode() == SingleSelection && current.isValid() &&
        current.row() >= start && current.row() <= end)
    {
        int totalToRemove = end - start + 1;
        if (model()->rowCount(parent) <= totalToRemove) { // no more children
            if (parent.isValid())
                setCurrentIndex(parent);
        } else {
            setCurrentIndex(model()->sibling(start > 0 ? start - 1 : end + 1,
                                             current.column(), current));
        }
    }

    // Remove all affected editors; this is more efficient than waiting for updateGeometries() to clean out editors for invalid indexes
    QMap<QPersistentModelIndex, QPointer<QWidget> >::iterator it = d->editors.begin();
    while (it != d->editors.end()) {
        QModelIndex index = it.key();
        if (index.row() <= start && index.row() >= end && model()->parent(index) == parent) {
            d->releaseEditor(it.value());
            it = d->editors.erase(it);
        } else {
            ++it;
        }
    }
}

/*!
    \internal

    This slot is called when rows have been removed. The deleted
    rows are those under the given \a parent from \a start to \a end
    inclusive.
*/
void QAbstractItemViewPrivate::rowsRemoved(const QModelIndex &, int, int)
{
    state = QAbstractItemView::NoState;
}

/*!
    \internal

    This slot is called when columns are about to be removed. The deleted
    columns are those under the given \a parent from \a start to \a end
    inclusive.
*/
void QAbstractItemViewPrivate::columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(QAbstractItemView);

    state = QAbstractItemView::CollapsingState;

    // Ensure one selected item in single selection mode.
    QModelIndex current = q->currentIndex();
    if (current.isValid() && selectionMode == QAbstractItemView::SingleSelection &&
        current.column() >= start && current.column() <= end)
    {
        if (model->columnCount(parent) < end - start + 1) { // no more columns
            if (parent.isValid())
                q->setCurrentIndex(parent);
        } else {
            q->setCurrentIndex(model->sibling(current.row(),
                                              start > 0 ? start - 1 : end + 1,
                                              current));
        }
    }

    // Remove all affected editors; this is more efficient than waiting for updateGeometries() to clean out editors for invalid indexes
    QMap<QPersistentModelIndex, QPointer<QWidget> >::iterator it = editors.begin();
    while (it != editors.end()) {
        QModelIndex index = it.key();
        if (index.column() <= start && index.column() >= end && model->parent(index) == parent) {
            releaseEditor(it.value());
            it = editors.erase(it);
        } else {
            ++it;
        }
    }
}

/*!
    \internal

    This slot is called when columns have been removed. The deleted
    rows are those under the given \a parent from \a start to \a end
    inclusive.
*/
void QAbstractItemViewPrivate::columnsRemoved(const QModelIndex &, int, int)
{
    state = QAbstractItemView::NoState;
}

/*!
    This slot is called when the selection is changed. The previous
    selection (which may be empty), is specified by \a deselected, and the
    new selection by \a selected.
*/
void QAbstractItemView::selectionChanged(const QItemSelection &selected,
                                         const QItemSelection &deselected)
{
    d_func()->setDirtyRegion(visualRegionForSelection(deselected));
    d_func()->setDirtyRegion(visualRegionForSelection(selected));
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
    Q_ASSERT(model());

    Q_D(QAbstractItemView);
    if (previous.isValid()) {
        QModelIndex buddy = model()->buddy(previous);
        QWidget *editor = d->editors.value(buddy);
        if (editor) {
            commitData(editor);
            closeEditor(editor, QAbstractItemDelegate::NoHint);
        }
        d->setDirtyRegion(visualRect(previous));
        d->updateDirtyRegion();
    }
    if (current.isValid() && !d->autoScrollTimer.isActive()) {
        scrollTo(current);
        edit(current, CurrentChanged, 0);
        if (current.row() == (model()->rowCount(rootIndex()) - 1))
            d->fetchMore();
    }
}

#ifndef QT_NO_DRAGANDDROP
/*!
    Starts a drag by calling drag->start() using the given \a supportedActions.
*/
void QAbstractItemView::startDrag(Qt::DropActions supportedActions)
{
    Q_D(QAbstractItemView);
    QModelIndexList indexes = selectedIndexes();
    if (indexes.count() > 0) {
        // setup pixmap
        QRect rect = visualRect(indexes.at(0));
        QList<QRect> rects;
        for (int i = 0; i < indexes.count(); ++i) {
            rects.append(visualRect(indexes.at(i)));
            rect |= rects.at(i);
        }
        rect = rect.intersect(d->viewport->rect());
        QPixmap pixmap(rect.size());
        pixmap.fill(palette().base().color());
        QPainter painter(&pixmap);
        QStyleOptionViewItem option = viewOptions();
        option.state |= QStyle::State_Selected;
        for (int j = 0; j < indexes.count(); ++j) {
            option.rect = QRect(rects.at(j).topLeft() - rect.topLeft(),
                                rects.at(j).size());
            itemDelegate()->paint(&painter, option, indexes.at(j));
        }
        painter.end();
        // create drag object
        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(model()->mimeData(indexes));
        if (drag->start(supportedActions) == Qt::MoveAction)
            d->removeSelectedRows();
    }
}
#endif // QT_NO_DRAGANDDROP

/*!
    Returns QStyleOptionViewItem structure populated with the view's
    palette, font, state, alignments etc.
*/
QStyleOptionViewItem QAbstractItemView::viewOptions() const
{
    Q_D(const QAbstractItemView);
    QStyleOptionViewItem option;
    option.init(this);
    option.font = font();
    option.state &= ~QStyle::State_HasFocus;
    if (d->iconSize.isValid()) {
        option.decorationSize = d->iconSize;
    } else {
        int pm = style()->pixelMetric(QStyle::PM_SmallIconSize);
        option.decorationSize = QSize(pm, pm);
    }
    option.decorationPosition = QStyleOptionViewItem::Left;
    option.decorationAlignment = Qt::AlignCenter;
    option.displayAlignment = QStyle::visualAlignment(layoutDirection(),
                                                      Qt::AlignLeft|Qt::AlignVCenter);
    option.textElideMode = d->textElideMode;
    option.rect = QRect();
    option.showDecorationSelected = style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected);
    return option;
}

/*!
    Returns the item view's state.

    \sa setState()
*/
QAbstractItemView::State QAbstractItemView::state() const
{
    return d_func()->state;
}

/*!
    Sets the item view's state to the given \a state

    \sa state()
*/
void QAbstractItemView::setState(State state)
{
    d_func()->state = state;
}

/*!
  Schedules a layout of the items in the view to be executed when the
  event processing starts.

  Even if scheduleDelayedItemsLayout() is called multiple times before
  events are processed, the view will only do the layout once.

  \sa executeDelayedItemsLayout()
*/
void QAbstractItemView::scheduleDelayedItemsLayout()
{
    d_func()->doDelayedItemsLayout();
}

/*!
  Executes the scheduled layouts without waiting for the event processing
  to begin.

  \sa scheduleDelayedItemsLayout()
*/
void QAbstractItemView::executeDelayedItemsLayout()
{
    d_func()->executePostedLayout();
}

/*!
    \since 4.1

    Marks the given \a region as dirty and schedules it to be updated.
    You only need to call this function if you are implementing
    your own view subclass.

    \sa scrollDirtyRegion(), dirtyRegionOffset()
*/

void QAbstractItemView::setDirtyRegion(const QRegion &region)
{
    d_func()->setDirtyRegion(region);
}

/*!
  Prepares the view for scrolling by (\a{dx},\a{dy}) pixels by moving the dirty regions in the
  opposite direction. You only need to call this function if you are implementing a scrolling
  viewport in your view subclass.

  If you implement scrollContentsBy() in a subclass of QAbstractItemView, call this function
  before you call QWidget::scroll() on the viewport. Alternatively, just call update().

  \sa scrollContentsBy(), dirtyRegionOffset(), setDirtyRegion()
*/
void QAbstractItemView::scrollDirtyRegion(int dx, int dy)
{
    d_func()->scrollDirtyRegion(dx, dy);
}

/*!
  Returns the offset of the dirty regions in the view.

  If you use scrollDirtyRegion() and implementa paintEvent() in a subclass of QAbstractItemView,
  you should translate the area given by the paint event with the offset returned from this function.

  \sa scrollDirtyRegion(), setDirtyRegion()
*/
QPoint QAbstractItemView::dirtyRegionOffset() const
{
    return d_func()->scrollDelayOffset;
}

/*!
  \internal
*/
void QAbstractItemView::startAutoScroll()
{
    d_func()->autoScrollTimer.start(d_func()->autoScrollInterval, this);
    d_func()->autoScrollCount = 0;
}

/*!
  \internal
*/
void QAbstractItemView::stopAutoScroll()
{
    d_func()->autoScrollTimer.stop();
    d_func()->autoScrollCount = 0;
}

/*!
  \internal
*/
void QAbstractItemView::doAutoScroll()
{
    // find how much we should scroll with
    Q_D(QAbstractItemView);
    int verticalStep = verticalScrollBar()->pageStep();
    int horizontalStep = horizontalScrollBar()->pageStep();
    if (d->autoScrollCount < qMax(verticalStep, horizontalStep))
        ++d->autoScrollCount;

    int margin = d->autoScrollMargin;
    int verticalValue = verticalScrollBar()->value();
    int horizontalValue = horizontalScrollBar()->value();

    QPoint pos = d->viewport->mapFromGlobal(QCursor::pos());
    QRect area = static_cast<QAbstractItemView*>(d->viewport)->d_func()->clipRect(); // access QWidget private by bending C++ rules

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
        update();
}

/*!
    Returns the SelectionFlags to be used when updating a selection with
    to include the \a index specified. The \a event is a user input event,
    such as a mouse or keyboard event.

    Reimplement this function to define your own selection behavior.
*/
QItemSelectionModel::SelectionFlags QAbstractItemView::selectionCommand(const QModelIndex &index,
                                                                        const QEvent *event) const
{
    Q_D(const QAbstractItemView);
    switch (selectionMode()) {
    case NoSelection: // Never update selection model
        return QItemSelectionModel::NoUpdate;
    case SingleSelection: // ClearAndSelect on valid index otherwise NoUpdate
        if (!index.isValid() || (event && event->type() == QEvent::MouseButtonRelease))
            return QItemSelectionModel::NoUpdate;
        return QItemSelectionModel::ClearAndSelect|d->selectionBehaviorFlags();
    case MultiSelection:
        return d->multiSelectionCommand(index, event);
    case ExtendedSelection:
        return d->extendedSelectionCommand(index, event);
    case ContiguousSelection:
        return d->contiguousSelectionCommand(index, event);
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
        case QEvent::KeyPress:
            if (static_cast<const QKeyEvent*>(event)->key() == Qt::Key_Space)
                return QItemSelectionModel::Toggle|selectionBehaviorFlags();
            break;
        case QEvent::MouseButtonPress:
            if (static_cast<const QMouseEvent*>(event)->button() == Qt::LeftButton)
                return QItemSelectionModel::Toggle|selectionBehaviorFlags();
            break;
        case QEvent::MouseMove:
            if (static_cast<const QMouseEvent*>(event)->buttons() & Qt::LeftButton)
                return QItemSelectionModel::ToggleCurrent|selectionBehaviorFlags();
        default:
            break;
        }
        return QItemSelectionModel::NoUpdate;
    }

    return QItemSelectionModel::Toggle|selectionBehaviorFlags();
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
        case QEvent::MouseButtonPress: {
            modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
            // NoUpdate when pressing without modifiers on a selected item
            if (!(modifiers & Qt::ShiftModifier)
                && !(modifiers & Qt::ControlModifier)
                && selectionModel->isSelected(index))
                return QItemSelectionModel::NoUpdate;
            // Clear on MouseButtonPress on non-valid item with no modifiers and not Qt::RightButton
            Qt::MouseButton button = static_cast<const QMouseEvent*>(event)->button();
            if (!index.isValid() && !(button & Qt::RightButton)
                && !(modifiers & Qt::ShiftModifier) && !(modifiers & Qt::ControlModifier))
                return QItemSelectionModel::Clear;
             // just pressing on an invalid index should not select anything, also pressing with anything but the left mouse button should not do anything
            if (!index.isValid() || button != Qt::LeftButton)
                return QItemSelectionModel::NoUpdate;
            break; }
        case QEvent::MouseButtonRelease: {
            // ClearAndSelect on MouseButtonRelease if MouseButtonPress on selected item
            modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
            Qt::MouseButton button = static_cast<const QMouseEvent*>(event)->button();
            if (index.isValid()
                && index == pressedIndex
                && !(pressedModifiers & Qt::ShiftModifier)
                && !(pressedModifiers & Qt::ControlModifier)
                && selectionModel->isSelected(index)
                && !(button & Qt::RightButton))
                return QItemSelectionModel::ClearAndSelect|selectionBehaviorFlags();
            return QItemSelectionModel::NoUpdate;
        }
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
    if (state == QAbstractItemView::DragSelectingState)
        return QItemSelectionModel::SelectCurrent|selectionBehaviorFlags();

    return QItemSelectionModel::ClearAndSelect|selectionBehaviorFlags();
}

QItemSelectionModel::SelectionFlags
QAbstractItemViewPrivate::contiguousSelectionCommand(const QModelIndex &index,
                                                     const QEvent *event) const
{
    QItemSelectionModel::SelectionFlags flags = extendedSelectionCommand(index, event);
    const int Mask = QItemSelectionModel::Clear | QItemSelectionModel::Select
                     | QItemSelectionModel::Deselect | QItemSelectionModel::Toggle
                     | QItemSelectionModel::Current;

    switch (flags & Mask) {
    case QItemSelectionModel::Clear:
    case QItemSelectionModel::ClearAndSelect:
    case QItemSelectionModel::SelectCurrent:
        return flags;
    case QItemSelectionModel::NoUpdate:
        if (event && event->type() == QEvent::MouseButtonRelease)
            return flags;
        return QItemSelectionModel::ClearAndSelect|selectionBehaviorFlags();
    default:
        return QItemSelectionModel::SelectCurrent|selectionBehaviorFlags();
    }
}

void QAbstractItemViewPrivate::fetchMore()
{
    if (!model || !model->canFetchMore(root))
        return;
    int last = model->rowCount(root) - 1;
    if (last < 0)
        return;
    QModelIndex index = model->index(last, 0, root);
    QRect rect = q_func()->visualRect(index);
    if (viewport->rect().contains(rect))
        model->fetchMore(root);
}

bool QAbstractItemViewPrivate::shouldEdit(QAbstractItemView::EditTrigger trigger,
                                          const QModelIndex &index)
{
    if (!index.isValid())
        return false;
    if ((model->flags(index) & Qt::ItemIsEditable) == 0)
        return false;
    if (state == QAbstractItemView::EditingState)
        return false;
    if (editors.contains(index))
        return false;
    if ((trigger & editTriggers) == QAbstractItemView::SelectedClicked
        && !selectionModel->isSelected(index))
        return false;
    return (trigger & editTriggers);
}

bool QAbstractItemViewPrivate::shouldAutoScroll(const QPoint &pos)
{
    if (!autoScroll)
        return false;
    QRect area = static_cast<QAbstractItemView*>(viewport)->d_func()->clipRect(); // access QWidget private by bending C++ rules
    return (pos.y() - area.top() < autoScrollMargin)
        || (area.bottom() - pos.y() < autoScrollMargin)
        || (pos.x() - area.left() < autoScrollMargin)
        || (area.right() - pos.x() < autoScrollMargin);
}

void QAbstractItemViewPrivate::doDelayedItemsLayout()
{
    if (!layoutPosted) {
        int slot = q_func()->metaObject()->indexOfSlot("doItemsLayout()");
        QApplication::postEvent(q_func(), new QMetaCallEvent(slot));
        layoutPosted = true;
    }
}

QWidget *QAbstractItemViewPrivate::editor(const QModelIndex &index,
                                          const QStyleOptionViewItem &options)
{
    Q_Q(QAbstractItemView);
    if (!q->itemDelegate())
        return 0;

    QWidget *w = editors.value(index);
    if (!w) {
        w = q->itemDelegate()->createEditor(viewport, options, index);
        if (w) {
            w->installEventFilter(q->itemDelegate());
            QObject::connect(w, SIGNAL(destroyed(QObject*)), q, SLOT(editorDestroyed(QObject*)));
            q->itemDelegate()->setEditorData(w, index);
            q->itemDelegate()->updateEditorGeometry(w, options, index);
            editors.insert(index, w);
            QWidget::setTabOrder(w, q);
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
        model->removeRows((*it).top(), count, parent);
    }
}

#include "moc_qabstractitemview.cpp"
#endif // QT_NO_ITEMVIEWS

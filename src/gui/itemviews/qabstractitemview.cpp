/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qabstractitemview.h"

#ifndef QT_NO_ITEMVIEWS
#include <qpointer.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qdrag.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qdatetime.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qitemdelegate.h>
#include <private/qabstractitemview_p.h>
#include <private/qabstractitemmodel_p.h>

QAbstractItemViewPrivate::QAbstractItemViewPrivate()
    :   model(QAbstractItemModelPrivate::staticEmptyModel()),
        itemDelegate(0),
        selectionModel(0),
        selectionMode(QAbstractItemView::ExtendedSelection),
        selectionBehavior(QAbstractItemView::SelectItems),
        pressedModifiers(Qt::NoModifier),
        pressedPosition(QPoint(-1, -1)),
        state(QAbstractItemView::NoState),
        editTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed),
        lastTrigger(QAbstractItemView::NoEditTriggers),
        tabKeyNavigation(false),
#ifndef QT_NO_DRAGANDDROP
        showDropIndicator(true),
        dragEnabled(false),
        dragDropMode(QAbstractItemView::NoDragDrop),
        overwrite(false),
        dropIndicatorPosition(QAbstractItemView::OnItem),
#endif
        autoScroll(true),
        autoScrollMargin(16),
        autoScrollInterval(50),
        autoScrollCount(0),
        alternatingColors(false),
        textElideMode(Qt::ElideRight),
        verticalScrollMode(QAbstractItemView::ScrollPerItem),
        horizontalScrollMode(QAbstractItemView::ScrollPerItem),
        currentIndexSet(false)
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

    doDelayedItemsLayout();

    q->setAttribute(Qt::WA_InputMethodEnabled);
}

/*!
    \class QAbstractItemView

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
    dataChanged(), rowsInserted(), rowsAboutToBeRemoved(), selectionChanged(),
    and currentChanged().

    The root item is returned by rootIndex(), and the current item by
    currentIndex(). To make sure that an item is visible use
    scrollTo().

    Some of QAbstractItemView's functions are concerned with
    scrolling, for example setHorizontalScrollMode() and
    setVerticalScrollMode(). To set the range of the scrollbars, you
    can, for example, reimplement the view's resizeEvent() function:

    \code
        void MyView::resizeEvent(QResizeEvent *event) {
            horizontalScrollBar()->setRange(0, realWidth - width());
            ...
        }
    \endcode

    Note that the range is not updated until the widget is shown.

    Several other functions are concerned with selection control; for
    example setSelectionMode(), and setSelectionBehavior(). This class
    provides a default selection model to work with
    (selectionModel()), but this can be replaced by using
    setSelectionModel() with an instance of QItemSelectionModel.

    For complete control over the display and editing of items you can
    specify a delegate with setItemDelegate().

    QAbstractItemView provides a lot of protected functions. Some are
    concerned with editing, for example, edit(), and commitData(),
    whilst others are keyboard and mouse event handlers.

    \sa {View Classes}, {Model/View Programming}, QAbstractItemModel, {Chart Example}
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
    \value PositionAtCenter  Scroll to position the item at the center of the viewport.
*/


/*!
  \enum QAbstractItemView::EditTrigger

  This enum describes actions which will initiate item editing.

  \value NoEditTriggers  No editing possible.
  \value CurrentChanged  Editing start whenever current item changes.
  \value DoubleClicked   Editing starts when an item is double clicked.
  \value SelectedClicked Editing starts when clicking on an already selected item.
  \value EditKeyPressed  Editing starts when the platform edit key has been pressed over an item.
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
    \value AnimatingState The item view is performing an animation.
*/

/*!
  \since 4.2
  \enum QAbstractItemView::ScrollMode

  \value ScrollPerItem    The view will scroll the contents one item at a time.
  \value ScrollPerPixel   The view will scroll the contents one pixel at a time.
*/

/*!
    \fn QRect QAbstractItemView::visualRect(const QModelIndex &index) const = 0
    Returns the rectangle on the viewport occupied by the item at \a index.

    If your item is displayed in several areas then visualRect should return
    the primary area that contains index and not the complete area that index
    might encompasses, touch or cause drawing.

    In the base class this is a pure virtual function.

    \sa indexAt(), visualRegionForSelection()
*/

/*!
    \fn void QAbstractItemView::scrollTo(const QModelIndex &index, ScrollHint hint) = 0

    Scrolls the view if necessary to ensure that the item at \a index
    is visible. The view will try to position the item according to the given \a hint.

    In the base class this is a pure virtual function.
*/

/*!
    \fn QModelIndex QAbstractItemView::indexAt(const QPoint &point) const = 0

    Returns the model index of the item at the viewport coordinates \a point.

    In the base class this is a pure virtual function.

    \sa visualRect()
*/

/*!
    \fn void QAbstractItemView::activated(const QModelIndex &index)

    This signal is emitted when the item specified by \a index is
    activated by the user. How to activate items depends on the
    platform; e.g., by single- or double-clicking the item, or by
    pressing the Return or Enter key when the item is current.

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

    \sa clicked(), activated()
*/

/*!
    \fn QModelIndex QAbstractItemView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) = 0

    Moves the cursor in the view according to the given \a cursorAction and
    keyboard modifiers specified by \a modifiers.

    In the base class this is a pure virtual function.
*/

/*!
    \fn int QAbstractItemView::horizontalOffset() const = 0

    Returns the horizontal offset of the view.

    In the base class this is a pure virtual function.

    \sa verticalOffset()
*/

/*!
    \fn int QAbstractItemView::verticalOffset() const = 0

    Returns the vertical offset of the view.

    In the base class this is a pure virtual function.

    \sa horizontalOffset()
*/

/*!
  \fn bool QAbstractItemView::isIndexHidden(const QModelIndex &index) const

  Returns true if the item referred to by the given \a index is hidden in the view,
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

    \sa selectionCommand(), selectedIndexes()
*/

/*!
    \fn QRegion QAbstractItemView::visualRegionForSelection(const QItemSelection &selection) const = 0

    Returns the region from the viewport of the items in the given
    \a selection.

    In the base class this is a pure virtual function.

    \sa visualRect(), selectedIndexes()
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

  \bold{Note:} This function will also create and set a new selection model,
  replacing any previously set with setSelectionModel(), but the old selection
  model will not be deleted.

  \sa selectionModel(), setSelectionModel()
*/
void QAbstractItemView::setModel(QAbstractItemModel *model)
{
    Q_D(QAbstractItemView);
    if (model == d->model)
        return;
    if (d->model) {
        disconnect(d->model, SIGNAL(destroyed()),
                   this, SLOT(_q_modelDestroyed()));
        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        disconnect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this, SLOT(rowsInserted(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   this, SLOT(_q_rowsRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                   this, SLOT(_q_columnsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                   this, SLOT(_q_columnsRemoved(QModelIndex,int,int)));

        disconnect(d->model, SIGNAL(modelReset()), this, SLOT(reset()));
        disconnect(d->model, SIGNAL(layoutChanged()), this, SLOT(doItemsLayout()));
    }
    d->model = (model ? model : QAbstractItemModelPrivate::staticEmptyModel());

    // These asserts do basic sanity checking of the model
    Q_ASSERT_X(d->model->index(0,0) == d->model->index(0,0),
               "QAbstractItemView::setModel",
               "A model should return the exact same index "
               "(including its internal id/pointer) when asked for it twice in a row.");
    Q_ASSERT_X(d->model->index(0,0).parent() == QModelIndex(),
               "QAbstractItemView::setModel",
               "The parent of a top level index should be invalid");

    if (d->model) {
        connect(d->model, SIGNAL(destroyed()),
                this, SLOT(_q_modelDestroyed()));
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(rowsInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(_q_rowsRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(_q_columnsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                this, SLOT(_q_columnsRemoved(QModelIndex,int,int)));

        connect(d->model, SIGNAL(modelReset()), this, SLOT(reset()));
        connect(d->model, SIGNAL(layoutChanged()), this, SLOT(doItemsLayout()));
    }
    QItemSelectionModel *selModel = selectionModel();
    setSelectionModel(new QItemSelectionModel(d->model, this));
    delete selModel;
    reset(); // kill editors, set new root and do layout
}

/*!
    Returns the model that this view is presenting.
*/
QAbstractItemModel *QAbstractItemView::model() const
{
    Q_D(const QAbstractItemView);
    return (d->model == QAbstractItemModelPrivate::staticEmptyModel() ? 0 : d->model);
}

/*!
    Sets the current selection model to the given \a selectionModel.

    Note that, if you call setModel() after this function, the given \a selectionModel
    will be replaced by a one created by the view.

    \sa selectionModel(), setModel(), clearSelection()
*/
void QAbstractItemView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    // ### if the given model is null, we should use the original selection model
    Q_ASSERT(selectionModel);
    Q_D(QAbstractItemView);

    if (selectionModel->model() != d->model) {
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
    Returns the current selection model.

    \sa setSelectionModel(), selectedIndexes()
*/
QItemSelectionModel* QAbstractItemView::selectionModel() const
{
    Q_D(const QAbstractItemView);
    return d->selectionModel;
}

/*!
    Sets the item delegate for this view and its model to \a delegate.
    This is useful if you want complete control over the editing and
    display of items.

    Any existing delegate will be removed, but not deleted. QAbstractItemView
    does not take ownership of \a delegate.

    \sa itemDelegate()
*/
void QAbstractItemView::setItemDelegate(QAbstractItemDelegate *delegate)
{
    Q_D(QAbstractItemView);

    if (d->itemDelegate) {
        disconnect(d->itemDelegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                   this, SLOT(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
        disconnect(d->itemDelegate, SIGNAL(commitData(QWidget*)), this, SLOT(commitData(QWidget*)));
    }

    d->itemDelegate = delegate;

    if (d->itemDelegate) {
        connect(d->itemDelegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                this, SLOT(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
        connect(d->itemDelegate, SIGNAL(commitData(QWidget*)), this, SLOT(commitData(QWidget*)));
    }
}

/*!
    Returns the item delegate used by this view and model. This is
    either one set with setItemDelegate(), or the default one.

    \sa setItemDelegate()
*/
QAbstractItemDelegate *QAbstractItemView::itemDelegate() const
{
    return d_func()->itemDelegate;
}

/*!
    \reimp
*/
QVariant QAbstractItemView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    const QModelIndex current = currentIndex();
    if (!current.isValid() || query != Qt::ImMicroFocus)
        return QAbstractScrollArea::inputMethodQuery(query);
    return visualRect(current);
}

/*!
    \since 4.2

    Sets the given item \a delegate used by this view and model for the given
    \a row. All items on \a row will be drawn and managed by \a delegate
    instead of using the default delegate (i.e., itemDelegate()).

    Any existing row delegate for \a row will be removed, but not
    deleted. QAbstractItemView does not take ownership of \a delegate.

    Note: If a delegate has been assigned to both a row and a column, the row
    delegate (i.e., this delegate) will take presedence and manage the
    intersecting cell index.

    \sa itemDelegateForRow(), setItemDelegateForColumn(), itemDelegate()
*/
void QAbstractItemView::setItemDelegateForRow(int row, QAbstractItemDelegate *delegate)
{
    Q_D(QAbstractItemView);
    if (QAbstractItemDelegate *rowDelegate = d->rowDelegates.value(row, 0)) {
        disconnect(rowDelegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                   this, SLOT(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
        disconnect(rowDelegate, SIGNAL(commitData(QWidget*)), this, SLOT(commitData(QWidget*)));
    }
    if (delegate) {
        d->rowDelegates.insert(row, delegate);
        connect(delegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                this, SLOT(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
        connect(delegate, SIGNAL(commitData(QWidget*)), this, SLOT(commitData(QWidget*)));
    } else {
        d->rowDelegates.remove(row);
    }
}

/*!
   \since 4.2

   Returns the item delegate used by this view and model for the given \a row,
   or 0 if no delegate has been assigned. You can call itemDelegate() to get a
   pointer to the current delegate for a given index.

   \sa setItemDelegateForRow(), itemDelegateForColumn(), setItemDelegate()
*/
QAbstractItemDelegate *QAbstractItemView::itemDelegateForRow(int row) const
{
    Q_D(const QAbstractItemView);
    return d->rowDelegates.value(row, 0);
}

/*!
    \since 4.2

    Sets the given item \a delegate used by this view and model for the given
    \a column. All items on \a column will be drawn and managed by \a delegate
    instead of using the default delegate (i.e., itemDelegate()).

    Any existing column delegate for \a column will be removed, but not
    deleted. QAbstractItemView does not take ownership of \a delegate.

    Note: If a delegate has been assigned to both a row and a column, the row
    delegate will take presedence and manage the intersecting cell index.

    \sa itemDelegateForColumn(), setItemDelegateForRow(), itemDelegate()
*/
void QAbstractItemView::setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate)
{
    Q_D(QAbstractItemView);
    if (QAbstractItemDelegate *columnDelegate = d->columnDelegates.value(column, 0)) {
        disconnect(columnDelegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                   this, SLOT(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
        disconnect(columnDelegate, SIGNAL(commitData(QWidget*)), this, SLOT(commitData(QWidget*)));
    }
    if (delegate) {
        d->columnDelegates.insert(column, delegate);
        connect(delegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                this, SLOT(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
        connect(delegate, SIGNAL(commitData(QWidget*)), this, SLOT(commitData(QWidget*)));
    } else {
        d->columnDelegates.remove(column);
    }
}

/*!
    \since 4.2

    Returns the item delegate used by this view and model for the given \a
    column.  You can call itemDelegate() to get a pointer to the current delegate
    for a given index.

    \sa setItemDelegateForColumn(), itemDelegateForRow(), itemDelegate()
*/
QAbstractItemDelegate *QAbstractItemView::itemDelegateForColumn(int column) const
{
    Q_D(const QAbstractItemView);
    return d->columnDelegates.value(column, 0);
}

/*!
   Returns the item delegate used by this view and model for
   the given \a index.
*/
QAbstractItemDelegate *QAbstractItemView::itemDelegate(const QModelIndex &index) const
{
    Q_D(const QAbstractItemView);
    return d->delegateForIndex(index);
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
    Q_D(QAbstractItemView);
    d->selectionMode = mode;
}

QAbstractItemView::SelectionMode QAbstractItemView::selectionMode() const
{
    Q_D(const QAbstractItemView);
    return d->selectionMode;
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
    Q_D(QAbstractItemView);
    d->selectionBehavior = behavior;
}

QAbstractItemView::SelectionBehavior QAbstractItemView::selectionBehavior() const
{
    Q_D(const QAbstractItemView);
    return d->selectionBehavior;
}

/*!
    Sets the current item to be the item at \a index.
    Depending on the current selection mode, the item may also be selected.
    Note that this function also updates the starting position for any
    new selections the user performs.

    To set an item as the current item without selecting it, call

    \c{selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);}

    \sa currentIndex(), currentChanged(), selectionMode
*/
void QAbstractItemView::setCurrentIndex(const QModelIndex &index)
{
    Q_D(QAbstractItemView);
    if (d->selectionModel) {
        QItemSelectionModel::SelectionFlags command = selectionCommand(index, 0);
        d->selectionModel->setCurrentIndex(index, command);
        d->currentIndexSet = true;
        if ((command & QItemSelectionModel::Current) == 0)
            d->pressedPosition = visualRect(currentIndex()).center() + d->offset();
    }
}

/*!
    Returns the model index of the current item.

    \sa setCurrentIndex()
*/
QModelIndex QAbstractItemView::currentIndex() const
{
    Q_D(const QAbstractItemView);
    return d->selectionModel ? d->selectionModel->currentIndex() : QModelIndex();
}


/*!
  Reset the internal state of the view.
*/
void QAbstractItemView::reset()
{
    Q_D(QAbstractItemView);
    _q_abstractitemview_editor_const_iterator it = d->editors.constBegin();
    for (; it != d->editors.constEnd(); ++it)
        d->releaseEditor(d->editorForIterator(it));
    d->editors.clear();
    d->persistent.clear();
    d->currentIndexSet = false;
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
    Q_ASSERT_X(index.isValid() ? index.model() == d->model : true,
               "QAbstractItemView::setRootIndex", "index must be from the currently set model");
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

  \sa setSelection(), selectedIndexes(), clearSelection()
*/
void QAbstractItemView::selectAll()
{
    Q_D(QAbstractItemView);
    d->selectAll(selectionCommand(d->model->index(0, 0, d->root)));
}

/*!
    Starts editing the item corresponding to the given \a index if it is
    editable.

    Note that this function does not change the current index. Since the current
    index defines the next and previous items to edit, users may find that
    keyboard navigation does not work as expected. To provide consistent navigation
    behavior, call setCurrentIndex() before this function with the same model
    index.

    \sa QModelIndex::flags()
*/
void QAbstractItemView::edit(const QModelIndex &index)
{
    Q_D(QAbstractItemView);
    if (!d->isIndexValid(index))
        qWarning("edit: index was invalid");
    if (!edit(index, AllEditTriggers, 0))
        qWarning("edit: editing failed");
}

/*!
    Clears all selected items. The current index will not be changed.

    \sa setSelection(), selectAll()
*/
void QAbstractItemView::clearSelection()
{
    Q_D(QAbstractItemView);
    if (d->selectionModel)
        d->selectionModel->clearSelection();
}

/*!
    \internal

    This function is intended to lay out the items in the view.
    The default implementatiidon just calls updateGeometries() and updates the viewport.
*/
void QAbstractItemView::doItemsLayout()
{
    Q_D(QAbstractItemView);
    d->delayedLayout.stop();
    updateGeometries();
    d->viewport->update();
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
    Q_D(QAbstractItemView);
    d->editTriggers = actions;
}

QAbstractItemView::EditTriggers QAbstractItemView::editTriggers() const
{
    Q_D(const QAbstractItemView);
    return d->editTriggers;
}

/*!
    \since 4.2
    \property QAbstractItemView::verticalScrollMode
    \brief how the view scrolls its contents in the vertical direction

    This property controlls how the view scroll its contents vertically.
    Scrolling can be done either per pixel or per item.
*/

void QAbstractItemView::setVerticalScrollMode(ScrollMode mode)
{
    Q_D(QAbstractItemView);
    d->verticalScrollMode = mode;
    updateGeometries(); // update the scrollbars
}

QAbstractItemView::ScrollMode QAbstractItemView::verticalScrollMode() const
{
    Q_D(const QAbstractItemView);
    return d->verticalScrollMode;
}

/*!
    \since 4.2
    \property QAbstractItemView::horizontalScrollMode
    \brief how the view scrolls its contents in the horizontal direction

    This property controlls how the view scroll its contents horizontally.
    Scrolling can be done either per pixel or per item.
*/

void QAbstractItemView::setHorizontalScrollMode(ScrollMode mode)
{
    Q_D(QAbstractItemView);
    d->horizontalScrollMode = mode;
    updateGeometries(); // update the scrollbars
}

QAbstractItemView::ScrollMode QAbstractItemView::horizontalScrollMode() const
{
    Q_D(const QAbstractItemView);
    return d->horizontalScrollMode;
}

#ifndef QT_NO_DRAGANDDROP
/*!
    \since 4.2
    \property QAbstractItemView::dragDropOverwriteMode
    \brief the view's drag and drop behavior

    If its value is \c true, the selected data will overwrite the
    existing item data when dropped, while moving the data will clear
    the item. If its value is \c false, the selected data will be
    inserted as a new item when the data is dropped. When the data is
    moved, the item is removed as well.

    The default value is \c false, as in the QListView and QTreeView
    subclasses. In the QTableView subclass, on the other hand, the
    property has been set to \c true.

    \sa dragDropMode
*/
void QAbstractItemView::setDragDropOverwriteMode(bool overwrite)
{
    Q_D(QAbstractItemView);
    d->overwrite = overwrite;
}

bool QAbstractItemView::dragDropOverwriteMode() const
{
    Q_D(const QAbstractItemView);
    return d->overwrite;
}
#endif

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
    Q_D(QAbstractItemView);
    d->autoScroll = enable;
}

bool QAbstractItemView::hasAutoScroll() const
{
    Q_D(const QAbstractItemView);
    return d->autoScroll;
}

/*!
  \property QAbstractItemView::tabKeyNavigation
  \brief whether item navigation with tab and backtab is enabled.
*/

void QAbstractItemView::setTabKeyNavigation(bool enable)
{
    Q_D(QAbstractItemView);
    d->tabKeyNavigation = enable;
}

bool QAbstractItemView::tabKeyNavigation() const
{
    Q_D(const QAbstractItemView);
    return d->tabKeyNavigation;
}

#ifndef QT_NO_DRAGANDDROP
/*!
  \property QAbstractItemView::showDropIndicator
  \brief whether the drop indicator is shown when dragging items and dropping.

  \sa dragEnabled DragDropMode dragDropOverwriteMode acceptDrops
*/

void QAbstractItemView::setDropIndicatorShown(bool enable)
{
    Q_D(QAbstractItemView);
    d->showDropIndicator = enable;
}

bool QAbstractItemView::showDropIndicator() const
{
    Q_D(const QAbstractItemView);
    return d->showDropIndicator;
}

/*!
  \property QAbstractItemView::dragEnabled
  \brief whether the view supports dragging of its own items

  \sa showDropIndicator DragDropMode dragDropOverwriteMode acceptDrops
*/

void QAbstractItemView::setDragEnabled(bool enable)
{
    Q_D(QAbstractItemView);
    d->dragEnabled = enable;
}

bool QAbstractItemView::dragEnabled() const
{
    Q_D(const QAbstractItemView);
    return d->dragEnabled;
}

/*!
    \since 4.2
    \enum QAbstractItemView::DragDropMode

    Describes the various drag and drop events the view can act upon.
    By default the view does not support dragging or dropping (\c
    NoDragDrop).

    \value NoDragDrop Does not support dragging or dropping.
    \value DragOnly The view supports dragging of its own items
    \value DropOnly The view accepts drops
    \value DragDrop The view supports both dragging and dropping
    \value InternalMove only accepts move operations only from itself.

    Note that the model used needs to provide support for drag and drop operations.

    \sa setDragDropMode() {Using Drag and Drop with Item Views}
*/

/*!
  \property QAbstractItemView::dragDropMode
  \brief the drag and drop event the view will act upon

  \since 4.2
  \sa showDropIndicator dragDropOverwriteMode
*/
void QAbstractItemView::setDragDropMode(DragDropMode behavior)
{
    Q_D(QAbstractItemView);
    d->dragDropMode = behavior;
    setDragEnabled(behavior == DragOnly || behavior == DragDrop || behavior == InternalMove);
    setAcceptDrops(behavior == DropOnly || behavior == DragDrop || behavior == InternalMove);
}

QAbstractItemView::DragDropMode QAbstractItemView::dragDropMode() const
{
    Q_D(const QAbstractItemView);
    DragDropMode setBehavior = d->dragDropMode;
    if (!dragEnabled() && !acceptDrops())
        return NoDragDrop;

    if (dragEnabled() && !acceptDrops())
        return DragOnly;

    if (!dragEnabled() && acceptDrops())
        return DropOnly;

    if (dragEnabled() && acceptDrops()) {
        if (setBehavior == InternalMove)
            return setBehavior;
        else
            return DragDrop;
    }

    return NoDragDrop;
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
    Q_D(QAbstractItemView);
    d->alternatingColors = enable;
    if (isVisible())
        d->viewport->update();
}

bool QAbstractItemView::alternatingRowColors() const
{
    Q_D(const QAbstractItemView);
    return d->alternatingColors;
}

/*!
    \property QAbstractItemView::iconSize
    \brief the size of items

    Setting this property when the view is visible will cause the
    items to be laid out again.
*/
void QAbstractItemView::setIconSize(const QSize &size)
{
    Q_D(QAbstractItemView);
    if (size == d->iconSize)
        return;
    d->iconSize = size;
    d->doDelayedItemsLayout();
}

QSize QAbstractItemView::iconSize() const
{
    Q_D(const QAbstractItemView);
    return d->iconSize;
}

/*!
    \property QAbstractItemView::textElideMode
    \brief the the position of the "..." in elided text.
*/
void QAbstractItemView::setTextElideMode(Qt::TextElideMode mode)
{
    Q_D(QAbstractItemView);
    d->textElideMode = mode;
}

Qt::TextElideMode QAbstractItemView::textElideMode() const
{
    return d_func()->textElideMode;
}

/*!
  \reimp
*/
bool QAbstractItemView::focusNextPrevChild(bool next)
{
    Q_D(QAbstractItemView);
    if (d->tabKeyNavigation) {
        QKeyEvent event(QEvent::KeyPress, next ? Qt::Key_Tab : Qt::Key_Backtab, Qt::NoModifier);
        keyPressEvent(&event);
        if (event.isAccepted())
            return true;
    }
    return QAbstractScrollArea::focusNextPrevChild(next);
}

/*!
  \reimp
*/
bool QAbstractItemView::event(QEvent *event)
{
    if (event->type() == QEvent::LocaleChange)
        viewport()->update();
    return QAbstractScrollArea::event(event);
}

/*!
    \fn bool QAbstractItemView::viewportEvent(QEvent *event)

    This function is used to handle tool tips, and What's
    This? mode, if the given \a event is a QEvent::ToolTip,or a
    QEvent::WhatsThis. It passes all other
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
    case QEvent::Leave:
        d->enteredIndex = QModelIndex();
        break;
    case QEvent::ToolTip:
    case QEvent::QueryWhatsThis:
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        const QModelIndex index = indexAt(he->pos());
        QStyleOptionViewItem option = viewOptions();
        option.rect = visualRect(index);
        option.state |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
        bool retval = false;
        // ### Qt 5: make this a normal function call to a virtual function
        QMetaObject::invokeMethod(d->delegateForIndex(index), "helpEvent",
                                  Q_RETURN_ARG(bool, retval),
                                  Q_ARG(QHelpEvent *, he),
                                  Q_ARG(QAbstractItemView *, this),
                                  Q_ARG(QStyleOptionViewItem, option),
                                  Q_ARG(QModelIndex, index));
        return retval;
    }
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
    QPersistentModelIndex index = indexAt(pos);

    if (!d->selectionModel
        || (d->state == EditingState && d->hasEditor(index)))
        return;

    d->pressedAlreadySelected = d->selectionModel->isSelected(index);
    d->pressedIndex = index;
    d->pressedModifiers = event->modifiers();
    QItemSelectionModel::SelectionFlags command = selectionCommand(index, event);
    QPoint offset = d->offset();
    if ((command & QItemSelectionModel::Current) == 0)
        d->pressedPosition = pos + offset;

    if (d->pressedPosition == QPoint(-1, -1))
        d->pressedPosition = visualRect(currentIndex()).center() + offset;

    if (edit(index, NoEditTriggers, event))
        return;

    if (index.isValid()) {
        // we disable scrollTo for mouse press so the item doesn't change position
        // when the user is interacting with it (ie. clicking on it)
        bool autoScroll = d->autoScroll;
        d->autoScroll = false;
        d->selectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        d->autoScroll = autoScroll;
    }

    QRect rect(d->pressedPosition - offset, pos);
    setSelection(rect, command);

    // signal handlers may change the model
    if (index.isValid()) {
        emit pressed(index);
        if (d->autoScroll) {
            //we delay the autoscrolling to filter out double click event
            //100 is to be sure that there won't be a double-click misinterpreted as a 2 single clicks
            d->delayedAutoScroll.start(QApplication::doubleClickInterval()+100, this);
        }

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
        topLeft = d->pressedPosition - d->offset();
        if ((topLeft - bottomRight).manhattanLength() > QApplication::startDragDistance()) {
            startDrag(d->model->supportedDragActions());
            setState(NoState); // the startDrag will return when the dnd operation is done
            stopAutoScroll();
        }
        return;
    }
#endif // QT_NO_DRAGANDDROP

    QModelIndex index = indexAt(bottomRight);
    QModelIndex buddy = d->model->buddy(d->pressedIndex);
    if (state() == EditingState && d->hasEditor(buddy)
        || edit(index, NoEditTriggers, event))
        return;

    if (d->selectionMode != SingleSelection)
        topLeft = d->pressedPosition - d->offset();
    else
        topLeft = bottomRight;

    if (d->enteredIndex != index) {
        // signal handlers may change the model
        QPersistentModelIndex persistent = index;
        if (persistent.isValid()) {
            emit entered(persistent);
#ifndef QT_NO_STATUSTIP
            QString statustip = d->model->data(persistent, Qt::StatusTipRole).toString();
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
    if (index.isValid() && d->dragEnabled
        && state() != DragSelectingState
        && event->buttons() != Qt::NoButton) {
        bool dragging = d->model->flags(index) & Qt::ItemIsDragEnabled;
        bool selected = d->selectionModel->isSelected(index);
        if (dragging && selected) {
            setState(DraggingState);
            return;
        }
    }
#endif

    if ((event->buttons() & Qt::LeftButton) && d->selectionAllowed(index) && d->selectionModel) {
        setState(DragSelectingState);
        QItemSelectionModel::SelectionFlags command = selectionCommand(index, event);

        // Do the normalize ourselves, since QRect::normalized() is flawed
        QRect selectionRect = QRect(topLeft, bottomRight);
        QPersistentModelIndex persistent = index;
        setSelection(selectionRect, command);

        // set at the end because it might scroll the view
        if (persistent.isValid())
            d->selectionModel->setCurrentIndex(persistent, QItemSelectionModel::NoUpdate);
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

    QPoint pos = event->pos();
    QPersistentModelIndex index = indexAt(pos);

    if (state() == EditingState) {
        if (d->isIndexValid(index) && d->sendDelegateEvent(index, event))
            d->viewport->update(visualRect(index));
        return;
    }

    setState(NoState);

    bool click = (index == d->pressedIndex && index.isValid());
    bool selectedClicked = click && (event->button() & Qt::LeftButton) && d->pressedAlreadySelected;
    EditTrigger trigger = (selectedClicked ? SelectedClicked : NoEditTriggers);
    bool edited = edit(index, trigger, event);

    if (d->selectionModel)
        d->selectionModel->select(index, selectionCommand(index, event));

    if (click) {
        emit clicked(index);
        if (edited)
            return;
        if (style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, 0, this))
            emit activated(index);
    }
}

/*!
    This function is called with the given \a event when a mouse button is
    double clicked inside the widget. If the double-click is on a valid item it
    emits the doubleClicked() signal and calls edit() on the item.
*/
void QAbstractItemView::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(QAbstractItemView);

    QModelIndex index = indexAt(event->pos());
    if (!index.isValid() || (d->pressedIndex != index)) {
        mousePressEvent(event);
        return;
    }
    // signal handlers may change the model
    QPersistentModelIndex persistent = index;
    emit doubleClicked(persistent);
    if ((event->button() & Qt::LeftButton) && !edit(persistent, DoubleClicked, event)
        && !style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, 0, this))
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
    if (dragDropMode() == InternalMove
        && (event->source() != this|| !(event->possibleActions() & Qt::MoveAction)))
        return;

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
    if (dragDropMode() == InternalMove
        && (event->source() != this || !(event->possibleActions() & Qt::MoveAction)))
        return;

    // ignore by default
    event->ignore();

    QModelIndex index = indexAt(event->pos());
    if (!(event->source() == this && selectedIndexes().contains(index))
        && d->canDecode(event)) {
        Qt::DropAction dropAction = (d->model->supportedDropActions() & event->proposedAction())
                                    ? event->proposedAction() : Qt::IgnoreAction;

        if (index.isValid() && d->showDropIndicator) {
            QRect rect = visualRect(index);
            d->dropIndicatorPosition = d->position(event->pos(), rect, index);
            switch (d->dropIndicatorPosition) {
            case AboveItem:
                if (d->model->flags(index.parent()) & Qt::ItemIsDropEnabled) {
                    d->dropIndicatorRect = QRect(rect.left(), rect.top(), rect.width(), 0);
                    event->setDropAction(dropAction);
                    event->accept();
                } else {
                    d->dropIndicatorRect = QRect();
                }
                break;
            case BelowItem:
                if (d->model->flags(index.parent()) & Qt::ItemIsDropEnabled) {
                    d->dropIndicatorRect = QRect(rect.left(), rect.bottom(), rect.width(), 0);
                    event->setDropAction(dropAction);
                    event->accept();
                } else {
                    d->dropIndicatorRect = QRect();
                }
                break;
            case OnItem:
                if (d->model->flags(index) & Qt::ItemIsDropEnabled) {
                    d->dropIndicatorRect = rect;
                    event->setDropAction(dropAction);
                    event->accept();
                } else {
                    d->dropIndicatorRect = QRect();
                }
                break;
            case OnViewport:
                d->dropIndicatorRect = QRect();
                break;
            }
        } else {
            d->dropIndicatorRect = QRect();
            d->dropIndicatorPosition = OnViewport;
            if (d->model->flags(rootIndex()) & Qt::ItemIsDropEnabled) {
                event->setDropAction(dropAction);
                event->accept(); // allow dropping in empty areas
            }
        }
        d->viewport->update();
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
    Q_D(QAbstractItemView);
    stopAutoScroll();
    setState(NoState);
    d->viewport->update();
}

/*!
    This function is called with the given \a event when a drop event occurs over
    the widget. If the model accepts the even position the drop event is accepted;
    otherwise it is ignored.

    \sa startDrag()
*/
void QAbstractItemView::dropEvent(QDropEvent *event)
{
    Q_D(QAbstractItemView);
    if (dragDropMode() == InternalMove) {
        if (event->source() != this || !(event->possibleActions() & Qt::MoveAction))
            return;
    }

    QModelIndex index;
    int col = -1;
    int row = -1;
    if (d->dropOn(event, &row, &col, &index)) {
        if (d->model->dropMimeData(event->mimeData(),
                    dragDropMode() == InternalMove ? Qt::MoveAction : event->dropAction(), row, col, index)) {
            if (dragDropMode() == InternalMove)
                event->setDropAction(Qt::MoveAction);
            event->accept();
        }
    }
    stopAutoScroll();
    setState(NoState);
    d->viewport->update();
}

/*!
    If the event hasn't already been accepted, determines the index to drop on.

    if (row == -1 && col == -1)
        // append to this drop index
    else
        // place at row, col in drop index

    \internal
  */
bool QAbstractItemViewPrivate::dropOn(QDropEvent *event, int *dropRow, int *dropCol, QModelIndex *dropIndex)
{
    Q_Q(QAbstractItemView);
    if (event->isAccepted())
        return false;

    QModelIndex index;
    // rootIndex() (i.e. the viewport) might be a valid index
    if (viewport->rect().contains(event->pos())) {
        index = q->indexAt(event->pos());
        if (!index.isValid())
            index = root;
    }

    // If we are allowed to do the drop
    if (model->supportedDropActions() & event->proposedAction()) {
        int row = -1;
        int col = -1;
        if (index != root) {
            dropIndicatorPosition = position(event->pos(), q->visualRect(index), index);
            switch (dropIndicatorPosition) {
            case QAbstractItemView::AboveItem:
                row = index.row();
                col = index.column();
                index = index.parent();
                break;
            case QAbstractItemView::BelowItem:
                row = index.row() + 1;
                col = index.column();
                index = index.parent();
                break;
            case QAbstractItemView::OnItem:
            case QAbstractItemView::OnViewport:
                break;
            }
        } else {
            dropIndicatorPosition = QAbstractItemView::OnViewport;
        }
        *dropIndex = index;
        *dropRow = row;
        *dropCol = col;
        return true;
    }
    return false;
}

QAbstractItemView::DropIndicatorPosition
QAbstractItemViewPrivate::position(const QPoint &pos, const QRect &rect, const QModelIndex &index) const
{
    QAbstractItemView::DropIndicatorPosition r = QAbstractItemView::OnViewport;
    if (!overwrite) {
        const int margin = 2;
        if (pos.y() - rect.top() < margin) {
            r = QAbstractItemView::AboveItem;
        } else if (rect.bottom() - pos.y() < margin) {
            r = QAbstractItemView::BelowItem;
        } else if (rect.contains(pos, true)) {
            r = QAbstractItemView::OnItem;
        }
    } else {
        QRect touchingRect = rect;
        touchingRect.adjust(-1, -1, 1, 1);
        if (touchingRect.contains(pos, false)) {
            r = QAbstractItemView::OnItem;
        }
    }

    if (r == QAbstractItemView::OnItem
        && (!(model->flags(index) & Qt::ItemIsDropEnabled) || !(model->flags(index.parent()) & Qt::ItemIsDropEnabled)))
        r = pos.y() < rect.center().y() ? QAbstractItemView::AboveItem : QAbstractItemView::BelowItem;

    return r;
}

#endif // QT_NO_DRAGANDDROP

/*!
    This function is called with the given \a event when the widget obtains the focus.
    By default, the event is ignored.

    \sa setFocus(), focusOutEvent()
*/
void QAbstractItemView::focusInEvent(QFocusEvent *event)
{
    Q_D(QAbstractItemView);
    QAbstractScrollArea::focusInEvent(event);
    if (selectionModel() && !d->currentIndexSet && !currentIndex().isValid()) {
        bool autoScroll = d->autoScroll;
        d->autoScroll = false;
        selectionModel()->setCurrentIndex(
            moveCursor(MoveNext, Qt::NoModifier), // first visible index
            QItemSelectionModel::NoUpdate);
        d->autoScroll = autoScroll;
    }
    d->viewport->update();
}

/*!
    This function is called with the given \a event when the widget
    looses the focus. By default, the event is ignored.

    \sa clearFocus(), focusInEvent()
*/
void QAbstractItemView::focusOutEvent(QFocusEvent *event)
{
    Q_D(QAbstractItemView);
    QAbstractScrollArea::focusOutEvent(event);
    d->viewport->update();
}

/*!
    This function is called with the given \a event when a key event is sent to
    the widget. The default implementation handles basic cursor movement, e.g. Up,
    Down, Left, Right, Home, PageUp, and PageDown; the activated() signal is
    emitted if the current index is valid and the activation key is pressed
    (e.g. Enter or Return, depending on the platform).
    This function is where editing is initiated by key press, e.g. if F2 is
    pressed.

    \sa edit(), moveCursor(), keyboardSearch(), tabKeyNavigation
*/
void QAbstractItemView::keyPressEvent(QKeyEvent *event)
{
    Q_D(QAbstractItemView);

#ifdef QT_KEYPAD_NAVIGATION
    switch (event->key()) {
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

#if !defined(QT_NO_CLIPBOARD) && !defined(QT_NO_SHORTCUT)
    if (event == QKeySequence::Copy) {
        QVariant variant = model()->data(currentIndex(), Qt::DisplayRole);
        if (variant.type() == QVariant::String)
            QApplication::clipboard()->setText(variant.toString());
        event->accept();
    }
#endif

    QPersistentModelIndex newCurrent;
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

    QPersistentModelIndex oldCurrent = currentIndex();
    if (newCurrent != oldCurrent && newCurrent.isValid()) {
        QItemSelectionModel::SelectionFlags command = selectionCommand(newCurrent, event);
        if (command != QItemSelectionModel::NoUpdate
             || style()->styleHint(QStyle::SH_ItemView_MovementWithoutUpdatingSelection, 0, this)) {
            if (command & QItemSelectionModel::Current) {
                d->selectionModel->setCurrentIndex(newCurrent, QItemSelectionModel::NoUpdate);
                if (d->pressedPosition == QPoint(-1, -1))
                    d->pressedPosition = visualRect(oldCurrent).center();
                QRect rect(d->pressedPosition - d->offset(), visualRect(newCurrent).center());
                setSelection(rect, command);
            } else {
                d->selectionModel->setCurrentIndex(newCurrent, command);
                d->pressedPosition = visualRect(newCurrent).center() + d->offset();
            }
            return;
        }
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
    case Qt::Key_Select:
        if (!edit(currentIndex(), AnyKeyPressed, event) && d->selectionModel)
            d->selectionModel->select(currentIndex(), selectionCommand(currentIndex(), event));
#ifdef QT_KEYPAD_NAVIGATION
        if ( event->key()==Qt::Key_Select ) {
            // Also do Key_Enter action.
            if (currentIndex().isValid()) {
                if (state() != EditingState)
                    emit activated(currentIndex());
            } else {
                event->ignore();
            }
        }
#endif
        break;
#ifdef Q_WS_MAC
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (!edit(currentIndex(), EditKeyPressed, event))
            event->ignore();
        break;
    case Qt::Key_O:
        if (event->modifiers() & Qt::ControlModifier && currentIndex().isValid())
            emit activated(currentIndex());
        break;
#else
    case Qt::Key_F2:
        if (!edit(currentIndex(), EditKeyPressed, event))
            event->ignore();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        // ### we can't open the editor on enter, becuse
        // some widgets will forward the enter event back
        // to the viewport, starting an endless loop
        if (state() != EditingState || hasFocus()) {
            if (currentIndex().isValid())
                emit activated(currentIndex());
        }
        break;
#endif
    case Qt::Key_A:
        if (event->modifiers() & Qt::ControlModifier) {
            SelectionMode mode = d->selectionMode;
            if (mode == MultiSelection || mode == ExtendedSelection)
                d->selectAll(QItemSelectionModel::ClearAndSelect
                             |d->selectionBehaviorFlags());
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
    else if (event->timerId() == d->delayedEditing.timerId()) {
        d->delayedEditing.stop();
        edit(currentIndex());
    } else if (event->timerId() == d->delayedLayout.timerId()) {
        d->delayedLayout.stop();
        doItemsLayout();
    } else if (event->timerId() == d->delayedAutoScroll.timerId()) {
        d->delayedAutoScroll.stop();
        //end of the timer: if the current item is still the same as the one when the mouse press occurred
        //we only get here if there was no double click
        if (d->pressedIndex.isValid() && d->pressedIndex == currentIndex())
            scrollTo(d->pressedIndex);
    }
}

/*!
    \reimp
*/
void QAbstractItemView::inputMethodEvent(QInputMethodEvent *event)
{
    if (!edit(currentIndex(), AnyKeyPressed, event))
        event->ignore();
}

#ifndef QT_NO_DRAGANDDROP
/*!
    \enum QAbstractItemView::DropIndicatorPosition

    This enum indicates the position of the drop indicator in
    relation to the index at the current mouse position:

    \value OnItem  The item will be dropped on the index.

    \value AboveItem  The item will be dropped above the index.

    \value BelowItem  The item will be dropped below the index.

    \value OnViewport  The item will be dropped onto a region of the viewport with
no items. The way each view handles items dropped onto the viewport depends on
the behavior of the underlying model in use.
*/


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

  \sa QItemSelectionModel::selectedIndexes()
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

    Editing can be forced by specifying the \a trigger to be
    QAbstractItemView::AllEditTriggers.

    \sa closeEditor()
*/
bool QAbstractItemView::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event)
{
    Q_D(QAbstractItemView);

    if (!d->isIndexValid(index))
        return false;

    if (QWidget *w = (d->persistent.isEmpty() ? 0 : d->editorForIndex(index))) {
        if (w->focusPolicy() == Qt::NoFocus)
            return false;
        w->setFocus();
        return true;
    }

    if (trigger == DoubleClicked) {
        d->delayedEditing.stop();
        d->delayedAutoScroll.stop();
    } else if (trigger == CurrentChanged) {
        d->delayedEditing.stop();
    }

    if (d->sendDelegateEvent(index, event)) {
        d->viewport->update(visualRect(index));
        return true;
    }

    // save the previous trigger before updating
    EditTriggers lastTrigger = d->lastTrigger;
    d->lastTrigger = trigger;

    if (!d->shouldEdit(trigger, d->model->buddy(index)))
        return false;

    if (d->delayedEditing.isActive())
        return false;

    // we will receive a mouseButtonReleaseEvent after a
    // mouseDoubleClickEvent, so we need to check the previous trigger
    if (lastTrigger == DoubleClicked && trigger == SelectedClicked)
        return false;

    // we may get a double click event later
    if (trigger == SelectedClicked)
        d->delayedEditing.start(QApplication::doubleClickInterval(), this);
    else
        d->openEditor(index, d->shouldForwardEvent(trigger, event) ? event : 0);

    return true;
}

/*!
  \internal
*/
void QAbstractItemView::updateEditorData()
{
    Q_D(QAbstractItemView);
    d->updateEditorData(QModelIndex(), QModelIndex());
}

/*!
    \internal
*/
void QAbstractItemView::updateEditorGeometries()
{
    Q_D(QAbstractItemView);
    QStyleOptionViewItem option = viewOptions();
    _q_abstractitemview_editor_iterator it = d->editors.begin();
    while (it != d->editors.end()) {
        QModelIndex index = d->indexForIterator(it);
        QWidget *editor = d->editorForIterator(it);
        if (index.isValid() && editor) {
            option.rect = visualRect(index);
            if (option.rect.isValid()) {
                editor->show();
                QAbstractItemDelegate *delegate = d->delegateForIndex(index);
                if (delegate)
                    delegate->updateEditorGeometry(editor, option, index);
            } else {
                editor->hide();
            }
            ++it;
        } else {
            d->releaseEditor(editor);
            it = d->editors.erase(it);
        }
    }
}

/*!
    \internal
*/
void QAbstractItemView::updateGeometries()
{
    Q_D(QAbstractItemView);
    updateEditorGeometries();
    d->fetchMore();
}

/*!
  \internal
*/
void QAbstractItemView::verticalScrollbarValueChanged(int value)
{
    Q_D(QAbstractItemView);
    if (verticalScrollBar()->maximum() == value)
        d->model->fetchMore(d->root);
}

/*!
  \internal
*/
void QAbstractItemView::horizontalScrollbarValueChanged(int value)
{
    Q_D(QAbstractItemView);
    if (horizontalScrollBar()->maximum() == value)
        d->model->fetchMore(d->root);
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

    \sa edit(), commitData()
*/

void QAbstractItemView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_D(QAbstractItemView);

    // Close the editor
    if (editor && !d->persistent.contains(editor)) {
        setState(NoState);
        d->removeEditor(editor);
        bool hadFocus = editor->hasFocus();
        QModelIndex index = d->indexForEditor(editor);
        editor->removeEventFilter(d->delegateForIndex(index));
        if (hadFocus)
            setFocus(); // this will send a focusLost event to the editor
        QApplication::sendPostedEvents(editor, 0);
        d->releaseEditor(editor);
    }

    // The EndEditHint part
    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect
                                                | d->selectionBehaviorFlags();
    switch (hint) {
    case QAbstractItemDelegate::EditNextItem: {
        QModelIndex index = moveCursor(MoveNext, Qt::NoModifier);
        if (index.isValid()) {
            QPersistentModelIndex persistent(index);
            d->selectionModel->setCurrentIndex(persistent, flags);
            // currentChanged signal would have already started editing
            if (!(editTriggers() & QAbstractItemView::CurrentChanged))
                edit(persistent);
        } break; }
    case QAbstractItemDelegate::EditPreviousItem: {
        QModelIndex index = moveCursor(MovePrevious, Qt::NoModifier);
        if (index.isValid()) {
            QPersistentModelIndex persistent(index);
            d->selectionModel->setCurrentIndex(persistent, flags);
            // currentChanged signal would have already started editing
            if (!(editTriggers() & QAbstractItemView::CurrentChanged))
                edit(persistent);
        } break; }
    case QAbstractItemDelegate::SubmitModelCache:
        d->model->submit();
        break;
    case QAbstractItemDelegate::RevertModelCache:
        d->model->revert();
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
    Q_D(QAbstractItemView);
    if (!editor || !d->itemDelegate)
        return;
    QModelIndex index = d->indexForEditor(editor);
    if (!index.isValid())
        return;
    QAbstractItemDelegate *delegate = d->delegateForIndex(index);
    editor->removeEventFilter(delegate);
    delegate->setModelData(editor, d->model, index);
    editor->installEventFilter(delegate);
}

/*!
  This function is called when the given \a editor has been destroyed.

  \sa closeEditor()
*/
void QAbstractItemView::editorDestroyed(QObject *editor)
{
    Q_D(QAbstractItemView);
    QWidget *w = ::qobject_cast<QWidget*>(editor);
    d->removeEditor(w);
    d->persistent.removeAll(w);
    if (state() == EditingState)
        setState(NoState);
}

/*!
    \obsolete
    Sets the horizontal scrollbar's steps per item to \a steps.

    This is the number of steps used by the horizontal scrollbar to
    represent the width of an item.

    Note that if the view has a horizontal header, the item steps
    will be ignored and the header section size will be used instead.

    \sa horizontalStepsPerItem() setVerticalStepsPerItem()
*/
void QAbstractItemView::setHorizontalStepsPerItem(int steps)
{
    Q_UNUSED(steps);
    // do nothing
}

/*!
    \obsolete
    Returns the horizontal scrollbar's steps per item.

    \sa setHorizontalStepsPerItem() verticalStepsPerItem()
*/
int QAbstractItemView::horizontalStepsPerItem() const
{
    return 1;
}

/*!
    \obsolete
    Sets the vertical scrollbar's steps per item to \a steps.

    This is the number of steps used by the vertical scrollbar to
    represent the height of an item.

    Note that if the view has a vertical header, the item steps
    will be ignored and the header section size will be used instead.

    \sa verticalStepsPerItem() setHorizontalStepsPerItem()
*/
void QAbstractItemView::setVerticalStepsPerItem(int steps)
{
    Q_UNUSED(steps);
    // do nothing
}

/*!
    \obsolete
    Returns the vertical scrollbar's steps per item.

    \sa setVerticalStepsPerItem() horizontalStepsPerItem()
*/
int QAbstractItemView::verticalStepsPerItem() const
{
    return 1;
}

/*!
  Moves to and selects the item best matching the string \a search.
  If no item is found nothing happens.

  In the default implementation, the search is reset if \a search is empty, or
  the time interval since the last search has exceeded
  QApplication::keyboardInputInterval().
*/
void QAbstractItemView::keyboardSearch(const QString &search)
{
    Q_D(QAbstractItemView);
    if (!d->model->rowCount(d->root) || !d->model->columnCount(d->root))
        return;

    QModelIndex start = currentIndex().isValid() ? currentIndex()
                        : d->model->index(0, 0, d->root);
    QTime now(QTime::currentTime());
    bool skipRow = false;
    if (search.isEmpty()
        || (d->keyboardInputTime.msecsTo(now) > QApplication::keyboardInputInterval())) {
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
        int newRow = (start.row() < d->model->rowCount(parent) - 1) ? start.row() + 1 : 0;
        start = d->model->index(newRow, start.column(), parent);
    }

    // search from start with wraparound
    const QString searchString = sameKey ? QString(d->keyboardInput.at(0)) : d->keyboardInput;
    QModelIndex current = start;
    QModelIndexList match;
    do {
        match = d->model->match(current, Qt::DisplayRole, searchString);
        if (match.value(0).isValid() && (match.value(0).flags() & Qt::ItemIsEnabled)) {
            setCurrentIndex(match.first());
            break;
        }
        current = current.sibling((match.value(0).row()+1)%d->model->rowCount(d->root), 0);
    } while (current != start && match.value(0).isValid());
}

/*!
    Returns the size hint for the item with the specified \a index or
    an invalid size for invalid indexes.

    \sa sizeHintForRow(), sizeHintForColumn()
*/
QSize QAbstractItemView::sizeHintForIndex(const QModelIndex &index) const
{
    Q_D(const QAbstractItemView);
    if (!d->isIndexValid(index) || !d->itemDelegate)
        return QSize();
    return d->delegateForIndex(index)->sizeHint(viewOptions(), index);
}

/*!
    Returns the height size hint for the specified \a row or -1 if
    there is no model.

    The returned height is calculated using the size hints of the
    given \a row's items, i.e. the returned value is the maximum
    height among the items. Note that to control the height of a row,
    you must reimplement the QAbstractItemDelegate::sizeHint()
    function.

    This function is used in views with a vertical header to find the
    size hint for a header section based on the contents of the given
    \a row.

    \sa sizeHintForColumn()
*/
int QAbstractItemView::sizeHintForRow(int row) const
{
    Q_D(const QAbstractItemView);

    if (row < 0 || row >= d->model->rowCount() || !model())
        return -1;

    QStyleOptionViewItem option = viewOptions();
    int height = 0;
    int colCount = d->model->columnCount(d->root);
    QModelIndex index;
    for (int c = 0; c < colCount; ++c) {
        index = d->model->index(row, c, d->root);
        if (QWidget *editor = d->editorForIndex(index))
            height = qMax(height, editor->size().height());
        int hint = d->delegateForIndex(index)->sizeHint(option, index).height();
        height = qMax(height, hint);
    }
    return height;
}

/*!
    Returns the width size hint for the specified \a column or -1 if there is no model.

    This function is used in views with a horizontal header to find the size hint for
    a header section based on the contents of the given \a column.

    \sa sizeHintForRow()
*/
int QAbstractItemView::sizeHintForColumn(int column) const
{
    Q_D(const QAbstractItemView);

    if (column < 0 || column >= d->model->columnCount() || !model())
        return -1;

    QStyleOptionViewItem option = viewOptions();
    int width = 0;
    int rows = d->model->rowCount(d->root);
    QModelIndex index;
    for (int r = 0; r < rows; ++r) {
        index = d->model->index(r, column, d->root);
        if (QWidget *editor = d->editorForIndex(index))
            width = qMax(width, editor->sizeHint().width());
        int hint = d->delegateForIndex(index)->sizeHint(option, index).width();
        width = qMax(width, hint);
    }
    return width;
}

/*!
    Opens a persistent editor on the item at the given \a index.
    If no editor exists, the delegate will create a new editor.

    \sa closePersistentEditor()
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

  \sa openPersistentEditor()
*/
void QAbstractItemView::closePersistentEditor(const QModelIndex &index)
{
    Q_D(QAbstractItemView);
    QWidget *editor = d->editorForIndex(index);
    if (editor) {
        d->persistent.removeAll(editor);
        d->releaseEditor(editor);
    }
    d->removeEditor(editor);
}

/*!
    \since 4.1

    Sets the given \a widget on the item at the given \a index,
    passing the ownership of the widget to the viewport.

    If \a index is invalid (e.g., if you pass the root index), this function
    will do nothing.

    Note that the given \a widget's \l
    {QWidget}{autoFillBackground} property must be set to true,
    otherwise the widget's background will be transparent, showing both
    the model data and the item at the given \a index.

    This function should only be used to display static content within
    the visible area corresponding to an item of data. If you want to
    display custom dynamic content or implement a custom editor
    widget, subclass QItemDelegate instead.

    \sa {Delegate Classes}
*/
void QAbstractItemView::setIndexWidget(const QModelIndex &index, QWidget *widget)
{
    Q_D(QAbstractItemView);
    if (!d->isIndexValid(index))
        return;
    if (QWidget *oldWidget = indexWidget(index)) {
        d->removeEditor(oldWidget);
        oldWidget->deleteLater();
    }
    if (widget) {
        widget->setParent(viewport());
        widget->setGeometry(visualRect(index));
        d->persistent.append(widget);
        d->addEditor(index, widget);
        widget->show();
    }
}

/*!
    \since 4.1

    Returns the widget for the item at the given \a index.
*/
QWidget* QAbstractItemView::indexWidget(const QModelIndex &index) const
{
    Q_D(const QAbstractItemView);
    if (!d->isIndexValid(index))
        return 0;
    return d->editorForIndex(index);
}

/*!
    \since 4.1

    Scrolls the view to the top.

    \sa scrollTo(), scrollToBottom()
*/
void QAbstractItemView::scrollToTop()
{
    verticalScrollBar()->setValue(verticalScrollBar()->minimum());
}

/*!
    \since 4.1

    Scrolls the view to the bottom.

    \sa scrollTo(), scrollToTop()
*/
void QAbstractItemView::scrollToBottom()
{
    Q_D(QAbstractItemView);
    if (d->delayedLayout.isActive()) {
        d->executePostedLayout();
        updateGeometries();
    }
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

/*!
    \since 4.3

    Updates the area occupied by the given \a index.

    \sa update()
*/
void QAbstractItemView::updateIndex(const QModelIndex &index)
{
    Q_D(QAbstractItemView);
    if (index.isValid())
        d->viewport->update(visualRect(index));
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
        if (d->hasEditor(topLeft)) {
            QAbstractItemDelegate *delegate = d->delegateForIndex(topLeft);
            if (delegate) {
                delegate->setEditorData(d->editorForIndex(topLeft), topLeft);
            }
        }
        if (isVisible() && !d->delayedLayout.isActive()) {
            // otherwise the items will be update later anyway
            d->viewport->update(visualRect(topLeft));
        }
        return;
    }
    d->updateEditorData(topLeft, bottomRight);
    if (!isVisible() || d->delayedLayout.isActive())
        return; // no need to update
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
    Q_D(QAbstractItemView);
    if (!isVisible())
        d->fetchMore();
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
    if (d->selectionMode == SingleSelection && current.isValid() &&
        current.row() >= start && current.row() <= end &&
        current.parent() == parent)
    {
        int totalToRemove = end - start + 1;
        if (d->model->rowCount(parent) <= totalToRemove) { // no more children
            if (parent.isValid())
                setCurrentIndex(parent);
        } else {
            setCurrentIndex(d->model->sibling(start > 0 ? start - 1 : end + 1,
                                             current.column(), current));
        }
    }

    // Remove all affected editors; this is more efficient than waiting for updateGeometries() to clean out editors for invalid indexes
    _q_abstractitemview_editor_iterator it = d->editors.begin();
    while (it != d->editors.end()) {
        QModelIndex index = d->indexForIterator(it);
        if (index.row() <= start && index.row() >= end && d->model->parent(index) == parent) {
            d->releaseEditor(d->editorForIterator(it));
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
void QAbstractItemViewPrivate::_q_rowsRemoved(const QModelIndex &, int, int)
{
    Q_Q(QAbstractItemView);
    q->setState(QAbstractItemView::NoState);
}

/*!
    \internal

    This slot is called when columns are about to be removed. The deleted
    columns are those under the given \a parent from \a start to \a end
    inclusive.
*/
void QAbstractItemViewPrivate::_q_columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(QAbstractItemView);

    q->setState(QAbstractItemView::CollapsingState);

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
    _q_abstractitemview_editor_iterator it = editors.begin();
    while (it != editors.end()) {
        QModelIndex index = indexForIterator(it);
        if (index.column() <= start && index.column() >= end && model->parent(index) == parent) {
            releaseEditor(editorForIterator(it));
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
void QAbstractItemViewPrivate::_q_columnsRemoved(const QModelIndex &, int, int)
{
    Q_Q(QAbstractItemView);
    q->setState(QAbstractItemView::NoState);
}

/*!
    \internal
*/
void QAbstractItemViewPrivate::_q_modelDestroyed()
{
    model = QAbstractItemModelPrivate::staticEmptyModel();
}

/*!
    This slot is called when the selection is changed. The previous
    selection (which may be empty), is specified by \a deselected, and the
    new selection by \a selected.

    \sa setSelection()
*/
void QAbstractItemView::selectionChanged(const QItemSelection &selected,
                                         const QItemSelection &deselected)
{
    Q_D(QAbstractItemView);
    if (isVisible() && updatesEnabled()) {
        d->setDirtyRegion(visualRegionForSelection(deselected));
        d->setDirtyRegion(visualRegionForSelection(selected));
    }
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
    Q_D(QAbstractItemView);
    Q_ASSERT(d->model);

    if (previous.isValid()) {
        QModelIndex buddy = d->model->buddy(previous);
        QWidget *editor = d->editorForIndex(buddy);
        if (editor && !d->persistent.contains(editor)) {
            commitData(editor);
            if (current.row() != previous.row())
                closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
            else
                closeEditor(editor, QAbstractItemDelegate::NoHint);
        }
        if (isVisible()) {
            d->setDirtyRegion(visualRect(previous));
            d->updateDirtyRegion();
        }
    }
    if (current.isValid() && !d->autoScrollTimer.isActive()) {
        if (d->autoScroll)
            scrollTo(current);
        d->setDirtyRegion(visualRect(current));
        d->updateDirtyRegion();
        edit(current, CurrentChanged, 0);
        if (current.row() == (d->model->rowCount(d->root) - 1))
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
        QMimeData *data = d->model->mimeData(indexes);
        if (!data)
            return;
        QRect rect;
        QPixmap pixmap = d->renderToPixmap(indexes, &rect);
        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->setHotSpot(d->viewport->mapFromGlobal(QCursor::pos()) - rect.topLeft());
        if (drag->start(supportedActions) == Qt::MoveAction)
            d->clearOrRemove();
    }
}
#endif // QT_NO_DRAGANDDROP

/*!
    Returns a QStyleOptionViewItem structure populated with the view's
    palette, font, state, alignments etc.
*/
QStyleOptionViewItem QAbstractItemView::viewOptions() const
{
    Q_D(const QAbstractItemView);
    QStyleOptionViewItem option;
    option.init(this);
    option.state &= ~QStyle::State_MouseOver;
    option.font = font();
    if (!hasFocus())
        option.state &= ~QStyle::State_Active;

    option.state &= ~QStyle::State_HasFocus;
    if (d->iconSize.isValid()) {
        option.decorationSize = d->iconSize;
    } else {
        int pm = style()->pixelMetric(QStyle::PM_SmallIconSize);
        option.decorationSize = QSize(pm, pm);
    }
    option.decorationPosition = QStyleOptionViewItem::Left;
    option.decorationAlignment = Qt::AlignCenter;
    option.displayAlignment = Qt::AlignLeft|Qt::AlignVCenter;
    option.textElideMode = d->textElideMode;
    option.rect = QRect();
    option.showDecorationSelected = style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, 0, this);
    return option;
}

/*!
    Returns the item view's state.

    \sa setState()
*/
QAbstractItemView::State QAbstractItemView::state() const
{
    Q_D(const QAbstractItemView);
    return d->state;
}

/*!
    Sets the item view's state to the given \a state.

    \sa state()
*/
void QAbstractItemView::setState(State state)
{
    Q_D(QAbstractItemView);
    d->state = state;
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
    Q_D(QAbstractItemView);
    d->doDelayedItemsLayout();
}

/*!
  Executes the scheduled layouts without waiting for the event processing
  to begin.

  \sa scheduleDelayedItemsLayout()
*/
void QAbstractItemView::executeDelayedItemsLayout()
{
    Q_D(QAbstractItemView);
    d->executePostedLayout();
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
    Q_D(QAbstractItemView);
    d->setDirtyRegion(region);
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
    Q_D(QAbstractItemView);
    d->scrollDirtyRegion(dx, dy);
}

/*!
  Returns the offset of the dirty regions in the view.

  If you use scrollDirtyRegion() and implement a paintEvent() in a subclass of
  QAbstractItemView, you should translate the area given by the paint event with
  the offset returned from this function.

  \sa scrollDirtyRegion(), setDirtyRegion()
*/
QPoint QAbstractItemView::dirtyRegionOffset() const
{
    Q_D(const QAbstractItemView);
    return d->scrollDelayOffset;
}

/*!
  \internal
*/
void QAbstractItemView::startAutoScroll()
{
    Q_D(QAbstractItemView);
    d->autoScrollTimer.start(d->autoScrollInterval, this);
    d->autoScrollCount = 0;
}

/*!
  \internal
*/
void QAbstractItemView::stopAutoScroll()
{
    Q_D(QAbstractItemView);
    d->autoScrollTimer.stop();
    d->autoScrollCount = 0;
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
    if (verticalUnchanged && horizontalUnchanged) {
        stopAutoScroll();
    } else {
#ifndef QT_NO_DRAGANDDROP
        d->dropIndicatorRect = QRect();
        d->dropIndicatorPosition = QAbstractItemView::OnViewport;
#endif
        d->viewport->update();
    }
}

/*!
    Returns the SelectionFlags to be used when updating a selection with
    to include the \a index specified. The \a event is a user input event,
    such as a mouse or keyboard event.

    Reimplement this function to define your own selection behavior.

    \sa setSelection()
*/
QItemSelectionModel::SelectionFlags QAbstractItemView::selectionCommand(const QModelIndex &index,
                                                                        const QEvent *event) const
{
    Q_D(const QAbstractItemView);
    switch (d->selectionMode) {
        case NoSelection: // Never update selection model
            return QItemSelectionModel::NoUpdate;
        case SingleSelection: // ClearAndSelect on valid index otherwise NoUpdate
            if (event && event->type() == QEvent::MouseButtonRelease)
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
    Q_UNUSED(index);

    if (event) {
        switch (event->type()) {
        case QEvent::KeyPress:
            if (static_cast<const QKeyEvent*>(event)->key() == Qt::Key_Space
             || static_cast<const QKeyEvent*>(event)->key() == Qt::Key_Select)
                return QItemSelectionModel::Toggle|selectionBehaviorFlags();
            break;
        case QEvent::MouseButtonPress:
            if (static_cast<const QMouseEvent*>(event)->button() == Qt::LeftButton &&
                !selectionModel->isSelected(index))
                return QItemSelectionModel::Toggle|selectionBehaviorFlags();
            break;
        case QEvent::MouseButtonRelease:
            if (static_cast<const QMouseEvent*>(event)->button() == Qt::LeftButton &&
                index == pressedIndex && pressedAlreadySelected)
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
        case QEvent::MouseMove: {
            // Toggle on MouseMove
            modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
            if (modifiers & Qt::ControlModifier)
                return QItemSelectionModel::ToggleCurrent|selectionBehaviorFlags();
            break;
        }
        case QEvent::MouseButtonPress: {
            modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
            const Qt::MouseButton button = static_cast<const QMouseEvent*>(event)->button();
            const bool rightButtonPressed = button & Qt::RightButton;
            const bool shiftKeyPressed = modifiers & Qt::ShiftModifier;
            const bool controlKeyPressed = modifiers & Qt::ControlModifier;
            const bool indexIsSelected = selectionModel->isSelected(index);
            if (!shiftKeyPressed && !controlKeyPressed && indexIsSelected)
                return QItemSelectionModel::NoUpdate;
            if (!index.isValid() && !rightButtonPressed && !shiftKeyPressed && !controlKeyPressed)
                return QItemSelectionModel::Clear;
            if (!index.isValid())
                return QItemSelectionModel::NoUpdate;
            break;
        }
        case QEvent::MouseButtonRelease: {
            // ClearAndSelect on MouseButtonRelease if MouseButtonPress on selected item
            modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
            const Qt::MouseButton button = static_cast<const QMouseEvent*>(event)->button();
            const bool rightButtonPressed = button & Qt::RightButton;
            const bool shiftKeyPressed = modifiers & Qt::ShiftModifier;
            const bool controlKeyPressed = modifiers & Qt::ControlModifier;
            if (index == pressedIndex && selectionModel->isSelected(index)
                && !shiftKeyPressed && !controlKeyPressed && !rightButtonPressed)
                return QItemSelectionModel::ClearAndSelect|selectionBehaviorFlags();
            return QItemSelectionModel::NoUpdate;
        }
        case QEvent::KeyPress: {
            // NoUpdate on Key movement and Ctrl
            modifiers = static_cast<const QKeyEvent*>(event)->modifiers();
            switch (static_cast<const QKeyEvent*>(event)->key()) {
            case Qt::Key_Backtab:
                modifiers = modifiers & ~Qt::ShiftModifier; // special case for backtab
            case Qt::Key_Down:
            case Qt::Key_Up:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            case Qt::Key_Tab:
#ifdef QT_KEYPAD_NAVIGATION
                return QItemSelectionModel::NoUpdate;
#else
                if (modifiers & Qt::ControlModifier)
                    return QItemSelectionModel::NoUpdate;
#endif
                break;
            case Qt::Key_Select:
                return QItemSelectionModel::Toggle|selectionBehaviorFlags();
            case Qt::Key_Space:// Toggle on Ctrl-Qt::Key_Space, Select on Space
                if (modifiers & Qt::ControlModifier)
                    return QItemSelectionModel::Toggle|selectionBehaviorFlags();
                return QItemSelectionModel::Select|selectionBehaviorFlags();
            default:
                break;
            }
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
        if (event &&
            (event->type() == QEvent::MouseButtonPress
             || event->type() == QEvent::MouseButtonRelease))
            return flags;
        return QItemSelectionModel::ClearAndSelect|selectionBehaviorFlags();
    default:
        return QItemSelectionModel::SelectCurrent|selectionBehaviorFlags();
    }
}

void QAbstractItemViewPrivate::fetchMore()
{
    if (!model->canFetchMore(root))
        return;
    int last = model->rowCount(root) - 1;
    if (last < 0) {
        model->fetchMore(root);
        return;
    }

    QModelIndex index = model->index(last, 0, root);
    QRect rect = q_func()->visualRect(index);
    if (viewport->rect().intersects(rect))
        model->fetchMore(root);
}

bool QAbstractItemViewPrivate::shouldEdit(QAbstractItemView::EditTrigger trigger,
                                          const QModelIndex &index) const
{
    if (!index.isValid())
        return false;
    Qt::ItemFlags flags = model->flags(index);
    if (((flags & Qt::ItemIsEditable) == 0) || ((flags & Qt::ItemIsEnabled) == 0))
        return false;
    if (state == QAbstractItemView::EditingState)
        return false;
    if (hasEditor(index))
        return false;
    if (trigger == QAbstractItemView::AllEditTriggers) // force editing
        return true;
    if ((trigger & editTriggers) == QAbstractItemView::SelectedClicked
        && !selectionModel->isSelected(index))
        return false;
    return (trigger & editTriggers);
}

bool QAbstractItemViewPrivate::shouldForwardEvent(QAbstractItemView::EditTrigger trigger,
                                                  const QEvent *event) const
{
    if (!event || (trigger & editTriggers) != QAbstractItemView::AnyKeyPressed)
        return false;

    switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::InputMethod:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
            return true;
        default:
            break;
    };

    return false;
}

bool QAbstractItemViewPrivate::shouldAutoScroll(const QPoint &pos) const
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
   if (!delayedLayout.isActive())
        delayedLayout.start(0, q_func());
}

QWidget *QAbstractItemViewPrivate::editor(const QModelIndex &index,
                                          const QStyleOptionViewItem &options)
{
    Q_Q(QAbstractItemView);
    QWidget *w = editorForIndex(index);
    if (!w) {
        QAbstractItemDelegate *delegate = delegateForIndex(index);
        if (!delegate)
            return 0;
        w = delegate->createEditor(viewport, options, index);
        if (w) {
            w->installEventFilter(delegate);
            QObject::connect(w, SIGNAL(destroyed(QObject*)), q, SLOT(editorDestroyed(QObject*)));
            delegate->updateEditorGeometry(w, options, index);
            delegate->setEditorData(w, index);
            addEditor(index, w);
            QWidget::setTabOrder(q, w);

            if (q->currentIndex() == index) {
            // Special cases for some editors containing QLineEdit
#ifndef QT_NO_LINEEDIT
            if (QLineEdit *le = ::qobject_cast<QLineEdit*>(w))
                le->selectAll();
#endif
#ifndef QT_NO_SPINBOX
            if (QSpinBox *sb = ::qobject_cast<QSpinBox*>(w))
                sb->selectAll();
#endif
            }
        }
    }
    return w;
}

void QAbstractItemViewPrivate::updateEditorData(const QModelIndex &tl, const QModelIndex &br)
{
    // we are counting on having relatively few editors
    const bool checkIndexes = tl.isValid() && br.isValid();
    const QModelIndex parent = tl.parent();
    _q_abstractitemview_editor_iterator it = editors.begin();
    for (; it != editors.end(); ++it) {
        QWidget *editor = editorForIterator(it);
        const QModelIndex index = indexForIterator(it);
        QAbstractItemDelegate *delegate = delegateForIndex(index);
        if (delegate && editor && index.isValid()
            && (!checkIndexes
                || (index.row() >= tl.row() && index.row() <= br.row()
                    && index.column() >= tl.column() && index.column() <= br.column()
                    && index.parent() == parent))) {
            delegate->setEditorData(editor, index);
        }
    }
}

/*!
    \internal

    In DND if something has been moved then this is called.
    Typically this means you should "remove" the selected item or row,
    but the behavior is view dependant (table just clears the selected indexes for example).

    Either remove the selected rows or clear them
  */
void QAbstractItemViewPrivate::clearOrRemove()
{
#ifndef QT_NO_DRAGANDDROP
    const QItemSelection selection = selectionModel->selection();
    QList<QItemSelectionRange>::const_iterator it = selection.begin();

    if (!overwrite) {
        for (; it != selection.end(); ++it) {
            QModelIndex parent = (*it).parent();
            if ((*it).left() != 0)
                continue;
            if ((*it).right() != (model->columnCount(parent) - 1))
                continue;
            int count = (*it).bottom() - (*it).top() + 1;
            model->removeRows((*it).top(), count, parent);
        }
    } else {
        // we can't remove the rows so reset the items (i.e. the view is like a table)
        QModelIndexList list = selection.indexes();
        for (int i=0; i < list.size(); ++i) {
            QModelIndex index = list.at(i);
            QMap<int, QVariant> roles = model->itemData(index);
            for (QMap<int, QVariant>::Iterator it = roles.begin(); it != roles.end(); ++it)
                it.value() = QVariant();
            model->setItemData(index, roles);
        }
    }
#endif
}

QWidget *QAbstractItemViewPrivate::editorForIndex(const QModelIndex &index) const
{
    _q_abstractitemview_editor_const_iterator it = editors.begin();
    for (; it != editors.end(); ++it) {
        if (indexForIterator(it) == index)
            return editorForIterator(it);
    }
    return 0;
}

QModelIndex QAbstractItemViewPrivate::indexForEditor(QWidget *editor) const
{
    _q_abstractitemview_editor_const_iterator it = editors.begin();
    for (; it != editors.end(); ++it) {
        if (editorForIterator(it) == editor)
            return indexForIterator(it);
    }
    return QModelIndex();
}

void QAbstractItemViewPrivate::removeEditor(QWidget *editor)
{
    _q_abstractitemview_editor_iterator it = editors.begin();
    for (; it != editors.end(); ) {
        if (editorForIterator(it) == editor)
            it = editors.erase(it);
        else
            ++it;
    }
}

void QAbstractItemViewPrivate::addEditor(const QModelIndex &index, QWidget *editor)
{
    editors.append(QPair<QPersistentModelIndex, QPointer<QWidget> >(index, editor));
}

bool QAbstractItemViewPrivate::sendDelegateEvent(const QModelIndex &index, QEvent *event) const
{
    Q_Q(const QAbstractItemView);
    QModelIndex buddy = model->buddy(index);
    QStyleOptionViewItem options = q->viewOptions();
    options.rect = q->visualRect(buddy);
    options.state |= (buddy == q->currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
    QAbstractItemDelegate *delegate = delegateForIndex(index);
    return (event && delegate && delegate->editorEvent(event, model, options, buddy));
}

bool QAbstractItemViewPrivate::openEditor(const QModelIndex &index, QEvent *event)
{
    Q_Q(QAbstractItemView);

    QModelIndex buddy = model->buddy(index);
    QStyleOptionViewItem options = q->viewOptions();
    options.rect = q->visualRect(buddy);
    options.state |= (buddy == q->currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);

    QWidget *w = editor(buddy, options);
    if (!w)
        return false;

    if (event)
        QApplication::sendEvent(w->focusProxy() ? w->focusProxy() : w, event);

    q->setState(QAbstractItemView::EditingState);
    w->show();
    w->setFocus();

    return true;
}

QPixmap QAbstractItemViewPrivate::renderToPixmap(const QModelIndexList &indexes, QRect *r) const
{
    Q_Q(const QAbstractItemView);
    QRect rect = q->visualRect(indexes.at(0));
    QList<QRect> rects;
    for (int i = 0; i < indexes.count(); ++i) {
        rects.append(q->visualRect(indexes.at(i)));
        rect |= rects.at(i);
    }
    rect = rect.intersected(viewport->rect());
    QPixmap pixmap(rect.size());
    pixmap.fill(q->palette().base().color());
    QPainter painter(&pixmap);
    QStyleOptionViewItem option = q->viewOptions();
    option.state |= QStyle::State_Selected;
    for (int j = 0; j < indexes.count(); ++j) {
        option.rect = QRect(rects.at(j).topLeft() - rect.topLeft(), rects.at(j).size());
        delegateForIndex(indexes.at(j))->paint(&painter, option, indexes.at(j));
    }
    painter.end();
    if (r) *r = rect;
    return pixmap;
}

void QAbstractItemViewPrivate::selectAll(QItemSelectionModel::SelectionFlags command)
{
    if (!selectionModel)
        return;

    QItemSelection selection;
    QModelIndex tl = model->index(0, 0, root);
    QModelIndex br = model->index(model->rowCount(root) - 1,
                                  model->columnCount(root) - 1,
                                  root);
    selection.append(QItemSelectionRange(tl, br));
    selectionModel->select(selection, command);
}

#include "moc_qabstractitemview.cpp"
#endif // QT_NO_ITEMVIEWS

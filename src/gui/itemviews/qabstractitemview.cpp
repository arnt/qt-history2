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
#include <qrubberband.h>
#include <qdebug.h>

#include <private/qabstractitemview_p.h>

QAbstractItemViewPrivate::QAbstractItemViewPrivate()
    :   model(0),
        delegate(0),
        selectionModel(0),
        selectionMode(QAbstractItemView::ExtendedSelection),
        selectionBehavior(QAbstractItemView::SelectItems),
        state(QAbstractItemView::NoState),
        editTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed),
        tabKeyNavigation(false),
        showDropIndicator(false),
        dragEnabled(false),
        autoScroll(true),
        autoScrollMargin(16),
        autoScrollInterval(50),
        autoScrollCount(0),
        layoutPosted(false),
        alternatingColors(false),
        textElideMode(Qt::ElideRight),
        dropIndicator(0)
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

    dropIndicator = new QRubberBand(QRubberBand::Line, viewport);

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

    For complete control over the display and editing of items you can
    specify a delegate with setItemDelegate().

    QAbstractItemView provides a lot of protected functions. Some are
    concerned with editing, for example, edit(), and commitData(),
    whilst others are keyboard and mouse event handlers.

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemModel

*/

/*!
    \enum QAbstractItemView::SelectionMode

    This enum indicates how the view responds to user selections:

    \value SingleSelection  When the user selects an item, any
    already-selected item becomes unselected, and the user cannot
    unselect the selected item.

    \value MultiSelection  When the user selects an item in the usual
    way, the selection status of that item is toggled and the other
    items are left alone.

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

    \value NoSelection  Items cannot be selected.

    In other words, \c SingleSelection is a real single-selection list
    view, \c MultiSelection a real multi-selection list view,
    \c ExtendedSelection is a list view in which users can select
    multiple items, but usually want to select either just one or a
    range of contiguous items, and \c NoSelection
    is a list view where the user can navigate without selecting
    items.
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

    In the base class this is a pure virtual function.
*/

/*!
    \fn void QAbstractItemView::scrollTo(const QModelIndex &index, ScrollHint hint) = 0

    Scrolls the view if necessary to ensure that the item at \a index
    is visible. The view will try to position the item according to the given \a hint.

    In the base class this is a pure virtual function.
*/

/*!
    \fn QModelIndex QAbstractItemView::indexAt(const QPoint &p) const

    \overload

    Returns the model index of the item at point \a p.

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

    \sa viewportEntered(), activated(), clicked(), doubleClicked(), pressed()
*/

/*!
    \fn void QAbstractItemView::viewportEntered()

    This signal is emitted when the mouse cursor enters the viewport.

    \sa entered()
*/

/*!
    \fn void QAbstractItemView::pressed(const QModelIndex &index)

    This signal is emitted when a mouse button is pressed. The item the
    mouse was pressed on is specified by \a index (which may be invalid if
    the mouse was not pressed on an item).

    \sa activated(), clicked(), doubleClicked(), entered()
*/

/*!
    \fn void QAbstractItemView::clicked(const QModelIndex &index)

    This signal is emitted when a mouse button is clicked. The item the
    mouse was clicked on is specified by \a index (which may be invalid if
    the mouse was not clicked on an item).

    \sa activated(), doubleClicked(), entered(), pressed()
*/

/*!
    \fn void QAbstractItemView::doubleClicked(const QModelIndex &index)

    This signal is emitted when a mouse button is double-clicked. The
    item the mouse was double-clicked on is specified by \a index (which
    may be invalid if the mouse was not double-clicked on an item).
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
        disconnect(d->model, SIGNAL(modelReset()), this, SLOT(reset()));
        disconnect(d->model, SIGNAL(layoutChanged()), this, SLOT(doItemsLayout()));
    }

    d->model = model;

    if (d->model) {
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(rowsInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(modelReset()), this, SLOT(reset()));
        connect(d->model, SIGNAL(layoutChanged()), this, SLOT(doItemsLayout()));
    }

    setSelectionModel(new QItemSelectionModel(d->model));
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
        disconnect(d->delegate, SIGNAL(closeEditor(QWidget*,EndEditHint)),
                   this, SLOT(closeEditor(QWidget*,EndEditHint)));
        disconnect(d->delegate, SIGNAL(commitData(QWidget*)), this, SLOT(commitData(QWidget*)));
    }

    d->delegate = delegate;

    if (d->delegate) {
        connect(d->delegate, SIGNAL(closeEditor(QWidget*,EndEditHint)),
                this, SLOT(closeEditor(QWidget*,EndEditHint)));
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
  \brief which selection mode the view operates in.


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
  \brief which selection behavior the view uses.

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
    Sets the current item to be the itm at \a index.

    \sa currentIndex()
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
    QMap<QPersistentModelIndex, QWidget*>::iterator it = d->editors.begin();
    for (; it != d->editors.end(); ++it) {
        QObject::disconnect(it.value(), SIGNAL(destroyed(QObject*)),
                            this, SLOT(editorDestroyed(QObject*)));
        d->releaseEditor(it.value());
    }
    d->editors.clear();
    d->persistent.clear();
    setState(NoState);
    setRootIndex(QModelIndex());
}

/*!
    Sets the root item to the item at \a index.

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
    The default implementation just calls updateGeometries() and updates the viewport.
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

#ifndef QT_NO_DRAGANDDROP
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
  alternating colors, otherwise the background will be drawn using the QPalette::Base color.
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
    \fn bool QAbstractItemView::viewportEvent(QEvent *event)

    This function is used to handle tool tips, status tips, and What's
    This? mode, if the given \a event is a QEvent::ToolTip, a
    QEvent::WhatsThis, or a QEvent::StatusTip. It passes all other
    events on to its base class viewportEvent() handler.
*/
bool QAbstractItemView::viewportEvent(QEvent *e)
{
    Q_D(QAbstractItemView);
    switch (e->type()) {
#ifndef QT_NO_TOOLTIP
    case QEvent::ToolTip: {
        if (!isActiveWindow())
            break;
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        if (!he)
            break;
        QModelIndex index = indexAt(he->pos());
        if (index.isValid()) {
            QString tooltip = model()->data(index, Qt::ToolTipRole).toString();
            QToolTip::showText(he->globalPos(), tooltip, this);
        }
        return true; }
#endif
#ifndef QT_NO_WHATSTHIS
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        QModelIndex index = indexAt(he->pos());
        if (index.isValid()) {
            QString whatsthis = model()->data(index, Qt::WhatsThisRole).toString();
            QWhatsThis::showText(he->globalPos(), whatsthis, this);
        }
        return true; }
#endif
    case QEvent::StatusTip: {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        QModelIndex index = indexAt(he->pos());
        if (index.isValid()) {
            QString statustip = model()->data(index, Qt::StatusTipRole).toString();
            if (!statustip.isEmpty())
                setStatusTip(statustip);
        }
        return true; }
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        d->viewport->update();
        break;
    case QEvent::KeyPress: {
        if (!d->tabKeyNavigation)
            break;
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
    return QAbstractScrollArea::viewportEvent(e);
}

/*!
    This function is called when a mouse event \a e occurs. If a valid
    item is pressed on it is made into the current item. This function
    emits the pressed() signal.
*/
void QAbstractItemView::mousePressEvent(QMouseEvent *e)
{
    Q_D(QAbstractItemView);
    QPoint pos = e->pos();
    QModelIndex index = indexAt(pos);

    if (!selectionModel() || (d->state == EditingState && d->editors.contains(index)))
        return;

    QPoint offset(horizontalOffset(), verticalOffset());
    d->pressedIndex = index;
    d->pressedModifiers = e->modifiers();
    QItemSelectionModel::SelectionFlags command = selectionCommand(index, e);
    if ((command & QItemSelectionModel::Current) == 0)
        d->pressedPosition = pos + offset;

    if (index.isValid())
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);

    QRect rect(d->pressedPosition - offset, pos);
    setSelection(rect.normalized(), command);

    emit pressed(index);

    if (e->button() == Qt::LeftButton)
        edit(index, SelectedClicked, e);
}

/*!
    This function is called when a mouse move event \a e occurs. If a
    selection is in progress and new items are moved over the
    selection is extended; if a drag is in progress it is continued.
*/
void QAbstractItemView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QAbstractItemView);
    QPoint topLeft;
    QPoint bottomRight = e->pos();

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
        if (index.isValid())
            emit entered(index);
        else
            emit viewportEntered();
        d->enteredIndex = index;
    }

    if (!(e->buttons() & Qt::LeftButton))
        return; // if the left button is not pressed there is nothing more to do

#ifndef QT_NO_DRAGANDDROP
    if (index.isValid() && d->dragEnabled && state() != DragSelectingState) {
        bool dragging = model()->flags(index) & Qt::ItemIsDragEnabled;
        bool selected = selectionModel()->isSelected(index);
        if (dragging && selected) {
            setState(DraggingState);
            return;
        }
    }

    setState(DragSelectingState);
#endif

    if (selectionModel())
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    QRect selectionRect = QRect(topLeft, bottomRight).normalized();
    setSelection(selectionRect, selectionCommand(index, e));
}

/*!
    This function is called when a mouse release event \a e
    occurs. It will emit the clicked() signal if an item was being
    pressed.
*/
void QAbstractItemView::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint pos = e->pos();
    QModelIndex index = indexAt(pos);

    if (state() == EditingState)
        return;

    setState(NoState);

    if (selectionModel() && index == selectionModel()->currentIndex())
        selectionModel()->select(index, selectionCommand(index, e));

    if (index == d_func()->pressedIndex) {
        emit clicked(index);
        if (style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
            emit activated(index);
    }
}

/*!
    This function is called when a mouse double-click event \a e
    occurs. If the double-click is on a valid item it emits the
    doubleClicked() signal and calls edit() on the item.
*/
void QAbstractItemView::mouseDoubleClickEvent(QMouseEvent *e)
{
    QModelIndex index = indexAt(e->pos());
    if (!index.isValid())
        return;
    emit doubleClicked(index);
    if (!edit(index, DoubleClicked, e)
        && !style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
        emit activated(index);
}

#ifndef QT_NO_DRAGANDDROP

/*!
    This function is called when drag enter event \a e occurs. If the
    drag is over a valid dropping place (e.g. over an item that
    accepts drops), the event is accepted.

    \sa dropEvent() startDrag()
*/
void QAbstractItemView::dragEnterEvent(QDragEnterEvent *e)
{
    if (d_func()->canDecode(e))
        e->accept();
    else
        e->ignore();
}

/*!
    This function is called when drag move event \a e occurs. It can
    cause the view to scroll, for example if the user drags a
    selection to view's right or bottom edge.

    \sa dropEvent() startDrag()
*/
void QAbstractItemView::dragMoveEvent(QDragMoveEvent *e)
{
    Q_D(QAbstractItemView);
    // the ignore by default
    e->ignore();

    if (!model())
        return;

    QModelIndex index = indexAt(e->pos());
    index = model()->sibling(index.row(), 0, index);

    if (d->canDecode(e)) {
        if (index.isValid()) {
            // update the drag indicator geometry
            QRect rect = visualRect(index);
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
                if (model()->flags(index) & Qt::ItemIsDropEnabled) {
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
            if (!d->dropIndicator->isVisible() && d->showDropIndicator) {
                d->dropIndicator->show();
                d->dropIndicator->raise();
            }
            e->accept(); // allow dropping on dropenabled items
        } else {
            e->accept(); // allow dropping in empty areas
        }
    } // can decode

    if (d->shouldAutoScroll(e->pos()))
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
    d_func()->dropIndicator->hide();
}

/*!
    This function is called when drop event \a e occurs. If there's a
    valid item under the mouse pointer when the drop occurs, the drop
    is accepted.

    \sa startDrag()
*/
void QAbstractItemView::dropEvent(QDropEvent *e)
{
    Q_D(QAbstractItemView);
    QModelIndex index;
    // if we drop on the viewport
    if (d->viewport->rect().contains(e->pos())) {
        index = indexAt(e->pos());
        if (!index.isValid())
            index = rootIndex(); // drop on viewport
    }
    // if we are allowed to do the drop
    if (model()->supportedDropActions() & e->proposedAction()) {
        int row = -1;
        int col = -1;
        if (index.isValid() &&
            (model()->flags(index) & Qt::ItemIsDropEnabled
             || model()->flags(index.parent()) & Qt::ItemIsDropEnabled)) {
            switch (d->position(e->pos(), visualRect(index), 2)) {
            case QAbstractItemViewPrivate::Above:
                row = index.row();
                col = index.column();
                index = index.parent();
                break;
            case QAbstractItemViewPrivate::Below:
                row = index.row() + 1;
                col = index.column();
                index = index.parent();
                break;
            case QAbstractItemViewPrivate::On:
                break;
            }
        }
        if (model()->dropMimeData(e->mimeData(), e->proposedAction(), row, col, index))
            e->acceptProposedAction();
    }
    stopAutoScroll();
    d->dropIndicator->hide();
}

#endif // QT_NO_DRAGANDDROP

/*!
    This function is called when focus event \a e occurs and is a
    focus in event.
*/
void QAbstractItemView::focusInEvent(QFocusEvent *e)
{
    QAbstractScrollArea::focusInEvent(e);
    QModelIndex index = currentIndex();
    if (index.isValid())
        d_func()->viewport->update(visualRect(index));
}

/*!
    This function is called when focus event \a e occurs and is a
    focus out event.
*/
void QAbstractItemView::focusOutEvent(QFocusEvent *e)
{
    QAbstractScrollArea::focusOutEvent(e);
    QModelIndex index = currentIndex();
    if (index.isValid())
        d_func()->viewport->update(visualRect(index));
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
    Q_D(QAbstractItemView);
    if (!model())
        return;

#ifdef QT_KEYPAD_NAVIGATION
    switch( e->key() ) {
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
            e->ignore();
        return;
    default:
        if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
            e->ignore();
            return;
        }
    }
#endif

    bool hadCurrent = true;
    QModelIndex current = currentIndex();
    if (!current.isValid()) {
        hadCurrent = false;
        setCurrentIndex(model()->index(0, 0, QModelIndex()));
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
            QItemSelectionModel::SelectionFlags command = selectionCommand(newCurrent, e);
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
    }

    switch (e->key()) {
    // ignored keys
    case Qt::Key_Down:
    case Qt::Key_Up:
#ifdef QT_KEYPAD_NAVIGATION
        if (QApplication::keypadNavigationEnabled()) {
            e->accept(); // don't change focus
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
        e->ignore();
        break;
    case Qt::Key_Backtab:
    case Qt::Key_Tab:
        e->accept(); // don't change focus
        break;
    case Qt::Key_Space:
        selectionModel()->select(currentIndex(), selectionCommand(currentIndex(), e));
        break;
#ifdef Q_WS_MAC
    case Qt::Key_Return:
        if (!edit(currentIndex(), EditKeyPressed, e))
            e->ignore();
        break;
    case Qt::Key_O:
        if (e->modifiers() & Qt::ControlModifier)
            emit activated(currentIndex());
        break;
#else
    case Qt::Key_F2:
        if (!edit(currentIndex(), EditKeyPressed, e))
            e->ignore();
        break;
    case Qt::Key_Return:
    case Qt::Key_Select:
        emit activated(currentIndex());
        break;
#endif
    case Qt::Key_A:
        if (e->modifiers() & Qt::ControlModifier) {
            SelectionMode mode = selectionMode();
            if (mode == MultiSelection || mode == ExtendedSelection)
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
}

/*!
    This function is called when a resize event \a e occurs.
*/
void QAbstractItemView::resizeEvent(QResizeEvent *e)
{
    QAbstractScrollArea::resizeEvent(e);
    updateGeometries();
}

/*!
  This function is called when a timer event \a e occurs.
*/
void QAbstractItemView::timerEvent(QTimerEvent *e)
{
    Q_D(QAbstractItemView);
    if (e->timerId() == d->autoScrollTimer.timerId())
        doAutoScroll();
    else if (e->timerId() == d->updateTimer.timerId())
        d->updateDirtyRegion();
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
    EditingState; otherwise returns false.

    The action that caused the editing process is described by
    \a trigger, and the associated event is specified by \a event.

    \sa closeEditor()
*/
bool QAbstractItemView::edit(const QModelIndex &index,
                             EditTrigger trigger,
                             QEvent *event)
{
    Q_D(QAbstractItemView);
    if (!model())
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

    if (event && event->type() == QEvent::KeyPress
        && d->editTriggers & AnyKeyPressed
        && trigger & AnyKeyPressed)
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
    QMap<QPersistentModelIndex, QWidget*>::iterator it = d_func()->editors.begin();
    for (; it != d_func()->editors.end(); ++it)
        itemDelegate()->setEditorData(it.value(), it.key());
}

/*!
    \internal
*/
void QAbstractItemView::updateEditorGeometries()
{
    Q_D(QAbstractItemView);
    QStyleOptionViewItem option = viewOptions();
    QMap<QPersistentModelIndex, QWidget*>::iterator it = d->editors.begin();
    while (it != d->editors.end()) {
        option.rect = visualRect(it.key());
        if (option.rect.isValid())
            it.value()->show();
        else
            it.value()->hide();
        if (it.key().isValid()) {
            itemDelegate()->updateEditorGeometry(it.value(), option, it.key());
            ++it;
        } else {
            // remove editors in deleted indexes
            d->releaseEditor(it.value());
            it = d->editors.erase(it);
        }
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
    if (verticalScrollBar()->maximum() == value)
        model()->fetchMore(rootIndex());
}

/*!
  \internal
*/
void QAbstractItemView::horizontalScrollbarValueChanged(int value)
{
    if (horizontalScrollBar()->maximum() == value)
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
        QObject::disconnect(editor, SIGNAL(destroyed(QObject*)),
                            this, SLOT(editorDestroyed(QObject*)));
        QModelIndex index = d->editors.key(editor);
        d->editors.remove(index);
        d->releaseEditor(editor);
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
    if (!model())
        return;
    QModelIndex index = d_func()->editors.key(editor);
    itemDelegate()->setModelData(editor, model(), index);
}

/*!
  Remove the editor \a editor from the map.
*/
void QAbstractItemView::editorDestroyed(QObject *editor)
{
    QWidget *w = ::qobject_cast<QWidget*>(editor);
    QPersistentModelIndex key = d_func()->editors.key(w);
    d_func()->editors.remove(key);
    d_func()->persistent.removeAll(w);
    if (state() == EditingState)
        setState(NoState);
}

/*!
    Sets the horizontal scrollbar's steps per item to \a steps.

    This is the number of steps used by the horizontal scrollbar to
    represent the width of an item.

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
    if (!model())
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
             ? QItemSelectionModel::ClearAndSelect
             : QItemSelectionModel::NoUpdate));
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
    if(!model())
        return -1;

    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    int height = 0;
    int colCount = model()->columnCount(rootIndex());
    QModelIndex index;
    for (int c = 0; c < colCount; ++c) {
        index = model()->index(row, c, rootIndex());
        height = qMax(height, delegate->sizeHint(option, index).height());
    }
    return height;
}

/*!
    Returns the width size hint for the specified \a column or -1 if there is no model.
*/
int QAbstractItemView::sizeHintForColumn(int column) const
{
    if(!model())
        return -1;

    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    int width = 0;
    int rows = model()->rowCount(rootIndex());
    QModelIndex index;
    for (int r = 0; r < rows; ++r) {
        index = model()->index(r, column, rootIndex());
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
  returns the widget for the item at the given \a index.
*/
QWidget* QAbstractItemView::indexWidget(const QModelIndex &index) const
{
    return d_func()->editors.value(index);
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
    }
}

#ifndef QT_NO_DRAGANDDROP
/*!
    Starts a drag by calling drag->start() using the given \a supportedActions.
*/
void QAbstractItemView::startDrag(Qt::DropActions supportedActions)
{
    Q_D(QAbstractItemView);
    QModelIndexList indexes;
    if (selectionModel())
        indexes = selectionModel()->selectedIndexes();
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
  Prepares the view for scrolling by (\a{dx},\a{dy}) pixels by moving the dirty regions in the
  opposite direction. You only need to call this function if you are implementing a scrolling
  viewport in your view subclass.

  If you implement scrollContentsBy() in a subclass of QAbstractItemView, call this function
  before you call QWidget::scroll() on the viewport. Alternatively, just call update().

  \sa scrollContentsBy(), dirtyRegionOffset()
*/
void QAbstractItemView::scrollDirtyRegion(int dx, int dy)
{
    d_func()->scrollDirtyRegion(dx, dy);
}

/*!
  Returns the offset of the dirty regions in the view.

  If you use scrollDirtyRegion() and implementa paintEvent() in a subclass of QAbstractItemView,
  you should translate the area given by the paint event with the offset returned from this function.

  \sa scrollDirtyRegion()
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
        d->dropIndicator->hide();
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
            if (!(pressedModifiers & Qt::ShiftModifier)
                && !(pressedModifiers & Qt::ControlModifier)
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
                && index == pressedIndex
                && !(pressedModifiers & Qt::ShiftModifier)
                && !(pressedModifiers & Qt::ControlModifier)
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
    if (state == QAbstractItemView::DragSelectingState)
        return QItemSelectionModel::SelectCurrent|selectionBehaviorFlags();

    return QItemSelectionModel::ClearAndSelect|selectionBehaviorFlags();
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
    return (trigger & editTriggers);
}

bool QAbstractItemViewPrivate::shouldAutoScroll(const QPoint &pos)
{
    if (!autoScroll)
        return false;
    QRect area = static_cast<QAbstractItemView*>(viewport)->d_func()->clipRect();  // access QWidget private by bending C++ rules
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
    QWidget *w = editors.value(index);
    if (!w) {
        w = q->itemDelegate()->createEditor(viewport, options, index);
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
        model->removeRows((*it).top(), count, parent);
    }
}
#endif // QT_NO_ITEMVIEWS

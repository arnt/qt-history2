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

#include "qlistwidget.h"
#include <qitemdelegate.h>
#include <qpainter.h>
#include <private/qlistview_p.h>

// workaround for VC++ 6.0 linker bug (?)
typedef bool(*LessThan)(const QListWidgetItem *left, const QListWidgetItem *right);

class QListModel : public QAbstractListModel
{
public:
    QListModel(QListWidget *parent = 0);
    ~QListModel();

    void clear();
    QListWidgetItem *at(int row) const;
    void insert(int row, QListWidgetItem *item);
    void remove(QListWidgetItem *item);
    QListWidgetItem *take(int row);

    int rowCount() const;

    QModelIndex index(QListWidgetItem *item) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

    bool isSortable() const;
    void sort(int column, const QModelIndex &parent, Qt::SortOrder order);

    static bool lessThan(const QListWidgetItem *left, const QListWidgetItem *right);
    static bool greaterThan(const QListWidgetItem *left, const QListWidgetItem *right);

    void itemChanged(QListWidgetItem *item);

private:
    QList<QListWidgetItem*> lst;
};

QListModel::QListModel(QListWidget *parent)
    : QAbstractListModel(parent)
{
}

QListModel::~QListModel()
{
    clear();
}

void QListModel::clear()
{
    for (int i = 0; i < lst.count(); ++i) {
        if (lst.at(i)) {
            lst.at(i)->model = 0;
            delete lst.at(i);
        }
    }
    lst.clear();
    emit reset();
}

QListWidgetItem *QListModel::at(int row) const
{
    if (row >= 0 && row < lst.count())
        return lst.at(row);
    return 0;
}

void QListModel::remove(QListWidgetItem *item)
{
    int row = lst.indexOf(item);
    if (row != -1) {
        lst.at(row)->model = 0;
        lst.removeAt(row);
    }
}

void QListModel::insert(int row, QListWidgetItem *item)
{
    Q_ASSERT(item);
    item->model = this;
    if (row >= 0 && row <= lst.count()) {
        if (lst.contains(item))
            qWarning("The item %p has already been inserted", item);
        lst.insert(row, item);
        emit rowsInserted(QModelIndex::Null, row, row);
    }
}

QListWidgetItem *QListModel::take(int row)
{
    if (row >= 0 && row <= lst.count()) {
        emit rowsRemoved(QModelIndex::Null, row, row);
        lst.at(row)->model = 0;
        return lst.takeAt(row);
    }
    return 0;
}

int QListModel::rowCount() const
{
    return lst.count();
}

QModelIndex QListModel::index(QListWidgetItem *item) const
{
    int row = lst.indexOf(item);
    if (row == -1)
        return QModelIndex::Null;
    return createIndex(row, 0, item);
}

QModelIndex QListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (isValid(row, column, parent))
        return createIndex(row, column, lst.at(row));
    return QModelIndex::Null;
}

QVariant QListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= lst.count())
        return QVariant();
    return lst.at(index.row())->data(role);
}

bool QListModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid() || index.row() >= lst.count())
        return false;
    lst.at(index.row())->setData(role, value);
    emit dataChanged(index, index);
    return true;
}

bool QListModel::insertRows(int row, const QModelIndex &, int count)
{
    QListWidget *view = ::qt_cast<QListWidget*>(QObject::parent());
    if (row < rowCount())
        for (int r = row; r < row + count; ++r)
            lst.insert(r, new QListWidgetItem(view));
    else
        for (int r = 0; r < count; ++r)
            lst.append(new QListWidgetItem(view));
    emit rowsInserted(QModelIndex::Null, row, row + count - 1);
    return true;
}

bool QListModel::removeRows(int row, const QModelIndex &, int count)
{
    if (row < rowCount()) {
        emit rowsRemoved(QModelIndex::Null, row, row + count - 1);
        for (int r = 0; r < count; ++r)
            delete lst.takeAt(row);
        return true;
    }
    return false;
}

QAbstractItemModel::ItemFlags QListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= lst.count())
        return 0;
    return lst.at(index.row())->flags();
}

bool QListModel::isSortable() const
{
    return true;
}

void QListModel::sort(int column, const QModelIndex &parent, Qt::SortOrder order)
{
    if (column != 0 || parent.isValid())
        return;
    LessThan compare = (order == Qt::AscendingOrder ? &lessThan : &greaterThan);
    qHeapSort(lst.begin(), lst.end(), compare);
    emit dataChanged(index(0, 0), index(lst.count() - 1, 0));
}

bool QListModel::lessThan(const QListWidgetItem *left, const QListWidgetItem *right)
{
    return *left < *right;
}

bool QListModel::greaterThan(const QListWidgetItem *left, const QListWidgetItem *right)
{
    return !(*left < *right);
}

void QListModel::itemChanged(QListWidgetItem *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}

/*!
    \class QListWidgetItem
    \brief The QListWidgetItem class provides an item for use with the
    QListWidget item view class.

    \ingroup model-view

    The QListWidgetItem class provides a list item for use with the QListWidget
    class. List items provide label information that is displayed in list
    widgets.

    The item view convenience classes use a classic item-based interface
    rather than a pure Model/View approach. For a more flexible list view
    widget, consider using the QListView class with a standard model.

    List items can be automatically inserted into a list when they are
    constructed by specifying the list widget:

    \quotefile snippets/qlistwidget-using/mainwindow.cpp
    \skipto new QListWidgetItem(tr("Hazel
    \printuntil new QListWidgetItem(tr("Hazel

    They can also be created without a parent widget, and later inserted into
    a list (see \l{QListWidget::insertItem()}).

    List items are typically used to display text() and an icon(). These are
    set with the setText() and setIcon() functions. The appearance of the text
    can be customized with setFont(), setTextColor(), and setBackgroundColor().
    List items can be aligned using the setAlignment() function.
    Tooltips, status tips and "What's This?" help can be added to list items
    with setToolTip(), setStatusTip(), and setWhatsThis().

    Items can be made checkable by calling setFlags() with the appropriate
    value (see \l{QAbstractItemModel::ItemFlags}). Checkable items can be
    checked and unchecked with the setChecked() function. The corresponding
    checked() function indicates whether the item is currently checked.

    The isItemHidden() function can be used to determine whether the
    item is hidden.  Items can be hidden with setItemHidden().

    \sa QListWidgetItem \link model-view-programming.html Model/View Programming\endlink
*/

/*!
    \fn QListWidgetItem::QListWidgetItem(QListWidget *parent)

    Constructs an empty list widget item with the given \a parent.
    If the parent is not specified, the item will need to be inserted into a
    list widget with QListWidget::insertItem().
*/
QListWidgetItem::QListWidgetItem(QListWidget *view)
    : view(view), model(0),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled)
{
    if (view)
        model = ::qt_cast<QListModel*>(view->model());
    if (model)
        model->insert(model->rowCount(), this);
}

/*!
    \fn QListWidgetItem::QListWidgetItem(const QString &text, QListWidget *parent)

    Constructs an empty list widget item with the given \a text and \a parent.
    If the parent is not specified, the item will need to be inserted into a
    list widget with QListWidget::insertItem().
*/
QListWidgetItem::QListWidgetItem(const QString &text, QListWidget *view)
    : view(view), model(0),
      itemFlags(QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsEnabled)
{
    setData(QAbstractItemModel::DisplayRole, text);
    if (view)
        model = ::qt_cast<QListModel*>(view->model());
    if (model)
        model->insert(model->rowCount(), this);
}

/*!
  Destroys the list item.
*/
QListWidgetItem::~QListWidgetItem()
{
    if (model)
        model->remove(this);
}

/*!
  This function sets the data for a given \a role to the given \a value (see
  \l{QAbstractItemModel::Role}). Reimplement this function if you need
  extra roles or special behavior for certain roles.
*/
void QListWidgetItem::setData(int role, const QVariant &value)
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).role == role) {
            values[i].value = value;
            return;
        }
    }
    values.append(Data(role, value));
    if (model)
        model->itemChanged(this);
}

/*!
   This function returns the item's data for a given \a role (see
   {QAbstractItemModel::Role}). Reimplement this function if you need
   extra roles or special behavior for certain roles.
*/
QVariant QListWidgetItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
}

/*!
  Returns true if this item's text is less then \a other item's text;
  otherwise returns false.
*/
bool QListWidgetItem::operator<(const QListWidgetItem &other) const
{
    return text() < other.text();
}

/*!
  Removes all list items.
*/
void QListWidgetItem::clear()
{
    values.clear();
    if (model)
        model->itemChanged(this);
}

/*!
  \fn QAbstractItemModel::ItemFlags QListWidgetItem::flags() const

  Returns the item flags for this item (see {QAbstractItemModel::ItemFlags}).
*/

/*!
    \fn QString QListWidgetItem::text() const

    Returns the list item's text.

    \sa setText()
*/

/*!
    \fn QIcon QListWidgetItem::icon() const

    Returns the list item's icon.

    \sa setIcon()
*/

/*!
    \fn QString QListWidgetItem::statusTip() const

    Returns the list item's status tip.

    \sa setStatusTip()
*/

/*!
    \fn QString QListWidgetItem::toolTip() const

    Returns the list item's tooltip.

    \sa setToolTip() statusTip() whatsThis()
*/

/*!
    \fn QString QListWidgetItem::whatsThis() const

    Returns the list item's "What's This?" help text.

    \sa setWhatsThis() statusTip() toolTip()
*/

/*!
  \fn QFont QListWidgetItem::font() const

  Returns the font used to display this list item's text.
*/

/*!
  \fn int QListWidgetItem::textAlignment() const

  Returns the text alignment for the list item (see \l{Qt::AlignmentFlag}).
*/

/*!
    \fn QColor QListWidgetItem::backgroundColor() const

    Returns the color used to display the list item's background.

    \sa setBackgroundColor() textColor()
*/

/*!
    \fn QColor QListWidgetItem::textColor() const

    Returns the used to display the list item's text.

    \sa setTextColor() backgroundColor()
*/

/*!
    \fn int QListWidgetItem::checked() const

    Returns the checked state of the list item (see \l{QCheckBox::ToggleState}).
*/

/*!
  \fn void QListWidgetItem::setFlags(QAbstractItemModel::ItemFlags flags)

  Sets the item flags for the list item to \a flags (see
  \l{QAbstractItemModel::ItemFlags}).
*/

/*!
    \fn void QListWidgetItem::setText(const QString &text)

    Sets the text for the list widget item's to the given \a text.

    \sa text()
*/

/*!
    \fn void QListWidgetItem::setIcon(const QIcon &icon)

    Sets the icon for the list item to the given \a icon.

    \sa icon()
*/

/*!
    \fn void QListWidgetItem::setStatusTip(const QString &statusTip)

    Sets the status tip for the list item to the text specified by
    \a statusTip.

    \sa statusTip() setToolTip() setWhatsThis()
*/

/*!
    \fn void QListWidgetItem::setToolTip(const QString &toolTip)

    Sets the tooltip for the list item to the text specified by \a toolTip.

    \sa toolTip() setStatusTip() setWhatsThis()
*/

/*!
    \fn void QListWidgetItem::setWhatsThis(const QString &whatsThis)

    Sets the "What's This?" help for the list item to the text specified
    by \a whatsThis.

    \sa whatsThis() setStatusTip() setToolTip()
*/

/*!
  \fn void QListWidgetItem::setFont(const QFont &font)

  Sets the font used when painting the item to the given \a font.
*/

/*!
  \fn void QListWidgetItem::setTextAlignment(int alignment)

  Sets the list item's text alignment to \a alignment (see
  \l{Qt::AlignmentFlag}).
*/

/*!
    \fn void QListWidgetItem::setBackgroundColor(const QColor &color)

    Sets the background color of the list item to the given \a color.

    \sa backgroundColor() setTextColor()
*/

/*!
    \fn void QListWidgetItem::setTextColor(const QColor &color)

    Sets the text color for the list item to the given \a color.

    \sa textColor() setBackgroundColor()
*/

/*!
    \fn void QListWidgetItem::setChecked(const bool checked)

    Checks the list item if \a checked is true; otherwise the list item
    will be shown as unchecked.

    \sa checked()
*/

#define d d_func()
#define q q_func()

class QListWidgetPrivate : public QListViewPrivate
{
    Q_DECLARE_PUBLIC(QListWidget)
public:
    QListWidgetPrivate() : QListViewPrivate() {}
    inline QListModel *model() const { return ::qt_cast<QListModel*>(q_func()->model()); }
    void emitPressed(const QModelIndex &index, Qt::ButtonState button);
    void emitClicked(const QModelIndex &index, Qt::ButtonState button);
    void emitDoubleClicked(const QModelIndex &index, Qt::ButtonState button);
    void emitKeyPressed(const QModelIndex &index, Qt::Key key, Qt::ButtonState state);
    void emitReturnPressed(const QModelIndex &index);
    void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current);
    void emitItemEntered(const QModelIndex &index, Qt::ButtonState state);
    void emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index);
    void emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
};

void QListWidgetPrivate::emitPressed(const QModelIndex &index, Qt::ButtonState button)
{
    emit q->pressed(model()->at(index.row()), button);
}

void QListWidgetPrivate::emitClicked(const QModelIndex &index, Qt::ButtonState button)
{
    emit q->clicked(model()->at(index.row()), button);
}

void QListWidgetPrivate::emitDoubleClicked(const QModelIndex &index, Qt::ButtonState button)
{
    emit q->doubleClicked(model()->at(index.row()), button);
}

void QListWidgetPrivate::emitKeyPressed(const QModelIndex &index, Qt::Key key,
                                        Qt::ButtonState state)
{
    emit q->keyPressed(model()->at(index.row()), key, state);
}

void QListWidgetPrivate::emitReturnPressed(const QModelIndex &index)
{
    emit q->returnPressed(model()->at(index.row()));
}

void QListWidgetPrivate::emitCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit q->currentChanged(model()->at(current.row()), model()->at(previous.row()));
}

void QListWidgetPrivate::emitItemEntered(const QModelIndex &index, Qt::ButtonState state)
{
    emit q->itemEntered(model()->at(index.row()), state);
}

void QListWidgetPrivate::emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index)
{
    emit q->aboutToShowContextMenu(menu, model()->at(index.row()));
}

void QListWidgetPrivate::emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft == bottomRight) // this should always be true, unless we sort
        emit q->itemChanged(model()->at(topLeft.row()));
}

/*!
    \class QListWidget
    \brief The QListWidget class provides an item-based list widget.

    \ingroup model-view
    \mainclass

    QListWidget is a convenience class that provides a list view similar to
    the one supplied by QListView, but with a classic item-based interface
    for adding and removing items from the list. QListWidget uses an internal
    model to manage the items.

    For a more flexible list view widget, use the QListView class with a
    standard model.

    List widgets are constructed in the same way as other widgets:

    \quotefile snippets/qlistwidget-using/mainwindow.h
    \skipto QListWidget *
    \printuntil QListWidget *
    \quotefile snippets/qlistwidget-using/mainwindow.cpp
    \skipto listWidget = new

    The selectionMode() of a list widget determines how many of the items in
    the list can be selected at the same time, and whether complex selections
    of items can be created. This can be set with the setSelectionMode()
    function.

    There are two ways to add items to the list: they can be constructed with
    the list widget as their parent widget, or they can be constructed with
    no parent widget and added to the list later. If a list widget already
    exists when the items are constructed, the first method is easier to use:

    \skipto new QListWidgetItem
    \printuntil new QListWidgetItem(tr("Pine")

    If you need to insert a new item into the list at a particular position,
    it is more convenient to construct the item without a parent widget and
    use the insertItem() function to place it within the list:

    \skipto QListWidgetItem *newItem
    \printuntil newItem->setText
    \skipto listWidget->insertItem
    \printuntil listWidget->insertItem

    For multiple items, insertItems() can be used instead. The number of
    items in the list is found with the count() function.
    To remove items from the list, use removeItem().

    The current item in the list can be found with currentItem(), and changed
    with setCurrentItem(). The user can also change the current item by
    navigating with the keyboard or clicking on a different item. When the
    current item changes, the currentChanged() signal is emitted with the
    new current item and the item that was previously current.

    \sa QListWidgetItem \link model-view-programming.html Model/View Programming\endlink
*/


/*!
    \fn void QListWidget::insertItem(int row, const QString &label)

    Inserts an item with the text \a label in the list widget at the
    position given by \a row.

    \sa appendItem()
*/

/*!
    \fn void QListWidget::appendItem(QListWidgetItem *item)

    Inserts the \a item at the the end of the list widget.

    \sa insertItem()
*/

/*!
    \fn void QListWidget::appendItem(const QString &label)

    Inserts an item with the text \a label at the end of the list
    widget.
*/

/*!
    \fn void QListWidget::appendItems(const QStringList &labels)

    Inserts items with the text \a labels at the end of the list widget.

    \sa insertItems()
*/

/*!
    \fn void QListWidget::pressed(QListWidgetItem *item, Qt::ButtonState button)

    This signal is emitted when a item has been pressed (mouse click
    and release). The \a item may be 0 if the mouse was not pressed on
    an item. The button clicked is specified by \a button (see
    \l{Qt::ButtonState}).
*/

/*!
    \fn void QListWidget::clicked(QListWidgetItem *item, Qt::ButtonState button)

    This signal is emitted when a mouse button is clicked. The \a item
    may be 0 if the mouse was not clicked on an item.  The button
    clicked is specified by \a button (see \l{Qt::ButtonState}).
*/

/*!
    \fn void QListWidget::doubleClicked(QListWidgetItem *item, Qt::ButtonState button);

    This signal is emitted when a mouse button is double clicked. The
    \a item may be 0 if the mouse was not clicked on an item.  The
    button clicked is specified by \a button (see
    \l{Qt::ButtonState}).
*/

/*!
    \fn void QListWidget::keyPressed(QListWidgetItem *item, Qt::Key key, Qt::ButtonState state)

    This signal is emitted if keyTracking is turned on an a key was
    pressed. The \a item is the current item when the key was pressed, the
    \a key tells which key was pressed and \a state which modifier
    keys (see \l{Qt::ButtonState}).
*/

/*!
    \fn void QListWidget::returnPressed(QListWidgetItem *item)

    This signal is emitted when return has been pressed on an \a item.
*/

/*!
    \fn void QListWidget::currentChanged(QListWidgetItem *current, QListWidgetItem *previous)

    This signal is emitted whenever the current item changes. The \a
    previous item is the item that previously had the focus, \a
    current is the new current item.
*/

/*!
    \fn void QListWidget::selectionChanged()

    This signal is emitted whenever the selection changes.

    \sa selectedItems() isSelected()
*/

/*!
    \fn void QListWidget::itemEntered(QListWidgetItem *item, Qt::ButtonState state)

    This signal is emitted when the mouse cursor enters an item. The
    \a item is the item entered and \a state specifies the mouse
    button and any modifiers pressed as the item was entered (see
    \l{Qt::ButtonState}). This signal is only emitted when
    mouseTracking is turned on, or when a mouse button is pressed
    while moving into an item.
*/

/*!
    \fn void QListWidget::aboutToShowContextMenu(QMenu *menu, QListWidgetItem *item)

    This signal is emitted when the widget is about to show a context
    menu. The \a menu is the menu about to be shown, and the \a item
    is the clicked item the context menu was called for.

    \sa QMenu::addAction()
*/

/*!
    \fn void QListWidget::itemChanged(QListWidgetItem *item)

    This signal is emitted whenever the data of \a item has changed.
*/



/*!
    Constructs an empty QListWidget with the given \a parent.
*/

QListWidget::QListWidget(QWidget *parent)
    : QListView(*new QListWidgetPrivate(), parent)
{
    setModel(new QListModel(this));
    setup();
}

/*!
    Destroys the list widget and all its items.
*/

QListWidget::~QListWidget()
{
}

/*!
    Returns the item that occupies the given \a row in the list.

    \sa row()
*/

QListWidgetItem *QListWidget::item(int row) const
{
    return d->model()->at(row);
}

/*!
    Returns the row containing the given \a item.

    \sa item()
*/

int QListWidget::row(const QListWidgetItem *item) const
{
    Q_ASSERT(item);
    return d->model()->index(const_cast<QListWidgetItem*>(item)).row();
}


/*!
    Inserts the \a item at the position in the list given by \a row.

    \sa appendItem()
*/

void QListWidget::insertItem(int row, QListWidgetItem *item)
{
    d->model()->insert(row, item);
}

/*!
    Inserts items from the list of \a labels into the list, starting at the
    given \a row.

    \sa insertItem(), appendItem()
*/

void QListWidget::insertItems(int row, const QStringList &labels)
{
    QListModel *model = d->model();
    int r = (row > -1 && row <= count()) ? row : count();
    for (int i = 0; i < labels.count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(labels.at(i));
        model->insert(r + i, item);
    }
}

/*!
    Removes and returns the item from the given \a row in the list widget.

    \sa insertItem() appendItem()
*/

QListWidgetItem *QListWidget::takeItem(int row)
{
    return d->model()->take(row);
}

/*!
      Returns the number of items in the list.

*/
int QListWidget::count() const
{
    return d->model()->rowCount();
}

/*!
  Returns the current item.
*/
QListWidgetItem *QListWidget::currentItem() const
{
    return d->model()->at(currentIndex().row());
}


/*!
  Sets the current item to \a item.
*/
void QListWidget::setCurrentItem(QListWidgetItem *item)
{
    setCurrentIndex(d->model()->index(item));
}

/*!
  Sorts all the items in the list widget according to the specified \a order.
*/
void QListWidget::sortItems(Qt::SortOrder order)
{
    d->model()->sort(0, QModelIndex::Null, order);
}

/*!
  Opens an editor for the given \a item. The editor remains open after editing.

  \sa closePersistentEditor()
*/
void QListWidget::openPersistentEditor(QListWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item);
    QAbstractItemView::openPersistentEditor(index);
}

/*!
  Closes the persistent editor for the given \a item.

  \sa openPersistentEditor()
*/
void QListWidget::closePersistentEditor(QListWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item);
    QAbstractItemView::closePersistentEditor(index);
}

/*!
  Returns true if \a item is selected; otherwise returns false.
*/
bool QListWidget::isSelected(const QListWidgetItem *item) const
{
    QModelIndex index = d->model()->index(const_cast<QListWidgetItem*>(item));
    return selectionModel()->isSelected(index);
}

/*!
  Selects or deselects the given \a item depending on whether \a select is
  true of false.
*/
void QListWidget::setSelected(const QListWidgetItem *item, bool select)
{
    QModelIndex index = d->model()->index(const_cast<QListWidgetItem*>(item));
    selectionModel()->select(index, select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Returns a list of all selected items in the list widget.
*/

QList<QListWidgetItem*> QListWidget::selectedItems() const
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    QList<QListWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items.append(d->model()->at(indexes.at(i).row()));
    return items;
}

/*!
  Finds items that matches the \a text, using the criteria given in the
  \a flags (see {QAbstractItemModel::MatchFlags}).
*/

QList<QListWidgetItem*> QListWidget::findItems(const QString &text,
                                               QAbstractItemModel::MatchFlags flags) const
{
    QModelIndex topLeft = d->model()->index(0, 0);
    int role = QAbstractItemModel::DisplayRole;
    int hits = d->model()->rowCount();
    QModelIndexList indexes = d->model()->match(topLeft, role, text,hits, flags);
    QList<QListWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items << d->model()->at(indexes.at(i).row());
    return items;
}

/*!
  Returns true if the \a item is explicitly hidden; otherwise returns false.
*/
bool QListWidget::isItemHidden(const QListWidgetItem *item) const
{
    return isRowHidden(row(item));
}

/*!
  If \a hide is true, the \a item will be hidden; otherwise it will be shown.
*/
void QListWidget::setItemHidden(const QListWidgetItem *item, bool hide)
{
    setRowHidden(row(item), hide);
}

/*!
  Returns true if the \a item is in the viewport; otherwise returns false.
*/

bool QListWidget::isItemVisible(const QListWidgetItem *item) const
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QListWidgetItem*>(item));
    QRect rect = itemViewportRect(index);
    return rect.isValid() && d->viewport->rect().contains(rect);
}

/*!
  Scrolls the view if necessary to ensure that the \a item is visible.
*/

void QListWidget::ensureItemVisible(const QListWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QListWidgetItem*>(item));
    QListView::ensureItemVisible(index);
}

/*!
  Removes all items in the view.
*/
void QListWidget::clear()
{
    d->model()->clear();
}

/*!
  \internal
*/
void QListWidget::setModel(QAbstractItemModel *model)
{
    QListView::setModel(model);
}

/*!
  \internal
*/
void QListWidget::setup()
{
    setModel(new QListModel(this));
    connect(this, SIGNAL(pressed(QModelIndex,ButtonState)),
            SLOT(emitPressed(QModelIndex,ButtonState)));
    connect(this, SIGNAL(clicked(QModelIndex,ButtonState)),
            SLOT(emitClicked(QModelIndex,ButtonState)));
    connect(this, SIGNAL(doubleClicked(QModelIndex,ButtonState)),
            SLOT(emitDoubleClicked(QModelIndex,ButtonState)));
    connect(this, SIGNAL(keyPressed(QModelIndex,Key,ButtonState)),
            SLOT(emitKeyPressed(QModelIndex,Key,ButtonState)));
    connect(this, SIGNAL(returnPressed(QModelIndex)),
            SLOT(emitReturnPressed(QModelIndex)));
    connect(this, SIGNAL(itemEntered(QModelIndex,ButtonState)),
            SLOT(emitItemEntered(QModelIndex,ButtonState)));
    connect(this, SIGNAL(aboutToShowContextMenu(QMenu*,QModelIndex)),
            SLOT(emitAboutToShowContextMenu(QMenu*,QModelIndex)));
    connect(selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(emitCurrentChanged(QModelIndex,QModelIndex)));
    connect(selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SIGNAL(selectionChanged()));
    connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(emitItemChanged(QModelIndex,QModelIndex)));
}

#include "moc_qlistwidget.cpp"

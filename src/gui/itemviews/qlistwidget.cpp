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
    Q_OBJECT
public:
    QListModel(QListWidget *parent = 0);
    ~QListModel();

    void clear();
    QListWidgetItem *at(int row) const;
    void insert(int row, QListWidgetItem *item);
    void remove(QListWidgetItem *item);
    QListWidgetItem *take(int row);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex index(QListWidgetItem *item) const;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    bool insertRows(int row, int count = 1, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex());

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

    void sort(int column, Qt::SortOrder order);
    static bool itemLessThan(const QListWidgetItem *left, const QListWidgetItem *right);
    static bool itemGreaterThan(const QListWidgetItem *left, const QListWidgetItem *right);
    QList<QListWidgetItem*> find(const QRegExp &rx) const;

    void itemChanged(QListWidgetItem *item);

protected:
    void updatePersistentIndexes(int row, int count);

private:
    QList<QListWidgetItem*> lst;
};

#include "qlistwidget.moc"

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
    Q_ASSERT(item);
    int row = lst.indexOf(item);
    Q_ASSERT(row != -1);
    emit rowsAboutToBeRemoved(QModelIndex(), row, row);
    updatePersistentIndexes(row, -1);
    lst.at(row)->model = 0;
    lst.removeAt(row);
}

void QListModel::insert(int row, QListWidgetItem *item)
{
    Q_ASSERT(item);
    item->model = this;
    Q_ASSERT(row >= 0 && row <= lst.count());
    if (lst.contains(item))
        qWarning("The item %p has already been inserted", item);
    lst.insert(row, item);
    updatePersistentIndexes(row, 1);
    emit rowsInserted(QModelIndex(), row, row);
}

QListWidgetItem *QListModel::take(int row)
{
    Q_ASSERT(row >= 0 && row < lst.count());
    emit rowsAboutToBeRemoved(QModelIndex(), row, row);
    updatePersistentIndexes(row, -1);
    lst.at(row)->model = 0;
    return lst.takeAt(row);
}

int QListModel::rowCount(const QModelIndex &) const
{
    return lst.count();
}

QModelIndex QListModel::index(QListWidgetItem *item) const
{
    Q_ASSERT(item);
    int row = lst.indexOf(item);
    Q_ASSERT(row != -1);
    return createIndex(row, 0, item);
}

QModelIndex QListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return createIndex(row, column, lst.at(row));
    return QModelIndex();
}

QVariant QListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= lst.count())
        return QVariant();
    return lst.at(index.row())->data(role);
}

bool QListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= lst.count())
        return false;
    lst.at(index.row())->setData(role, value);
    emit dataChanged(index, index);
    return true;
}

bool QListModel::insertRows(int row, int count, const QModelIndex &)
{
    // insert rows
    QListWidget *view = ::qobject_cast<QListWidget*>(QObject::parent());
    QListWidgetItem *itm = 0;
    if (row < rowCount()) {
        for (int r = row; r < row + count; ++r) {
            itm = new QListWidgetItem();
            itm->view = view;
            itm->model = this;
            lst.insert(r, itm);
        }
    } else {
        for (int r = 0; r < count; ++r) {
            itm = new QListWidgetItem();
            itm->view = view;
            itm->model = this;
            lst.append(itm);
        }
    }
    updatePersistentIndexes(row, count);
    emit rowsInserted(QModelIndex(), row, row + count - 1);
    return true;
}

bool QListModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row >= 0 && row < rowCount()) {
        emit rowsAboutToBeRemoved(QModelIndex(), row, row + count - 1);
        // remove items
        QListWidgetItem *itm = 0;
        for (int r = row; r < row + count; ++r) {
            itm = lst.takeAt(row);
            itm->view = 0;
            itm->model = 0;
            delete itm;
        }
        updatePersistentIndexes(row, -count);
        return true;
    }
    return false;
}

QAbstractItemModel::ItemFlags QListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= lst.count())
        return QAbstractItemModel::ItemIsDropEnabled; // we allow drops outside the items
    return lst.at(index.row())->flags();
}

void QListModel::sort(int column, Qt::SortOrder order)
{
    if (column != 0)
        return;
    LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
    qSort(lst.begin(), lst.end(), compare);
    emit dataChanged(index(0, 0), index(lst.count() - 1, 0));
}

bool QListModel::itemLessThan(const QListWidgetItem *left, const QListWidgetItem *right)
{
    return *left < *right;
}

bool QListModel::itemGreaterThan(const QListWidgetItem *left, const QListWidgetItem *right)
{
    return !(*left < *right);
}

QList<QListWidgetItem*> QListModel::find(const QRegExp &rx) const
{
    QList<QListWidgetItem*> result;
    QList<QListWidgetItem*>::const_iterator it = lst.begin();
    for (; it != lst.end(); ++it) {
        if (rx.exactMatch((*it)->text()))
            result << (*it);
    }
    return result;
}

void QListModel::itemChanged(QListWidgetItem *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}

void QListModel::updatePersistentIndexes(int row, int count)
{
    // update persistent model indexes
    for (int i = 0; i < persistentIndexesCount(); ++i) {
        int r = persistentIndexAt(i).row();
        if (r >= lst.count()) // deleted at the end
            setPersistentIndex(i, QModelIndex());
        else if (r >= row) // update existing
            setPersistentIndex(i, index(r + count));
    }
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
    rather than a pure model/view approach. For a more flexible list view
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
    \fn QListWidget *QListWidgetItem::listWidget() const

    Returns the list widget that contains the item.
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
                |QAbstractItemModel::ItemIsUserCheckable
                |QAbstractItemModel::ItemIsEnabled)
{
    if (view)
        model = ::qobject_cast<QListModel*>(view->model());
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
                |QAbstractItemModel::ItemIsUserCheckable
                |QAbstractItemModel::ItemIsEnabled)
{
    setData(QAbstractItemModel::DisplayRole, text);
    if (view)
        model = ::qobject_cast<QListModel*>(view->model());
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
  Creates an exact copy of the item.
*/
QListWidgetItem *QListWidgetItem::clone() const
{
    QListWidgetItem * item = new QListWidgetItem();
    *item = *this;
    return item;
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
            break;
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

#ifndef QT_NO_DATASTREAM

/*!
  Reads the item from the \a stream.
*/
void QListWidgetItem::read(QDataStream &stream)
{
    int count;
    int role;
    QVariant value;
    stream >> count;
    for (int i = 0; i < count; ++i) {
        stream >> role;
        stream >> value;
        values.append(Data(role, value));
    }
}

/*!
  Writes the item to the \a stream.
*/
void QListWidgetItem::write(QDataStream &stream) const
{
    stream << values.count();
    for (int i = 0; i < values.count(); ++i) {
        stream << values.at(i).role;
        stream << values.at(i).value;
    }
}

QDataStream &operator<<(QDataStream &stream, QListWidgetItem &item)
{
    item.read(stream);
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const QListWidgetItem &item)
{
    item.write(stream);
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QListWidgetItem &item)
{
    item.read(stream);
    return stream;
}

QDataStream &operator>>(QDataStream &stream, const QListWidgetItem &item)
{
    item.write(stream);
    return stream;
}

#endif // QT_NO_DATASTREAM

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
    \fn Qt::CheckState QListWidgetItem::checkState() const

    Returns the checked state of the list item (see \l{Qt::CheckState}.

    \sa flags()
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
    \fn void QListWidgetItem::setCheckState(Qt::CheckState state)

    Sets the check state of the list item to \a state.

    \sa checkState()
*/

#define d d_func()
#define q q_func()

class QListWidgetPrivate : public QListViewPrivate
{
    Q_DECLARE_PUBLIC(QListWidget)
public:
    QListWidgetPrivate() : QListViewPrivate() {}
    inline QListModel *model() const { return ::qobject_cast<QListModel*>(q_func()->model()); }
    void setup();
    void emitItemPressed(const QModelIndex &index);
    void emitItemClicked(const QModelIndex &index);
    void emitItemDoubleClicked(const QModelIndex &index);
    void emitItemActivated(const QModelIndex &index);
    void emitItemEntered(const QModelIndex &index);
    void emitItemChanged(const QModelIndex &index);
    void emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &current);
};

void QListWidgetPrivate::setup()
{
    q->setModel(new QListModel(q));
    // view signals
    QObject::connect(q, SIGNAL(pressed(QModelIndex)), q, SLOT(emitItemPressed(QModelIndex)));
    QObject::connect(q, SIGNAL(clicked(QModelIndex)), q, SLOT(emitItemClicked(QModelIndex)));
    QObject::connect(q, SIGNAL(doubleClicked(QModelIndex)),
                     q, SLOT(emitItemDoubleClicked(QModelIndex)));
    QObject::connect(q, SIGNAL(activated(QModelIndex)), q, SLOT(emitItemActivated(QModelIndex)));
    QObject::connect(q, SIGNAL(entered(QModelIndex)), q, SLOT(emitItemEntered(QModelIndex)));
    // model signals
    QObject::connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                     q, SLOT(emitItemChanged(QModelIndex)));
    // selection signals
    QObject::connect(q->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                     q, SLOT(emitCurrentItemChanged(QModelIndex,QModelIndex)));
    QObject::connect(q->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                     q, SIGNAL(itemSelectionChanged()));
}

void QListWidgetPrivate::emitItemPressed(const QModelIndex &index)
{
    emit q->itemPressed(model()->at(index.row()));
}

void QListWidgetPrivate::emitItemClicked(const QModelIndex &index)
{
    emit q->itemClicked(model()->at(index.row()));
}

void QListWidgetPrivate::emitItemDoubleClicked(const QModelIndex &index)
{
    emit q->itemDoubleClicked(model()->at(index.row()));
}

void QListWidgetPrivate::emitItemActivated(const QModelIndex &index)
{
    emit q->itemActivated(model()->at(index.row()));
}

void QListWidgetPrivate::emitItemEntered(const QModelIndex &index)
{
    emit q->itemEntered(model()->at(index.row()));
}

void QListWidgetPrivate::emitItemChanged(const QModelIndex &index)
{
    emit q->itemChanged(model()->at(index.row()));
}

void QListWidgetPrivate::emitCurrentItemChanged(const QModelIndex &current,
                                                const QModelIndex &previous)
{
    QListWidgetItem *currentItem = model()->at(current.row());
    emit q->currentItemChanged(currentItem, model()->at(previous.row()));
    emit q->currentTextChanged(currentItem ? currentItem->text() : QString());
    emit q->currentRowChanged(current.row());
}

/*!
    \class QListWidget
    \brief The QListWidget class provides an item-based list widget.

    \ingroup model-view
    \mainclass

    QListWidget is a convenience class that provides a list view similar to
    the one supplied by QListView, but with a classic item-based interface
    for adding and removing items. QListWidget uses an internal model to
    manage each QListWidgetItem in the list.

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
    current item changes, the currentItemChanged() signal is emitted with the
    new current item and the item that was previously current.

    \sa QListWidgetItem \link model-view-programming.html Model/View Programming\endlink
*/

/*!
    \fn void QListWidget::addItem(QListWidgetItem *item)

    Inserts the \a item at the the end of the list widget.

    \sa insertItem()
*/

/*!
    \fn void QListWidget::addItem(const QString &label)

    Inserts an item with the text \a label at the end of the list
    widget.
*/

/*!
    \fn void QListWidget::addItems(const QStringList &labels)

    Inserts items with the text \a labels at the end of the list widget.

    \sa insertItems()
*/

/*!
    \fn void QListWidget::itemPressed(QListWidgetItem *item)

    This signal is emitted when a item has been pressed (mouse click
    and release). The \a item may be 0 if the mouse was not pressed on
    an item.
*/

/*!
    \fn void QListWidget::itemClicked(QListWidgetItem *item)

    This signal is emitted when a mouse button is clicked. The \a item
    may be 0 if the mouse was not clicked on an item.
*/

/*!
    \fn void QListWidget::itemDoubleClicked(QListWidgetItem *item)

    This signal is emitted when a mouse button is double clicked. The
    \a item may be 0 if the mouse was not clicked on an item.
*/

/*!
  \fn void QListWidget::itemActivated(QListWidgetItem *item)

  This signal is emitted when ... ###
*/

/*!
    \fn void QListWidget::itemEntered(QListWidgetItem *item)

    This signal is emitted when the mouse cursor enters an item. The
    \a item is the item entered. This signal is only emitted when
    mouseTracking is turned on, or when a mouse button is pressed
    while moving into an item.
*/

/*!
    \fn void QListWidget::itemChanged(QListWidgetItem *item)

    This signal is emitted whenever the data of \a item has changed.
*/

/*!
    \fn void QListWidget::currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)

    This signal is emitted whenever the current item changes. The \a
    previous item is the item that previously had the focus, \a
    current is the new current item.
*/

/*!
  \fn void QListWidget::currentTextChanged(const QString &currentText)

  This signal is emitted whenever the current item changes. The \a currentText
  is the text data in the current item. If there is no current item, the \a currentText
  is invalid.
*/

/*!
  \fn void QListWidget::currentRowChanged(int currentRow)

  This signal is emitted whenever the current item changes. The \a currentRow
  is the row of the current item. If there is no current item, the \a currentRow is -1.
*/

/*!
    \fn void QListWidget::itemSelectionChanged()

    This signal is emitted whenever the selection changes.

    \sa selectedItems() isItemSelected()
*/

/*!
    Constructs an empty QListWidget with the given \a parent.
*/

QListWidget::QListWidget(QWidget *parent)
    : QListView(*new QListWidgetPrivate(), parent)
{
    d->setup();
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

    \sa addItem()
*/

void QListWidget::insertItem(int row, QListWidgetItem *item)
{
    d->model()->insert(row, item);
}

/*!
    Inserts an item with the text \a label in the list widget at the
    position given by \a row.

    \sa addItem()
*/

void QListWidget::insertItem(int row, const QString &label)
{
    d->model()->insert(row, new QListWidgetItem(label));
}

/*!
    Inserts items from the list of \a labels into the list, starting at the
    given \a row.

    \sa insertItem(), addItem()
*/

void QListWidget::insertItems(int row, const QStringList &labels)
{
    QListModel *model = d->model();
    int r = (row > -1 && row <= count()) ? row : count();
    for (int i = 0; i < labels.count(); ++i)
        model->insert(r + i, new QListWidgetItem(labels.at(i)));
}

/*!
    Removes and returns the item from the given \a row in the list
    widget, otherwise return 0;

    \sa insertItem() addItem()
*/

QListWidgetItem *QListWidget::takeItem(int row)
{
    if (row < 0 || row >= d->model()->rowCount())
        return 0;
    return d->model()->take(row);
}

/*!
  \property QListWidget::count
  \brief the number of items in the list including any hidden items.
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
    selectionModel()->setCurrentIndex(d->model()->index(item),
                                      d->selectionMode == SingleSelection
                                      ? QItemSelectionModel::ClearAndSelect
                                      : QItemSelectionModel::NoUpdate);
}

/*!
  \property QListWidget::currentRow
  \brief the row of the current item.
*/

int QListWidget::currentRow() const
{
    return currentIndex().row();
}

void QListWidget::setCurrentRow(int row)
{
    selectionModel()->setCurrentIndex(d->model()->index(row),
                                      d->selectionMode == SingleSelection
                                      ? QItemSelectionModel::ClearAndSelect
                                      : QItemSelectionModel::NoUpdate);
}

/*!
  Returns a pointer to the item at the coordinates \a p.
*/

QListWidgetItem *QListWidget::itemAt(const QPoint &p) const
{
    QModelIndex index = indexAt(p);
    if (index.isValid())
        return d->model()->at(index.row());
    return 0;
}

/*!
  Returns the rectangle on the viewport occupied by the item at \a item.
*/
QRect QListWidget::visualItemRect(const QListWidgetItem *item) const
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QListWidgetItem*>(item));
    Q_ASSERT(index.isValid());
    return visualRect(index);
}

/*!
  Sorts all the items in the list widget according to the specified \a order.
*/
void QListWidget::sortItems(Qt::SortOrder order)
{
    d->model()->sort(0, order);
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
  Returns true if \a item is selected and not hidden; otherwise returns false.
*/
bool QListWidget::isItemSelected(const QListWidgetItem *item) const
{
    QModelIndex index = d->model()->index(const_cast<QListWidgetItem*>(item));
    return selectionModel()->isSelected(index) && !isIndexHidden(index);
}

/*!
  Selects or deselects the given \a item depending on whether \a select is
  true of false.
*/
void QListWidget::setItemSelected(const QListWidgetItem *item, bool select)
{
    QModelIndex index = d->model()->index(const_cast<QListWidgetItem*>(item));
    selectionModel()->select(index, select
                             ? QItemSelectionModel::Select
                             : QItemSelectionModel::Deselect);
}

/*!
  Returns a list of all selected items in the list widget.
*/

QList<QListWidgetItem*> QListWidget::selectedItems() const
{
    QModelIndexList indexes = selectedIndexes();
    QList<QListWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items.append(d->model()->at(indexes.at(i).row()));
    return items;
}

/*!
  Finds items with the text that matches the regular expression \a rx.
*/

QList<QListWidgetItem*> QListWidget::findItems(const QRegExp &rx) const
{
    return d->model()->find(rx);
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
  Scrolls the view if necessary to ensure that the \a item is visible.
*/

void QListWidget::scrollToItem(const QListWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QListWidgetItem*>(item));
    Q_ASSERT(index.isValid());
    QListView::scrollTo(index);
}

/*!
  Removes all items and selections in the view.
*/
void QListWidget::clear()
{
    selectionModel()->clear();
    d->model()->clear();
}

/*!
  \internal
*/
void QListWidget::setModel(QAbstractItemModel *model)
{
    QListView::setModel(model);
}

#include "moc_qlistwidget.cpp"

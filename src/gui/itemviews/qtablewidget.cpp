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

#include "qtablewidget.h"
#include <qheaderview.h>
#include <qitemdelegate.h>
#include <qpainter.h>
#include <qabstractitemmodel.h>
#include <private/qabstractitemmodel_p.h>
#include <private/qtableview_p.h>

// workaround for VC++ 6.0 linker bug (?)
typedef bool(*LessThan)(const QTableWidgetItem *left, const QTableWidgetItem *right);

class QTableModel : public QAbstractTableModel
{
public:
    QTableModel(int rows, int columns, QTableWidget *parent);
    ~QTableModel();

    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool insertColumns(int column, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeColumns(int column, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    void setItem(int row, int column, QTableWidgetItem *item);
    void setItem(const QModelIndex &index, QTableWidgetItem *item);
    QTableWidgetItem *takeItem(int row, int column);
    QTableWidgetItem *item(int row, int column) const;
    QTableWidgetItem *item(const QModelIndex &index) const;
    void removeItem(QTableWidgetItem *item);

    void setHorizontalHeaderItem(int section, QTableWidgetItem *item);
    void setVerticalHeaderItem(int section, QTableWidgetItem *item);
    QTableWidgetItem *horizontalHeaderItem(int section);
    QTableWidgetItem *verticalHeaderItem(int section);

    QModelIndex index(const QTableWidgetItem *item) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const;

#ifdef Q_NO_USING_KEYWORD
    int rowCount(const QModelIndex &parent) const
        { return QAbstractItemModel::rowCount(parnet); }
    int columnCount(const QModelIndex &parent) const
        { return QAbstractItemModel::columnCount(parnet); }
    void sort(int column, const QModelIndex &parent, Qt::SortOrder order)
        { QAbstractItemModel::sort(column, parent, order); }
#else
    using QAbstractItemModel::rowCount;
    using QAbstractItemModel::columnCount;
    using QAbstractItemModel::sort;
#endif
    
    void setRowCount(int rows);
    void setColumnCount(int columns);

    int rowCount() const;
    int columnCount() const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

    bool isSortable() const;
    void sort(int column, const QModelIndex &parent, Qt::SortOrder order);

    static bool itemLessThan(const QTableWidgetItem *left, const QTableWidgetItem *right);
    static bool itemGreaterThan(const QTableWidgetItem *left, const QTableWidgetItem *right);

    bool isValid(const QModelIndex &index) const;
    inline long tableIndex(int row, int column) const
        {  return (row * horizontal.count()) + column; }

    void clear();
    void itemChanged(QTableWidgetItem *item);

private:
    QVector<QTableWidgetItem*> table;
    QVector<QTableWidgetItem*> vertical;
    QVector<QTableWidgetItem*> horizontal;
    mutable QChar strbuf[65];
};

QTableModel::QTableModel(int rows, int columns, QTableWidget *parent)
    : QAbstractTableModel(parent),
      table(rows * columns), vertical(rows), horizontal(columns) {}

QTableModel::~QTableModel()
{
    clear();
}

bool QTableModel::insertRows(int row, const QModelIndex &, int count)
{
    // insert rows
    int rc = vertical.count();
    int cc = horizontal.count();
    vertical.insert(row, count, 0);
    if (rc == 0)
        table.resize(cc * count);
    else
        table.insert(tableIndex(row, 0), cc * count, 0);
    // update persistent model indexes
    for (int i = 0; i < persistentIndexesCount(); ++i) {
        QModelIndex idx = persistentIndexAt(i);
        if (idx.row() >= row)
            setPersistentIndex(i, index(idx.row() + count, idx.column()));
    }
    emit rowsInserted(QModelIndex::Null, row, row + count - 1);
    return true;
}

bool QTableModel::insertColumns(int column, const QModelIndex &, int count)
{
    // insert columns
    int rc = vertical.count();
    int cc = horizontal.count();
    horizontal.insert(column, count, 0);
    if (cc == 0)
        table.resize(rc * count);
    else
        for (int row = 0; row < rc; ++row)
            table.insert(tableIndex(row, column), count, 0);
    // update persistent model indexes
    for (int i = 0; i < persistentIndexesCount(); ++i) {
        QModelIndex idx = persistentIndexAt(i);
        if (idx.column() >= column)
            setPersistentIndex(i, index(idx.row(), idx.column() + count));
    }
    emit columnsInserted(QModelIndex::Null, column, column + count - 1);
    return true;
}

bool QTableModel::removeRows(int row, const QModelIndex &, int count)
{
    if (row >= 0 && row < vertical.count()) {
        emit rowsAboutToBeRemoved(QModelIndex::Null, row, row + count - 1);
        // remove rows
        int i = tableIndex(row, columnCount() - 1);
        table.remove(qMax(i, 0), count * columnCount());
        vertical.remove(row, count);
        // update persistent model indexes
        for (int j = 0; j < persistentIndexesCount(); ++j) {
            QModelIndex idx = persistentIndexAt(j);
            if (idx.row() >= vertical.count())
                setPersistentIndex(j, QModelIndex::Null);
            else if (idx.row() >= row)
                setPersistentIndex(j, index(idx.row() - count, idx.column()));
        }
        return true;
    }
    return false;
}

bool QTableModel::removeColumns(int column, const QModelIndex &, int count)
{
    if (column >= 0 && column < horizontal.count()) {
        emit columnsAboutToBeRemoved(QModelIndex::Null, column, column + count - 1);
        // remove columns
        for (int row = rowCount() - 1; row >= 0; --row)
            table.remove(tableIndex(row, column), count);
        horizontal.remove(column, count);
        // update persistent model indexes
        for (int i = 0; i < persistentIndexesCount(); ++i) {
            QModelIndex idx = persistentIndexAt(i);
            if (idx.column() >= horizontal.count())
                setPersistentIndex(i, QModelIndex::Null);
            else if (idx.column() >= column)
                setPersistentIndex(i, index(idx.row(), idx.column() - count));
        }
        return true;
    }
    return false;
}

void QTableModel::setItem(int row, int column, QTableWidgetItem *item)
{
    item->model = this;
    int i = tableIndex(row, column);
    if (i >= 0 && i < table.count())
        table[i] = item;
}

void QTableModel::setItem(const QModelIndex &index, QTableWidgetItem *item)
{
    if (!isValid(index))
        return;

    long i = tableIndex(index.row(), index.column());
    delete table.at(i);
    table[i] = item;
}

QTableWidgetItem *QTableModel::takeItem(int row, int column)
{
    long i = tableIndex(row, column);
    QTableWidgetItem *itm = table.value(i);
    if (itm) {
        itm->model = 0;
        table[i] = 0;
    }
    return itm;
}

QTableWidgetItem *QTableModel::item(int row, int column) const
{
    return table.value(tableIndex(row, column));
}

QTableWidgetItem *QTableModel::item(const QModelIndex &index) const
{
    if (!isValid(index))
        return 0;
    return table.at(tableIndex(index.row(), index.column()));
}

void QTableModel::removeItem(QTableWidgetItem *item)
{
    int i = table.indexOf(item);
    if (i != -1) {
        table[i] = 0;
        QModelIndex idx = index(item);
        emit dataChanged(idx, idx);
        return;
    }

    i = vertical.indexOf(item);
    if (i != -1) {
        vertical[i] = 0;
        emit headerDataChanged(Qt::Vertical, i, i);
        return;
    }

    i = horizontal.indexOf(item);
    if (i != -1) {
        horizontal[i] = 0;
        emit headerDataChanged(Qt::Horizontal, i, i);
        return;
    }
}

void QTableModel::setHorizontalHeaderItem(int section, QTableWidgetItem *item)
{
    Q_ASSERT(item);
    item->model = this;
    vertical[section] = item;
}


void QTableModel::setVerticalHeaderItem(int section, QTableWidgetItem *item)
{
    Q_ASSERT(item);
    item->model = this;
    horizontal[section] = item;
}

QTableWidgetItem *QTableModel::horizontalHeaderItem(int section)
{
    return horizontal.value(section);
}

QTableWidgetItem *QTableModel::verticalHeaderItem(int section)
{
    return vertical.value(section);
}

QModelIndex QTableModel::index(const QTableWidgetItem *item) const
{
    int i = table.indexOf(const_cast<QTableWidgetItem*>(item));
    int row = i / columnCount();
    int col = i % columnCount();
    return index(row, col);
}

QModelIndex QTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent)) {
        QTableWidgetItem *item = table.at(tableIndex(row, column));
        return createIndex(row, column, item);
    }
    return QModelIndex::Null;
}

void QTableModel::setRowCount(int rows)
{
    int rc = vertical.count();
    if (rc == rows)
        return;
    if (rc < rows)
        insertRows(qMax(rc - 1, 0), QModelIndex::Null, rows - rc);
    else
        removeRows(qMax(rows - 1, 0), QModelIndex::Null, rc - rows);
}

void QTableModel::setColumnCount(int columns)
{
    int cc = horizontal.count();
    if (cc == columns)
        return;
    if (cc < columns)
        insertColumns(qMax(cc - 1, 0), QModelIndex::Null, columns - cc);
    else
        removeColumns(qMax(columns - 1, 0), QModelIndex::Null, cc - columns);
}

int QTableModel::rowCount() const
{
    return vertical.count();
}

int QTableModel::columnCount() const
{
    return horizontal.count();
}

QVariant QTableModel::data(const QModelIndex &index, int role) const
{
    QTableWidgetItem *itm = item(index);
    if (itm)
        return itm->data(role);
    return QVariant();
}

bool QTableModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    QTableWidgetItem *itm = item(index);

    if (itm) {
        itm->setData(role, value);
        return true;
    }

    QTableWidget *view = qt_cast<QTableWidget*>(QObject::parent());
    if (!view)
        return false;

    itm = view->createItem();
    itm->setData(role, value);
    view->setItem(index.row(), index.column(), itm);
    return true;
}

QAbstractItemModel::ItemFlags QTableModel::flags(const QModelIndex &index) const
{
    QTableWidgetItem *itm = item(index);
    if (itm)
        return itm->flags();
    return QAbstractItemModel::ItemIsEditable
        |QAbstractItemModel::ItemIsSelectable
        |QAbstractItemModel::ItemIsCheckable
        |QAbstractItemModel::ItemIsEnabled;
}

bool QTableModel::isSortable() const
{
    return true;
}

void QTableModel::sort(int column, const QModelIndex &parent, Qt::SortOrder order)
{
    Q_UNUSED(parent);
    QVector<QTableWidgetItem*> sorting(rowCount());
    for (int i = 0; i < sorting.count(); ++i)
        sorting[i] = item(i, column);
    LessThan compare = order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan;
    qHeapSort(sorting.begin(), sorting.end(), compare);
    for (int j = 0; j < sorting.count(); ++j)
        table[tableIndex(j, column)] = sorting.at(j);
}

bool QTableModel::itemLessThan(const QTableWidgetItem *left, const QTableWidgetItem *right)
{
    return *left < *right;
}

bool QTableModel::itemGreaterThan(const QTableWidgetItem *left, const QTableWidgetItem *right)
{
    return !(*left < *right);
}

QVariant QTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QTableWidgetItem *itm = 0;
    if (orientation == Qt::Horizontal && section < horizontal.count())
        itm = horizontal.at(section);
    else if (section < vertical.count())
        itm = vertical.at(section);
    if (itm)
        return itm->data(role);
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool QTableModel::setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value)
{
    QTableWidgetItem *itm = 0;
    if (orientation == Qt::Horizontal)
        itm = horizontal.at(section);
    else
        itm = vertical.at(section);

    if (itm) {
        itm->setData(role, value);
        return true;
    }

    return false;
}

bool QTableModel::isValid(const QModelIndex &index) const
{
    return index.isValid() && index.row() < vertical.count() && index.column() < horizontal.count();
}

void QTableModel::clear()
{
    for (int i = 0; i < table.count(); ++i) {
        if (table.at(i)) {
            table.at(i)->model = 0;
            delete table.at(i);
            table[i] = 0;
        }
    }
    for (int j = 0; j < vertical.count(); ++j) {
        if (vertical.at(j)) {
            vertical.at(j)->model = 0;
            delete vertical.at(j);
            vertical[j] = 0;
        }
    }
    for (int k = 0; k < horizontal.count(); ++k) {
        if (horizontal.at(k)) {
            horizontal.at(k)->model = 0;
            delete horizontal.at(k);
            horizontal[k] = 0;
        }
    }
    emit reset();
}

void QTableModel::itemChanged(QTableWidgetItem *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}


/*!
  \class QTableWidgetSelectionRange

  \brief The QTableWidgetSelectionRange provides a class for storing a
  selection range in a QTableWidget.

  \ingroup model-view

  The QTableWidgetSelectionRange class stores the top left and bottom
  right rows and columns of a selection range in a table. The
  selections in the table may consist of several selection ranges.

  \sa QTableWidget
*/

/*!
  Constructs the table selection range.
*/
QTableWidgetSelectionRange::QTableWidgetSelectionRange()
    : top(-1), left(-1), bottom(-1), right(-1)
{
}

/*!
  Constructs the table selection range from the given \a top, \a left, \a bottom and \a right table rows and columns.
*/
QTableWidgetSelectionRange::QTableWidgetSelectionRange(int top, int left, int bottom, int right)
    : top(top), left(left), bottom(bottom), right(right)
{
}

/*!
  Constructs a the table selection range by copying the given \a other table selection range.
*/
QTableWidgetSelectionRange::QTableWidgetSelectionRange(const QTableWidgetSelectionRange &other)
    : top(other.top), left(other.left), bottom(other.bottom), right(other.right)
{
}

/*!
  Destroys the table selection range.
*/
QTableWidgetSelectionRange::~QTableWidgetSelectionRange()
{
}

/*!
  \fn inline int QTableWidgetSelectionRange::topRow() const

  Returns the top row of the range.
*/

/*!
  \fn inline int QTableWidgetSelectionRange::bottomRow() const

  Returns the bottom row of the range.
*/

/*!
  \fn inline int QTableWidgetSelectionRange::leftColumn() const

  Returns the left column of the range.
*/

/*!
  \fn inline int QTableWidgetSelectionRange::rightColumn() const

  Returns the right column of the range.
*/


/*!
    \class QTableWidgetItem
    \brief The QTableWidgetItem class provides an item for use with the
    QTableWidget class.

    \ingroup model-view

    Table items are used to hold pieces of information for table widgets.
    Items usually contain text, icons, or checkboxes

    The QTableWidgetItem class is a convenience class that replaces the
    \c QTableItem class in Qt 3. It provides an item for use with
    the QTableWidget class.

    Items are usually constructed with a table widget as their parent then
    inserted at a particular position specified by row and column numbers:

    \quotefile snippets/qtablewidget-using/mainwindow.cpp
    \skipto QTableWidgetItem *newItem
    \printuntil tableWidget->setItem(

    Each item can have its own background color which is set with
    the setBackgroundColor() function. The current background color can be
    found with backgroundColor().
    The text label for each item can be rendered with its own font and text
    color. These are specified with the setFont() and setTextColor() functions,
    and read with font() and textColor().

    Items can be made checkable by setting the appropriate flag value with the
    setFlags() function. The current state of the item's flags can be read
    with flags().

    \sa QTableWidget
*/

/*!
    \fn int QTableWidgetItem::checked() const

    Returns the checked state of the list item (see \l{QCheckBox::ToggleState}).

    Only checkable items can be checked. By default, items are not
    checkable.

    \sa flags()
*/

/*!
    \fn void QTableWidgetItem::setChecked(const bool checked)

    Checks the item if \a checked is true; otherwise it will be shown as
    unchecked.

    \sa checked()
*/

/*!
    \fn QTableWidget *QTableWidgetItem::tableWidget() const

    Returns the table widget that contains the item.
*/

/*!
    \fn QAbstractItemModel::ItemFlags QTableWidgetItem::flags() const

    Returns the flags used to describe the item. These determine whether
    the item can be checked, edited, and selected.

    \sa setFlags()
*/

/*!
    \fn void QTableWidgetItem::setFlags(QAbstractItemModel::ItemFlags flags)

    Sets the flags for the item to the given \a flags. These determine whether
    the item can be selected or modified.

    \sa flags()
*/

/*!
    \fn QString QTableWidgetItem::text() const

    Returns the item's text.

    \sa setText()
*/

/*!
    \fn void QTableWidgetItem::setText(const QString &text)

    Sets the item's text to the \a text specified.

    \sa text() setFont() setTextColor()
*/

/*!
    \fn QIcon QTableWidgetItem::icon() const

    Returns the item's icon.

    \sa setIcon()
*/

/*!
    \fn void QTableWidgetItem::setIcon(const QIcon &icon)

    Sets the item's icon to the \a icon specified.

    \sa icon() setText()
*/

/*!
    \fn QString QTableWidgetItem::statusTip() const

    Returns the item's status tip.

    \sa setStatusTip()
*/

/*!
    \fn void QTableWidgetItem::setStatusTip(const QString &statusTip)

    Sets the item's status tip to the string specified by \a statusTip.

    \sa statusTip() setToolTip() setWhatsThis()
*/

/*!
    \fn QString QTableWidgetItem::toolTip() const

    Returns the item's tooltip.

    \sa setToolTip()
*/

/*!
    \fn void QTableWidgetItem::setToolTip(const QString &toolTip)

    Sets the item's tooltip to the string specified by \a toolTip.

    \sa toolTip() setStatusTip() setWhatsThis()
*/

/*!
    \fn QString QTableWidgetItem::whatsThis() const

    Returns the item's "What's This?" help.

    \sa setWhatsThis()
*/

/*!
    \fn void QTableWidgetItem::setWhatsThis(const QString &whatsThis)

    Sets the item's "What's This?" help to the string specified by \a whatsThis.

    \sa whatsThis() setStatusTip() setToolTip()
*/

/*!
    \fn QFont QTableWidgetItem::font() const

    Returns the font used to render the item's text.

    \sa setFont()
*/

/*!
    \fn void QTableWidgetItem::setFont(const QFont &font)

    Sets the font used to display the item's text to the given \a font.

    \sa font() setText() setTextColor()
*/

/*!
    \fn QColor QTableWidgetItem::backgroundColor() const

    Returns the color used to render the item's background.

    \sa textColor() setBackgroundColor()
*/

/*!
    \fn void QTableWidgetItem::setBackgroundColor(const QColor &color)

    Sets the item's background color to the specified \a color.

    \sa backgroundColor() setTextColor()
*/

/*!
    \fn QColor QTableWidgetItem::textColor() const

    Returns the color used to render the item's text.

    \sa backgroundColor() setTextColor()
*/

/*!
    \fn void QTableWidgetItem::setTextColor(const QColor &color)

    Sets the color used to display the item's text to the given \a color.

    \sa textColor() setFont() setText()
*/

/*!
  \fn int QTableWidgetItem::textAlignment() const

  Returns the text alignment for the item's text (see \l{Qt::AlignmentFlag}).
*/

/*!
  \fn void QTableWidgetItem::setTextAlignment(int alignment)

  Sets the text alignment for the item's text to the \a alignment
  specified (see \l{Qt::AlignmentFlag}).
*/

/*!
    Constructs a table item that does not belong to any table.
*/
QTableWidgetItem::QTableWidgetItem()
    : view(0), model(0),
      itemFlags(QAbstractItemModel::ItemIsEditable
                |QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsCheckable
                |QAbstractItemModel::ItemIsEnabled)
{
}

/*!
    Constructs a table item with the given \a text.
*/
QTableWidgetItem::QTableWidgetItem(const QString &text)
    : view(0), model(0),
      itemFlags(QAbstractItemModel::ItemIsEditable
                |QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsCheckable
                |QAbstractItemModel::ItemIsEnabled)
{
    setData(QAbstractItemModel::DisplayRole, text);
}

/*!
    Destroys the table item.
*/
QTableWidgetItem::~QTableWidgetItem()
{
    if (model)
        model->removeItem(this);
}

/*!
    Sets the item's data for the given \a role to the specified \a value.
*/
void QTableWidgetItem::setData(int role, const QVariant &value)
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
    Returns the item's data for the given \a role.
*/
QVariant QTableWidgetItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
}

/*!
    Returns true if the item is less than the \a other item; otherwise returns
    false.
*/
bool QTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
    return text() < other.text();
}

/*!
    Clears all the item's information.
*/
void QTableWidgetItem::clear()
{
    values.clear();
    if (model)
        model->itemChanged(this);
}

/*!
    \class QTableWidget qtablewidget.h
    \brief The QTableWidget class provides an item-based table view with a default model.

    \ingroup model-view
    \mainclass

    Table widgets provide standard table display facilities for applications.
    The items in a QTableWidget are provided by QTableWidgetItem.

    If you want a table that uses your own data model you should
    use QTableView rather than this class.

    Table widgets can be constructed with the required numbers of rows and
    columns:

    \quotefile snippets/qtablewidget-using/mainwindow.cpp
    \skipto tableWidget = new
    \printuntil tableWidget = new

    Alternatively, tables can be constructed without a given size and resized
    later:

    \quotefile snippets/qtablewidget-resizing/mainwindow.cpp
    \skipto tableWidget = new
    \printuntil tableWidget = new
    \skipto tableWidget->setRowCount(
    \printuntil tableWidget->setColumnCount(

    Items are created ouside the table (with no parent widget) and inserted
    into the table with setItem():

    \skipto QTableWidgetItem *newItem
    \printuntil tableWidget->setItem(

    Tables can be given both horizontal and vertical headers. The simplest way
    to create the headers is to supply a list of strings to the
    setHorizontalHeaderLabels() and setVerticalHeaderLabels() functions. These
    will provide simple textual headers for the table's columns and rows.
    More sophisticated headers can be created from existing table items
    that are usually constructed outside the table. For example, we can
    construct a table item with an icon and aligned text, and use it as the
    header for a particular column:

    \quotefile snippets/qtablewidget-using/mainwindow.cpp
    \skipto QTableWidgetItem *cubesHeaderItem
    \printuntil cubesHeaderItem->setTextAlignment

    The number of rows in the table can be found with rowCount(), and the
    number of columns with columnCount(). The table can be cleared with the
    clear() function.

    \sa QTableWidgetItem \link model-view-programming.html Model/View Programming\endlink
*/

/*!
    \property QTableWidget::rowCount
    \brief the number of rows in the table
*/

/*!
    \property QTableWidget::columnCount
    \brief the number of columns in the table
*/

/*!
    \property QTableWidget::sortingEnabled
    \brief whether the items in the table can be sorted
    by clicking on the horizontal header.
*/

#define d d_func()
#define q q_func()

class QTableWidgetPrivate : public QTableViewPrivate
{
    Q_DECLARE_PUBLIC(QTableWidget)
public:
    QTableWidgetPrivate() : QTableViewPrivate(), sortingEnabled(false) {}
    inline QTableModel *model() const { return ::qt_cast<QTableModel*>(q_func()->model()); }
    void emitPressed(const QModelIndex &index, Qt::MouseButton button,
                     Qt::KeyboardModifiers modifiers);
    void emitClicked(const QModelIndex &index, Qt::MouseButton button,
                     Qt::KeyboardModifiers modifiers);
    void emitDoubleClicked(const QModelIndex &index, Qt::MouseButton button,
                           Qt::KeyboardModifiers modifiers);
    void emitKeyPressed(const QModelIndex &index, Qt::Key key,
                        Qt::KeyboardModifiers modifiers);
    void emitReturnPressed(const QModelIndex &index);
    void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current);
    void emitItemEntered(const QModelIndex &index, Qt::MouseButton button,
                         Qt::KeyboardModifiers modifiers);
    void emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index);
    void emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    bool sortingEnabled;
};

void QTableWidgetPrivate::emitPressed(const QModelIndex &index, Qt::MouseButton button,
                                      Qt::KeyboardModifiers modifiers)
{
    emit q->pressed(model()->item(index), button, modifiers);
}

void QTableWidgetPrivate::emitClicked(const QModelIndex &index, Qt::MouseButton button,
                                      Qt::KeyboardModifiers modifiers)
{
    emit q->clicked(model()->item(index), button, modifiers);
}

void QTableWidgetPrivate::emitDoubleClicked(const QModelIndex &index, Qt::MouseButton button,
                                            Qt::KeyboardModifiers modifiers)
{
    emit q->doubleClicked(model()->item(index), button, modifiers);
}

void QTableWidgetPrivate::emitKeyPressed(const QModelIndex &index, Qt::Key key,
                                         Qt::KeyboardModifiers modifiers)
{
    emit q->keyPressed(model()->item(index), key, modifiers);
}

void QTableWidgetPrivate::emitReturnPressed(const QModelIndex &index)
{
    emit q->returnPressed(model()->item(index));
}

void QTableWidgetPrivate::emitCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit q->currentChanged(model()->item(current), model()->item(previous));
}

void QTableWidgetPrivate::emitItemEntered(const QModelIndex &index, Qt::MouseButton button,
                                          Qt::KeyboardModifiers modifiers)
{
    emit q->itemEntered(model()->item(index), button, modifiers);
}

void QTableWidgetPrivate::emitAboutToShowContextMenu(QMenu *menu, const QModelIndex &index)
{
    emit q->aboutToShowContextMenu(menu, model()->item(index));
}

void QTableWidgetPrivate::emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft == bottomRight) // this should always be true, unless we sort
        emit q->itemChanged(model()->item(topLeft));
}

/*!
    \fn void QTableWidget::pressed(QTableWidgetItem *item, QMouseEvent *event)

    This signal is emitted when a item has been pressed (mouse click
    and release). The \a item may be 0 if the mouse was not pressed on
    an item..
*/

/*!
    \fn void QTableWidget::clicked(QTableWidgetItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)

    This signal is emitted when the specified \a item is clicked.
    The state of the mouse buttons is described by \a button; the
    \a modifiers reflect the state of the keyboard's modifier keys.

    The item may be 0 if the mouse was not clicked on an item.
*/

/*!
    \fn void QTableWidget::doubleClicked(QTableWidgetItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

    This signal is emitted when the specified \a item is double clicked.
    The state of the mouse buttons is described by \a button; the
    \a modifiers reflect the state of the keyboard's modifier keys.

    The item may be 0 if the mouse was not clicked on an item.
*/

/*!
    \fn void QTableWidget::keyPressed(QTableWidgetItem *item, QKeyEvent *event)

    This signal is emitted if keyTracking is turned on and a key was
    pressed. The \a item is the current item as the key was pressed.
*/

/*!
    \fn void QTableWidget::returnPressed(QTableWidgetItem *item)

    This signal is emitted when return has been pressed on an \a item.
*/

/*!
    \fn void QTableWidget::currentChanged(QTableWidgetItem *current, QTableWidgetItem *previous)

    This signal is emitted whenever the current item changes. The \a
    previous item is the item that previously had the focus, \a
    current is the new current item.
*/

/*!
    \fn void QTableWidget::selectionChanged()

    This signal is emitted whenever the selection changes.

    \sa selectedItems() isSelected()
*/

/*!
    \fn void QTableWidget::itemEntered(QTableWidgetItem *item, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)

    This signal is emitted when the mouse cursor enters an item. The
    \a item is the item entered. The state of the mouse buttons is specified
    by \a button; the \a modifiers reflect the state of the keyboard's
    modifier keys.

    This signal is only emitted when mouseTracking is turned on, or when a
    mouse button is pressed while moving into an item.
*/

/*!
    \fn void QTableWidget::aboutToShowContextMenu(QMenu *menu, QTableWidgetItem *item)

    This signal is emitted when the widget is about to show a context
    menu. The \a menu is the menu about to be shown, and the \a item is the
    clicked item the context menu was called for.

    \sa QMenu::addAction()
*/

/*!
    \fn void QTableWidget::itemChanged(QTableWidgetItem *item)

    This signal is emitted whenever the data of \a item has changed.
*/

/*!
    Creates a new table view with the given \a parent.
*/
QTableWidget::QTableWidget(QWidget *parent)
    : QTableView(*new QTableWidgetPrivate, parent)
{
    setModel(new QTableModel(0, 0, this));
    setup();
}

/*!
    Creates a new table view with the given \a rows and \a columns, and with the given \a parent.
*/
QTableWidget::QTableWidget(int rows, int columns, QWidget *parent)
    : QTableView(*new QTableWidgetPrivate, parent)
{
    setModel(new QTableModel(rows, columns, this));
    setup();
}

/*!
    Destroys this QTableWidget.
*/
QTableWidget::~QTableWidget()
{
}

/*!
    Sets the number of rows in this table's model to \a rows. If
    this is less than rowCount(), the data in the unwanted rows
    is discarded.

    \sa setColumnCount()
*/
void QTableWidget::setRowCount(int rows)
{
    d->model()->setRowCount(rows);
}

/*!
  Returns the number of rows.
*/

int QTableWidget::rowCount() const
{
    return d->model()->rowCount();
}

/*!
    Sets the number of columns in this table's model to \a columns. If
    this is less than columnCount(), the data in the unwanted columns
    is discarded.

    \sa setRowCount()
*/
void QTableWidget::setColumnCount(int columns)
{
    d->model()->setColumnCount(columns);
}

/*!
  Returns the number of columns.
*/

int QTableWidget::columnCount() const
{
    return d->model()->columnCount();
}

/*!
  Returns the row for the \a item.
*/
int QTableWidget::row(const QTableWidgetItem *item) const
{
    Q_ASSERT(item);
    return d->model()->index(item).row();
}

/*!
  Returns the column for the \a item.
*/
int QTableWidget::column(const QTableWidgetItem *item) const
{
    Q_ASSERT(item);
    return d->model()->index(item).column();
}


/*!
    Returns the item for the given \a row and \a column.

    \sa setItem()
*/
QTableWidgetItem *QTableWidget::item(int row, int column) const
{
    return d->model()->item(row, column);
}

/*!
    Sets the item for the given \a row and \a column to \a item.

    \sa item()
*/
void QTableWidget::setItem(int row, int column, QTableWidgetItem *item)
{
    Q_ASSERT(item);
    item->view = this;
    d->model()->setItem(row, column, item);
}

/*!
    Removes the item at \a row and \a column from the table without deleting it.
*/
QTableWidgetItem *QTableWidget::takeItem(int row, int column)
{
    QTableWidgetItem *item = d->model()->takeItem(row, column);
    item->view = 0;
    return item;
}

/*!
  Removes item \a item from the table.
*/
void QTableWidget::removeItem(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    d->model()->removeItem(item);
}

/*!
  Returns the vertical header item for row \a row.
*/
QTableWidgetItem *QTableWidget::verticalHeaderItem(int row) const
{
    return d->model()->verticalHeaderItem(row);
}

/*!
  Sets the vertical header item for row \a row to \a item.
*/
void QTableWidget::setVerticalHeaderItem(int row, QTableWidgetItem *item)
{
    item->view = this;
    d->model()->setHorizontalHeaderItem(row, item);
}

/*!
  Returns the horizontal header item for column \a column.
*/
QTableWidgetItem *QTableWidget::horizontalHeaderItem(int column) const
{
    return d->model()->horizontalHeaderItem(column);
}

/*!
  Sets the horizontal header item for column \a column to \a item.
*/
void QTableWidget::setHorizontalHeaderItem(int column, QTableWidgetItem *item)
{
    item->view = this;
    d->model()->setVerticalHeaderItem(column, item);
}

/*!
  Sets the vertical header labels using \a labels.
*/
void QTableWidget::setVerticalHeaderLabels(const QStringList &labels)
{
    QTableModel *model = d->model();
    QTableWidgetItem *item = 0;
    for (int i = 0; i < model->rowCount() && i < labels.count(); ++i) {
        item = model->verticalHeaderItem(i);
        if (!item) {
            item = createItem();
            setVerticalHeaderItem(i, item);
        }
        item->setText(labels.at(i));
    }
}

/*!
  Sets the horizontal header labels using \a labels.
*/
void QTableWidget::setHorizontalHeaderLabels(const QStringList &labels)
{
    QTableModel *model = d->model();
    QTableWidgetItem *item = 0;
    for (int i = 0; i < model->columnCount() && i < labels.count(); ++i) {
        item = model->horizontalHeaderItem(i);
        if (!item) {
            item = createItem();
            setHorizontalHeaderItem(i, item);
        }
        item->setText(labels.at(i));
    }
}

/*!
  Returns the row of the current item.
*/
int QTableWidget::currentRow() const
{
    return currentIndex().row();
}

/*!
  Returns the column of the current item.
*/
int QTableWidget::currentColumn() const
{
    return currentIndex().column();
}

/*!
  Returns the current item.
*/
QTableWidgetItem *QTableWidget::currentItem() const
{
    return d->model()->item(currentIndex());
}

/*!
  Sets the current item to \a item.
*/
void QTableWidget::setCurrentItem(QTableWidgetItem *item)
{
    setCurrentIndex(d->model()->index(item));
}

/*!
  Sorts all the rows in the table widget based on \a column and \a order.
*/
void QTableWidget::sortItems(int column, Qt::SortOrder order)
{
    d->model()->sort(column, QModelIndex::Null, order);
    horizontalHeader()->setSortIndicator(column, order);
}

/*!
  If \a enable is true, the items in the widget will be sorted if the user
  clicks on a horizontal header section; otherwise sorting is disabled.
*/

void QTableWidget::setSortingEnabled(bool enable)
{
    d->sortingEnabled = enable;
    if (!enable && horizontalHeader()->isSortIndicatorShown())
        horizontalHeader()->setSortIndicatorShown(false);
}

/*!
  Returns if sorting is enabled; otherwise returns false.
  Sorting is enabled when the user clicks on a horizontal header section.
*/

bool QTableWidget::isSortingEnabled() const
{
    return d->sortingEnabled;
}

/*!
  Opens an editor for the give \a item. The editor remains open after editing.

  \sa closePersistentEditor()
*/
void QTableWidget::openPersistentEditor(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item);
    QAbstractItemView::openPersistentEditor(index);
}

/*!
  Closes the persistent editor for \a item.

  \sa openPersistentEditor()
*/
void QTableWidget::closePersistentEditor(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item);
    QAbstractItemView::closePersistentEditor(index);
}

/*!
  Returns true if the \a item is selected, otherwise returns false.
*/

bool QTableWidget::isSelected(const QTableWidgetItem *item) const
{
    QModelIndex index = d->model()->index(item);
    return selectionModel()->isSelected(index) && !isIndexHidden(index);
}

/*!
  Selects or deselects \a item depending on \a select.
*/
void QTableWidget::setSelected(const QTableWidgetItem *item, bool select)
{
    QModelIndex index = d->model()->index(item);
    selectionModel()->select(index, select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Selects or deselects the \a range depending on \a select.
*/
void QTableWidget::setSelected(const QTableWidgetSelectionRange &range, bool select)
{
    if (!model()->hasIndex(range.topRow(), range.leftColumn(), root()) ||
        !model()->hasIndex(range.bottomRow(), range.rightColumn(), root()))
        return;

    QModelIndex topLeft = model()->index(range.topRow(), range.leftColumn(), root());
    QModelIndex bottomRight = model()->index(range.bottomRow(), range.rightColumn(), root());

    selectionModel()->select(QItemSelection(topLeft, bottomRight),
                             select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Returns a list of all selected ranges.

  \sa QTableWidgetSelectionRange
*/

QList<QTableWidgetSelectionRange> QTableWidget::selectedRanges() const
{
    const QList<QItemSelectionRange> ranges = selectionModel()->selection();
    QList<QTableWidgetSelectionRange> result;
    for (int i = 0; i < ranges.count(); ++i)
        result.append(QTableWidgetSelectionRange(ranges.at(i).top(),
                                                 ranges.at(i).left(),
                                                 ranges.at(i).bottom(),
                                                 ranges.at(i).right()));
    return result;
}

/*!
  Returns a list of all selected items. If a cell is empty and \a fillEmptyCells is true,
  the item is created and set in that cell.
*/

QList<QTableWidgetItem*> QTableWidget::selectedItems(bool fillEmptyCells)
{
    QModelIndexList indexes = selectedIndexes();
    QList<QTableWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i) {
        QModelIndex index = indexes.at(i);
        QTableWidgetItem *item = d->model()->item(index);
        if (!item && index.isValid() && fillEmptyCells)
            setItem(index.row(), index.column(), item = createItem());
        if (item)
            items.append(item);
    }
    return items;
}

/*!
  Finds items that matches the \a text, using the criteria given in the
  \a flags (see QAbstractItemModel::MatchFlags).
*/

QList<QTableWidgetItem*> QTableWidget::findItems(const QString &text,
                                                 QAbstractItemModel::MatchFlags flags) const
{
    QModelIndex topLeft = d->model()->index(0, 0);
    int role = QAbstractItemModel::DisplayRole;
    int hits = d->model()->rowCount() * d->model()->columnCount();
    QModelIndexList indexes = d->model()->match(topLeft, role, text, hits, flags);
    QList<QTableWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items << d->model()->item(indexes.at(i));
    return items;
}

/*!
  Returns the visual row of the given \a item.
*/

int QTableWidget::visualRow(const QTableWidgetItem *item) const
{
    return verticalHeader()->visualIndex(row(item));
}

/*!
  Returns the visual column of the given \a item.
*/

int QTableWidget::visualColumn(const QTableWidgetItem *item) const
{
    return horizontalHeader()->visualIndex(column(item));
}

/*!
  Returns a pointer to the item at the \a visualRow
  and \a visualColumn in the view.
*/

QTableWidgetItem *QTableWidget::visualItem(int visualRow, int visualColumn) const
{
    int row = verticalHeader()->logicalIndex(visualRow);
    int column = horizontalHeader()->logicalIndex(visualColumn);
    return item(row, column);
}

/*!
  Returns true if the \a item is in the viewport, otherwise returns false.
*/

bool QTableWidget::isItemVisible(const QTableWidgetItem *item) const
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTableWidgetItem*>(item));
    QRect rect = itemViewportRect(index);
    if (rect.isValid())
        return d->viewport->rect().contains(rect);
    return false;
}

/*!
  Scrolls the view if necessary to ensure that the \a item is visible.
*/

void QTableWidget::ensureVisible(const QTableWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(const_cast<QTableWidgetItem*>(item));
    QTableView::ensureVisible(index);
}

/*!
  Inserts an empty row into the table at \a row.
*/
void QTableWidget::insertRow(int row)
{
    d->model()->insertRows(row);
}

/*!
  Inserts an empty column into the table at \a column.
*/
void QTableWidget::insertColumn(int column)
{
    d->model()->insertColumns(column);
}

/*!
  Removes the row \a row and all its items from the table.
*/
void QTableWidget::removeRow(int row)
{
    d->model()->removeRows(row);
}

/*!
  Removes the column \a column and all its items from the table.
*/
void QTableWidget::removeColumn(int column)
{
    d->model()->removeColumns(column);
}

/*!
  Removes all items and selections in the view.
*/

void QTableWidget::clear()
{
    selectionModel()->clear();
    d->model()->clear();
}

/*!
  Returns a new QTableWidgetItem.
  This is called whenever the table widget creates a new item internally.
*/

QTableWidgetItem *QTableWidget::createItem() const
{
    return new QTableWidgetItem();
}

/*!
  \internal
*/
void QTableWidget::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
}

/*!
  \internal
*/
void QTableWidget::setup()
{
    connect(this, SIGNAL(pressed(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
            SLOT(emitPressed(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)));
    connect(this, SIGNAL(clicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
            SLOT(emitClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)));
    connect(this, SIGNAL(doubleClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
            SLOT(emitDoubleClicked(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)));
    connect(this, SIGNAL(keyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)),
            SLOT(emitKeyPressed(QModelIndex,Qt::Key,Qt::KeyboardModifiers)));
    connect(this, SIGNAL(returnPressed(QModelIndex)),
            SLOT(emitReturnPressed(QModelIndex)));
    connect(this, SIGNAL(itemEntered(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)),
            SLOT(emitItemEntered(QModelIndex,Qt::MouseButton,Qt::KeyboardModifiers)));
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

#include "moc_qtablewidget.cpp"

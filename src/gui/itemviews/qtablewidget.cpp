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

    void setRowCount(int rows);
    void setColumnCount(int columns);

    int rowCount() const;
    int columnCount() const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

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
    vertical.insert(row, count, 0);
    int i = tableIndex(row, qMax(horizontal.count() - 1, 0));
    table.insert(i, count * horizontal.count(), 0);
    emit rowsInserted(QModelIndex::Null, row, row + count - 1);
    return true;
}

bool QTableModel::insertColumns(int column, const QModelIndex &, int count)
{
    int cc = horizontal.count();
    horizontal.insert(column, count, 0);
    if (cc == 0)
        table.resize(vertical.count() * count);
    else
        for (int row = 0; row < vertical.count(); ++row)
            table.insert(tableIndex(row, column), count, 0);
    emit columnsInserted(QModelIndex::Null, column, column + count - 1);
    return true;
}

bool QTableModel::removeRows(int row, const QModelIndex &, int count)
{
    emit rowsRemoved(QModelIndex::Null, row, row + count - 1);
    int i = tableIndex(row, columnCount() - 1);
    table.remove(qMax(i, 0), count * columnCount());
    vertical.remove(row);
    return true;
}

bool QTableModel::removeColumns(int column, const QModelIndex &, int count)
{
    emit columnsRemoved(QModelIndex::Null, column, column + count - 1);
    for (int row = 0; row < rowCount(); ++row)
        table.remove(tableIndex(row, column));
    horizontal.remove(column);
    return true;
}

void QTableModel::setItem(int row, int column, QTableWidgetItem *item)
{
    item->model = this;
    table[tableIndex(row, column)] = item;
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
    QTableWidgetItem *itm = table.at(i);
    itm->model = 0;
    table[i] = 0;
    return itm;
}

QTableWidgetItem *QTableModel::item(int row, int column) const
{
    return table.at(tableIndex(row, column));
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
        return;
    }

    i = vertical.indexOf(item);
    if (i != -1) {
        vertical[i] = 0;
        return;
    }

    i = horizontal.indexOf(item);
    if (i != -1) {
        horizontal[i] = 0;
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
    return horizontal.at(section);
}

QTableWidgetItem *QTableModel::verticalHeaderItem(int section)
{
    return vertical.at(section);
}

QModelIndex QTableModel::index(const QTableWidgetItem *item) const
{
    int i = table.indexOf(const_cast<QTableWidgetItem*>(item));
    int row = i / columnCount();
    int col = i % columnCount();
    return index(row, col);
}

QModelIndex QTableModel::index(int row, int column, const QModelIndex &) const
{
    if (row >= 0 && row < vertical.count() && column >= 0 && column < horizontal.count()) {
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
    if (!itm) {
        itm = new QTableWidgetItem();
        setItem(index, itm);
    }
    itm->setData(role, value);
    emit dataChanged(index, index);
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

QVariant QTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QTableWidgetItem *itm = 0;
    if (orientation == Qt::Horizontal)
        itm = horizontal.at(section);
    else
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
        }
    }
    for (int j = 0; j < vertical.count(); ++j) {
        if (vertical.at(j)) {
            vertical.at(j)->model = 0;
            delete vertical.at(j);
        }
    }
    for (int k = 0; k < horizontal.count(); ++k) {
        if (horizontal.at(k)) {
            horizontal.at(k)->model = 0;
            delete horizontal.at(k);
        }
    }
    emit reset();
}

void QTableModel::itemChanged(QTableWidgetItem *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}

// item

QTableWidgetItem::QTableWidgetItem()
    : itemFlags(QAbstractItemModel::ItemIsEditable
                |QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsCheckable
                |QAbstractItemModel::ItemIsEnabled),
      model(0)
{
}

QTableWidgetItem::QTableWidgetItem(const QString &text)
    : itemFlags(QAbstractItemModel::ItemIsEditable
                |QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsCheckable
                |QAbstractItemModel::ItemIsEnabled),
      model(0)
{
    setData(QAbstractItemModel::DisplayRole, text);
}

QTableWidgetItem::~QTableWidgetItem()
{
    if (model)
        model->removeItem(this);
}


bool QTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
    return text() < other.text();
}

void QTableWidgetItem::setData(int role, const QVariant &value)
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

QVariant QTableWidgetItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
}

/*!
    \class QTableWidget qtablewidget.h
    \brief The QTableWidget class provides a table view that uses the
    predefined QTableModel model.

    \ingroup model-view
    \mainclass

    If you want a table that uses your own data model you should
    subclass QTableView rather than this class.

    Items are set with setItem(), or with setText() or setIcon();
    these last two are convenience functions that create a QTableItem
    for you. The label and icon for a given row is set with
    setRowText() and setRowIconSet(), and for a given column with
    setColumnText() and setColumnIconSet(). The number of rows is set
    with setRowCount(), and the number of columns with
    setColumnCount().

    \sa \link model-view-programming.html Model/View Programming\endlink
*/

#define d d_func()
#define q q_func()

class QTableWidgetPrivate : public QTableViewPrivate
{
    Q_DECLARE_PUBLIC(QTableWidget)
public:
    QTableWidgetPrivate() : QTableViewPrivate() {}
    inline QTableModel *model() const { return ::qt_cast<QTableModel*>(q_func()->model()); }
    void emitPressed(const QModelIndex &index, int button);
    void emitClicked(const QModelIndex &index, int button);
    void emitDoubleClicked(const QModelIndex &index, int button);
    void emitKeyPressed(const QModelIndex &index, Qt::Key key, Qt::ButtonState state);
    void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current);
};

void QTableWidgetPrivate::emitPressed(const QModelIndex &index, int button)
{
    emit q->pressed(model()->item(index), button);
}

void QTableWidgetPrivate::emitClicked(const QModelIndex &index, int button)
{
    emit q->clicked(model()->item(index), button);
}

void QTableWidgetPrivate::emitDoubleClicked(const QModelIndex &index, int button)
{
    emit q->doubleClicked(model()->item(index), button);
}

void QTableWidgetPrivate::emitKeyPressed(const QModelIndex &index, Qt::Key key,
                                         Qt::ButtonState state)
{
    emit q->keyPressed(model()->item(index), key, state);
}

void QTableWidgetPrivate::emitCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit q->currentChanged(model()->item(current), model()->item(previous));
}


/*!
    \fn void QTableWidget::clicked(QTableWidgetItem *item, int button)

    This signal is emitted when a mouse button is clicked. The \a item
    may be 0 if the mouse was not clicked on an item.  The button
    clicked is specified by \a button (see \l{Qt::ButtonState}).
*/

/*!
    \fn void QTableWidget::doubleClicked(QTableWidgetItem *item, int button);

    This signal is emitted when a mouse button is double clicked. The
    \a item may be 0 if the mouse was not clicked on an item.  The
    button clicked is specified by \a button (see
    \l{Qt::ButtonState}).
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
  ###
*/
int QTableWidget::row(const QTableWidgetItem *item) const
{
    Q_ASSERT(item);
    return d->model()->index(item).row();
}

/*!
  ###
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
    d->model()->setItem(row, column, item);
}

QTableWidgetItem *QTableWidget::takeItem(int row, int column)
{
    return d->model()->takeItem(row, column);
}

QTableWidgetItem *QTableWidget::verticalHeaderItem(int row) const
{
    return d->model()->verticalHeaderItem(row);
}

void QTableWidget::setVerticalHeaderItem(int row, QTableWidgetItem *item)
{
    d->model()->setHorizontalHeaderItem(row, item);
}

QTableWidgetItem *QTableWidget::horizontalHeaderItem(int column) const
{
    return d->model()->horizontalHeaderItem(column);
}

void QTableWidget::setHorizontalHeaderItem(int column, QTableWidgetItem *item)
{
    d->model()->setVerticalHeaderItem(column, item);
}

void QTableWidget::setVerticalHeaderLabels(const QStringList &labels)
{
    QTableModel *model = d->model();
    QTableWidgetItem *item = 0;
    for (int i = 0; i < model->rowCount() && i < labels.count(); ++i) {
        item = model->verticalHeaderItem(i);
        if (!item) {
            item = new QTableWidgetItem();
            setVerticalHeaderItem(i, item);
        }
        item->setText(labels.at(i));
    }
}

void QTableWidget::setHorizontalHeaderLabels(const QStringList &labels)
{
    QTableModel *model = d->model();
    QTableWidgetItem *item = 0;
    for (int i = 0; i < model->columnCount() && i < labels.count(); ++i) {
        item = model->horizontalHeaderItem(i);
        if (!item) {
            item = new QTableWidgetItem();
            setHorizontalHeaderItem(i, item);
        }
        item->setText(labels.at(i));
    }
}

QTableWidgetItem *QTableWidget::currentItem() const
{
    return d->model()->item(currentIndex());
}

void QTableWidget::setCurrentItem(QTableWidgetItem *item)
{
    setCurrentIndex(d->model()->index(item));
}

void QTableWidget::openPersistentEditor(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item);
    QAbstractItemView::openPersistentEditor(index);
}

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
    return selectionModel()->isSelected(index);
}

void QTableWidget::setSelected(const QTableWidgetItem *item, bool select)
{
    QModelIndex index = d->model()->index(item);
    selectionModel()->select(index, select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Returns a list of all selected items.
*/

QList<QTableWidgetItem*> QTableWidget::selectedItems() const
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    QList<QTableWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i)
        items.append(d->model()->item(indexes.at(i)));
    return items;
}

/*!
  ###
*/
void QTableWidget::insertRow(int row)
{
    d->model()->insertRows(row);
}

/*!
  ###
*/
void QTableWidget::insertColumn(int column)
{
    d->model()->insertColumns(column);
}

/*!
  ###
*/
void QTableWidget::removeRow(int row)
{
    d->model()->removeRows(row);
}

/*!
  ###
*/
void QTableWidget::removeColumn(int column)
{
    d->model()->removeColumns(column);
}

/*!
  Removes all items in the view.
*/

void QTableWidget::clear()
{
    d->model()->clear();
}

void QTableWidget::removeItem(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    d->model()->removeItem(item);
}

void QTableWidget::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
}

void QTableWidget::setup()
{
    connect(this, SIGNAL(pressed(const QModelIndex&, int)),
            SLOT(emitPressed(const QModelIndex&, int)));
    connect(this, SIGNAL(clicked(const QModelIndex&, int)),
            SLOT(emitClicked(const QModelIndex&, int)));
    connect(this, SIGNAL(doubleClicked(const QModelIndex&, int)),
            SLOT(emitDoubleClicked(const QModelIndex&, int)));
    connect(this, SIGNAL(keyPressed(const QModelIndex&, Qt::Key, Qt::ButtonState)),
            SLOT(emitKeyPressed(const QModelIndex&, Qt::Key, Qt::ButtonState)));
    connect(selectionModel(),
            SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(emitCurrentChanged(const QModelIndex&, const QModelIndex&)));
    connect(selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SIGNAL(selectionChanged()));
}

#include "moc_qtablewidget.cpp"

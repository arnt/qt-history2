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
    inline long tableIndex(int row, int column) const {  return (row * c) + column; }

private:
    int r, c;
    QVector<QTableWidgetItem*> table;
    QVector<QTableWidgetItem*> verticalHeader;
    QVector<QTableWidgetItem*> horizontalHeader;
    mutable QChar strbuf[65];
};

QTableModel::QTableModel(int rows, int columns, QTableWidget *parent)
    : QAbstractTableModel(parent), r(rows), c(columns),
      table(rows * columns), verticalHeader(rows), horizontalHeader(columns) {}

QTableModel::~QTableModel()
{
    for (int i = 0; i < r * c; ++i) {
        if (table.at(i)) {
            table.at(i)->model = 0;
            delete table.at(i);
        }
    }
    for (int j = 0; j < r; ++j) {
        if (verticalHeader.at(j)) {
            verticalHeader.at(j)->model = 0;
            delete verticalHeader.at(j);
        }
    }
    for (int k = 0; k < c; ++k) {
        if (horizontalHeader.at(k)) {
            horizontalHeader.at(k)->model = 0;
            delete horizontalHeader.at(k);
        }
    }
}

bool QTableModel::insertRows(int, const QModelIndex &, int)
{
// FIXME: not implemented
    qDebug("insertRows: not implemented");
    return false;
}

bool QTableModel::insertColumns(int, const QModelIndex &, int)
{
// FIXME: not implemented
    qDebug("insertColumns: not implemented");
    return false;
}

bool QTableModel::removeRows(int, const QModelIndex &, int)
{
// FIXME: not implemented
    qDebug("removeRows: not implemented");
    return false;
}

bool QTableModel::removeColumns(int, const QModelIndex &, int)
{
// FIXME: not implemented
    qDebug("removeColumns: not implemented");
    return false;
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

    i = verticalHeader.indexOf(item);
    if (i != -1) {
        verticalHeader[i] = 0;
        return;
    }

    i = horizontalHeader.indexOf(item);
    if (i != -1) {
        horizontalHeader[i] = 0;
        return;
    }
}

void QTableModel::setHorizontalHeaderItem(int section, QTableWidgetItem *item)
{
    Q_ASSERT(item);
    item->model = this;
    verticalHeader[section] = item;
}

void QTableModel::setVerticalHeaderItem(int section, QTableWidgetItem *item)
{
    Q_ASSERT(item);
    item->model = this;
    horizontalHeader[section] = item;
}

QTableWidgetItem *QTableModel::horizontalHeaderItem(int section)
{
    return horizontalHeader.at(section);
}

QTableWidgetItem *QTableModel::verticalHeaderItem(int section)
{
    return verticalHeader.at(section);
}

QModelIndex QTableModel::index(const QTableWidgetItem *item) const
{
    // ### slow but will be fixed when table is not a vector anymore
    int i = table.indexOf(const_cast<QTableWidgetItem*>(item));
    int row = i / columnCount();
    int col = i % columnCount();
    return index(row, col);
}


QModelIndex QTableModel::index(int row, int column, const QModelIndex &) const
{
    if (row >= 0 && row < r && column >= 0 && column < c) {
        QTableWidgetItem *item = table.at(tableIndex(row, column)); // FIXME: headers ?
        return createIndex(row, column, item);
    }
    return QModelIndex::Null;
}

void QTableModel::setRowCount(int rows)
{
    if (r == rows)
        return;
    int _r = qMin(r, rows);
    int s = rows * c;
    r = rows;

    int top = qMax(_r - 1, 0);
    int bottom = qMax(r - 1, 0);

    if (r < _r)
        emit rowsRemoved(QModelIndex::Null, top, bottom);

    table.resize(s); // FIXME: this will destroy the layout and leak memory
    verticalHeader.resize(r);
    for (int j = _r; j < r; ++j)
        verticalHeader[j] = 0;

    if (r >= _r)
        emit rowsInserted(QModelIndex::Null, top, bottom);
}

void QTableModel::setColumnCount(int columns)
{
    if (c == columns)
        return;
    int _c = qMin(c, columns);
    long s = r * columns;
    c = columns;

    int left = qMax(_c - 1, 0);
    int right = qMax(c - 1, 0);

    if (c < _c)
        emit columnsRemoved(QModelIndex::Null, left, right);

    table.resize(s); // FIXME: this will destroy the layout and leak memory
    horizontalHeader.resize(c);
    for (int j = _c; j < c; ++j)
        horizontalHeader[j] = 0;

    if (c >= _c)
        emit columnsInserted(QModelIndex::Null, left, right);
}

int QTableModel::rowCount() const
{
    return r;
}

int QTableModel::columnCount() const
{
    return c;
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
        itm = horizontalHeader.at(section);
    else
        itm = verticalHeader.at(section);

    if (itm)
        return itm->data(role);
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool QTableModel::setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value)
{
    QTableWidgetItem *itm = 0;
    if (orientation == Qt::Horizontal)
        itm = horizontalHeader.at(section);
    else
        itm = verticalHeader.at(section);

    if (itm) {
        itm->setData(role, value);
        return true;
    }

    return false;
}

bool QTableModel::isValid(const QModelIndex &index) const
{
    return index.isValid() && index.row() < r && index.column() < c;
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

class QTableWidgetPrivate : public QTableViewPrivate
{
    Q_DECLARE_PUBLIC(QTableWidget)
public:
    QTableWidgetPrivate() : QTableViewPrivate() {}
    inline QTableModel *model() const { return ::qt_cast<QTableModel*>(q_func()->model()); }
};

#define d d_func()
#define q q_func()

/*!
    Creates a new table view with the given \a parent. The table view
    uses a QTableModel to hold its data.
*/
QTableWidget::QTableWidget(QWidget *parent)
    : QTableView(*new QTableWidgetPrivate, parent)
{
    setModel(new QTableModel(0, 0, this));
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
void QTableWidget::insertRow(int /*row*/)
{

}

/*!
  ###
*/
void QTableWidget::insertColumn(int /*column*/)
{

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

void QTableWidget::removeItem(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    d->model()->removeItem(item);
}

void QTableWidget::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
}

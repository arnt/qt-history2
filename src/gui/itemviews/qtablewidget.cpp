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
#include <private/qtableview_p.h>

class QTableModel : public QAbstractTableModel
{
public:
    QTableModel(int rows = 0, int columns = 0, QObject *parent = 0);
    ~QTableModel();

    virtual void setRowCount(int rows);
    virtual void setColumnCount(int columns);

    virtual bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null,
                            int count = 1);
    virtual bool insertColumns(int column, const QModelIndex &parent = QModelIndex::Null,
                               int count = 1);

    virtual bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null,
                            int count = 1);
    virtual bool removeColumns(int column, const QModelIndex &parent = QModelIndex::Null,
                               int count = 1);

    virtual void setText(int row, int column, const QString &text);
    virtual void setIconSet(int row, int column, const QIconSet &iconSet);
    QString text(int row, int column) const;
    QIconSet iconSet(int row, int column) const;

    virtual void setRowText(int row, const QString &text);
    virtual void setRowIconSet(int row, const QIconSet &iconSet);
    QString rowText(int row) const;
    QIconSet rowIconSet(int row) const;

    virtual void setColumnText(int column, const QString &text);
    virtual void setColumnIconSet(int column, const QIconSet &iconSet);
    QString columnText(int column) const;
    QIconSet columnIconSet(int column) const;

    void setItem(int row, int column, const QTableWidgetItem &item);
    QTableWidgetItem item(int row, int column) const;
    QTableWidgetItem item(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null,
                      QModelIndex::Type type = QModelIndex::View) const;

    int rowCount() const;
    int columnCount() const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

    bool isValid(const QModelIndex &index) const;
    int tableIndex(int row, int column) const;

private:
    int r, c;
    QVector<QTableWidgetItem> table;
    QVector<QTableWidgetItem> leftHeader;
    QVector<QTableWidgetItem> topHeader;
};

QTableModel::QTableModel(int rows, int columns, QObject *parent)
    : QAbstractTableModel(parent), r(rows), c(columns),
      table(rows * columns), leftHeader(rows), topHeader(columns) {}

QTableModel::~QTableModel()
{
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

    table.resize(s); // FIXME: this will destroy the layout
    leftHeader.resize(r);
    for (int j = _r; j < r; ++j)
        leftHeader[j].setText(QString::number(j));

    if (r >= _r)
        emit rowsInserted(QModelIndex::Null, top, bottom);
}

void QTableModel::setColumnCount(int columns)
{
    if (c == columns)
        return;
    int _c = qMin(c, columns);
    int s = r * columns;
    c = columns;

    int left = qMax(_c - 1, 0);
    int right = qMax(c - 1, 0);

    if (c < _c)
        emit columnsRemoved(QModelIndex::Null, left, right);

    table.resize(s); // FIXME: this will destroy the layout
    topHeader.resize(c);
    for (int j = _c; j < c; ++j)
        topHeader[j].setText(QString::number(j));

    if (c >= _c)
        emit columnsInserted(QModelIndex::Null, left, right);
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

void QTableModel::setText(int row, int column, const QString &text)
{
    QModelIndex index = createIndex(row, column);
    setData(index, QAbstractItemModel::DisplayRole, QVariant(text));
}

void QTableModel::setIconSet(int row, int column, const QIconSet &iconSet)
{
    QModelIndex index = createIndex(row, column);
    setData(index, QAbstractItemModel::DecorationRole, QVariant(iconSet));
}

QString QTableModel::text(int row, int column) const
{
    QModelIndex index = createIndex(row, column);
    return data(index, QAbstractItemModel::DisplayRole).toString();
}

QIconSet QTableModel::iconSet(int row, int column) const
{
    QModelIndex index = createIndex(row, column);
    return data(index, QAbstractItemModel::DecorationRole).toIconSet();
}

void QTableModel::setRowText(int row, const QString &text)
{
    QModelIndex index = QAbstractItemModel::createIndex(row, 0, 0, QModelIndex::VerticalHeader);
    setData(index, QAbstractItemModel::DecorationRole, QVariant(text));
}

void QTableModel::setRowIconSet(int row, const QIconSet &iconSet)
{
    QModelIndex index = createIndex(row, 0, 0, QModelIndex::VerticalHeader);
    setData(index, QAbstractItemModel::DecorationRole, QVariant(iconSet));
}

QString QTableModel::rowText(int row) const
{
    QModelIndex index = createIndex(row, 0, 0, QModelIndex::VerticalHeader);
    return data(index, QAbstractItemModel::DisplayRole).toString();
}

QIconSet QTableModel::rowIconSet(int row) const
{
    QModelIndex index = createIndex(row, 0, 0, QModelIndex::VerticalHeader);
    return data(index, QAbstractItemModel::DecorationRole).toIconSet();
}

void QTableModel::setColumnText(int column, const QString &text)
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QAbstractItemModel::DisplayRole, QVariant(text));
}

void QTableModel::setColumnIconSet(int column, const QIconSet &iconSet)
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QAbstractItemModel::DecorationRole, QVariant(iconSet));
}

QString QTableModel::columnText(int column) const
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, QAbstractItemModel::DisplayRole).toString();
}

QIconSet QTableModel::columnIconSet(int column) const
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, QAbstractItemModel::DecorationRole).toIconSet();
}

void QTableModel::setItem(int row, int column, const QTableWidgetItem &item)
{
    table[tableIndex(row, column)] = item;
}

QTableWidgetItem QTableModel::item(int row, int column) const
{
    return table.at(tableIndex(row, column));
}

QTableWidgetItem QTableModel::item(const QModelIndex &index) const
{
    if (!isValid(index))
        return QTableWidgetItem();
    if (index.type() == QModelIndex::VerticalHeader)
        return leftHeader.at(index.row());
    else if (index.type() == QModelIndex::HorizontalHeader)
        return topHeader.at(index.column());
    else
        return table.at(tableIndex(index.row(), index.column()));
    return QTableWidgetItem();
}

QModelIndex QTableModel::index(int row, int column, const QModelIndex &, QModelIndex::Type type) const
{
    if (row >= 0 && row < r && column >= 0 && column < c)
        return createIndex(row, column, 0, type);
    return QModelIndex::Null;
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
    if (index.isValid())
        return item(index).data(role);
    return QVariant();
}

bool QTableModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid())
        return false;
    if (index.type() == QModelIndex::VerticalHeader)
        leftHeader[index.row()].setData(role, value);
    else if (index.type() == QModelIndex::HorizontalHeader)
        topHeader[index.column()].setData(role, value);
    else
        table[tableIndex(index.row(), index.column())].setData(role, value);
    emit dataChanged(index, index);
    return true;
}

bool QTableModel::isSelectable(const QModelIndex &index) const
{
    return item(index).isSelectable();
}

bool QTableModel::isEditable(const QModelIndex &index) const
{
    return item(index).isEditable();
}

bool QTableModel::isValid(const QModelIndex &index) const
{
    return index.isValid() && index.row() < r && index.column() < c;
}

int QTableModel::tableIndex(int row, int column) const
{
    return (row * c) + column;
}


bool QTableWidgetItem::operator ==(const QTableWidgetItem &other) const
{
    if (values.count() != other.values.count()
        || edit != other.edit
        || select != other.select)
        return false;

    for (int i = 0; values.count(); ++i)
        if (values.at(i).role != other.values.at(i).role
            || values.at(i).value != other.values.at(i).value)
            return false;
    return true;
}

QVariant QTableWidgetItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
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


/*!
    \class QTableWidget qtablewidget.h
    \brief The QTableWidget class provides a table view that uses the
    predefined QTableModel model.

    \ingroup model-view
    \mainclass

    If you want a table that uses your own data model you should
    subclass QTableView rather than this class.

    Items are set with setItem(), or with setText() or setIconSet();
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
    Returns the item for the given \a row and \a column.

    \sa setItem() text() iconSet()
*/
QTableWidgetItem QTableWidget::item(int row, int column) const
{
    return d->model()->item(row, column);
}

/*!
    Sets the item for the given \a row and \a column to \a item.

    \sa item() setText() setIconSet()
*/
void QTableWidget::setItem(int row, int column, const QTableWidgetItem &item)
{
    d->model()->setItem(row, column, item);
}

/*!
    Sets the text for the given \a row and \a column to \a text.

    \sa text() setIconSet() setItem()
*/
void QTableWidget::setText(int row, int column, const QString &text)
{
    d->model()->setText(row, column, text);
}

/*!
    Sets the icon for the given \a row and \a column to \a iconSet.

    \sa iconSet() setText() setItem()
*/
void QTableWidget::setIconSet(int row, int column, const QIconSet &iconSet)
{
    d->model()->setIconSet(row, column, iconSet);
}

/*!
    Returns the text for the given \a row and \a column.

    \sa setText() iconSet()
*/
QString QTableWidget::text(int row, int column) const
{
    return d->model()->text(row, column);
}

/*!
    Returns the icon for the given \a row and \a column.

    \sa setIconSet() text()
*/
QIconSet QTableWidget::iconSet(int row, int column) const
{
    return d->model()->iconSet(row, column);
}

/*!
    Sets the text for the given \a row to \a text.

    \sa rowText() setRowIconSet()
*/
void QTableWidget::setRowText(int row, const QString &text)
{
    d->model()->setRowText(row, text);
}

/*!
    Sets the icon for the given \a row to \a iconSet.

    \sa rowIconSet() setRowText()
*/
void QTableWidget::setRowIconSet(int row, const QIconSet &iconSet)
{
    d->model()->setRowIconSet(row, iconSet);
}

/*!
    Returns the text for the given \a row.

    \sa setRowText() rowIconSet()
*/
QString QTableWidget::rowText(int row) const
{
    return d->model()->rowText(row);
}

/*!
    Returns the icon for the given \a row.

    \sa setRowIconSet() rowText()
*/
QIconSet QTableWidget::rowIconSet(int row) const
{
    return d->model()->rowIconSet(row);
}

/*!
    Sets the text for the given \a column to \a text.

    \sa columnText() setColumnIconSet()
*/
void QTableWidget::setColumnText(int column, const QString &text)
{
    d->model()->setColumnText(column, text);
}

/*!
    Sets the icon for the given \a column to \a iconSet.

    \sa columnIconSet() setColumnText()
*/
void QTableWidget::setColumnIconSet(int column, const QIconSet &iconSet)
{
    d->model()->setColumnIconSet(column, iconSet);
}

/*!
    Returns the text for the given \a column.

    \sa setColumnText() columnIconSet()
*/
QString QTableWidget::columnText(int column) const
{
    return d->model()->columnText(column);
}

/*!
    Returns the icon for the given \a column.

    \sa setColumnIconSet() columnText()
*/
QIconSet QTableWidget::columnIconSet(int column) const
{
    return d->model()->columnIconSet(column);
}

#include "qtablemodel.h"
#include <qtl.h>

QTableModel::QTableModel(int rows, int columns, QObject *parent, const char *name)
    : QGenericItemModel(parent, name), r(rows), c(columns),
      table(rows * columns), leftHeader(rows), topHeader(columns) {}

QTableModel::~QTableModel()
{
    qDeleteAll(table);
    qDeleteAll(leftHeader);
    qDeleteAll(topHeader);
}

void QTableModel::setRowCount(int rows)
{
    if (r == rows)
	return;
    int _r = qMin(r, rows);
    int s = rows * c;
    r = rows;

    table.resize(s); // FIXME: this will destroy the layout
    for (int i = _r * c; i < s; ++i)
	table[i] = new QTableModelItem();

    leftHeader.resize(r);
    for (int j = _r; j < r; ++j) {
        QTableModelItem *item = new QTableModelItem();
        item->setText(QString::number(j));
        leftHeader[j] = item;
    }

    int top = qMax(r - 1, 0);
    int bottom = qMax(r - 1, 0);
    int right = qMax(c - 1, 0);
    QModelIndex topLeft = index(top, 0, 0);
    QModelIndex bottomRight = index(bottom, right, 0);
    if (r > _r)
	emit contentsInserted(topLeft, bottomRight);
    else
	emit contentsRemoved(0, topLeft, bottomRight);
}

void QTableModel::setColumnCount(int columns)
{
    if (c == columns)
	return;
    int _c = qMin(c, columns);
    int s = r * columns;
    c = columns;

    table.resize(s); // FIXME: this will destroy the layout
    for (int i = r * _c; i < s; ++i)
	table[i] = new QTableModelItem();

    topHeader.resize(c);
    for (int j = _c; j < c; ++j) {
        QTableModelItem *item = new QTableModelItem();
        item->setText(QString::number(j));
        topHeader[j] = item;
    }

    int left = qMax(_c - 1, 0);
    int bottom = qMax(r - 1, 0);
    int right = qMax(c - 1, 0);
    QModelIndex topLeft = index(0, left, 0);
    QModelIndex bottomRight = index(bottom, right, 0);
    if (c > _c)
	emit contentsInserted(topLeft, bottomRight);
    else
	emit contentsRemoved(0, topLeft, bottomRight);
}

void QTableModel::insertRow(int)
{
// FIXME: not implemented
    qDebug("insertRow: not implemented");
}

void QTableModel::insertColumn(int)
{
// FIXME: not implemented
    qDebug("insertColumn: not implemented");
}

void QTableModel::removeRow(int)
{
// FIXME: not implemented
    qDebug("removeRow: not implemented");
}

void QTableModel::removeColumn(int)
{
// FIXME: not implemented
    qDebug("removeColumn: not implemented");
}

void QTableModel::setText(int row, int column, const QString &text)
{
    QModelIndex index(row, column, 0);
    setData(index, QGenericItemModel::Display, QVariant(text));
}

void QTableModel::setIconSet(int row, int column, const QIconSet &iconSet)
{
    QModelIndex index(row, column, 0);
    setData(index, QGenericItemModel::Decoration, QVariant(iconSet));
}

QString QTableModel::text(int row, int column) const
{
    QModelIndex index(row, column, 0);
    return data(index, QGenericItemModel::Display).toString();
}

QIconSet QTableModel::iconSet(int row, int column) const
{
    QModelIndex index(row, column, 0);
    return data(index, QGenericItemModel::Decoration).toIconSet();
}

void QTableModel::setRowText(int row, const QString &text)
{
    QModelIndex idx(row, 0, 0, QModelIndex::VerticalHeader);
    setData(idx, QGenericItemModel::Decoration, QVariant(text));
}

void QTableModel::setRowIconSet(int row, const QIconSet &iconSet)
{
    QModelIndex index(row, 0, 0, QModelIndex::VerticalHeader);
    setData(index, QGenericItemModel::Decoration, QVariant(iconSet));
}

QString QTableModel::rowText(int row) const
{
    QModelIndex index(row, 0, 0, QModelIndex::VerticalHeader);
    return data(index, QGenericItemModel::Display).toString();
}

QIconSet QTableModel::rowIconSet(int row) const
{
    QModelIndex index(row, 0, 0, QModelIndex::VerticalHeader);
    return data(index, QGenericItemModel::Decoration).toIconSet();
}

void QTableModel::setColumnText(int column, const QString &text)
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QGenericItemModel::Display, QVariant(text));
}

void QTableModel::setColumnIconSet(int column, const QIconSet &iconSet)
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QGenericItemModel::Decoration, QVariant(iconSet));
}

QString QTableModel::columnText(int column) const
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, QGenericItemModel::Display).toString();
}

QIconSet QTableModel::columnIconSet(int column) const
{
    QModelIndex idx(0, column, 0, QModelIndex::HorizontalHeader);
    return data(idx, QGenericItemModel::Decoration).toIconSet();
}

void QTableModel::setItem(int row, int column, QTableModelItem *item)
{
    table[tableIndex(row, column)] = item;
}

const QTableModelItem *QTableModel::item(int row, int column) const
{
    return table.at(tableIndex(row, column));
}

const QTableModelItem *QTableModel::item(const QModelIndex &index) const
{
    if (!isValid(index))
	return 0;
    if (index.type() == QModelIndex::VerticalHeader)
	return leftHeader.at(index.row());
    else if (index.type() == QModelIndex::HorizontalHeader)
	return topHeader.at(index.column());
    else
	return table.at(tableIndex(index.row(), index.column()));
    return 0;
}

QTableModelItem *QTableModel::item(const QModelIndex &index)
{
    if (!isValid(index))
	return 0;
        if (index.type() == QModelIndex::VerticalHeader)
	return leftHeader[index.row()];
    else if (index.type() == QModelIndex::HorizontalHeader)
	return topHeader[index.column()];
    else
	return table[tableIndex(index.row(), index.column())];
    return 0;
}

QModelIndex QTableModel::index(int row, int column, const QModelIndex&,
                               QModelIndex::Type type) const
{
    if (row >= 0 && row < r && column >= 0 && column < c)
	return QModelIndex(row, column, 0, type);
    return QModelIndex();
}

int QTableModel::rowCount(const QModelIndex &) const
{
    return r;
}

int QTableModel::columnCount(const QModelIndex &) const
{
    return c;
}

QVariant QTableModel::data(const QModelIndex &index, int role) const
{
    const QTableModelItem *itm = item(index);
    if (!itm)
	return QVariant();
    if (role == QGenericItemModel::Display)
	return itm->text();
    if (role == QGenericItemModel::Decoration)
	return itm->iconSet();
    return QVariant();
}

void QTableModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    QTableModelItem *itm = item(index);
    if (!itm)
	return;
    if (!itm->isEditable()) // ### should this test really be here?
	return;
    if (role == QGenericItemModel::Display)
	itm->setText(value.toString());
    else if (role == QGenericItemModel::Decoration)
	itm->setIconSet(value.toIconSet());
    emit contentsChanged(index, index);
}

// inserts a complete row, returns index of leftmost item
QModelIndex QTableModel::insertItem(const QModelIndex &index)
{
    QModelIndex insert = index;
    if (!insert.isValid() ||
	insert.row() > rowCount() ||
	insert.column() > 0)
	insert = QModelIndex(rowCount(), 0, 0);

    int ti = tableIndex(insert.row(), 0) - 1;
    table.insert(ti, c, 0);
    for (int i = ti; i < (ti + c); ++i)
	table[i] = new QTableModelItem();

    QTableModelItem *item = new QTableModelItem();
    item->setText(QString::number(index.row()));
    leftHeader.insert(index.row(), 1, item);
    ++r;

    return index;
}

bool QTableModel::isSelectable(const QModelIndex &index) const
{
    const QTableModelItem *itm = item(index);
    return itm && itm->isSelectable();
}

bool QTableModel::isEditable(const QModelIndex &index) const
{
    const QTableModelItem *itm = item(index);
    return itm && itm->isEditable();
}

bool QTableModel::isValid(const QModelIndex &index) const
{
    return index.isValid() && index.row() < r && index.column() < c;
}

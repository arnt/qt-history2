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

void QTableModel::setText(int row, int column, const QString &text)
{
    QModelIndex index(row, column, 0);
    int e = element(index, QVariant::String);
    setData(index, e, QVariant(text));
}

void QTableModel::setIconSet(int row, int column, const QIconSet &iconSet)
{
    QModelIndex index(row, column, 0);
    int e = element(index, QVariant::String);
    setData(index, e, QVariant(iconSet));
}

QString QTableModel::text(int row, int column) const
{
    QModelIndex index(row, column, 0);
    int e = element(index, QVariant::String);
    return data(index, e).toString();
}

QIconSet QTableModel::iconSet(int row, int column) const
{
    QModelIndex index(row, column, 0);
    int e = element(index, QVariant::IconSet);
    return data(index, e).toIconSet();
}

void QTableModel::setRowText(int row, const QString &text)
{
    QModelIndex index(row, 0, 0, QModelIndex::VerticalHeader);
    int e = element(index, QVariant::String);
    setData(index, e, QVariant(text));
}

void QTableModel::setRowIconSet(int row, const QIconSet &iconSet)
{
    QModelIndex index(row, 0, 0, QModelIndex::VerticalHeader);
    int e = element(index, QVariant::IconSet);
    setData(index, e, QVariant(iconSet));
}

QString QTableModel::rowText(int row) const
{
    QModelIndex index(row, 0, 0, QModelIndex::VerticalHeader);
    int e = element(index, QVariant::String);
    return data(index, e).toString();
}

QIconSet QTableModel::rowIconSet(int row) const
{
    QModelIndex index(row, 0, 0, QModelIndex::VerticalHeader);
    int e = element(index, QVariant::IconSet);
    return data(index, e).toIconSet();
}

void QTableModel::setColumnText(int column, const QString &text)
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    int e = element(index, QVariant::String);
    setData(index, e, QVariant(text));
}

void QTableModel::setColumnIconSet(int column, const QIconSet &iconSet)
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    int e = element(index, QVariant::IconSet);
    setData(index, e, QVariant(iconSet));
}

QString QTableModel::columnText(int column) const
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    int e = element(index, QVariant::String);
    return data(index, e).toString();
}

QIconSet QTableModel::columnIconSet(int column) const
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    int e = element(index, QVariant::IconSet);
    return data(index, e).toIconSet();
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

QModelIndex QTableModel::index(int row, int column, const QModelIndex &) const
{
    if (row >= 0 && row < r && column >= 0 && column < c)
	return QModelIndex(row, column, 0);
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

QVariant QTableModel::data(const QModelIndex &index, int element) const
{
    const QTableModelItem *itm = item(index);
    if (!itm)
	return QVariant();
    if (element == 0)
	return itm->text();
    if (element == 1)
	return itm->iconSet();
    return QVariant();
}

void QTableModel::setData(const QModelIndex &index, int element, const QVariant &variant)
{
    QTableModelItem *itm = item(index);
    if (!itm)
	return;
    if (!itm->isEditable())
	return;
    if (element == 0)
	itm->setText(variant.toString());
    else if (element == 1)
	itm->setIconSet(variant.toIconSet());
    emit contentsChanged(index, index);
}

QVariant::Type QTableModel::type(const QModelIndex &index, int element) const
{
    const QTableModelItem *itm = item(index);
    if (!itm)
	return QVariant::Invalid;
    if (element == 0)
	return QVariant::String;
    if (element == 1)
	return QVariant::IconSet;
    return QVariant::Invalid;
}

int QTableModel::element(const QModelIndex &index, QVariant::Type type) const
{
    if (!index.isValid())
	return -1;
    if (type == QVariant::String)
	return 0;
    if (type == QVariant::IconSet)
	return 1;
    return -1;
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

#include "qtableview.h"
#include <qgenericheader.h>
#include <private/qgenerictableview_p.h>

class QTableModel : public QAbstractItemModel
{
public:
    QTableModel(int rows = 0, int columns = 0, QObject *parent = 0);
    ~QTableModel();

    virtual void setRowCount(int rows);
    virtual void setColumnCount(int columns);

    virtual bool insertRow(int row, const QModelIndex &parent = 0);
    virtual bool insertColumn(int column, const QModelIndex &parent = 0);

    virtual bool removeRow(int row, const QModelIndex &parent = 0);
    virtual bool removeColumn(int column, const QModelIndex &parent = 0);

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

    void setItem(int row, int column, const QTableViewItem &item);
    QTableViewItem item(int row, int column) const;
    QTableViewItem item(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = 0,
                      QModelIndex::Type type = QModelIndex::View) const;

    int rowCount(const QModelIndex &parent = 0) const;
    int columnCount(const QModelIndex &parent = 0) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::Display) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

    bool isValid(const QModelIndex &index) const;
    int tableIndex(int row, int column) const;

private:
    int r, c;
    QVector<QTableViewItem> table;
    QVector<QTableViewItem> leftHeader;
    QVector<QTableViewItem> topHeader;
};

QTableModel::QTableModel(int rows, int columns, QObject *parent)
    : QAbstractItemModel(parent), r(rows), c(columns),
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

    table.resize(s); // FIXME: this will destroy the layout
    leftHeader.resize(r);
    for (int j = _r; j < r; ++j)
        leftHeader[j].setText(QString::number(j));

    int top = qMax(r - 1, 0);
    int bottom = qMax(r - 1, 0);
    int right = qMax(c - 1, 0);
    QModelIndex topLeft = index(top, 0, 0);
    QModelIndex bottomRight = index(bottom, right, 0);
    if (r > _r)
        emit contentsInserted(topLeft, bottomRight);
    else
        emit contentsRemoved(topLeft, bottomRight);
}

void QTableModel::setColumnCount(int columns)
{
    if (c == columns)
        return;
    int _c = qMin(c, columns);
    int s = r * columns;
    c = columns;

    table.resize(s); // FIXME: this will destroy the layout
    topHeader.resize(c);
    for (int j = _c; j < c; ++j)
        topHeader[j].setText(QString::number(j));

    int left = qMax(_c - 1, 0);
    int bottom = qMax(r - 1, 0);
    int right = qMax(c - 1, 0);
    QModelIndex topLeft = index(0, left, 0);
    QModelIndex bottomRight = index(bottom, right, 0);
    if (c > _c)
        emit contentsInserted(topLeft, bottomRight);
    else
        emit contentsRemoved(topLeft, bottomRight);
}

bool QTableModel::insertRow(int, const QModelIndex &)
{
// FIXME: not implemented
    qDebug("insertRow: not implemented");
    return false;
}

bool QTableModel::insertColumn(int, const QModelIndex &)
{
// FIXME: not implemented
    qDebug("insertColumn: not implemented");
    return false;
}

bool QTableModel::removeRow(int, const QModelIndex &)
{
// FIXME: not implemented
    qDebug("removeRow: not implemented");
    return false;
}

bool QTableModel::removeColumn(int, const QModelIndex &)
{
// FIXME: not implemented
    qDebug("removeColumn: not implemented");
    return false;
}

void QTableModel::setText(int row, int column, const QString &text)
{
    QModelIndex index(row, column, 0);
    setData(index, QAbstractItemModel::Display, QVariant(text));
}

void QTableModel::setIconSet(int row, int column, const QIconSet &iconSet)
{
    QModelIndex index(row, column, 0);
    setData(index, QAbstractItemModel::Decoration, QVariant(iconSet));
}

QString QTableModel::text(int row, int column) const
{
    QModelIndex index(row, column, 0);
    return data(index, QAbstractItemModel::Display).toString();
}

QIconSet QTableModel::iconSet(int row, int column) const
{
    QModelIndex index(row, column, 0);
    return data(index, QAbstractItemModel::Decoration).toIconSet();
}

void QTableModel::setRowText(int row, const QString &text)
{
    QModelIndex idx(row, 0, 0, QModelIndex::VerticalHeader);
    setData(idx, QAbstractItemModel::Decoration, QVariant(text));
}

void QTableModel::setRowIconSet(int row, const QIconSet &iconSet)
{
    QModelIndex index(row, 0, 0, QModelIndex::VerticalHeader);
    setData(index, QAbstractItemModel::Decoration, QVariant(iconSet));
}

QString QTableModel::rowText(int row) const
{
    QModelIndex index(row, 0, 0, QModelIndex::VerticalHeader);
    return data(index, QAbstractItemModel::Display).toString();
}

QIconSet QTableModel::rowIconSet(int row) const
{
    QModelIndex index(row, 0, 0, QModelIndex::VerticalHeader);
    return data(index, QAbstractItemModel::Decoration).toIconSet();
}

void QTableModel::setColumnText(int column, const QString &text)
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QAbstractItemModel::Display, QVariant(text));
}

void QTableModel::setColumnIconSet(int column, const QIconSet &iconSet)
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QAbstractItemModel::Decoration, QVariant(iconSet));
}

QString QTableModel::columnText(int column) const
{
    QModelIndex index(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, QAbstractItemModel::Display).toString();
}

QIconSet QTableModel::columnIconSet(int column) const
{
    QModelIndex idx(0, column, 0, QModelIndex::HorizontalHeader);
    return data(idx, QAbstractItemModel::Decoration).toIconSet();
}

void QTableModel::setItem(int row, int column, const QTableViewItem &item)
{
    table[tableIndex(row, column)] = item;
}

QTableViewItem QTableModel::item(int row, int column) const
{
    return table.at(tableIndex(row, column));
}

QTableViewItem QTableModel::item(const QModelIndex &index) const
{
    if (!isValid(index))
        return QTableViewItem();
    if (index.type() == QModelIndex::VerticalHeader)
        return leftHeader.at(index.row());
    else if (index.type() == QModelIndex::HorizontalHeader)
        return topHeader.at(index.column());
    else
        return table.at(tableIndex(index.row(), index.column()));
    return QTableViewItem();
}

QModelIndex QTableModel::index(int row, int column, const QModelIndex &, QModelIndex::Type type) const
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
    emit contentsChanged(index, index);
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


bool QTableViewItem::operator ==(const QTableViewItem &other) const
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

QVariant QTableViewItem::data(int role) const
{
    role = (role == QAbstractItemModel::Edit ? QAbstractItemModel::Display : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
}

void QTableViewItem::setData(int role, const QVariant &value)
{
    role = (role == QAbstractItemModel::Edit ? QAbstractItemModel::Display : role);
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).role == role) {
            values[i].value = value;
            return;
        }
    }
    values.append(Data(role, value));
}


/*!
  \class QTableView qtableview.h

  \brief Table view implementation using the QTableModel by default
*/

class QTableViewPrivate : public QGenericTableViewPrivate
{
    Q_DECLARE_PUBLIC(QTableView)
public:
    QTableViewPrivate() : QGenericTableViewPrivate() {}
    inline QTableModel *model() const { return ::qt_cast<QTableModel*>(q_func()->model()); }
};

#define d d_func()
#define q q_func()

QTableView::QTableView(QWidget *parent)
    : QGenericTableView(*new QTableViewPrivate, new QTableModel(), parent)
{
    model()->setParent(this); // make sure the model gets deleted
}

QTableView::~QTableView()
{
}

void QTableView::setRowCount(int rows)
{
    d->model()->setRowCount(rows);
}

void QTableView::setColumnCount(int columns)
{
    d->model()->setColumnCount(columns);
}

int QTableView::rowCount() const
{
    return model()->rowCount();
}

int QTableView::columnCount() const
{
    return model()->columnCount();
}

QTableViewItem QTableView::item(int row, int column) const
{
    return d->model()->item(row, column);
}

void QTableView::setItem(int row, int column, const QTableViewItem &item)
{
    d->model()->setItem(row, column, item);
}

void QTableView::setText(int row, int column, const QString &text)
{
    d->model()->setText(row, column, text);
}

void QTableView::setIconSet(int row, int column, const QIconSet &iconSet)
{
    d->model()->setIconSet(row, column, iconSet);
}

QString QTableView::text(int row, int column) const
{
    return d->model()->text(row, column);
}

QIconSet QTableView::iconSet(int row, int column) const
{
    return d->model()->iconSet(row, column);
}

void QTableView::setRowText(int row, const QString &text)
{
    d->model()->setRowText(row, text);
}

void QTableView::setRowIconSet(int row, const QIconSet &iconSet)
{
    d->model()->setRowIconSet(row, iconSet);
}

QString QTableView::rowText(int row) const
{
    return d->model()->rowText(row);
}

QIconSet QTableView::rowIconSet(int row) const
{
    return d->model()->rowIconSet(row);
}

void QTableView::setColumnText(int column, const QString &text)
{
    d->model()->setColumnText(column, text);
}

void QTableView::setColumnIconSet(int column, const QIconSet &iconSet)
{
    d->model()->setColumnIconSet(column, iconSet);
}

QString QTableView::columnText(int column) const
{
    return d->model()->columnText(column);
}

QIconSet QTableView::columnIconSet(int column) const
{
    return d->model()->columnIconSet(column);
}

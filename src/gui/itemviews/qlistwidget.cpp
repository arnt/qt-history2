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
#include <private/qgenericlistview_p.h>

class QListModel : public QAbstractListModel
{
public:
    QListModel(QObject *parent = 0);

    QListWidgetItem item(int row) const;
    void setItem(int row, const QListWidgetItem &item);
    void append(const QListWidgetItem &item);

    int rowCount() const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int row, const QModelIndex &parent = QModelIndex(), int count = 1);
    bool removeRows(int row, const QModelIndex &parent = QModelIndex(), int count = 1);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

private:
    QList<QListWidgetItem> lst;
};

QListModel::QListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QListWidgetItem QListModel::item(int row) const
{
    if (row >= 0 && row < (int)lst.count())
        return lst.at(row);
    else
        return QListWidgetItem(); // FIXME we need invalid?
}

void QListModel::setItem(int row, const QListWidgetItem &item)
{
    if (row >= 0 && row < (int)lst.count())
        lst[row] = item;
}

int QListModel::rowCount() const
{
    return lst.count();
}

QVariant QListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
        return QVariant();
    return lst.at(index.row()).data(role);
}

bool QListModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid() || index.row() >= (int)lst.count())
        return false;
    lst[index.row()].setData(role, value);
    emit dataChanged(index, index);
    return true;
}

bool QListModel::insertRows(int row, const QModelIndex &, int)
{
    QListWidgetItem item;
    if (row < rowCount())
        lst.insert(row, item);
    else
        lst.append(item);
    return true;
}

bool QListModel::removeRows(int row, const QModelIndex &, int)
{
    if (row < rowCount()) {
        lst.removeAt(row);
        return true;
    }
    return false;
}

bool QListModel::isSelectable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
        return false;
    return lst.at(index.row()).isSelectable();
}

bool QListModel::isEditable(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
        return false;
    return lst.at(index.row()).isEditable();
}

void QListModel::append(const QListWidgetItem &item)
{
    lst.append(item);
    int row = lst.count() - 1;
    emit rowsInserted(QModelIndex(), row, row);
}

/*!
    \class QListWidgetItem
*/

/*!
    \fn QListWidgetItem::QListWidgetItem()
*/

/*!
    \fn QListWidgetItem::~QListWidgetItem()
*/

/*!
    \fn QString QListWidgetItem::text() const
*/

/*!
    \fn QIconSet QListWidgetItem::iconSet() const
*/

/*!
    \fn bool QListWidgetItem::isEditable() const
*/

/*!
    \fn bool QListWidgetItem::isSelectable() const
*/

/*!
    \fn void QListWidgetItem::setText(const QString &text)
*/

/*!
    \fn void QListWidgetItem::setIconSet(const QIconSet &iconSet)
*/

/*!
    \fn void QListWidgetItem::setEditable(bool editable)
*/

/*!
    \fn void QListWidgetItem::setSelectable(bool selectable)
*/

/*!
    \fn bool QListWidgetItem::operator !=(const QListWidgetItem &other)
*/

/*!
*/

bool QListWidgetItem::operator ==(const QListWidgetItem &other) const
{
    if (values.count() != other.values.count()
        || edit != other.edit
        || select != other.select)
        return false;

    for (int i=0; values.count(); ++i)
        if (values.at(i).role != other.values.at(i).role
            || values.at(i).value != other.values.at(i).value)
            return false;

    return true;
}

/*!
*/

QVariant QListWidgetItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).role == role)
            return values.at(i).value;
    }
    return QVariant();
}

/*!
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
}

class QListWidgetPrivate : public QGenericListViewPrivate
{
    Q_DECLARE_PUBLIC(QListWidget)
public:
    QListWidgetPrivate() : QGenericListViewPrivate() {}
    inline QListModel *model() const { return ::qt_cast<QListModel*>(q_func()->model()); }
};

#define d d_func()
#define q q_func()

#ifdef QT_COMPAT
QListWidget::QListWidget(QWidget *parent, const char* name)
    : QGenericListView(*new QListWidgetPrivate(), parent)
{
    setModel(new QListModel(this));
    setObjectName(name);
}
#endif

/*!
    \class QListWidget
*/

/*!
*/

QListWidget::QListWidget(QWidget *parent)
    : QGenericListView(*new QListWidgetPrivate(), parent)
{
    setModel(new QListModel(this));
    model()->setParent(this);
}

/*!
*/

QListWidget::~QListWidget()
{
}

/*!
*/

void QListWidget::setText(int row, const QString &text)
{
    model()->setData(model()->index(row,0), QAbstractItemModel::DisplayRole, text);
}

/*!
*/

void QListWidget::setIconSet(int row, const QIconSet &iconSet)
{
    model()->setData(model()->index(row,0), QAbstractItemModel::DecorationRole, iconSet);
}

/*!
*/

QString QListWidget::text(int row) const
{
    return model()->data(model()->index(row,0), QAbstractItemModel::DisplayRole).toString();
}

/*!
*/

QIconSet QListWidget::iconSet(int row) const
{
    return model()->data(model()->index(row,0), QAbstractItemModel::DecorationRole).toIconSet();
}

/*!
*/

QListWidgetItem QListWidget::item(int row) const
{
    return d->model()->item(row);
}

/*!
*/

void QListWidget::setItem(int row, const QListWidgetItem &item)
{
    d->model()->setItem(row, item);
}

/*!
*/

void QListWidget::appendItem(const QListWidgetItem &item)
{
    d->model()->append(item);
}

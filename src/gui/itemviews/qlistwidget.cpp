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
#include <private/qlistview_p.h>

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

    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);

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
    emit rowsInserted(QModelIndex::Null, row, row);
}

/*!
    \class QListWidgetItem
    \brief The QListWidgetItem class holds a single list widget data
    item.

    ###
*/

/*!
    \fn QListWidgetItem::QListWidgetItem()

    Creates a new list widget item that isEditable() and
    isSelectable(), but which has no text() or icon().
*/

/*!
    \fn QListWidgetItem::~QListWidgetItem()

    Destructs this list widget item.
*/

/*!
    \fn QString QListWidgetItem::text() const

    Returns this list widget item's text.

    \sa setText()
*/

/*!
    \fn QIconSet QListWidgetItem::icon() const

    Returns this list widget item's iconset.

    \sa setIcon()
*/

/*!
    \fn bool QListWidgetItem::isEditable() const

    Returns true if this list widget item is editable; otherwise
    returns false.

    \sa setEditable()
*/

/*!
    \fn bool QListWidgetItem::isSelectable() const

    Returns true if this list widget item is selectable; otherwise
    returns false.

    \sa setSelectable()
*/

/*!
    \fn void QListWidgetItem::setText(const QString &text)

    Sets this list widget item's \a text.

    \sa text()
*/

/*!
    \fn void QListWidgetItem::setIcon(const QIconSet &icon)

    Sets this list widget item's \a icon.

    \sa icon()
*/

/*!
    \fn void QListWidgetItem::setEditable(bool editable)

    If \a editable is true, this list widget item can be edited;
    otherwise it cannot be edited.

    \sa isEditable()
*/

/*!
    \fn void QListWidgetItem::setSelectable(bool selectable)

    If \a selectable is true, this list widget item can be selected;
    otherwise it cannot be selected.

    \sa isSelectable()
*/

/*!
    \fn bool QListWidgetItem::operator!=(const QListWidgetItem &other) const

    Returns true if this list widget item and the \a other list widget
    item have at least one role for which their values differ;
    otherwise returns false.
*/

/*!
    Returns true if this list widget item and the \a other list widget
    item have the same values for every role; otherwise returns false.
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
    Returns the data for this list widget item's \a role.

    \sa setData()
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
    Sets the data for this list widget item's \a role to \a value.

    \sa data()
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

class QListWidgetPrivate : public QListViewPrivate
{
    Q_DECLARE_PUBLIC(QListWidget)
public:
    QListWidgetPrivate() : QListViewPrivate() {}
    inline QListModel *model() const { return ::qt_cast<QListModel*>(q_func()->model()); }
};

#define d d_func()
#define q q_func()

#ifdef QT_COMPAT
/*!
    Use the single-argument overload and call setObjectName() instead.
*/
QListWidget::QListWidget(QWidget *parent, const char* name)
    : QListView(*new QListWidgetPrivate(), parent)
{
    setModel(new QListModel(this));
    setObjectName(name);
}
#endif

/*!
    \class QListWidget
    \brief The QListWidget class provides a list or icon view with a
    predefined model.

    \mainclass

    ###
*/

/*!
    Constructs a new QListWidget with the given \a parent.
*/

QListWidget::QListWidget(QWidget *parent)
    : QListView(*new QListWidgetPrivate(), parent)
{
    setModel(new QListModel(this));
    model()->setParent(this);
}

/*!
    Destructs this list widget.
*/

QListWidget::~QListWidget()
{
}

/*!
    Sets the \a{row}-th item's \a text.

    \sa text() setItem()
*/

void QListWidget::setText(int row, const QString &text)
{
    model()->setData(model()->index(row,0), QAbstractItemModel::DisplayRole, text);
}

/*!
    Sets the \a{row}-th item's \a icon.

    \sa iconSet() setItem()
*/

void QListWidget::setIconSet(int row, const QIconSet &icon)
{
    model()->setData(model()->index(row,0), QAbstractItemModel::DecorationRole, icon);
}

/*!
    Returns the text for the \a{row}-th item.

    \sa item() icon() setItem()
*/

QString QListWidget::text(int row) const
{
    return model()->data(model()->index(row,0), QAbstractItemModel::DisplayRole).toString();
}

/*!
    Returns the icon for the \a{row}-th item.

    \sa item() text() setItem()
*/

QIconSet QListWidget::iconSet(int row) const
{
    return model()->data(model()->index(row,0), QAbstractItemModel::DecorationRole).toIcon();
}

/*!
    Returns the \a{row}-th item.

    \sa text() icon() setItem()
*/

QListWidgetItem QListWidget::item(int row) const
{
    return d->model()->item(row);
}

/*!
    Sets the list's \a{row}-th \a item.

    \sa appendItem()
*/

void QListWidget::setItem(int row, const QListWidgetItem &item)
{
    d->model()->setItem(row, item);
}

/*!
    Appends the given \a item to the list.

    \sa setItem()
*/

void QListWidget::appendItem(const QListWidgetItem &item)
{
    d->model()->append(item);
}

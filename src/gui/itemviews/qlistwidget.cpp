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

class QListItemDelegate : public QItemDelegate
{
public:
    QListItemDelegate(QObject *parent) : QItemDelegate(parent) {}
    ~QListItemDelegate() {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QAbstractItemModel *model, const QModelIndex &index) const;
    QSize sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option,
                   const QAbstractItemModel *model, const QModelIndex &index) const;
};


void QListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    // enabled
    if ((model->flags(index) & QAbstractItemModel::ItemIsEnabled) == 0)
        opt.state &= ~QStyle::Style_Enabled;
    // set font
    QVariant value = model->data(index, QAbstractItemModel::FontRole);
    if (value.isValid())
        opt.font = value.toFont();
    // set text color
    value = model->data(index, QAbstractItemModel::TextColorRole);
    if (value.isValid() && value.toColor().isValid())
        opt.palette.setColor(QPalette::Text, value.toColor());
    // draw the background color
    value = model->data(index, QAbstractItemModel::BackgroundColorRole);
    if (value.isValid() && value.toColor().isValid())
        painter->fillRect(option.rect, value.toColor());
    // draw the item
    QItemDelegate::paint(painter, opt, model, index);
}

QSize QListItemDelegate::sizeHint(const QFontMetrics &/*fontMetrics*/,
                                  const QStyleOptionViewItem &option,
                                  const QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    QVariant value = model->data(index, QAbstractItemModel::FontRole);
    QFont fnt = value.isValid() ? value.toFont() : option.font;
    return QItemDelegate::sizeHint(QFontMetrics(fnt), option, model, index);
}

class QListModel : public QAbstractListModel
{
public:
    QListModel(QListWidget *parent = 0);

    QListWidgetItem *at(int row) const;
    void insert(int row, QListWidgetItem *item);
    void append(QListWidgetItem *item);
    void remove(QListWidgetItem *item);
    QListWidgetItem *take(int row);

    int rowCount() const;

    QModelIndex index(QListWidgetItem *item) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

    bool isSortable() const;
    void sort(int column, const QModelIndex &parent, Qt::SortOrder order);

private:
    QList<QListWidgetItem*> lst;
};

QListModel::QListModel(QListWidget *parent)
    : QAbstractListModel(parent)
{
}

QListWidgetItem *QListModel::at(int row) const
{
    if (row >= 0 && row < lst.count())
        return lst.at(row);
    return 0;
}

void QListModel::append(QListWidgetItem *item)
{
    lst.append(item);
    int row = lst.count() - 1;
    emit rowsInserted(QModelIndex::Null, row, row);
}

void QListModel::remove(QListWidgetItem *item)
{
    int row = lst.indexOf(item);
    if (row != -1)
        lst.removeAt(row);
}

void QListModel::insert(int row, QListWidgetItem *item)
{
    if (row >= 0 && row <= lst.count()) {
        lst.insert(row, item);
        emit rowsInserted(QModelIndex::Null, row, row);
    }
}

QListWidgetItem *QListModel::take(int row)
{
    if (row >= 0 && row <= lst.count()) {
        emit rowsRemoved(QModelIndex::Null, row, row);
        return lst.takeAt(row);
    }
    return 0;
}

int QListModel::rowCount() const
{
    return lst.count();
}

QModelIndex QListModel::index(QListWidgetItem *item) const
{
    int row = lst.indexOf(item);
    if (row == -1)
        return QModelIndex::Null;
    return createIndex(row, 0, item);
}

QModelIndex QListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (isValid(row, column, parent))
        return createIndex(row, column, lst.at(row));
    return QModelIndex::Null;
}

QVariant QListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= (int)lst.count())
        return QVariant();
    return lst.at(index.row())->data(role);
}

bool QListModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid() || index.row() >= (int)lst.count())
        return false;
    lst.at(index.row())->setData(role, value);
    emit dataChanged(index, index);
    return true;
}

bool QListModel::insertRows(int row, const QModelIndex &, int count)
{
    QListWidget *view = ::qt_cast<QListWidget*>(QObject::parent());
    if (row < rowCount())
        for (int r = row; r < row + count; ++r)
            lst.insert(r, new QListWidgetItem(view));
    else
        for (int r = 0; r < count; ++r)
            lst.append(new QListWidgetItem(view));
    emit rowsInserted(QModelIndex::Null, row, row + count - 1);
    return true;
}

bool QListModel::removeRows(int row, const QModelIndex &, int count)
{
    if (row < rowCount()) {
        emit rowsRemoved(QModelIndex::Null, row, row + count - 1);
        for (int r = 0; r < count; ++r)
            delete lst.takeAt(row);
        return true;
    }
    return false;
}

QAbstractItemModel::ItemFlags QListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= lst.count())
        return 0;
    return lst.at(index.row())->flags();
}

bool QListModel::isSortable() const
{
    return true;
}

void QListModel::sort(int column, const QModelIndex &parent, Qt::SortOrder order)
{
    if (column != 0 || parent.isValid())
        return;
    if (order == Qt::AscendingOrder)
        qHeapSort(lst.begin(), lst.end());
    else
        qHeapSort(lst.end(), lst.begin());
    emit dataChanged(index(0, 0), index(lst.count() - 1, 0));
}

/*!
    \class QListWidgetItem
    \brief The QListWidgetItem class provides an item for use with the
    QListWidget item view class.

    \ingroup model-view

*/

/*!
    Creates a new list widget item that isEditable() and
    isSelectable(), but which has no text() or icon().
*/

QListWidgetItem::QListWidgetItem(QListWidget *view)
    : view(view),
      itemFlags(QAbstractItemModel::ItemIsEditable
                |QAbstractItemModel::ItemIsSelectable
                |QAbstractItemModel::ItemIsCheckable
                |QAbstractItemModel::ItemIsEnabled)
{
    if (view)
        view->appendItem(this);
}

/*!
    Destroys the list item.
*/

QListWidgetItem::~QListWidgetItem()
{
    if (view)
        view->removeItem(this);
}

void QListWidgetItem::openPersistentEditor()
{
    if (view)
        view->openPersistentEditor(this);
}

void QListWidgetItem::closePersistentEditor()
{
    if (view)
        view->closePersistentEditor(this);
}

bool QListWidgetItem::operator<(const QListWidgetItem &other) const
{
    return text() < other.text();
}

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

QVariant QListWidgetItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
}

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
    \fn QString QListWidgetItem::statusTip() const

    Returns the list item's status tip.

    \sa setStatusTip()
*/

/*!
    \fn QString QListWidgetItem::toolTip() const

    Returns the list item's tooltip.

    \sa setToolTip()
*/

/*!
    \fn QString QListWidgetItem::whatsThis() const

    Returns the list item's "What's This?" help text.

    \sa setWhatsThis()
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
    \fn int QListWidgetItem::checked() const
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
    \fn void QListWidgetItem::setStatusTip(const QString &statusTip)

    Sets the status tip for this list item to the text specified by
    \a statusTip.

    \sa statusTip()
*/

/*!
    \fn void QListWidgetItem::setToolTip(const QString &toolTip)

    Sets the tooltip for this list item to the text specified by
    \a toolTip.

    \sa toolTip()
*/

/*!
    \fn void QListWidgetItem::setWhatsThis(const QString &whatsThis)

    Sets the "What's This?" help for this list item to the text specified
    by \a whatsThis.
*/

/*!
    \fn void QListWidgetItem::setBackgroundColor(const QColor &color)

    Sets the background \a color of the list item.

    \sa backgroundColor() setTextColor()
*/

/*!
    \fn void QListWidgetItem::setTextColor(const QColor &color)

    Sets the text \a color for this list item.

    \sa color() setBackgroundColor()
*/

/*!
    \fn void QListWidgetItem::setChecked(const bool checked)

    Checks the list item if \a checked is true; otherwise the list item
    will be shown as unchecked.

    \sa checked()
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
    setObjectName(name);
    setModel(new QListModel(this));
    setItemDelegate(new QListItemDelegate(this));
}
#endif

/*!
    \class QListWidget
    \brief The QListWidget class provides an item-based list or icon view using
    a default model.

    \ingroup model-view
    \mainclass

    QListWidget is a convenience class that provides a list view, like that
    supplied by QListView, but with a classic item-based interface for adding
    and removing items from the list. QListWidget uses an internal model
    to manage the items.

    For a more flexible list view widget, use the QListView class with a
    standard model.

    The number of items in the list can be found using the row() function
*/

/*!
    Constructs a new QListWidget with the given \a parent.
*/

QListWidget::QListWidget(QWidget *parent)
    : QListView(*new QListWidgetPrivate(), parent)
{
    setModel(new QListModel(this));
    setItemDelegate(new QListItemDelegate(this));
}

/*!
    Destroys the list widget.
*/

QListWidget::~QListWidget()
{
}

/*!
    Returns the \a{row}-th item.

    \sa setItem() row()
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

    \sa appendItem()
*/

void QListWidget::insertItem(int row, QListWidgetItem *item)
{
    Q_ASSERT(item);
    item->view = this;
    d->model()->insert(row, item);
}

/*!
    Inserts items from the list of \a labels into the list, starting at the
    given \a row.

    \sa insertItem(), appendItem()
*/

void QListWidget::insertItems(const QStringList &labels, int row)
{
    int insertRow = (row > -1 && row <= count()) ? row : count();
    for (int i=0; i<labels.count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(this);
        item->setText(labels.at(i));
        d->model()->insert(insertRow + i, item);
    }
}

/*!
    Appends the given \a item to the list.

    \sa insertItem()
*/

void QListWidget::appendItem(QListWidgetItem *item)
{
    Q_ASSERT(item);
    item->view = this;
    d->model()->append(item);
}

/*!
    Removes the item at \a row from the list without deleting it.

    \sa insertItem() appendItem()
*/

QListWidgetItem *QListWidget::takeItem(int row)
{
    QListWidgetItem *item =  d->model()->take(row);
    item->view = 0;
    return item;
}

/*!
      Returns the number of items in the list.

*/
int QListWidget::count() const
{
    return d->model()->rowCount();
}

/*!
  ###
*/

void QListWidget::sort(Qt::SortOrder order)
{
    d->model()->sort(0, QModelIndex::Null, order);
}

/*!
  Removes the given \a item from the list.
*/

void QListWidget::removeItem(QListWidgetItem *item)
{
    Q_ASSERT(item);
    d->model()->remove(item);
}

void QListWidget::setModel(QAbstractItemModel *model)
{
    QListView::setModel(model);
}

void QListWidget::openPersistentEditor(QListWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item);
    QAbstractItemView::openPersistentEditor(index);
}

void QListWidget::closePersistentEditor(QListWidgetItem *item)
{
    Q_ASSERT(item);
    QModelIndex index = d->model()->index(item);
    QAbstractItemView::closePersistentEditor(index);
}

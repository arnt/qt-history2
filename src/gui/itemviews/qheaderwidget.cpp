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

#include "qheaderwidget.h"
#include <qabstractitemmodel.h>
#include <qitemdelegate.h>
#include <qpainter.h>
#include <private/qabstractitemmodel_p.h>
#include <private/qheaderview_p.h>

class QHeaderModel : public QAbstractTableModel
{
    friend class QHeaderWidgetItem;

public:
    QHeaderModel(Qt::Orientation orientation, int sections, QHeaderWidget *parent);
    ~QHeaderModel();

    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool insertColumns(int column, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeColumns(int column, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    bool insertSections(int section, int count = 1);
    bool removeSections(int section, int count = 1);
    void setSectionCount(int sections);

    void setItem(int section, QHeaderWidgetItem *item);
    QHeaderWidgetItem *takeItem(int section);
    QHeaderWidgetItem *item(int section) const;
    void removeItem(QHeaderWidgetItem *item);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null);
    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, int role, QVariant &value);
    
    int rowCount() const;
    int columnCount() const;

    void clear();
    void emitDataChanged(QHeaderWidgetItem *item);

private:
    QVector<QHeaderWidgetItem*> items;
    Qt::Orientation orientation;
};

QHeaderModel::QHeaderModel(Qt::Orientation orientation, int sections, QHeaderWidget *parent)
    : QAbstractTableModel(parent),
      orientation(orientation)
{
    setSectionCount(sections);
}

QHeaderModel::~QHeaderModel()
{
    clear();
}

bool QHeaderModel::insertRows(int row, const QModelIndex &, int count)
{
    if (orientation == Qt::Vertical)
        return insertSections(row, count);
    return false;
}

bool QHeaderModel::insertColumns(int column, const QModelIndex &, int count)
{
    if (orientation == Qt::Horizontal)
        return insertSections(column, count);
    return false;
}

bool QHeaderModel::removeRows(int row, const QModelIndex &, int count)
{
    if (orientation == Qt::Vertical)
        return removeSections(row, count);
    return false;
}

bool QHeaderModel::removeColumns(int column, const QModelIndex &, int count)
{
    if (orientation == Qt::Horizontal)
        return removeSections(column, count);
    return false;
}


bool QHeaderModel::insertSections(int section, int count)
{
    items.insert(section, count, 0);
    return true;
}

bool QHeaderModel::removeSections(int section, int count)
{
    items.remove(section, count);
    return true;
}

void QHeaderModel::setSectionCount(int sections)
{
    items.resize(sections);
}

void QHeaderModel::setItem(int section, QHeaderWidgetItem *item)
{
    if (section >= 0 || section < items.count()) {
        delete items.at(section);
        items[section] = item;
    }
}

QHeaderWidgetItem *QHeaderModel::takeItem(int section)
{
    if (section >= 0 || section < items.count()) {
        QHeaderWidgetItem *item = items.at(section);
        items.remove(section);
        return item;
    }
    return 0;
}

QHeaderWidgetItem *QHeaderModel::item(int section) const
{
    if (section >= 0 || section < items.count())
        return items.at(section);
    return 0;
}

void QHeaderModel::removeItem(QHeaderWidgetItem *item)
{
    int i = items.indexOf(item);
    if (i != -1) {
        items.at(i)->model = 0;
        items.at(i)->view = 0;
        delete items.at(i);
        items[i] = 0;
    }
}

QModelIndex QHeaderModel::index(int, int, const QModelIndex &)
{
    return QModelIndex::Null;
}

QVariant QHeaderModel::data(const QModelIndex &, int) const
{
    return QVariant();
}

QVariant QHeaderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != this->orientation || section < 0 || section >= items.count())
        return QVariant();
    if (items.at(section))
        return items.at(section)->data(role);
    return section;
}

bool QHeaderModel::setHeaderData(int section, Qt::Orientation orientation, int role, QVariant &value)
{
    if (orientation != this->orientation || section < 0 || section >= items.count())
        return false;
    items.at(section)->setData(role, value);
    emit headerDataChanged(orientation, section, section);
    return true;
}

int QHeaderModel::rowCount() const
{
    return orientation == Qt::Vertical ? items.count() : 1;
}

int QHeaderModel::columnCount() const
{
    return orientation == Qt::Horizontal ? items.count() : 1;
}

void QHeaderModel::clear()
{
    for (int i = 0; i < items.count(); ++i) {
        items.at(i)->model = 0;
        items.at(i)->view = 0;
        delete items.at(i);
        items[i] = 0;
    }
    emit reset();
}

void QHeaderModel::emitDataChanged(QHeaderWidgetItem *item)
{
    int sec = items.indexOf(item);
    emit headerDataChanged(orientation, sec, sec);
}

/*!
  \class QHeaderWidgetItem
  \brief The QHeaderWidgetItem class provides an item for use with the
  QHeaderWidget item view class.

  \ingroup model-view

*/

/*!
  Creates an empty header item.
*/

QHeaderWidgetItem::QHeaderWidgetItem()
    : view(0), model(0),
      itemFlags(QAbstractItemModel::ItemIsEnabled)
{
}

/*!
  Destroys the header item.
*/

QHeaderWidgetItem::~QHeaderWidgetItem()
{
    if (model) {
        int sec = model->items.indexOf(this);
        model->items[sec] = 0; // remove this
    }
}

/*!
    This function sets \a value for a given \a role (see
  {QAbstractItemModel::Role}). Reimplement this function if you need
  extra roles or special behavior for certain roles.
*/

void QHeaderWidgetItem::setData(int role, const QVariant &value)
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
        model->emitDataChanged(this);
}

/*!
   This function returns the items data for a given \a role (see
   {QAbstractItemModel::Role}). Reimplement this function if you need
   extra roles or special behavior for certain roles.
*/

QVariant QHeaderWidgetItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
}

/*!
  Returns true if this items text is less then \a other items text.
*/

bool QHeaderWidgetItem::operator<(const QHeaderWidgetItem &other) const
{
    return text() < other.text();
}

/*!
  Removes all item data.
*/
void QHeaderWidgetItem::clear()
{
    values.clear();
    if (model)
        model->emitDataChanged(this);
}

#define d d_func()
#define q q_func()

// private

class QHeaderWidgetPrivate : public QHeaderViewPrivate
{
    Q_DECLARE_PUBLIC(QHeaderWidget)
public:
    QHeaderWidgetPrivate() : QHeaderViewPrivate() {}
    inline QHeaderModel *model() const { return ::qt_cast<QHeaderModel*>(q_func()->model()); }

    void emitClicked(int section, Qt::ButtonState state);
    void emitItemChanged(Qt::Orientation orientation, int first, int last);
};

void QHeaderWidgetPrivate::emitClicked(int section, Qt::ButtonState state)
{
    emit q->clicked(model()->item(section), state);
}

void QHeaderWidgetPrivate::emitItemChanged(Qt::Orientation orientation, int first, int last)
{
    Q_UNUSED(orientation);
    if (first == last) // this should always be true
        emit q->itemChanged(model()->item(first));
    else
        qWarning("QHeaderWidgetPrivate: several items were changed");
    // Only one item at a time can change, so the warning should never be shown
}

// public

/*!
  \class QHeaderWidget qheaderwidget.h

  \brief The QHeaderWidget class provides a header for the model/view
  convenience view widgets.

  \ingroup model-view

  Header widgets are used by QTableWidget and QTreeWidget to display header
  labels for each row and column that they display.

  A header widget is constructed with a specific orientation and number of
  sections.
  A header that provides the column labels at the top of a table is
  constructed with a horizontal orientation; a header that provides the rows
  has a vertical orientation. The parent widget is usually a QTableWidget or
  a QTreeWidget:

  \code
      QHeaderWidget *columnsHeader = new QHeaderWidget(Qt::Horizontal, columns, tableWidget);
      QHeaderWidget *rowsHeader = new QHeaderWidget(Qt::Vertical, rows, tableWidget);
  \endcode

  For horizontal headers, each section corresponds to a column label which
  is provided by a \c QHeaderWidgetItem. For vertical headers, each section
  corresponds to a row label. 

  \sa QHeaderWidgetItem QTableWidget QTreeWidget \link model-view-programming.html
  Model/View Programming\endlink
*/


/*!
    Constructs a header view with the given \a orientation and \a parent widget.
*/

QHeaderWidget::QHeaderWidget(Qt::Orientation orientation, int sections, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setModel(new QHeaderModel(orientation, sections, this));
}

/*!
    Destroys the header widget and all its items.
*/

QHeaderWidget::~QHeaderWidget()
{
}

/*!
  Sets the number of sections in the header to \a sections.
*/

void QHeaderWidget::setSectionCount(int sections)
{
    d->model()->setSectionCount(sections);
}

/*!
  Returns the pointer to the item at \a section.
*/

QHeaderWidgetItem *QHeaderWidget::item(int section) const
{
    return d->model()->item(section);
}

/*!
  Sets the item at \a section to be \a item.
*/

void QHeaderWidget::setItem(int section, QHeaderWidgetItem *item)
{
    item->view = this;
    item->model = d->model();
    d->model()->setItem(section, item);
}

/*!
  Removes the item at \a section and returns it.
*/

QHeaderWidgetItem *QHeaderWidget::takeItem(int section)
{
    QHeaderWidgetItem *item = d->model()->takeItem(section);
    item->view = 0;
    item->model = 0;
    return item;
}

/*!
  Removes all items in the header.
*/

void QHeaderWidget::clear()
{
    d->model()->clear();
}

/*!
  \internal
*/

void QHeaderWidget::setModel(QAbstractItemModel *model)
{
    QHeaderView::setModel(model);
}

#include "moc_qheaderwidget.cpp"

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
    friend class QHeaderWidget;
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
    int rowCount() const;
    int columnCount() const;

    void clear();

private:
    QVector<QHeaderWidgetItem*> items;
    Qt::Orientation orientation;
    mutable QChar strbuf[65];
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
        delete items.at(i);
        item[i] = 0;
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
        delete items.at(i);
        items[i] = 0;
    }
    emit reset();
}

// item

QHeaderWidgetItem::QHeaderWidgetItem(QHeaderWidget *view)
    : itemFlags(QAbstractItemModel::ItemIsEnabled),
      view(view)
{
}

QHeaderWidgetItem::~QHeaderWidgetItem()
{
    view->removeItem(this);
}

bool QHeaderWidgetItem::operator<(const QHeaderWidgetItem &other) const
{
    return text() < other.text();
}

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
}

QVariant QHeaderWidgetItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
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
};

void QHeaderWidgetPrivate::emitClicked(int section, Qt::ButtonState state)
{
    emit q->clicked(model()->item(section), state);
}

// public

QHeaderWidget::QHeaderWidget(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setModel(new QHeaderModel(orientation, 0, this));
}

QHeaderWidget::~QHeaderWidget()
{
}

void QHeaderWidget::setSectionCount(int sections)
{
    d->model()->setSectionCount(sections);
}

QHeaderWidgetItem *QHeaderWidget::item(int section) const
{
    return d->model()->item(section);
}

void QHeaderWidget::setItem(int section, QHeaderWidgetItem *item)
{
    d->model()->setItem(section, item);
}

QHeaderWidgetItem *QHeaderWidget::takeItem(int section)
{
    return d->model()->takeItem(section);
}

void QHeaderWidget::clear()
{
    d->model()->clear();
}

void QHeaderWidget::removeItem(QHeaderWidgetItem *item)
{
    d->model()->removeItem(item);
}

void QHeaderWidget::setModel(QAbstractItemModel *model)
{
    QHeaderView::setModel(model);
}

#include "moc_qheaderwidget.cpp"

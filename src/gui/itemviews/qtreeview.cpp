#include "qtreeview.h"

/*!
  \class QTreeView qtreeview.h

  \brief Tree view implementation using the QTreeModel by default
*/


QTreeView::QTreeView(QWidget *parent)
    : QGenericTreeView(new QTreeModel, parent)
{
    model()->setParent(this); // make sure the model gets deleted
}

QTreeView::QTreeView(QTreeModel *model, QWidget *parent)
    : QGenericTreeView(model, parent)
{
}

void QTreeView::setColumnCount(int columns)
{
    model()->setColumnCount(columns);
}

int QTreeView::columnCount() const
{
    return model()->columnCount();
}
/*
void QTreeView::setText(const QModelIndex &item, int column, const QString &text)
{
    model()->setText(item, column, text);
}

void QTreeView::setIconSet(const QModelIndex &item, int column, const QIconSet &iconSet)
{
    model()->setIconSet(item, column, iconSet);
}

QString QTreeView::text(const QModelIndex &item, int column) const
{
    return model()->text(item, column);
}

QIconSet QTreeView::iconSet(const QModelIndex &item, int column) const
{
    return model()->iconSet(item, column);
}
*/
void QTreeView::setColumnText(int column, const QString &text)
{
    model()->setColumnText(column, text);
}

void QTreeView::setColumnIconSet(int column, const QIconSet &iconSet)
{
    model()->setColumnIconSet(column, iconSet);
}

QString QTreeView::columnText(int column) const
{
    return model()->columnText(column);
}

QIconSet QTreeView::columnIconSet(int column) const
{
    return model()->columnIconSet(column);
}

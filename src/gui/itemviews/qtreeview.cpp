#include "qtreeview.h"

/*!
  \class QTreeView qtreeview.h

  \brief Tree view implementation using the QTreeModel by default
*/


QTreeView::QTreeView(QWidget *parent, const char *name)
    : QGenericTreeView(new QTreeModel, parent, name)
{
    model()->setParent(this); // make sure the model gets deleted
}

QTreeView::QTreeView(QTreeModel *model, QWidget *parent, const char *name)
    : QGenericTreeView(model, parent, name)
{
}

void QTreeView::setColumnCount(int columns)
{
    tree()->setColumnCount(columns);
}

int QTreeView::columnCount() const
{
    return tree()->columnCount();
}
/*
void QTreeView::setText(const QModelIndex &item, int column, const QString &text)
{
    tree()->setText(item, column, text);
}

void QTreeView::setIconSet(const QModelIndex &item, int column, const QIconSet &iconSet)
{
    tree()->setIconSet(item, column, iconSet);
}

QString QTreeView::text(const QModelIndex &item, int column) const
{
    return tree()->text(item, column);
}

QIconSet QTreeView::iconSet(const QModelIndex &item, int column) const
{
    return tree()->iconSet(item, column);
}
*/
void QTreeView::setColumnText(int column, const QString &text)
{
    tree()->setColumnText(column, text);
}

void QTreeView::setColumnIconSet(int column, const QIconSet &iconSet)
{
    tree()->setColumnIconSet(column, iconSet);
}

QString QTreeView::columnText(int column) const
{
    return tree()->columnText(column);
}

QIconSet QTreeView::columnIconSet(int column) const
{
    return tree()->columnIconSet(column);
}

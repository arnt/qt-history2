#include "qtableview.h"
#include <qgenericheader.h>

/*!
  \class QTableView qtableview.h

  \brief Table view implementation using the QTableModel by default
*/

QTableView::QTableView(QWidget *parent, const char *name)
    : QGenericTableView(new QTableModel, parent, name)
{
    model()->setParent(this); // make sure the model gets deleted
}

QTableView::QTableView(QTableModel *model, QWidget *parent, const char *name)
    : QGenericTableView(model, parent, name)
{
}

QTableView::~QTableView()
{
}

void QTableView::setRowCount(int rows)
{
    table()->setRowCount(rows);
}

void QTableView::setColumnCount(int columns)
{
    table()->setColumnCount(columns);
}

int QTableView::rowCount() const
{
    return table()->rowCount();
}

int QTableView::columnCount() const
{
    return table()->columnCount();
}

void QTableView::setText(int row, int column, const QString &text)
{
    table()->setText(row, column, text);
}

void QTableView::setIconSet(int row, int column, const QIconSet &iconSet)
{
    table()->setIconSet(row, column, iconSet);
}

QString QTableView::text(int row, int column) const
{
    return table()->text(row, column);
}

QIconSet QTableView::iconSet(int row, int column) const
{
    return table()->iconSet(row, column);
}

void QTableView::setRowText(int row, const QString &text)
{
    table()->setRowText(row, text);
}

void QTableView::setRowIconSet(int row, const QIconSet &iconSet)
{
    table()->setRowIconSet(row, iconSet);
}

QString QTableView::rowText(int row) const
{
    return table()->rowText(row);
}

QIconSet QTableView::rowIconSet(int row) const
{
    return table()->rowIconSet(row);
}

void QTableView::setColumnText(int column, const QString &text)
{
    table()->setColumnText(column, text);
}

void QTableView::setColumnIconSet(int column, const QIconSet &iconSet)
{
    table()->setColumnIconSet(column, iconSet);
}

QString QTableView::columnText(int column) const
{
    return table()->columnText(column);
}

QIconSet QTableView::columnIconSet(int column) const
{
    return table()->columnIconSet(column);
}

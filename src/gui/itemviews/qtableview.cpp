#include "qtableview.h"
#include <qgenericheader.h>

/*!
  \class QTableView qtableview.h

  \brief Table view implementation using the QTableModel by default
*/

QTableView::QTableView(QWidget *parent)
    : QGenericTableView(new QTableModel, parent)
{
    model()->setParent(this); // make sure the model gets deleted
}

QTableView::QTableView(QTableModel *model, QWidget *parent)
    : QGenericTableView(model, parent)
{
}

QTableView::~QTableView()
{
}

void QTableView::setRowCount(int rows)
{
    model()->setRowCount(rows);
}

void QTableView::setColumnCount(int columns)
{
    model()->setColumnCount(columns);
}

int QTableView::rowCount() const
{
    return model()->rowCount();
}

int QTableView::columnCount() const
{
    return model()->columnCount();
}

void QTableView::setText(int row, int column, const QString &text)
{
    model()->setText(row, column, text);
}

void QTableView::setIconSet(int row, int column, const QIconSet &iconSet)
{
    model()->setIconSet(row, column, iconSet);
}

QString QTableView::text(int row, int column) const
{
    return model()->text(row, column);
}

QIconSet QTableView::iconSet(int row, int column) const
{
    return model()->iconSet(row, column);
}

void QTableView::setRowText(int row, const QString &text)
{
    model()->setRowText(row, text);
}

void QTableView::setRowIconSet(int row, const QIconSet &iconSet)
{
    model()->setRowIconSet(row, iconSet);
}

QString QTableView::rowText(int row) const
{
    return model()->rowText(row);
}

QIconSet QTableView::rowIconSet(int row) const
{
    return model()->rowIconSet(row);
}

void QTableView::setColumnText(int column, const QString &text)
{
    model()->setColumnText(column, text);
}

void QTableView::setColumnIconSet(int column, const QIconSet &iconSet)
{
    model()->setColumnIconSet(column, iconSet);
}

QString QTableView::columnText(int column) const
{
    return model()->columnText(column);
}

QIconSet QTableView::columnIconSet(int column) const
{
    return model()->columnIconSet(column);
}

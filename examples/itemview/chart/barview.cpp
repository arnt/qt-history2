/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
    barview.cpp

    Provides a view to represent a one-dimensional sequence of integers
    obtained from a list model as a series of rows.
*/

#include <QAbstractItemModel>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QItemSelection>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPoint>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSizePolicy>

#include "barview.h"

BarView::BarView(QWidget *parent, Qt::Orientation orient)
    : QAbstractItemView(parent)
{
    orientation = orient;
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

/*
    Returns the position of the item in viewport coordinates.
*/

QRect BarView::itemViewportRect(const QModelIndex &index) const
{
/*
    qDebug("View:\n%d,%d %d,%d", rect().x(), rect().x(), rect().width(),
        rect().height());
    qDebug("Viewport:\n%d,%d %d,%d\n", viewport()->rect().x(),
        viewport()->rect().x(), viewport()->rect().width(),
        viewport()->rect().height());
*/
    QRect rect = itemRect(index);

    if (rect.isValid())
        return QRect(viewport()->rect());
    else
        return rect;
}

/*
    Returns the rectangle of the item at position \a index in the
    model. The rectangle is in contents coordinates.
*/

QRect BarView::itemRect(const QModelIndex &index) const
{
    if (!index.isValid())
        return QRect();
    else {
        if (validValue(QModelIndex::Null, index.row())) {
            if (orientation == Qt::Vertical)
                return QRect(index.row(),0,1,1);
            else
                return QRect(0,index.row(),1,1);
        }
        else
            return QRect();
    }
}


void BarView::ensureItemVisible(const QModelIndex &index)
{
}

/*
    Returns the item that covers the coordinate given in the view.
*/

QModelIndex BarView::itemAt(int x, int /* y */) const
{
    return model()->index(0, 0, QModelIndex(), QModelIndex::View);
}

void BarView::dataChanged(const QModelIndex &/* topLeft */,
    const QModelIndex &/* bottomRight */)
{
    updateGeometries();
    if (isVisible())
        repaint();
}

void BarView::rowsInserted(const QModelIndex &/* parent */, int /* start */,
    int /* end */)
{
    updateGeometries();
    if (isVisible())
        repaint();
}

void BarView::rowsRemoved(const QModelIndex &/* parent */, int /* start */,
    int /* end */)
{
    updateGeometries();
    if (isVisible())
        repaint();
}

/*
    Find the indices corresponding to the horizontal extent of the selection.
*/

void BarView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    QModelIndex leftIndex = itemAt(rect.left(), 0);
    QModelIndex rightIndex = itemAt(rect.right(), 0);

    QItemSelection selection(leftIndex, rightIndex, model());

    selectionModel()->select(selection, command);
}

QModelIndex BarView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState)
{
    QModelIndex current = currentItem();

    switch (cursorAction) {
    case MoveLeft:{
        if (current.row() > 0)
            return model()->index(current.row() - 1, 0, QModelIndex(),
                QModelIndex::View);
        else
            return model()->index(0, 0, QModelIndex(), QModelIndex::View);
        break;}
    case MoveRight:{
        if (current.row() < rows(current) - 1)
            return model()->index(current.row() + 1, 0, QModelIndex(),
                QModelIndex::View);
        else
            return model()->index(rows(current) - 1, 0,QModelIndex(),
                QModelIndex::View);
        break;}
    case MoveUp:
        return current;
    case MoveDown:
        return current;
    case MovePageUp:
        return current;
    case MovePageDown:
        return current;
    case MoveHome:
        return model()->index(0, 0, QModelIndex(), QModelIndex::View);
    case MoveEnd:
        return model()->index(rows(current) - 1, 0, QModelIndex(),
            QModelIndex::View);
    default:
        return current;
    }
}

int BarView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int BarView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

/*
    Returns a rectangle corresponding to the selection in viewport cooridinates.
*/

QRect BarView::selectionViewportRect(const QItemSelection &selection) const
{
    int ranges = selection.count();

    if (ranges == 0)
        return QRect();

    // Note that we use the top and bottom functions of the selection range
    // since the data is stored in rows.

    int firstRow = selection.at(0).top();
    int lastRow = selection.at(0).top();

    for (int i = 0; i < ranges; ++i) {
        firstRow = qMin(firstRow, selection.at(i).top());
        lastRow = qMax(lastRow, selection.at(i).bottom());
    }

    QModelIndex firstItem = model()->index(qMin(firstRow, lastRow), 0,
        QModelIndex(), QModelIndex::View);
    QModelIndex lastItem = model()->index(qMax(firstRow, lastRow), 0,
        QModelIndex(), QModelIndex::View);

    QRect firstRect = itemViewportRect(firstItem);
    QRect lastRect = itemViewportRect(lastItem);

    return QRect(firstRect.left(), firstRect.top(),
        lastRect.right() - firstRect.left(), firstRect.height());
}

/*
    Render the data from the table model as a pie chart.
    We interpret the data in the following way:

    Column  0       1       2
            Title   Amount  Color
*/

void BarView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());

    QRect updateRect = event->rect();
    QBrush background(Qt::white);
    QPen foreground(Qt::black);

    painter.fillRect(updateRect, background);
    painter.setPen(foreground);

    int width = viewport()->rect().width();
    int height = viewport()->rect().height();
    int size = int(0.9 * qMin(3*width/4, height));
    QRect barRect = QRect(width/2 - size/2, height/2 - size/2, size, size);

    int validItems = validRows(QModelIndex::Null);
    double largest = 0.0;

    for (int row = 0; row < model()->rowCount(QModelIndex::Null); ++row) {
        QModelIndex index = model()->index(row, 1, QModelIndex::Null,
            QModelIndex::View);
        double value = model()->data(index,
            QAbstractItemModel::DisplayRole).toDouble();

        if (value > 0.0)
            largest = qMax(largest, value);
    }

    if (orientation == Qt::Vertical)
        painter.translate(-width/8, 0);
    else
        painter.translate(width/8, 0);

    if (orientation == Qt::Horizontal) {
        // Translate the origin to the point we want to rotate about.
        painter.translate(width/2, height/2);
        // Rotate the coordinate system.
        painter.rotate(90.0);
        // Translate eveything back into place.
        painter.translate(-width/2, -height/2);
    }

    if (validItems > 0) {

        int currentRow = 0;
        double barWidth = size/validItems;
        double scale;

        if (largest != 0.0)
            scale = size/largest;
        else
            scale = 0.0;

        for (int row = 0; row < model()->rowCount(QModelIndex::Null); ++row) {

            int barPosition = int(barRect.left() + (barWidth * currentRow));

            QModelIndex index = model()->index(row, 1, QModelIndex::Null,
                QModelIndex::View);

            double barHeight = scale * model()->data(index,
                QAbstractItemModel::DisplayRole).toDouble();

            if (int(barHeight) > 0) {

                QModelIndex colorIndex = model()->index(row, 2,
                    QModelIndex::Null, QModelIndex::View);
                QColor color = QColor(model()->data(colorIndex,
                    QAbstractItemModel::DisplayRole).toString());

                painter.setBrush(QBrush(color));

                painter.drawRect(int(barPosition),
                    int(barRect.bottom() - barHeight), int(barWidth),
                    int(barHeight));

                barPosition += barWidth;
            }
            currentRow++;
        }
    }

    if (orientation == Qt::Horizontal)
        painter.resetXForm();
/*
    if (validItems > 0) {
        QFont font = painter.font();
        int fontHeight = QFontMetrics(font).height();
        double itemHeight = qMin(size/validItems, fontHeight * 1.2);

        for (int row = 0; row < model()->rowCount(QModelIndex::Null); ++row) {
*/
}

void BarView::resizeEvent(QResizeEvent * /* event */)
{
    updateGeometries();
}

void BarView::updateGeometries()
{
    if (viewport()->width() < rows()) {
        horizontalScrollBar()->setPageStep(viewport()->width());
        horizontalScrollBar()->setRange(0, rows() - viewport()->width() - 1);
    }
}

QSize BarView::sizeHint() const
{
    return QSize(400, 400);
}

int BarView::rows(const QModelIndex &index) const
{
    return model()->rowCount(model()->parent(index));
}

/*
    Return the number of valid rows in the table.
*/

int BarView::validRows(const QModelIndex &parent) const
{
    int n = 0;
    for (int row = 0; row < model()->rowCount(parent); ++row) {
        if (validValue(parent, row))
            n++;
    }
    return n;
}

/*
    Check whether the data in the given row has a valid double value, and is
    greater than zero.
*/

bool BarView::validValue(const QModelIndex &parent, int row) const
{
    QModelIndex index = model()->index(row, 1, parent, QModelIndex::View);
    
    return (model()->data(index,
        QAbstractItemModel::DisplayRole).toDouble() >= 0.0);
}

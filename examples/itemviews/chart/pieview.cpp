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
    pieview.cpp

    Provides a view to represent a one-dimensional sequence of integers
    obtained from a list model as a series of rows.
*/

#include <math.h>
#include <QtGui>

#ifndef M_PI
#define M_PI 3.1415927
#endif

#include "pieview.h"

PieView::PieView(QWidget *parent)
    : QAbstractItemView(parent)
{
    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setRange(0, 0);

    margin = 8;
    totalSize = 300;
    pieSize = totalSize - 2*margin;
    validItems = 0;
    totalValue = 0.0;
}

bool PieView::isIndexHidden(const QModelIndex & /*index*/) const
{
    return false;
}

/*
    Returns the position of the item in viewport coordinates.
*/

QRect PieView::visualRect(const QModelIndex &index) const
{
    QRect rect = itemRect(index);
    if (rect.isValid())
        return QRect(rect.left() - horizontalScrollBar()->value(),
                     rect.top() - verticalScrollBar()->value(),
                     rect.width(), rect.height());
    else
        return rect;
}

/*
    Returns the rectangle of the item at position \a index in the
    model. The rectangle is in contents coordinates.
*/

QRect PieView::itemRect(const QModelIndex &index) const
{
    if (!index.isValid())
        return QRect();

    // Check whether the index's row is in the list of rows represented
    // by slices.
    QModelIndex valueIndex;

    if (index.column() != 1)
        valueIndex = model()->index(index.row(), 1, rootIndex());
    else
        valueIndex = index;

    if (model()->data(valueIndex).toDouble() > 0.0) {

        int listItem = 0;
        for (int row = index.row()-1; row >= 0; --row) {
            if (model()->data(model()->index(row, 1, rootIndex())).toDouble() > 0.0)
                listItem++;
        }

        double itemHeight;

        switch (index.column()) {
        case 0:
            itemHeight = QFontMetrics(viewOptions().font).height();

            return QRect(int(totalSize + itemHeight),
                         int(margin + listItem*itemHeight),
                         pieSize, int(itemHeight));
        case 1:
            return viewport()->rect();
        }

    }
    return QRect();
}

void PieView::scrollTo(const QModelIndex &index)
{
    QRect area = viewport()->rect();
    QRect rect = visualRect(index);

    if (rect.left() < area.left())
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() - rect.left());
    else if (rect.right() > area.right())
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() + rect.left() - area.width());

    if (rect.top() < area.top())
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() - rect.top());
    else if (rect.bottom() > area.bottom())
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() + rect.bottom() - area.height());
}

/*
    Returns the item that covers the coordinate given in the view.
*/

QModelIndex PieView::indexAt(const QPoint &point) const
{
    if (validItems == 0)
        return QModelIndex();

    // Transform the view coordinates into contents widget coordinates.
    int wx = point.x() + horizontalScrollBar()->value();
    int wy = point.y() + verticalScrollBar()->value();

    if (wx < totalSize) {
        double cx = wx - totalSize/2;
        double cy = totalSize/2 - wy;

        // Determine the distance from the center point of the pie chart.
        double d = pow(pow(cx, 2) + pow(cy, 2), 0.5);

        if (d == 0 || d > pieSize/2)
            return QModelIndex();

        // Determine the angle of the point.
        double angle = 180 * acos(cx/d) / M_PI;
        if (cy < 0)
            angle = 360 - angle;

        // Find the relevant slice of the pie.
        double startAngle = 0.0;

        for (int row = 0; row < model()->rowCount(rootIndex()); ++row) {

            QModelIndex index = model()->index(row, 1, rootIndex());
            double value = model()->data(index).toDouble();

            if (value > 0.0) {
                double sliceAngle = 360*value/totalValue;

                if (angle >= startAngle && angle < (startAngle + sliceAngle))
                    return model()->index(row, 0, rootIndex());

                startAngle += sliceAngle;
            }
        }
    } else {
        double itemHeight = QFontMetrics(viewOptions().font).height();
        int listItem = int((wy - margin) / itemHeight);
        int validRow = 0;

        for (int row = 0; row < model()->rowCount(rootIndex()); ++row) {

            QModelIndex index = model()->index(row, 1, rootIndex());
            if (model()->data(index).toDouble() > 0.0) {

                if (listItem == validRow)
                    return model()->index(row, 0, rootIndex());

                // Update the list index that corresponds to the next valid row.
                validRow++;
            }
        }
    }

    return QModelIndex();
}

void PieView::dataChanged(const QModelIndex &topLeft,
                          const QModelIndex &bottomRight)
{
    QAbstractItemView::dataChanged(topLeft, bottomRight);

    validItems = 0;
    totalValue = 0.0;

    for (int row = 0; row < model()->rowCount(rootIndex()); ++row) {

        QModelIndex index = model()->index(row, 1, rootIndex());
        double value = model()->data(index).toDouble();

        if (value > 0.0) {
            totalValue += value;
            validItems++;
        }
    }
}

void PieView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row <= end; ++row) {

        QModelIndex index = model()->index(row, 1, rootIndex());
        double value = model()->data(index).toDouble();

        if (value > 0.0) {
            totalValue += value;
            validItems++;
        }
    }

    QAbstractItemView::rowsInserted(parent, start, end);
}

void PieView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row <= end; ++row) {

        QModelIndex index = model()->index(row, 1, rootIndex());
        double value = model()->data(index).toDouble();
        if (value > 0.0) {
            totalValue -= value;
            validItems--;
        }
    }

    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

/*
    Find the indices corresponding to the extent of the selection.
*/

void PieView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    // Use viewport coordinates because the itemRect() function will transform
    // them into widget coordinates.
    QModelIndex firstIndex = indexAt(rect.topLeft());
    QModelIndex lastIndex = indexAt(rect.bottomRight());

    QItemSelection selection(firstIndex, lastIndex);

    selectionModel()->select(selection, command);
}

QModelIndex PieView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                Qt::KeyboardModifiers /*modifiers*/)
{
    QModelIndex current = currentIndex();

    switch (cursorAction) {
        case MoveLeft:
        case MoveUp:
            if (current.row() > 0)
                return model()->index(current.row() - 1, 0, rootIndex());
            else
                return model()->index(0, 0, rootIndex());
            break;
        case MoveRight:
        case MoveDown:
            if (current.row() < rows(current) - 1)
                return model()->index(current.row() + 1, 0, rootIndex());
            else
                return model()->index(rows(current) - 1, 0, rootIndex());
            break;
        default:
            return current;
    }
}

int PieView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int PieView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

/*
    Returns a rectangle corresponding to the selection in viewport coordinates.
*/

QRect PieView::visualRectForSelection(const QItemSelection &selection) const
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

    QModelIndex firstItem = model()->index(qMin(firstRow, lastRow), 0, rootIndex());
    QModelIndex lastItem = model()->index(qMax(firstRow, lastRow), 0, rootIndex());

    QRect firstRect = visualRect(firstItem);
    QRect lastRect = visualRect(lastItem);

    return firstRect.unite(lastRect);
}

void PieView::paintEvent(QPaintEvent *event)
{
    QItemSelectionModel *selections = selectionModel();
    QStyleOptionViewItem option = viewOptions();
    QStyle::State state = option.state;

    QBrush background = option.palette.base();
    QPen foreground(option.palette.color(QPalette::Foreground));
    QPen textPen(option.palette.color(QPalette::Text));
    QPen highlightedPen(option.palette.color(QPalette::HighlightedText));

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(event->rect(), background);
    painter.setPen(foreground);

    // Viewport rectangles
    QRect pieRect = QRect(margin, margin, pieSize, pieSize);
    QPoint keyPoint = QPoint(totalSize - horizontalScrollBar()->value(),
                             margin - verticalScrollBar()->value());

    if (validItems > 0) {

        painter.save();
        painter.translate(pieRect.x() - horizontalScrollBar()->value(),
                          pieRect.y() - verticalScrollBar()->value());
        painter.drawEllipse(0, 0, pieSize, pieSize);
        double startAngle = 0.0;
        int row;

        for (row = 0; row < model()->rowCount(rootIndex()); ++row) {

            QModelIndex index = model()->index(row, 1, rootIndex());
            double value = model()->data(index).toDouble();

            if (value > 0.0) {
                double angle = 360*value/totalValue;

                QModelIndex colorIndex = model()->index(row, 0, rootIndex());
                QColor color = QColor(model()->data(colorIndex,
                    QAbstractItemModel::DecorationRole).toString());

                painter.setBrush(QBrush(color));
                painter.drawPie(0, 0, pieSize, pieSize, int(startAngle*16),
                                int(angle*16));

                startAngle += angle;
            }
        }
        painter.restore();

        int keyNumber = 0;
        painter.setFont(viewOptions().font);
        double itemHeight = QFontMetrics(viewOptions().font).height();
        double fontHeight = QFontMetrics(viewOptions().font).ascent();
        QModelIndex current = currentIndex();

        painter.save();

        for (row = 0; row < model()->rowCount(rootIndex()); ++row) {

            QModelIndex index = model()->index(row, 1, rootIndex());
            double value = model()->data(index).toDouble();

            if (value > 0.0) {
                QModelIndex labelIndex = model()->index(row, 0, rootIndex());

                QString label = model()->data(labelIndex).toString();
                QColor color = QColor(model()->data(labelIndex,
                    QAbstractItemModel::DecorationRole).toString());

                painter.setBrush(QBrush(color));
                painter.setPen(textPen);

                painter.save();
                painter.translate(keyPoint);
                QRectF thisItemRect(0, keyNumber*itemHeight,
                    itemHeight * 0.8, itemHeight * 0.9);
                painter.drawRect(thisItemRect);
                painter.restore();

                QRect textRect = itemRect(labelIndex);
                textRect.translate(-horizontalScrollBar()->value(),
                                   -verticalScrollBar()->value());

                if (labelIndex == current) {
                    painter.fillRect(textRect, option.palette.highlight());
                    painter.setPen(highlightedPen);
                }
                else if (selections->isSelected(labelIndex)) {
                    painter.fillRect(textRect, option.palette.highlight());
                    painter.setPen(highlightedPen);
                } else {
                    painter.fillRect(textRect, background);
                    painter.setPen(textPen);
                }

                QPointF textPosition(textRect.x(), textRect.y() + fontHeight);

                painter.drawText(textPosition, label);
                keyNumber++;
            }
        }

        painter.restore();
    }
}

int PieView::rows(const QModelIndex &index) const
{
    return model()->rowCount(model()->parent(index));
}

void PieView::resizeEvent(QResizeEvent * /* event */)
{
    updateGeometries();
}

void PieView::updateGeometries()
{
    horizontalScrollBar()->setPageStep(viewport()->width());
    horizontalScrollBar()->setRange(0, qMax(0, 2*totalSize - viewport()->width()));
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0, qMax(0, totalSize - viewport()->height()));
}

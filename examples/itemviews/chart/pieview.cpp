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

#include "pieview.h"

PieView::PieView(QWidget *parent)
    : QAbstractItemView(parent)
{
    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setRange(0, 0);

    m_margin = 8;
    m_size = 300;
    m_pieSize = m_size - 2*m_margin;
    m_validItems = 0;
    m_totalValue = 0.0;
}

bool PieView::isIndexHidden(const QModelIndex & /*index*/) const
{
    return false;
}

/*
    Returns the position of the item in viewport coordinates.
*/

QRect PieView::viewportRectForIndex(const QModelIndex &index) const
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

            return QRect(int(m_size + itemHeight),
                         int(m_margin + listItem*itemHeight),
                         m_pieSize, int(itemHeight));
        case 1:
            return viewport()->rect();
        case 2:
            itemHeight = QFontMetrics(viewOptions().font).height();

            return QRect(m_size, int(m_margin + listItem*itemHeight),
                         int(itemHeight),
                         int(itemHeight)).unite(QRect(m_margin, m_margin,
                         m_pieSize, m_pieSize));
        }

    }
    return QRect();
}

void PieView::ensureVisible(const QModelIndex &index)
{
    QRect area = viewport()->rect();
    QRect rect = viewportRectForIndex(index);

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

QModelIndex PieView::indexAt(int x, int y) const
{
    if (m_validItems == 0)
        return QModelIndex();

    // Transform the view coordinates into contents widget coordinates.
    int wx = x + horizontalScrollBar()->value();
    int wy = y + verticalScrollBar()->value();

    if (wx < m_size) {
        double cx = wx - m_size/2;
        double cy = m_size/2 - wy;

        // Determine the distance from the center point of the pie chart.
        double d = pow(pow(cx, 2) + pow(cy, 2), 0.5);

        if (d == 0 || d > m_pieSize/2)
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
                double sliceAngle = 360*value/m_totalValue;

                if (angle >= startAngle && angle < (startAngle + sliceAngle))
                    return model()->index(row, 0, rootIndex());

                startAngle += sliceAngle;
            }
        }
    } else {
        double itemHeight = QFontMetrics(viewOptions().font).height();
        int listItem = int((wy - m_margin) / itemHeight);
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

    m_validItems = 0;
    m_totalValue = 0.0;

    for (int row = 0; row < model()->rowCount(rootIndex()); ++row) {

        QModelIndex index = model()->index(row, 1, rootIndex());
        double value = model()->data(index).toDouble();

        if (value > 0.0) {
            m_totalValue += value;
            m_validItems++;
        }
    }
}

void PieView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row <= end; ++row) {

        QModelIndex index = model()->index(row, 1, rootIndex());
        double value = model()->data(index).toDouble();

        if (value > 0.0) {
            m_totalValue += value;
            m_validItems++;
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
            m_totalValue -= value;
            m_validItems--;
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
    QModelIndex firstIndex = indexAt(rect.left(), rect.top());
    QModelIndex lastIndex = indexAt(rect.right(), rect.bottom());

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

QRect PieView::selectionViewportRect(const QItemSelection &selection) const
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

    QRect firstRect = viewportRectForIndex(firstItem);
    QRect lastRect = viewportRectForIndex(lastItem);

    return firstRect.unite(lastRect);
}

/*
    Render the data from the table model as a pie chart.
    We interpret the data in the following way:

    Column  0       1       2
            Title   Amount  Color

    The figure is always drawn with the chart on the left and the key on
    the right. This means that we must try and obtain an area that is wider
    than it is tall. We do this by imposing a particular aspect ratio on
    the chart and applying it to the available vertical space. This ensures
    that we always obtain the maximum horizontal space for the aspect ratio
    used.
    We also apply fixed size margin around the figure.

    We use logical coordinates to draw the chart and key, and position them
    on the view using viewports.
*/

void PieView::paintEvent(QPaintEvent *event)
{
    QItemSelectionModel *selections = selectionModel();
    QStyleOptionViewItem option = viewOptions();
    QStyle::StyleFlags state = option.state;

    QBrush background = option.palette.base();
    QPen foreground(option.palette.color(QPalette::Foreground));
    QPen textPen(option.palette.color(QPalette::Text));
    QPen highlightedPen(option.palette.color(QPalette::HighlightedText));

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(event->rect(), background);
    painter.setPen(foreground);

    // Viewport rectangles
    QRect pieRect = QRect(m_margin, m_margin, m_pieSize, m_pieSize);
    QPoint keyPoint = QPoint(m_size - horizontalScrollBar()->value(),
                             m_margin - verticalScrollBar()->value());

    if (m_validItems > 0) {

        painter.save();
        painter.translate(pieRect.x() - horizontalScrollBar()->value(),
                          pieRect.y() - verticalScrollBar()->value());
        painter.drawEllipse(0, 0, m_pieSize, m_pieSize);
        double startAngle = 0.0;
        int row;

        for (row = 0; row < model()->rowCount(rootIndex()); ++row) {

            QModelIndex index = model()->index(row, 1, rootIndex());
            double value = model()->data(index).toDouble();

            if (value > 0.0) {
                double angle = 360*value/m_totalValue;

                QModelIndex colorIndex = model()->index(row, 2, rootIndex());
                QColor color = QColor(model()->data(colorIndex).toString());

                painter.setBrush(QBrush(color));
                painter.drawPie(0, 0, m_pieSize, m_pieSize, int(startAngle*16),
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
                QModelIndex colorIndex = model()->index(row, 2, rootIndex());

                QString label = model()->data(labelIndex).toString();
                QColor color = QColor(model()->data(colorIndex).toString());

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

                painter.drawText(textPosition, label, QPainter::Auto);
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
    horizontalScrollBar()->setRange(0, qMax(0, 2*m_size - viewport()->width()));
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0, qMax(0, m_size - viewport()->height()));
}

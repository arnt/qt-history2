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

#include "pieview.h"

PieView::PieView(QWidget *parent)
    : QAbstractItemView(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

/*
    Returns the position of the item in viewport coordinates.
*/

QRect PieView::itemViewportRect(const QModelIndex &index) const
{
    QRect rect = itemRect(index);
    if (rect.isValid())
        return QRect(rect.left(), rect.top(), viewport()->width(),
                      viewport()->height());
    else
        return rect;
}

/*
    Returns the rectangle of the item at position \a index in the
    model. The rectangle is in contents coordinates.
*/

QRect PieView::itemRect(const QModelIndex &index) const
{
    //if (!index.isValid())
    //    return QRect();
    //else
    //    return QRect(0,0,1,1);
    return QRect();
}


void PieView::ensureItemVisible(const QModelIndex & /* index */)
{
}

/*
    Returns the item that covers the coordinate given in the view.
*/

QModelIndex PieView::itemAt(int x, int /* y */) const
{
    return model()->index(0, 0, QModelIndex(), QModelIndex::View);
}

void PieView::dataChanged(const QModelIndex &/* topLeft */,
    const QModelIndex &/* bottomRight */)
{
    if (isVisible())
        repaint();
}

void PieView::rowsInserted(const QModelIndex &/* parent */, int /* start */,
    int /* end */)
{
    if (isVisible())
        repaint();
}

void PieView::rowsRemoved(const QModelIndex &/* parent */, int /* start */,
    int /* end */)
{
    if (isVisible())
        repaint();
}
/*
void PieView::verticalScrollbarAction(int action)
{
}

void PieView::horizontalScrollbarAction(int action)
{
}
*/

/*
    Find the indices corresponding to the horizontal extent of the selection.
*/

void PieView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    QModelIndex leftIndex = itemAt(rect.left(), 0);
    QModelIndex rightIndex = itemAt(rect.right(), 0);

    QItemSelection selection(leftIndex, rightIndex, model());

    selectionModel()->select(selection, command);
}

QModelIndex PieView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState)
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

int PieView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int PieView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

/*
    Returns a rectangle corresponding to the selection in viewport cooridinates.
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
    QPainter painter(viewport());

    QRect updateRect = event->rect();
    QBrush background(Qt::white);
    QPen foreground(Qt::black);

    painter.fillRect(updateRect, background);
    painter.setPen(foreground);

    double total = 0.0;
    int validItems = 0;

    for (int row = 0; row < model()->rowCount(QModelIndex::Null); ++row) {
        QModelIndex index = model()->index(row, 1, QModelIndex::Null,
            QModelIndex::View);
        double value = model()->data(index,
            QAbstractItemModel::DisplayRole).toDouble();

        if (value > 0.0) {
            total += value;
            validItems++;
        }
    }

    int width = viewport()->rect().width();
    int height = viewport()->rect().height();
    double aspect = 0.5;
    int margin = 8;
    int vsize = int(qMin(aspect*(width-2*margin), height-2*margin));
    int hsize = int(vsize/aspect);

    // Viewport rectangles
    QRect pieRect = QRect(margin, height/2 - vsize/2, hsize/2-margin, vsize);
    QRect keyRect = QRect(margin+hsize/2, height/2 - vsize/2, hsize/2, vsize);

    painter.setWindow(-50, -50, 100, 100);
    painter.setViewport(pieRect);
    painter.drawEllipse(-50, -50, 100, 100);

    if (validItems > 0) {

        double currentTotal = 0.0;
        int currentKeyItem = 0;
        QFont font = painter.font();
        double itemHeight = 100.0/validItems;
        double fontHeight = itemHeight/1.2;
        font.setPixelSize(int(fontHeight));

        for (int row = 0; row < model()->rowCount(QModelIndex::Null); ++row) {

            int angle = int(360*currentTotal*16/total);

            QModelIndex index = model()->index(row, 1, QModelIndex::Null,
                QModelIndex::View);

            double amount = model()->data(index,
                QAbstractItemModel::DisplayRole).toDouble();

            if (int(amount) > 0) {
                QModelIndex colorIndex = model()->index(row, 2,
                    QModelIndex::Null, QModelIndex::View);
                QColor color = QColor(model()->data(colorIndex,
                    QAbstractItemModel::DisplayRole).toString());

                painter.setWindow(-50, -50, 100, 100);
                painter.setViewport(pieRect);

                painter.setBrush(QBrush(color));
                painter.drawPie(-50, -50, 100, 100, angle,
                                int(360*amount*16/total));

                QModelIndex labelIndex = model()->index(row, 0,
                    QModelIndex::Null, QModelIndex::View);

                QString label = model()->data(labelIndex,
                    QAbstractItemModel::DisplayRole).toString();

                painter.setWindow(0, 0, 100, 100);
                painter.setViewport(keyRect);

                painter.drawRect(0, (currentKeyItem + 0.1)*itemHeight,
                    int(itemHeight * 0.8), int(itemHeight * 0.9));
                painter.drawText(itemHeight,
                    int((currentKeyItem*itemHeight)+fontHeight),
                    label);

                currentTotal += amount;
                currentKeyItem++;
            }
        }
    }
}

QSize PieView::sizeHint() const
{
    return QSize(400, 400);
}

int PieView::rows(const QModelIndex &index) const
{
    return model()->rowCount(model()->parent(index));
}

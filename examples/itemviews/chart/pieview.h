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

#ifndef PIEVIEW_H
#define PIEVIEW_H

#include <QAbstractItemView>
#include <QFont>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QRect>
#include <QSize>
#include <QWidget>

class PieView : public QAbstractItemView
{
    Q_OBJECT

public:
    PieView(QWidget *parent = 0);

    QRect itemViewportRect(const QModelIndex &index) const;
    void ensureVisible(const QModelIndex &index);
    QModelIndex indexAt(int x, int y) const;

protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

protected:
    bool isIndexHidden(const QModelIndex &index) const;
    void setSelection(const QRect&, QItemSelectionModel::SelectionFlags command);
    QRect selectionViewportRect(const QItemSelection &selection) const;
    QRect itemRect(const QModelIndex &item) const;
    int horizontalOffset() const;
    int verticalOffset() const;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
                           Qt::KeyboardModifiers modifiers);

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    int rows(const QModelIndex &index = QModelIndex()) const;
    void updateGeometries();

    int m_margin;
    int m_size;
    int m_pieSize;
    int m_validItems;
    double m_totalValue;
};

#endif

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGENERICTABLEVIEW_H
#define QGENERICTABLEVIEW_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QGenericHeader;
class QGenericTableViewPrivate;

class Q_GUI_EXPORT QGenericTableView : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGenericTableView)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid)

public:
    QGenericTableView(QWidget *parent = 0);
    ~QGenericTableView();

    void setModel(QAbstractItemModel *model);
    void setSelectionModel(QItemSelectionModel *selectionModel);

    QGenericHeader *horizontalHeader() const;
    QGenericHeader *verticalHeader() const;
    void setHorizontalHeader(QGenericHeader *header);
    void setVerticalHeader(QGenericHeader *header);

    int rowViewportPosition(int row) const;
    int rowHeight(int row) const;
    int rowAt(int y) const;
    int columnViewportPosition(int column) const;
    int columnWidth(int column) const;
    int columnAt(int x) const;
    bool isRowHidden(int row) const;
    bool isColumnHidden(int column) const;
    bool showGrid() const;

    QRect itemViewportRect(const QModelIndex &item) const;
    void ensureItemVisible(const QModelIndex &item);
    QModelIndex itemAt(int x, int y) const;

    QSize sizeHint() const;
    
public slots:
    void setShowGrid(bool show);
    void setGridStyle(Qt::PenStyle style);
    void selectRow(int row, Qt::ButtonState state = Qt::NoButton);
    void selectColumn(int column, Qt::ButtonState state = Qt::NoButton);
    void hideRow(int row);
    void hideColumn(int column);
    void showRow(int row);
    void showColumn(int column);
    void resizeRowToContents(int row, bool checkHeader = true);
    void resizeColumnToContents(int column, bool checkHeader = true);

protected slots:
    void rowIndexChanged(int row, int oldIndex, int newIndex);
    void columnIndexChanged(int column, int oldIndex, int newIndex);
    void rowHeightChanged(int row, int oldHeight, int newHeight);
    void columnWidthChanged(int column, int oldWidth, int newWidth);
    void rowCountChanged(int oldCount, int newCount);
    void columnCountChanged(int oldCount, int newCount);

protected:
    QGenericTableView(QGenericTableViewPrivate &, QWidget *parent = 0);
    void scrollContentsBy(int dx, int dy);

    void paintEvent(QPaintEvent *e);

    int horizontalOffset() const;
    int verticalOffset() const;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state);

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    QRect selectionViewportRect(const QItemSelection &selection) const;

    void updateGeometries();

    int rowSizeHint(int row) const;
    int columnSizeHint(int column) const;

    void verticalScrollbarAction(int action);
    void horizontalScrollbarAction(int action);
};

#endif

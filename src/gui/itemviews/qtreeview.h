/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTREEVIEW_H
#define QTREEVIEW_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QTreeViewPrivate;
class QHeaderView;

class Q_GUI_EXPORT QTreeView : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTreeView)
    Q_PROPERTY(bool rootIsDecorated READ rootIsDecorated WRITE setRootIsDecorated)
    Q_PROPERTY(int indentation READ indentation WRITE setIndentation)

public:
    QTreeView(QWidget *parent = 0);
    ~QTreeView();

    void setModel(QAbstractItemModel *model);
    void setSelectionModel(QItemSelectionModel *selectionModel);

    QHeaderView *header() const;
    void setHeader(QHeaderView *header);

    int indentation() const;
    void setIndentation(int i);

    bool rootIsDecorated() const;
    void setRootIsDecorated(bool show);

    int columnViewportPosition(int column) const;
    int columnWidth(int column) const;
    int columnAt(int x) const;

    bool isColumnHidden(int column) const;
    bool isOpen(const QModelIndex &index) const;

    QRect itemViewportRect(const QModelIndex &index) const;
    void ensureItemVisible(const QModelIndex &index);
    QModelIndex itemAt(int x, int y) const;
    QModelIndex itemAbove(const QModelIndex &index) const;
    QModelIndex itemBelow(const QModelIndex &index) const;

    void doItemsLayout();
    void reset();

signals:
    void expanded(const QModelIndex &index);
    void collapsed(const QModelIndex &index);

public slots:
    void hideColumn(int column);
    void showColumn(int column);
    void open(const QModelIndex &index);
    void close(const QModelIndex &index);

protected slots:
    void resizeColumnToContents(int column, bool checkHeader = true);
    void columnWidthChanged(int column, int oldSize, int newSize);
    void columnCountChanged(int oldCount, int newCount);
    void dataChanged();

protected:
    QTreeView(QTreeViewPrivate &dd, QWidget *parent = 0);
    void scrollContentsBy(int dx, int dy);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);

    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state);
    int horizontalOffset() const;
    int verticalOffset() const;

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    QRect selectionViewportRect(const QItemSelection &selection) const;

    void paintEvent(QPaintEvent *e);
    virtual void drawRow(QPainter *painter, const QStyleOptionViewItem &options,
                         const QModelIndex &index) const;
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;

    void mousePressEvent(QMouseEvent *e);

    void updateGeometries();
    void verticalScrollbarAction(int action);
    void horizontalScrollbarAction(int action);

    int columnSizeHint(int column) const;
};

#endif

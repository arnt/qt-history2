/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTREEVIEW_H
#define QTREEVIEW_H

#include <QtGui/qabstractitemview.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_TREEVIEW

class QTreeViewPrivate;
class QHeaderView;

class Q_GUI_EXPORT QTreeView : public QAbstractItemView
{
    Q_OBJECT
    Q_PROPERTY(int indentation READ indentation WRITE setIndentation)
    Q_PROPERTY(bool rootIsDecorated READ rootIsDecorated WRITE setRootIsDecorated)
    Q_PROPERTY(bool uniformRowHeights READ uniformRowHeights WRITE setUniformRowHeights)
    Q_PROPERTY(bool itemsExpandable READ itemsExpandable WRITE setItemsExpandable)
    Q_PROPERTY(bool sortingEnabled READ isSortingEnabled WRITE setSortingEnabled)
    Q_PROPERTY(bool animated READ isAnimated WRITE setAnimated)
    Q_PROPERTY(bool allColumnsShowFocus READ allColumnsShowFocus WRITE setAllColumnsShowFocus)
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap)

public:
    explicit QTreeView(QWidget *parent = 0);
    ~QTreeView();

    void setModel(QAbstractItemModel *model);
    void setRootIndex(const QModelIndex &index);
    void setSelectionModel(QItemSelectionModel *selectionModel);

    QHeaderView *header() const;
    void setHeader(QHeaderView *header);

    int indentation() const;
    void setIndentation(int i);

    bool rootIsDecorated() const;
    void setRootIsDecorated(bool show);

    bool uniformRowHeights() const;
    void setUniformRowHeights(bool uniform);

    bool itemsExpandable() const;
    void setItemsExpandable(bool enable);

    int columnViewportPosition(int column) const;
    int columnWidth(int column) const;
    void setColumnWidth(int column, int width);
    int columnAt(int x) const;

    bool isColumnHidden(int column) const;
    void setColumnHidden(int column, bool hide);

    bool isRowHidden(int row, const QModelIndex &parent) const;
    void setRowHidden(int row, const QModelIndex &parent, bool hide);

    bool isRowSpanning(int row, const QModelIndex &parent) const;
    void setRowSpanning(int row, const QModelIndex &parent, bool span);

    bool isExpanded(const QModelIndex &index) const;
    void setExpanded(const QModelIndex &index, bool expand);

    void setSortingEnabled(bool enable);
    bool isSortingEnabled() const;

    void setAnimated(bool enable);
    bool isAnimated() const;

    void setAllColumnsShowFocus(bool enable);
    bool allColumnsShowFocus() const;

    void setWordWrap(bool on);
    bool wordWrap() const;

    void keyboardSearch(const QString &search);

    QRect visualRect(const QModelIndex &index) const;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
    QModelIndex indexAt(const QPoint &p) const;
    QModelIndex indexAbove(const QModelIndex &index) const;
    QModelIndex indexBelow(const QModelIndex &index) const;

    void doItemsLayout();
    void reset();

    void sortByColumn(int column, Qt::SortOrder order);

Q_SIGNALS:
    void expanded(const QModelIndex &index);
    void collapsed(const QModelIndex &index);

public Q_SLOTS:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void hideColumn(int column);
    void showColumn(int column);
    void expand(const QModelIndex &index);
    void collapse(const QModelIndex &index);
    void resizeColumnToContents(int column);
    void sortByColumn(int column);
    void selectAll();
    void expandAll();
    void collapseAll();

protected Q_SLOTS:
    void columnResized(int column, int oldSize, int newSize);
    void columnCountChanged(int oldCount, int newCount);
    void columnMoved();
    void reexpand();
    void rowsRemoved(const QModelIndex &parent, int first, int last);

protected:
    QTreeView(QTreeViewPrivate &dd, QWidget *parent = 0);
    void scrollContentsBy(int dx, int dy);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    int horizontalOffset() const;
    int verticalOffset() const;

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    QRegion visualRegionForSelection(const QItemSelection &selection) const;
    QModelIndexList selectedIndexes() const;

    void timerEvent(QTimerEvent *event);
    void paintEvent(QPaintEvent *event);

    void drawTree(QPainter *painter, const QRegion &region) const;
    virtual void drawRow(QPainter *painter,
                         const QStyleOptionViewItem &options,
                         const QModelIndex &index) const;
    virtual void drawBranches(QPainter *painter,
                              const QRect &rect,
                              const QModelIndex &index) const;

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

    void updateGeometries();

    int sizeHintForColumn(int column) const;
    int indexRowSizeHint(const QModelIndex &index) const;

    void horizontalScrollbarAction(int action);

    bool isIndexHidden(const QModelIndex &index) const;

private:
    Q_DECLARE_PRIVATE(QTreeView)
    Q_DISABLE_COPY(QTreeView)
    Q_PRIVATE_SLOT(d_func(), void _q_endAnimatedOperation())
    Q_PRIVATE_SLOT(d_func(), void _q_currentChanged(const QModelIndex&, const QModelIndex &))
};

#endif // QT_NO_TREEVIEW

QT_END_HEADER

#endif // QTREEVIEW_H

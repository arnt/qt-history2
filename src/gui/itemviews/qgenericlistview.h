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

#ifndef QGENERICLISTVIEW_H
#define QGENERICLISTVIEW_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QGenericListViewItem;
class QGenericListViewPrivate;

class Q_GUI_EXPORT QGenericListView : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGenericListView)

public:
    enum Flow { LeftToRight, TopToBottom };
    enum Movement { Static, Free, Snap };
    enum Size { Automatic, Small, Large };
    enum ResizeMode { Fixed, Adjust };
    enum LayoutMode { Instant, Delayed };

    QGenericListView(QAbstractItemModel *model, QWidget *parent = 0);
    ~QGenericListView();

    void setMovement(Movement movement);
    Movement movement() const;

    void setFlow(Flow flow);
    QGenericListView::Flow flow() const;

    void setWrapping(bool enable);
    bool isWrapping() const;

    void setIconSize(Size size);
    QGenericListView::Size iconSize() const;

    void setResizeMode(ResizeMode mode);
    QGenericListView::ResizeMode resizeMode() const;

    void setLayoutMode(LayoutMode mode);
    QGenericListView::LayoutMode layoutMode() const;

    void setSpacing(int space);
    int spacing() const;

    void setGridSize(const QSize &size);
    QSize gridSize() const;

    QRect itemViewportRect(const QModelIndex &item) const;
    void ensureItemVisible(const QModelIndex &item);
    QModelIndex itemAt(int x, int y) const;

    void doItemsLayout();

protected:
    QGenericListView(QGenericListViewPrivate &, QAbstractItemModel *model, QWidget *parent = 0);
    void scrollContentsBy(int dx, int dy);
    void resizeContents(int w, int h);

    void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsRemoved(const QModelIndex &parent,
                         const QModelIndex &topLeft, const QModelIndex &bottomRight);

    bool doItemsLayout(int num);
    void doItemsLayout(const QRect &bounds, const QModelIndex &first, const QModelIndex &last);
    
    void doStaticLayout(const QRect &bounds, int first, int last);
    void doDynamicLayout(const QRect &bounds, int first, int last);

    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
        
    void timerEvent(QTimerEvent *e);
    void resizeEvent(QResizeEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);

    QDragObject *dragObject();
    void startDrag();

    void getViewOptions(QItemOptions *options) const;
    void paintEvent(QPaintEvent *e);

    int horizontalOffset() const;
    int verticalOffset() const;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state);
    QRect itemRect(const QModelIndex &item) const;

    void setSelection(const QRect &rect, int selectionCommand);
    QRect selectionViewportRect(const QItemSelection &selection) const;

    bool supportsDragAndDrop() const;

    int itemIndex(QGenericListViewItem *item) const;
    void insertItem(int index, QGenericListViewItem &item);
    void removeItem(int index);
    void moveItem(int index, const QPoint &dest);

    void updateGeometries();
};

#endif /* QGENERICLISTVIEW_H */

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
    Q_ENUMS(Movement Flow IconSize ResizeMode LayoutMode)
    Q_PROPERTY(Movement movement READ movement WRITE setMovement)
    Q_PROPERTY(Flow flow READ flow WRITE setFlow)
    Q_PROPERTY(bool isWrapping READ isWrapping WRITE setWrapping)
    Q_PROPERTY(IconSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode)
    Q_PROPERTY(LayoutMode layoutMode READ layoutMode WRITE setLayoutMode)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(QSize gridSize READ gridSize WRITE setGridSize)
        
public:
    enum Movement { Static, Free, Snap };
    enum Flow { LeftToRight, TopToBottom };
    enum IconSize { Automatic, Small, Large };
    enum ResizeMode { Fixed, Adjust };
    enum LayoutMode { Instant, Delayed };

    QGenericListView(QAbstractItemModel *model, QWidget *parent = 0);
    ~QGenericListView();

    void setMovement(Movement movement);
    Movement movement() const;

    void setFlow(Flow flow);
    Flow flow() const;

    void setWrapping(bool enable);
    bool isWrapping() const;

    void setIconSize(IconSize size);
    IconSize iconSize() const;

    void setResizeMode(ResizeMode mode);
    ResizeMode resizeMode() const;

    void setLayoutMode(LayoutMode mode);
    LayoutMode layoutMode() const;

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

    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);

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

    QStyleOptionViewItem viewOptions() const;
    void paintEvent(QPaintEvent *e);

    int horizontalOffset() const;
    int verticalOffset() const;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state);
    QRect itemRect(const QModelIndex &item) const;

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    QRect selectionViewportRect(const QItemSelection &selection) const;

    bool supportsDragAndDrop() const;

    int itemIndex(QGenericListViewItem *item) const;
    void insertItem(int index, QGenericListViewItem &item);
    void removeItem(int index);
    void moveItem(int index, const QPoint &dest);

    void updateGeometries();
};

#endif

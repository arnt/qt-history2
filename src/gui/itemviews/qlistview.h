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

#ifndef QLISTVIEW_H
#define QLISTVIEW_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QListViewPrivate;

class Q_GUI_EXPORT QListView : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QListView)
    Q_ENUMS(Movement Flow IconSize ResizeMode LayoutMode ViewMode)
    Q_PROPERTY(Movement movement READ movement WRITE setMovement)
    Q_PROPERTY(Flow flow READ flow WRITE setFlow)
    Q_PROPERTY(bool isWrapping READ isWrapping WRITE setWrapping)
    Q_PROPERTY(IconSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode)
    Q_PROPERTY(LayoutMode layoutMode READ layoutMode WRITE setLayoutMode)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(QSize gridSize READ gridSize WRITE setGridSize)
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode)

public:
    enum Movement { Static, Free, Snap };
    enum Flow { LeftToRight, TopToBottom };
    enum IconSize { Automatic, Small, Large };
    enum ResizeMode { Fixed, Adjust };
    enum LayoutMode { SinglePass, Batched };
    enum ViewMode { ListMode, IconMode };

    QListView(QWidget *parent = 0);
    ~QListView();

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
    
    void setViewMode(ViewMode mode);
    ViewMode viewMode() const;

    void clearPropertyFlags();

    QRect itemViewportRect(const QModelIndex &item) const;
    void ensureItemVisible(const QModelIndex &item);
    QModelIndex itemAt(int x, int y) const;

    void doItemsLayout();
    void reset();

protected:
    QListView(QListViewPrivate &, QWidget *parent = 0);

    void scrollContentsBy(int dx, int dy);
    void resizeContents(int width, int height);

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
    bool isDragEnabled(const QModelIndex &) const;

    QStyleOptionViewItem viewOptions() const;
    void paintEvent(QPaintEvent *e);

    int horizontalOffset() const;
    int verticalOffset() const;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state);
    QRect itemRect(const QModelIndex &item) const;

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    QRect selectionViewportRect(const QItemSelection &selection) const;

    void updateGeometries();
};

#endif

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
    Q_DECLARE_PRIVATE(QGenericListView);

public:
    enum Flow { TopToBottom = 0, LeftToRight = 1 };
    enum Wrap { Off = 0, On = 1 };
    enum Movement { Static, Free, Snap };

    QGenericListView(QAbstractItemModel *model, QWidget *parent = 0);
    ~QGenericListView();

    void setMovement(Movement movement);
    void setFlow(Flow flow);
    void setWrapping(Wrap wrap);
    void setSpacing(int space);
    void setGridSize(const QSize &size);
    void setSelection(const QRect &rect, int selectionCommand);

protected:
    QGenericListView(QGenericListViewPrivate &, QAbstractItemModel *model, QWidget *parent = 0);
    void scrollContentsBy(int dx, int dy);
    void resizeContents(int w, int h);

    void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsRemoved(const QModelIndex &parent, const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void startItemsLayout();
    bool doItemsLayout(int num);
    void stopItemsLayout();

    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);

    QDragObject *dragObject();
    void startDrag();

    void getViewOptions(QItemOptions *options) const;
    void paintEvent(QPaintEvent *e);

    QModelIndex itemAt(int x, int y) const;
    int horizontalOffset() const;
    int verticalOffset() const;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state);
    QRect itemRect(const QModelIndex &item) const;
    QRect itemViewportRect(const QModelIndex &item) const;
    void ensureItemVisible(const QModelIndex &item);

    QRect selectionViewportRect(const QItemSelection &selection) const;

    void doItemsLayout(const QRect &bounds, const QModelIndex &first, const QModelIndex &last);
    void doStaticLayout(const QRect &bounds, int first, int last);
    void doDynamicLayout(const QRect &bounds, int first, int last);

    bool supportsDragAndDrop() const;

    int itemIndex(QGenericListViewItem *item) const;
    void insertItem(int index, QGenericListViewItem &item);
    void removeItem(int index);
    void moveItem(int index, const QPoint &dest);

    void updateGeometries();
};

#endif /* QGENERICLISTVIEW_H */

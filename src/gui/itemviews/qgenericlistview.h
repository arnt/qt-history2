#ifndef QGENERICLISTVIEW_H
#define QGENERICLISTVIEW_H

#include <qabstractitemview.h>

class QGenericListViewItem;
class QGenericListViewPrivate;

class QGenericListView : public QAbstractItemView
{
    friend class QGenericListViewPrivate;

    Q_OBJECT

public:
    enum Flow { TopToBottom = 0, LeftToRight = 1 };
    enum Wrap { Off = 0, On = 1 };
    enum Movement { Static, Free, Snap };

    QGenericListView(QGenericItemModel *model, QWidget *parent = 0, const char *name = 0);
    ~QGenericListView();

    void setMovement(Movement movement);
    void setFlow(Flow flow);
    void setWrapping(Wrap wrap);
    void setSpacing(int space);
    void setGridSize(const QSize &size);
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionUpdateMode mode);

protected:
    void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsRemoved(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    
    void startItemsLayout();
    bool doItemsLayout(int num);
    void stopItemsLayout();
    
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDropEvent(QDropEvent *e);
    QDragObject *dragObject();
    void startDrag();
    
    void getViewOptions(QItemOptions *options) const;
    void drawContents(QPainter *p, int cx, int cy, int cw, int ch);
    QModelIndex itemAt(int x, int y) const;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state);
    QRect itemRect(const QModelIndex &item) const;
    
    QRect selectionRect(const QItemSelection *selection) const;
    
    void doItemsLayout(const QRect &bounds, const QModelIndex &first, const QModelIndex &last);
    void doStaticLayout(const QRect &bounds, int first, int last);
    void doDynamicLayout(const QRect &bounds, int first, int last);

    bool supportsDragAndDrop() const;

    int itemIndex(QGenericListViewItem *item) const;
    void insertItem(int index, QGenericListViewItem &item);
    void removeItem(int index);
    void moveItem(int index, const QPoint &dest);

    void updateGeometries();
    
private:
    QGenericListViewPrivate *d;
};

#endif /* QGENERICLISTVIEW_H */

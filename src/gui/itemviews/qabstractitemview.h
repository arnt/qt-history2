#ifndef QABSTRACTITEMVIEW_H
#define QABSTRACTITEMVIEW_H

#ifndef QT_H
#include <qviewport.h>
#include <qgenericitemmodel.h>
#include <qitemselectionmodel.h>
#include <qitemdelegate.h>
#include <qdragobject.h>
#include <qevent.h>
#endif

class QAbstractItemViewPrivate;

class Q_GUI_EXPORT QAbstractItemView : public QViewport
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractItemView);

public:

    QAbstractItemView(QGenericItemModel *model, QWidget *parent = 0);
    ~QAbstractItemView();

    QGenericItemModel *model() const;

    void setHorizontalFactor(int factor);
    int horizontalFactor() const;
    void setVerticalFactor(int factor);
    int verticalFactor() const;

    virtual void clearSelections();
    virtual void setSelectionMode(QItemSelectionModel::SelectionMode mode);
    QItemSelectionModel::SelectionMode selectionMode() const;

    virtual void setSelectionModel(QItemSelectionModel *selectionModel);
    QItemSelectionModel* selectionModel() const;

    void setCurrentItem(const QModelIndex &data);
    QModelIndex currentItem() const;

    void setItemDelegate(QItemDelegate *delegate);
    QItemDelegate *itemDelegate() const;

    virtual void setStartEditActions(int actions);
    int startEditActions() const;

    bool eventFilter(QObject *object, QEvent *event);

    // FIXME: duplicates QScrollView API
    virtual int contentsX() const = 0;
    virtual int contentsY() const = 0;
    virtual int contentsWidth() const = 0;
    virtual int contentsHeight() const = 0;
    
    int visibleWidth() const;
    int visibleHeight() const;
    QRect visibleRect() const;

protected slots:
    virtual void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void contentsRemoved(const QModelIndex &parent,
				 const QModelIndex &topLeft, const QModelIndex &bottomRight);

    virtual void selectionChanged(const QItemSelectionPointer &deselected,
				  const QItemSelectionPointer &selected);

    virtual void currentChanged(const QModelIndex &old, const QModelIndex &current);

    virtual void startItemsLayout();
    virtual bool doItemsLayout(int num);
    virtual void updateCurrentEditor();
    virtual void updateGeometries();


signals:
    void needMore();

protected:
    QAbstractItemView(QAbstractItemViewPrivate &, QGenericItemModel *model, QWidget *parent = 0);

    enum CursorAction { MoveDown, MoveUp, MoveLeft, MoveRight, MoveHome, MoveEnd, MovePageUp, MovePageDown };
    virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state) = 0;
    
    inline QModelIndex itemAt(const QPoint &p) const { return itemAt(p.x(), p.y()); }
    virtual QModelIndex itemAt(int x, int y) const = 0;

    virtual QRect itemViewportRect(const QModelIndex &item) const = 0;
    virtual void ensureItemVisible(const QModelIndex &item) = 0;
    
    virtual void updateItem(const QModelIndex &item);
    virtual void updateRow(const QModelIndex &item);
    virtual void updateViewport(const QRect &rect);
    inline void updateViewport() { viewport()->update(); }

    virtual void setSelection(const QRect&, QItemSelectionModel::SelectionUpdateMode) = 0;
    virtual QRect selectionRect(const QItemSelection *selection) const = 0;
    
    virtual bool startEdit(const QModelIndex &item, QItemDelegate::StartEditAction action, QEvent *event);
    virtual void endEdit(const QModelIndex &item, bool accept);

    virtual QItemSelectionModel::SelectionBehavior selectionBehavior() const;
    virtual QItemSelectionModel::SelectionUpdateMode selectionUpdateMode(
	ButtonState state, const QModelIndex &item = QModelIndex(),
	QEvent::Type type = QEvent::None, Qt::Key key = Qt::Key_unknown) const;

    void clearArea(QPainter *painter, const QRect &rect) const;

    virtual bool supportsDragAndDrop() const;
    virtual QDragObject *dragObject();
    virtual void startDrag();

    QRect dragRect() const;

    void setRoot(const QModelIndex &item);
    QModelIndex root() const;

    // FIXME: find better solutions
    virtual void getViewOptions(QItemOptions *options) const;

    enum State { NoState, Dragging, Selecting, Editing, Opening, Closing };
    State state() const;
    void setState(State state);
    
    virtual void viewportMousePressEvent(QMouseEvent *e);
    virtual void viewportMouseMoveEvent(QMouseEvent *e);
    virtual void viewportMouseReleaseEvent(QMouseEvent *e);
    virtual void viewportMouseDoubleClickEvent(QMouseEvent *e);
    virtual void viewportContextMenuEvent(QContextMenuEvent *e);
    virtual void viewportDragEnterEvent(QDragEnterEvent *e);
    virtual void viewportDragMoveEvent(QDragMoveEvent *e);
    virtual void viewportDragLeaveEvent(QDragLeaveEvent *e);
    virtual void viewportDropEvent(QDropEvent *e);
    
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);
    
private slots:
    void fetchMore();
};

class QItemViewDragObjectPrivate;

class QItemViewDragObject : public QDragObject
{
public:
    QItemViewDragObject(QAbstractItemView *dragSource);
    ~QItemViewDragObject();

    void append(QModelIndex &item);
    void set(QModelIndexList &itemss);

    const char *format(int i) const;
    bool canDecode(QMimeSource *src) const;
    QByteArray encodedData(const char *mime) const;
    bool decode(QMimeSource *src) const;

private:
    QItemViewDragObjectPrivate *d;
};

#endif /* QABSTRACTITEMVIEW_H */

#ifndef QABSTRACTITEMVIEW_H
#define QABSTRACTITEMVIEW_H

#ifndef QT_H
#include <qscrollview.h>
#include <qgenericitemmodel.h>
#include <qitemselectionmodel.h>
#include <qitemdelegate.h>
#include <qdragobject.h>
#endif

class QAbstractItemViewPrivate;

class Q_GUI_EXPORT QAbstractItemView : public QScrollView
{
    friend class QAbstractItemViewPrivate;

    Q_OBJECT

public:

    QAbstractItemView(QGenericItemModel *model, QWidget *parent = 0, const char *name = 0);
    ~QAbstractItemView();

    QGenericItemModel *model() const;

//     virtual void sort(int column, SortOrder order);
//     int sorted(int row) const;

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

protected slots:
    virtual void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void contentsRemoved(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    virtual void selectionChanged(const QItemSelectionPointer &deselected,
				  const QItemSelectionPointer &selected);

    virtual void currentChanged(const QModelIndex &old, const QModelIndex &current);

    virtual void startItemsLayout();
    virtual bool doItemsLayout(int num);
    virtual void updateCurrentEditor();
    virtual void updateGeometries();

protected:
    virtual void updateItem(const QModelIndex &item);
    virtual void updateRow(const QModelIndex &item);
    virtual void ensureItemVisible(const QModelIndex &item);

    inline QModelIndex itemAt(const QPoint &p) const { return itemAt(p.x(), p.y()); }
    virtual QModelIndex itemAt(int x, int y) const = 0;

    enum CursorAction {
	MoveDown,
	MoveUp,
	MoveLeft,
	MoveRight,
	MoveHome,
	MoveEnd,
	MovePageUp,
	MovePageDown
    };

    virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state) = 0;
    virtual QRect itemRect(const QModelIndex &item) const = 0;

    virtual void setSelection(const QRect&, QItemSelectionModel::SelectionUpdateMode) = 0;
    virtual QRect selectionRect(const QItemSelection *selection) const = 0;

    virtual bool handleEdit(const QModelIndex &item, QItemDelegate::StartEditAction action, QEvent *event);
    virtual void endEdit(const QModelIndex &item, bool accept);

    virtual QItemSelectionModel::SelectionBehavior selectionBehavior() const;
    virtual QItemSelectionModel::SelectionUpdateMode selectionUpdateMode(ButtonState state) const;

    void drawSelectionRect(QPainter *painter, const QRect &rect) const;
    void clearArea(QPainter *painter, const QRect &rect) const;

    virtual bool supportsDragAndDrop() const;
    virtual QDragObject *dragObject();
    virtual void startDrag();

    QRect dragRect() const;

    void setRoot(const QModelIndex &item);
    QModelIndex root() const;

    // FIXME: find better solutions
    virtual void getViewOptions(QItemOptions *options) const;

    enum State {
	NoState,
	Dragging,
	Selecting,
	Editing,
	Opening,
	Closing
    };

    State state() const;
    void setState(State state);

protected:
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsMouseDoubleClickEvent(QMouseEvent *e);
    void contentsDragEnterEvent(QDragEnterEvent *e);
    void contentsDropEvent(QDropEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);

private slots:
    void fetchMore();

private:
    QAbstractItemViewPrivate *d;
};

class QItemViewDragObjectPrivate;

class QItemViewDragObject : public QDragObject
{
public:
    QItemViewDragObject(QAbstractItemView *dragSource, const char *name = 0);
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

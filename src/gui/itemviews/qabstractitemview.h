#ifndef QABSTRACTITEMVIEW_H
#define QABSTRACTITEMVIEW_H

#ifndef QT_H
#include <qviewport.h>
#include <qabstractitemmodel.h>
#include <qitemselectionmodel.h>
#include <qabstractitemdelegate.h>
#include <qdragobject.h>
#include <qevent.h>
#endif

class QAbstractItemViewPrivate;

class Q_GUI_EXPORT QAbstractItemView : public QViewport
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractItemView)

public:

    enum SelectionMode {
        Single,
        Multi,
        Extended
    };

    enum SelectionBehavior {
        SelectItems,
        SelectRows,
        SelectColumns
    };

    QAbstractItemView(QAbstractItemModel *model, QWidget *parent = 0);
    ~QAbstractItemView();

    QAbstractItemModel *model() const;

    void setHorizontalFactor(int factor);
    int horizontalFactor() const;
    void setVerticalFactor(int factor);
    int verticalFactor() const;

    void clearSelections();
    void setSelectionModel(QItemSelectionModel *selectionModel);
    QItemSelectionModel* selectionModel() const;
    void setSelectionMode(int mode);
    int selectionMode() const;
    void setSelectionBehavior(int behavior);
    int selectionBehavior() const;
    void setCurrentItem(const QModelIndex &data);
    QModelIndex currentItem() const;
    QModelIndex root() const;

    void setItemDelegate(QAbstractItemDelegate *delegate);
    QAbstractItemDelegate *itemDelegate() const;

    void setStartEditActions(int actions);
    int startEditActions() const;

    bool eventFilter(QObject *object, QEvent *event);

public slots:
    void setRoot(const QModelIndex &index);
    void edit(const QModelIndex &index);

protected slots:
    virtual void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void contentsRemoved(const QModelIndex &parent,
                                 const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void selectionChanged(const QItemSelection &deselected, const QItemSelection &selected);
    virtual void currentChanged(const QModelIndex &old, const QModelIndex &current);
    virtual void startItemsLayout();
    virtual bool doItemsLayout(int num);
    virtual void updateCurrentEditor();
    virtual void updateGeometries();

    virtual void verticalScrollbarAction(int action);
    virtual void horizontalScrollbarAction(int action);

signals:
    void needMore();
    void pressed(const QModelIndex &index, int button);
    void clicked(const QModelIndex &index, int button);
    void doubleClicked(const QModelIndex &index, int button);
    void returnPressed(const QModelIndex &index);
    void spacePressed(const QModelIndex &index);

protected:
    QAbstractItemView(QAbstractItemViewPrivate &, QAbstractItemModel *model, QWidget *parent = 0);

    enum CursorAction { MoveDown, MoveUp, MoveLeft, MoveRight, MoveHome, MoveEnd, MovePageUp, MovePageDown };
    virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state) = 0;

    inline QModelIndex itemAt(const QPoint &p) const { return itemAt(p.x(), p.y()); }
    virtual QModelIndex itemAt(int x, int y) const = 0;

    virtual int horizontalOffset() const = 0;
    virtual int verticalOffset() const = 0;

    virtual QRect itemViewportRect(const QModelIndex &item) const = 0;
    virtual void ensureItemVisible(const QModelIndex &item) = 0;

    virtual void updateItem(const QModelIndex &item);
    virtual void updateRow(const QModelIndex &item);

    virtual void setSelection(const QRect&, int command) = 0;
    virtual QRect selectionViewportRect(const QItemSelection &selection) const = 0;

    virtual bool startEdit(const QModelIndex &item, QAbstractItemDelegate::StartEditAction action, QEvent *event);
    virtual void endEdit(const QModelIndex &item, bool accept);
    QWidget *currentEditor() const;

    virtual int selectionCommand(ButtonState state,
                                 const QModelIndex &item = QModelIndex(),
                                 QEvent::Type type = QEvent::None,
                                 Qt::Key key = Qt::Key_unknown) const;

    void clearArea(QPainter *painter, const QRect &rect) const;

    virtual bool supportsDragAndDrop() const;
    virtual QDragObject *dragObject();
    virtual void startDrag();

    // FIXME: find better solutions
    virtual void getViewOptions(QItemOptions *options) const;

    enum State { NoState, Dragging, Selecting, Editing, Opening, Closing };
    State state() const;
    void setState(State state);

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
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
    Q_DECLARE_PRIVATE(QItemViewDragObject)
public:
    QItemViewDragObject(QAbstractItemView *dragSource);
    ~QItemViewDragObject();

    void append(QModelIndex &item);
    void set(QModelIndexList &itemss);

    const char *format(int i) const;
    bool canDecode(QMimeSource *src) const;
    QByteArray encodedData(const char *mime) const;
    bool decode(QMimeSource *src) const;
};

#endif /* QABSTRACTITEMVIEW_H */

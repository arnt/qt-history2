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

#ifndef QABSTRACTITEMVIEW_H
#define QABSTRACTITEMVIEW_H

#include <qviewport.h>
#include <qabstractitemmodel.h>
#include <qitemselectionmodel.h>
#include <qabstractitemdelegate.h>

class QMenu;
class QDrag;
class QEvent;
class QAbstractItemViewPrivate;

class Q_GUI_EXPORT QAbstractItemView : public QViewport
{
    Q_OBJECT
    Q_PROPERTY(bool autoScroll READ hasAutoScroll WRITE setAutoScroll)
    Q_PROPERTY(int keyboardInputInterval READ keyboardInputInterval WRITE setKeyboardInputInterval)
    Q_PROPERTY(EditTriggers editTriggers READ editTriggers WRITE setEditTriggers)
    Q_PROPERTY(bool keyTracking READ hasKeyTracking WRITE setKeyTracking)
    Q_PROPERTY(bool tabKeyNavigation READ tabKeyNavigation WRITE setTabKeyNavigation)
    Q_PROPERTY(bool showDropIndicator READ showDropIndicator WRITE setDropIndicatorShown)
    Q_PROPERTY(bool dragEnabled READ dragEnabled WRITE setDragEnabled)
    Q_PROPERTY(bool alternatingRowColors READ alternatingRowColors WRITE setAlternatingRowColors)
    Q_PROPERTY(QColor oddRowColor READ oddRowColor WRITE setOddRowColor)
    Q_PROPERTY(QColor evenRowColor READ evenRowColor WRITE setEvenRowColor)
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)
    Q_PROPERTY(SelectionBehavior selectionBehavior READ selectionBehavior WRITE setSelectionBehavior)
    Q_ENUMS(SelectionMode SelectionBehaviour)
    Q_FLAGS(EditTriggers)

public:
    enum SelectionMode {
        NoSelection,
        SingleSelection,
        MultiSelection,
        ExtendedSelection
    };

    enum SelectionBehavior {
        SelectItems,
        SelectRows,
        SelectColumns
    };

    enum EditTrigger {
        NoEditTriggers = 0,
        CurrentChanged = 1,
        DoubleClicked = 2,
        SelectedClicked = 4,
        EditKeyPressed = 8,
        AnyKeyPressed = 16,
        AllEditTriggers = 31
    };

    Q_DECLARE_FLAGS(EditTriggers, EditTrigger)

    QAbstractItemView(QWidget *parent = 0);
    ~QAbstractItemView();

    virtual void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;

    virtual void setSelectionModel(QItemSelectionModel *selectionModel);
    QItemSelectionModel *selectionModel() const;

    void setItemDelegate(QAbstractItemDelegate *delegate);
    QAbstractItemDelegate *itemDelegate() const;

    void setSelectionMode(QAbstractItemView::SelectionMode mode);
    QAbstractItemView::SelectionMode selectionMode() const;

    void setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior);
    QAbstractItemView::SelectionBehavior selectionBehavior() const;

    QModelIndex currentIndex() const;
    QModelIndex root() const;

    void setEditTriggers(EditTriggers triggers);
    EditTriggers editTriggers() const;

    void setAutoScroll(bool enable);
    bool hasAutoScroll() const;

    void setKeyTracking(bool enable);
    bool hasKeyTracking() const;

    void setTabKeyNavigation(bool enable);
    bool tabKeyNavigation() const;

    void setDropIndicatorShown(bool enable);
    bool showDropIndicator() const;

    void setDragEnabled(bool enable);
    bool dragEnabled() const;

    bool alternatingRowColors() const;
    void setAlternatingRowColors(bool enable);

    QColor oddRowColor() const;
    void setOddRowColor(const QColor &odd);

    QColor evenRowColor() const;
    void setEvenRowColor(const QColor &even);

    virtual void keyboardSearch(const QString &search);
    void setKeyboardInputInterval(int msec);
    int keyboardInputInterval() const;

    virtual QRect itemViewportRect(const QModelIndex &index) const = 0;
    virtual void ensureVisible(const QModelIndex &index) = 0;
    inline QModelIndex itemAt(const QPoint &p) const { return itemAt(p.x(), p.y()); }
    virtual QModelIndex itemAt(int x, int y) const = 0;

    QSize itemSizeHint(const QModelIndex &index) const;
    virtual int rowSizeHint(int row) const;
    virtual int columnSizeHint(int column) const;

    void openPersistentEditor(const QModelIndex &index);
    void closePersistentEditor(const QModelIndex &index);

public slots:
    virtual void reset();
    virtual void setRoot(const QModelIndex &index);
    virtual void doItemsLayout();
    virtual void selectAll();
    void edit(const QModelIndex &index);
    void clearSelection();
    void setCurrentIndex(const QModelIndex &index);

protected slots:
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void rowsInserted(const QModelIndex &parent, int start, int end);
    virtual void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    virtual void updateEditorData();
    virtual void updateEditorGeometries();
    virtual void updateGeometries();
    virtual void verticalScrollbarAction(int action);
    virtual void horizontalScrollbarAction(int action);
    virtual void verticalScrollbarValueChanged(int value);
    virtual void horizontalScrollbarValueChanged(int value);
    virtual void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
    virtual void commitData(QWidget *editor);
    virtual void editorDestroyed(QObject *editor);

signals:
    void rootChanged(const QModelIndex &old, const QModelIndex &root);
    void pressed(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void clicked(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void doubleClicked(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void keyPressed(const QModelIndex &index, Qt::Key key, Qt::KeyboardModifiers modifiers);
    void returnPressed(const QModelIndex &index);
    void aboutToShowContextMenu(QMenu *menu, const QModelIndex &index);
    void itemEntered(const QModelIndex &index, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void viewportEntered(Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

protected:
    QAbstractItemView(QAbstractItemViewPrivate &, QWidget *parent = 0);

    void setHorizontalFactor(int factor);
    int horizontalFactor() const;
    void setVerticalFactor(int factor);
    int verticalFactor() const;

    enum CursorAction { MoveUp, MoveDown, MoveLeft, MoveRight,
                        MoveHome, MoveEnd, MovePageUp, MovePageDown,
                        MoveNext, MovePrevious };
    virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
                                   Qt::KeyboardModifiers modifiers) = 0;

    virtual int horizontalOffset() const = 0;
    virtual int verticalOffset() const = 0;

    virtual bool isIndexHidden(const QModelIndex &index) const = 0;

    virtual void setSelection(const QRect&, QItemSelectionModel::SelectionFlags command) = 0;
    virtual QRect selectionViewportRect(const QItemSelection &selection) const = 0;
    virtual QModelIndexList selectedIndexes() const;

    virtual bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);

    virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index,
                                                                 const QEvent *event = 0) const;

    virtual void startDrag();

    virtual QStyleOptionViewItem viewOptions() const;

    enum State {
        NoState,
        DraggingState,
        DragSelectingState,
        EditingState,
        OpeningState,
        ClosingState
    };

    State state() const;
    void setState(State state);

    void startAutoScroll();
    void stopAutoScroll();
    void doAutoScroll();

    bool event(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void contextMenuEvent(QContextMenuEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
    void timerEvent(QTimerEvent *e);

private:
    Q_DECLARE_PRIVATE(QAbstractItemView)
    Q_DISABLE_COPY(QAbstractItemView)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractItemView::EditTriggers);

#endif

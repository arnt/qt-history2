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
#include <qdragobject.h>
#include <qevent.h>
#include <qdatetime.h>

class QMenu;
class QAbstractItemViewPrivate;

class Q_GUI_EXPORT QAbstractItemView : public QViewport
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractItemView)
    Q_PROPERTY(bool autoScroll READ hasAutoScroll WRITE setAutoScroll)
    Q_PROPERTY(int keyboardInputInterval READ keyboardInputInterval WRITE setKeyboardInputInterval)
    Q_PROPERTY(BeginEditActions beginEditActions READ beginEditActions WRITE setBeginEditActions)
    Q_PROPERTY(bool keyTracking READ hasKeyTracking WRITE setKeyTracking)
    Q_ENUMS(SelectionMode SelectionBehaviour)
    Q_FLAGS(BeginEditActions)
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

    enum BeginEditAction {
        NeverEdit = 0,
        CurrentChanged = 1,
        DoubleClicked = 2,
        SelectedClicked = 4,
        EditKeyPressed = 8,
        AnyKeyPressed = 16,
        AlwaysEdit = 31
    };

    Q_DECLARE_FLAGS(BeginEditActions, BeginEditAction);

    QAbstractItemView(QWidget *parent = 0);
    ~QAbstractItemView();

    virtual void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() const;

    virtual void setSelectionModel(QItemSelectionModel *selectionModel);
    QItemSelectionModel *selectionModel() const;

    void setItemDelegate(QAbstractItemDelegate *delegate);
    QAbstractItemDelegate *itemDelegate() const;

    void setSelectionMode(int mode); // FIXME: make property
    int selectionMode() const;

    void setSelectionBehavior(int behavior); // FIXME: make property
    int selectionBehavior() const;

    QModelIndex currentIndex() const;
    QModelIndex root() const;

    void setBeginEditActions(BeginEditActions actions);
    BeginEditActions beginEditActions() const;

    void setAutoScroll(bool enable);
    bool hasAutoScroll() const;

    void setKeyTracking(bool enable);
    bool hasKeyTracking() const;

    virtual void keyboardSearch(const QString &search);
    void setKeyboardInputInterval(int msec);
    int keyboardInputInterval() const;

    virtual QRect itemViewportRect(const QModelIndex &index) const = 0;
    virtual void ensureItemVisible(const QModelIndex &index) = 0;
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
    void edit(const QModelIndex &index);
    virtual void selectAll();
    void clearSelection();
    void setCurrentIndex(const QModelIndex &index);

protected slots:
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void rowsInserted(const QModelIndex &parent, int start, int end);
    virtual void rowsRemoved(const QModelIndex &parent, int start, int end);
    virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    virtual void updateEditorData();
    virtual void updateEditorGeometries();
    virtual void updateGeometries();
    virtual void verticalScrollbarAction(int action);
    virtual void horizontalScrollbarAction(int action);
    virtual void selectionModelDestroyed();
    virtual void doneEditing(QWidget *editor);
    virtual void commitData(QWidget *editor);
    virtual void editorDestroyed(QObject *editor);

signals:
    void rootChanged(const QModelIndex &old, const QModelIndex &root);
    void pressed(const QModelIndex &index, Qt::ButtonState button);
    void clicked(const QModelIndex &index, Qt::ButtonState button);
    void doubleClicked(const QModelIndex &index, Qt::ButtonState button);
    void keyPressed(const QModelIndex &index, Qt::Key key, Qt::ButtonState state);
    void returnPressed(const QModelIndex &index);
    void aboutToShowContextMenu(QMenu *menu, const QModelIndex &index);
    void itemEntered(const QModelIndex &index, Qt::ButtonState state);
    void viewportEntered(Qt::ButtonState state);

protected:
    QAbstractItemView(QAbstractItemViewPrivate &, QWidget *parent = 0);

    void setHorizontalFactor(int factor);
    int horizontalFactor() const;
    void setVerticalFactor(int factor);
    int verticalFactor() const;

    enum CursorAction { MoveUp, MoveDown, MoveLeft, MoveRight,
                        MoveHome, MoveEnd, MovePageUp, MovePageDown };
    virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
                                   Qt::ButtonState state) = 0;

    virtual int horizontalOffset() const = 0;
    virtual int verticalOffset() const = 0;

    virtual bool isIndexHidden(const QModelIndex &index) const = 0;

    virtual void setSelection(const QRect&, QItemSelectionModel::SelectionFlags command) = 0;
    virtual QRect selectionViewportRect(const QItemSelection &selection) const = 0;

    virtual bool edit(const QModelIndex &index, BeginEditAction action, QEvent *event);
    virtual void endEdit(const QModelIndex &index, bool accepted = true);

    virtual QItemSelectionModel::SelectionFlags selectionCommand(Qt::ButtonState state,
                                                                 const QModelIndex &index,
                                                                 QEvent::Type type = QEvent::None,
                                                                 Qt::Key key = Qt::Key_unknown) const;

    virtual QDragObject *dragObject();
    virtual void startDrag();
    virtual bool isDragEnabled(const QModelIndex &index) const;

    virtual QStyleOptionViewItem viewOptions() const;

    enum State { NoState, Dragging, Selecting, Editing, Opening, Closing };
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
    void dropEvent(QDropEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
    void timerEvent(QTimerEvent *e);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractItemView::BeginEditActions);

#endif

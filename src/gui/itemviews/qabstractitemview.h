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

#ifndef QABSTRACTITEMVIEW_H
#define QABSTRACTITEMVIEW_H

#ifndef QT_H
#include <qviewport.h>
#include <qabstractitemmodel.h>
#include <qitemselectionmodel.h>
#include <qabstractitemdelegate.h>
#include <qdragobject.h>
#include <qevent.h>
#include <qdatetime.h>
#endif

class QMenu;
class QAbstractItemViewPrivate;

class Q_GUI_EXPORT QAbstractItemView : public QViewport
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractItemView)
    Q_PROPERTY(int keyboardInputInterval READ keyboardInputInterval WRITE setKeyboardInputInterval)
    Q_PROPERTY(bool autoScroll READ autoScroll WRITE setAutoScroll)
    Q_ENUMS(SelectionMode SelectionBehaviour)

public:

    enum SelectionMode {
        SingleSelection,
        MultiSelection,
        ExtendedSelection
    };

    enum SelectionBehavior {
        SelectItems,
        SelectRows,
        SelectColumns
    };

    QAbstractItemView(QAbstractItemModel *model, QWidget *parent = 0);
    ~QAbstractItemView();

    QAbstractItemModel *model() const;

    void setSelectionModel(QItemSelectionModel *selectionModel);
    QItemSelectionModel *selectionModel() const;
    void setSelectionMode(int mode);
    int selectionMode() const;
    void setSelectionBehavior(int behavior);
    int selectionBehavior() const;
    QModelIndex currentItem() const;
    QModelIndex root() const;

    void setItemDelegate(QAbstractItemDelegate *delegate);
    QAbstractItemDelegate *itemDelegate() const;

    void setStartEditActions(int actions);
    int startEditActions() const;

    void setAutoScroll(bool b);
    bool autoScroll() const;

    virtual void keyboardSearch(const QString &search);
    void setKeyboardInputInterval(int msec);
    int keyboardInputInterval() const;

    virtual QRect itemViewportRect(const QModelIndex &item) const = 0;
    virtual void ensureItemVisible(const QModelIndex &item) = 0;
    inline QModelIndex itemAt(const QPoint &p) const { return itemAt(p.x(), p.y()); }
    virtual QModelIndex itemAt(int x, int y) const = 0;

    QSize itemSizeHint(const QModelIndex &index) const;

    virtual int rowSizeHint(int row) const;
    virtual int columnSizeHint(int column) const;

    void setPersistentEditor(const QModelIndex &index, QWidget *editor = 0);

public slots:
    virtual void setRoot(const QModelIndex &index);
    virtual void doItemsLayout();
    void edit(const QModelIndex &index);
    void clearSelections();
    void setCurrentItem(const QModelIndex &index);

protected slots:
    virtual void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void contentsRemoved(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void selectionChanged(const QItemSelection &deselected, const QItemSelection &selected);
    virtual void currentChanged(const QModelIndex &old, const QModelIndex &current);
    virtual void updateEditors();
    virtual void updateGeometries();
    virtual void verticalScrollbarAction(int action);
    virtual void horizontalScrollbarAction(int action);

signals:
    void needMore();
    void rootChanged(const QModelIndex &old, const QModelIndex &root);
    void pressed(const QModelIndex &index, int button);
    void clicked(const QModelIndex &index, int button);
    void doubleClicked(const QModelIndex &index, int button);
    void returnPressed(const QModelIndex &index);
    void spacePressed(const QModelIndex &index);
    void deletePressed(const QModelIndex &index);
    void aboutToShowContextMenu(QMenu *menu, const QModelIndex &index);

protected:
    QAbstractItemView(QAbstractItemViewPrivate &, QAbstractItemModel *model, QWidget *parent = 0);

    bool eventFilter(QObject *object, QEvent *event);

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

    virtual void setSelection(const QRect&, QItemSelectionModel::SelectionFlags command) = 0;
    virtual QRect selectionViewportRect(const QItemSelection &selection) const = 0;

    virtual bool startEdit(const QModelIndex &item,
                           QAbstractItemDelegate::StartEditAction action, QEvent *event);
    virtual void endEdit(const QModelIndex &item, bool accept);
    QWidget *currentEditor() const;

    virtual QItemSelectionModel::SelectionFlags selectionCommand(Qt::ButtonState state,
                                                                 const QModelIndex &index = QModelIndex(),
                                                                 QEvent::Type type = QEvent::None,
                                                                 Qt::Key key = Qt::Key_unknown) const;

    virtual bool supportsDragAndDrop() const;
    virtual QDragObject *dragObject();
    virtual void startDrag();

    virtual QItemOptions viewOptions() const;

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
    void showEvent(QShowEvent *e);
    void timerEvent(QTimerEvent *e);

private slots:
    void fetchMore();
};

#endif /* QABSTRACTITEMVIEW_H */

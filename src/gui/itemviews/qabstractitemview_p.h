#ifndef QABSTRACTITEMVIEW_P_H
#define QABSTRACTITEMVIEW_P_H

#include <private/qviewport_p.h>
#include <qrubberband.h>

class QAbstractItemViewPrivate : public QViewportPrivate
{
    Q_DECLARE_PUBLIC(QAbstractItemView);

public:
    QAbstractItemViewPrivate();
    virtual ~QAbstractItemViewPrivate();

    void init();

    inline bool shouldEdit(const QModelIndex &item, QAbstractItemDelegate::StartEditAction action)
        { return q_func()->model()->isEditable(item) && (action & startEditActions); }

    bool createEditor(const QModelIndex &item, QAbstractItemDelegate::StartEditAction action, QEvent *event);
//     bool sendItemEvent(const QModelIndex &data, QEvent *event);
//     QWidget *findPersistentEditor(const QModelIndexPtr &item) const;
//     void insertPersistentEditor(const QModelIndexPtr &item, QWidget *editor);

    QAbstractItemModel *model;
    QPointer<QWidget> currentEditor;
    QModelIndex editItem;
    mutable QAbstractItemDelegate *delegate;
    QItemSelectionModel *selectionModel;
    int selectionMode, selectionBehavior;
    QRubberBand *rubberBand;
 //    QVector<int> sorting;
//     int sortColumn;

    // #### this datastructur is far to inefficient. We need a faster
    // #### way to associate data with an item and look it up.
    // use QHash<QGenericModelItenPtr, QWidget*>
    //QList<QPair<QModelIndex, QWidget*> > persistentEditors;

    bool layoutLock; // FIXME: this is because the layout will trigger resize events
    //QRect dragRect;
    QModelIndex pressedItem;
    Qt::ButtonState pressedState;
    QPoint pressedPosition;
    QAbstractItemView::State state;
    QPoint cursorIndex;
    int startEditActions;

    QModelIndex root;
    int horizontalFactor;
    int verticalFactor;
};

#include <private/qdragobject_p.h>

class QItemViewDragObjectPrivate : public QDragObjectPrivate
{
    Q_DECLARE_PUBLIC(QItemViewDragObject);
public:
//    static bool decode(QMimeSource *src, QModelIndexList &items);
    QModelIndexList items;
    QAbstractItemModel *model;
};

#endif

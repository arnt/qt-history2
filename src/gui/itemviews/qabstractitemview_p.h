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

#ifndef QABSTRACTITEMVIEW_P_H
#define QABSTRACTITEMVIEW_P_H

class QRubberBand;

#include <private/qviewport_p.h>
#include <qdatetime.h>
#include <qmap.h>

class QAbstractItemViewPrivate : public QViewportPrivate
{
    Q_DECLARE_PUBLIC(QAbstractItemView)

public:
    QAbstractItemViewPrivate();
    virtual ~QAbstractItemViewPrivate();

    void init();

    void fetchMore();
    bool shouldEdit(QAbstractItemView::BeginEditAction action, const QModelIndex &index);
    bool shouldAutoScroll(const QPoint &pos);
    void doDelayedItemsLayout();

    QWidget *editor(const QModelIndex &index, const QStyleOptionViewItem &options);

    QItemSelectionModel::SelectionFlags multiSelectionCommand(const QModelIndex &index,
                                                              const QEvent *event) const;
    QItemSelectionModel::SelectionFlags extendedSelectionCommand(const QModelIndex &index,
                                                                 const QEvent *event) const;

    inline QItemSelectionModel::SelectionFlags selectionBehaviorFlags() const
    {
        switch (selectionBehavior) {
        case QAbstractItemView::SelectRows: return QItemSelectionModel::Rows;
        case QAbstractItemView::SelectColumns: return QItemSelectionModel::Columns;
        case QAbstractItemView::SelectItems: default: return QItemSelectionModel::NoUpdate;
        }
    }

    mutable QAbstractItemModel *model;
    mutable QAbstractItemDelegate *delegate;
    mutable QItemSelectionModel *selectionModel;
    QAbstractItemView::SelectionMode selectionMode;
    QAbstractItemView::SelectionBehavior selectionBehavior;

    QMap<QPersistentModelIndex, QWidget*> editors;
    QList<QWidget*> persistent;

    QPersistentModelIndex enteredItem;
    QPersistentModelIndex pressedItem;
    Qt::KeyboardModifiers pressedModifiers;
    QPoint pressedPosition;

    QAbstractItemView::State state;
    QPoint cursorIndex;
    QAbstractItemView::BeginEditActions beginEditActions;

    QPersistentModelIndex root;
    int horizontalFactor;
    int verticalFactor;

    bool hasKeyTracking;
    QString keyboardInput;
    QTime keyboardInputTime;
    int inputInterval;

    bool autoScroll;
    int autoScrollTimer;
    int autoScrollMargin;
    int autoScrollInterval;
    int autoScrollCount;

    bool dragEnabled;
    bool layoutPosted;

    bool alternatingColors;
    QColor oddColor;
    QColor evenColor;
};

/*
  Template functions for vector manipulation.
*/

#include <qvector.h>

template <typename T>
inline int qBinarySearch(const QVector<T> &vec, const T &item, int start, int end)
{
    int i = (start + end + 1) >> 1;
    while (end - start > 0) {
        if (vec.at(i) > item)
            end = i - 1;
        else
            start = i;
        i = (start + end + 1) >> 1;
    }
    return i;
}

template <typename T>
inline int qSortedInsert(QVector<T> &vec, const T &item)
{
    qDebug("inserting not implemented: %Ld", item);
    return -1;
}

template <typename T>
inline void qExpand(QVector<T> &vec, int after, size_t n)
{
    size_t m = vec.size() - after - 1;
    vec.resize(vec.size() + n);
    T *b = static_cast<T *>(vec.data());
    T *src = b + after + 1;
    T *dst = src + n;
    memmove(dst, src, m * sizeof(T));
}

template <typename T>
inline void qCollapse(QVector<T> &vec, int after, size_t n)
{
    if (after + 1 + n < static_cast<size_t>(vec.size())) {
        T *b = vec.data();
        T *dst = b + after + 1;
        T *src = dst + n;
        size_t m = vec.size() - n - after - 1;
        memmove(dst, src, m * sizeof(T));
    }
    vec.resize(vec.size() - n);
}

#endif

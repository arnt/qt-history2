#ifndef QABSTRACTITEMVIEW_P_H
#define QABSTRACTITEMVIEW_P_H

class QRubberBand;

#include <private/qviewport_p.h>

class QAbstractItemViewPrivate : public QViewportPrivate
{
    Q_DECLARE_PUBLIC(QAbstractItemView)

public:
    QAbstractItemViewPrivate();
    virtual ~QAbstractItemViewPrivate();

    void init();

    bool shouldEdit(QAbstractItemDelegate::StartEditAction action, const QModelIndex &index);
    bool shouldAutoScroll(const QPoint &pos);

    QWidget *requestEditor(QAbstractItemDelegate::StartEditAction action,
                           QEvent *event, const QModelIndex &index);
    QWidget *persistentEditor(const QModelIndex &index) const;
    void setPersistentEditor(QWidget *editor, const QModelIndex &index);

    QAbstractItemModel *model;
    QPointer<QWidget> currentEditor;
    QModelIndex editItem;
    mutable QAbstractItemDelegate *delegate;
    QItemSelectionModel *selectionModel;
    int selectionMode;
    int selectionBehavior;

    QMap<QPersistentModelIndex, QWidget*> persistentEditors;

    QModelIndex pressedItem;
    Qt::ButtonState pressedState;
    QPoint pressedPosition;
    QAbstractItemView::State state;
    QPoint cursorIndex;
    int startEditActions;

    QPersistentModelIndex root;
    int horizontalFactor;
    int verticalFactor;

    QString keyboardInput;
    QTime keyboardInputTime;
    int inputInterval;

    bool autoScroll;
    int autoScrollTimer;
    int autoScrollMargin;
    int autoScrollInterval;
    int autoScrollCount;

    QPixmap backBuffer;
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

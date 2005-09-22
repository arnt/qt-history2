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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qabstractscrollarea_p.h"
#include "QtGui/qapplication.h"
#include "QtCore/qdatetime.h"
#include "QtGui/qevent.h"
#include "QtGui/qmime.h"
#include "QtCore/qmap.h"
#include "QtCore/qtimer.h"
#include "QtGui/qregion.h"
#include "QtCore/qdebug.h"

#ifndef QT_NO_ITEMVIEWS

class Q_GUI_EXPORT QAbstractItemViewPrivate : public QAbstractScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(QAbstractItemView)

public:
    QAbstractItemViewPrivate();
    virtual ~QAbstractItemViewPrivate();

    void init();

    void fetchMore();
    bool shouldEdit(QAbstractItemView::EditTrigger trigger, const QModelIndex &index);
    bool shouldAutoScroll(const QPoint &pos);
    void doDelayedItemsLayout();

    QWidget *editor(const QModelIndex &index, const QStyleOptionViewItem &options);

    QItemSelectionModel::SelectionFlags multiSelectionCommand(const QModelIndex &index,
                                                              const QEvent *event) const;
    QItemSelectionModel::SelectionFlags extendedSelectionCommand(const QModelIndex &index,
                                                                 const QEvent *event) const;
    QItemSelectionModel::SelectionFlags contiguousSelectionCommand(const QModelIndex &index,
                                                                   const QEvent *event) const;

    inline QItemSelectionModel::SelectionFlags selectionBehaviorFlags() const
    {
        switch (selectionBehavior) {
        case QAbstractItemView::SelectRows: return QItemSelectionModel::Rows;
        case QAbstractItemView::SelectColumns: return QItemSelectionModel::Columns;
        case QAbstractItemView::SelectItems: default: return QItemSelectionModel::NoUpdate;
        }
    }

#ifndef QT_NO_DRAGANDDROP
    inline bool canDecode(QDropEvent *e) const {
        if (!model)
            return false;
        QStringList modelTypes = model->mimeTypes();
        const QMimeData *mime = e->mimeData();
        for (int i = 0; i < modelTypes.count(); ++i)
            if (mime->hasFormat(modelTypes.at(i))
                && (e->proposedAction() & model->supportedDropActions()))
                return true;
        return false;
    }

    inline void paintDropIndicator(QPainter *painter)
    {
        if (showDropIndicator && state == QAbstractItemView::DraggingState)
            if (dropIndicatorRect.height() == 0) // FIXME: should be painted by style
                painter->drawLine(dropIndicatorRect.topLeft(), dropIndicatorRect.topRight());
            else painter->drawRect(dropIndicatorRect);
    }

    inline QAbstractItemView::DropIndicatorPosition position(const QPoint &pos,
                                                             const QRect &rect,
                                                             int margin) const {
        if (pos.y() - rect.top() < margin) return QAbstractItemView::AboveItem;
        if (rect.bottom() - pos.y() < margin) return QAbstractItemView::BelowItem;
        return QAbstractItemView::OnItem;
    }
#endif

    inline void releaseEditor(QWidget *editor) const {
        editor->removeEventFilter(delegate);
        editor->deleteLater();
    }

    inline void executePostedLayout() const {
        if (layoutPosted) {
            layoutPosted = false;
            const_cast<QAbstractItemView*>(q_func())->doItemsLayout();
        }
    }

    inline void setDirtyRegion(const QRegion &visualRegion) {
        updateRegion += visualRegion;
        if (!updateTimer.isActive())
            updateTimer.start(0, q_func());
    }

    inline void scrollDirtyRegion(int dx, int dy) {
        scrollDelayOffset = QPoint(-dx, -dy);
        updateDirtyRegion();
        scrollDelayOffset = QPoint(0, 0);
    }

    inline void scrollContentsBy(int dx, int dy) {
        scrollDirtyRegion(dx, dy);
        viewport->scroll(dx, dy);
    }
    
    void updateDirtyRegion() {
        updateTimer.stop();
        viewport->update(updateRegion);
        updateRegion = QRegion();
    }

    void removeSelectedRows();

    virtual bool selectionAllowed(const QModelIndex &index) const {
        // in some views we want to go ahead with selections, even if the index is invalid
        return index.isValid();
    }

    QPointer<QAbstractItemModel> model;
    QPointer<QAbstractItemDelegate> delegate;
    QPointer<QItemSelectionModel> selectionModel;

    QAbstractItemView::SelectionMode selectionMode;
    QAbstractItemView::SelectionBehavior selectionBehavior;

    QMap<QPersistentModelIndex, QWidget*> editors;
    QList<QWidget*> persistent;

    QPersistentModelIndex enteredIndex;
    QPersistentModelIndex pressedIndex;
    Qt::KeyboardModifiers pressedModifiers;
    QPoint pressedPosition;

    QAbstractItemView::State state;
    QPoint cursorIndex;
    QAbstractItemView::EditTriggers editTriggers;

    QPersistentModelIndex root;
    QPersistentModelIndex hover;
    int horizontalStepsPerItem;
    int verticalStepsPerItem;

    bool tabKeyNavigation;

#ifndef QT_NO_DRAGANDDROP
    bool showDropIndicator;
    QRect dropIndicatorRect;
    bool dragEnabled;
#endif
    QAbstractItemView::DropIndicatorPosition dropIndicatorPosition;

    QString keyboardInput;
    QTime keyboardInputTime;

    bool autoScroll;
    QBasicTimer autoScrollTimer;
    int autoScrollMargin;
    int autoScrollInterval;
    int autoScrollCount;

    mutable bool layoutPosted;
    bool alternatingColors;

    QSize iconSize;
    Qt::TextElideMode textElideMode;

    QRegion updateRegion; // used for the internal update system
    QBasicTimer updateTimer;

    QPoint scrollDelayOffset;
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

#endif // QT_NO_ITEMVIEWS

#endif // QABSTRACTITEMVIEW_P_H

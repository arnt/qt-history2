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

class QRubberBand;

#include <private/qabstractscrollarea_p.h>
#include <qapplication.h>
#include <qdatetime.h>
#include <qevent.h>
#include <qmime.h>
#include <qmap.h>
#include <qtimer.h>
#include <qdebug.h>

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

    inline QItemSelectionModel::SelectionFlags selectionBehaviorFlags() const
    {
        switch (selectionBehavior) {
        case QAbstractItemView::SelectRows: return QItemSelectionModel::Rows;
        case QAbstractItemView::SelectColumns: return QItemSelectionModel::Columns;
        case QAbstractItemView::SelectItems: default: return QItemSelectionModel::NoUpdate;
        }
    }

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

    enum Position { Above, Below, On };

    inline Position position(const QPoint &pos, const QRect &rect, int margin) const {
        if (pos.y() - rect.top() < margin) return Above;
        if (rect.bottom() - pos.y() < margin) return Below;
        return On;
    }

    inline QColor oddRowColor() const {
        if (oddColor.isValid()) return oddColor;
        return q_func()->palette().color(QPalette::Midlight);
    }

    inline QColor evenRowColor() const {
        if (evenColor.isValid()) return evenColor;
        return q_func()->palette().color(QPalette::Base);
    }

    inline void releaseEditor(QWidget *editor) const {
        editor->removeEventFilter(delegate);
        editor->deleteLater();
    }

    inline void executePostedLayout() const {
        if (layoutPosted) const_cast<QAbstractItemView*>(q_func())->doItemsLayout();
    }

    inline void setDirtyRect(const QRect &visualRect) {
        updateRect |= visualRect;
        if (!updateTimer.isActive())
            updateTimer.start(0, q_func());
    }

    inline void repaintDirtyRect() {
        updateTimer.stop();
        viewport->repaint(updateRect);
        QApplication::syncX();
        updateRect = QRect();
    }

    void removeSelectedRows();

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
    int horizontalStepsPerItem;
    int verticalStepsPerItem;

    bool tabKeyNavigation;
    bool showDropIndicator;
    bool dragEnabled;
    QString keyboardInput;
    QTime keyboardInputTime;

    bool autoScroll;
    QBasicTimer autoScrollTimer;
    int autoScrollMargin;
    int autoScrollInterval;
    int autoScrollCount;

    bool layoutPosted;

    bool alternatingColors;
    QColor oddColor;
    QColor evenColor;

    QSize iconSize;

    QRubberBand *dropIndicator;

    QRect updateRect; // used for the internal update system
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

#endif // QABSTRACTITEMVIEW_P_H

/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOLUMNVIEW_P_H
#define QCOLUMNVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QT_NO_QCOlUMNVIEW

#include "qcolumnview.h"
#include <private/qabstractitemview_p.h>

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qtimeline.h>
#include <QtGui/qabstractitemdelegate.h>
#include <QtGui/qabstractitemview.h>
#include <QtGui/qitemdelegate.h>
#include <qlistview.h>
#include <qevent.h>
#include <qscrollbar.h>

class QColumnViewPreviewColumn : public QAbstractItemView {

public:
    QColumnViewPreviewColumn(QWidget *widget) : QAbstractItemView(){
        previewWidget = widget;
        setMinimumWidth(previewWidget->minimumWidth());
    }

    void resizeEvent(QResizeEvent * event){
        previewWidget->resize(
                qMax(previewWidget->minimumWidth(), event->size().width()),
                previewWidget->height());
        QSize p = viewport()->size();
        QSize v = previewWidget->size();
        horizontalScrollBar()->setRange(0, v.width() - p.width());
        horizontalScrollBar()->setPageStep(p.width());
        verticalScrollBar()->setRange(0, v.height() - p.height());
        verticalScrollBar()->setPageStep(p.height());

        QAbstractScrollArea::resizeEvent(event);
    }

    QRect visualRect(const QModelIndex &) const
    {
        return QRect();
    }
    void scrollTo(const QModelIndex &, ScrollHint)
    {
    }
    QModelIndex indexAt(const QPoint &) const
    {
        return QModelIndex();
    }
    QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers)
    {
        return QModelIndex();
    }
    int horizontalOffset () const {
        return 0;
    }
    int verticalOffset () const {
        return 0;
    }
    QRegion visualRegionForSelection(const QItemSelection &) const
    {
        return QRegion();
    }
    bool isIndexHidden(const QModelIndex &) const
    {
        return false;
    }
    void setSelection(const QRect &, QItemSelectionModel::SelectionFlags)
    {
    }
private:
    QWidget *previewWidget;
};

class QColumnViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QColumnView)

public:
    QColumnViewPrivate();
    ~QColumnViewPrivate();

    QAbstractItemView *createColumn(const QModelIndex &index, bool show);

    void updateScrollbars();
    void closeColumns(const QModelIndex &parent = QModelIndex(), bool build = false);
    void doLayout();
    void setPreviewWidget(QWidget *widget);

    void _q_gripMoved(int offset);
    void _q_changeCurrentColumn();
    void _q_clicked(const QModelIndex &index);

    QList<QAbstractItemView*> columns;
    QVector<int> columnSizes; // used during init and corner moving
    bool showResizeGrips;
    int offset;
    QTimeLine currentAnimation;
    QWidget *previewWidget;
    QAbstractItemView *previewColumn;
};

/*!
 * This is a delegate that will paint the triangle
 */
class QColumnViewDelegate : public QItemDelegate
{

public:
    explicit QColumnViewDelegate(QObject *parent = 0) : QItemDelegate(parent) {};
    ~QColumnViewDelegate() {};

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};
#endif // QT_NO_QCOLUMNVIEW

#endif //QCOLUMNVIEW_P_H


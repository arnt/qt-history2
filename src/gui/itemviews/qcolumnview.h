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

#ifndef QCOLUMNVIEW_H
#define QCOLUMNVIEW_H

#include <QtGui/qabstractitemview.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_COLUMNVIEW

class QColumnViewPrivate;

class Q_GUI_EXPORT QColumnView : public QAbstractItemView {

Q_OBJECT
    Q_PROPERTY(bool resizeGripsVisible READ resizeGripsVisible WRITE setResizeGripsVisible)

Q_SIGNALS:
    void updatePreviewWidget(const QModelIndex &index);

public:
    explicit QColumnView(QWidget *parent = 0);
    ~QColumnView();

    // QAbstractItemView overloads
    QModelIndex indexAt(const QPoint &point) const;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
    QSize sizeHint() const;
    QRect visualRect(const QModelIndex &index) const;
    void setModel(QAbstractItemModel *model);
    void setSelectionModel(QItemSelectionModel * selectionModel);
    void setRootIndex(const QModelIndex &index);
    void selectAll();

    // QColumnView functions
    void setResizeGripsVisible(bool visible);
    bool resizeGripsVisible() const;

    QWidget *previewWidget() const;
    void setPreviewWidget(QWidget *widget);

    void setColumnWidths(const QList<int> &list);
    QList<int> columnWidths() const;

protected:
    QColumnView(QColumnViewPrivate &dd, QWidget *parent = 0);

    // QAbstractItemView overloads
    bool isIndexHidden(const QModelIndex &index) const;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    void resizeEvent(QResizeEvent *event);
    void setSelection(const QRect & rect, QItemSelectionModel::SelectionFlags command);
    QRegion visualRegionForSelection(const QItemSelection &selection) const;
    int horizontalOffset() const;
    int verticalOffset() const;
    void scrollContentsBy(int dx, int dy);

    // QColumnView functions
    virtual QAbstractItemView* createColumn(const QModelIndex &rootIndex);
    void initilizeColumn(QAbstractItemView *column) const;

protected Q_SLOTS:
    // QAbstractItemView overloads
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    Q_DECLARE_PRIVATE(QColumnView)
    Q_DISABLE_COPY(QColumnView)
    Q_PRIVATE_SLOT(d_func(), void _q_gripMoved(int))
    Q_PRIVATE_SLOT(d_func(), void _q_changeCurrentColumn())
    Q_PRIVATE_SLOT(d_func(), void _q_clicked(const QModelIndex &))
};

#endif // QT_NO_COLUMNVIEW

QT_END_HEADER

#endif // QCOLUMNVIEW_H


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

#ifndef QGENERICTREEVIEW_H
#define QGENERICTREEVIEW_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QGenericTreeViewPrivate;
class QGenericHeader;

class Q_GUI_EXPORT QGenericTreeView : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGenericTreeView)
    Q_PROPERTY(bool showRootDecoration READ isRootDecorationShown WRITE showRootDecoration)
    Q_PROPERTY(int indentation READ indentation WRITE setIndentation)

public:
    QGenericTreeView(QAbstractItemModel *model, QWidget *parent = 0);
    ~QGenericTreeView();

    QGenericHeader *header() const;
    void setHeader(QGenericHeader *header);

    int indentation() const;
    void setIndentation(int i);

    bool isRootDecorationShown() const;
    void showRootDecoration(bool show);

    int columnViewportPosition(int column) const;
    int columnWidth(int column) const;
    int columnAt(int x) const;

    bool isColumnHidden(int column) const;
    bool isOpen(const QModelIndex &index) const;

    QRect itemViewportRect(const QModelIndex &index) const;
    void ensureItemVisible(const QModelIndex &index);
    QModelIndex itemAt(int x, int y) const;
    QModelIndex itemAbove(const QModelIndex &index) const;
    QModelIndex itemBelow(const QModelIndex &index) const;

    void doItemsLayout();

signals:
    void expanded(const QModelIndex &index);
    void collapsed(const QModelIndex &index);

public slots:
    void hideColumn(int column);
    void open(const QModelIndex &index);
    void close(const QModelIndex &index);

protected slots:
    void resizeColumnToContents(int column, bool checkHeader = true);
    void columnWidthChanged(int column, int oldSize, int newSize);
    void columnCountChanged(int oldCount, int newCount);
    void dataChanged();

protected:
    QGenericTreeView(QGenericTreeViewPrivate &dd, QAbstractItemModel *model, QWidget *parent = 0);
    void scrollContentsBy(int dx, int dy);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);

    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state);
    int horizontalOffset() const;
    int verticalOffset() const;

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    QRect selectionViewportRect(const QItemSelection &selection) const;

    void paintEvent(QPaintEvent *e);
    virtual void drawRow(QPainter *painter, const QStyleOptionViewItem &options,
                         const QModelIndex &index) const;
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;

    void mousePressEvent(QMouseEvent *e);

    void updateGeometries();
    void verticalScrollbarAction(int action);
    void horizontalScrollbarAction(int action);

    int columnSizeHint(int column) const;
};

#endif
